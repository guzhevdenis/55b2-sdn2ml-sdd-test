/**
 * @file
 *
 * @author Mariano Ruiz (UPM)
 * @author Alberto de Gracias (UPM)
 * @date 10 May 2019
 */

 //basically it's a parser for the XML config file 

#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <log.h>
#include <iostream>

#include "DeviceNDSSDNPub.h"

#define NDS_EPOCH 1514764800 /* 00:00 of 1/1/2018 in UTC format. */
#define S2NS 1000000000 //Seconds to nanoSeconds

using namespace ndssdn;
static std::map<std::string, DeviceNDSSDNPUB*> m_DevicesMap;
static std::mutex m_lockDevicesMap;

DeviceNDSSDNPUB::DeviceNDSSDNPUB (nds::Factory& factory,
                                const std::string& deviceName,
                                const nds::namedParameters_t& namedParameters) :
      m_deviceName(deviceName),
      rootNode(deviceName),
	  m_stopPUB(true),
	  m_publishingMode(publishingMode_t::periodic),
	  m_factory(factory)
{
  {
	  std::lock_guard<std::mutex> lock(m_lockDevicesMap);
	  if(m_DevicesMap.find(deviceName) != m_DevicesMap.end())
	  {
		  throw std::logic_error("Device:"+deviceName+" already allocated");
	  }
	  m_DevicesMap[deviceName] = this;
  }

  nds::namedParameters_t::const_iterator findParam =  namedParameters.find("INIT");
  
  bool initPVs = true;
  
  if (findParam != namedParameters.end() && findParam->second=="NO") {
    initPVs = false;
  }

  m_constructionStatus=RetStatusError;

  //Checking if the network interface is found
  findParam =  namedParameters.find("INTERFACE");
  if (findParam != namedParameters.end())  {
	  m_ifeth =  findParam->second;
  }
  else {
	  const char *env_interface = std::getenv("SDN_INTERFACE_NAME");
	  m_ifeth.assign(env_interface);
  }

  std::string errorMsg(STRING_UNDEFINED);
  char path [256] = STRING_UNDEFINED;

  if (!m_ifeth.empty()){ //if interface is not defined do not continue checking
		findParam =  namedParameters.find("PVS_CONFIG_PATH");
		if (findParam != namedParameters.end())  {
		  m_constructionStatus = parseXMLFile(findParam->second,m_xmlTopic);
		  //Getting TOPIC from XML file
		  if(m_constructionStatus==RetStatusSuccess){
			  m_constructionStatus=sdn::TopicHelper::LocateDefinitionFile(m_topicName.c_str(),path);
			  if(m_constructionStatus==RetStatusSuccess){
				  m_constructionStatus=sdn::TopicHelper::LoadDefinitionFile(&m_topic, path);
				  if(m_constructionStatus==RetStatusSuccess)
					  m_constructionStatus=createPVs(m_xmlTopic);
				  if(m_constructionStatus!=RetStatusSuccess) errorMsg="Error creating NDS PVs";
				  else errorMsg = "Error loading definition file";
			  }
			  else errorMsg="Locate definition file error: Topic not found";

		  }
		  else errorMsg="Check if XML file exist or XML parsing error content";
		}
		else errorMsg="PVS_CONFIG_PATH not defined";
  }
  else errorMsg="SDN interface is not defined. Check SDN_INTERFACE_NAME or INTERFACE parameter";

  if (m_constructionStatus==RetStatusSuccess){

	  	m_stateMachine = rootNode.addChild(nds::StateMachine( false,
	                    std::bind(&DeviceNDSSDNPUB::stm_SwitchOn, this),
	                    std::bind(&DeviceNDSSDNPUB::stm_SwitchOff, this),
	                    std::bind(&DeviceNDSSDNPUB::stm_Start, this),
	                    std::bind(&DeviceNDSSDNPUB::stm_Stop, this),
	                    std::bind(&DeviceNDSSDNPUB::stm_recover, this),
	                    std::bind(&DeviceNDSSDNPUB::stm_allowChange, this,
	                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
/////////////////////////////////////////////////////////////////////////
/// Creating Auxiliary PVs
//////////////////////////////////////////////////////////////////////////
	  m_publishingRate_PV = rootNode.addChild(nds::PVVariableOut<std::int32_t>("publishingRate"));

//////////////////////////////////////////////////////////////////////////
/// Initializing rootNode
//////////////////////////////////////////////////////////////////////////
	  rootNode.initialize(this, factory);
     
     if(initPVs){
       initializePVs();
     }
    
#ifdef DEBUG
	  rootNode.setLogLevel(nds::logLevel_t::debug);
	  ndsDebugStream(rootNode) << "Log level: Debug" << std::endl;
	  ndsDebugStream(rootNode) << "SDN XML topic definiton file: " << path<< std::endl;
	  ndsDebugStream(rootNode) << "SDN XML topic name: " << m_topicName<< std::endl;
#else
	  rootNode.setLogLevel(nds::logLevel_t::warning);
#endif
  }
  else {

	m_DevicesMap.erase(deviceName);

	throw std::logic_error(errorMsg.c_str());
  }
}

DeviceNDSSDNPUB::~DeviceNDSSDNPUB ()
{
	std::lock_guard<std::mutex> lock(m_lockDevicesMap);

	nds::state_t currentState;
	currentState=rootNode.getState();
	int timeout(0);
	switch (currentState)
	{
		case nds::state_t::running:
			rootNode.setState(nds::state_t::on);
			while (rootNode.getState()!=nds::state_t::on){
				::usleep(1000);
				timeout++;
				if (timeout>500) break;
			}
			rootNode.setState(nds::state_t::off);
			while (rootNode.getState()!=nds::state_t::off){
				::usleep(1000);
				timeout++;
				if (timeout>500) break;
			}

			break;
		case nds::state_t::on:
			rootNode.setState(nds::state_t::off);
			while (rootNode.getState()!=nds::state_t::off){
				::usleep(1000);
				timeout++;
				if (timeout>500) break;
			}
			break;
		default:
			break;
	}

	m_DevicesMap.erase(m_deviceName);
}

void DeviceNDSSDNPUB::stm_SwitchOn (void)
{
  ndsDebugStream(rootNode) << (m_deviceName + "-STM: Switching On" ) << std::endl;
  bool ethfound;
  ethfound=net_is_interface_valid(m_ifeth.c_str());
  if(!ethfound){
	  throw nds::StateMachineError("Selected Ethernet Interface is not present.");
  }

  if(subscribePVs(m_xmlTopic)==RetStatusError){
	  m_constructionStatus = RetStatusError;
	  throw nds::StateMachineError("One or more PVs can't subscribe to its source PV.");
    }

  //////////////////////////////////////////////////////////////////////////
  /// Initializing Publisher
  //////////////////////////////////////////////////////////////////////////
  m_pub.reset(new sdn::Publisher(m_topic));
  m_pub->SetInterface(m_ifeth.c_str());
  if(m_pub->Configure()==RetStatusError){
      m_constructionStatus = RetStatusError;
      throw nds::StateMachineError("The SDN Publisher can not be correctly configured. ");
  }
}

void DeviceNDSSDNPUB::stm_SwitchOff (void)
{
  ndsDebugStream(m_stateMachine) << (m_deviceName + "-STM: Switching Off" ) << std::endl;

  unsubscribePVs(m_xmlTopic);
}

void DeviceNDSSDNPUB::stm_Start (void)
{
  const std::string threadName = m_deviceName;
  ndsDebugStream(m_stateMachine) << (m_deviceName + "-STM: Starting" ) << std::endl;
  m_stopPUB = false;
  if(m_publishingMode == publishingMode_t::periodic){
	  m_threadPUB = rootNode.runInThread(threadName, std::bind(&DeviceNDSSDNPUB::publishingThread, this));
  }
}

void DeviceNDSSDNPUB::stm_Stop (void)
{
  ndsDebugStream(m_stateMachine) << (m_deviceName + "-STM: Stopping" ) << std::endl;
  m_stopPUB = true;
  if(m_publishingMode == publishingMode_t::periodic){
	  m_threadPUB.join();
	  ndsDebugStream(m_stateMachine) << "SDN successfully stopped." << std::endl;
  }
}

void DeviceNDSSDNPUB::stm_recover (void)
{
  ndsDebugStream(m_stateMachine) << (m_deviceName + "-STM: Recovering" ) << std::endl;
}

bool DeviceNDSSDNPUB::stm_allowChange(const nds::state_t currentState, const nds::state_t currentGlobalState __attribute__((unused)), const nds::state_t desiredState)
{
	if(m_constructionStatus==RetStatusError){
		throw nds::StateMachineError(m_errorLog);
	}
  if (currentState == nds::state_t::off && desiredState == nds::state_t::running) {
      ndsErrorStream(m_stateMachine) << (m_deviceName + "-STM: State change not allowed until switching on the SDNPUB' state machine" ) << std::endl;
      return false;
  } else {
      ndsDebugStream(m_stateMachine) << (m_deviceName + "-STM: Allowing change" ) << std::endl;
      return true;
  }
}

void DeviceNDSSDNPUB::PV_int32Attribute_writer(const timespec& /*timestamp*/, const std::int32_t& value, const std::string& pvName){

	std::string attFromPV = pvName.substr (m_deviceName.length()+1);
	sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
	xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

	if(m_stateMachine.getLocalState()==nds::state_t::running){
		if(it!=m_xmlTopic.end()){
			uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
			TypeIdentifier_t attType = p_type_def->GetAttributeType(attRank);

			switch(attType){
			case (ANYTYPE_ATTR_SINT8):
					m_topic.SetAttribute<std::int8_t>(attRank, static_cast<std::int8_t>(value));
			break;
			case (ANYTYPE_ATTR_SINT16):
					m_topic.SetAttribute<std::int16_t>(attRank, static_cast<std::int16_t>(value));
			break;
			case (ANYTYPE_ATTR_SINT32):
					m_topic.SetAttribute<std::int32_t>(attRank, value);
			break;
			case(ANYTYPE_ATTR_SINT64):
					m_topic.SetAttribute<std::int64_t>(attRank, static_cast<std::int64_t>(value));
			break;
			default:
				ndsErrorStream(rootNode) << "Introspection failed for attribute: " << attFromPV << " attribute type: " << (int)attType << std::endl;
				return;
				break;
			}

			if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
				if(m_pub->Publish()!=STATUS_ERROR){
					ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV << "] attribute." << m_topic.GetName() << std::endl;
				}
			}
		}else{
			ndsErrorStream(rootNode) << "Attribute ["<< attFromPV << "] not found." << std::endl;
		}
	}else{
		ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
	}
}

void DeviceNDSSDNPUB::PV_int64Attribute_writer(const timespec& /*timestamp*/, const std::int64_t& value, const std::string& pvName){

	std::string attFromPV = pvName.substr (m_deviceName.length()+1);
	sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
	xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

	if(m_stateMachine.getLocalState()==nds::state_t::running){
		if(it!=m_xmlTopic.end()){
			uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
			TypeIdentifier_t attType = p_type_def->GetAttributeType(attRank);

			switch(attType){
			case (ANYTYPE_ATTR_SINT8):
					m_topic.SetAttribute<std::int8_t>(attRank, static_cast<std::int8_t>(value));
			break;
			case (ANYTYPE_ATTR_SINT16):
					m_topic.SetAttribute<std::int16_t>(attRank, static_cast<std::int16_t>(value));
			break;
			case (ANYTYPE_ATTR_SINT32):
					m_topic.SetAttribute<std::int32_t>(attRank, static_cast<std::int32_t>(value));
			break;
			case(ANYTYPE_ATTR_SINT64):
					m_topic.SetAttribute<std::int64_t>(attRank, value);
			break;
			default:
				ndsErrorStream(rootNode) << "Introspection failed for attribute: "<< attFromPV <<" attribute type: "<< (int)attType<< std::endl;
				return;
				break;
			}

			if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
				if(m_pub->Publish()!=STATUS_ERROR){
					ndsDebugStream(rootNode) <<"Publishing topic. Triggered by [" << attFromPV << "] attribute."<<m_topic.GetName()<<std::endl;
				}
			}
		}else{
			ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
		}
	}else{
		ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
	}
}

void DeviceNDSSDNPUB::PV_floatAttribute_writer(const timespec& /*timestamp*/, const float& value, const std::string& pvName){

	std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
			uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
			TypeIdentifier_t attType = p_type_def->GetAttributeType(attRank);

    		switch(attType){
    		case(ANYTYPE_ATTR_FLOAT):
					m_topic.SetAttribute<float>(attRank, value);
    		break;
    		case(ANYTYPE_ATTR_DOUBLE):
					m_topic.SetAttribute<double>(attRank, static_cast<double>(value));
    		break;
    		default:
				ndsErrorStream(rootNode) << "Introspection failed for attribute: "<< attFromPV << " attribute type: " << (int)attType << std::endl;
				return;
    			break;
    		}
    		//m_topic.SetAttribute<float>(p_type_def->GetAttributeRank(pvName.c_str()), value);
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			if(m_pub->Publish()!=STATUS_ERROR){
    				ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV <<"] attribute." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute ["<< attFromPV << "] not found."<< std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode)<<"SDN Device is not running"<<std::endl;
    }
}

void DeviceNDSSDNPUB::PV_doubleAttribute_writer(const timespec& /*timestamp*/, const double& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
			uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
			TypeIdentifier_t attType = p_type_def->GetAttributeType(attRank);

    		switch(attType){
    		case(ANYTYPE_ATTR_FLOAT):
					m_topic.SetAttribute<float>(attRank, static_cast<float>(value));
    		break;
    		case(ANYTYPE_ATTR_DOUBLE):
					m_topic.SetAttribute<double>(attRank, value);
    		break;
    		default:
				ndsErrorStream(rootNode) << "Introspection failed for attribute: " << attFromPV << " attribute type: " << (int)attType << std::endl;
				return;
    			break;
    		}
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			if(m_pub->Publish()!=STATUS_ERROR){
    				ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV << "] attribute." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found."<< std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running"<<std::endl;
    }
}

template <typename O, typename I>
void DeviceNDSSDNPUB::convertVectorAndSetAttr(const std::vector<I> &input, uint_t attrMult, uint_t attRank){
	O temp_buff[attrMult];
	for(uint_t i = 0; i < attrMult; i++){
		if(i < input.size()){
			temp_buff[i] = static_cast<O>(input[i]);
		}else{
			temp_buff[i] = static_cast<O>(0);
		}
	}
	m_topic.SetAttribute(attRank, temp_buff);
}

void DeviceNDSSDNPUB::PV_VboolAttribute_writer(const timespec& /*timestamp*/, const std::vector<bool>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

	uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
	//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
	const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
	std::string saMult(aMult);
	uint_t attrMult=std::stoi(saMult);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
        if(it!=m_xmlTopic.end()){
        	/*
        	 * vector<bool> cannot be used here, due to being a specialization of std::vector which may or may
        	 * not store its data in a single contiguous chunk of memory, so it does not implement a data method
        	 * See http://www.cplusplus.com/reference/vector/vector-bool/ for more information
        	 */

    		convertVectorAndSetAttr<bool>(value, attrMult, attRank);

    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV <<"] attribute." << std::endl;
    			if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute ["<< attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::PV_Vuint8Attribute_writer(const timespec& /*timestamp*/, const std::vector<std::uint8_t>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
			uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
			TypeIdentifier_t attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){
    		//case(TypeIdentifier_t::ANYTYPE_ATTR_UINT8):
    		case(ANYTYPE_ATTR_UINT8):
				convertVectorAndSetAttr<std::uint8_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_UINT16):
				convertVectorAndSetAttr<std::uint16_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_UINT32):
				convertVectorAndSetAttr<std::uint32_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_UINT64):
				convertVectorAndSetAttr<std::uint64_t>(value, attrMult, attRank);
    			break;
    		default:
    			ndsErrorStream(rootNode) << "Introspection failed for attribute: " << attFromPV << std::endl;
    			break;
    		}
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
				ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV <<"] attribute." << std::endl;
				if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::PV_Vuint16Attribute_writer(const timespec& /*timestamp*/, const std::vector<std::uint16_t>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
			uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
			TypeIdentifier_t attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){
    		case(ANYTYPE_ATTR_UINT16):
				convertVectorAndSetAttr<std::uint16_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_UINT32):
				convertVectorAndSetAttr<std::uint32_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_UINT64):
				convertVectorAndSetAttr<std::uint64_t>(value, attrMult, attRank);
    			break;
    		default:
    			ndsErrorStream(rootNode) << "Introspection failed for attribute: " << attFromPV << std::endl;
    			break;
    		}

    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
				ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV << "] attribute." << std::endl;
				if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute ["<< attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) <<"Attribute ["<< attFromPV << "] not found."<<std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode)<<"SDN Device is not running"<<std::endl;
    }
}

