#include "SdnTransport.h"
#include <poll.h>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
typedef std::chrono::steady_clock Clock;
volatile std::sig_atomic_t stopRequested = 0;
void stop(int) { stopRequested = 1; }

int positiveMilliseconds(const char* value)
{
    const std::string text(value);
    std::size_t end = 0;
    const long number = std::stol(text, &end);
    if (end != text.size() || number <= 0 || number > 60000)
        throw std::runtime_error("Timing must be an integer in 1..60000 ms");
    return static_cast<int>(number);
}
}

int main(int argc, char** argv)
{
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: vnc-mp1 INTERFACE|- TOPIC_DIRECTORY [period_ms=100]\n"
                  << "Use '-' for SDN_INTERFACE_NAME. SIGINT/SIGTERM stop the process.\n";
        return 2;
    }
    try {
        std::string interface(argv[1]);
        if (interface == "-") {
            const char* value = std::getenv("SDN_INTERFACE_NAME");
            interface = value ? value : "";
        }
        if (interface.empty()) throw std::runtime_error("SDN interface is required");
        const std::chrono::milliseconds period(argc > 3 ? positiveMilliseconds(argv[3]) : 100);
        std::signal(SIGINT, stop);
        std::signal(SIGTERM, stop);

        std::array<std::unique_ptr<vnc::SdnInputReceiver>, vnc::kIocCount> receivers;
        std::array<pollfd, vnc::kIocCount> descriptors{};
        for (std::size_t i = 0; i < vnc::kIocCount; ++i) {
            receivers[i].reset(new vnc::SdnInputReceiver(
                "pcf" + std::to_string(i) + "-nds-sdn-topic", argv[2], interface));
            descriptors[i].fd = receivers[i]->socket();
            descriptors[i].events = POLLIN;
        }
        vnc::SdnOutputPublisher publisher(argv[2], interface);
        vnc::LatestInputs cache;
        vnc::VncMp1Processor processor;
        std::uint64_t sequence = 0;
        Clock::time_point deadline = Clock::now();
        while (!stopRequested) {
            const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - Clock::now()).count();
            // At most 50 ms idle wait, including during shutdown.
            const int timeout = remaining <= 0 ? 0 : static_cast<int>(remaining > 50 ? 50 : remaining);
            const int status = ::poll(descriptors.data(), descriptors.size(), timeout);
            if (status < 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error("SDN socket poll failed");
            }
            for (std::size_t i = 0; i < vnc::kIocCount && !stopRequested; ++i) {
                if (descriptors[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                    throw std::runtime_error("SDN input socket failed: PCF" + std::to_string(i));
                if (!(descriptors[i].revents & POLLIN)) continue;
                // Bounded draining: one busy IOC cannot starve the other five or publication.
                for (unsigned int n = 0; n < 32 && !stopRequested; ++n) {
                    vnc::CountRates rates{};
                    if (!receivers[i]->receive(rates)) break;
                    cache.update(i, rates);
                }
            }
            const auto now = Clock::now();
            if (!stopRequested && now >= deadline) {
                vnc::OutputData output{};
                output.totalCountRate = processor.process(cache.data());
                // Unix/POSIX wall-clock ns; not NDS epoch and not a measurement timestamp.
                output.timestamp = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
                output.sequenceNumber = sequence++;
                publisher.publish(output);
                deadline = now + period; // No catch-up burst after a scheduling delay.
            }
        }
        std::cerr << "VNC MP1 stopped\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "VNC MP1: " << error.what() << '\n';
        return 1;
    }
}
