#include "SdnTransport.h"
#include <sdn-core/sdn-topic-xml.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace {
void loadTopic(sdn::Topic& topic, const std::string& name, const std::string& directory)
{
    const std::string path = directory + "/" + name + ".xml";
    if (sdn::TopicHelper::LoadDefinitionFile(&topic, path.c_str()) != STATUS_SUCCESS)
        throw std::runtime_error("Cannot load topic: " + path);
    if (name != topic.GetName()) throw std::runtime_error("Topic name mismatch: " + path);
}

std::uint32_t checkField(sdn::Topic& topic, const char* name,
                         unsigned int type, std::size_t size)
{
    sdn::base::AnyType* definition = topic.GetTypeDefinition();
    if (!definition || !definition->HasAttribute(name) ||
        static_cast<unsigned int>(definition->GetAttributeType(name)) != type ||
        definition->GetAttributeSize(name) != size)
        throw std::runtime_error(std::string("Invalid topic field: ") + name);
    const std::uint32_t offset = definition->GetAttributeOffset(name);
    if (offset > topic.GetSize() || size > topic.GetSize() - offset)
        throw std::runtime_error(std::string("Invalid topic offset: ") + name);
    return offset;
}

void describe(sdn::Topic& topic)
{
    std::cerr << topic.GetName() << " " << topic.GetMCastGroup() << ":"
              << topic.GetMCastPort() << " payload=" << topic.GetSize() << " bytes\n";
}
}

namespace vnc {
SdnInputReceiver::SdnInputReceiver(const std::string& name, const std::string& directory,
                                   const std::string& interface)
{
    static_assert(sizeof(double) == 8, "SDN count rates require 64-bit double");
    loadTopic(m_topic, name, directory);
    if (m_topic.GetSize() != kChannelCount * sizeof(double))
        throw std::runtime_error("Expected eight scalar doubles: " + name);
    for (std::size_t i = 0; i < kChannelCount; ++i) {
        const std::string field = "CountRate0" + std::to_string(i);
        m_offsets[i] = checkField(m_topic, field.c_str(), ANYTYPE_ATTR_DOUBLE, sizeof(double));
    }
    m_subscriber.reset(new sdn::Subscriber(m_topic));
    if (m_subscriber->SetInterface(interface.c_str()) != STATUS_SUCCESS ||
        m_subscriber->Configure() != STATUS_SUCCESS || m_subscriber->GetSocket() < 0)
        throw std::runtime_error("Cannot configure subscriber: " + name);
    describe(m_topic);
}

int SdnInputReceiver::socket() { return m_subscriber->GetSocket(); }

bool SdnInputReceiver::receive(CountRates& rates)
{
    // STATUS_ERROR also covers empty queues and invalid packets in this API.
    if (m_subscriber->Receive(static_cast<std::uint64_t>(0)) != STATUS_SUCCESS) return false;
    const unsigned char* payload = static_cast<const unsigned char*>(m_subscriber->GetTopicInstance());
    if (!payload) throw std::runtime_error("Subscriber has no payload");
    for (std::size_t i = 0; i < kChannelCount; ++i) {
        unsigned char bytes[sizeof(double)];
        std::memcpy(bytes, payload + m_offsets[i], sizeof(bytes));
        if (!m_subscriber->IsPayloadOrdered()) std::reverse(bytes, bytes + sizeof(bytes));
        std::memcpy(&rates[i], bytes, sizeof(bytes));
    }
    return true;
}

SdnOutputPublisher::SdnOutputPublisher(const std::string& directory, const std::string& interface)
{
    loadTopic(m_topic, "vnc-mp1-topic", directory);
    checkField(m_topic, "timestamp", ANYTYPE_ATTR_UINT64, sizeof(std::uint64_t));
    checkField(m_topic, "sequenceNumber", ANYTYPE_ATTR_UINT64, sizeof(std::uint64_t));
    checkField(m_topic, "totalCountRate", ANYTYPE_ATTR_DOUBLE, sizeof(double));
    if (m_topic.GetSize() != 24u) throw std::runtime_error("Unexpected MP1 payload size");
    m_publisher.reset(new sdn::Publisher(m_topic));
    if (m_publisher->SetInterface(interface.c_str()) != STATUS_SUCCESS ||
        m_publisher->Configure() != STATUS_SUCCESS)
        throw std::runtime_error("Cannot configure MP1 publisher");
    describe(m_topic);
}

void SdnOutputPublisher::publish(const OutputData& output)
{
    // Named attributes avoid C++ struct padding and XML rank assumptions.
    if (!m_topic.SetAttribute("timestamp", &output.timestamp) ||
        !m_topic.SetAttribute("sequenceNumber", &output.sequenceNumber) ||
        !m_topic.SetAttribute("totalCountRate", &output.totalCountRate))
        throw std::runtime_error("Cannot fill MP1 payload");
    if (m_publisher->Publish() != STATUS_SUCCESS)
        throw std::runtime_error("Cannot publish MP1 payload");
}
}
