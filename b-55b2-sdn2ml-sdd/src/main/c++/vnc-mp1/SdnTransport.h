#ifndef VNC_MP1_SDN_TRANSPORT_H
#define VNC_MP1_SDN_TRANSPORT_H

#include "Processing.h"
#include <sdn-core/sdn-api.h>
#include <memory>
#include <string>

namespace vnc {
class SdnInputReceiver {
public:
    SdnInputReceiver(const std::string& name, const std::string& directory,
                     const std::string& interface);
    int socket();
    bool receive(CountRates& rates);
private:
    // Reverse destruction order keeps the topic alive until the participant is gone.
    sdn::Topic m_topic;
    std::array<std::uint32_t, kChannelCount> m_offsets;
    std::unique_ptr<sdn::Subscriber> m_subscriber;
};

class SdnOutputPublisher {
public:
    SdnOutputPublisher(const std::string& directory, const std::string& interface);
    void publish(const OutputData& output);
private:
    sdn::Topic m_topic;
    std::unique_ptr<sdn::Publisher> m_publisher;
};
}
#endif
