#ifndef CORE_PARTICIPANT_H
#define CORE_PARTICIPANT_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/core-participant.h $
* $Id: core-participant.h 179115 2026-01-09 15:15:20Z kimh19 $
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

/*
  ToDo - Topic externally instantiated should not be destroyed upon Participant destruction 
*/

/* Global header files */

/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

#include "sdn-base.h" /* Privately scoped base classes definition */

#include "sdn-topic.h"
#include "sdn-packet.h"

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {

namespace core {

class ParticipantImpl : public ccs::base::AnyObject
{

  private:

    char m_iface_name [STRING_MAX_LENGTH];
#if 0
    char m_topic_name [STRING_MAX_LENGTH];
#endif
    /* Initializer methods */
    void Initialize (void);

  public:

    Metadata_t m_mdata; /* To collect all metadata information */

    bool m_ext_topic; /* Topic externally provided */

    Packet* m_packet;
    Topic* m_topic;

    uint_t m_depth;

    void (* m_cb) (void*); /* Routine called as part of message handling */
    void* m_attr;          /* Routine attribute */

    /* Initializer methods */

    /* Accessor methods */
    bool IsInitialized (void) { return (this->m_topic)->IsInitialized(); };

    char* GetInterface (void) { return (char*) this->m_iface_name; };
    char* GetTopicName (void) { return (char*) this->m_mdata.name; };
    uint_t GetTopicSize (void) { return this->m_mdata.size; };

    RET_STATUS SetInterface (const char* name);
    RET_STATUS SetMetadata (Metadata_t& mdata);
    // Deleted - RET_STATUS SetTopicName (const char* name);
    RET_STATUS SetTopic (Topic& topic);
    RET_STATUS SetBufferDepth (uint_t depth);

    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size) = 0; /* Pure virtual method */ /* The data payload and not the instance of the class */
    void* GetTopicHeader (void) { return (void*) ((this->m_packet != NULL) ? ((this->m_packet)->GetHeader())->GetInstance() : NULL); }; /* The data payload and not the instance of the class */
    void* GetTopicInstance (void) { return (void*) ((this->m_packet != NULL) ? (this->m_packet)->GetPayload() : NULL); }; /* The data payload and not the instance of the class */

    RET_STATUS SetCallback (void (* cb)(void*)) { this->m_cb = cb; return STATUS_SUCCESS; }; /* Routine called as part of message handling */
    RET_STATUS SetCallback (void (* cb)(void*), void* attr) { this->m_attr = attr; this->m_cb = cb; return STATUS_SUCCESS; }; /* Routine called as part of message handling */

    virtual int GetSocket (void) const = 0; /* Pure virtual method */

    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) = 0;        /* Pure virtual method */
    virtual RET_STATUS Configure (void) = 0; /* Pure virtual method */

    /* Constructor methods */
    ParticipantImpl (void);

    /* Destructor method */
    virtual ~ParticipantImpl (void); /* Virtual method */

    /* Display methods */

};

/* Global variables */

/* Function declaration */

RET_STATUS Participant_Configure (ParticipantImpl* self);

/* Function definition */

} /* namespace core */

} /* namespace sdn */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CORE_PARTICIPANT_H */
