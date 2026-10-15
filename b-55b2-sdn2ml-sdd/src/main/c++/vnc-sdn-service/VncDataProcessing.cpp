#include "VncDataProcessing.h"
#include <onnxruntime_cxx_api.h>
#include <vector>

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
                input[35] = 1.0f;
                input[47] = 1.0f;
              }

              Ort::Env env;
              Ort::SessionOptions options; 
              Ort::Session session;
              Ort::MemoryInfo memory; 
              std::array<float, kModelInputSize> input; 
              Ort::Value inputTensor;
              std::string inputName;
              std::string outputName;
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
        for (const IocCountRates& rates : data)
        //Here is the logic for selection which Count rates we should take
            for (std::size_t ch = 0; ch < kChannelCount; ch += kModelChannelStep)
                m_impl->input[k++] = static_cast<float>(rates[ch]);
        const char *inputNames[] = {m_impl->inputName.c_str()};
        const char *outputNames[] = {m_impl->outputName.c_str()};
        std::vector<Ort::Value> result = m_impl->session.Run(
            Ort::RunOptions{nullptr}, inputNames, &m_impl->inputTensor, 1, outputNames, 1);
        if (result.empty() || result[0].GetTensorTypeAndShapeInfo().GetElementCount() == 0)
            return false; 

        output.outputValue = result[0].GetTensorData<float>()[0];

        return true;
            
            
            
    }


}