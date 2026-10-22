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
    const std::size_t kPlasmaParamCount = 6; // One ML model and one output topic per plasma parameter
    const std::int64_t kPeriodsNs = 1000000; // 1 ms (VNC requirements)
    const std::int64_t kPublishOffsetNs = 800000; // Fixed publish moment inside the cycle
    const std::int64_t kMaxStaleCycles = 10; // Stale value older than this is published as invalid
    const std::size_t kMaxPacketsPerIoc = 4; // for MVP purposes
    const std::size_t kModelInputSize = 48; // 24 input Count Rates and 24 flags for detectors

    typedef std::array<double, kChannelCount> IocCountRates;
    typedef std::array<IocCountRates, kIocCount> InputData;
    // The same model input is shared by all models
    typedef std::array<float, kModelInputSize> Features;

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

    enum class Quality : std::int64_t
    {
        ok = 0,      // Calculated in this cycle
        stale = 1,   // Last value: the model was late or there were no new inputs
        invalid = 2  // No value yet or stale for more than kMaxStaleCycles
    };

    struct OutputData
    {
        double value = 0.0;
        std::int64_t timestamp = 0;      // Publish time
        std::int64_t inputTimestamp = 0; // Cycle start of the inputs the value was calculated from
        std::int64_t cycleId = 0;        // The same in all output topics of one cycle
        Quality quality = Quality::invalid;
    };

    // Convert count rates into the model input. Done once per cycle for all models.
    void buildFeatures(const InputData &data, Features &features);

    // This is the ML model implementation algorithms.
    class MlModel
    {
    public:
        explicit MlModel(const std::string &modelPath);
        ~MlModel();
        bool run(const Features &features, double &output);
        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;
    };

}

#endif
