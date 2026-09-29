/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/AsynchronousSubscriber.h $
* $Id: AsynchronousSubscriber.h 179115 2026-01-09 15:15:20Z kimh19 $
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
 * @file AsynchronousSubscriber.h
 * @brief Header file for sdn::AsynchronousSubscriber classes
 * @date 18/04/2018
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 *
 * @detail This header file contains the definition of the AsynchronousSubscriber
 * interface classes.
 *
 * @todo Provide template implementation to associate participants to particular
 * payload structures.
 */

#ifndef _AsynchronousSubscriber_h_
#define _AsynchronousSubscriber_h_

// Global header files

// Local header files

#include "constants.h" // Constants valid for this scope
#include "sdn-types.h" // Misc. type definition, e.g. RET_STATUS

#include "sdn-topic.h" // sdn::Topic related types and class definition

// Constants

#ifdef __cplusplus

// Type definition

namespace sdn {

class AsynchronousSubscriberImpl; // Forward class declaration

/**
 * @brief The class provides the API to an SDN asynchronous subscriber.
 * @detail After instantiation, the network interface can be specified
 * using the SetInterface method. The payload intialisation, notification
 * of participant on the SDN network, thread launch, etc. are all taken
 * care of through calling the Configure method.
 * Topic reception is being performed by the thread and topic instance
 * updated and held upon successful reception. The application can access
 * the last successfully received topic instance by reference or copy
 * operation. 
 *
 * @notice The design is based on the 'bridge' pattern hiding implementation
 * details to the application through the API and, as such, enables changes
 * of implementation with limited impact on the application themselves.
 *
 * @notice Notification through callback is being supported through the
 * SetCallback method provided by sdn::core::Participant and/or 
 * application-specific inheritance of this class and specialising the
 * HandleReceive method.
 */

class AsynchronousSubscriberIface
{

  private:
        
    /**
     * @brief Reference to the actual implementation.
     */
    AsynchronousSubscriberImpl* p_impl;
        
  public:

    /**
     * @brief Constructor. Topic specified through metadata.
     * @detail The topic will be instantiated and reference kept
     * internally upon calling the Configure method.
     *
     * @code
       #include "sdn-topic.h"
     
       struct MyPayload {
         sdn::types::uint64 timestamp;
         sdn::types::float64 signal;
       };
      
       sdn::Metadata_t mdata = InitializeMetadataStatic("MyTopicName", sizeof(MyPayload), 0u);   
       sdn::AsynchronousSubscriber* sub = static_cast<sdn::AsynchronousSubscriber*>(NULL);
     
       bool ok = (static_cast<sdn::AsynchronousSubscriber*>(NULL) !=
                  (sub = new (std::nothrow) sdn::AsynchronousSubscriber (mdata)));
       if (ok) {
         ok = (sub->Configure() == STATUS_SUCCESS);
       }
       @endcode
     */
    AsynchronousSubscriberIface (Metadata_t& mdata);

    /**
     * @brief Constructor. Topic externally instantiated.
     * @detail A copy of the topic reference will be used but the
     * instantiation of the topic is under the responsibility of
     * the calling application. This method is used by generic
     * applications that declare topic structure at runtime and
     * therefore need to programmatically access its introspection 
     * interface.
     *
     * @code
       #include "sdn-topic.h"
     
       sdn::Metadata_t mdata; sdn::TopicHelper::InitializeMetadata(mdata, "MARTeTopic", 0u);
       sdn::Topic* topic = static_cast<sdn::Topic*>(NULL);
       sdn::AsynchronousSubscriber* sub = static_cast<sdn::AsynchronousSubscriber*>(NULL);
     
       bool ok = (static_cast<sdn::Topic*>(NULL) != (topic = new (std::nothrow) sdn::Topic));
     
       if (ok) {
         topic->SetMetadata(mdata);
       }
     
       // Programatically define topic datatype
       if (ok) {
         ok = (topic->AddAttribute<uint64>(0u, "index") == STATUS_SUCCESS);
       }
       if (ok) {
         ok = (topic->AddAttribute<uint64>(1u, "timestamp") == STATUS_SUCCESS);
       }
       if (ok) {
         ok = (topic->AddAttribute<uint8>(2u, "reserved", 64) == STATUS_SUCCESS);
       }
       if (ok) {
         ok = (topic->Configure() == STATUS_SUCCESS);
       }
     
       // Create subscriber from topic instance
       if (ok) {
         ok = (static_cast<sdn::AsynchronousSubscriber*>(NULL) !=
               (sub = new (std::nothrow) sdn::AsynchronousSubscriber (*topic)));
       }
       if (ok) {
         ok = (sub->Configure() == STATUS_SUCCESS);
       }
       @endcode
     */
    AsynchronousSubscriberIface (Topic& topic);

    /**
     * @brief Destructor. 
     * @detail The thread is terminated, etc.
     */
    virtual ~AsynchronousSubscriberIface (void);

