/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn_typedef.h $
* $Id: sdn_typedef.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project		: CODAC Core System
*
* Description	: SDN Software - Communication API sdn_typedef header file.
*
* Author        : Eui Jae LEE, Hyung Gi KIM (Mobiis Co., Ltd)
*
* Co-Author     : Mahajan Kirti
*
* Copyright (c) : 2010-2026 ITER Organization,
*				  CS 90 046
*				  13067 St. Paul-lez-Durance Cedex
*				  France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

/*! \file sdn_typedef.h 
 *  \brief defines SDN typedefs   
 */
#ifndef SDN_TYPEDEF_H
#define SDN_TYPEDEF_H

#include <stdio.h>
#include <stdint.h>


#include "sdn-core/sdn-types.h" // Misc. type definition
#include "sdn-core/sdn-tools.h" // Misc. helper functions
#include "sdn-core/sdn-base.h" // Misc. class definition
#include "sdn-core/sdn-api.h" // SDN core library - API definition
#include "sdn-core/sdn-topic-xml.h"

/** \brief pack the classes to make the code portable between different architecture machines
 */
#define PACKED __attribute__((__packed__))

/* SDN Return Value */
#define	SR_RET			int
#define	SR_OK			0
#define	SR_ERROR		1

// socket error
#define SR_ERR_INVALID_SOCKET	0x101
#define	SR_ERR_SOCKET			0x102
// send/receive error
#define SR_ERR_RECV 			0x201
#define	SR_ERR_SEND				0x202
#define	SR_ERR_PACKET_LOST		0x203
#define	SR_ERR_PACKET_LENGTH	0x204
#define	SR_ERR_DATA_VALIDITY	0x205
#define SR_ERR_PACKET_DELAY		0x206
#define SR_ERR_RECV_TIMEOUT		0x207
#define SR_ERR_NOTIFY                 0x208
// status error
#define SR_ERR_ALREADY			0x301
#define SR_ERR_NOT_INITIALIZED	0x302
// other error
#define SR_ERR_WAIT_TIMEOUT 	0x401
#define SR_ERR_TOPIC_NOT_FOUND  0x1001
#define SR_ERR_NODE_NOT_FOUND   0x1002
#define SR_ERR_OUT_OF_MEMORY  0x1003


/* Topic Definition Module */
#define	MAX_TOPIC_NAME_LEN				40
#define	MAX_TOPIC_DESCRIPTION_LEN		80

#define	MAX_IP_ADDR_LEN				26

#define	MAX_WAIT_LIST_COUNT			128

#define MAX_HOST_NAME_LEN       128
#define MAX_APP_NAME_LEN        128
#define MAX_PUBSUB_DATA_CNT   25


typedef enum
{
  STATUS_ACK_UNKNOWN,
  STATUS_ACK_WAITING,
  STATUS_ACK_TIMEOUT,
  STATUS_ACK_ACKNOWLEDGED,
  STATUS_ACK_TIMEOUT_NO_PEER
} STATUS_ACK;


class sdn_generic_metadata
{
public:
  sdn_generic_metadata()
  {
  };

  /* For backward compatibility with CCSv4.0 */
  sdn_generic_metadata(const char *spec_name, const char *spec_desc,
                       unsigned short spec_version,
                       unsigned short spec_payloadsize)
  {
	sdn::Metadata_t mdata;
	sdn::TopicHelper::InitializeMetadata(mdata, spec_name, spec_payloadsize, spec_version);
	strcpy(mdata.desc, spec_desc);
	mdata.vers = spec_version;
	mdata.size = spec_payloadsize;
	topic = new sdn::Topic(mdata);

    name = spec_name;
    description = spec_desc;
    version = spec_version;
    payloadsize = spec_payloadsize;
    is_event = 0;
  };

  /* New with CCSv4.1 */
  sdn_generic_metadata(const char *spec_name, const char *spec_desc,
                       unsigned short spec_version,
                       unsigned short spec_payloadsize, char spec_isevent)
  {
	  sdn::Metadata_t mdata;
	  sdn::TopicHelper::InitializeMetadata(mdata, spec_name);
	  strcpy(mdata.desc, spec_desc);
	  mdata.vers = spec_version;
	  mdata.size = spec_payloadsize;
	  topic = new sdn::Topic(mdata);

    name = spec_name;
    description = spec_desc;
    version = spec_version;
    payloadsize = spec_payloadsize;
    is_event = spec_isevent;
  };

  /* New with CCSv6.2 */
  sdn_generic_metadata(const char * const metadata_content, char spec_isevent=0)
  {
		topic = sdn::TopicHelper::InstantiateFromXMLContent(metadata_content);
	  
		bool ret = (static_cast<sdn::Topic*>(NULL) != topic);
		if(ret)
		{
			name = topic->GetName();
			description = topic->GetDescription();
			version = topic->GetVersion();
			payloadsize = topic->GetSize();
			is_event = spec_isevent;
		}
  };

  sdn::Topic *getTopic() const
  {
    return topic;
  };							/* New attribute in CCSv6.2 */

  const char *getName() const
  {
    return name;
  };

  const char *getDescription() const
  {
    return description;
  };

  unsigned short getVersion() const
  {
    return version;
  };

  unsigned short getPayloadsize() const
  {
    return payloadsize;
  };

  char isEvent() const
  {
    return is_event;
  };                            /* New attribute in CCSv4.1 */

private:
	sdn::Topic* topic;			 /* New attribute in CCSv6.2 */
  const char *name;
  const char *description;
  unsigned short version;
  unsigned short payloadsize;
  char is_event;                /* New attribute in CCSv4.1 */
};


class PACKED sdn_generic_userdata
{
public:
};

#endif
