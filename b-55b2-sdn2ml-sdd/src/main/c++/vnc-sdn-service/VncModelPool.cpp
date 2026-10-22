#include "VncModelPool.h"
#include "Timestamping.h"
#include <pthread.h>
#include <sched.h>
#include <algorithm>
#include <cstring>
#include <exception>
#include <iostream>

namespace
{
    // Idle worker spins this many times before it starts to yield the CPU.
    // TODO. With isolated cores (isolcpus) yield can be removed.
    const std::size_t kSpinsBeforeYield = 10000;

    void cpuRelax()
    {
#if defined(__x86_64__) || defined(__i386__)
        __builtin_ia32_pause();
#endif
    }

    void pinToCpu(int cpu)
    {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set);
        const int error = ::pthread_setaffinity_np(::pthread_self(), sizeof(set), &set);
        if (error != 0)
            std::cerr << "cannot pin worker to cpu " << cpu << ": " << std::strerror(error) << std::endl;
    }
}

namespace vnc
{
    struct alignas(64) ModelPool::Slot
    {
        enum State : int { idle, busy };

        Slot(const std::string &modelPath, std::size_t workerIndex)
            : model(modelPath), worker(workerIndex) {}

        MlModel model;
        const std::size_t worker;
        std::atomic<int> state{idle};

        // Job. Written by the main thread while the slot is idle.
        Features input{};
        std::int64_t jobCycleId = 0;
        std::int64_t jobInputTimestamp = 0;

        // Result. Written by the worker while the slot is busy.
        double result = 0.0;
        bool resultOk = false;
        std::int64_t resultCycleId = 0;
        std::int64_t resultInputTimestamp = 0;
        std::int64_t runs = 0, errors = 0, maxRunNs = 0;

        // Last published value. Used by the main thread only.
        std::int64_t consumedCycleId = 0;
        bool hasValue = false;
        double lastValue = 0.0;
        std::int64_t lastCycleId = 0;
        std::int64_t lastInputTimestamp = 0;
        std::int64_t notOkCycles = 0;
    };

    ModelPool::ModelPool(const ModelConfigs &configs, const std::string &modelDir,
                         const std::vector<int> &workerCpus)
    {
        std::size_t workerCount = 0;
        for (std::size_t i = 0; i < kPlasmaParamCount; ++i) {
            m_slots[i].reset(new Slot(modelDir + "/" + configs[i].modelFile, configs[i].worker));
            workerCount = std::max(workerCount, configs[i].worker + 1);
            std::cout << configs[i].topic << " model " << configs[i].modelFile
                      << " runs on worker " << configs[i].worker << std::endl;
        }
        // Workers start after all slots are created, m_slots is not changed later
        for (std::size_t w = 0; w < workerCount; ++w) {
            const int cpu = w < workerCpus.size() ? workerCpus[w] : -1;
            m_workers.emplace_back(&ModelPool::workerBody, this, w, cpu);
        }
    }

    ModelPool::~ModelPool()
    {
        stop();
    }

    void ModelPool::stop()
    {
        m_stop.store(true, std::memory_order_relaxed);
        for (std::thread &worker : m_workers)
            if (worker.joinable()) worker.join();
    }

    void ModelPool::dispatch(std::int64_t cycleId, std::int64_t inputTimestamp, const Features &features)
    {
        for (std::unique_ptr<Slot> &slot : m_slots) {
            // Busy model is still calculating an older cycle, its input must not be changed
            if (slot->state.load(std::memory_order_acquire) != Slot::idle) continue;
            slot->input = features;
            slot->jobCycleId = cycleId;
            slot->jobInputTimestamp = inputTimestamp;
            slot->state.store(Slot::busy, std::memory_order_release);
        }
    }

    void ModelPool::collect(std::int64_t cycleId, Outputs &outputs)
    {
        for (std::size_t i = 0; i < kPlasmaParamCount; ++i) {
            Slot &slot = *m_slots[i];
            // Take a new result, also a late one from an older cycle: it is newer than the last value
            if (slot.state.load(std::memory_order_acquire) == Slot::idle &&
                slot.resultCycleId != slot.consumedCycleId) {
                slot.consumedCycleId = slot.resultCycleId;
                if (slot.resultOk) {
                    slot.hasValue = true;
                    slot.lastValue = slot.result;
                    slot.lastCycleId = slot.resultCycleId;
                    slot.lastInputTimestamp = slot.resultInputTimestamp;
                }
            }

            OutputData &output = outputs[i];
            output.cycleId = cycleId;
            output.value = slot.lastValue;
            output.inputTimestamp = slot.lastInputTimestamp;
            if (!slot.hasValue || cycleId - slot.lastCycleId > kMaxStaleCycles)
                output.quality = Quality::invalid;
            else if (slot.lastCycleId == cycleId)
                output.quality = Quality::ok;
            else
                output.quality = Quality::stale;
            if (output.quality != Quality::ok) ++slot.notOkCycles;
        }
    }

    ModelStats ModelPool::stats(std::size_t model) const
    {
        const Slot &slot = *m_slots.at(model);
        return ModelStats{slot.runs, slot.errors, slot.notOkCycles, slot.maxRunNs};
    }

    void ModelPool::workerBody(std::size_t worker, int cpu)
    {
        if (cpu >= 0) pinToCpu(cpu);

        std::vector<Slot *> own;
        for (std::unique_ptr<Slot> &slot : m_slots)
            if (slot->worker == worker) own.push_back(slot.get());

        std::size_t idleSpins = 0;
        while (!m_stop.load(std::memory_order_relaxed)) {
            bool worked = false;
            for (Slot *slot : own) {
                if (slot->state.load(std::memory_order_acquire) != Slot::busy) continue;

                const std::int64_t started = monotonicNs();
                double value = 0.0;
                bool ok = false;
                try {
                    ok = slot->model.run(slot->input, value);
                }
                catch (const std::exception &) {
                    ok = false;
                }
                const std::int64_t runNs = monotonicNs() - started;

                ++slot->runs;
                if (!ok) ++slot->errors;
                if (runNs > slot->maxRunNs) slot->maxRunNs = runNs;
                slot->result = value;
                slot->resultOk = ok;
                slot->resultCycleId = slot->jobCycleId;
                slot->resultInputTimestamp = slot->jobInputTimestamp;
                slot->state.store(Slot::idle, std::memory_order_release);
                worked = true;
            }

            if (worked) idleSpins = 0;
            else if (++idleSpins > kSpinsBeforeYield) std::this_thread::yield();
            else cpuRelax();
        }
    }
}