    /**
     * @brief Accessor method.
     * @detail Returns the interface bound to by the Participant.
     * @return Interface name.
     */
    const char* GetInterface (void) const;

    /**
     * @brief Accessor method.
     * @detail Returns the topic name used by the Participant.
     * @return Topic name.
     */
    const char* GetTopicName (void) const;

    /**
     * @brief Accessor method.
     * @detail Returns the topic size used by the Participant.
     * @return Topic size.
     */
    uint_t GetTopicSize (void) const;

    /**
     * @brief Accessor method.
     * @detail Returns the reference to the last received message header.
     * @return Header reference.
     *
     * @code
       #include "sdn-header.h"
     
       class MyAsynchronousSubscriber : public AsynchronousSubscriber {
         public:
           MyAsynchronousSubscriber() : AsynchronousSubscriber(mdata) { this->Configure(); };
           virtual void HandleReceive() {
             sdn::Header_t* header = static_cast<sdn::Header_t*>(this->GetTopicHeader());
             log_info("Communication latency '%lu'", header->recv_time - header->send_time)
           };
       };
       @endcode
     */
    const void* GetTopicHeader (void) const;

    /**
     * @brief Accessor method.
     * @detail Returns the reference to the last received message payload.
     * @return Payload reference.
     *
     * @code
       struct MyPayload {
         sdn::types::uint64 timestamp;
         sdn::types::float64 signal;
       };
      
       sdn::Metadata_t mdata = InitializeMetadataStatic("MyTopicName", sizeof(MyPayload), 0u);   
     
       class MyAsynchronousSubscriber : public AsynchronousSubscriber {
         public:
           MyAsynchronousSubscriber() : AsynchronousSubscriber(mdata) { this->Configure(); };
           virtual void HandleReceive() {
             struct MyPayload* topic = static_cast<struct MyPayload*>(this->GetTopicInstance()));
             // Process received instance
           };
       };
       @endcode
     */
    const void* GetTopicInstance (void) const;

    /**
     * @brief SetAffinity method.
     * @detail The method can be used to set the thread affinity to a particulat core.
     * By default, the thread handling asycnhronous reception is using the same affinity
     * as the application process. Changing policy/priority can be done while runnign,
     * upon conditions, etc.
     * @return STATUS_SUCCESS if successful, STATUS_ERROR otherwise.
     */
    RET_STATUS SetAffinity (uint_t core);

    /**
     * @brief SetPriority method.
     * @detail The method can be used to set the thread scheduling policy and priority.
     * By default, the thread scheduling is inherited from the application process.
     * Changing policy/priority can be done while running, upon conditions, etc.
     * @return STATUS_SUCCESS if successful, STATUS_ERROR otherwise.
     *
     * @code
       sdn::Metadata_t mdata = InitializeMetadataStatic("MyTopicName", sizeof(MyPayload), 0u);   
       sdn::AsynchronousSubscriber* sub = static_cast<sdn::AsynchronousSubscriber*>(NULL);
     
       bool ok = (static_cast<sdn::AsynchronousSubscriber*>(NULL) !=
                  (sub = new (std::nothrow) sdn::AsynchronousSubscriber (mdata)));
     
       if (ok) {
         // Nail thread to CPU core 0
         ok = (sub->SetAffinity(0u) == STATUS_SUCCESS);
       }
       if (ok) {
         // Requires particular priviledges to do so
         ok = (sub->SetPriority(SCHED_FIFO, 40u) == STATUS_SUCCESS);
       }
     
       if (ok) {
         ok = (sub->Configure() == STATUS_SUCCESS);
       }
       @endcode
     */
    RET_STATUS SetPriority (uint_t policy, uint_t priority);

    /**
     * @brief SetTimeout method.
     * @detail The method can be used to specify a message reception timeout and, as such,
     * can be used with 0 to have the thread spin waiting on message reception for lower
     * jitter if running isolated on a core. By default, the timeout is 1s. Changing timeout
     * can be done while running, upon conditions, etc.
     * @return STATUS_SUCCESS if successful, STATUS_ERROR otherwise.
     *
     * @code
       sdn::Metadata_t mdata = InitializeMetadataStatic("MyTopicName", sizeof(MyPayload), 0u);   
       sdn::AsynchronousSubscriber* sub = static_cast<sdn::AsynchronousSubscriber*>(NULL);
      
       bool ok = (static_cast<sdn::AsynchronousSubscriber*>(NULL) !=
                  (sub = new (std::nothrow) sdn::AsynchronousSubscriber (mdata)));
       
       if (ok) {
         // Nail thread to CPU core 0
         ok = (sub->SetAffinity(0u) == STATUS_SUCCESS);
       }
       if (ok) {
         // Spin waits for message reception
         ok = (sub->SetTimeout(0ul) == STATUS_SUCCESS);
       }
     
       if (ok) {
         ok = (sub->Configure() == STATUS_SUCCESS);
       }
       @endcode
     */
    RET_STATUS SetTimeout (uint64_t timeout);

