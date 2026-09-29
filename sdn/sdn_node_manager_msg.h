//+============================================================================
// $HeadURL: 
// https://svnpub.iter.org/codac/iter/codac/dev/units/m-sdn/trunk/src/main/c++/include/sdn_node_manager_msg.h $
// $Id: sdnmon_msg.h 48548 2014-07-17 11:03:54Z kimh12 $
//
// Project       : CODAC Core System
//
// Description   : SDN Software - Communication API SdnNodeManager Message header file.
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

/*! \file sdn_node_manager_msg.h 
 *   \brief SDN Node Manager Message Header File
 */

#ifndef SDN_NODE_MANAGER_MSG_H
#define SDN_NODE_MANAGER_MSG_H

#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>

#include "sdn_typedef.h"
#include "sys/time.h"

///////////////////////////////////////////////////////////////////////////
// environment variables
// * SDN_MONITORING_NODE=any_text_if_enable
// * SDN_NODE_MNGR_ADDRESS=multicast_ip_addr:port
//   - the last address of multicast_ip_addr shall be below 254
//   - multicast_ip_addr:port : SDN_MONITOR => SDN_NODE
//   - (multicast_ip_addr+1):(port+1) : SDN_MONITOR <= SDN_NODE


struct NodeMngrMsg
{
  uint32_t idSender;
  uint32_t idMessage;
  int32_t idSubMessage;
  uint32_t noPacketSequence;
  uint16_t isPublisher;         /* 0:subscriber, 1:publisher */
  uint16_t isEventTopic;        /* 0:None Event type, 1:Event type */
  char strTopicName[MAX_TOPIC_NAME_LEN];
  char strHostName[MAX_HOST_NAME_LEN];
  char strAppName[MAX_APP_NAME_LEN];
  struct timeval timeAction;    /* sender time, receive time, etc. */
  void *p_user;
  uint16_t iData1;
};

#define MAX_QNAME_LEN   (MAX_APP_NAME_LEN+32)

#define NODE_MNGR_MSGQ_NAME "NODEMGR"
#define EVENT_ACK_CB_MSGQ_NAME "EVACKCB"
#define SUB_EXCEPT_CB_MSGQ_NAME "SUBEXCB"

#define MSGQ_ID_NODE_MGR   0x1
#define MSGQ_ID_EVENT_ACK_CB   0x2
#define MSGQ_ID_SUB_EXCEPT_CB   0x3


#define NODE_MNGR_MAX_MSG_CNT 10
#define MSGQ_SEND_REPEAT_CNT  100

//#define USE_BLOCKED_MSGQ

#ifdef USE_BLOCKED_MSGQ
#define MSGQ_FLAGS O_CREAT|O_RDWR
#define MSGQ_RECV_WAIT_TIME_FOR_NODEMSG   500   /*micro seconde. 0:no timedwait */
#define MSGQ_RECV_WAIT_TIME_FOR_SUB_EXCPT_CB   500      /*micro seconde. 0:no timedwait */
#define MSGQ_RECV_WAIT_TIME_FOR_EVENT_ACK_CB   500      /*micro seconde. 0:no timedwait */
#else
// if use NON_BLOCK, waittime is for usleep
#define MSGQ_FLAGS O_CREAT|O_RDWR|O_NONBLOCK
#define MSGQ_RECV_WAIT_TIME_FOR_NODEMSG   250   /*micro seconde. 0:no timedwait */
#define MSGQ_RECV_WAIT_TIME_FOR_SUB_EXCPT_CB   250      /*micro seconde. 0:no timedwait */
#define MSGQ_RECV_WAIT_TIME_FOR_EVENT_ACK_CB   250      /*micro seconde. 0:no timedwait */
#endif

#define MSG_SENDER_ID_NODE_MNGR           0x1000
#define MSG_SENDER_ID_DISCOVERY_MNGR 0x1001
#define MSG_SENDER_ID_PUBLISHER           0x1003
#define MSG_SENDER_ID_SUBSCRIBER        0x1004


// message IDs
#define MSG_ID_DISCOVERY_MSG    0x0100
#define MSG_ID_PUBLISHER_START  0x0200
#define MSG_ID_PUBLISHER_STOP   0x0210
#define MSG_ID_PACKET_SENT        0x0300
#define MSG_ID_SET_ACK_TIMEOUT  0x0400
#define MSG_ID_SET_ACK_CALLBACK 0x0500
#define MSG_ID_ADD_ACK_EVENT      0x0510
#define MSG_ID_PACKET_ACK_RECEIVED  0x0600
#define MSG_ID_SUBSCRIBER_START 0x0700
#define MSG_ID_SUBSCRIBER_STOP  0x0710
#define MSG_ID_PACKET_RECEIVED  0x0800
#define MSG_ID_SET_EXCEPTION_CALLBACK 0x0900
#define MSG_ID_UNSET_EXCEPTION_CALLBACK 0x0910
#define MSG_ID_SUBSCRIBER_EXCEPTION    0x0b00
#define MSG_ID_EVENT_ACK_CALLBACK    0x0c00
#define MSG_ID_TERMINATE_THREAD       0xf000

// sub message IDs of discovery message
#define	DISCOVERY_SUB_MSG_ID_ANNOUNCE				0x01
#define	DISCOVERY_SUB_MSG_ID_LEAVE  				      0x02
#define	DISCOVERY_SUB_MSG_ID_QUERY  				   0x03

#define PARTICIPANT_TYPE_SUBSCRIBER  0x0
#define PARTICIPANT_TYPE_PUBLISHER  0x1

#define TOPIC_TYPE_NONEVENT           0x0
#define TOPIC_TYPE_EVENT                0x1
#endif
