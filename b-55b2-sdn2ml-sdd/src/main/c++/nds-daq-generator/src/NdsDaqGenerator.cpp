/**
 * @file NdsDaqGenerator.cpp
 * @brief SIMULATOR: periodically publishes 8 count-rate PVs. No real hardware.
 *
 * Update mechanism matches DANDataGenerator: a worker thread with 100 ms sleep
 * between pushes (not a static stub).
 */

#include "../include/NdsDaqGenerator.h"

#include <cmath>
#include <ctime>
#include <functional>
#include <unistd.h>

namespace {
timespec wallClockTimestamp()
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}

const char* kCountRateNames[NdsDaqGenerator::kChannelCount] = {
    "CountRate00", "CountRate01", "CountRate02", "CountRate03",
    "CountRate04", "CountRate05", "CountRate06", "CountRate07"
};
}

NdsDaqGenerator::NdsDaqGenerator(nds::Factory& factory,
                                 const std::string& deviceName,
                                 const nds::namedParameters_t& parameters)
    : m_stop(false)
    , m_threadRunning(false)
    , m_tick(0)
    , m_lastEnabled(0)
{
    (void)parameters;

    nds::Port rootNode(deviceName);

    m_stateMachine = rootNode.addChild(nds::StateMachine(
        false,
        std::bind(&NdsDaqGenerator::switchOn, this),
        std::bind(&NdsDaqGenerator::switchOff, this),
        std::bind(&NdsDaqGenerator::start, this),
        std::bind(&NdsDaqGenerator::stop, this),
        std::bind(&NdsDaqGenerator::recover, this),
        std::bind(&NdsDaqGenerator::allowChange, this,
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    for (std::size_t i = 0; i < kChannelCount; ++i) {
        m_countRatePVs[i] = rootNode.addChild(nds::PVVariableIn<double>(kCountRateNames[i]));
        m_countRatePVs[i].setScanType(nds::scanType_t::interrupt);
    }
    m_baseRatePV = rootNode.addChild(nds::PVVariableOut<double>("BaseRate"));
    m_enablePV = rootNode.addChild(nds::PVVariableOut<std::int32_t>("Enable"));
    m_enableRbvPV = rootNode.addChild(nds::PVVariableIn<std::int32_t>("EnableRBV"));
    m_enableRbvPV.setScanType(nds::scanType_t::interrupt);

    rootNode.initialize(this, factory);
    m_baseRatePV.write(wallClockTimestamp(), 1000.0);
    m_enablePV.write(wallClockTimestamp(), 0);
    m_enableRbvPV.push(wallClockTimestamp(), 0);
    startSimulation();
}

NdsDaqGenerator::~NdsDaqGenerator()
{
    stopSimulation();
}

void NdsDaqGenerator::startSimulation()
{
    m_stop = false;
    m_threadRunning = true;
    m_thread = std::thread(&NdsDaqGenerator::simulationThreadBody, this);
}

void NdsDaqGenerator::stopSimulation()
{
    m_stop = true;
    if (m_threadRunning && m_thread.joinable()) {
        m_thread.join();
        m_threadRunning = false;
    }
}

void NdsDaqGenerator::pushEnableRbv(const timespec& ts)
{
    const std::int32_t enabled =
        (m_stateMachine.getLocalState() == nds::state_t::running) ? 1 : 0;
    m_enablePV.write(ts, enabled);
    if (enabled != m_lastEnabled) {
        m_enableRbvPV.push(ts, enabled);
        m_lastEnabled = enabled;
    }
}

void NdsDaqGenerator::switchOn()
{
}

void NdsDaqGenerator::switchOff()
{
    pushEnableRbv(wallClockTimestamp());
}

void NdsDaqGenerator::start()
{
    pushEnableRbv(wallClockTimestamp());
}

void NdsDaqGenerator::stop()
{
    pushEnableRbv(wallClockTimestamp());
}

void NdsDaqGenerator::recover()
{
    throw nds::StateMachineRollBack("Cannot recover");
}

bool NdsDaqGenerator::allowChange(const nds::state_t,
                                  const nds::state_t,
                                  const nds::state_t)
{
    return true;
}

void NdsDaqGenerator::simulationThreadBody()
{
    while (!m_stop) {
        const timespec ts = wallClockTimestamp();
        pushEnableRbv(ts);
        const std::int32_t enabled = m_enablePV.getValue();
        if (enabled != 0) {
            const double baseRate = m_baseRatePV.getValue();
            const double noisePhase = static_cast<double>(m_tick) * 0.1;
            for (std::size_t i = 0; i < kChannelCount; ++i) {
                const double channelOffset = static_cast<double>(i) * 250.0;
                const double noise = 50.0 * std::sin(noisePhase + static_cast<double>(i));
                const double value = baseRate + channelOffset + noise + static_cast<double>(m_tick % 17);
                m_countRatePVs[i].push(ts, value);
            }
            ++m_tick;
        }
        ::usleep(100000);
    }
}

NDS_DEFINE_DRIVER(NdsDaqGenerator, NdsDaqGenerator)
