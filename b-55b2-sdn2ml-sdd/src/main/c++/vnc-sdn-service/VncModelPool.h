#ifndef VNC_MODEL_POOL_H
#define VNC_MODEL_POOL_H

#include "VncDataProcessing.h"
#include <atomic>
#include <thread>
#include <vector>

namespace vnc
{

    struct ModelConfig
    {
        std::string topic;          // Output SDN topic
        std::string valueAttribute; // Topic attribute for the model output
        std::string modelFile;      // ONNX file in the models directory
        std::size_t worker;         // Worker thread that runs the model
    };

    typedef std::array<ModelConfig, kPlasmaParamCount> ModelConfigs;
    typedef std::array<OutputData, kPlasmaParamCount> Outputs;

    struct ModelStats
    {
        std::int64_t runs;
        std::int64_t errors;
        std::int64_t notOkCycles;
        std::int64_t maxRunNs;
    };

    // Runs the models on a fixed set of worker threads.
    // Each model has a slot: the main thread puts a job into an idle slot and the
    // worker returns the result through it. The slot is handed over by an atomic
    // state flag, so there are no locks and no allocations in the 1 ms loop.
    class ModelPool
    {
    public:
        // workerCpus[i] is the CPU for worker i, -1 - not pinned
        ModelPool(const ModelConfigs &configs, const std::string &modelDir,
                  const std::vector<int> &workerCpus);
        ~ModelPool();

        // Give the same input to every idle model. A model still busy with an older cycle is skipped.
        void dispatch(std::int64_t cycleId, std::int64_t inputTimestamp, const Features &features);

        // Fresh value of this cycle or the last value with stale/invalid quality.
        void collect(std::int64_t cycleId, Outputs &outputs);

        void stop();

        // Valid after stop()
        ModelStats stats(std::size_t model) const;

    private:
        struct Slot;
        void workerBody(std::size_t worker, int cpu);

        std::array<std::unique_ptr<Slot>, kPlasmaParamCount> m_slots;
        std::vector<std::thread> m_workers;
        std::atomic<bool> m_stop{false};
    };
}
#endif
