/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn_publisher.h $
* $Id: sdn_publisher.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project		: CODAC Core System
*
* Description	: SDN Software - Communication API SdnPublisher header file.
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

/*! \file sdn_publisher.h 
 *  \brief publisher API
 */

#ifndef SDN_PUBLISHER_H
#define SDN_PUBLISHER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "sdn-core/sdn-base.h"
#include "sdn-core/sdn-api.h"
#include "sdn-core/sdn-header.h"

#include "sdn_typedef.h"
#include "sdn_node_info.h"
#include "sdn_log.h"
#include "sdn_node_manager_msg.h"


#define	DEFAULT_TIMEOUT_ACK_US_		1000


/******************************************************************************
 *	Classes for Internal Use
*******************************************************************************/


/*---------------- Template class SdnPublisher declaration --------------------------------*/

/// class SdnPublisher is a template class for SDN topic publishing.
/// The topic_metadata class and topic_userdata class on a topic are necessary as template parameters. 
/// This class holds communication parameters for the topic to be published.
template < class topic_metadata, class topic_userdata > class SdnPublisher
{
public:
  SdnPublisher()
  {
    m_noSend = 0;

    m_pPublisher_ = NULL;
    m_usTimeoutAck_ = 0;

		// for event ack
		m_sockEvent_ = -1;
  }

  // for function test
  int m_noSend;                 //!< packet processing (sequence number increment,...), but inhibit packet send (for packet loss test)

  SR_RET setPublisher(topic_metadata & theTopicMetaData);
  SR_RET setPublisher(topic_metadata & theTopicMetaData, int enable_loopback);
  SR_RET unsetPublisher();
  SR_RET publish(topic_userdata & theTopicUserData, unsigned int *pSeqNumber = NULL);

  // for event topic
  SR_RET setEventAckTimeout(int usec);
  SR_RET getEventAckTimeout(int &usec);
  SR_RET setEventAckCallback(void (*callback_func)
                        (unsigned int seq_num, STATUS_ACK ack_status,
                         int elapsed_usec));
  pthread_t getAckRecvThreadId();

private:
  sdn::Publisher * m_pPublisher_;
  topic_metadata m_csTopicMetaData_;
  int m_usTimeoutAck_;

	// for event ack
  int m_sockEvent_;
  pthread_t m_threadIdAckRecv_;
  static void *threadAckRecv_(void *p_param);
  static void cleanupThreadAckRecv_(void *p_param);
};



/*--------------------publish template function defintion -----------------*/

