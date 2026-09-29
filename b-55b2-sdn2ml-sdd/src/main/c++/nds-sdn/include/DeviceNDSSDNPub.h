/**
 * @file
 * 
 * @authors Mariano Ruiz (UPM)
 * @authors Alberto de Gracia (UPM)
 * @date 10 May 2019
 */

#ifndef NDS_SDN_EXAMPLES_INCLUDE_DEVICENDSSDNPUB_H_
#define NDS_SDN_EXAMPLES_INCLUDE_DEVICENDSSDNPUB_H_

#include <atomic>

#include <nds3/nds.h>

#include "definitions.h"
#include <sdn.h>
#include <sdn-core/sdn-topic-xml.h>
#include <common/AnyValue.h>
#include <common/AnyValueHelper.h>


/**
 * @brief Class used to handle a SDN Publisher using the SDN API.
 *
 * It contains the port implementation and all the PV nodes to control the reception of the data to publish it in SDN.
 *
 * @ingroup Device
 */
class DeviceNDSSDNPUB
{

public:

	/**
	 * @brief Constructor of the class.
	 *
	 * It will create all the NDS nodes objects necessaries for the device and will save the maps
	 * to the nodes and/or PVs.
	 *
	 * @param factory    the control system factory that requested the creation of the device
	 * @param deviceName     the name given to the device
	 * @param namedParameters String. See following description:
	 *
	 */
  DeviceNDSSDNPUB(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& namedParameters);

	/**
	 * @brief Destructor of the class.
	 */
  ~DeviceNDSSDNPUB();

private:
  /**
   * @brief Delegate function called in the transition from OFF and ON state.
   */
  void stm_SwitchOn(void);

  /**
   * @brief Delegate function called in the transition from ON and OFF state.
   */
  void stm_SwitchOff(void);

  /**
   * @brief Delegate function called in the transition from ON and RUNNING state.
   */
  void stm_Start (void);

  /**
   * @brief Delegate function called in the transition from RUNNING and ON state.
   */
  void stm_Stop(void);

  /**
   * @brief Delegate function called in the transition from FAULT to OFF state.
   */
  void stm_recover(void);

  /**
   * @brief Function to check if the required transition between states is allowed.
   *
   * @param currentState Current state of the state machine.
   * @param desiredState Desired state where the state machine should switch.
   * @return true if the transition of states is allowed.
   *
   */
  bool stm_allowChange(const nds::state_t currentState, const nds::state_t currentGlobalState, const nds::state_t desiredState);

  /**
   * @brief Function executed by the thread created by the driver, it will be publishing the Topic in case no single attribute has the
   * trigger field to yes.
   */
  void publishingThread(void);


  void initializePVs(void);

  /**
   * @brief This function will be called when the source PV sets a new int32_t value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_int32Attribute_writer(const timespec& timestamp, const std::int32_t& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new int64_t value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_int64Attribute_writer(const timespec& timestamp, const std::int64_t& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new float value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_floatAttribute_writer(const timespec& timestamp, const float& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new double value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_doubleAttribute_writer(const timespec& timestamp, const double& value, const std::string& pvName);

  /**
   *  @brief Takes a vector, converts it to O, and sets the attribute in the topic.
   *
   *  @tparam O 		Type desired
   *  @tparam I 		Type of the input vector
   *  @param input 		Input vector
   *  @param attrMult 	Number of elements/multiplicity of the attribute
   *  @param attRank	Identifier/Rank of the attribute in the topic
   */

  template <typename O, typename I>
  void convertVectorAndSetAttr(const std::vector<I> &input, uint_t attrMult, uint_t attRank);

