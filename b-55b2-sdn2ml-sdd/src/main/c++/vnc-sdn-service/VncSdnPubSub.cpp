#include "VncSdnPubSub.h"
#include <sdn-core/sdn-topic-xml.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace vnc
{
    // Load the topic data
    void parseXmlTopic(sdn::Topic &topic, const std::string &name, const std::string &directory)
    {
        const std::string path = directory + "/" + name + ".xml";
        if (sdn::TopicHelper::LoadDefinitionFile(&topic, path.c_str()) != STATUS_SUCCESS)
            throw std::runtime_error("Error load topic: " + path);
        if (name != topic.GetName())
            throw std::runtime_error("Topic name mismatch" + path);
    }

    // Get the offset for Channel Count Rate
    std::uint32_t checkField(sdn::Topic& topic, const char* name, 
                            unsigned int type, std::size_t size)
   {
        sdn::base::AnyType* definition = topic.GetTypeDefinition();
        const std::uint32_t offset = definition->GetAttributeOffset(name);
        return offset;
   }

    VncInputSubscriber::VncInputSubscriber(const std::string &name, const std::string &directory,
                                           const std::string &interface)
    {
        // Get topic description from XML
        parseXmlTopic(m_topic, name, directory);
        // TODO. Add topic validation

        // Calculate offsets for channel  count rates
        for (std::size_t i = 0; i < kChannelCount; ++i){
            const std::string field = "CountRate0" + std::to_string(i);
            m_offsets[i] = checkField(m_topic, field.c_str(), ANYTYPE_ATTR_DOUBLE, sizeof(double));
        }

        // Better make_unique, but we in C++11 standart
        m_subscriber.reset(new sdn::Subscriber(m_topic));
        m_subscriber->SetInterface(interface.c_str());

        // Check configuration
        m_subscriber->Configure();

        std::cout << m_topic.GetName() << " " << "topic is subscribed" << std::endl;

        // TODO. Add exception throwing
    }

    bool VncInputSubscriber::subscribe (IocCountRates& rates)
    {
        //Get the data from socket to the rates
        // STATUS_ERROR also covers empty queues and invalid packets in this API.
        if (m_subscriber->Receive(static_cast<std::uint64_t>(0)) != STATUS_SUCCESS) return false;
        const unsigned char* payload = static_cast<const unsigned char*>(m_subscriber->GetTopicInstance());
        if (!payload) return false;
        for (std::size_t i = 0; i < kChannelCount; ++i) {
            unsigned char bytes[sizeof(double)];
            //Get data from each channel
            std::memcpy(bytes, payload + m_offsets[i], sizeof(bytes));
            //if (!m_subscriber->IsPayloadOrdered()) std::reverse(bytes, bytes + sizeof(bytes));
            // Pack data to rates according to each channel
            std::memcpy(&rates[i], bytes, sizeof(bytes));
        }
        return true;
    }

    VncOutputPublisher::VncOutputPublisher(const std::string &name, const std::string &directory,
                                           const std::string &interface)
    {
        parseXmlTopic(m_topic, name, directory);

        m_publisher.reset(new sdn::Publisher(m_topic));


        if(m_publisher->SetInterface(interface.c_str()) != STATUS_SUCCESS || 
            m_publisher->Configure() != STATUS_SUCCESS)
            throw std::runtime_error ("cannot configure publisher");

        m_publisher->Configure();
        std::cout << m_topic.GetName() << " " << "topic is ready to be published" << std::endl;

        // TODO. Add exception throwing
    }

    bool VncOutputPublisher::publish(const OutputData& output)
    {
        // Logic for one topic only
        if (!m_topic.SetAttribute("totalCountRate", &output.outputValue)) return false;;
        return m_publisher->Publish() == STATUS_SUCCESS;
    }
}