    /**
     * @brief Accessor method. Specifies the depth of the socket buffer.
     * @param depth The send buffer depth in number of topic instances.
     */
    RET_STATUS SetBufferDepth (uint_t depth);

    /**
     * @brief SetCallback method.
     * @detail The method can be used to provide a function reference to be called
     * upon message reception.
     * @pre
     *   cb != static_cast<void (*)(void*)>(NULL)
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     *
     * @warning The callback is currently managed as part of the ccs::mcast::Participant
     * message handling which is unaware of SDN-specific protocol. The callback will be
     * called even in case of message reception which would otherwise be rejected as 
     * invalid by the sdn::core::Participant.
     */
    RET_STATUS SetCallback (void (* cb)(void*));

    /**
     * @brief SetCallback method.
     * @detail The method can be used to provide a function reference to be called
     * with application-specific attribute upon message reception.
     * @pre
     *   cb != static_cast<void (*)(void*)>(NULL)
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     *
     * @code
       void __callback (sdn::AsynchronousSubscriber* sub) 
       { 
         // Do something
       };

       sdn::Metadata_t mdata = InitializeMetadataStatic("MyTopicName", sizeof(MyPayload), 0u);   
       sdn::AsynchronousSubscriber* sub = static_cast<sdn::AsynchronousSubscriber*>(NULL);
     
       bool ok = (static_cast<sdn::AsynchronousSubscriber*>(NULL) !=
                  (sub = new (std::nothrow) sdn::AsynchronousSubscriber (mdata)));
     
       if (ok) {
         // Statically defined callback called with Participant instance
         ok = (sub->SetCallback((void (*) (void*)) &__callback, static_cast<void*>(sub)) == STATUS_SUCCESS);
       }
     
       if (ok) {
         ok = (sub->Configure() == STATUS_SUCCESS);
       }
       @endcode
     *
     * @warning The callback is currently managed as part of the ccs::mcast::Participant
     * message handling which is unaware of SDN-specific protocol. The callback will be
     * called even in case of message reception which would otherwise be rejected as 
     * invalid by the sdn::core::Participant.
     */
    RET_STATUS SetCallback (void (* cb)(void*), void* attr);

    /**
     * @brief SetInterface method. Specifies the network interface to bind to.
     * @detail The method can be called to overwrite the default SDN interface
     * which is defined through SDN_INTERFACE_NAME environment variable or set
     * to 'lo' if undefined.
     * @pre 
     *   ccs::base::IsInterfaceValid(GetInterface()) == true
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    RET_STATUS SetInterface (const char* name);
        
    /**
     * @brief Configure method. Completes the instance initialisation.
     * @detail The method is called after participant initialisation to
     * complete the configuration phase, e.g. interface defined, callback
     * installed, etc. The method configures the sdn::Subscriber and spawns
     * the thread for asynchronous reception. The thread also handles
     * discovery protocol and replies to topic introspection requests.
     * @pre 
     *   sdn::topicHelper::IsMetadataValid() == true &&
     *   ccs::base::IsInterfaceValid(GetInterface()) == true
     * @return STATUS_SUCCESS if preconditions are met, STATUS_ERROR otherwise.
     */
    RET_STATUS Configure (void);

    /**
     * @brief Copy method. 
     * @detail The last received topic instance is copied to the provided reference.
     * The application is responsible for buffer allocation. The size of the memory
     * footprint is retrieved using the sdn::Topic::GetSize().
     */
    RET_STATUS CopyTopicInstance (void* instance) const;

    /**
     * @brief Copy method. 
     * @detail The last received topic instance is copied to the provided reference with
     * at most the provided size.
     */
    RET_STATUS CopyTopicInstance (void* instance, uint_t size) const;

    /**
     * @brief HandleReceive method. 
     * @detail May be overloaded by application-specific implementations. The method
     * will be called upon successful reception by the thread.
     *
     * @code
       #include <AsynchrounousSubscriber.h>
 
       struct MyPayload {
         sdn::types::uint64 timestamp;
         sdn::types::float64 signal;
       };
 
       sdn::Metadata_t mdata = InitializeMetadataStatic("MyTopicName", sizeof(MyPayload), 0u);   
 
       class MyAsynchronousSubscriber : public AsynchronousSubscriber {
         private:
           MyPayload payload = { 0ul, 0.0 };
         public:
           MyAsynchronousSubscriber() : AsynchronousSubscriber(mdata) { this->Configure(); };
           virtual void HandleReceive() {
             // Take a local copy of the payload
             this->CopyTopicInstance(static_cast<void*>(&payload));
           };
       };
       @endcode
     */
    virtual void HandleReceive (void) { return; };

};

typedef AsynchronousSubscriberIface AsynchronousSubscriber;

// Global variables

// Function declaration

// Function definition

} // namespace sdn

extern "C" {

#endif // __cplusplus

// ToDo - Insert C API declaration

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _AsynchronousSubscriber_h_