void DeviceNDSSDNPUB::PV_Vuint32Attribute_writer(const timespec& /*timestamp*/, const std::vector<std::uint32_t>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
			uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
			TypeIdentifier_t attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){

    		case(ANYTYPE_ATTR_UINT32):
				convertVectorAndSetAttr<std::uint32_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_UINT64):
				convertVectorAndSetAttr<std::uint64_t>(value, attrMult, attRank);
    			break;
    		default:
    			ndsErrorStream(rootNode) << "Introspection failed for attribute: " << attFromPV << std::endl;
    			break;
    		}

    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
				ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV <<"] attribute." << std::endl;
				if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::PV_Vint8Attribute_writer(const timespec& /*timestamp*/, const std::vector<std::int8_t>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
			uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
			TypeIdentifier_t attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){
    		case(ANYTYPE_ATTR_SINT8):
				convertVectorAndSetAttr<std::int8_t>(value, attrMult, attRank);
				break;
    		case(ANYTYPE_ATTR_SINT16):
				convertVectorAndSetAttr<std::int16_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_SINT32):
				convertVectorAndSetAttr<std::int32_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_SINT64):
				convertVectorAndSetAttr<std::int64_t>(value, attrMult, attRank);
    			break;
    		default:
    			ndsErrorStream(rootNode) <<"Introspection failed for attribute: "<< attFromPV << std::endl;
    			break;
    		}
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
				ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV <<"] attribute." << std::endl;
				if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute ["<< attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::PV_Vint16Attribute_writer(const timespec& /*timestamp*/, const std::vector<std::int16_t>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
    		TypeIdentifier_t  attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){
    		case(ANYTYPE_ATTR_SINT16):
				convertVectorAndSetAttr<std::int16_t>(value, attrMult, attRank);
				break;
    		case(ANYTYPE_ATTR_SINT32):
				convertVectorAndSetAttr<std::int32_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_SINT64):
				convertVectorAndSetAttr<std::int64_t>(value, attrMult, attRank);
    			break;
    		default:
    			ndsErrorStream(rootNode) << "Introspection failed for attribute: " << attFromPV << std::endl;
    			break;
    		}
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV <<"] attribute." << std::endl;
    			if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::PV_Vint32Attribute_writer(const timespec& /*timestamp*/, const std::vector<std::int32_t>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
    		TypeIdentifier_t  attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){
    		case(ANYTYPE_ATTR_SINT32):
				convertVectorAndSetAttr<std::int32_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_SINT64):
				convertVectorAndSetAttr<std::int64_t>(value, attrMult, attRank);
    			break;
    		default:
    			ndsErrorStream(rootNode) <<"Introspection failed for attribute: "<< attFromPV << std::endl;
    			break;
    		}
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV <<"] attribute."<<std::endl;
				if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::PV_Vint64Attribute_writer(const timespec& /*timestamp*/, const std::vector<std::int64_t>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
    		TypeIdentifier_t  attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){
    		case(ANYTYPE_ATTR_SINT32):
				convertVectorAndSetAttr<std::int32_t>(value, attrMult, attRank);
    			break;
    		case(ANYTYPE_ATTR_SINT64):
				convertVectorAndSetAttr<std::int64_t>(value, attrMult, attRank);
    			break;
    		default:
    			ndsErrorStream(rootNode) <<"Introspection failed for attribute: "<< attFromPV << std::endl;
    			break;
    		}
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) <<"Publishing topic. Triggered by [" <<attFromPV<<"] attribute."<<std::endl;
				if(m_pub->Publish()==RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::PV_VfloatAttribute_writer(const timespec& /*timestamp*/, const std::vector<float>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
    		TypeIdentifier_t  attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){
    		case(ANYTYPE_ATTR_FLOAT):
				convertVectorAndSetAttr<float>(value, attrMult, attRank);
				break;
    		case(ANYTYPE_ATTR_DOUBLE):
				convertVectorAndSetAttr<double>(value, attrMult, attRank);
    			break;
    		default:
    			ndsErrorStream(rootNode) << "Introspection failed for attribute: " << attFromPV << std::endl;
    			break;
    		}
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) <<"Publishing topic. Triggered by [" <<attFromPV<<"] attribute."<<std::endl;
    			    			if(m_pub->Publish()==RetStatusError){
    				ndsErrorStream(rootNode) <<"Error publishing topic from attribute ["<< attFromPV << "]."<<std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) <<"Attribute ["<< attFromPV << "] not found."<<std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode)<<"SDN Device is not running"<<std::endl;
    }
}

