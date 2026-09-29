/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/Subscriber.h $
* $Id: Subscriber.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project	: CODAC Core System
*
* Description	: SDN - Infrastructure tools - Prototype
*
* Author        : Bertrand Bauvir
*
* Copyright (c) : 2010-2026 ITER Organization,
*		  CS 90 046
*		  13067 St. Paul-lez-Durance Cedex
*		  France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

/**
 * @file Subscriber.h
 * @brief Header file for sdn::Subscriber interface classes
 * @date 18/04/2018
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 *
 * @detail This header file contains the definition of the Subscriber
 * interface classes.
 *
 * @todo Provide template implementation to associate participants to particular
 * payload structures.
 * @todo Make Configure implicit upon calling CopyTopicInstance or Receive
 * methods.
 */

#ifndef _Subscriber_h_
#define _Subscriber_h_

/* Global header files */

/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

#include "sdn-topic.h" /* sdn::Topic and sdn::Metadata_t definition */

#include "Participant.h"

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {

class SubscriberImpl; /* Forward class declaration */

/**
 * @brief The class provides the API to an SDN subscriber.
 * @detail After instantiation, the network interface can be specified
 * using the SetInterface method. The payload intialisation, notification
 * of participant on the SDN network, etc. are all taken care of through
 * calling the Configure method.
 *
 * Topic instance is defined by the calling application and passed to the 
 * Publisher by reference or copy operation before Publish is called.
 *
 * @notice The design is based on the 'bridge' pattern hiding implementation
 * details to the application through the API and, as such, enables changes
 * of implementation with limited impact on the application themselves.
 *
 * @todo Payload byte ordering to be implicitly done by the Subscriber in
 * case of introspectable topic type definition.
 */

class SubscriberIface : public ParticipantIface
{
        
  private:
        
    /**
     * @brief Reference to the actual implementation.
     */
    SubscriberImpl* p_impl;
        
  public:
        
    /**
     * @brief Constructor. NOOP.
     */
    SubscriberIface (void);

    /**
     * @brief Constructor.
     * @detail Instantiate the Subscriber with topic association. This is equivalent
     * to using the default constructor and calling SetMetadata(). No exception is
     * thrown in case the metadata is invalid. Rather, the consecutive call to 
     * Configure() may return and error in that case.
     */
    SubscriberIface (Metadata_t& mdata);

    /**
     * @brief Constructor.
     * @detail Instantiate the Subscriber with topic association. This is equivalent
     * to using the default constructor and calling SetTopic().
     *
     * @warning The topic instance life-cycle is managed by the calling application.
     * The instance must therefore be destroyed explicitly when not required
     * anylonger.
     */
    SubscriberIface (Topic& topic);
        
    /**
     * @brief Destructor.
     */
    virtual ~SubscriberIface (void);

    /**
     * @brief Accessor method. See sdn::Participant::IsInitialized
     */
    virtual bool IsInitialized (void) const; /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::IsPayloadOrdered
     * @detail The method is used to detect whether the received payload accessible
     * by refence or copy operation is following the same byte ordering as the host
     * native ordering scheme, and whether application-side conversion is required or
     * not. 
     *
     * In case the method returns false, helper routines are available to convert to
     * host byte order. See sdn::HelperTools::IsLittleEndian,
     * sdn::HelperTools::IsBigEndian, sdn::HelperTools::FromNetworkByteOrder, etc.
     *
     * @return TRUE if the received payload is directly interpretable by the receiving
     * host without byte ordering conversion required, FALSE otherwise. 
     *
     * @todo For topics with introspectable type definition, the sdn::Subscriber should 
     * be able to perform implicit conversion, if required, and always return true.
     *
     * @code
       typedef struct MyTopic {
         uint64_t index;
         uint64_t timestamp;
         char buffer [STRING_MAX_LENGTH];
         uint8_t reserved [8];
       } MyTopic_t;

       sdn::Metadata_t mdata; sdn::TopicHelper::InitializeMetadata(mdata, "MyTopic", sizeof(MyTopic_t));

       // Create Subscriber using metadata
       sdn::Subscriber* sub = static_cast<sdn::Subscriber*>(NULL);

       bool ok = (static_cast<sdn::Subscriber*>(NULL) != 
		  (sub = new (std::nothrow) sdn::Subscriber (mdata)));

       if (ok) {
         // No more initialisation steps
	 ok = (sub->Configure() == STATUS_SUCCESS);
       }

       // Define payload instance
       MyTopic_t pub_t; memset(&pub_t, 0, sizeof(MyTopic_t));

       if (ok) {
         // Receive topic instance - Blocking, with timeout
	 ok = (sub->Receive(static_cast<void*>(&sub_t), 1000000000ul) == STATUS_SUCCESS);
       }

       if (ok) {
         // Test received message byte ordering
	 if (sub->IsPayloadOrdered() == false) {
	   if (sdn::HelperTools::IsLittleEndian()) {
	     // Convert payload to native byte ordering
	     sub_t.counter = sdn::HelperTools::FromNetworkByteOrder(sub_t.counter);
	     sub_t.timestamp = sdn::HelperTools::FromNetworkByteOrder(sub_t.timestamp);
	   }
	 }
       }
       @endcode
     */
    virtual bool IsPayloadOrdered (void) const; /* Bug 10327 - Same byte ordering as host */

    /**
     * @brief Accessor method. See sdn::Participant::GetInterface
     * @pre
     *   Configure() == STATUS_SUCCESS
     */
    virtual const char* GetInterface (void) const; /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::GetTopicName
     * @pre
     *   Configure() == STATUS_SUCCESS
     */
    virtual const char* GetTopicName (void) const; /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::GetTopicSize
     * @pre
     *   Configure() == STATUS_SUCCESS
     */
    virtual uint_t GetTopicSize (void) const; /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::GetTopicHeader
     * @pre
     *   Configure() == STATUS_SUCCESS
     * @return Header reference if preconditions are met, NULL otherwise.
     */
    virtual void* GetTopicHeader (void) const;   /* Specializes virtual method */

    /**
     * @brief Accessor method. See sdn::Participant::GetTopicInstance
     * @pre
     *   Configure() == STATUS_SUCCESS
     * @return Payload reference if preconditions are met, NULL otherwise.
     */
    virtual void* GetTopicInstance (void) const; /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::SetInterface
     * @pre 
     *   sdn::HelperTools::IsInterfaceValid(GetInterface()) == true
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS SetInterface (const char* name); /* Specializes virtual method */

    /**
     * @brief Accessor method. Specifies the depth of the socket buffer.
     * @param depth The send buffer depth in number of topic instances.
     */
    virtual RET_STATUS SetBufferDepth (uint_t depth); // Specializes virtual method

    // Deleted - virtual RET_STATUS SetTopicName (const char* name); /* Specializes virtual method */

    /**
     * @brief Accessor method. See sdn::Participant::SetMetadata
     * @pre
     *   sdn::TopicHelper::IsMetadataValid(mdata) == true &&
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS SetMetadata (Metadata_t& mdata); /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::SetTopic
     */
    virtual RET_STATUS SetTopic (Topic& topic); /* Specializes virtual method */

    /**
     * @brief Accessor method. See sdn::Participant::SetCallback
     * @detail The method is called after message reception.
     * @pre
     *   cb != static_cast<void (*)(void*)>(NULL)
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS SetCallback (void (* cb)(void*)); /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::SetCallback
     * @detail The method is called after message reception.
     * @pre
     *   cb != static_cast<void (*)(void*)>(NULL)
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr); /* Specializes virtual method */
        
    virtual int GetSocket (void); /* Specializes virtual method */

    /**
     * @brief See sdn::Participant::Configure
     * @pre 
     *   sdn::TopicHelper::IsMetadataValid() == true &&
     *   sdn::HelperTools::IsInterfaceValid(GetInterface()) == true
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS Configure (void); /* Specializes virtual method */

    /**
     * @brief See sdn::Participant::CopyTopicInstance
     * @detail The method copies the payload of the message buffer held by the
     * sdn::Participant to the reference pointed by the method parameter.
     * @pre
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size); /* Specializes virtual method */

    virtual RET_STATUS Do (void) { return this->Receive(); }; /* Specializes virtual method */

    /**
     * @brief See sdn::Participant::DoBackgroundActivity
     * @pre 
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS DoBackgroundActivity (void); /* Specializes virtual method */

    /**
     * @brief Receive method.
     * @detail The method blocks till a message is received. The message is checked
     * for validity. The calling application can access the received topic instance
     * by reference, see GetTopicInstance(), or copy operation, see CopyTopicInstance().
     * The message header will autonomously be changed to include the receive timestamp
     * so as to support receiver-side latency statistics, etc.
     * @pre 
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met and message successfully received,
     * STATUS_ERROR otherwise.
     *
     * @code
       typedef struct MyTopic {
         uint64_t index;
         uint64_t timestamp;
         char buffer [STRING_MAX_LENGTH];
         uint8_t reserved [8];
       } MyTopic_t;

       sdn::Metadata_t mdata; sdn::TopicHelper::InitializeMetadata(mdata, "MyTopic", sizeof(MyTopic_t));

       // Create Subscriber using metadata
       sdn::Subscriber* sub = static_cast<sdn::Subscriber*>(NULL);

       bool ok = (static_cast<sdn::Subscriber*>(NULL) != 
		  (sub = new (std::nothrow) sdn::Subscriber (mdata)));

       if (ok) {
         // No more initialisation steps
	 ok = (sub->Configure() == STATUS_SUCCESS);
       }

       if (ok) {
         // Receive topic instance - Blocking, no timeout
	 ok = (sub->Receive() == STATUS_SUCCESS);
       }

       if (ok) {      
         // Access payload instance by reference
         MyTopic_t* p_topic = static_cast<MyTopic_t*>(sub->GetTopicInstance());

	 // Do something with received payload
       }
       @endcode
     */
    RET_STATUS Receive (void);

     /**
     * @brief Receive method with timeout.
     * @detail The method is polling (non-blocking) if called with zero and blocks
     * at most <timeout> nanoseconds otherwise. 
     * @param[1] timeout [ns]
     * @pre 
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met and message successfully received,
     * STATUS_ERROR otherwise.
     *
     * @code
       typedef struct MyTopic {
         uint64_t index;
         uint64_t timestamp;
         char buffer [STRING_MAX_LENGTH];
         uint8_t reserved [8];
       } MyTopic_t;

       sdn::Metadata_t mdata; sdn::TopicHelper::InitializeMetadata(mdata, "MyTopic", sizeof(MyTopic_t));

       // Create Subscriber using metadata
       sdn::Subscriber* sub = static_cast<sdn::Subscriber*>(NULL);

       bool ok = (static_cast<sdn::Subscriber*>(NULL) != 
		  (sub = new (std::nothrow) sdn::Subscriber (mdata)));

       if (ok) {
         // No more initialisation steps
	 ok = (sub->Configure() == STATUS_SUCCESS);
       }

       // Access payload instance by reference
       MyTopic_t* p_topic = static_cast<MyTopic_t*>(sub->GetTopicInstance());

       // Non-blocking spinwait on message reception
       while (sub->Receive(0ul) != STATUS_SUCCESS) {};

       // Do something with received payload
       @endcode
     */
    RET_STATUS Receive (uint64_t timeout);

    /**
     * @brief Receive method with external reference and timeout.
     * @detail The method is polling (non-blocking) if called with zero and blocks
     * at most <timeout> nanoseconds otherwise. 
     * @param[1] reference to externally help topic instance
     * @param[2] timeout [ns]
     * @pre 
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met and message successfully received,
     * STATUS_ERROR otherwise.
     *
     * @code
       typedef struct MyTopic {
         uint64_t index;
         uint64_t timestamp;
         char buffer [STRING_MAX_LENGTH];
         uint8_t reserved [8];
       } MyTopic_t;

       sdn::Metadata_t mdata; sdn::TopicHelper::InitializeMetadata(mdata, "MyTopic", sizeof(MyTopic_t));

       // Create Subscriber using metadata
       sdn::Subscriber* sub = static_cast<sdn::Subscriber*>(NULL);

       bool ok = (static_cast<sdn::Subscriber*>(NULL) != 
		  (sub = new (std::nothrow) sdn::Subscriber (mdata)));

       if (ok) {
         // No more initialisation steps
	 ok = (sub->Configure() == STATUS_SUCCESS);
       }

       // Define payload instance
       MyTopic_t sub_t; memset(&pub_t, 0, sizeof(MyTopic_t));

       if (ok) {
         // Receive topic instance - Blocking, with timeout
	 ok = (sub->Receive(static_cast<void*>(&sub_t), 100000ul) == STATUS_SUCCESS);
       }

       if (ok) {      
	 // Do something with received payload
       }
       @endcode
     */
    RET_STATUS Receive (void* instance, uint64_t timeout) { RET_STATUS status = this->Receive(timeout); if (status == STATUS_SUCCESS) this->CopyTopicInstance(instance, this->GetTopicSize()); return status; };

//	virtual bool operator= (const SubscriberIface& iface) const = 0; // Pure virtual method

};

typedef SubscriberIface Subscriber; /* For backward compatibility purposes with v1.0 */

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace sdn */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _Subscriber_h_ */
