#ifndef CORE_PUBLISHER_H
#define CORE_PUBLISHER_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/core-publisher.h $
* $Id: core-publisher.h 179115 2026-01-09 15:15:20Z kimh19 $
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
//#include "mcast-publisher.h"

#include "core-participant.h"

#include "sdn-packet.h"
#include "sdn-header.h"
#include "sdn-topic.h"

/* Constants */

#define OBJTYPE_CORE_PUBLISHER (char*) "sdn::core::Publisher_Base"

#ifdef __cplusplus

/* Type definition */

namespace sdn {

namespace core {

class Publisher_Base : public ParticipantImpl /* Base class */
{

  private:

    /* Initializer methods */
    void Initialize (void) {};

  public:

    /* Initializer methods */

    /* Accessor methods */

    /* Miscellaneous methods */
    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size) { RET_STATUS status = STATUS_ERROR; if ((this->m_topic)->GetSize() >= size) { memcpy(this->GetTopicInstance(), instance, size); status = STATUS_SUCCESS; } return status; }; /* Specializes virtual method */
    virtual RET_STATUS Do (void) { return STATUS_SUCCESS; }; /* Specializes virtual method */
    virtual RET_STATUS Publish (void) = 0; /* Pure virtual method */

    /* Constructor methods */
    Publisher_Base (void);

    /* Destructor method */
   ~Publisher_Base (void) { return; };

}; 

class PublisherImpl : public Publisher_Base
{

  private:

    base::PublisherIface* m_base; /* Instantiate and publish to MCAST or UCAST socket */

    types::Endianness_t m_comm_order; /* Bug 10327 - Select byte order for messaging over the network */
    types::Endianness_t m_host_order;

    uint_t m_source_port; /* Bug 10809 - Select explicit source UDP port */
    int m_ttl; /* Bug 14996 - TTL needs to be configurable */

    /* Initializer methods */
    void Initialize (void);

  public:

    /* Initializer methods */

    /* Accessor methods */
    virtual int GetSocket (void) const { int sock = -1; if (this->m_base != NULL) sock = (this->m_base)->GetSocket(); return sock; }; /* Specializes virtual method */

    types::Endianness_t GetPayloadOrder (void) const { return this->m_comm_order; };
    types::Endianness_t SetPayloadOrder (types::Endianness_t order) { types::Endianness_t past = this->GetPayloadOrder(); this->m_comm_order = order; return past; };

    RET_STATUS SetInterface (const char* name, uint_t port) { this->m_source_port = port; return this->ParticipantImpl::SetInterface(name); };
    RET_STATUS SetTimeToLive (uint8_t ttl) { this->m_ttl = ttl; return STATUS_SUCCESS; };

    /* Miscellaneous methods */
    virtual RET_STATUS Configure (void);
    virtual RET_STATUS Publish (void);

    /* Constructor methods */
    PublisherImpl (void);
    PublisherImpl (Metadata_t& mdata) { /* Initialize resources */ this->Initialize(); this->SetMetadata(mdata); return; };
    // Deleted - PublisherImpl (char* name) { /* Initialize resources */ this->Initialize(); this->SetTopicName(name); return; };
    PublisherImpl (Topic& topic);

    /* Destructor method */
   ~PublisherImpl (void);

};
#if 0
typedef class Publisher_Asyn : public PublisherImpl
{

  private:

    base::AnyThread* m_thread; /* Thread to manage publisher instance */

    uint64_t m_accuracy;
    bool m_synchronous;  /* Publication is triggered when topic is externally updated */
    volatile bool m_trigger;  /* Publication is triggered when topic is externally updated */

    void (* m_cb) (void*); /* Routine called before topic publication  */
    void* m_attr;          /* Routine attribute */

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
    RET_STATUS SetPeriod (uint64_t period, int64_t phase = 0L, uint64_t accuracy = DEFAULT_THREAD_ACCURACY);
    RET_STATUS SetPriority (uint_t policy, uint_t priority);

    /* Miscellaneous methods */
    RET_STATUS Preamble (void) { return this->Configure(); };
    RET_STATUS Do (void);
    RET_STATUS Trigger (void) { return ((__sync_val_compare_and_swap(&(this->m_trigger), false, true)) ? STATUS_ERROR : STATUS_SUCCESS); };

    /* Constructor methods */
    Publisher_Asyn (void) : PublisherImpl() { /* Initialize resources */ this->Initialize(); return; };
    Publisher_Asyn (Metadata_t& mdata) : PublisherImpl(mdata) { /* Initialize resources */ this->Initialize(); return; };
    // Deleted - Publisher_Asyn (char* name) : PublisherImpl(name) { /* Initialize resources */ this->Initialize(); return; };
    Publisher_Asyn (Topic& topic) : PublisherImpl(topic) { /* Initialize resources */ this->Initialize(); return; };

    /* Destructor method */
   ~Publisher_Asyn (void);

} PublisherThread; /* For backward compatibility purposes with v1.0 */
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

#endif /* CORE_PUBLISHER_H */
