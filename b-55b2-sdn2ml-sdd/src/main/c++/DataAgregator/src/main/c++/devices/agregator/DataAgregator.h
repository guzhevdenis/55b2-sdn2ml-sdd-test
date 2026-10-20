#include <onnxruntime_cxx_api.h>
#include <nds3/nds.h>
#include <array>
#include <cstddef>
#include <mutex>
#include <vector>
#ifndef DATAAGREGATOR_H
#define DATAAGREGATOR_H
class DataAgregator
{
    public:
    DataAgregator(nds::Factory &factory,
                 const std::string &deviceName,
                const nds::namedParameters_t &parameters);


    void getTemperature00_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature01_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature02_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature03_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature04_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature05_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature06_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature07_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature08_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature09_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature10_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature11_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature12_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature13_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature14_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature15_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature16_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature17_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature18_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature19_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature20_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature21_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature22_Writer(const timespec& pTimestamp, const float& pValue);
    void getTemperature23_Writer(const timespec& pTimestamp, const float& pValue);
    void getDetector24(const timespec* pTimestamp, float* pValue);
    void getDetector25(const timespec* pTimestamp, float* pValue);
    void getDetector26(const timespec* pTimestamp, float* pValue);
    void getDetector27(const timespec* pTimestamp, float* pValue);
    void getDetector28(const timespec* pTimestamp, float* pValue);
    void getDetector29(const timespec* pTimestamp, float* pValue);
    void getDetector30(const timespec* pTimestamp, float* pValue);
    void getDetector31(const timespec* pTimestamp, float* pValue);
    void getDetector32(const timespec* pTimestamp, float* pValue);
    void getDetector33(const timespec* pTimestamp, float* pValue);
    void getDetector34(const timespec* pTimestamp, float* pValue);
    void getDetector35(const timespec* pTimestamp, float* pValue);
    void getDetector36(const timespec* pTimestamp, float* pValue);
    void getDetector37(const timespec* pTimestamp, float* pValue);
    void getDetector38(const timespec* pTimestamp, float* pValue);
    void getDetector39(const timespec* pTimestamp, float* pValue);
    void getDetector40(const timespec* pTimestamp, float* pValue);
    void getDetector41(const timespec* pTimestamp, float* pValue);
    void getDetector42(const timespec* pTimestamp, float* pValue);
    void getDetector43(const timespec* pTimestamp, float* pValue);
    void getDetector44(const timespec* pTimestamp, float* pValue);
    void getDetector45(const timespec* pTimestamp, float* pValue);
    void getDetector46(const timespec* pTimestamp, float* pValue);
    void getDetector47(const timespec* pTimestamp, float* pValue);
    void getPlasmaParameter(const timespec* pTimestamp, float* pValue);
    void updateFromSubscribedInput(std::size_t index, const timespec& timestamp, float value);
    void recalculateOutputs(const std::array<float, 24>& inputs, const timespec& timestamp);
    void handleTemperatureWrite(std::size_t index, const timespec& timestamp, float value);
    float detectorSelectionValue(std::size_t index) const;
    float model_calculate_power(const std::array<float, 48>& inputs);
    std::vector<float> model_calculate_profile(const std::array<float, 48>& inputs);
    void getNuclearProfile(const timespec* pTimestamp, std::vector<float>* pValue);

    private:
        nds::PVBaseOut PV00;
        nds::PVBaseOut PV01;
        nds::PVBaseOut PV02;
        nds::PVBaseOut PV03;
        nds::PVBaseOut PV04;
        nds::PVBaseOut PV05;
        nds::PVBaseOut PV06;
        nds::PVBaseOut PV07;
        nds::PVBaseOut PV08;
        nds::PVBaseOut PV09;
        nds::PVBaseOut PV10;
        nds::PVBaseOut PV11;
        nds::PVBaseOut PV12;
        nds::PVBaseOut PV13;
        nds::PVBaseOut PV14;
        nds::PVBaseOut PV15;
        nds::PVBaseOut PV16;
        nds::PVBaseOut PV17;
        nds::PVBaseOut PV18;
        nds::PVBaseOut PV19;
        nds::PVBaseOut PV20;
        nds::PVBaseOut PV21;
        nds::PVBaseOut PV22;
        nds::PVBaseOut PV23;
        nds::PVBaseIn PV24;
        nds::PVBaseIn PV25;
        nds::PVBaseIn PV26;
        nds::PVBaseIn PV27;
        nds::PVBaseIn PV28;
        nds::PVBaseIn PV29;
        nds::PVBaseIn PV30;
        nds::PVBaseIn PV31;
        nds::PVBaseIn PV32;
        nds::PVBaseIn PV33;
        nds::PVBaseIn PV34;
        nds::PVBaseIn PV35;
        nds::PVBaseIn PV36;
        nds::PVBaseIn PV37;
        nds::PVBaseIn PV38;
        nds::PVBaseIn PV39;
        nds::PVBaseIn PV40;
        nds::PVBaseIn PV41;
        nds::PVBaseIn PV42;
        nds::PVBaseIn PV43;
        nds::PVBaseIn PV44;
        nds::PVBaseIn PV45;
        nds::PVBaseIn PV46;
        nds::PVBaseIn PV47;
        nds::PVBaseIn plasma_parameter;
        nds::PVBaseIn nuclear_profile;

        std::array<float, 24> m_subscribedInputs;
        std::array<bool, 24> m_subscribedValid;
        std::size_t m_subscribedValidCount;
        bool m_aggregatedOnce;
        std::mutex m_inputsMutex;

        float m_latestPlasmaParameter;
        timespec m_latestPlasmaTimestamp;
        std::vector<float> m_latestNuclearProfile;
        std::mutex m_plasmaParameterMutex;

        Ort::Env m_ortEnv;
        Ort::SessionOptions m_ortSessionOptions;
        Ort::Session m_profileSession;
        Ort::Session m_powerSession;
};

#endif