void DeviceNDSSDNPUB::PV_VdoubleAttribute_writer(const timespec& /*timestamp*/, const std::vector<double>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		uint_t attRank = p_type_def->GetAttributeRank(attFromPV.c_str());
    		TypeIdentifier_t  attType = p_type_def->GetAttributeType(attRank);
    		//uint_t attrMult = p_type_def->GetAttributeMultiplicity(attRank);
			const char* aMult = p_type_def->GetAttributeMultiplicity(attRank);
			std::string saMult(aMult);
			uint_t attrMult=std::stoi(saMult);

    		switch(attType){
    		case(ANYTYPE_ATTR_FLOAT):
				convertVectorAndSetAttr<float>(value, attrMult, attRank);
				break;
    		case(ANYTYPE_ATTR_DOUBLE):
				convertVectorAndSetAttr<double>(value, attrMult, attRank);
				break;
    		default:
    			ndsErrorStream(rootNode) << "Introspection failed for attribute: " << attFromPV << std::endl;
    			break;
    		}
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV << "] attribute." << std::endl;
    			if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute ["<< attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::PV_stringAttribute_writer(const timespec& /*timestamp*/, const std::string& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		m_topic.SetAttribute<std::string>(p_type_def->GetAttributeRank(attFromPV.c_str()), value);
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) <<"Publishing topic. Triggered by [" << attFromPV <<"] attribute." << std::endl;
    			if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic form attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}
