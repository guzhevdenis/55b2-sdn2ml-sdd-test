#include "VncSdnPubSub.h"
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <time.h>

volatile std::sig_atomic_t stopRequested = 0;
void stop(int) { stopRequested = 1; };

namespace {
const std::int64_t kPeriodNs = 1000000; // 1 ms / 1 kHz target
const unsigned kMaxPacketsPerIoc = 4;

std::int64_t monotonicNs()
{
    timespec now{};
    if (::clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        throw std::runtime_error("clock_gettime failed");
    return static_cast<std::int64_t>(now.tv_sec) * 1000000000LL + now.tv_nsec;
}

bool waitUntil(std::int64_t deadline)
{
    timespec next{};
    next.tv_sec = static_cast<time_t>(deadline / 1000000000LL);
    next.tv_nsec = static_cast<long>(deadline % 1000000000LL);
    while (!stopRequested) {
        const int error = ::clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
        if (error == 0) return !stopRequested;
        if (error != EINTR)
            throw std::runtime_error(std::string("clock_nanosleep: ") + std::strerror(error));
        // An unrelated signal must not shift the absolute deadline.
    }
    return false;
}
}

int main() try
{
    // TODO. Make it via configuration file
    std::string inputInterface{"ens192"};
    std::string outputInterface{"ens192"};

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    // SDN Subscribers configuration
    std::array<std::unique_ptr<vnc::VncInputSubscriber>, vnc::kIocCount> subscribers;

    for (std::size_t i = 0; i < vnc::kIocCount; ++i)
    {
        subscribers[i].reset(new vnc::VncInputSubscriber(
            "pcf" + std::to_string(i) + "-nds-sdn-topic",
            "/home/codac-dev/55b2-sdn2ml-sdd/55b2-sdn2ml-sdd/b-55b2-sdn2ml-sdd/src/main/resources",
            inputInterface));

        std::cout << "socket for pcf" << i << "-nds-sdn-topic -- " << subscribers[i]->socket() << std::endl;
    }
    
    // SDN Publishers configuration
    std::array<std::unique_ptr<vnc::VncOutputPublisher>, vnc::kPlasmaParamCount> publishers;
    for (std::size_t i = 0; i < vnc::kPlasmaParamCount; ++i)
    {
        publishers[i].reset(new vnc::VncOutputPublisher(
            "vnc-mp" + std::to_string(i) + "-topic",
            "/home/codac-dev/55b2-sdn2ml-sdd/55b2-sdn2ml-sdd/b-55b2-sdn2ml-sdd/src/main/resources",
            outputInterface));
    }

    // All application objects are prepared before entering the periodic loop.
    vnc::InputCountRates InputData;
    vnc::MlProcessing processor;
    vnc::OutputData outputData{};
    std::uint64_t cycles = 0, skippedPeriods = 0, publishErrors = 0;
    std::int64_t maxWakeLatenessNs = 0, maxWorkNs = 0;
    std::int64_t next = monotonicNs() + kPeriodNs;
    while (!stopRequested)
    {
        if (!waitUntil(next)) break;
        const std::int64_t started = monotonicNs();
        if (started - next > maxWakeLatenessNs)
            maxWakeLatenessNs = started - next;

        for (std::size_t i = 0; i < vnc::kIocCount && !stopRequested; ++i) {
            // Receive(0) does not wait. Bound draining so a busy IOC cannot
            // keep us here indefinitely. Retain cached values when no data arrives.
            for (unsigned n = 0; n < kMaxPacketsPerIoc && !stopRequested; ++n) {
                vnc::IocCountRates rates{};
                if (!subscribers[i]->subscribe(rates)) break;
                InputData.update(i, rates);
            }
        }
        if (stopRequested) break;

        outputData.outputValue = processor.process(InputData.data());
        for (std::size_t i = 0; i < vnc::kPlasmaParamCount; ++i)
            if (!publishers[i]->publish(outputData)) ++publishErrors;
        ++cycles;

        const std::int64_t finished = monotonicNs();
        if (finished - started > maxWorkNs) maxWorkNs = finished - started;
        next += kPeriodNs;
        if (next <= finished) {
            // Skip elapsed slots in constant time; never publish a catch-up burst.
            const std::int64_t skipped = (finished - next) / kPeriodNs + 1;
            skippedPeriods += static_cast<std::uint64_t>(skipped);
            next += skipped * kPeriodNs;
        }
    }
    // No console/file logging in the time-sensitive loop.
    std::cout << "cycles=" << cycles << " skipped_periods=" << skippedPeriods
              << " publish_errors=" << publishErrors
              << " max_wake_lateness_ns=" << maxWakeLatenessNs
              << " max_work_ns=" << maxWorkNs << std::endl;
    return publishErrors == 0 ? 0 : 1;
}
catch (const std::exception& error)
{
    std::cerr << "vnc-sdn-service: " << error.what() << std::endl;
    return 1;
}
