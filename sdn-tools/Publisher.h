/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/Publisher.h $
* $Id: Publisher.h 179115 2026-01-09 15:15:20Z kimh19 $
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
 * @file Publisher.h
 * @brief Header file for sdn::Publisher interface classes
 * @date 18/04/2018
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 *
 * @detail This header file contains the definition of the Publisher
 * interface classes.
 *
 * @todo Provide template implementation to associate participants to particular
 * payload structures.
 * @todo Make Configure implicit upon calling CopyTopicInstance or Publish
 * methods.
 */

#ifndef _Publisher_h_
#define _Publisher_h_

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

class PublisherImpl;  /* Forward class declaration */

/**
 * @brief The class provides the API to an SDN publisher.
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
 */

class PublisherIface : public ParticipantIface
{
        
  private:
    
    /**
     * @brief Reference to the actual implementation.
     */
    PublisherImpl* p_impl;
    
  public:
        
    /**
     * @brief Constructor. NOOP.
     */
    PublisherIface (void);

    /**
     * @brief Constructor.
     * @detail Instantiate the Publisher with topic association. This is equivalent
     * to using the default constructor and calling SetMetadata(). No exception is
     * thrown in case the metadata is invalid. Rather, the consecutive call to 
     * Configure() may return and error in that case.
     */
    PublisherIface (Metadata_t& mdata);

    /**
     * @brief Constructor.
     * @detail Instantiate the Publisher with topic association. This is equivalent
     * to using the default constructor and calling SetTopic().
     *
     * @warning The topic instance life-cycle is managed by the calling application.
     * The instance must therefore be destroyed explicitly when not required
     * anylonger.
     */
    PublisherIface (Topic& topic);
        
    /**
     * @brief Destructor.
     */
    virtual ~PublisherIface (void);

	
	virtual PublisherIface& operator= (const PublisherIface& iface);
		
    /**
     * @brief Accessor method. See sdn::Participant::IsInitialized
     */
    virtual bool IsInitialized (void) const; /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::IsPayloadOrdered
     * @detail In absence of any application-side action, the Publisher publishes
     * SDN topics according to the host native byte order. This applies to the 
     * message header, footer, and payload presumable which is being set by the
     * application before being passed by reference or copy operation to the
     * Publisher. The application may opt to order the payload differently, in 
     * which case the Publisher can be instructed to also change the byte ordering
     * of header and footer to match the ordering of the payload.
     *
     * In case the method returns false, helper routines are available to convert to
     * host byte order. See sdn::HelperTools::IsLittleEndian,
     * sdn::HelperTools::IsBigEndian, sdn::HelperTools::FromNetworkByteOrder, etc.
     *
     * @return TRUE if the payload is directly interpretable by the application host
     * without byte ordering conversion required, FALSE otherwise.
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
     * @brief Accessor method. See sdn::Participant::GetPayloadOrder
     * @return The declared byte ordering which will be the host
     * native ordering after instantiation of the Publisher.
     *
     * @todo Make the method virtual as it applies to all Participants.
     */
    types::Endianness_t GetPayloadOrder (void) const;
        
    /**
     * @brief Accessor method. See sdn::Participant::SetPayloadOrder
     * @detail Declaration of the payload byte ordering performed by the
     * application. Without any action on the application side, the byte
     * ordering will be assumed to be the host native ordering.
     * @return The previously declared byte ordering which will be the
     * host native ordering after instantiation of the Publisher.
     *
     * @todo Make the method virtual as it applies to all Participants.
     *
     * @code
       typedef struct MyTopic {
         uint64_t index;
         uint64_t timestamp;
         char buffer [STRING_MAX_LENGTH];
         uint8_t reserved [8];
       } MyTopic_t;

       sdn::Metadata_t mdata; sdn::TopicHelper::InitializeMetadata(mdata, "MyTopic", sizeof(MyTopic_t));

       // Create Publisher using metadata
       sdn::Publisher* pub = static_cast<sdn::Publisher*>(NULL);

       bool ok = (static_cast<sdn::Publisher*>(NULL) != 
		  (pub = new (std::nothrow) sdn::Publisher (mdata)));

       if (ok) {
         // This publisher will choose to use network byte ordering
	 pub->SetPayloadOrder(sdn::types::BigEndian);
       }

       if (ok) {
         // No more initialisation steps
	 ok = (pub->Configure() == STATUS_SUCCESS);
       }

       // Define payload instance
       MyTopic_t pub_t; memset(&pub_t, 0, sizeof(MyTopic_t));
       sstrncpy(pub_t.buffer, "Hello world!", STRING_MAX_LENGTH);

       while (ok) {
         // Update topic payload   
         pub_t.index += 1ul;
         pub_t.timestamp = get_time();

	 // Test host native byte ordering
	 if (sdn::HelperTools::IsLittleEndian() == true) {
	   pub_t.index = sdn::HelperTools::ToNetworkByteOrder(pub_t.index);
	   pub_t.timestamp = sdn::HelperTools::ToNetworkByteOrder(pub_t.timestamp);
	 }

         // Publish
         ok = (pub->Publish(static_cast<void*>(&pub_t)) == STATUS_SUCCESS);

	 // Wait for 1s
	 wait_until(pub_t.timestamp + 1000000000ul);
       }
       @endcode
     */
    void SetPayloadOrder (types::Endianness_t order);