void DeviceNDSSDNPUB::PV_timespecAttribute_writer(const timespec& /*timestamp*/, const timespec& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		m_topic.SetAttribute<timespec>(p_type_def->GetAttributeRank(attFromPV.c_str()), value);
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV << "] attribute." << std::endl;
    			if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }

}

void DeviceNDSSDNPUB::PV_VtimespecAttribute_writer(const timespec& /*timestamp*/, const std::vector<timespec>& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		m_topic.SetAttribute<std::vector<timespec>>(p_type_def->GetAttributeRank(attFromPV.c_str()), value);
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) <<"Publishing topic. Triggered by [" <<attFromPV<<"] attribute."<<std::endl;
    			if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute ["<< attFromPV << "]."<<std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }

}

void DeviceNDSSDNPUB::PV_timestampAttribute_writer(const timespec& /*timestamp*/, const nds::timestamp_t& value, const std::string& pvName){

    std::string attFromPV = pvName.substr (m_deviceName.length()+1);
    sdn::base::AnyType* p_type_def = m_topic.GetTypeDefinition();
    xmlTopic::iterator it =  m_xmlTopic.find(attFromPV);

    if(m_stateMachine.getLocalState()==nds::state_t::running){
    	if(it!=m_xmlTopic.end()){
    		m_topic.SetAttribute<nds::timestamp_t>(p_type_def->GetAttributeRank(attFromPV.c_str()), value);
    		if(it->second.trigger){ //if attribute has trigger to yes the topic will be published
    			ndsDebugStream(rootNode) << "Publishing topic. Triggered by [" << attFromPV <<"] attribute." << std::endl;
    			if(m_pub->Publish() == RetStatusError){
    				ndsErrorStream(rootNode) << "Error publishing topic from attribute [" << attFromPV << "]." << std::endl;
    			}
    		}
    	}else{
    		ndsErrorStream(rootNode) << "Attribute [" << attFromPV << "] not found." << std::endl;
    	}
    }else{
    	ndsWarningStream(rootNode) << "SDN Device is not running" << std::endl;
    }
}

