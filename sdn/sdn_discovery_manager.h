//+============================================================================
// $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn_discovery_manager.h $
// $Id: sdn_discovery_manager.h 179115 2026-01-09 15:15:20Z kimh19 $
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

/*! \file sdn_discovery_manager.h 
 *  \brief Class SdnDiscoveryManager Definition 
 */

#ifndef SDN_DISCOVERY_MANAGER_H
#define SDN_DISCOVERY_MANAGER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <list>
#include <vector>

#include "sdn_typedef.h"
#include "sdn_topic_list.h"
#include "sdn_discovery_msg.h"

#define	SDN_DISCOVERY_INTERFACE_NAME_ENVNAME		"SDN_INTERFACE_NAME" 
#define	DEFAULT_SDN_DISCOVERY_INTERFACE_NAME	  "lo"

///////////////////////////////////////////////////////////////////////////
// environment variables
// * SDN_DISCOVERY_ADDRESS=multicast_ip_addr:port
#define	SDN_DISCOVERY_ADDRESS_ENVNAME	          "SDN_DISCOVERY_ADDRESS"
#define	DEFAULT_SDN_DISCOVERY_ADDRESS	          "239.0.1.2"
#define	DEFAULT_SDN_DISCOVERY_PORT		          10002



// Alarm ID
#define DM_ALARM_PUB_DUPLICATE    1
#define DM_ALARM_METADATA         2
#define DM_ALARM_MCAST_ADDR       3


/// class SdnDiscoveryManager 
/// manages discovery messages and provides functions to send them.
///
/// This class creates a thread to receive and process discovery messages from other SDN nodes.
/// It also provide public member functions to send discovery messages.
class SdnDiscoveryManager
{
public:
  SdnDiscoveryManager()
  {
    m_sockDiscoveryRecv_ = -1;
    m_sockDiscoverySend_ = -1;
    m_idDiscoveryThread_ = (pthread_t) 0;
    m_fnCallback_ = NULL;
    m_fnCallbackAlarm_ = NULL;
    m_hostName_[0] = 0;
    m_appName_[0] = 0;
    m_discoveryInterfaceIpAddr_[0] = 0;
    m_mcastDiscoveryIpAddr_[0] = 0;
    m_bManageAllTopics_ = 0;
    m_mcastDiscoveryPort_ = DEFAULT_SDN_DISCOVERY_PORT;
  };

  SR_RET initialize(int all_topics = 0);
  SR_RET deinitialize();
  SR_RET clear();
  SR_RET sendAnnounce(const char *p_topic_name, const char *p_topic_desc,
                      int topic_verion, int payload_size, int b_event,
                      int b_publisher, const char *mcast_addr,
                      unsigned short mcast_port, int b_response);
  SR_RET sendLeave(const char *p_topic_name, int b_publisher);
  SR_RET sendQuery(const char *p_topic_name = VAL_TOPICNAME__ALL_TOPICS);
  SR_RET getTopicNameList(std::vector < std::string > *p_list_topic_name);
  SR_RET getTopicData(const char *p_topic_name, TopicData * p_topicdata);
  SR_RET getTopicPublisher(const char *p_topic_name, PubsubData * p_pubdata);
  SR_RET getTopicSubscriberList(const char *p_topic_name,
                                std::vector < PubsubData > *p_list_subdata);
  SR_RET
    setCallback(void (*fnCallback)
                (const char *p_cmd, const char *p_topic_name));
  SR_RET
    setCallbackAlarm(void (*fnCallbackAlarm)
                     (int alarm_id, PubsubData * pubsub_recv,
                      Metadata * metadata_recv, PubsubData * pubsub_set,
                      Metadata * metadata_set));

private:
  char m_hostName_[MAX_HOST_NAME_LEN];
  char m_appName_[MAX_APP_NAME_LEN];
  char m_discoveryInterfaceIpAddr_[MAX_IP_ADDR_LEN];
  char m_mcastDiscoveryIpAddr_[MAX_IP_ADDR_LEN];
  int m_bManageAllTopics_;
  std::vector < std::string > m_listManageTopicName_;
  unsigned short m_mcastDiscoveryPort_;
  TopicList m_csTopicList_;

  void (*m_fnCallback_) (const char *p_cmd, const char *p_topic_name);
  void (*m_fnCallbackAlarm_) (int alarm_id, PubsubData * pubsub_recv,
                              Metadata * metadata_recv,
                              PubsubData * pubsub_set,
                              Metadata * metadata_set);

  pthread_mutex_t m_mutexTopicList_;
  pthread_mutex_t m_mutexTopicNameList_;
  int m_sockDiscoveryRecv_;
  int m_sockDiscoverySend_;
  struct sockaddr_in m_sockaddrDiscoverySend_;

  int sendDiscoveryMessage_(char *p_send_message);
  int processDiscoveryMessage_(char *p_msg);

  pthread_t m_idDiscoveryThread_;
  static void *threadDiscovery_(void *p_param);
  static void cleanupThreadDiscovery_(void *p_param);
};

#endif
