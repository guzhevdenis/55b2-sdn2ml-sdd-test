#ifndef VNC_DATA_PROCESSING_H
#define VNC_DATA_PROCESSING_H

#include <array>
#include <cstdint>

namespace vnc
{

    const std::size_t kIocCount = 6;
    const std::size_t kChannelCount = 8;
    const std::size_t kPlasmaParamCount = 1;

    typedef std::array<double, kChannelCount> IocCountRates;
    typedef std::array<IocCountRates, kIocCount> InputData;

    class InputCountRates
    {
    public:
        // Update input data per IOC
        void update(std::size_t ioc, const IocCountRates &rates);
        const InputData &data() const { return m_data; };

        // TO DO. Check the quality of data
        bool validate() const {};

    private:
        InputData m_data{};
    };

    class OutputData
    {
    public:
        std::uint64_t timestamp;
        double outputValue;

        // TO DO. Calculate quality data tag.
        const std::uint64_t CalculateQuality() { return qualityTag; };
    private:
        std::uint64_t qualityTag;
    };

    // This is the ML model implementation algorithms.
    class MlProcessing
    {
    public:
        double process(const InputData &data) const;
    };

}

#endif