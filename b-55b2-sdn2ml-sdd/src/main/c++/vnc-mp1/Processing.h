#ifndef VNC_MP1_PROCESSING_H
#define VNC_MP1_PROCESSING_H

#include <array>
#include <cstdint>

namespace vnc {
const std::size_t kIocCount = 6;
const std::size_t kChannelCount = 8;
typedef std::array<double, kChannelCount> CountRates;
typedef std::array<CountRates, kIocCount> InputData;

// This is an application model, not a wire-layout struct.
struct OutputData {
    std::uint64_t timestamp;
    std::uint64_t sequenceNumber;
    double totalCountRate;
};

class LatestInputs {
public:
    void update(std::size_t ioc, const CountRates& rates);
    const InputData& data() const { return m_data; }
private:
    InputData m_data{};
};

class VncMp1Processor {
public:
    double process(const InputData& data) const;
};
}
#endif
