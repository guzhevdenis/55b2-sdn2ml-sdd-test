//+============================================================================
// $HeadURL: 
// https://svnpub.iter.org/codac/iter/codac/dev/units/m-sdn/trunk/src/main/c++/include/sdn_node_manager.h $
// $Id: sdn_node_manager.h 48548 2014-07-17 11:03:54Z kimh12 $
//
// Project       : CODAC Core System
//
// Description   : SDN Software - Communication API SdnNodeManager header file.
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

/*! \file sdn_node_manager.h 
 *   \brief SDN Node Manager Header File
 */

#ifndef SDN_NODE_MANAGER_H
#define SDN_NODE_MANAGER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>
#include <vector>

#include <fcntl.h>              /* Defines O_* constants */
#include <sys/stat.h>           /* Defines mode constants */
#include <mqueue.h>
#include "sdn_typedef.h"
#include "sdn_topic_list.h"
#include "sdn_discovery_msg.h"
#include "sdn_node_manager_msg.h"


#define MAX_COUNT_ACKLIST_			(10*MAX_PUBSUB_DATA_CNT)

#define QUEUE_T    mqd_t

#define FLAG_EMPTY      0x0
#define FLAG_IN_USE     0x1

class SubStatus
{
////////////////////////////////////////////////////////////////////////////////
/// class SubStatus 
/// describes subscriber information and its acknowledge status of event packet
  friend class EventAckList;

  SubStatus()
  {
    is_acknowledged = 0;
  };

  char is_acknowledged;         //!< indicates if it is acknowledged
  std::string hostname;         //!< host name of the controller
  std::string appname;          //!< application name
};


class EventAckList
{
///////////////////////////////////////////////////////////////////////////////
/// class EventAckList
/// manages subscriber list if the topic is for event
public:
  EventAckList()
  {
    m_indexAckList_ = 0;
    for (int i = 0; i < MAX_COUNT_ACKLIST_; i++)
      {
        m_AckList_[i].is_valid = 0;
        m_AckList_[i].ack_status = STATUS_ACK_UNKNOWN;
        m_AckList_[i].idx_pubsub_data = -1;
      }
  };

  int addNewEvent(unsigned int seq_num, struct timeval sendtime,
                  std::vector < PubsubData > *peerinfo, int idx_pubsubdata);
  int checkTimeoutEvent(char *topic_name, int timeout,
                        struct timeval curtime);
  int processAckMessage(char *topic_name, unsigned int seq_num,
                        char *host_name, char *app_name, int timeout,
                        struct timeval recvtime, int idx_pubsubdata);

private:
  int m_indexAckList_;
  struct
  {
    char is_valid;
    STATUS_ACK ack_status;
    unsigned int seq_num;
    struct timeval sendtime;
      std::list < SubStatus > list_substatus;
    int idx_pubsub_data;
  } m_AckList_[MAX_COUNT_ACKLIST_];

};


/// class SdnNodeManager 
class SdnNodeManager
{
public:
  SdnNodeManager()
  {
    threadNodeManager_ = (pthread_t) 0;
    threadEventAckCb_ = (pthread_t) 0;
    threadSubExceptionCb_ = (pthread_t) 0;
    m_pubsubNodeDataCnt = 0;
    queueMsgNodeMgr_ = (QUEUE_T) - 1;
    queueMsgEventAckCb_ = (QUEUE_T) - 1;
    queueMsgSubExceptionCb_ = (QUEUE_T) - 1;

    memset(m_pubsubNodeData, 0, sizeof(m_pubsubNodeData));
    memset(m_qname_NodeMgr_, 0, MAX_QNAME_LEN);
    memset(m_qname_EventAckCb_, 0, MAX_QNAME_LEN);
    memset(m_qname_SubExceptionCb_, 0, MAX_QNAME_LEN);
  };

  SR_RET initialize();
  SR_RET deinitialize();
  SR_RET sendQuery(const char *p_topic_name);
  SR_RET createMsgQ(int msgqid);
  SR_RET closeMsgQ(int msgqid);
  SR_RET sendMsgQ(int msgqid, NodeMngrMsg msg, unsigned int msg_size);
  SR_RET receiveMsgQ(int msgqid, NodeMngrMsg & msg, unsigned int max_msg_size,
                     int timeout);
  int isThreadAckCallbackRunning();

  struct stPubSubNodeData
  {
    char flag;                  /* 0:empty, 1:in use */
    char topicName[MAX_TOPIC_NAME_LEN];
    char hostName[MAX_HOST_NAME_LEN];
    char appName[MAX_APP_NAME_LEN];
    int topicType;              /* 1:Event Type, 0:None Event Type */
    int usTimeoutEventAck;
    int isPublisher;

    union
    {
      struct stPubData
      {
        uint64_t sentPacketCntTotal;
        uint64_t sentPacketBytesTotal;
        uint64_t sentPacketCnt;
        uint64_t sentPacketBytes;
        void (*m_cbEventAck_) (unsigned int, STATUS_ACK, int);
      } pubData;

      struct stSubData
      {
        int64_t receivedPacketCntTotal;
        int64_t receivedPacketBytesTotal;
        int32_t latencyMinTotal;
        int32_t latencyMaxTotal;
        int32_t latencyAvgTotal;
        int64_t receivedPacketCnt;
        int64_t receivedPacketBytes;
        int32_t latencyMin;
        int32_t latencyMax;
        int64_t latencySum;
        void (*m_cbSubscriberExceptionHndlr) (char *, char *, char *, int);
      } subData;
    } data;
  } m_pubsubNodeData[MAX_PUBSUB_DATA_CNT];

  int m_pubsubNodeDataCnt;

private:
  char m_qname_NodeMgr_[MAX_QNAME_LEN];
  char m_qname_EventAckCb_[MAX_QNAME_LEN];
  char m_qname_SubExceptionCb_[MAX_QNAME_LEN];

  QUEUE_T queueMsgNodeMgr_;
  QUEUE_T queueMsgEventAckCb_;
  QUEUE_T queueMsgSubExceptionCb_;

  pthread_t threadNodeManager_;
  pthread_t threadSubExceptionCb_;
  pthread_t threadEventAckCb_;

  std::vector < PubsubData > m_list_subdata;
  pthread_mutex_t m_mutexPubSubDataList_;

  int processNodeMngrMessage_(NodeMngrMsg msg);
  void SdnStatistics_(unsigned int duration_sec);
  void checkTimeoutEvents_();

  int setSubscriberExceptionCallback_(void *, char *);
  void *getSubscriberExceptionCallback_(char *topic_name);
  int setEventAckCallback_(void *, char *);
  void *getEventAckCallback_(char *topic_name);

  static void *threadNodeMngrFunc_(void *p_param);
  static void cleanupThreadNodeMngr_(void *p_param);
  static void *threadSubExceptionCbFunc_(void *p_param);
  static void cleanupThreadSubExceptionCb_(void *p_param);
  static void *threadEventAckCbFunc_(void *p_param);
  static void cleanupThreadEventAckCb_(void *p_param);

  class EventAckList m_eventAckList_;
};


///////////////////////////////////////////////////////////////////
#endif
