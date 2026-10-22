#include "VncDataProcessing.h"
#include <onnxruntime_cxx_api.h>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace {
    // First kModelChannelCount count rates are used (PCF0...PCF2, all channels).
    // TODO: include an algorithm to choose which counts rate to insert into model
    const std::size_t kModelChannelCount = 24;
    const std::array<std::int64_t,2> kInputShape = {1, vnc::kModelInputSize};

Ort::SessionOptions makeSessionOptions()
{
    Ort::SessionOptions options;
    options.SetIntraOpNumThreads(1);
    options.SetInterOpNumThreads(1);
    options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    return options;
}

// ONNX Runtime expects a single environment per process
Ort::Env &sharedEnv()
{
    static Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "vnc-sdn-service");
    return env;
}
}


namespace vnc {
    struct MlModel::Impl
    {
        explicit Impl(const std::string &modelPath)
            : options(makeSessionOptions()),
              session(sharedEnv(), modelPath.c_str(), options),
              memory(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)),
              input{},
              inputTensor(Ort::Value::CreateTensor<float>(memory, input.data(), input.size(), kInputShape.data(), kInputShape.size())),
              binding(session)
              {
                Ort::AllocatorWithDefaultOptions allocator;
                inputName = session.GetInputNameAllocated(0, allocator).get();
                outputName = session.GetOutputNameAllocated(0, allocator).get();

                // Output is allocated once, so Run() does not allocate in the 1 ms loop
                Ort::TypeInfo typeInfo = session.GetOutputTypeInfo(0);
                auto shapeInfo = typeInfo.GetTensorTypeAndShapeInfo();
                if (shapeInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
                    throw std::runtime_error("Model output is not a float tensor: " + modelPath);
                outputShape = shapeInfo.GetShape();
                std::size_t count = 1;
                for (std::int64_t &dim : outputShape) {
                    if (dim <= 0) dim = 1; // Dynamic batch dimension
                    count *= static_cast<std::size_t>(dim);
                }
                output.assign(count, 0.0f);
                outputTensor = Ort::Value::CreateTensor<float>(memory, output.data(), output.size(), outputShape.data(), outputShape.size());

                binding.BindInput(inputName.c_str(), inputTensor);
                binding.BindOutput(outputName.c_str(), outputTensor);
              }

              Ort::SessionOptions options;
              Ort::Session session;
              Ort::MemoryInfo memory;
              Features input;
              Ort::Value inputTensor;
              std::vector<std::int64_t> outputShape;
              std::vector<float> output;
              Ort::Value outputTensor{nullptr};
              Ort::IoBinding binding;
              Ort::RunOptions runOptions;
              std::string inputName;
              std::string outputName;
    };
    void InputCountRates::update (std::size_t ioc, const IocCountRates& rates)
    {
        m_data.at(ioc) = rates;
        m_updated = true;
    }

    void buildFeatures(const InputData &data, Features &features)
    {
        features.fill(0.0f);
        //The model was trained on ln(count rate); empty channels are passed as 0;
        //Later on the mechanism to define which count rate to use should be defined
        std::size_t k = 0;
        for (std::size_t ioc = 0; ioc < kIocCount && k < kModelChannelCount; ++ioc)
            for (std::size_t ch = 0; ch < kChannelCount && k < kModelChannelCount; ++ch, ++k) {
                const double rate = data[ioc][ch];
                features[k] = rate > 0.0 ? static_cast<float>(std::log(rate)) : 0.0f;
            }
        // Detector flags. Check the logic behind it
        features[35] = 1.0f;
        features[47] = 1.0f;
    }

    MlModel::MlModel(const std::string &modelPath)
        :m_impl(new Impl(modelPath))
    {
    }
    MlModel::~MlModel() = default;

    bool MlModel::run(const Features &features, double &output)
    {
        m_impl->input = features;
        m_impl->session.Run(m_impl->runOptions, m_impl->binding);
        output = m_impl->output[0];
        return std::isfinite(output);
    }


}
