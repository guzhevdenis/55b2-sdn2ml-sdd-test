#include "VncDataProcessing.h"
#include <onnxruntime_cxx_api.h>
#include "Timestamping.h"
#include <vector>
#include <iostream>
#include <fstream>

namespace {    
    const std::size_t kModelChannelCount = 24;
    const std::size_t kModelInputSize  = 48; // 24 input Count Rates and 24 flags for detectors; 


    // TODO: include an algorithm to choose which counts rate to insert into model 
    const std::size_t kModelChannelStep = 2; 
    const std::array<std::int64_t,2> kInputShape = {1, kModelInputSize};

Ort::SessionOptions makeSessionOptions()
{
    Ort::SessionOptions options;
    options.SetIntraOpNumThreads(1);
    options.SetInterOpNumThreads(1);
    options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    return options;
}
}


namespace vnc {
    struct MlProcessing::Impl
    {
        explicit Impl(const std::string &modelPath)
            : env(ORT_LOGGING_LEVEL_WARNING, "vnc-sdn-service"), 
              options(makeSessionOptions()), 
              session(env, modelPath.c_str(), options),
              memory(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)),
              input{},
              inputTensor(Ort::Value::CreateTensor<float>(memory, input.data(), input.size(), kInputShape.data(), kInputShape.size()))
              {
                Ort::AllocatorWithDefaultOptions allocator; 
                inputName = session.GetInputNameAllocated(0, allocator).get();
                outputName = session.GetOutputNameAllocated(0, allocator).get();
                // Check the logic behind it 
                for (int i = 24; i < 28; ++i) {
                    input[i] = 0.0f;
                }
                input[35] = 1.0f;
                input[47] = 1.0f;


                //To Check the correctness of calculation
/*                             const std::array<float, 24> neutronPower = {
                10.31028511f, 10.02734101f, 9.37763289f, 8.53562233f, 7.64873979f, 7.11232744f,
                6.90274274f, 6.48310735f, 6.68835471f, 6.85435450f, 6.77764659f, 5.47646355f,
                7.16472038f, 6.95844839f, 6.39191711f, 5.42053500f, 4.67282883f, 3.78418963f,
                2.83321334f, 3.04452244f, 3.58351894f, 3.46573590f, 4.00733319f, 2.19722458f
            }; */
              }

              Ort::Env env;
              Ort::SessionOptions options; 
              Ort::Session session;
              Ort::MemoryInfo memory; 
              std::array<float, kModelInputSize> input; 
              Ort::Value inputTensor;
              std::string inputName;
              std::string outputName;
              std::ofstream logFile{"/home/codac-dev/55b2-sdn2ml-sdd/55b2-sdn2ml-sdd/b-55b2-sdn2ml-sdd/src/main/c++/vnc-sdn-service/logs/ml_calc_log.txt"};
    };
    void InputCountRates::update (std::size_t ioc, const IocCountRates& rates)
    {
        m_data.at(ioc) = rates;
        m_updated = true;
    }
    MlProcessing::MlProcessing(const std::string &powerModelPath)
        :m_impl(new Impl(powerModelPath))
    {
    }
    MlProcessing::~MlProcessing() = default;

    bool MlProcessing::process(const InputData &data, OutputData &output)
    {
        std::size_t k = 0; 
        //First kModelChannelCount count rates (PCF0...PCF2, all channels).
        //The model was trained on ln(count rate); empty channels are passed as 0;
        //Later on the mechanism to define which count rate to use should be defined 
        for (const IocCountRates& rates : data)
        //Here is the logic for selection which Count rates we should take
            for (std::size_t ch = 0; ch < kChannelCount && kModelChannelCount; ch++){
                    const double rate = rates[ch];
                    m_impl->input[k++] = rate > 0.0 ? static_cast<float>(std::log(rate)): 0.0f;
            }
        m_impl->logFile << std::endl;  

        const char *inputNames[] = {m_impl->inputName.c_str()};
        const char *outputNames[] = {m_impl->outputName.c_str()};

        const float* t = m_impl->inputTensor.GetTensorData<float>();
        m_impl->logFile << "tensor ptr= " << t[0] << " ";
        std::vector<Ort::Value> result = m_impl->session.Run(
            Ort::RunOptions{nullptr}, inputNames, &m_impl->inputTensor, 1, outputNames, 1);
        if (result.empty() || result[0].GetTensorTypeAndShapeInfo().GetElementCount() == 0)
            return false; 
        

        output.outputValue = result[0].GetTensorData<float>()[0];
        output.timestamp = monotonicNs();
        m_impl->logFile << " output value " << output.outputValue << std::endl;

        return true;

    }


}