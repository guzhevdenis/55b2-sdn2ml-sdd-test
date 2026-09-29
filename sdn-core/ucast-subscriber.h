#ifndef UCAST_SUBSCRIBER_H
#define UCAST_SUBSCRIBER_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/ucast-subscriber.h $
* $Id: ucast-subscriber.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project	: CODAC Core System
*
* Description	: Infrastructure tools - Prototype
*
* Author        : Bertrand Bauvir
*
* Copyright (c) : 2010-2026 ITER Organization,
*                 CS 90 046
*                 13067 St. Paul-lez-Durance Cedex
*                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

/* Global header files */

/* Local header files */

#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

#include "ucast-participant.h"

/* Constants */

#define OBJTYPE_UCAST_SUBSCRIBER "ccs::ucast::SubscriberImpl"

#ifdef __cplusplus

/* Type definition */

namespace ccs {

namespace ucast {

/* 
   Documentation - ccs::ucast::SubscriberImpl::SetCallback() used in case Buffer and size defined using ccs::ucast::Participant::SetBuffer()
                   and in case of successful receive. This is meant to be useful for 1. the case where subscribers are being aggregated in a
                   composite class, and 2. in case of multirate messages.
 */

class SubscriberImpl : public ParticipantImpl /* Implementation class */
{

  private:

    volatile bool m_blocking;
    uint64_t m_timeout;

    volatile bool m_updated; /* Buffer has been updated ... */
    uint_t m_recvsize;       /* ... with size */

    /* Initializer methods */
    void Initialize (void) { this->m_blocking = false; this->m_timeout = 0; this->m_updated = false; this->m_recvsize = 0u; return; };

  public:

    /* Initializer methods */

    /* Accessor methods */
    uint64_t GetTimeout (void) const { return this->m_timeout; };
    void SetTimeout (const uint64_t timeout) { this->m_blocking = true; this->m_timeout = timeout; return; };

    bool IsUpdated (void) { return __sync_val_compare_and_swap(&this->m_updated, true, true); }; /* Successful Receive */
    uint_t GetReceivedSize (void) const { return this->m_recvsize; };

    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) { return this->Receive(); };

    RET_STATUS Open (void);  
    RET_STATUS Close (void); 

    RET_STATUS Receive (void* buffer, uint_t& size, const uint64_t timeout); /* Blocking wait with configurable timeout - Size updated with actual number of bytes received */
    RET_STATUS Receive (void* buffer, uint_t& size);                         /* Blocking wait - Size updated with actual number of bytes received */

    RET_STATUS Receive (const uint64_t timeout); /* Blocking wait with configurable timeout - Buffer and size defined using sdn::ucast::Participant::SetBuffer() */
    RET_STATUS Receive (void);                   /* Blocking wait - Buffer and size defined using sdn::ucast::Participant::SetBuffer() */

    /* Constructor methods */
    SubscriberImpl (void);
    SubscriberImpl (const char* iface, const char* addr, const uint_t port);

    /* Destructor method */
    virtual ~SubscriberImpl (void); /* Note - Virtual destructor */

};

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace ucast */

} /* namespace ccs */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UCAST_SUBSCRIBER_H */
