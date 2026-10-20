#include <time.h>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

std::int64_t monotonicNs()
{
    timespec now{}; 
    if (::clock_gettime(CLOCK_MONOTONIC, &now)!= 0)
        throw std::runtime_error("clock gettime failed");
    return static_cast<std::int64_t>(now.tv_sec) * 1000000000LL + now.tv_nsec;

}