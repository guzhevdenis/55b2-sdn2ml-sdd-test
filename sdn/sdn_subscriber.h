/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn_subscriber.h $
* $Id: sdn_subscriber.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project		: CODAC Core System
*
* Description	: SDN Software - Communication API SdnSubscriber header file.
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

/*! \file sdn_subscriber.h 
 *    \brief subscriber API
 */

#ifndef SDN_SUBSCRIBER_H
#define SDN_SUBSCRIBER_H

#include "sdn-core/sdn-base.h"
#include "sdn-core/sdn-api.h"
#include "sdn-core/sdn-header.h"

#include "sdn_typedef.h"
#include "sdn_node_info.h"
#include "sdn_log.h"
#include "sdn_node_manager_msg.h"



/*---------------- Template class SdnSubscriber declaration --------------------------------*/

/// class SdnSubscriber is a template class for SDN topic subscription.
/// The topic_metadata class and topic_userdata class on a topic are necessary as template parameters. 
/// This class holds communication parameters for the topic to be subscribed.

template < class topic_metadata, class topic_userdata > class SdnSubscriber
{
public:
  SdnSubscriber()
  {
    m_nextPacketSequence_ = 1;
    m_lostPackets_ = 0;
    m_bAsyncRecv_ = 0;
    m_fnCallbackReceive_ = NULL;
    m_fnCallbackException = NULL;
    m_threadIdAsyncRecv_ = (pthread_t) 0;
    m_bCheckDelay_ = 0;
    m_pSubscriber_ = NULL;
    m_bCheckDelay_ = 0;
    m_usDelayThreshold_ = 0;
    m_usLatency_ = 0;
    m_retAsyncRecv_ = SR_OK;

		// for event ack
		m_sockEvent_ = -1;
  };

  SR_RET setSubscriber(topic_metadata & theTopicMetaData);
  SR_RET unsetSubscriber();
  SR_RET receive(topic_userdata & theTopicUserData, int timeout_usec = -1);
  SR_RET getAsyncUserData(topic_userdata & theTopicUserData);
  SR_RET
    setReceiveCallback(SR_RET(*fnCallbackReceive)
                       (SdnSubscriber < topic_metadata, topic_userdata > *,
                        SR_RET));
  SR_RET unsetReceiveCallback();
  SR_RET setExceptionHndCallback(SR_RET(*fn) (char *, char *, char *, int));
  SR_RET unsetExceptionHndCallback();
  SR_RET reportSubscriberException(SdnSubscriber * p_sub, int reason);
  SR_RET enableCheckDelay(int usec);
  SR_RET disableCheckDelay();

  SR_RET getLatency(int &latency);
  SR_RET getPacketSequenceNumber(unsigned int &seqNumber);
  SR_RET getPacketSenderTime(struct timespec &senderTime);

  SR_RET addToWaitList(ListWaitSubscriber & list_wait);
  SR_RET removeFromWaitList(ListWaitSubscriber & list_wait);
  SR_RET checkTopicAvailable(ListWaitSubscriber & list_wait, int &available);

  pthread_t getAsyncRecvThreadId();

private:
  sdn::Subscriber * m_pSubscriber_;
  topic_metadata m_csTopicMetaData_;

  // async receive
  int m_bAsyncRecv_;
  SR_RET m_retAsyncRecv_;
  pthread_t m_threadIdAsyncRecv_;
  SR_RET(*m_fnCallbackReceive_) (SdnSubscriber < topic_metadata,
                                 topic_userdata > *p_subscriber,
                                 SR_RET err_code);

  SR_RET(*m_fnCallbackException) (char *, char *, char *, int);


  // check packet sequence number 
  unsigned int m_nextPacketSequence_;
  unsigned int m_lostPackets_;

  // check packet delay
  int m_bCheckDelay_;           // for packet delay check enable
  int m_usDelayThreshold_;      // used as a delay threshold when m_bCheckDelay_ is true
  int m_usLatency_;             // save latency for the last packet when m_bCheckDelay_ is true

	// for event_ack
  int m_sockEvent_;
  int m_lenSendAck_;
  char m_bufSendAck_[256];

  /*------------------- Internal function ----------------------------------------------*/

  static void *threadAsyncReceive_(void *p_param);      // async receive thread
};


/*---------- Template function declaration exposed as API --------------------------------*/