    /**
     * @brief Accessor method. See sdn::Participant::SetInterface
     * @pre 
     *   sdn::HelperTools::IsInterfaceValid(GetInterface()) == true
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS SetInterface (const char* name); /* Specializes virtual method */

    /**
     * @brief Accessor method.
     * @detail Publisher specific method to set the network interface and override
     * the default publisher-side UDP port to use.
     * @param name Named interface
     * @param port UDP source port,'0u' defaults the source port to be identical
     *           to the UDP destination port
     * @pre 
     *   sdn::HelperTools::IsInterfaceValid(param[1]) == true &&
     *   (port == 0u || (port > 2^10 (priviledged range) && port < 2^16))
     *   
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    RET_STATUS SetInterface (const char* name, uint_t port);

    /**
     * @brief Accessor method. Specifies the Time To Live (TTL) value on the socket.
     * @param depth The TTL value.
     */
    RET_STATUS SetTimeToLive (uint8_t ttl);

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
     * @detail The method is called before message publication.
     * @pre
     *   cb != static_cast<void (*)(void*)>(NULL)
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS SetCallback (void (* cb)(void*)); /* Specializes virtual method */
        
    /**
     * @brief Accessor method. See sdn::Participant::SetCallback
     * @detail The method is called before message publication.
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
     * @detail The method copies the instance pointed to by the parameter to the
     * message buffer held by the sdn::Participant.
     * @pre
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size); /* Specializes virtual method */

    virtual RET_STATUS Do (void) { return this->Publish(); }; /* Specializes virtual method */

    /**
     * @brief See sdn::Participant::DoBackgroundActivity
     * @pre 
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    virtual RET_STATUS DoBackgroundActivity (void); /* Specializes virtual method */

    /**
     * @brief Publish method.
     * @detail The method is called after the topic instance is updated by the
     * calling application. The topic instance is either manipulated by reference,
     * see GetTopicInstance(), or copy operation, see CopyTopicInstance(). The
     * message header will autonomously be changed to increase instance counter
     * and send timestamp so as to support receiver-side packet loss detection,
     * latency statistics, etc.
     * @pre 
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met and message successfully sent,
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

       // Create Publisher using metadata
       sdn::Publisher* pub = static_cast<sdn::Publisher*>(NULL);

       bool ok = (static_cast<sdn::Publisher*>(NULL) != 
		  (pub = new (std::nothrow) sdn::Publisher (mdata)));

       if (ok) {
         // No more initialisation steps
	 ok = (pub->Configure() == STATUS_SUCCESS);
       }

       if (ok) {      
         // Define payload instance
         MyTopic_t pub_t; memset(&pub_t, 0, sizeof(MyTopic_t));

         pub_t.index = 1ul;
         pub_t.timestamp = get_time();
         sstrncpy(pub_t.buffer, "Hello world!", STRING_MAX_LENGTH);

         ok = (pub->CopyTopicInstance(&pub_t, sizeof(MyTopic_t)) == STATUS_SUCCESS);
       }

       if (ok) {
         // Publish topic instance
	 ok = (pub->Publish() == STATUS_SUCCESS);
       }
       @endcode
     */
    RET_STATUS Publish (void);

    /**
     * @brief Publish method.
     * @detail The method is provided to publish externally held topic instance. The
     * method concatenates both a CopyTopicInstance() and Publish().
     * @param[1] reference to externally help topic instance
     * @pre 
     *   Configure() == STATUS_SUCCESS
     * @return STATUS_SUCCESS if preconditions are met and message successfully sent,
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

       // Create Publisher using metadata
       sdn::Publisher* pub = static_cast<sdn::Publisher*>(NULL);

       bool ok = (static_cast<sdn::Publisher*>(NULL) != 
		  (pub = new (std::nothrow) sdn::Publisher (mdata)));

       if (ok) {
         // No more initialisation steps
	 ok = (pub->Configure() == STATUS_SUCCESS);
       }

       // Define payload instance
       MyTopic_t pub_t; memset(&pub_t, 0, sizeof(MyTopic_t));
       sstrncpy(pub_t.buffer, "Hello world!", STRING_MAX_LENGTH);

       while (ok) {
         // Update topic payload   
         pub_t.index += 1ul;
         pub_t.timestamp = get_time();

         // Publish
         ok = (pub->Publish(static_cast<void*>(&pub_t)) == STATUS_SUCCESS);

	 // Wait for 1s
	 wait_until(pub_t.timestamp + 1000000000ul);
       }
       @endcode
     */
    RET_STATUS Publish (void* instance) { /* Update payload */ this->CopyTopicInstance(instance, this->GetTopicSize()); return this->Publish(); };

//	PublisherIface& operator= (const PublisherIface& iface);

};
    
typedef PublisherIface Publisher; /* For backward compatibility purposes with v1.0 */

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

#endif /* _Publisher_h_ */