/*---------------- setPublisher function definition ---------------------------*/
/** \fn SR_RET SdnPublisher<topic_metadata, topic_userdata>::setPublisher(topic_metadata& theTopicMetaData, int enable_loopback)
 *  brief This function needs a topic_metadata class as its parameter.
 *
 *  The topic related information, such as topic name, multicast address, and multicast port are filled from this parameter, 
 *  and socket is created with this information. Registration to the SDN monitor node on the publishing topic is done via ${SDN_INTERFACE_NAME}.
 *
 *  @param[in] theTopicMetaData <topic>_metadata class defined in the topic definition file.
 *  @param[in] enable_loopback Optional parameter that disables/enables loopback of multicast packets.
 *				The default value is 1 to allow for publishers and subscribers to reside on the same physical machine.
 *				May be set to 0 in order to improve performance, bypassing part of the IPv4 OS checks.
 *  @retval SR_OK No error.
 *  @retval SR_ERROR Other errors not specified by error codes.
 *  @retval SR_ERR_ALREADY SdnPublisher object is initialized more than once.
 *  @retval SR_ERR_SOCKET Socket related function returns error.
 *  @retval SR_ERR_NOT_INITIALIZED SDN software is not initialized.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 *
 *  \note After the SdnPublisher object is created, setPublisher()should be called first before any other member functions.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnPublisher < topic_metadata, topic_userdata >::setPublisher(topic_metadata & theTopicMetaData)
{
  RET_STATUS status;
	SR_RET ret;
  NodeMngrMsg sndmsg;
	
  log_debug("<%s> setPublisher called.", theTopicMetaData.getName());

  if (g_NodeInfo.isInitialized() == 0)
    {
      log_error("<%s> g_NodeInfo is not initialized. initializeSDN() first.",
             theTopicMetaData.getName());
      return SR_ERR_NOT_INITIALIZED;
    }

  if (m_pPublisher_ != NULL)
    {
      log_error("<%s> topic is set already to this publisher.",
             theTopicMetaData.getName());
      return SR_ERR_ALREADY;
    }

  /* initialize member variables for repetitive set/unset subscriber */
  m_noSend = 0;

	m_csTopicMetaData_ = theTopicMetaData;

  /* create Publisher object */
  m_pPublisher_ = new (std::nothrow) sdn::Publisher(*(theTopicMetaData.getTopic()));
  if (m_pPublisher_ == NULL)
    {
      log_error("<%s> Failed to create object.", theTopicMetaData.getName());
      return SR_ERR_OUT_OF_MEMORY;
    }

  /* set metadata */
	status = m_pPublisher_->Configure();
	if(status != STATUS_SUCCESS)
	{
      log_error("<%s> m_pPublisher_->Configure() failed.", theTopicMetaData.getName());
			return SR_ERROR;
	}

  /* notification to other nodes     */
  ret = g_NodeInfo.registerPubsub(m_csTopicMetaData_.getName(),
                              m_csTopicMetaData_.getDescription(),
                              (unsigned short) m_csTopicMetaData_.getVersion(),
                              (unsigned short) m_csTopicMetaData_.getPayloadsize(),
                              m_csTopicMetaData_.isEvent(), PARTICIPANT_TYPE_PUBLISHER,
                              (char*)"", 0);
  if (ret != SR_OK)
    {
      log_error("<%s> registerPubsub() failed. ret=0x%x.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null", ret);
      return status;
    }

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_PUBLISHER_START;
  sndmsg.idSender = MSG_SENDER_ID_PUBLISHER;
  strncpy(sndmsg.strTopicName, theTopicMetaData.getName(), MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = theTopicMetaData.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_PUBLISHER;
  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error("<%s> Sending notify message error.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERROR;
    }


  /* Process for event topic */
  if (theTopicMetaData.isEvent())
  {
    setEventAckTimeout(DEFAULT_TIMEOUT_ACK_US_);
    ret = pthread_create(&m_threadIdAckRecv_, NULL, &threadAckRecv_, this);
    if (ret != 0)
      {
        log_error("<%s> pthread_create() failed.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
        m_threadIdAckRecv_ = (pthread_t) 0;
        return SR_ERROR;
      }
  }

  return SR_OK;
}


/** \fn SR_RET SdnPublisher<topic_metadata, topic_userdata>::setPublisher(topic_metadata & theTopicMetaData, int enable_loopback)
 *	\brief Deprecated API.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnPublisher < topic_metadata, topic_userdata >::setPublisher(topic_metadata & theTopicMetaData, int enable_loopback)
{
	log_warning("setPublisher(topic_metadata & theTopicMetaData, int enable_loopback) is deprecated.");
	return setPublisher(theTopicMetaData);
}


/*------------Unset SdnPublisher and close the sockets -----------------------------*/
/** \fn SR_RET SdnPublisher<topic_metadata, topic_userdata>::unsetPublisher()
 *  \brief This function should be called at the end of using SdnPublisher object, or before any further calls to 
 * setPublisher() method, and before terminatedSDN().
 *
 *  This function closes the socket and cleans up the resources used by the SdnPublisher object. Deregistration 
 *  from the SDN monitoring node on the published topic is done.
 *
 *  @param None.
 *  @retval SR_OK No error.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnPublisher < topic_metadata, topic_userdata >::unsetPublisher()
{
  NodeMngrMsg sndmsg;
	SR_RET ret;

  log_debug("<%s> unsetPublisher called.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");

  if (m_pPublisher_ == NULL)
    {
      log_error("<%s> publisher is not initialized.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  /* notification to other nodes     */
  ret =
    g_NodeInfo.unregisterPubsub(m_csTopicMetaData_.getName(), 1);
  if (ret != SR_OK)
    {
      log_error("<%s> registerPubsub() failed. ret=0x%x.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null", ret);
    }

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_PUBLISHER_STOP;
  sndmsg.idSender = MSG_SENDER_ID_PUBLISHER;
  strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_PUBLISHER;
  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error("<%s> Sending notify message error.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
    }

  delete(m_pPublisher_);
  m_pPublisher_ = NULL;

  return SR_OK;
}


