#include <nds3/nds.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <mutex>
#include <ctime>
#include "DataAgregator.h"
#include <cmath>    // Для std::exp
#include <algorithm> // Для std::min, std::max
// This class declares our device in the contructor and supplies the functionalities to support it

namespace {
std::mutex g_tsLogMutex;
const char* kTsLogPath = "../../../../src/main/c++/DataAgregator/log/DataAgregator_timestamps.log";
const char* kTsFallbackLogPath = "DataAgregator_timestamps.log";
std::ofstream g_tsLogFile;
bool g_tsUsingFallback = false;
std::once_flag g_tsLogInitOnce;

void ensureTsLogOpened() {
    std::call_once(g_tsLogInitOnce, []() {
        g_tsLogFile.open(kTsLogPath, std::ios::app);
        if (!g_tsLogFile.is_open()) {
            g_tsLogFile.clear();
            g_tsLogFile.open(kTsFallbackLogPath, std::ios::app);
            if (g_tsLogFile.is_open()) {
                g_tsUsingFallback = true;
                std::cerr << "DataAgregator: primary log path unavailable, using current directory: "
                          << kTsFallbackLogPath << std::endl;
            }
        }
    });
}

void logTimestampEvent(const char* eventName, const timespec& ts) {
    static bool s_reported = false;
    ensureTsLogOpened();
    if (!g_tsLogFile.is_open()) {
        if (!s_reported) {
            std::cerr << "DataAgregator: cannot open timestamp log file: " << kTsLogPath;
            if (g_tsUsingFallback) {
                std::cerr << " or fallback: " << kTsFallbackLogPath;
            }
            std::cerr << std::endl;
            s_reported = true;
        }
        return;
    }
    std::lock_guard<std::mutex> lock(g_tsLogMutex);
    g_tsLogFile << eventName << "," << ts.tv_sec << "." << ts.tv_nsec << std::endl;
}
}

         // Constructor. Here we setup the device structure
     DataAgregator::DataAgregator(nds::Factory &factory,
                 const std::string &deviceName,
                const nds::namedParameters_t &parameters)
        : m_ortEnv(ORT_LOGGING_LEVEL_WARNING, "DataAgregator"),
          m_ortSessionOptions(),
          m_profileSession(m_ortEnv, MODEL1_PATH, m_ortSessionOptions),
          m_powerSession(m_ortEnv, MODEL2_PATH, m_ortSessionOptions)
     {
        (void)parameters;
        m_subscribedInputs.fill(0.0f);
        m_subscribedValid.fill(false);
        m_subscribedValidCount = 0;
        m_aggregatedOnce = false;
        m_latestPlasmaParameter = 0.0f;
        m_latestPlasmaTimestamp = {0, 0};
        m_latestNuclearProfile.clear();

        
         nds::Port rootNode(deviceName);

         PV00 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature00-received", std::bind(&DataAgregator::getTemperature00_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV01 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature01-received", std::bind(&DataAgregator::getTemperature01_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV02 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature02-received", std::bind(&DataAgregator::getTemperature02_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV03 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature03-received", std::bind(&DataAgregator::getTemperature03_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV04 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature04-received", std::bind(&DataAgregator::getTemperature04_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV05 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature05-received", std::bind(&DataAgregator::getTemperature05_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV06 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature06-received", std::bind(&DataAgregator::getTemperature06_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV07 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature07-received", std::bind(&DataAgregator::getTemperature07_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV08 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature08-received", std::bind(&DataAgregator::getTemperature08_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV09 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature09-received", std::bind(&DataAgregator::getTemperature09_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV10 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature10-received", std::bind(&DataAgregator::getTemperature10_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV11 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature11-received", std::bind(&DataAgregator::getTemperature11_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV12 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature12-received", std::bind(&DataAgregator::getTemperature12_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV13 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature13-received", std::bind(&DataAgregator::getTemperature13_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV14 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature14-received", std::bind(&DataAgregator::getTemperature14_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV15 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature15-received", std::bind(&DataAgregator::getTemperature15_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV16 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature16-received", std::bind(&DataAgregator::getTemperature16_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV17 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature17-received", std::bind(&DataAgregator::getTemperature17_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV18 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature18-received", std::bind(&DataAgregator::getTemperature18_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV19 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature19-received", std::bind(&DataAgregator::getTemperature19_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV20 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature20-received", std::bind(&DataAgregator::getTemperature20_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV21 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature21-received", std::bind(&DataAgregator::getTemperature21_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV22 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature22-received", std::bind(&DataAgregator::getTemperature22_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV23 = rootNode.addChild(
         nds::PVDelegateOut<float>("Temperature23-received", std::bind(&DataAgregator::getTemperature23_Writer,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV24 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector24", std::bind(&DataAgregator::getDetector24,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV25 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector25", std::bind(&DataAgregator::getDetector25,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV26 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector26", std::bind(&DataAgregator::getDetector26,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV27 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector27", std::bind(&DataAgregator::getDetector27,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV28 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector28", std::bind(&DataAgregator::getDetector28,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV29 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector29", std::bind(&DataAgregator::getDetector29,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV30 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector30", std::bind(&DataAgregator::getDetector30,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV31 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector31", std::bind(&DataAgregator::getDetector31,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV32 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector32", std::bind(&DataAgregator::getDetector32,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV33 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector33", std::bind(&DataAgregator::getDetector33,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV34 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector34", std::bind(&DataAgregator::getDetector34,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV35 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector35", std::bind(&DataAgregator::getDetector35,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV36 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector36", std::bind(&DataAgregator::getDetector36,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV37 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector37", std::bind(&DataAgregator::getDetector37,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV38 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector38", std::bind(&DataAgregator::getDetector38,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV39 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector39", std::bind(&DataAgregator::getDetector39,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV40 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector40", std::bind(&DataAgregator::getDetector40,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV41 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector41", std::bind(&DataAgregator::getDetector41,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV42 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector42", std::bind(&DataAgregator::getDetector42,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV43 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector43", std::bind(&DataAgregator::getDetector43,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV44 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector44", std::bind(&DataAgregator::getDetector44,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV45 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector45", std::bind(&DataAgregator::getDetector45,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV46 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector46", std::bind(&DataAgregator::getDetector46,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         PV47 = rootNode.addChild(  
         nds::PVDelegateIn<float>("Detector47", std::bind(&DataAgregator::getDetector47,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         
         // Термоядерная мощность
         plasma_parameter = rootNode.addChild(  
         nds::PVDelegateIn<float>("PlasmaParameter", std::bind(&DataAgregator::getPlasmaParameter,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));
         // Профиль
         nuclear_profile = rootNode.addChild(  
         nds::PVDelegateIn<std::vector<float>>("NuclearProfile", std::bind(&DataAgregator::getNuclearProfile,
            this,
            std::placeholders::_1,
            std::placeholders::_2)));   
        rootNode.initialize(this, factory);
     }

     // Для одной и второй модели Саша использует разные входные значения, с помощью которых можно проверить результат
     // Под комментарием входные значения для модели по мощности и переменная для передачи значения между устройствами
void DataAgregator::handleTemperatureWrite(std::size_t index, const timespec& timestamp, float value)
{
  updateFromSubscribedInput(index, timestamp, value);
}

float DataAgregator::detectorSelectionValue(std::size_t index) const
{
  // Detector35 and Detector47 are enabled, others are disabled.
  return (index == 11 || index == 23) ? 1.0f : 0.0f;
}

  void DataAgregator::getTemperature00_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(0, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature01_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(1, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature02_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(2, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature03_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(3, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature04_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(4, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature05_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(5, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature06_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(6, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature07_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(7, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature08_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(8, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature09_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(9, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature10_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(10, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature11_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(11, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature12_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(12, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature13_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(13, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature14_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(14, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature15_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(15, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature16_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(16, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature17_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(17, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature18_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(18, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature19_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(19, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature20_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(20, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature21_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(21, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature22_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(22, pTimestamp, pValue);
  }
  void DataAgregator::getTemperature23_Writer(const timespec& pTimestamp, const float& pValue)
  {
  handleTemperatureWrite(23, pTimestamp, pValue);
  }


// Значения выбора детекора
  void DataAgregator::getDetector24(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(0);}
  void DataAgregator::getDetector25(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(1);}
  void DataAgregator::getDetector26(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(2);}
  void DataAgregator::getDetector27(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(3);}
  void DataAgregator::getDetector28(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(4);}
  void DataAgregator::getDetector29(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(5);}
  void DataAgregator::getDetector30(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(6);}
  void DataAgregator::getDetector31(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(7);}
  void DataAgregator::getDetector32(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(8);}
  void DataAgregator::getDetector33(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(9);}
  void DataAgregator::getDetector34(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(10);}
  void DataAgregator::getDetector35(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(11);}
  void DataAgregator::getDetector36(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(12);}
  void DataAgregator::getDetector37(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(13);}
  void DataAgregator::getDetector38(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(14);}
  void DataAgregator::getDetector39(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(15);}
  void DataAgregator::getDetector40(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(16);}
  void DataAgregator::getDetector41(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(17);}
  void DataAgregator::getDetector42(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(18);}
  void DataAgregator::getDetector43(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(19);}
  void DataAgregator::getDetector44(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(20);}
  void DataAgregator::getDetector45(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(21);}
  void DataAgregator::getDetector46(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(22);}
  void DataAgregator::getDetector47(const timespec* pTimestamp, float* pValue){(void)pTimestamp; *pValue = detectorSelectionValue(23);}
     
 void DataAgregator::getNuclearProfile(const timespec* pTimestamp, std::vector<float>* pValue){
  (void)pTimestamp;
  std::lock_guard<std::mutex> lock(m_plasmaParameterMutex);
  *pValue = m_latestNuclearProfile;
}
std::vector<float> DataAgregator::model_calculate_profile(const std::array<float, 48>& inputs) {
    // Создаем аллокатор для работы со строками и памятью ONNX
    Ort::AllocatorWithDefaultOptions allocator;
    // Получаем имя входного тензора модели
    Ort::AllocatedStringPtr input_name = m_profileSession.GetInputNameAllocated(0, allocator);
    // Получаем имя выходного тензора модели
    Ort::AllocatedStringPtr output_name = m_profileSession.GetOutputNameAllocated(0, allocator);
    // Упаковываем имена в массивы для передачи в `session.Run`
    const char* input_names[] = { input_name.get() };
    const char* output_names[] = { output_name.get() };
    // Указываем форму входного тензора: один образец, 48 признаков
    static const std::array<int64_t, 2> input_shape = {1, 48};
    // Создаем описание памяти: используем CPU
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    // Создаем входной тензор из вектора inputs
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
      memory_info, 
       const_cast<float*>(inputs.data()), // ONNX API expects mutable pointer
       inputs.size(),                    // количество элементов
       input_shape.data(),   // указатель на форму
       input_shape.size()    // количество измерений
   );
    // Запускаем инференс модели с входными данными
    std::vector<Ort::Value> outputs = m_profileSession.Run(
      Ort::RunOptions{nullptr}, // опции запуска
      input_names,              // имена входов
      &input_tensor, 1,         // входной тензор и количество
      output_names, 1           // имена выходов и количество
   );

   // Получаем указатель на результат предсказания (матрица 23*40)
   float* raw_output = outputs[0].GetTensorMutableData<float>();
   
   const int rows = 40;
   const int cols = 23;
   std::vector<float> result;
   result.reserve(rows * cols);
   for (int i = 0; i < rows * cols; ++i) {
       float x = raw_output[i];
       // Операции с результатом для корректного вывода
       float clipped = std::min(std::max(0.0f, x), 30.0f);
       float processed = std::exp(clipped) - 1.0f;
       result.push_back(processed);
   }
   return result;
  }
  
void DataAgregator::recalculateOutputs(const std::array<float, 24>& inputs24, const timespec& timestamp)
{
  std::array<float, 48> inputs{};
  for (std::size_t i = 0; i < 24; ++i) {
    inputs[i] = inputs24[i];
  }
  // Detector channels: fixed one-shot configuration
  inputs[35] = 1.0f;
  inputs[47] = 1.0f;

  logTimestampEvent("received_timestamp", timestamp);
  const float power = model_calculate_power(inputs);
  const std::vector<float> profile = model_calculate_profile(inputs);

  {
    std::lock_guard<std::mutex> lock(m_plasmaParameterMutex);
    m_latestPlasmaParameter = power;
    m_latestNuclearProfile = profile;
    m_latestPlasmaTimestamp = timestamp;
  }
  plasma_parameter.push(timestamp, power);
  nuclear_profile.push(timestamp, profile);
}

void DataAgregator::updateFromSubscribedInput(std::size_t index, const timespec& timestamp, float value)
{
  if (index >= 24) {
    return;
  }

  bool readyToAggregate = false;
  std::array<float, 24> inputsSnapshot{};
  timespec calcTimestamp = timestamp;

  {
    std::lock_guard<std::mutex> lock(m_inputsMutex);
    if (m_aggregatedOnce) {
      return;
    }

    if (!m_subscribedValid[index]) {
      m_subscribedValid[index] = true;
      ++m_subscribedValidCount;
    }
    m_subscribedInputs[index] = value;

    if (m_subscribedValidCount == 24) {
      m_aggregatedOnce = true;
      readyToAggregate = true;
      inputsSnapshot = m_subscribedInputs;
      calcTimestamp = timestamp;
    }
  }

  if (!readyToAggregate) {
    return;
  }

  // Run model inference outside m_inputsMutex to minimize callback path latency.
  recalculateOutputs(inputsSnapshot, calcTimestamp);
}

 void DataAgregator::getPlasmaParameter(const timespec* pTimestamp, float* pValue)
{
   (void)pTimestamp;
   std::lock_guard<std::mutex> lock(m_plasmaParameterMutex);
   *pValue = m_latestPlasmaParameter;
}
   
float DataAgregator::model_calculate_power(const std::array<float, 48>& inputs) {
    // Создаем аллокатор для работы со строками и памятью ONNX
    Ort::AllocatorWithDefaultOptions allocator;
    // Получаем имя входного тензора модели
    Ort::AllocatedStringPtr input_name = m_powerSession.GetInputNameAllocated(0, allocator);
    // Получаем имя выходного тензора модели
    Ort::AllocatedStringPtr output_name = m_powerSession.GetOutputNameAllocated(0, allocator);
    // Упаковываем имена в массивы для передачи в `session.Run`
    const char* input_names[] = { input_name.get() };
    const char* output_names[] = { output_name.get() };
    // Указываем форму входного тензора: один образец, 48 признаков
    static const std::array<int64_t, 2> input_shape = {1, 48};
    // Создаем описание памяти: используем CPU
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    timespec tsBeforeCreateTensor;
    clock_gettime(CLOCK_REALTIME, &tsBeforeCreateTensor);
    logTimestampEvent("before_create_tensor", tsBeforeCreateTensor);
    // Создаем входной тензор из вектора inputs
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
      memory_info, 
       const_cast<float*>(inputs.data()), // ONNX API expects mutable pointer
       inputs.size(),                    // количество элементов
       input_shape.data(),   // указатель на форму
       input_shape.size()    // количество измерений
   );
    // Запускаем инференс модели с входными данными
    std::vector<Ort::Value> outputs = m_powerSession.Run(
      Ort::RunOptions{nullptr}, // опции запуска
      input_names,              // имена входов
      &input_tensor, 1,         // входной тензор и количество
      output_names, 1           // имена выходов и количество
   );
    timespec tsAfterSessionRun;
    clock_gettime(CLOCK_REALTIME, &tsAfterSessionRun);
    logTimestampEvent("after_session_run", tsAfterSessionRun);

   // Получаем указатель на результат предсказания (массив float, размером 1)
   float* result = outputs[0].GetTensorMutableData<float>();
   // Возвращаем первое (и единственное) значение из результата
   return result[0];
  }


 // If we want to load our device support dynamically then we have to compile it
 // as a dynamic module and we have to provide the functions to allocate it,
// deallocate it and retrieve its name
 NDS_DEFINE_DRIVER(DataAgregator, DataAgregator)