/*------------------- setsubscriber----------------------------------*/
/** \fn  SR_RET SdnSubscriber<topic_metadata, topic_userdata>::setSubscriber(topic_metadata& theTopicMetaData)
 *  \brief This function needs a topic_metadata class as its parameter.
 *
 *  The topic related information, such as topic name, multicast address, and multicast port are filled from this parameter, 
 *  and socket is created with this information. Registration to the SDN monitoring node on the subscribing topic is done 
 *  vid ${SDN_INTERFACE_NAME}.
 *
 *  @param[in] theTopicMetaData <topic>_metadata class defined int the topic definition file.
 *  @retval SR_OK No error.
 *  @retval SR_ERROR Other errors not specified by error codes.
 *  @retval SR_ERR_ALREADY SdnSubscriber object is initialized more than once.
 *  @retval SR_ERR_SOCKET Socket related function returns error.
 *  @retval SR_ERR_NOT_INITIALIZED SDN software is not initialized.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 *
 *  \note After the SdnSubscriber object is created, setSubscriber() should be called first before any other member functions.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::setSubscriber(topic_metadata & theTopicMetaData)
{
  RET_STATUS status;
  NodeMngrMsg sndmsg;
	SR_RET ret;
	
  log_debug(">%s< setSubscriber called.", theTopicMetaData.getName());

  if (g_NodeInfo.isInitialized() == 0)
    {
      log_error(">%s< g_NodeInof is not initialized. initializeSDN() first.",
             theTopicMetaData.getName());
      return SR_ERR_NOT_INITIALIZED;
    }

  if (m_pSubscriber_ != NULL)
    {
      log_error(">%s< topic is set already to this subscriber.",
             theTopicMetaData.getName());
      return SR_ERR_ALREADY;
    }


  /* initialize member variables for repetitive set/unset subscriber */
  m_nextPacketSequence_ = 1;
  m_lostPackets_ = 0;
  m_bAsyncRecv_ = 0;
  m_fnCallbackReceive_ = NULL;
  m_fnCallbackException = NULL;
  m_threadIdAsyncRecv_ = (pthread_t) 0;
  m_bCheckDelay_ = 0;

	m_csTopicMetaData_ = theTopicMetaData;
	
  /* create Subscriber object */
  m_pSubscriber_ = new (std::nothrow) sdn::Subscriber(*(theTopicMetaData.getTopic()));
  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< Failed to create object.", theTopicMetaData.getName());
      return SR_ERR_OUT_OF_MEMORY;
    }

  /* set metadata */
	status = m_pSubscriber_->Configure();
	if(status != STATUS_SUCCESS)
	{
      log_error(">%s< m_pSubscriber_->Configure() failed.", theTopicMetaData.getName());
			return SR_ERROR;
	}

  /* notification to other nodes     */
  ret = g_NodeInfo.registerPubsub(m_csTopicMetaData_.getName(),
                              m_csTopicMetaData_.getDescription(),
                              (unsigned short) m_csTopicMetaData_.getVersion(),
                              (unsigned short) m_csTopicMetaData_.getPayloadsize(),
                              m_csTopicMetaData_.isEvent(), PARTICIPANT_TYPE_SUBSCRIBER,
                              (char*)"", 0);
  if (ret != SR_OK)
    {
      log_error(">%s< registerPubsub() failed. ret=0x%x.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null", ret);
      return status;
    }

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_SUBSCRIBER_START;
  sndmsg.idSender = MSG_SENDER_ID_SUBSCRIBER;
  strncpy(sndmsg.strTopicName, theTopicMetaData.getName(), MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = theTopicMetaData.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_SUBSCRIBER;
  if (g_NodeInfo. m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg, sizeof(sndmsg)) != SR_OK)
  {
    log_error(">%s< Sending notify message error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
    return SR_ERROR;
  }

	// for event ack
  if (theTopicMetaData.isEvent())
  {
		sdn::Metadata_t mdata; 
	  struct sockaddr_in mcast_addr;
	  struct in_addr localInterface;

    // initialize send ack buffer
    // message format: "seq_num(4byte, little-endian) + host_name\0 + app_name\0"
    m_lenSendAck_ = 4 + strlen(g_NodeInfo.getHostName()) + 1 + strlen(g_NodeInfo.getAppName()) + 1;
    if (m_lenSendAck_ > (int) sizeof(m_bufSendAck_))
    {
      log_error(">%s< invalid buffer size. host name or app name is too long.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }

    memset(m_bufSendAck_, 0, m_lenSendAck_);
    strcpy(m_bufSendAck_ + 4, g_NodeInfo.getHostName());
    strcpy(m_bufSendAck_ + 4 + strlen(g_NodeInfo.getHostName()) + 1, g_NodeInfo.getAppName());

    /* Open datagram socket for publisher */
    m_sockEvent_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sockEvent_ < 0)
    {
      log_error(">%s< Opening datagram socket for acknowledge error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }
    else
    {
      log_debug(">%s< Opening the datagram socket for acknowledge...OK.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
    }

    /* Set local interface for outbound multicast datagrams. */
    /* The IP address specified must be associated with a local, */
    /* multicast capable interface. */
    /* *** setsockopt(IP_MULTICAST_IF) should be called before connect() */
    localInterface.s_addr = inet_addr(g_NodeInfo.getSdnIpAddr());
    if (setsockopt(m_sockEvent_, IPPROTO_IP, IP_MULTICAST_IF, (char *) &localInterface, sizeof(localInterface)) < 0)
    {
      log_error(">%s< Setting local interface for acknowledge error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }
    else
    {
      log_debug(">%s< Setting the local interface for acknowledge...OK.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
    }

    /* Connect to multicast group - Unclear why socket needs to be 'bound' AND 'connected' */

		sdn::TopicHelper::InitializeMetadata(mdata);
		sstrncpy(mdata.name, (char*)m_csTopicMetaData_.getName(), STRING_MAX_LENGTH);
		sdn::TopicHelper::GenerateMCastAddress(mdata);

    memset((char *) &mcast_addr, 0, sizeof(mcast_addr));
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_addr.s_addr = inet_addr(mdata.mcast_group);
    mcast_addr.sin_port = htons(mdata.mcast_port + 1);

    log_info(">%s< Mcast addr for event_ack [%s:%d].", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null", mdata.mcast_group, mdata.mcast_port + 1);

    /* Bind socket to multicast address */
    if (connect(m_sockEvent_, (struct sockaddr *) &mcast_addr, sizeof(mcast_addr)) != 0)
    {
      log_error(">%s< connect() for acknowledgement failed '%d - %s'\n", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null", errno, strerror(errno));
      return SR_ERROR;
    }
  }

  return SR_OK;
}


/*------------------- unsetsubscriber----------------------------------*/
/** \fn  SR_RET SdnSubscriber<topic_metadata, topic_userdata>::unsetSubscriber()
 *  \brief This function should be called at the end of using SdnSubscriber object, or before any further calls to 
 *  setPublisher() method, and before terminateSDN().
 *
 *  This function releases system resources used in the SdnSubscriber object. Deregistration from the SDN monitoring
 *  node on the subscribed topic is done.
 *
 *  @param None.
 *  @retval SR_OK No error.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata, topic_userdata >::unsetSubscriber()
{
  NodeMngrMsg sndmsg;
	SR_RET ret;

  log_debug(">%s< unsetSubscriber called.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");

  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }
	
  /* notification to other nodes     */
  ret =
    g_NodeInfo.unregisterPubsub(m_csTopicMetaData_.getName(), 0);
  if (ret != SR_OK)
    {
      log_error(">%s< registerPubsub() failed. ret=0x%x.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null", ret);
    }

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_SUBSCRIBER_STOP;
  sndmsg.idSender = MSG_SENDER_ID_SUBSCRIBER;
  strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_SUBSCRIBER;
  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error(">%s< Sending notify message error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
    }

  delete(m_pSubscriber_);
  m_pSubscriber_ = NULL;

  return SR_OK;
}


/* -----------------------receive ------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::receive(topic_userdata& theTopicUserData, int timeout_usec)
 *  \brief  Receive SDN packet on the topic.
 *
 *  The SDN packet is composed of SDN packet header and the user data payload.
 *  The SDN packet header is managed in the SdnSubscriber object using the internal SdnPacket object, and then removed.
 *  Only user data payload is delivered to the user. The packet format of the user data payload is defined in the topic_userdata.
 *  The timeout_usec is optional parameter to define timeout in micro-seconds. If timeout paramter is not given or less than 0,
 *  this function blocks indefinitely until any data packet is received. Otherwise, it returns SR_ERR_RECV_TIMEOUT in case no data
 *  packet is received before the timeout expires.
 *  In case of packet loss or packet delay, it is notified to user through exception callback which may be registerd by 
 *  setExceptionHndCallback() API. If other errors are not detected, the return value will be SR_OK for packet loss and packet
 *  delay. If the received packet is not the valid one, which includes different SDN API version or different topic version, it will be
 *  discarded and then continue to receive other packets.
 *
 *  @param[out] theTopicUserData <topic>_userdata class defined in the topic definition file 
 *  @param[in] timeout_usec Optional parameter to define timeout in micro-seconds
 *  @retval SR_OK No error.
 *  @retval SR_ERR_SOKCET Socket related function returns error.
 *  @retval SR_ERR_RECV Error occurred in the base class.
 *  @retval SR_ERR_RECV_TIMEOUT Socket receive function timeout.
 *  @retval SR_ERR_NOT_INITIALIZED SDN software is not initialized.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::receive(topic_userdata & theTopicUserData,
                            int timeout_usec)
{
  unsigned int seqnumber;
  fd_set fds;
  struct timeval timeout;
  SR_RET ret;
  RET_STATUS status;
	sdn::Header_t *p_header = NULL;
  NodeMngrMsg sndmsg;


  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  /* Wait until valid packet is received. */
  if (timeout_usec >= 0)
    {
      timeout.tv_sec = timeout_usec / 1000000;
      timeout.tv_usec = timeout_usec % 1000000;

      FD_ZERO(&fds);
      FD_SET(m_pSubscriber_->GetSocket(), &fds);
      ret =
        select(m_pSubscriber_->GetSocket() + 1, &fds, NULL, NULL,
               &timeout);
      if (ret < 0)
        {
          log_error(">%s< select() error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
          return SR_ERR_SOCKET;
        }
      else if (ret == 0)
        {
          return SR_ERR_RECV_TIMEOUT;
        }
    }

  status = m_pSubscriber_->Receive();
  if (status != STATUS_SUCCESS)
    {
      log_error(">%s< m_pSubscriber_->Receive() failed.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_RECV;
    }

	p_header = (sdn::Header_t *)m_pSubscriber_->GetTopicHeader();
  if (p_header == NULL)
    {
      log_error(">%s< m_pSubscriber_->GetTopicHeader() failed.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_RECV;
    }

	// for event ack
	if (m_csTopicMetaData_.isEvent())
  {
		int ack_len;
		
    // send event acknowledge packet
    // message format: "seq_num(4byte, little-endian) + host_name\0 + app_name\0"
    if (m_sockEvent_ >= 0)
      {
				m_bufSendAck_[0] = p_header->topic_counter & 0xff;
				m_bufSendAck_[1] = (p_header->topic_counter>>8) & 0xff;
				m_bufSendAck_[2] = (p_header->topic_counter>>16) & 0xff;
				m_bufSendAck_[3] = (p_header->topic_counter>>24) & 0xff;
        ack_len = send(m_sockEvent_, m_bufSendAck_, m_lenSendAck_, 0);
        if (ack_len != m_lenSendAck_)
          {
            log_error(">%s< sending acknowledgement error in size. req=%d, sent=%d\n", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null", m_lenSendAck_, ack_len);
          }
      }
  }

  /* Check packet contents and call exceptionCallback for any exceptional case */
	status = m_pSubscriber_->CopyTopicInstance(&theTopicUserData, sizeof(theTopicUserData));
  if (status != STATUS_SUCCESS)
    {
      log_error(">%s< m_pSubscriber_->CopyTopicInstance() failed.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }

  if (p_header == NULL)
    {
      log_error(">%s< Header not found.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }
	
	seqnumber = p_header->topic_counter;

  if (seqnumber == 0)           // check if seqnumber is 0 to ignore publisher restart
    {
      m_nextPacketSequence_ = seqnumber + 1;
    }
  else if (m_nextPacketSequence_ == 1)  // check if m_nextPacketSequence_ is 1(start value) to ignore subscriber restart
    {
      m_nextPacketSequence_ = seqnumber + 1;
    }
  else if (seqnumber != m_nextPacketSequence_)
    {
      log_warning(">%s< Packet lost for sequence number '%d'. received=%d", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null",
               m_nextPacketSequence_, seqnumber);
      m_lostPackets_++;
      m_nextPacketSequence_ = seqnumber + 1;
      reportSubscriberException(this, SR_ERR_PACKET_LOST);
    }
  else
    {
      m_nextPacketSequence_++;
    }

  m_usLatency_ = (p_header->recv_time - p_header->send_time) / 1000;
  if (m_bCheckDelay_)
    {
      if ((m_usDelayThreshold_ > 0) && (m_usLatency_ > m_usDelayThreshold_))
        {
          reportSubscriberException(this, SR_ERR_PACKET_DELAY);
        }
    }

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_PACKET_RECEIVED;
  sndmsg.idSender = MSG_SENDER_ID_SUBSCRIBER;
  strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_SUBSCRIBER;
  sndmsg.idSubMessage = m_usLatency_;
  sndmsg.iData1 = p_header->topic_size;
  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error(">%s< Sending notify message error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
    }

//#define	DEBUG_PRINT	1

#ifdef DEBUG_PRINT
	printf("\n<== Packet received\n");
	printf("[Metadata]\n");
	printf("Name: %s\n", m_csTopicMetaData_.getName());
	printf("Description: %s\n", m_csTopicMetaData_.getDescription());
	printf("Size: %d\n", m_csTopicMetaData_.getPayloadsize());
	printf("Version: %d\n", m_csTopicMetaData_.getVersion());
	printf("IsEvent: %d\n", m_csTopicMetaData_.isEvent());

	printf("[Packet header]\n");
	printf("sdn_version: %s\n", (char*)p_header->header_uid);
	printf("header_size=%d\n", p_header->header_size);
	printf("topic_uid=0x%x\n", p_header->topic_uid);
	printf("topic_version=0x%x\n", p_header->topic_version);
	printf("topic_size=%d\n", p_header->topic_size);
	printf("topic_counter=%ld\n", p_header->topic_counter);
	printf("send_time=%ld\n", p_header->send_time);
	printf("recv_time=%ld\n", p_header->recv_time);
	printf("delay=%ld\n", p_header->recv_time - p_header->send_time);
#endif

  return SR_OK;
}


/* -----------------------getAsyncUserData------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::getAsyncUserData(topic_userdata& theTopicUserData)
 *  \brief Get the SDN user data on the topic which is received asynchronously.
 *
 *  This function is valid only when the mode is set to asynchronous receive mode by setReceiveCallback(). 
 *  The most recently received user data payload is delivered. The packet format of the user data payload is 
 *  defined in the topic_userdata. 
 *
 *  @param[out] theTopicUserData <topic>_userdata class defined in the topic definition file 
 *  @retval SR_OK No error.
 *  @retval SR_ERR_SOKCET Socket related function returns error.
 *  @retval SR_ERR_RECV Error occurred in the base class.
 *  @retval SR_ERR_RECV_TIMEOUT Socket receive function timeout.
 *  @retval SR_ERR_NOT_INITIALIZED SDN software is not initialized.
 *
 *  \note This function should be called as soon as the callback function set by the setReceiveCallback() is called. 
 *  The getAsyncUserData() retrieves user data payload from the most recently received SDN packet, 
 *  and the SDN packets which were not taken before this packet will be lost.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::getAsyncUserData(topic_userdata & theTopicUserData)
{
  RET_STATUS status;

  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

	status = m_pSubscriber_->CopyTopicInstance(&theTopicUserData, sizeof(theTopicUserData));
  if (status != STATUS_SUCCESS)
    {
      log_error(">%s< m_pSubscriber_->CopyTopicInstance() failed.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }

  return m_retAsyncRecv_;
}


/* -----------------------setReceiveCallback------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::setReceiveCallback(SR_RET (*fnCallbackReceive)(SdnSubscriber<topic_metadata, topic_userdata>*, SR_RET))
 *  \brief Set SdnSubscriber object to asynchronous receive mode. 
 *
 *  When the SdnSubscriber object is in asynchronous receive mode, user shall retrieve the user data payload 
 *  using fnCallbackReceive() or getAsyncUserData(), instead of receive().
 *
 *  @param[in] fnCallbackReceive Callback function called when the SDN packet on the corresponding topic is available.
 *  @retval SR_OK No error.
 *  @retval SR_ERR_SOKCET Socket related function returns error.
 *  @retval SR_ERR_ALREADY Callback function is set more than once.
 *  @retval SR_ERR_INVALID_SOCKET Socket is not created.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::setReceiveCallback(SR_RET(*fnCallbackReceive)
                                       (SdnSubscriber < topic_metadata,
                                        topic_userdata > *, SR_RET))
{
  int i_ret;

  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  if (m_bAsyncRecv_)
    {
      log_error(">%s< Subscriber is already running in async receive mode. unsetReceiveCallback() first.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_ALREADY;
    }

  m_bAsyncRecv_ = 1;
  m_fnCallbackReceive_ = fnCallbackReceive;

  log_debug(">%s< Spawning async receive thread.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
  i_ret = pthread_create((pthread_t *) & m_threadIdAsyncRecv_, NULL, &threadAsyncReceive_, this);
  if (i_ret != 0)
    {
      log_error(">%s< Create threadAsyncReceive_ thread error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      m_bAsyncRecv_ = 0;
      m_fnCallbackReceive_ = NULL;
      m_threadIdAsyncRecv_ = (pthread_t) 0;
      return SR_ERROR;
    }

  return SR_OK;
}


/* -----------------------setExceptionHndCallback------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::setExceptionHndCallback(SR_RET (*fnCallbackReceive)(char *, char *, char *, int))
 *  \brief Set exception handler callback function of SdnSubscriber. 
 *
 *  If packet loss or packet delay is detected in receive() API, the exception handler callback function, if set, is executed to notify
 *  the user of this event. The parameters of the callback function are topic_name, host_name, app_name and error_code in order.
 *
 *  @param[in] fnCallbackException Pointer of callback function which returns SR_RET and has paramters of (char *topic_name,
 *			char *host_name, char *app_name, int err_code)
 *
 *  @retval SR_OK No error.
 *  @retval SR_ERR_NOTIFY Message queue error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::setExceptionHndCallback(SR_RET(*fnCallbackException)
                                            (char *, char *, char *, int))
{
  NodeMngrMsg sndmsg;

  m_fnCallbackException = fnCallbackException;
  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_SET_EXCEPTION_CALLBACK;
  sndmsg.idSender = MSG_SENDER_ID_SUBSCRIBER;
  strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(),
          MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_SUBSCRIBER;

  sndmsg.p_user = (void *) fnCallbackException;

  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error(">%s< Sending notify message error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOTIFY;
    }

  return SR_OK;
}


/* -----------------------unsetExceptionHndCallback------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::unsetExceptionHndCallback()
 *  \brief Remove exception handler callback function of SdnSubscriber. 
 *
 *  @param None.
 *
 *  @retval SR_OK No error.
 *  @retval SR_ERR_NOTIFY Message queue error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::unsetExceptionHndCallback()
{
  NodeMngrMsg sndmsg;

  m_fnCallbackException = NULL;
  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_UNSET_EXCEPTION_CALLBACK;
  sndmsg.idSender = MSG_SENDER_ID_SUBSCRIBER;
  strncpy(sndmsg.strTopicName, m_csTopicMetaData_.getName(),
          MAX_TOPIC_NAME_LEN);
  sndmsg.isEventTopic = m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_SUBSCRIBER;

  sndmsg.p_user = (void *) NULL;
  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error(">%s< Sending notify message error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOTIFY;
    }

  return SR_OK;
}


/* -----------------------reportSubscriberException------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::reportSubscriberException(SdnSubscriber * p_sub, int reason)
 *  \brief Report subscriber exception
 *
 *  @param[in] p_sub
 *  @param[in] reason
 *  @retval SR_OK No error.
 *  @retval SR_ERR_NOTIFY Message queue error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::reportSubscriberException(SdnSubscriber * p_sub,
                                              int reason)
{
  NodeMngrMsg sndmsg;

  if (p_sub == NULL)
    {
      log_info(">%s< Invalid p_sub parameter. Reporting is cancelled.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }

  if (m_fnCallbackException == NULL)
    {
      log_info(">%s< Callback function for subscriber exception is not defined. Reporting is cancelled.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }

  memset(&sndmsg, 0, sizeof(sndmsg));
  sndmsg.idMessage = MSG_ID_SUBSCRIBER_EXCEPTION;
  sndmsg.idSender = MSG_SENDER_ID_SUBSCRIBER;
  strncpy(sndmsg.strTopicName, p_sub->m_csTopicMetaData_.getName(),
          MAX_TOPIC_NAME_LEN);
  strncpy(sndmsg.strHostName, g_NodeInfo.getHostName(), MAX_HOST_NAME_LEN);
  strncpy(sndmsg.strAppName, g_NodeInfo.getAppName(), MAX_APP_NAME_LEN);
  sndmsg.isEventTopic = p_sub->m_csTopicMetaData_.isEvent();
  sndmsg.isPublisher = PARTICIPANT_TYPE_SUBSCRIBER;
  sndmsg.idSubMessage = reason;
  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_SUB_EXCEPT_CB, sndmsg,
                                        sizeof(sndmsg)) != SR_OK)
    {
      log_error(">%s< Sending notify message error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOTIFY;
    }

  return SR_OK;
}

/* -----------------------unsetReceiveCallback------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::unsetReceiveCallback()
 *  \brief Set SdnSubscriber object to synchronous receive mode.
 *
 *  When the SdnSubscriber object is in synchronous receive mode, which is default mode when the SdnSubscriber object is created, 
 *  user shall retrieve the user data payload using receive() function.
 *
 *  @param None.
 *  @retval SR_OK No error.
 *  @retval SR_ERROR Other errors not specified by error codes.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::unsetReceiveCallback()
{
  int i_ret;

  if (!m_bAsyncRecv_)
    {
      log_error(">%s< unsetReceiveCallback() error. Subscriber is not running in async receive mode.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERROR;
    }

  if (m_threadIdAsyncRecv_ != (pthread_t) 0)
    {
      i_ret = pthread_cancel(m_threadIdAsyncRecv_);
      if (i_ret != 0)
        log_warning(">%s< unsetReceiveCallback: pthread_cancel() error.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");

      m_threadIdAsyncRecv_ = (pthread_t) 0;
    }

  m_bAsyncRecv_ = 0;

  return SR_OK;
}


/* -----------------------enableCheckDelay ------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::enableCheckDelay(int usec)
 *  \brief Enable packet delay inspection.
 *
 *  In this mode, receive() function compares time flag of each packet which is set to the system time of the publisher 
 *  to the system time of the subscriber. If the time difference is greater than the usec parameter, 
 *  receive() function returns with error code SR_ERR_PACKET_DELAY.
 *
 *  @param[in] usec Packet delay threshold in micro-seconds.
 *  @retval SR_OK No error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::enableCheckDelay(int usec)
{
  m_bCheckDelay_ = 1;
  m_usDelayThreshold_ = usec;

  return SR_OK;
}


/* -----------------------disableCheckDelay ------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::disableCheckDelay()
 *  \brief Disable packet delay inspection 
 *
 *  @param None.
 *  @retval SR_OK No error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata, topic_userdata >::disableCheckDelay()
{
  m_bCheckDelay_ = 0;

  return SR_OK;
}


/* -----------------------getLatency ------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::getLatency(int& latency)
 *  \brief Get latency between send to receive.
 *
 *  This function is valid only when delay check is enabled by enableCheckDelay()
 *  @param[out] latency	 latency for the last packet  
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::getLatency(int &latency)
{
  latency = m_usLatency_;

  if (!m_bCheckDelay_)
    return SR_ERROR;

  return SR_OK;
}



/* -----------------------getPacketSequenceNumber------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::getPacketSequenceNumber(unsigned int& seqNumber)
 *  \brief Get the sequence number of SDN packet header.
 *
 *  @param[out] seqNumber Sequence number of the last SDN packet header.
 *  @retval SR_OK No error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::getPacketSequenceNumber(unsigned int &seqNumber)
{
	sdn::Header_t *p_header;

  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

	p_header = (sdn::Header_t *)m_pSubscriber_->GetTopicHeader();
  if (p_header == NULL)
    {
      log_error(">%s< m_pSubscriber_->GetTopicHeader() failed.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_RECV;
    }

	seqNumber = p_header->topic_counter;
	
  return SR_OK;
}


/* -----------------------getPacketSenderTime------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::getPacketSenderTime(struct timespec& senderTime)
 *  \brief Get the sendertime of SDN packet header.
 *
 *  The sendertime is set to system time of the publisher when the packet is published.
 *
 *  @param[out] senderTime Sendertime of the last SDN packet header.
 *  @retval SR_OK No error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::getPacketSenderTime(struct timespec & senderTime)
{
	sdn::Header_t *p_header;

  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

	p_header = (sdn::Header_t *)m_pSubscriber_->GetTopicHeader();
  if (p_header == NULL)
    {
      log_error(">%s< m_pSubscriber_->GetTopicHeader() failed.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_RECV;
    }

	senderTime.tv_sec = p_header->send_time / 1000000000L;
	senderTime.tv_nsec = p_header->send_time % 1000000000L;
	
  return SR_OK;
}


/* -----------------------addToWaitList------------------------------*/
/** \fn SR_RET SR_RET SdnSubscriber<topic_metadata, topic_userdata>::addToWaitList(ListWaitSubscriber &list_wait)
 *  \brief Add this subscriber to the wait list.
 *
 *  The wait list is used by waitSubscriberMessage() API.
 *  @param[in,out] list_wait Wait list of subscribers.
 *  @retval SR_OK No error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::addToWaitList(ListWaitSubscriber & list_wait)
{
  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  list_wait.m_waitSockFDList[list_wait.m_waitListCount] =
    m_pSubscriber_->GetSocket();
  list_wait.m_waitListCount++;

  return SR_OK;
}


/* -----------------------removeFromWaitList------------------------------*/
/** \fn SR_RET SR_RET SdnSubscriber<topic_metadata, topic_userdata>::removeFromWaitList(ListWaitSubscriber &list_wait)
 *  \brief Remove this subscriber from the wait list.
 *
 *  The wait list is used by waitSubscriberMessage() API.
 *  @param[in,out] list_wait Wait list of subscribers.
 *  @retval SR_OK No error.
 *  @retval SR_ERROR Other errors not specified by error codes.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::removeFromWaitList(ListWaitSubscriber & list_wait)
{
  int i, j;

  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  if (list_wait.m_waitListCount <= 0)
    return SR_ERROR;

  for (i = 0; i < list_wait.m_waitListCount; i++)
    {
      if (list_wait.m_waitSockFDList[i] == m_pSubscriber_->GetSocket())
        {
          FD_CLR(list_wait.m_waitSockFDList[i], &list_wait.m_fdsSrc);

          for (j = i; j < list_wait.m_waitListCount - 1; j++)
            {
              list_wait.m_waitSockFDList[j] =
                list_wait.m_waitSockFDList[j + 1];
            }

          list_wait.m_waitSockFDList[j] = 0;
          list_wait.m_waitListCount--;

          return SR_OK;
        }
    }

  return SR_ERROR;
}


/* -----------------------checkTopicAvailable------------------------------*/
/** \fn SR_RET SdnSubscriber<topic_metadata, topic_userdata>::checkTopicAvailable(ListWaitSubscriber &list_wait, int &available)
 *  \brief Check if message is available for this subscriber.
 *
 *  It sets parameter available to 1 if topic data is available for this subscriber, 0 otherwise.
 *  The wait list is used by waitSubscriberMessage() API.
 *
 *  @param[in] list_wait Wait list of subscribers.
 *  @param[out] available 1 of topic data is available, 0 otherwise.
 *  @retval SR_OK No error.
 */
template < class topic_metadata, class topic_userdata >
  SR_RET SdnSubscriber < topic_metadata,
  topic_userdata >::checkTopicAvailable(ListWaitSubscriber & list_wait,
                                        int &available)
{
  if (m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.", m_pSubscriber_ ? m_pSubscriber_->GetTopicName() : "null");
      return SR_ERR_NOT_INITIALIZED;
    }

  if (FD_ISSET(m_pSubscriber_->GetSocket(), &list_wait.m_fdsResult))
    available = 1;
  else
    available = 0;

  return SR_OK;
}


/** \fn pthread_t SdnSubscriber<topic_metadata, topic_userdata>::getAsyncRecvThreadId()
 *  \brief Gets the threadID of asynchronous data receive thread.
 *
 *  The threadID is used to change thread attribute such as scheduling policy and priority.
 *  Asynchronous data receive thread is SdnPublisher internal thread which receives 
 *  topic data from publisher asynchronously.
 *  Asynchronous receive is enabled by setReceiveCallback() API
 *
 *  @param None.
 *  @return Thread id of asynchronous data receive thread.
 */
template < class topic_metadata, class topic_userdata >
  pthread_t SdnSubscriber < topic_metadata,
  topic_userdata >::getAsyncRecvThreadId()
{
  return m_threadIdAsyncRecv_;
};



/*------------------- Internal Functions ----------------------------------*/

/*---------- asynchronous receive thread created by setReceiveCallback() -----------------------*/
template < class topic_metadata, class topic_userdata >
  void *SdnSubscriber < topic_metadata,
  topic_userdata >::threadAsyncReceive_(void *p_param)
{
  unsigned int seqnumber;
  RET_STATUS status;
	sdn::Header_t *p_header;
  NodeMngrMsg sndmsg;

  SdnSubscriber < topic_metadata, topic_userdata > *p_subscriber =
    static_cast < SdnSubscriber < topic_metadata,
    topic_userdata > *>(p_param);

  if (p_subscriber == NULL)
    {
      log_error("parameter null is not vaild.");
      return (void *) SR_ERR_NOT_INITIALIZED;
    }

  if (p_subscriber->m_pSubscriber_ == NULL)
    {
      log_error(">%s< subscriber is not initialized.",
             p_subscriber->m_csTopicMetaData_.getName());
      return (void *) SR_ERR_NOT_INITIALIZED;
    }

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  while (p_subscriber->m_bAsyncRecv_
         && (p_subscriber->m_pSubscriber_ != NULL))
    {
      status = p_subscriber->m_pSubscriber_->Receive();
      if (status != STATUS_SUCCESS)
        {
          log_error(">%s< m_pSubscriber_->Receive() failed.", p_subscriber->m_csTopicMetaData_.getName());

          p_subscriber->m_retAsyncRecv_ = SR_ERR_RECV;
		      if (p_subscriber->m_fnCallbackReceive_ != NULL)
			      p_subscriber->m_fnCallbackReceive_(p_subscriber, p_subscriber->m_retAsyncRecv_);

					continue;
        }

      p_subscriber->m_retAsyncRecv_ = SR_OK;

			p_header = (sdn::Header_t *)p_subscriber->m_pSubscriber_->GetTopicHeader();
		  if (p_header == NULL)
		    {
		      log_error(">%s< m_pSubscriber_->GetTopicHeader() failed.",
             p_subscriber->m_csTopicMetaData_.getName());
		      continue;
		    }

			// for event ack
			if (p_subscriber->m_csTopicMetaData_.isEvent())
	    {
				int ack_len;
				
	      // send event acknowledge packet
	      // message format: "seq_num(4byte, little-endian) + host_name\0 + app_name\0"
	      if (p_subscriber->m_sockEvent_ >= 0)
	        {
						p_subscriber->m_bufSendAck_[0] = p_header->topic_counter & 0xff;
						p_subscriber->m_bufSendAck_[1] = (p_header->topic_counter>>8) & 0xff;
						p_subscriber->m_bufSendAck_[2] = (p_header->topic_counter>>16) & 0xff;
						p_subscriber->m_bufSendAck_[3] = (p_header->topic_counter>>24) & 0xff;
	          ack_len = send(p_subscriber->m_sockEvent_, p_subscriber->m_bufSendAck_, p_subscriber->m_lenSendAck_, 0);
	          if (ack_len != p_subscriber->m_lenSendAck_)
	            {
	              log_error(">%s< sending acknowledgement error in size. req=%d, sent=%d\n", 
	              p_subscriber->m_csTopicMetaData_.getName(), p_subscriber->m_lenSendAck_, ack_len);
	            }
	        }
	    }

      /* Check packet contents and call exceptionCallback for any exceptional case */
			seqnumber = p_header->topic_counter;

      if (seqnumber == 0)       // check if seqnumber is 0 to ignore publisher restart
        {
          p_subscriber->m_nextPacketSequence_ = seqnumber + 1;
        }
      else if (p_subscriber->m_nextPacketSequence_ == 1)        // check if m_nextPacketSequence_ is 1(start value) to ignore subscriber restart
        {
          p_subscriber->m_nextPacketSequence_ = seqnumber + 1;
        }
      else if (seqnumber != p_subscriber->m_nextPacketSequence_)
        {
          log_warning(">%s< Packet lost for sequence number '%d'.",
                 p_subscriber->m_csTopicMetaData_.getName(),
                 p_subscriber->m_nextPacketSequence_);
          p_subscriber->m_lostPackets_++;
          p_subscriber->m_nextPacketSequence_ = seqnumber + 1;
          p_subscriber->reportSubscriberException(p_subscriber,
                                                  SR_ERR_PACKET_LOST);
        }
      else
        {
          p_subscriber->m_nextPacketSequence_++;
        }

		  p_subscriber->m_usLatency_ = (p_header->recv_time - p_header->send_time) / 1000;
		  if (p_subscriber->m_bCheckDelay_)
		    {
		      if ((p_subscriber->m_usDelayThreshold_ > 0) && (p_subscriber->m_usLatency_ > p_subscriber->m_usDelayThreshold_))
		        {
		          p_subscriber->reportSubscriberException(p_subscriber, SR_ERR_PACKET_DELAY);
		        }
		    }


      if (p_subscriber->m_fnCallbackReceive_ == NULL)
        {
          log_error(">%s< m_fnCallbackReceive_ pointer is NULL.",
                 p_subscriber->m_csTopicMetaData_.getName());
          break;
        }

      p_subscriber->m_fnCallbackReceive_(p_subscriber,
                                         p_subscriber->m_retAsyncRecv_);

		  memset(&sndmsg, 0, sizeof(sndmsg));
		  sndmsg.idMessage = MSG_ID_PACKET_RECEIVED;
		  sndmsg.idSender = MSG_SENDER_ID_SUBSCRIBER;
		  strncpy(sndmsg.strTopicName, p_subscriber->m_csTopicMetaData_.getName(), MAX_TOPIC_NAME_LEN);
		  sndmsg.isEventTopic = p_subscriber->m_csTopicMetaData_.isEvent();
		  sndmsg.isPublisher = PARTICIPANT_TYPE_SUBSCRIBER;
		  sndmsg.idSubMessage = p_subscriber->m_usLatency_;
		  sndmsg.iData1 = p_header->topic_size;
		  if (g_NodeInfo.m_nodeManager.sendMsgQ(MSGQ_ID_NODE_MGR, sndmsg,
		                                        sizeof(sndmsg)) != SR_OK)
		    {
		      log_error(">%s< Sending notify message error.",
                 p_subscriber->m_csTopicMetaData_.getName());
		    }
    }

  p_subscriber->m_threadIdAsyncRecv_ = (pthread_t) 0;

  return (void *) SR_OK;
}

#endif
