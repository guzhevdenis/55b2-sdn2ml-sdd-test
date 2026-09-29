#ifndef CORE_SUBSCRIBER_H
#define CORE_SUBSCRIBER_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/core-subscriber.h $
* $Id: core-subscriber.h 179115 2026-01-09 15:15:20Z kimh19 $
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

/* Global header files */

/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

#include "sdn-base.h" /* Privately scoped base classes definition */

//#include "mcast-participant.h"
//#include "mcast-subscriber.h"

#include "core-participant.h"

#include "sdn-packet.h"
#include "sdn-header.h"
#include "sdn-topic.h"

/* Constants */

#define OBJTYPE_CORE_SUBSCRIBER (char*) "sdn::core::Subscriber_Base"

#ifdef __cplusplus

/* Type definition */

namespace sdn {

namespace core {

class Subscriber_Base : public ParticipantImpl /* Base class */
{

  private:

    /* Initializer methods */
    void Initialize (void) {};

  public:

    /* Initializer methods */

    /* Accessor methods */
    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size) { RET_STATUS status = STATUS_ERROR; if ((this->m_topic)->GetSize() <= size) { memcpy(instance, this->GetTopicInstance(), (this->m_topic)->GetSize()); status = STATUS_SUCCESS; } return status; }; /* Specializes virtual method */

    /* Miscellaneous methods */
    virtual RET_STATUS Receive (uint64_t timeout) = 0; /* Pure virtual method */
    virtual RET_STATUS Receive (void) = 0;           /* Pure virtual method */

    /* Constructor methods */
    Subscriber_Base (void);

    /* Destructor method */
    virtual ~Subscriber_Base (void) { return; };

}; 

class SubscriberImpl : public Subscriber_Base
{

  private:

    base::SubscriberIface* m_base; /* Instantiate and receive from MCAST or UCAST socket */

    uint64_t m_doubled; 
    uint64_t m_dropped; 
    uint64_t m_missing; 
    uint64_t m_counter; 

    uint_t m_source;

    /* Initializer methods */
    void Initialize (void);

    /* Miscellaneous methods */
    bool IsValid (void);

  public:

    /* Initializer methods */

    /* Accessor methods */
    virtual int GetSocket (void) const { int sock = -1; if (this->m_base != NULL) sock = (this->m_base)->GetSocket(); return sock; }; /* Specializes virtual method */

    /* Miscellaneous methods */
    virtual RET_STATUS Configure (void);

    virtual RET_STATUS Do (void); // Specializes virtual method

    virtual RET_STATUS Receive (uint64_t timeout);
    virtual RET_STATUS Receive (void);

    /* Constructor methods */
    SubscriberImpl (void) { /* Initialize resources */ this->Initialize(); return; };
    SubscriberImpl (Metadata_t& mdata) { /* Initialize resources */ this->Initialize(); this->SetMetadata(mdata); return; };
    // Deleted - SubscriberImpl (char* name) { /* Initialize resources */ this->Initialize(); this->SetTopicName(name); return; };
    SubscriberImpl (Topic& topic) { /* Initialize resources */ this->Initialize(); /* WARNING - Topic externally instantiated should not be destroyed upon Participant destruction */ this->SetTopic(topic); return; };

    /* Destructor method */
    virtual ~SubscriberImpl (void);

};
#if 0
typedef class Subscriber_Asyn : public SubscriberImpl
{

  private:

    base::AnyThread* m_thread; /* Thread to manage subscriber instance */
    base::Statistics<uint64_t>* m_stats;

    void (* m_cb) (void*); /* Routine called upon topic reception */
    void* m_attr;         /* Routine attribute */

    /* Initializer methods */
    void Initialize (void);

  public:

    /* Initializer methods */
    RET_STATUS Launch (void);
    RET_STATUS Terminate (void);

    /* Accessor methods */
    RET_STATUS SetAffinity (uint_t core);
    RET_STATUS SetCallback (void (* cb)(void*)) { this->m_cb = cb; return STATUS_SUCCESS; };
    RET_STATUS SetCallback (void (* cb)(void*), void* attr) { this->m_attr = attr; this->m_cb = cb; return STATUS_SUCCESS; };
    RET_STATUS SetPriority (uint_t policy, uint_t priority);

    /* Miscellaneous methods */
    RET_STATUS Preamble (void) { return this->Configure(); };
    RET_STATUS Do (void);

    /* Constructor methods */
    Subscriber_Asyn (void) : SubscriberImpl() { /* Initialize resources */ this->Initialize(); return; };
    Subscriber_Asyn (Metadata_t& mdata) : SubscriberImpl(mdata) { /* Initialize resources */ this->Initialize(); return; };
    // Deleted - Subscriber_Asyn (char* name) : SubscriberImpl(name) { /* Initialize resources */ this->Initialize(); return; };
    Subscriber_Asyn (Topic& topic);

    /* Destructor method */
    virtual ~Subscriber_Asyn (void);

} SubscriberThread; /* For backward compatibility purposes with v1.0 */
#endif
/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace core */

} /* namespace sdn */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CORE_SUBSCRIBER_H */
