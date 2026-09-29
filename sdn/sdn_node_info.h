/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn_node_info.h $
* $Id: sdn_node_info.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project		: CODAC Core System
*
* Description	: SDN Software - Communication API SdnNodeInfo header file.
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

/*! \file sdn_node_info.h 
 *  \brief SDN intialization API
 */
#ifndef SDN_NODE_INFO_H
#define SDN_NODE_INFO_H

#include <list>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "sdn_typedef.h"
#include "sdn_discovery_manager.h"
#include "sdn_node_manager.h"


#define	TIMEOUT_NODE_CONNECTION		5
#define	TIMEOUT_NODE_MESSAGE		3

#define	LOCK(x)		pthread_mutex_lock(&x)
#define	UNLOCK(x)	pthread_mutex_unlock(&x)

///////////////////////////////////////////////////////////////////////////
// environment variables
#define SDN_CONFIG_DIRECTORY_ENVNAME			"SDN_CONFIG_DIRECTORY"
#define SDN_CONFIG_DIRECTORY_DEFAULT			"/etc/opt/codac/sdn"

#define	SDN_INTERFACE_NAME_ENVNAME				"SDN_INTERFACE_NAME"
#define	SDN_HOSTNAME_ENVNAME				"SDN_HOSTNAME"
#define	SDN_APP_NAME_ENVNAME					"SDN_APP_NAME"
#define	SDN_STATISTICS_TIME_WINDOW_ENVNAME		"SDN_STATISTICS_TIME_WINDOW"

#define	DEFAULT_SDN_STATISTICS_TIME_WINDOW		10000
#define	DEFAULT_SDN_INTERFACE_NAME	          "lo"


///////////////////////////////////////////////////////////////////////////
// variable enum types
typedef enum
{
  STATUS_SDN_NOT_READY,
  STATUS_SDN_READY,
} STATUS_SDN;


typedef enum
{
  WAIT_MODE_WAIT_ALL,
  WAIT_MODE_WAIT_ONE_OR_MORE,
} WAIT_MODE;


class PeerInfo
{
/// class PeerInfo 
/// describes peer information which need acknowledge for event topics.
/// It includes hostname, appname, and ipaddr.  
/// It will be used for topicname configuration check.
public:
  std::string m_nameHost;       //!< host name of the controller
  std::string m_nameApp;        //!< application name 
  std::string m_addrIp;         //!< IP address of the node
};


class PubsubInfo
{
/// class PubsubInfo 
/// describes topicname metadata; name, version and length.  
/// It will be used for topicname configuration check.
public:
  char m_isPublisher;           //!< indicates if this object is for publisher or subscriber
    std::string m_nameTopic;    //!< topic name of metadata
    std::string m_descTopic;    //!< topic description of metadata
  unsigned short m_versionTopic;        //!< topic version of metadata
  unsigned short m_sizePayload; //!< payload size of the topic userdata
  char m_isEvent;               //!< indicates if topic is event or not
    std::string m_mcastIpAddr;  //!< multicast IP address
  unsigned short m_mcastPort;   //!< multicast IP port number
    std::list < PeerInfo > m_listPeer;  //!< list of peer node information
};


class ListWaitSubscriber
{
/// class ListWaitSubscriber 
/// describes wait list of subscribers.  
/// It will be used for waitSubscriberMessage().
public:
  ListWaitSubscriber()
  {
    m_waitListCount = 0;
    for (int i = 0; i < MAX_WAIT_LIST_COUNT; i++)
      {
        m_waitSockFDList[i] = 0;
      }
  };

  fd_set m_fdsSrc;
  fd_set m_fdsResult;
  int m_waitSockFDList[MAX_WAIT_LIST_COUNT];
  int m_waitListCount;
};

class SdnNodeInfo
{
/// class SdnNodeInfo
/// describes node's publishing and subscription informations.
/// sdn network IP address, hostname,  process id, list of topicname on which the node is publishing and 
/// list of topicname on which the node is subscribing. 
///
public:
  pthread_mutex_t m_mutexSubList;       //!< mutex used set/get list of subscribers
  pthread_cond_t m_condSubList; //!< condition variable used set/get list of subscribers
    std::list < PubsubInfo > m_listPubsub;      //!< list of publishers and subscribers in this node


    SdnNodeInfo()
  {
    m_pid = (pid_t) - 1;
    m_verMajor_ = 0;
    m_verMinor_ = 0;
    memset(m_hostName_, 0, MAX_HOST_NAME_LEN);
    memset(m_appName_, 0, MAX_APP_NAME_LEN);
    memset(m_sdnIpAddr_, 0, MAX_IP_ADDR_LEN);
    m_statisticsTimeWindow = DEFAULT_SDN_STATISTICS_TIME_WINDOW;
  };

  SR_RET initialize(char *sdn_if_name);
  SR_RET deinitialize();
  int isInitialized();
  SR_RET registerPubsub(const char *topic_name, const char *topic_desc,
                        unsigned short topic_verion,
                        unsigned short payload_size, char is_event,
                        char is_publisher, char *multicast_ip_addr,
                        unsigned short multicast_port_number);
  SR_RET unregisterPubsub(const char *topic_name, char is_publisher);
  SR_RET getMulticastAddress(const char *topic_name, char *multicast_ip,
                             unsigned short *multicast_port);
  char getVersionMajor();
  char getVersionMinor();
  char *getHostName();
  char *getAppName();
  char *getSdnIpAddr();
  unsigned int getStatisticsTimeWindow();
  int getPeerInfo(const char *topic_name, int is_publisher, int is_event,
                  std::list < PeerInfo > &peer_info_list);

  int getPubSubInfo(const char *topic_name, int is_publisher, int is_event,
                    PubsubInfo & pubsubinfo);
  int getPubSubList(const char *topic_name,
                    std::list < PubsubInfo > &list_pubsubinfo);
  SdnDiscoveryManager m_discoveryManager;
  SdnNodeManager m_nodeManager;
  pid_t m_pid;

private:
  char m_verMajor_;
  char m_verMinor_;
  char m_hostName_[MAX_HOST_NAME_LEN];
  char m_appName_[MAX_APP_NAME_LEN];
  char m_sdnIpAddr_[MAX_IP_ADDR_LEN];
  unsigned int m_statisticsTimeWindow;

  pthread_mutex_t m_mutexNode_;
};



extern SdnNodeInfo g_NodeInfo;

extern SR_RET initializeSDN(char *def_conf_file, char *user_conf_file);
extern SR_RET initializeSDN();
extern SR_RET terminateSDN();
extern SR_RET setSDNLogLevel(int log_level);
extern pthread_t getLogThreadId();
extern SR_RET waitSubscriberMessage(ListWaitSubscriber & list_wait,
                                    WAIT_MODE wait_mode, int timeout_usec);

extern void discoveryMngrCallback(const char *str_cmd,
                                  const char *str_topic_name);
extern void nodeMngrCallback(const char *str_cmd, const char *str_topic_name);

#endif
