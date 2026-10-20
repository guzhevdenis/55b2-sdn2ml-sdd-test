#include "VncSdnPubSub.h"
#include "VncDataProcessing.h"
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
    std::ofstream logFile{"/home/codac-dev/55b2-sdn2ml-sdd/55b2-sdn2ml-sdd/b-55b2-sdn2ml-sdd/src/main/c++/vnc-sdn-service/logs/vnc_calc_log.txt"};
    // TODO. Make it via configuration file
    std::string inputInterface{"ens192"};
    std::string outputInterface{"ens192"};
    // Directory for all resources
    const std::string resourceDir{
        "/home/codac-dev/55b2-sdn2ml-sdd/55b2-sdn2ml-sdd/b-55b2-sdn2ml-sdd/src/main/resources"};

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
    
    // SDN Publishers configuration. For future use for all publishers
    std::array<std::unique_ptr<vnc::VncOutputPublisher>, vnc::kPlasmaParamCount> publishers;
    // The first publisher for MP12 Fusion Power.
    publishers[0].reset(new vnc::VncOutputPublisher("vnc-mp12-topic",
                                                    resourceDir + "/topics/",
                                                    outputInterface));
    // Receiving the data. Pack it in InputData
    vnc::InputCountRates InputData; // Data from all IOCs
    vnc::MlProcessing processor(resourceDir + "/models/xgb_1_fp_v0_1.onnx"); 
    vnc::OutputData outputData {}; 
    std::int64_t cycles = 0, skippedPeriods = 0, publishErrors = 0, inferenceErrors = 0;
    std::int64_t maxWakeLatenessNs = 0, maxWorksNs = 0; 
    std::int64_t next = monotonicNs() + vnc::kPeriodsNs; 

    while (!stopRequested)
    {
        if (!waitUntil(next)) break; 
        const std::int64_t started = monotonicNs();
        if (started - next > maxWakeLatenessNs)
            maxWakeLatenessNs = started - next; 
        for (std::size_t i = 0; i < vnc::kIocCount && !stopRequested; ++i){
            for (std::size_t n = 0; n < vnc::kMaxPacketsPerIoc && !stopRequested; ++n) {
                vnc::IocCountRates rates {}; 
                if (!subscribers[i]->subscribe(rates)) break; 
                InputData.update(i, rates);
            }
           
        }
        if (stopRequested) break; 

        const vnc::InputData& d = InputData.data();
        logFile << "input data " << d[0][0] << std::endl;
        if(InputData.takeUpdated() && !processor.process(InputData.data(), outputData))
            ++inferenceErrors;
        for (std::size_t i = 0; i < vnc::kPlasmaParamCount; ++i)
            // If we are not in time it publishes cached data 
            if (!publishers[i]->publish(outputData)) ++publishErrors;
        ++cycles;

        const std::int64_t finished = monotonicNs();
        if (finished - started > maxWorksNs) maxWorksNs = finished - started;

        next += vnc::kPeriodsNs; 

        if (next <= finished) {
            const std::int64_t skipped = (finished - next) / vnc::kPeriodsNs + 1;
            skippedPeriods += static_cast<std::uint64_t>(skipped);
            next += skipped * vnc::kPeriodsNs;
        }
    
    }

    //Review of the errors
    std::cout << std::endl;
    std::cout << " cycles = " << cycles << " skipped periods = " << skippedPeriods
              << " publish errors = " << publishErrors
              << " max wake lateness ns = " << maxWakeLatenessNs
              << " max work ns = " << maxWorksNs 
              << " inference error = " << inferenceErrors << std::endl;
    return 0;

    }