  /**
   * @brief This function will be called when the source PV sets a new vector<bool> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_VboolAttribute_writer(const timespec& timestamp, const std::vector<bool>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<uint8_t> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_Vuint8Attribute_writer(const timespec& timestamp, const std::vector<std::uint8_t>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<uint16_t> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_Vuint16Attribute_writer(const timespec& timestamp, const std::vector<std::uint16_t>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<uint32_t> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_Vuint32Attribute_writer(const timespec& timestamp, const std::vector<std::uint32_t>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<int8_t> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_Vint8Attribute_writer(const timespec& timestamp, const std::vector<std::int8_t>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<int16_t> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_Vint16Attribute_writer(const timespec& timestamp, const std::vector<std::int16_t>& value, const std::string& pvName);

  /**
     * @brief This function will be called when the source PV sets a new vector<int32_t> value.
     *
     * @param timestamp Timestamp when the PV was called by the source NDS device driver.
     * @param value Actual PV value set by the source NDS device driver.
     * @param pvName Name of the PV that is executing the delegate function
     * @ingroup Device-DelegateFunctions
     */
    void PV_Vint32Attribute_writer(const timespec& timestamp, const std::vector<std::int32_t>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<int64_t> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_Vint64Attribute_writer(const timespec& timestamp, const std::vector<std::int64_t>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<float> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_VfloatAttribute_writer(const timespec& timestamp, const std::vector<float>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<double> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_VdoubleAttribute_writer(const timespec& timestamp, const std::vector<double>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new std::string value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_stringAttribute_writer(const timespec& timestamp, const std::string& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new timespec value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_timespecAttribute_writer(const timespec& timestamp, const timespec& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new vector<timespec> value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_VtimespecAttribute_writer(const timespec& timestamp, const std::vector<timespec>& value, const std::string& pvName);

  /**
   * @brief This function will be called when the source PV sets a new nds::timestamp_t value.
   *
   * @param timestamp Timestamp when the PV was called by the source NDS device driver.
   * @param value Actual PV value set by the source NDS device driver.
   * @param pvName Name of the PV that is executing the delegate function
   * @ingroup Device-DelegateFunctions
   */
  void PV_timestampAttribute_writer(const timespec& timestamp, const nds::timestamp_t& value, const std::string& pvName);

  ///------------------------------------------------------------------------------------------
  ///------------------------------------------------------------------------------------------
  ///--------------------------------- MEMBER VARIABLES ------------------------------
  ///------------------------------------------------------------------------------------------
  ///------------------------------------------------------------------------------------------

  /**
   * @brief Name of the device in the control system
   * @ingroup DeviceMemberVariables
   */
  std::string m_deviceName;


  /**
   * @brief Root node of the NDS driver.
   *
   * All children nodes and PVs are contained within this node. This will be the
   * main class of the driver. To enable interface with Asyn the Root class is
   * declared as Port.
   * @ingroup DeviceMemberVariables
   */
  nds::Port rootNode;

	/**
	 * @brief State machine node to handle the device.
	 *
	 * This node is linked to the next functions: @n
	 * <ul>
	 * 		<li>@ref stm_SwitchOn </li>
	 * 		<li>@ref stm_SwitchOff </li>
	 * 		<li>@ref stm_Start </li>
	 * 		<li>@ref stm_Stop </li>
	 * 		<li>@ref stm_recover </li>
	 * 		<li>@ref stm_allowChange </li>
	 * </ul>
	 * @ingroup DeviceMemberVariables
	 */
  nds::StateMachine m_stateMachine;

  /**
   * @brief A thread that runs publishingThread().
   *
   * @ingroup DeviceMemberVariables
   */
  nds::Thread m_threadPUB;

  /**
   * @brief A boolean flag that stop the data publishing loop in publishingThread() when true.
   *
   * @ingroup DeviceMemberVariables
   */
  std::atomic<bool> m_stopPUB;

  RET_STATUS m_constructionStatus;

  std::string m_ifeth;
  std::string m_topicName; //To search the xml file with the topic description
  std::string m_topicFilePath;
  std::unique_ptr<sdn::Publisher> m_pub;
  sdn::Topic m_topic;
  ndssdn::xmlTopic m_xmlTopic;
  ndssdn::publishingMode_t m_publishingMode;

  std::string m_errorLog;

  /**
   * @brief Map of int32_t PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::int32_PVs m_int32_PVs;

  /**
   * @brief Map of int64_t PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::int64_PVs m_int64_PVs;

  /**
   * @brief Map of float PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::float_PVs m_float_PVs;

  /**
   * @brief Map of double PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::double_PVs m_double_PVs;

  /**
   * @brief Map of vector<bool> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::boolArray_PVs m_boolArray_PVs;

  /**
   * @brief Map of vector<uint8_t> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::uint8Array_PVs m_uint8Array_PVs;

  /**
   * @brief Map of vector<uint16_t> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::uint16Array_PVs m_uint16Array_PVs;

  /**
   * @brief Map of vector<uint32_t> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::uint32Array_PVs m_uint32Array_PVs;

  /**
   * @brief Map of vector<int8_t> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::int8Array_PVs m_int8Array_PVs;

  /**
   * @brief Map of vector<int16_t> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::int16Array_PVs m_int16Array_PVs;

  /**
   * @brief Map of vector<int32_t> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::int32Array_PVs m_int32Array_PVs;

  /**
   * @brief Map of vector<int64_t> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::int64Array_PVs m_int64Array_PVs;

  /**
   * @brief Map of vector<float> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::floatArray_PVs m_floatArray_PVs;

  /**
   * @brief Map of vector<double> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::doubleArray_PVs m_doubleArray_PVs;

  /**
   * @brief Map of string PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::string_PVs m_string_PVs;

  /**
   * @brief Map of timespec PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::timespec_PVs m_timespec_PVs;

  /**
   * @brief Map of vector<timespec> PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::timespecArray_PVs m_timespecArray_PVs;

  /**
   * @brief Map of nds::timestamp_t PVs to handle the attribute actualization and topic publishing if the trigger field is set to yes.
   *
   * @ingroup DeviceMemberVariables
   */
  ndssdn::timestamp_PVs m_timestamp_PVs;

  /**
   * @brief Output PV to configure the publishing rate of the Topic from the Control System.
   *
   * @ingroup Device
   */
  nds::PVVariableOut<std::int32_t> m_publishingRate_PV;
  nds::Factory& m_factory;


  ///////////////////////////////////////////////////////////////////////////////////
  ////////// Auxiliary functions
  ///////////////////////////////////////////////////////////////////////////////////

  /**
   *@brief Auxiliary function used for parsing all input parameters
   *@param filename Path of the input XML
   *@param ans
   *@return ndssdn::xmlTopic It returns a structure with all the PV data need by the creation and subscription phases.
   *@ingroup Device-Functions
   */
  RET_STATUS parseXMLFile(const std::string &filename, ndssdn::xmlTopic& ans);

  /**
   *@brief Function that create and add all the PVs required by the Device to the rootNode
   *@param inTopic
   *@return RetStatusSuccess If the all the PVs have been correctly created.
   *@ingroup Device-Functions
   */
  RET_STATUS createPVs( ndssdn::xmlTopic& inTopic );

  /**
   *@brief Function that subscribe all the PVs available in the Device to the different Source PVs obtained from the input XML file.
   *@param inTopic
   *@return RetStatusSuccess If the all the PVs have been correctly subscribed.
   *@ingroup Device-Functions
   */
  RET_STATUS subscribePVs( ndssdn::xmlTopic& inTopic );

  /**
     *@brief Function that unsubscribe all the PVs available in the Device to the different Source PVs obtained from the input XML file.
     *@param inTopic
     *@return RetStatusSuccess If the all the PVs have been correctly unsubscribed.
     *@ingroup Device-Functions
     */
    RET_STATUS unsubscribePVs( ndssdn::xmlTopic& inTopic );

  /**
   *@brief Comparator for map ignore case
   */

    struct CaseInsensitiveComparator
    {
    	bool operator()(const std::string& a, const std::string&b) const noexcept
    	{
    		return ::strcasecmp(a.c_str(), b.c_str()) < 0;
    	}
    };

    /**
     *@brief Map containing the supported dataTypes strings and their equivalent NDS types
     */

    const std::map<std::string, nds::dataType_t, CaseInsensitiveComparator> mapString2NDSType {
    	{"int32", nds::dataType_t::dataInt32},
		{"int64", nds::dataType_t::dataInt64},
		{"float", nds::dataType_t::dataFloat32},
		{"double", nds::dataType_t::dataFloat64},
		{"bool", nds::dataType_t::dataBoolArray},
		{"uint8", nds::dataType_t::dataUint8Array},
		{"uint16", nds::dataType_t::dataUint16Array},
		{"uint32", nds::dataType_t::dataUint32Array},
		{"int8", nds::dataType_t::dataInt8Array},
		{"int16", nds::dataType_t::dataInt16Array},
		{"string", nds::dataType_t::dataString},
		{"timespec", nds::dataType_t::dataTimespec},
		{"timestamp", nds::dataType_t::dataTimestamp}
    };

};

#endif /* NDS_SDN_EXAMPLES_INCLUDE_DEVICENDSSDNPUB_H_ */
