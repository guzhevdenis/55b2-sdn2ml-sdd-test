/**
 * @file NdsDaqGenerator.h
 * @brief SIMULATOR NDS driver: fake DAQ count-rate source (no hardware).
 */

#ifndef NDS_DAQ_GENERATOR_H
#define NDS_DAQ_GENERATOR_H

#include <nds3/nds.h>
#include <array>
#include <atomic>
#include <cstdint>
#include <thread>

class NdsDaqGenerator
{
public:
    static const std::size_t kChannelCount = 8;

    NdsDaqGenerator(nds::Factory& factory,
                    const std::string& deviceName,
                    const nds::namedParameters_t& parameters);
    ~NdsDaqGenerator();

private:
    void startSimulation();
    void stopSimulation();
    void simulationThreadBody();

    void switchOn();
    void switchOff();
    void start();
    void stop();
    void recover();
    bool allowChange(const nds::state_t currentLocal,
                     const nds::state_t currentGlobal,
                     const nds::state_t nextLocal);

    void pushEnableRbv(const timespec& ts);

    nds::StateMachine m_stateMachine;
    std::array<nds::PVVariableIn<double>, kChannelCount> m_countRatePVs;
    nds::PVVariableOut<double> m_baseRatePV;
    nds::PVVariableIn<std::int64_t> m_topicTimestamp;
    nds::PVVariableOut<std::int32_t> m_enablePV;
    nds::PVVariableIn<std::int32_t> m_enableRbvPV;
    std::atomic<bool> m_stop;
    std::atomic<bool> m_threadRunning;
    std::thread m_thread;
    std::uint64_t m_tick;
    std::int32_t m_lastEnabled;
};

#endif