/** \fn SR_RET SdnPublisher<topic_metadata, topic_userdata>::publish(topic_userdata& theTopicUserData, unsigned int* pSeqNumber)
 *	\brief Transmit SDN packet on the topic.
 *
 *  The SDN packet is composed of SDN packet header and the user data payload. 
 *  The packet format of the user data payload is defined in the topic_userdata. 
 *  The SDN packet header is added to this payload. The SDN packet header is managed 
 *  in the SdnPublisher object using the internal SdnPacket object.
 *  The optional parameter pSeqNumber is used to get the sequence number of the sent packet, 
 *  which is necessary to check the acknowledgement status of the sent packet when it is event topic.
 *
 *  @param[in]	theTopicUserData <topic>_userdata class defined in the topic definition file.
 *  @param[out]	pSeqNumber Optional parameter which returns the sequence number of sent packet.
 *  @retval SR_OK No error.
 *  @retval SR_ERR_SOKCET Socket related function returns error.
 *  @retval SR_ERR_SEND Socket send function returns 0.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnPublisher < topic_metadata,
  topic_userdata >::publish(topic_userdata & theTopicUserData,
                            unsigned int *pSeqNumber)
{
  NodeMngrMsg sndmsg;
  struct timespec packettime;
  RET_STATUS status;
	sdn::Header_t *p_header;

  if (m_pPublisher_ == NULL)
    {
      log_error("<%s> publisher is not initialized.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

	status = m_pPublisher_->CopyTopicInstance(&theTopicUserData, m_pPublisher_->GetTopicSize());
  if (status != STATUS_SUCCESS)
    {
      log_error("<%s> m_pPublisher_->CopyTopicInstance() failed.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_SEND;
    }

  if (m_noSend)
  {
		p_header = (sdn::Header_t *)m_pPublisher_->GetTopicHeader();
		if (p_header != NULL)
		  p_header->topic_counter++;

    return SR_OK;
  }

  clock_gettime(CLOCK_REALTIME, &packettime);

  if (m_csTopicMetaData_.isEvent())
    {
      memset(&sndmsg, 0, sizeof(sndmsg));
      sndmsg.idMessage = MSG_ID_ADD_ACK_EVENT;
      sndmsg.idSender = MSG_SENDER_ID_PUBLISHER;
      sndmsg.idSubMessage = 0;
      sndmsg.isEventTopic = 1;
      sndmsg.isPublisher = PARTICIPANT_TYPE_PUBLISHER;
      sndmsg.timeAction.tv_sec = packettime.tv_sec;
      sndmsg.timeAction.tv_usec = packettime.tv_nsec / 1000;
      strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
			p_header = (sdn::Header_t *)m_pPublisher_->GetTopicHeader();
		  if (p_header == NULL)
	      sndmsg.noPacketSequence = 0;
			else
	      sndmsg.noPacketSequence = p_header->topic_counter;
			
      if (g_NodeInfo.
          m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                 sizeof(sndmsg)) != SR_OK)
        {
          log_error("<%s> Sending notify message error.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
        }
    }

	status = m_pPublisher_->Publish();
  if (status != STATUS_SUCCESS)
    {
      log_error("<%s> m_pPublisher_->Publish() failed.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_SEND;
    }

	status = m_pPublisher_->DoBackgroundActivity();
  if (status != STATUS_SUCCESS)
    {
      log_error("<%s> m_pPublisher_->DoBackgroundActivity() failed.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERROR;
    }

	p_header = (sdn::Header_t *)m_pPublisher_->GetTopicHeader();
  if (p_header == NULL)
      log_error("<%s> m_pPublisher_->GetTopicHeader() failed.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_PACKET_SENT;
  sndmsg.idSender = MSG_SENDER_ID_PUBLISHER;
  sndmsg.idSubMessage = 0;
  sndmsg.isEventTopic = m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_PUBLISHER;
  sndmsg.timeAction.tv_sec = packettime.tv_sec;
  sndmsg.timeAction.tv_usec = packettime.tv_nsec / 1000;
  sndmsg.iData1 = p_header->topic_size;
  strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
  if (p_header != NULL)
	  sndmsg.noPacketSequence = p_header->topic_counter;
	else
	  sndmsg.noPacketSequence = 0;
  if (g_NodeInfo.
      m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                             sizeof(sndmsg)) != SR_OK)
    {
      log_error("<%s> Sending notify message error.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
    }

  return SR_OK;
}


/** \fn SR_RET SdnPublisher<topic_metadata, topic_userdata>::setEventAckTimeout(int usec)
 *  \brief Sets timeout in micro-seconds for acknowledgement of event.
 *
 *  Timeout for acknowledgement of event is set to 1000 micro-second as default.  
 *  This function is used to change the default timeout.
 *  If the event is not acknowledged before the timeout, the callback function is called if set by setEventAckCallback().
 *
 *  @param[in] usec Timeout in micro-seconds for acknowledgement of event
 *  @retval SR_OK No error.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 *  @retval SR_ERROR Other errors not specified by error codes.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnPublisher < topic_metadata,
  topic_userdata >::setEventAckTimeout(int usec)
{
  NodeMngrMsg sndmsg;

  if (m_pPublisher_ == NULL)
    {
      log_error("<%s> publisher is not initialized.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  m_usTimeoutAck_ = usec;

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_SET_ACK_TIMEOUT;
  sndmsg.idSender = MSG_SENDER_ID_PUBLISHER;
  strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_PUBLISHER;
  sndmsg.p_user = (void *) &m_usTimeoutAck_;
  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error("<%s> Sending notify message error.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_NOTIFY;
    }

  return SR_OK;
}



/** \fn SR_RET SdnPublisher<topic_metadata, topic_userdata>::getEventAckTimeout(int& usec)
 *  \brief Gets timeout in micro-seconds for acknowledgement of event.
 *
 *  This function is used to get the timeout in micro-seconds for acknowledgement of event.
 *
 *  @param[out] usec Timeout in micro-seconds for acknowledgement of event
 *  @retval SR_OK No error.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 *  @retval SR_ERROR Other errors not specified by error codes.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnPublisher < topic_metadata,
  topic_userdata >::getEventAckTimeout(int &usec)
{
  if (m_pPublisher_ == NULL)
    {
      log_error("<%s> publisher is not initialized.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  usec = m_usTimeoutAck_;
  return SR_OK;
}



/** \fn SR_RET SdnPublisher<topic_metadata, topic_userdata>::setEventAckCallback(void (*callback_func)(unsigned int seq_num, STATUS_ACK ack_status, int elapsed_usec))
 *  \brief Set the callback function which should be called when the acknowledgement status is changed.
 *
 *  User is notified by the callback function when the acknowledgement status is changed to STATUS_ACK_TIMEOUT or STATUS_ACK_ACKNOWLEDGED.
 *  If the callback function is not set or set to NULL, the change of acknowledgement is not notified.
 *  seq_num is the sequence number of the event whose status is changed.
 *  ack_status is the current status of the acknowledgement for the seq_num.
 *  elapsed_usec is elapsed time in micro-seconds after the event is sent.
 *
 *  @param[in] callback_func Callback function which should be called when the acknowledgement status is changed.
 *  @retval SR_OK No error.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 *  @retval SR_ERROR Other errors not specified by error codes.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnPublisher < topic_metadata,
  topic_userdata >::setEventAckCallback(void (*callback_func)
                                        (unsigned int seq_num,
                                         STATUS_ACK ack_status,
                                         int elapsed_usec))
{
  NodeMngrMsg sndmsg;

  if (m_pPublisher_ == NULL)
    {
      log_error("<%s> publisher is not initialized.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_SET_ACK_CALLBACK;
  sndmsg.idSender = MSG_SENDER_ID_PUBLISHER;
  strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_PUBLISHER;
  sndmsg.p_user = (void *) callback_func;
  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error("<%s> Sending notify message error.", m_pPublisher_ ? m_pPublisher_->GetTopicName() : "null");
      return SR_ERR_NOTIFY;
    }

  return SR_OK;
}



/** \fn pthread_t SdnPublisher<topic_metadata, topic_userdata>::getAckRecvThreadId()
 *  \brief Gets the thread ID of acknowledgement receive thread.
 *
 *  The thread ID is used to change thread attribute such as scheduling policy and priority.
 *  Acknowledgement receive thread is SdnPublisher internal thread which receives acknowledgements
 *  from subscribers for event topics.
 *
 *  @param None.
 *  @return Thread id of the acknowledgement receive thread.
 */
