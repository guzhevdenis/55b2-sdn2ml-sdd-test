/**
 * @file definitions.h
 *
 * @brief Header file to define data structures, function prototypes and logging macros for @e nds-sdn module
 *
 * @author Javier Moreno (GMV)
 * @version 1.0
 * @ingroup DataTypes
 *
 * Include nds-sdn.h instead of this file to have full access to the @e nds-sdn API.
 */

#ifndef NDS_SDN_INCLUDE_DEFINITIONS_H_
#define NDS_SDN_INCLUDE_DEFINITIONS_H_
#include <log.h>
#include <sdn.h>
#include <mutex>
#include <condition_variable>
#include <nds3/nds.h>

#define NULL_PTR(ptr) static_cast<ptr>(0)

/**
 *
 *
 * This module creates the @e NDS support for @e %SDN integration. It allows extending @e NDS drivers
 * with additional nodes that provides access to @e %SDN API. In such way, NDS drivers can archive
 * data without advanced acknowledge of @e %SDN. @n
 *
 * @defgroup NDS-SDN Module that details information about general features that can be controlled for a @e NDS driver that includes a @e nds-sdn node.
 *
 * @defgroup DataTypes Definitions of data type structures and functions to be used by classes and methods.
 * @ingroup NDS-SDN
 *
 */

/**
 * @ingroup NDS-SDN
 * @brief Namespace that contains all the module classes, data types and declarations.
 */
namespace ndssdn
{

// ---------------------------------------------------------------------------------------------
// ----------------------------- Data types structures definition ------------------------------
// ---------------------------------------------------------------------------------------------

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::int32_t PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::int32_t>> int32_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::int64_t PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::int64_t>> int64_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the float PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<float>> float_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the double PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<double>> double_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<bool> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<bool>>> boolArray_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::uint8_t> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<std::uint8_t>>> uint8Array_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::uint16_t> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<std::uint16_t>>> uint16Array_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::uint32_t> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<std::uint32_t>>> uint32Array_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::int8_t> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<std::int8_t>>> int8Array_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::int16_t> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<std::int16_t>>> int16Array_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::int32_t> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<std::int32_t>>> int32Array_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::int64_t> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<std::int64_t>>> int64Array_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::float> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<float>>> floatArray_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::double> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<double>>> doubleArray_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<std::string> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::string>>string_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the timespec PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<timespec>> timespec_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<timespec> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<std::vector<timespec>>> timespecArray_PVs;

  /**
   * @ingroup DataTypes
   * @brief Definition of map container to ease the std::vector<nds::timestamp_t> PVs management
   */
  typedef std::map<std::string,nds::PVDelegateOut<nds::timestamp_t>> timestamp_PVs;

  /**
   * @ingroup DataTypes
   * @brief Structure that defines all the information related to a single attribute. All the fields of
   * this structure all filled through an XML file except nelm which is recovered from the TOPIC definition.
   */
  typedef struct xmlAttribute
  {
	  /**
	   * @brief Name of the attribute to find in the @e topic definition
	   */
	  std::string attName;

	  /**
	   * @brief Name of the PV that will be the data source to fill the attribute in the @e topic
	   */
	  std::string pvSource;

	  /**
	   * @brief Datatype of the source PV.
	   */
	  std::string pvType;

	  /**
	   * @brief Number of elements expected for attribute. This field is retrieved from the @e topic definition
	   * In the case of receiving from the NDS PV less elements than attribute needs. The driver will fill the remaining elements with zeros.
	   * In the case of receiving from the NDS PV more elements than attribute needs. The driver will discard the remaining elements.
	   */
	  uint_t nelm=1;

	  /**
	   * @brief Name of the attribute to find in the @e topic definition
	   */
	  bool trigger;
  }xmlAttribute_t;

  /**
   * @ingroup DataTypes
   * @brief This map contains all the attributes information. These attributes are stored by name.
   */
  typedef std::map<std::string,xmlAttribute_t> xmlTopic;

  enum class publishingMode_t : std::uint8_t{
	  periodic,
	  trigger
  };


  // ---------------------------------------------------------------------------------------------
  // -------------------------- End of Data types structures definition --------------------------
  // ---------------------------------------------------------------------------------------------


}

#define NDS_SDN_API

#endif /* NDS_SDN_INCLUDE_DEFINITIONS_H_ */
