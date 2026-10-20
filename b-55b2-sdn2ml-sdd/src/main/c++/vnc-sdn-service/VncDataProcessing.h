#ifndef VNC_DATA_PROCESSING_H
#define VNC_DATA_PROCESSING_H

#include <array>
#include <cstdint>
#include <memory>
#include <string>
namespace vnc
{

    const std::size_t kIocCount = 6;
    const std::size_t kChannelCount = 8;
    const std::size_t kPlasmaParamCount = 1;
    const std::int64_t kPeriodsNs = 1000000; // 1 ms (VNC requirements)
    const std::size_t kMaxPacketsPerIoc = 4; // for MVP purposes 

    typedef std::array<double, kChannelCount> IocCountRates;
    typedef std::array<IocCountRates, kIocCount> InputData;

    class InputCountRates
    {
    public:
        // Update input data per IOC
        void update(std::size_t ioc, const IocCountRates &rates);
        const InputData &data() const { return m_data; };

        bool takeUpdated()
        {
            const bool updated = m_updated;
            m_updated = false; 
            return updated;
        }

        // TO DO. Check the quality of data
        bool validate() const {};

    private:
        InputData m_data{};
        bool m_updated = false;
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
        explicit MlProcessing(const std::string &powerModelPath);
        ~MlProcessing();
        bool process(const InputData &data, OutputData& output);
        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;
    };

}

#endif