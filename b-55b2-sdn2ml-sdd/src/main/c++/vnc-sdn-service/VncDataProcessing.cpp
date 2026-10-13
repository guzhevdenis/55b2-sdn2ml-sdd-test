#include "VncDataProcessing.h"
namespace vnc {
    void InputCountRates::update (std::size_t ioc, const IocCountRates& rates)
    {
        m_data.at(ioc) = rates;
    } 
    double MlProcessing::process(const InputData &data) const 
    {
        double totalCountRate = 0.0;
        for (const IocCountRates& rates : data)
            for (double rate: rates)
                totalCountRate += rate;
        return totalCountRate;
    }
}