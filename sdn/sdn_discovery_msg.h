//+============================================================================
// $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn_discovery_msg.h $
// $Id: sdn_discovery_msg.h 179115 2026-01-09 15:15:20Z kimh19 $
//
// Project       : CODAC Core System
//
// Description   : SDN Software
//
// Author        : Hyung Gi KIM
//
// Copyright (c) : 2010-2026 ITER Organization,
//                 CS 90 046
//                 13067 St. Paul-lez-Durance Cedex
//                 France
//
// This file is part of ITER CODAC software.
// For the terms and conditions of redistribution or use of this software
// refer to the file ITER-LICENSE.TXT located in the top level directory
// of the distribution package.
//
//-============================================================================

/*! \file sdn_discovery_msg.h 
 *   \brief SDN Discovery Message Definition
 */

#ifndef SDN_DISCOVERY_MSG_H
#define SDN_DISCOVERY_MSG_H


#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>


/////////////////////////////////////////////////////////
// message delimeters
// eg1) PVER#1.0\n
// eg2) META#TNAME=TOPIC1;TDESC=This is topic1;TVER=1;PLSIZE=52;;\n
// eg3) SUBLIST#SUB=HOST:FC1,APP:TEST1,,;SUB=HOST:FC2,APP:TEST2,,;;\n
#define	DELIM_KEY_VALUE			          "#"
#define	DELIM_KEY_LINE			          "\n"
#define	DELIM_KEY_END			            "\n\n"
#define	DELIM_PARAM_VALUE		          "="
#define	DELIM_PARAM_LINE		          ";"
#define	DELIM_PARAM_END			          ";;"
#define	DELIM_SUB_PARAM_VALUE	        ":"
#define	DELIM_SUB_PARAM_LINE	        ","
#define	DELIM_SUB_PARAM_END		        ",,"


/////////////////////////////////////////////////////////
// Keys & Values for Message
#define	KEY_PROTOCOL_VERSION			    "PVER"
#define	VALUE_PROTOCOL_VERSION_MAJOR	2
#define	VALUE_PROTOCOL_VERSION_MINOR	0
#define	KEY_MESSAGE_GROUP				      "MGRP"
#define	VALUE_MESSAGE_GROUP_DISCOVERY	"DISCOVERY"


/////////////////////////////////////////////////////////
// Message group: DISCOVERY
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// Keys
#define	KEY_CMD				                "CMD"
#define	KEY_TOPICNAME		              "TNAME"
#define	KEY_MCAST_ADDR                "MCAST_ADDR"
#define	KEY_MCAST_PORT                "MCAST_PORT"
#define	KEY_META			                "META"
#define	KEY_META_TOPICNAME			      "TNAME"
#define	KEY_META_TOPICDESC			      "TDESC"
#define	KEY_META_TOPICVER			        "TVER"
#define	KEY_META_PAYLOADSIZE		      "PLSIZE"
#define	KEY_META_ISEVENT			        "ISEVENT"
#define	KEY_NODE	                    "NODE"
#define	KEY_NODE_HOSTNAME			        "HOST"
#define	KEY_NODE_APPNAME			        "APP"
#define	KEY_NODE_IP				            "IP"
#define	KEY_NODE_ROLE 		            "ROLE"
#define	KEY_PARAM			                "PARAM"
#define	KEY_PARAM_ISRSP   			      "ISRSP"


/////////////////////////////////////////////////////////
// Values

// values for KEY_CMD 
#define	VAL_CMD__ANNOUNCE				      "ANNOUNCE"
#define	VAL_CMD__LEAVE  				      "LEAVE"
#define	VAL_CMD__QUERY  				      "QUERY"

// values for KEY_META_ISEVENT, KEY_PARAM_ISRSP
#define	VAL_BOOL__TRUE			          "1"
#define	VAL_BOOL__FALSE			          "0"

// values for KEY_NODE_ROLE 
#define	VAL_ROLE__PUB				          "PUB"
#define	VAL_ROLE__SUB				          "SUB"


#define VAL_TOPICNAME__ALL_TOPICS     "ALL"


class KeyValue
{
////////////////////////////////////////////////////////////////////////////////
/// class KeyValue
/// String KEY and its VALUE
///     VALUE is set to the other list of KeyValue objects.
public:
  /** \fn void clean()
	 *	\brief Initialize member variables of KeyValue object. 
	 */
  void clean();

  /** \fn int setValue(const char* p_key, const char* p_value)
	 *	\brief Set the KeyValue object with a key and a value. 
	 *	The m_listKeyValue member variable is cleared.
	 *	@param[in] p_key is string pointer for key name.
	 *	@param[in] p_value is string pointer for key value.
	 */
  int setValue(const char *p_key, const char *p_value);

  /** \fn int setList(const char* p_key, std::list<KeyValue>* p_list)
	 *	\brief Set the KeyValue object with a key and its sublist for other KeyValue objects. 
	 *	The m_strValue member variable is cleared.
	 *	@param[in] p_key is string pointer for key name.
	 *	@param[in] p_list is sublist for other KeyValue objects.
	 */
  int setList(const char *p_key, std::list < KeyValue > *p_list);

    std::string m_strKey;
    std::string m_strValue;
    std::list < KeyValue > m_listKeyValue;
};


class ConstructMessage
{
////////////////////////////////////////////////////////////////////////////////
/// class ConstructMessage
///     Makes a text string message from the list of KeyValue objects
///     The text string message follows the format defined in SDNMON communication protocol
public:
  std::string m_strMessage;

  /** \fn char* construct(const char* message_group, std::list<KeyValue>* p_list_kv)
	 *	\brief Makes a text string message from the list of KeyValue objects
	 *	It returns the constructed text string message.
	 *	@param[in] message_group is VALUE_MESSAGE_GROUP_REGISTER or VALUE_MESSAGE_GROUP_UI.
	 *	@param[in] p_list_kv is list of KeyValue objects.
	 */
  char *construct(const char *message_group,
                  std::list < KeyValue > *p_list_kv);
};


class ParseMessage
{
////////////////////////////////////////////////////////////////////////////////
/// class ParseMessage
///     Makes a list of KeyValue objects from the text string message.
///     The text string message follows the format defined in SDNMON communication protocol
public:
  std::list < KeyValue > m_listKeyValue;

  /** \fn std::list<KeyValue>* parse(char* message)
	 *	\brief Makes a list of KeyValue objects from the text string message. 
	 *	Resulted list of key_value is saved to m_listKeyValue member variables,
	 *	and then returns the pointer of m_listKeyValue as a return value
	 *	@param[in] message is the text string message of SDNMON communication protocol.
	 */
  std::list < KeyValue > *parse(char *message);

  /** \fn KeyValue* getKey(std::list<KeyValue>* p_list_kv, const char* key, int index = 0)
	 *	\brief Searches key in the KeyValue list and returns its popinter. 
	 *	Starting index in the list can be specified by optional parameter index.
	 *	@param[in] p_list_kv is the list of KeyValue in which the key is searched.
	 *	@param[in] key is the key name.
	 *	@param[in] index is the start point of searching in the list.
	 */
  // if p_list_kv parameter is NULL, key will be searched in the m_listKeyValue (1st depth)
  KeyValue *getKey(std::list < KeyValue > *p_list_kv, const char *key,
                   int index = 0);
};

#endif