void DeviceNDSSDNPUB::publishingThread(void){

	std::int32_t rate = m_publishingRate_PV.getValue();
	ndsDebugStream(rootNode) <<"Publishing rate: " <<rate<<std::endl;

  while (!m_stopPUB) {
    if(m_pub->Publish()==RetStatusError){
    	ndsErrorStream(rootNode) << "Error publishing topic form SDN Device publishing thread ." << std::endl;
    }
    usleep(rate);
  }
}

void DeviceNDSSDNPUB::initializePVs(void)
{
  std::int32_t initPublishingRate(10000); //10ms

  m_publishingRate_PV.write(rootNode.getTimestamp(), initPublishingRate);
}

RET_STATUS DeviceNDSSDNPUB::createPVs(xmlTopic &inTopic){
	ndsDebugStream(rootNode) << "======================================" << std::endl;
	ndsDebugStream(rootNode) << "Creating PVs..." << std::endl;
	ndsDebugStream(rootNode) << "======================================" << std::endl;

	RET_STATUS status = RetStatusSuccess;
	uint_t rank = 0u;
	//using sdn::base because getters from sdn::base don't have const char* as argument
	sdn::base::AnyType *p_type_def = m_topic.GetTypeDefinition();
	try{
		for(xmlTopic::iterator attFindParam = inTopic.begin(); attFindParam != inTopic.end(); ++attFindParam, ++rank){
			ndsDebugStream(rootNode) << "Attribute found [" << attFindParam->second.attName << "] type: " << attFindParam->second.pvType <<std::endl;

			auto it = mapString2NDSType.find(attFindParam->second.pvType);
			if (it == mapString2NDSType.end()){
				ndsErrorStream(rootNode) << "Not supported PV datatype. (" << attFindParam->second.pvType << ")" << std::endl;
				continue;
			}

			switch(it->second){
			case (nds::dataType_t::dataInt32):
			//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
			{
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				if(bMult == NULL){
					attFindParam->second.nelm = 1;
				}else{
					std::string sbMult(bMult);
					attFindParam->second.nelm = std::stoi(sbMult);
				}
				ndsDebugStream(rootNode) << "Mult: " << p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str()) << " TYPE: " << p_type_def->GetAttributeType(attFindParam->second.attName.c_str()) << std::endl;
				if(attFindParam->second.nelm > 1){
					m_int32Array_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<std::int32_t>>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<std::vector<std::int32_t>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_Vint32Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}else{

					m_int32_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::int32_t>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<std::int32_t>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_int32Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}
			}
				break;
			case (nds::dataType_t::dataInt64):
			//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
			{
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				if(bMult == NULL){
					attFindParam->second.nelm = 1;
				}else{
					std::string sbMult(bMult);
					attFindParam->second.nelm = std::stoi(sbMult);
				}
				ndsDebugStream(rootNode) << "Mult: " << p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str()) << " TYPE: "<<p_type_def->GetAttributeType(attFindParam->second.attName.c_str()) << std::endl;
				if(attFindParam->second.nelm > 1){
					m_int64Array_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<std::int64_t>>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<std::vector<std::int64_t>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_Vint64Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}else{

					m_int64_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::int64_t>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<std::int64_t>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_int64Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}
			}
				break;
			case (nds::dataType_t::dataFloat32):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				if(bMult == NULL){
					attFindParam->second.nelm = 1;
				}else{
					std::string sbMult(bMult);
					attFindParam->second.nelm = std::stoi(sbMult);
				}
				ndsDebugStream(rootNode) <<"Mult: "<< p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str())<<" TYPE: "<<p_type_def->GetAttributeType(attFindParam->second.attName.c_str())<<std::endl;
				if(attFindParam->second.nelm > 1){
					m_floatArray_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<float>>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<std::vector<float>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_VfloatAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}else{
					m_float_PVs.insert(std::pair<std::string,nds::PVDelegateOut<float>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<float>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_floatAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}
			}
				break;
			case (nds::dataType_t::dataFloat64):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				if(bMult == NULL){
					attFindParam->second.nelm = 1;
				}else{
					std::string sbMult(bMult);
					attFindParam->second.nelm = std::stoi(sbMult);
				}
				ndsDebugStream(rootNode) <<"Mult: "<<p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str())<<" TYPE: "<<p_type_def->GetAttributeType(attFindParam->second.attName.c_str())<<std::endl;
				if(attFindParam->second.nelm>1){
					m_doubleArray_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<double>>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<std::vector<double>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_VdoubleAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}else{
					m_double_PVs.insert(std::pair<std::string,nds::PVDelegateOut<double>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<double>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_doubleAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}
			}
				break;
			case (nds::dataType_t::dataBoolArray):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				std::string sbMult(bMult);
				attFindParam->second.nelm = std::stoi(sbMult);
				m_boolArray_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<bool>>>(attFindParam->second.attName,
						rootNode.addChild(nds::PVDelegateOut<std::vector<bool>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_VboolAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
			}
				break;
			case (nds::dataType_t::dataUint8Array):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				std::string sbMult(bMult);
				attFindParam->second.nelm = std::stoi(sbMult);
				m_uint8Array_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<uint8_t>>>(attFindParam->second.attName,
						rootNode.addChild(nds::PVDelegateOut<std::vector<uint8_t>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_Vuint8Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
			}
				break;
			case (nds::dataType_t::dataUint16Array):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				std::string sbMult(bMult);
				attFindParam->second.nelm = std::stoi(sbMult);
				m_uint16Array_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<uint16_t>>>(attFindParam->second.attName,
						rootNode.addChild(nds::PVDelegateOut<std::vector<uint16_t>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_Vuint16Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
			}
				break;
			case (nds::dataType_t::dataUint32Array):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				std::string sbMult(bMult);
				attFindParam->second.nelm = std::stoi(sbMult);
				m_uint32Array_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<uint32_t>>>(attFindParam->second.attName,
						rootNode.addChild(nds::PVDelegateOut<std::vector<uint32_t>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_Vuint32Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
			}
				break;
			case (nds::dataType_t::dataInt8Array):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				std::string sbMult(bMult);
				attFindParam->second.nelm = std::stoi(sbMult);
				m_int8Array_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<int8_t>>>(attFindParam->second.attName,
						rootNode.addChild(nds::PVDelegateOut<std::vector<int8_t>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_Vint8Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
			}
				break;
			case (nds::dataType_t::dataInt16Array):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				std::string sbMult(bMult);
				attFindParam->second.nelm =std::stoi(sbMult);
				m_int16Array_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<int16_t>>>(attFindParam->second.attName,
						rootNode.addChild(nds::PVDelegateOut<std::vector<int16_t>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_Vint16Attribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
			}
				break;
			case (nds::dataType_t::dataString):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				if(bMult == NULL){
					attFindParam->second.nelm = 1;
				}else{
					std::string sbMult(bMult);
					attFindParam->second.nelm = std::stoi(sbMult);
				}
				m_string_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::string>>(attFindParam->second.attName,
						rootNode.addChild(nds::PVDelegateOut<std::string>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_stringAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
			}
				break;
			case (nds::dataType_t::dataTimespec):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				auto sizeSDN = p_type_def->GetAttributeSize(attFindParam->second.attName.c_str());

				attFindParam->second.nelm = sizeSDN/sizeof(timespec);

				ndsDebugStream(rootNode) <<"Mult: "<<std::to_string(attFindParam->second.nelm)<<std::endl;
				if(attFindParam->second.nelm > 1){
					m_timespecArray_PVs.insert(std::pair<std::string,nds::PVDelegateOut<std::vector<timespec>>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<std::vector<timespec>>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_VtimespecAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}else{
					m_timespec_PVs.insert(std::pair<std::string,nds::PVDelegateOut<timespec>>(attFindParam->second.attName,
							rootNode.addChild(nds::PVDelegateOut<timespec>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_timespecAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
				}
			}
				break;
			case (nds::dataType_t::dataTimestamp):
			{
				//attFindParam->second.nelm = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				const char* bMult = p_type_def->GetAttributeMultiplicity(attFindParam->second.attName.c_str());
				if(bMult == NULL){
					attFindParam->second.nelm = 1;
				}else{
					std::string sbMult(bMult);
					attFindParam->second.nelm = std::stoi(sbMult);
				}
				m_timestamp_PVs.insert(std::pair<std::string,nds::PVDelegateOut<nds::timestamp_t>>(attFindParam->second.attName,
						rootNode.addChild(nds::PVDelegateOut<nds::timestamp_t>(attFindParam->second.attName,NULL, std::bind(&DeviceNDSSDNPUB::PV_timestampAttribute_writer,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))));
			}
				break;
			default:
				break;
			}
		}
	} catch (std::logic_error &e){
		m_errorLog.append("ERROR: ");
		m_errorLog.append(e.what());
		m_errorLog.append("\n");
		ndsErrorStream(rootNode) << "ERROR: " << e.what() << std::endl;
		status = RetStatusError;
	}

	ndsDebugStream(rootNode) << "======================================" << std::endl << std::endl;
	return status;
}


RET_STATUS DeviceNDSSDNPUB::subscribePVs(xmlTopic &inTopic)
{
	ndsDebugStream(rootNode) << "======================================" << std::endl;
	ndsDebugStream(rootNode) << "Subscribing PVs..."<< std::endl;
	ndsDebugStream(rootNode) << "======================================" << std::endl;
	RET_STATUS status = RetStatusSuccess;

	for(xmlTopic::iterator attFindParam = inTopic.begin(); attFindParam != inTopic.end(); ++attFindParam){
		ndsDebugStream(rootNode) <<"Trying to subscribe: "<< attFindParam->second.attName << " to: "<< attFindParam->second.pvSource<<" with mult: "<< attFindParam->second.nelm << std::endl;

		try{
			auto it = mapString2NDSType.find(attFindParam->second.pvType);
			if (it == mapString2NDSType.end()){
				status = RetStatusError;
				ndsErrorStream(rootNode) << "Not supported PV datatype. (" << attFindParam->second.pvType << ")" << std::endl;
				continue;
			}

			switch(it->second){
			case (nds::dataType_t::dataInt32):
				if(attFindParam->second.nelm > 1){
					m_int32Array_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}else{
					m_int32_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}
				break;
			case (nds::dataType_t::dataInt64):
				if(attFindParam->second.nelm > 1){
					m_int64Array_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}else{
					m_int64_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}
				break;
			case (nds::dataType_t::dataFloat32):
				if(attFindParam->second.nelm > 1){
					m_floatArray_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}else{
					m_float_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}
				break;
			case (nds::dataType_t::dataFloat64):
				if(attFindParam->second.nelm > 1){
					m_doubleArray_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}else{
					m_double_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}
				break;
			case (nds::dataType_t::dataBoolArray):
				m_boolArray_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				break;
			case (nds::dataType_t::dataUint8Array):
				m_uint8Array_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				break;
			case (nds::dataType_t::dataUint16Array):
				m_uint16Array_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				break;
			case (nds::dataType_t::dataUint32Array):
				m_uint32Array_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				break;

			case (nds::dataType_t::dataInt8Array):
				m_int8Array_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				break;
			case (nds::dataType_t::dataInt16Array):
				m_int16Array_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				break;

			case (nds::dataType_t::dataString):
				m_string_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				break;
			case (nds::dataType_t::dataTimespec):
				if(attFindParam->second.nelm > 1){
					m_timespecArray_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}else{
					m_timespec_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				}
				break;
			case (nds::dataType_t::dataTimestamp):
				m_timestamp_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
				break;
			default:
				break;
			}
		}//End Try
		catch(std::logic_error &e){
			ndsErrorStream(rootNode) << "ERROR: "<< e.what() << std::endl;
			status = RetStatusError;
		}
		catch(std::runtime_error &e){
			ndsErrorStream(rootNode) << "ERROR: "<< e.what() << std::endl;
			status = RetStatusError;
		}
	}

	ndsDebugStream(rootNode) << "======================================" << std::endl << std::endl;

	return status;

}

bool DeviceNDSSDNPUB::unsubscribePVs(ndssdn::xmlTopic &inTopic) {

	ndsDebugStream(rootNode) << "======================================" << std::endl;
	ndsDebugStream(rootNode) << "Unsubscribing PVs..." << std::endl;
	ndsDebugStream(rootNode) << "======================================" << std::endl;
	RET_STATUS status = RetStatusSuccess;

	for(xmlTopic::iterator attFindParam = inTopic.begin(); attFindParam != inTopic.end(); ++attFindParam){
		ndsDebugStream(rootNode) <<"Trying to unsubscribe: " << attFindParam->second.attName<<" to: " << attFindParam->second.pvSource<<" with mult: " << attFindParam->second.nelm << std::endl;

		try{
			auto it = mapString2NDSType.find(attFindParam->second.pvType);
			if (it == mapString2NDSType.end()){
				status = RetStatusError;
				ndsErrorStream(rootNode) << "Not supported PV datatype. (" << attFindParam->second.pvType << ")" << std::endl;
				continue;
			}

			switch(it->second){
			case (nds::dataType_t::dataInt32):
				if(attFindParam->second.nelm > 1){
					m_factory.unsubscribe(m_int32Array_PVs.at(attFindParam->second.attName).getFullName());
				}else{
					m_factory.unsubscribe(m_int32_PVs.at(attFindParam->second.attName).getFullName());
				}
				break;
			case (nds::dataType_t::dataInt64):
				if(attFindParam->second.nelm > 1){
					m_factory.unsubscribe(m_int64Array_PVs.at(attFindParam->second.attName).getFullName());
				}else{
					m_factory.unsubscribe(m_int64_PVs.at(attFindParam->second.attName).getFullName());
				}
				break;
			case (nds::dataType_t::dataFloat32):
				if(attFindParam->second.nelm > 1){
					m_factory.unsubscribe(m_floatArray_PVs.at(attFindParam->second.attName).getFullName());
				}else{
					//m_int32_PVs.at(attFindParam->second.attName).subscribeTo(attFindParam->second.pvSource);
					m_factory.unsubscribe(m_float_PVs.at(attFindParam->second.attName).getFullName());
				}
				break;
			case (nds::dataType_t::dataFloat64):
				if(attFindParam->second.nelm > 1){
					m_factory.unsubscribe(m_doubleArray_PVs.at(attFindParam->second.attName).getFullName());
				}else{
					m_factory.unsubscribe(m_double_PVs.at(attFindParam->second.attName).getFullName());
				}
				break;
			case (nds::dataType_t::dataBoolArray):
				m_factory.unsubscribe(m_boolArray_PVs.at(attFindParam->second.attName).getFullName());
				break;
			case (nds::dataType_t::dataUint8Array):
				m_factory.unsubscribe(m_uint8Array_PVs.at(attFindParam->second.attName).getFullName());
				break;
			case (nds::dataType_t::dataUint16Array):
				m_factory.unsubscribe(m_uint16Array_PVs.at(attFindParam->second.attName).getFullName());
				break;
			case (nds::dataType_t::dataUint32Array):
				m_factory.unsubscribe(m_uint32Array_PVs.at(attFindParam->second.attName).getFullName());
				break;
			case (nds::dataType_t::dataInt8Array):
				m_factory.unsubscribe(m_int8Array_PVs.at(attFindParam->second.attName).getFullName());
				break;
			case (nds::dataType_t::dataInt16Array):
				m_factory.unsubscribe(m_int16Array_PVs.at(attFindParam->second.attName).getFullName());
				break;
			case (nds::dataType_t::dataString):
				m_factory.unsubscribe(m_string_PVs.at(attFindParam->second.attName).getFullName());
				break;
			case (nds::dataType_t::dataTimespec):
				if(attFindParam->second.nelm > 1){
					m_factory.unsubscribe(m_timespecArray_PVs.at(attFindParam->second.attName).getFullName());

				}else{
					m_factory.unsubscribe(m_timespec_PVs.at(attFindParam->second.attName).getFullName());
				}
				break;
			case (nds::dataType_t::dataTimestamp):
				m_factory.unsubscribe(m_timestamp_PVs.at(attFindParam->second.attName).getFullName());
				break;
			default:
				break;
			}
		}//End Try
		catch(std::logic_error &e){
			ndsErrorStream(rootNode) << "ERROR: "<< e.what() << std::endl;
			return RetStatusError;
		}
		catch(std::runtime_error &e){
			ndsErrorStream(rootNode) << "ERROR: "<<e.what() << std::endl;
			return RetStatusError;
		}
	}

	return status;
}

RET_STATUS DeviceNDSSDNPUB::parseXMLFile(const std::string &filename,  ndssdn::xmlTopic& ans)
{

	ndsDebugStream(rootNode) <<"======================================"<<std::endl;
	ndsDebugStream(rootNode) <<"Parsing XML file"<<std::endl;
	ndsDebugStream(rootNode) <<"======================================"<<std::endl;

	std::ifstream input(filename);

	if(!input.good()){
		m_errorLog.append("Error reading input file: "+filename+ "\n");
		//ndsErrorStream(rootNode) <<"Error reading input file: "<<filename<< std::endl;
		return RetStatusError;
	}

	// populate tree structure pt
	using boost::property_tree::ptree;
	ptree pt;
	read_xml(input, pt);

	//xmlTopic ans;
	xmlAttribute_t attribute;

	BOOST_FOREACH(ptree::value_type const&tree, pt.get_child("nds-sdn")) {
		if( tree.first == "topic" ) {
			m_topicName = tree.second.get<std::string>("<xmlattr>.name");
			ndsDebugStream(rootNode) <<"Topic name: "<< m_topicName<<std::endl;
			BOOST_FOREACH(ptree::value_type const&v, pt.get_child("nds-sdn.topic")) {
				if( v.first == "attribute" ) {
					try{
						attribute.attName = v.second.get<std::string>("<xmlattr>.name");
						attribute.pvSource = v.second.get<std::string>("nds-pv.<xmlattr>.name");
						attribute.pvType = v.second.get<std::string>("nds-pv.<xmlattr>.type");
						attribute.trigger = v.second.get<std::string>("trigger_pub")=="yes"?true:false;
						if(attribute.trigger && m_publishingMode!= publishingMode_t::trigger){
							m_publishingMode = publishingMode_t::trigger;
						}
						attribute.nelm = 0;
						ans.insert(std::pair<std::string,xmlAttribute_t>(attribute.attName,attribute));

					}catch (std::exception &e)
					{
						m_errorLog.append("ERROR: ");
						m_errorLog.append(e.what());
						m_errorLog.append("\n");
						return RetStatusError;
					}
					ndsDebugStream(rootNode) <<"Read new attribute ["<<attribute.attName <<"]  with trigger: "<< (attribute.trigger==true?"yes":"no")<<", subscribed to: "<<attribute.pvSource<<
							" of type: "<<attribute.pvType<<std::endl;
				}
			}
		}

	}
	ndsDebugStream(rootNode) <<"Publishing method: "<<(m_publishingMode==publishingMode_t::trigger?"trigger":"periodic")<<std::endl;
	ndsDebugStream(rootNode) <<"======================================"<<std::endl<<std::endl;
	return RetStatusSuccess;
}

NDS_DEFINE_DRIVER(DeviceNDSSDNPUB, DeviceNDSSDNPUB)