template < class topic_metadata, class topic_userdata >
  pthread_t SdnPublisher < topic_metadata,
  topic_userdata >::getAckRecvThreadId()
{
  return m_threadIdAckRecv_;
};


/*--- thread to receive event acknowledge from subscribers (only created when the topic is event) ---*/
template < class topic_metadata, class topic_userdata > 
void * SdnPublisher < topic_metadata, topic_userdata >::threadAckRecv_(void *p_param)
{
#define	MAX_RECV_BUF_SIZE			1500
  SR_RET ret;
  int len;
  int offset;
  int ack_len;
  int reuse;
  char *p_host_name;
  char *p_app_name;
  unsigned int seq_num;
  char *p_buf;
  struct sockaddr_in addr;
  SdnPublisher *p_pub = static_cast<SdnPublisher *>(p_param);
  char recv_buf[MAX_RECV_BUF_SIZE];
  NodeMngrMsg sndmsg;

  struct ip_mreq subGroup;

  if (p_pub == NULL)
    {
      log_error("invalid null parameter.");
      return (void *) 1;
    }

  if (p_pub->m_pPublisher_ == NULL)
    {
      log_error("invalid null parameter.");
      return (void *) 1;
    }

  pthread_cleanup_push(p_pub->cleanupThreadAckRecv_, p_pub);

  ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  if (ret != 0)
    log_warning("pthread_setcancelstate(PTHREAD_CANCEL_ENABLE) failed.");

  p_pub->m_sockEvent_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (p_pub->m_sockEvent_ < 0)
    {
      log_error("socket() error.");
      return (void *) 1;
    }

  /* Enable SO_REUSEADDR to allow multiple instances of this */
  /* application to receive copies of the multicast datagrams. */
  reuse = 1;
  if (setsockopt
      (p_pub->m_sockEvent_, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse,
       sizeof(reuse)) < 0)
    {
      log_error("Setting SO_REUSEADDR error.");
      close(p_pub->m_sockEvent_);
      p_pub->m_sockEvent_ = -1;
      return (void *) 1;
    }

  sdn::Metadata_t mdata; 
	sdn::TopicHelper::InitializeMetadata(mdata);
  sstrncpy(mdata.name, (char*)p_pub->m_csTopicMetaData_.getName(), STRING_MAX_LENGTH);
	sdn::TopicHelper::GenerateMCastAddress(mdata);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(mdata.mcast_port + 1);
  addr.sin_addr.s_addr = inet_addr(mdata.mcast_group);

  log_info("Mcast addr for event_ack [%s:%d].", mdata.mcast_group, mdata.mcast_port + 1);

  if (bind(p_pub->m_sockEvent_, (struct sockaddr *) &addr, sizeof(addr)) != 0)
    {
      log_error("Binding datagram socket error.");
      close(p_pub->m_sockEvent_);
      p_pub->m_sockEvent_ = -1;
      return (void *) 1;
    }

  /* Join the multicast group on the local interface */
  /* Note that this IP_ADD_MEMBERSHIP option must be */
  /* called for each local interface over which the multicast */
  /* datagrams are to be received. */

  subGroup.imr_multiaddr.s_addr = inet_addr(mdata.mcast_group);
  subGroup.imr_interface.s_addr = inet_addr(g_NodeInfo.getSdnIpAddr());

  if (setsockopt
      (p_pub->m_sockEvent_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &subGroup,
       sizeof(subGroup)) < 0)
    {
      log_error("Adding multicast group error.");
      close(p_pub->m_sockEvent_);
      p_pub->m_sockEvent_ = -1;
      return (void *) SR_ERR_SOCKET;
    }


  // recv loop
  // message format: "seq_num(4byte, little-endian) + host_name\0 + app_name\0"
  while (1)
    {
      len = recvfrom(p_pub->m_sockEvent_, recv_buf, MAX_RECV_BUF_SIZE, 0,
                     (struct sockaddr *) NULL, (socklen_t *) NULL);

      if (len <= 0)             // socket closed or error
        {
          if ((errno == EINTR) || (errno == EAGAIN))
            {
              log_warning("recvfrom() returned errno %d.", errno);
              continue;
            }

          log_error("recvfrom() returned %d.", len);
          break;
        }
      else
        {
          offset = 0;
          while (len > 0)
            {
              p_buf = recv_buf + offset;

              seq_num = *(unsigned int *) p_buf;
              p_host_name = p_buf + 4;
              p_app_name = p_host_name + strlen(p_host_name) + 1;
              ack_len =
                (int) ((unsigned long) p_app_name + strlen(p_app_name) + 1 -
                       (unsigned long) p_buf);
              if (ack_len > len)
                {
                  log_warning("invalid ack message(msglen[%d], ack_len[%d], seqnum[%d], hostnam[%s], appname[%s].",
                         len, ack_len, seq_num, p_host_name, p_app_name);
                  break;
                }

              memset(&sndmsg, 0, sizeof(sndmsg));
              gettimeofday(&sndmsg.timeAction, NULL);
              sndmsg.idMessage = MSG_ID_PACKET_ACK_RECEIVED;
              sndmsg.idSender = MSG_SENDER_ID_PUBLISHER;
              sndmsg.isEventTopic = 1;
              sndmsg.isPublisher = PARTICIPANT_TYPE_PUBLISHER;
              sndmsg.noPacketSequence = seq_num;
              strncpy(sndmsg.strTopicName, p_pub->m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
              strncpy(sndmsg.strHostName, p_host_name, MAX_HOST_NAME_LEN);
              strncpy(sndmsg.strAppName, p_app_name, MAX_APP_NAME_LEN);

              if (g_NodeInfo.
                  m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg, sizeof(sndmsg)) != SR_OK)
                {
                  log_error("Sending notify message error.");
                }

              if (len > ack_len)
                {
                  log_info("message contains more than a ack message(len[%d], ack_len[%d].", len, ack_len);
                }

              offset += ack_len;
              len -= ack_len;
            }
        }
    }

  pthread_cleanup_pop(1);

  return (void *) 0;
}



