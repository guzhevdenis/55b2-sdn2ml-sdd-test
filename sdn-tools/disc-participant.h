#ifndef DISC_PARTICIPANT_H
#define DISC_PARTICIPANT_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/disc-participant.h $
* $Id: disc-participant.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include "sdn-base.h" /* Base classes definition */

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {

namespace disc {

class Participant : public ccs::base::AnyObject
{

  private:

    uint8_t* m_buffer;
    uint_t m_size;

    char   m_mcast_group [MAX_IP_ADDR_LENGTH];
    uint_t m_mcast_port;

    char m_if_name [MAX_IP_ADDR_LENGTH];

    /* Initializer methods */
    void Initialize (void);

  public:

    char m_host_name [STRING_MAX_LENGTH];
    char m_prog_name [STRING_MAX_LENGTH];
    uint_t m_prog_id;

    /* Initializer methods */

    /* Accessor methods */
    char* GetInterface (void) { return (char*) this->m_if_name; };
    char* GetMCastGroup (void) { return (char*) this->m_mcast_group; };
    uint_t GetMCastPort (void) { return this->m_mcast_port; };
    void SetInterface (const char* iface);
    void SetMCastGroup (const char* group);
    void SetMCastPort (uint_t port);

    void* GetBuffer (void) { return (void*) this->m_buffer; };
    uint_t GetSize (void) { return this->m_size; };

    /* Miscellaneous methods */
    virtual RET_STATUS Configure (void) = 0; /* Pure virtual method */
    virtual RET_STATUS Do (void) = 0;        /* Pure virtual method */

    /* Constructor methods */
    Participant (void);

    /* Destructor method */
    virtual ~Participant (void); /* Note - virtual destructor */

};

/* Global variables */

/* Function declaration */

RET_STATUS Participant_Configure (Participant* self);
bool Participant_IsQuery (const char* p_msg);

/* Function definition */

} /* namespace disc */

} /* namespace sdn */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DISC_PARTICIPANT_H */
