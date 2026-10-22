#include "VncSdnPubSub.h"
#include "VncDataProcessing.h"
#include "VncModelPool.h"
#include <poll.h>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <time.h>
#include <cstring>
#include "Timestamping.h"

volatile std::sig_atomic_t stopRequested = 0;
void stop(int) { stopRequested = 1; };
    //TO DO: Fix to TCN get time

    // Stops thread execution until deadline time
    bool waitUntil (std::int64_t deadline)
    {
        timespec next{};
        next.tv_sec = static_cast<time_t> (deadline / 1000000000LL);
        next.tv_nsec = static_cast<long> (deadline % 1000000000LL);
        while(!stopRequested) {
            const int error = ::clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
            if (error == 0) return !stopRequested;
            if (error != EINTR)
                throw std::runtime_error(std::string("clock nanosleep : ") + std::strerror(error));

        }
        return false;
    }


int main(int argc, char **argv)
{
    // TODO. Make it via configuration file
    std::string inputInterface{"ens192"};
    std::string outputInterface{"ens192"};
    // Directory for all resources
    const std::string resourceDir{
        "/home/codac-dev/55b2-sdn2ml-sdd/55b2-sdn2ml-sdd/b-55b2-sdn2ml-sdd/src/main/resources"};

    // One model per plasma parameter: output topic, value attribute, ONNX file, worker thread.
    // TODO. Only the MP12 model exists, the other parameters run its copies for now.
    const vnc::ModelConfigs models = {{
        {"vnc-mp12-topic", "fusionPower", "xgb_1_fp_v0_1.onnx", 0},
        {"vnc-mp13-topic", "value", "xgb_1_fp_v0_1.onnx", 0},
        {"vnc-mp14-topic", "value", "xgb_1_fp_v0_1.onnx", 0},
        {"vnc-mp15-topic", "value", "xgb_1_fp_v0_1.onnx", 1},
        {"vnc-mp20-topic", "value", "xgb_1_fp_v0_1.onnx", 1},
        {"vnc-mp64-topic", "value", "xgb_1_fp_v0_1.onnx", 1},
    }};
    // CPU for each worker thread, -1 - not pinned. TODO. Choose after the inference benchmark
    const std::vector<int> workerCpus = {-1, -1};

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    // SDN Subscribers configuration
    std::array<std::unique_ptr<vnc::VncInputSubscriber>, vnc::kIocCount> subscribers;
    std::array<pollfd, vnc::kIocCount> descriptors{};

    for (std::size_t i = 0; i < vnc::kIocCount; ++i)
    {
        subscribers[i].reset(new vnc::VncInputSubscriber(
            "pcf" + std::to_string(i) + "-nds-sdn-topic",
            resourceDir + "/topics/",
            inputInterface));

        descriptors[i].fd = subscribers[i]->socket();
        descriptors[i].events = POLLIN;

        std::cout << "socket for pcf" << std::to_string(i) << "-nds-sdn-topic -- " << descriptors[i].fd << std::endl;
    }

    // SDN Publishers configuration. One topic per plasma parameter
    std::array<std::unique_ptr<vnc::VncOutputPublisher>, vnc::kPlasmaParamCount> publishers;
    for (std::size_t i = 0; i < vnc::kPlasmaParamCount; ++i)
        publishers[i].reset(new vnc::VncOutputPublisher(models[i].topic,
                                                        resourceDir + "/topics/",
                                                        outputInterface,
                                                        models[i].valueAttribute));

    // Receiving the data. Pack it in InputData
    vnc::InputCountRates InputData; // Data from all IOCs
    vnc::Features features{};
    vnc::ModelPool pool(models, resourceDir + "/models", workerCpus);
    vnc::Outputs outputs{};
    std::int64_t cycles = 0, cycleId = 0, skippedPeriods = 0, publishErrors = 0;
    std::int64_t maxWakeLatenessNs = 0, maxPublishLatenessNs = 0, maxWorksNs = 0;
    std::int64_t next = monotonicNs() + vnc::kPeriodsNs;

    while (!stopRequested)
    {
        if (!waitUntil(next)) break;
        const std::int64_t started = monotonicNs();
        if (started - next > maxWakeLatenessNs)
            maxWakeLatenessNs = started - next;
        ++cycleId;

        for (std::size_t i = 0; i < vnc::kIocCount && !stopRequested; ++i){
            for (std::size_t n = 0; n < vnc::kMaxPacketsPerIoc && !stopRequested; ++n) {
                vnc::IocCountRates rates {};
                if (!subscribers[i]->subscribe(rates)) break;
                InputData.update(i, rates);
            }

        }
        if (stopRequested) break;

        // The same input goes to all idle models
        if (InputData.takeUpdated()) {
            vnc::buildFeatures(InputData.data(), features);
            pool.dispatch(cycleId, started, features);
        }
        const std::int64_t dispatched = monotonicNs();

        // Publish at the fixed moment of the cycle. Late models publish their last value as stale
        const std::int64_t publishAt = next + vnc::kPublishOffsetNs;
        if (!waitUntil(publishAt)) break;
        const std::int64_t published = monotonicNs();
        if (published - publishAt > maxPublishLatenessNs)
            maxPublishLatenessNs = published - publishAt;

        pool.collect(cycleId, outputs);
        for (std::size_t i = 0; i < vnc::kPlasmaParamCount; ++i) {
            outputs[i].timestamp = published;
            if (!publishers[i]->publish(outputs[i])) ++publishErrors;
        }
        ++cycles;

        const std::int64_t finished = monotonicNs();
        // Work of the main thread without waiting for the publish moment
        const std::int64_t worksNs = (dispatched - started) + (finished - published);
        if (worksNs > maxWorksNs) maxWorksNs = worksNs;

        next += vnc::kPeriodsNs;

        if (next <= finished) {
            const std::int64_t skipped = (finished - next) / vnc::kPeriodsNs + 1;
            skippedPeriods += skipped;
            // cycleId counts periods, so consumers see the gap
            cycleId += skipped;
            next += skipped * vnc::kPeriodsNs;
        }

    }
    pool.stop();

    //Review of the errors
    std::cout << std::endl;
    std::cout << " cycles = " << cycles << " skipped periods = " << skippedPeriods
              << " publish errors = " << publishErrors
              << " max wake lateness ns = " << maxWakeLatenessNs
              << " max publish lateness ns = " << maxPublishLatenessNs
              << " max work ns = " << maxWorksNs << std::endl;
    for (std::size_t i = 0; i < vnc::kPlasmaParamCount; ++i) {
        const vnc::ModelStats s = pool.stats(i);
        std::cout << " " << models[i].topic << ": runs = " << s.runs
                  << " inference errors = " << s.errors
                  << " not ok cycles = " << s.notOkCycles
                  << " max run ns = " << s.maxRunNs << std::endl;
    }
    return 0;

    }
