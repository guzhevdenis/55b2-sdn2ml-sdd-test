#ifndef MCAST_PARTICIPANT_H
#define MCAST_PARTICIPANT_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/mcast-participant.h $
* $Id: mcast-participant.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project	: CODAC Core System
*
* Description	: Infrastructure tools - Prototype
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

#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */
#include "sdn-base.h"

#include "mcast-constants.h" /* Constants valid for this scope */

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace ccs {

namespace mcast {

class ParticipantImpl : public base::AnyObject
{

  private:

    /* Initializer methods */
    void Initialize (void);

  public:

    void* m_buffer;
    uint_t m_size;

    uint_t m_depth; // Bug 10920 - Socket depth in bytes

    char   m_mcast_group [MaxIPv4AddrLength];
    uint_t m_mcast_port;

    char m_if_addr [MaxIPv4AddrLength];
    char m_if_name [MaxIPv4AddrLength];

    int m_socket; /* Socket handle */

    void (* m_cb) (void*); /* Routine called as part of message handling */
    void* m_attr;          /* Routine attribute */

    /* Initializer methods */

    /* Accessor methods */
    const char* GetAddress (void) const { return (const char*) this->m_if_addr; };
    const char* GetInterface (void) const { return (const char*) this->m_if_name; };
    const char* GetMCastAddr (void) const { return (const char*) this->m_mcast_group; };
    uint_t GetMCastPort (void) const { return this->m_mcast_port; };

    void SetInterface (const char* iface);
    void SetMCastAddr (const char* addr);
    void SetMCastPort (const uint_t port);
    void SetBufferDepth (const uint_t depth) { this->m_depth = depth; }; // Bug 10920 - Socket depth in bytes

    void* GetBuffer (void) const { return this->m_buffer; };
    uint_t GetSize (void) const { return this->m_size; };
    void SetBuffer (void* buffer, const uint_t size);

    RET_STATUS SetCallback (void (* cb)(void*)) { this->m_cb = cb; return STATUS_SUCCESS; }; /* Routine called as part of message handling */
    RET_STATUS SetCallback (void (* cb)(void*), void* attr) { this->m_attr = attr; this->m_cb = cb; return STATUS_SUCCESS; }; /* Routine called as part of message handling */

    int GetSocket (void) const { return this->m_socket; };

    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) = 0; /* Pure virtual method */

    /* Constructor methods */
    ParticipantImpl (void);

    /* Destructor method */
    virtual ~ParticipantImpl (void); /* Note - Virtual destructor */

};

/* Global variables */

/* Function declaration */

RET_STATUS Participant_Open (ParticipantImpl* self); /* Common actions */
RET_STATUS Participant_Close (ParticipantImpl* self); /* Common actions */

/* Function definition */

} /* namespace mcast */

} /* namespace ccs */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MCAST_PARTICIPANT_H */
