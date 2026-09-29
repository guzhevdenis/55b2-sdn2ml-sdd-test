#ifndef UCAST_PUBLISHER_H
#define UCAST_PUBLISHER_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/ucast-publisher.h $
* $Id: ucast-publisher.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include "ucast-participant.h"

/* Constants */

#define OBJTYPE_UCAST_PUBLISHER "ccs::ucast::PublisherImpl"

#ifdef __cplusplus

/* Type definition */

namespace ccs {

namespace ucast {

class PublisherImpl : public ParticipantImpl /* Implementation class */
{

  private:

    uint_t m_source_port;
    int m_ttl;

    /* Initializer methods */
    void Initialize (void) { this->m_source_port = 0u; this->m_ttl = -1; };

  public:

    /* Initializer methods */

    /* Accessor methods */
    void SetSourcePort (const uint_t port) { this->m_source_port = port; }; /* Bug 10809 - Publisher-side port */
    void SetTimeToLive (const uint8_t ttl) { this->m_ttl = ttl; }; /* Bug 14996 - TTL needs to be configurable */

    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) { return this->Publish(); };

    RET_STATUS Open (void);  
    RET_STATUS Close (void); 

    RET_STATUS Publish (const void* buffer, const uint_t size);
    RET_STATUS Publish (void);

    /* Constructor methods */
    PublisherImpl (void);
    PublisherImpl (const char* iface, const char* group, const uint_t port);

    /* Destructor method */
    virtual ~PublisherImpl (void); /* Note - Virtual destructor */

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

#endif /* UCAST_PUBLISHER_H */
