#include "Processing.h"
#include <iostream>
#include <stdexcept>

namespace {
void check(bool result, const char* message)
{
    if (!result) throw std::runtime_error(message);
}
}

int main()
{
    try {
        vnc::LatestInputs cache;
        vnc::VncMp1Processor processor;
        check(processor.process(cache.data()) == 0.0, "initial values must be zero");
        vnc::CountRates rates{};
        rates.fill(1.0);
        cache.update(0, rates);
        check(processor.process(cache.data()) == 8.0, "single IOC contributes eight values");
        for (std::size_t i = 1; i < vnc::kIocCount; ++i) {
            rates.fill(static_cast<double>(i + 1));
            cache.update(i, rates);
        }
        check(processor.process(cache.data()) == 168.0, "sum of six groups of eight values");
        rates.fill(10.0);
        cache.update(0, rates);
        check(processor.process(cache.data()) == 240.0, "replace one IOC and retain the other five");
        rates.fill(-1.0);
        cache.update(0, rates);
        check(processor.process(cache.data()) == 152.0, "sum values without validation");
        std::cout << "Processing tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
