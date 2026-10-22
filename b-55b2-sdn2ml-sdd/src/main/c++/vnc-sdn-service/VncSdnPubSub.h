#ifndef VNC_SDN_PUBSUB_H
#define VNC_SDN_PUBSUB_H

#include "VncDataProcessing.h"
#include <sdn-core/sdn-api.h>
#include <memory>
#include <string>

namespace vnc
{

    class VncInputSubscriber
    {
    public:
        VncInputSubscriber(const std::string &name, const std::string &directory,
                           const std::string &interface);
        bool subscribe(IocCountRates &rates);

        int socket() { return m_subscriber->GetSocket(); };

    private:
        std::unique_ptr<sdn::Subscriber> m_subscriber;
        sdn::Topic m_topic;
        // To parse payload effectively
        std::array<std::uint32_t, kChannelCount> m_offsets;
    };

    class VncOutputPublisher
    {
    public:
        VncOutputPublisher(const std::string &name, const std::string &directory,
                           const std::string &interface, const std::string &valueAttribute);
        bool publish(const OutputData &data);

    private:
        sdn::Topic m_topic;
        std::unique_ptr<sdn::Publisher> m_publisher;
        std::string m_valueAttribute;
    };
}
#endif
