/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdnapi.h $
* $Id: sdnapi.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project		: CODAC Core System
*
* Description	: SDN Software - Communication API SDNAPI header file.
*
* Author        : Eui Jae LEE, Hyung Gi KIM (Mobiis Co., Ltd)
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

/*! \file sdnapi.h 
 *  \brief SDN API Header File
 *
 * [ Version History ]
 * CCS v4.0
 *	. release first version of SDN API
 * CCS v4.1
 *	. New Function
 *		- Registration to SDN monitoring node (new)
 *	. New API
 *		- SR_RET initializeSDN(char* def_conf_file, char* user_conf_file)
 *		- SR_RET setSDNStatusCallback(void (*fnCallbackSDNStatus)(int is_ready))
 *		- SR_RET unsetSDNStatusCallback()
 *		- int isSDNStatusReady()
 *		- pthread_t getLogThreadId();
 *		- SR_RET Publisher::setEventAckTimeout(int usec)
 *		- SR_RET Publisher::getEventAckTimeout(int& usec)
 *		- SR_RET Publisher::getEventAckStatus(unsigned int seq_num, STATUS_ACK& ack_status)
 *		- SR_RET Publisher::setEventAckCallback(void (*callback_func)(unsigned int seq_num, STATUS_ACK ack_status, int elapsed_usec))
 *		- pthread_t Publisher::getAckRecvThreadId()
 *		- pthread_t Subscriber::getAsyncRecvThreadId()
 *	. Changes of API
 *		- Subscriber::setReceiveCallback(SR_RET (*fnCallbackReceive)(topic_userdata &, SR_RET)) is changed to
 *			Subscriber::setReceiveCallback(SR_RET (*fnCallbackReceive)(Subscriber<topic_metadata, topic_userdata>*, SR_RET))
 *			* Old API is obsolete, and it returns SR_ERROR for CCS v4.1. Old API will be removed from CCS v5.0
 *		- Subscriber::getPacketSequenceNumber(unsigned short& seqNumber) is changed to 
 *			Subscriber::getPacketSequenceNumber(unsigned int& seqNumber)
 *		- Subscriber::receive(topic_userdata& theTopicUserData) is changed to
 *			Subscriber::receive(topic_userdata& theTopicUserData, int timeout_usec=0)
 *		- Publisher::publish(topic_userdata& theTopicUserData) is changed to
 *			Publisher::publish(topic_userdata& theTopicUserData, unsigned int* pSeqNumber=NULL)
 *		- Publisher::setPublisher(topic_metadata& theTopicMetaData) is changed to
 *			Publisher::setPublisher(topic_metadata& theTopicMetaData, int enable_loopback=0) 
 * CCS v4.2
 *	. New API
 *		- SR_RET initializeSDN()
 *		- SR_RET waitSubscriberMessage(ListWaitSubscriber &list_wait, WAIT_MODE wait_mode, int timeout_usec)
 *		- SR_RET Subscriber::addToWaitList(ListWaitSubscriber &list_wait)
 *		- SR_RET Subscriber::removeFromWaitList(ListWaitSubscriber &list_wait)
 *		- SR_RET Subscriber::checkTopicAvailable(ListWaitSubscriber &list_wait, int &available)
 *	. Deprecated API (will be removed in the next release)
 *		- SR_RET initializeSDN(char* if_name)
 *		- int isSDNStatusReady()
 *		- SR_RET setSDNStatusCallback(void (*fnCallbackSDNStatus)(int is_ready))
 *		- SR_RET unsetSDNStatusCallback()
 * CCS v5.0
 *	. Deprecated API (will be removed in the next release)
 *		- SR_RET Publisher::getEventAckStatus(unsigned int seq_num, STATUS_ACK & ack_status)
 * CCS v6.0
 *	. New API
 *		- SR_RET initializeSDN()
 *  . Removed API
 *		- SR_RET initializeSDN(char* if_name)
 *		- int isSDNStatusReady()
 *		- SR_RET setSDNStatusCallback(void (*fnCallbackSDNStatus)(int is_ready))
 *		- SR_RET unsetSDNStatusCallback()
 *		- SR_RET Publisher::getEventAckStatus(unsigned int seq_num, STATUS_ACK & ack_status)
 *		- SR_RET setPrintfPacketData(int enable_print)
 *	. Deprecated API (will be removed in the next release)
 *		- SR_RET initializeSDN(char *def_conf_file, char *user_conf_file)
 *		- SR_RET setSDNLogLevel(int log_level)
 *		- pthread_t getLogThreadId()
 *		- Publisher::setPublisher(topic_metadata& theTopicMetaData, int enable_loopback)
 */

#ifndef SDN_SDNAPI_H
#define SDN_SDNAPI_H

#include "sdn_node_info.h"
#include "sdn_publisher.h"
#include "sdn_subscriber.h"

#endif