template < class topic_metadata, class topic_userdata > 
void SdnPublisher < topic_metadata, topic_userdata >::cleanupThreadAckRecv_(void *p_param)
{
  SdnPublisher *p_pub = static_cast<SdnPublisher *>(p_param);

  UNLOCK(g_NodeInfo.m_mutexSubList);

  if (p_pub == NULL)
    {
      log_error("invalid null parameter.");
      return;
    }

  if (p_pub->m_pPublisher_ == NULL)
    {
      log_error("invalid null parameter.");
      return;
    }

  if (p_pub->m_sockEvent_ >= 0)
    {
      struct ip_mreq subGroup;

		  sdn::Metadata_t mdata; 
			sdn::TopicHelper::InitializeMetadata(mdata);
		  sstrncpy(mdata.name, (char*)p_pub->m_csTopicMetaData_.getName(), STRING_MAX_LENGTH);
			sdn::TopicHelper::GenerateMCastAddress(mdata);

      subGroup.imr_multiaddr.s_addr = inet_addr(mdata.mcast_group);
      subGroup.imr_interface.s_addr = inet_addr(g_NodeInfo.getSdnIpAddr());

      if (setsockopt
          (p_pub->m_sockEvent_, IPPROTO_IP, IP_DROP_MEMBERSHIP,
           (char *) &subGroup, sizeof(subGroup)) < 0)
        {
          log_info("unsubscribe multicast group error. maybe it is not a membership now.");
        }

      close(p_pub->m_sockEvent_);
      p_pub->m_sockEvent_ = -1;
    }

  p_pub->m_threadIdAckRecv_ = (pthread_t) 0;

  return;
}

#endif
