#include "Processing.h"

namespace vnc {
void LatestInputs::update(std::size_t ioc, const CountRates& rates)
{
    m_data.at(ioc) = rates;
}

double VncMp1Processor::process(const InputData& data) const
{
    double total = 0.0;
    for (const CountRates& rates : data)
        for (double rate : rates)
            total += rate;
    return total;
}
}
