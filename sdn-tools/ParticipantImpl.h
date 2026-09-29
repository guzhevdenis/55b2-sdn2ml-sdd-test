/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/ParticipantImpl.h $
* $Id: ParticipantImpl.h 179115 2026-01-09 15:15:20Z kimh19 $
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
 * @file ParticipantImpl.h
 * @brief Header file for sdn::Participant implementation class
 * @date 18/04/2018
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 *
 * @detail This header file contains the definition of the Participant
 * implementation class.
 */

#ifndef _ParticipantImpl_h_
#define _ParticipantImpl_h_

/* Global header files */

/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

#include "sdn-base.h" /* Base classes definition */

#include "core-participant.h"

#include "disc-messenger.h"

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {

class ParticipantImpl : public ccs::base::AnyObject
{
        
  private:
        
    /* Initialiser methods */
    void Initialise (void);
        
  public:
        
    core::ParticipantImpl* m_base; /* Reference to core::Participant */
    disc::MessengerImpl* m_disc; /* Instantiate and manages disc::Messenger */

    /* Initialiser methods */
        
    /* Accessor methods */
    bool IsInitialized (void) { return (this->m_base)->IsInitialized(); };
        
    char* GetInterface (void) { return (this->m_base)->GetInterface(); };
    char* GetTopicName (void) { return (this->m_base)->GetTopicName(); };
    uint_t GetTopicSize (void) { return (this->m_base)->GetTopicSize(); };
        
    RET_STATUS SetInterface (const char* name);
    RET_STATUS SetMetadata (Metadata_t& mdata);
    RET_STATUS SetTopic (Topic& topic);
    RET_STATUS SetBufferDepth (uint_t depth);

    RET_STATUS CopyTopicInstance (void* instance, uint_t size) { RET_STATUS status = STATUS_ERROR; if (this->m_base != NULL) status = (this->m_base)->CopyTopicInstance(instance, size); return status; };

    void* GetTopicHeader (void) { void* ref = NULL; if (this->m_base != NULL) ref = (this->m_base)->GetTopicHeader(); return ref; }; /* The data payload and not the instance of the class */
    void* GetTopicInstance (void) { void* ref = NULL; if (this->m_base != NULL) ref = (this->m_base)->GetTopicInstance(); return ref; }; /* The data payload and not the instance of the class */
        
    RET_STATUS SetCallback (void (* cb)(void*)) { RET_STATUS status = STATUS_ERROR; if (this->m_base != NULL) status = (this->m_base)->SetCallback(cb); return status; };
    RET_STATUS SetCallback (void (* cb)(void*), void* attr) { RET_STATUS status = STATUS_ERROR; if (this->m_base != NULL) status = (this->m_base)->SetCallback(cb, attr); return status; };

    int GetSocket (void) { int sock = -1; if (this->m_base != NULL) sock = (this->m_base)->GetSocket(); return sock; };

    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) = 0;        /* Pure virtual method */
    virtual RET_STATUS Configure (void) = 0; /* Pure virtual method */
        
    virtual RET_STATUS DoBackgroundActivity (void); /* Default implementation to manage disc::Messenger interface */

    /* Constructor methods */
    ParticipantImpl (void);
        
    /* Destructor method */
    virtual ~ParticipantImpl (void);
        
    /* Display methods */
        
};
    
/* Global variables */

/* Function declaration */

RET_STATUS Participant_Configure (ParticipantImpl* self);

/* Function definition */

} /* namespace sdn */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ParticipantImpl_h_ */
