#ifndef SDN_CORE_H
#define SDN_CORE_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-core.h $
* $Id: sdn-core.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project	: CODAC Core System
*
* Description	: SDN - Infrastructure tools - Prototype
*
* Author        : Bertrand Bauvir
*
* Copyright (c) : 2010-2026 ITER Organization,
*				  CS 90 046
*				  13067 St. Paul-lez-Durance Cedex
*				  France
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

//#include "sdn-base.h" /* Favour PIMPL design pattern to avoid exposing the structure through the public interface */
//#include "sdn-mcast.h" /* SDN core library - API definition (sdn::mcast) */

#include "sdn-header.h"
#include "sdn-topic.h"
#include "sdn-packet.h"

//#include "core-participant.h" /* Favour PIMPL design pattern to avoid exposing the structure through the public interface */
//#include "core-publisher.h" /* Favour PIMPL design pattern to avoid exposing the structure through the public interface */
//#include "core-subscriber.h" /* Favour PIMPL design pattern to avoid exposing the structure through the public interface */

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {

namespace core {

class PublisherImpl;   /* Forward class declaration */
class SubscriberImpl;  /* Forward class declaration */

typedef class ParticipantIface
{

  public:

    Header* m_header; /* The instance of the class */ /* For backward compatibility purposes with v1.0 */ /* Deprecated */
    Topic* m_topic;   /* The instance of the class */ /* For backward compatibility purposes with v1.0 */ /* Deprecated */
    Packet* m_packet; /* The instance of the class */ /* For backward compatibility purposes with v1.0 */ /* Deprecated */

    /* Initializer methods */

    /* Accessor methods */
    virtual char* GetInterface (void) = 0;  /* Pure virtual method */
    virtual char* GetTopicName (void) = 0;  /* Pure virtual method */
    virtual uint_t GetTopicSize (void) = 0; /* Pure virtual method */

    virtual RET_STATUS SetInterface (const char* iface) = 0; /* Pure virtual method */
    // Deleted - virtual RET_STATUS SetTopicName (char* name) = 0; /* Pure virtual method */
    virtual RET_STATUS SetMetadata (Metadata_t& mdata) = 0; /* Pure virtual method */

    virtual bool IsInitialized (void) = 0; /* Pure virtual method */

    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size) = 0; /* Pure virtual method */
    virtual void* GetTopicHeader (void) = 0;   /* Pure virtual method */ /* The data payload and not the instance of the class */
    virtual void* GetTopicInstance (void) = 0; /* Pure virtual method */ /* The data payload and not the instance of the class */

    virtual RET_STATUS SetCallback (void (* cb)(void*)) = 0; /* Pure virtual method */ /* Routine called as part of message handling */
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr) = 0; /* Pure virtual method */ /* Routine called as part of message handling */

    virtual int GetSocket (void) const = 0; /* Pure virtual method */
    
    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) = 0; /* Pure virtual method */
    virtual RET_STATUS Configure (void) = 0; /* Pure virtual method */

    /* Constructor methods */
    ParticipantIface (void) { /* Initialize attributes */ this->m_header = NULL; this->m_topic = NULL; this->m_packet = NULL; return; };

    /* Destructor method */
    virtual ~ParticipantIface (void) {}; /* Note - virtual destructor */

} Participant; /* For backward compatibility purposes with v1.0 */

typedef class PublisherIface : public ParticipantIface
{

  private:

    PublisherImpl* p_impl;

  public:

    /* Initializer methods */

    /* Accessor methods */
    virtual char* GetInterface (void); /* Specializes virtual method */
    virtual char* GetTopicName (void); /* Specializes virtual method */
    virtual uint_t GetTopicSize (void); /* Specializes virtual method */

    virtual RET_STATUS SetInterface (const char* iface); /* Specializes virtual method */
    RET_STATUS SetInterface (const char* iface, uint_t port);
    // Deleted - virtual RET_STATUS SetTopicName (char* name); /* Specializes virtual method */
    virtual RET_STATUS SetMetadata (Metadata_t& mdata); /* Specializes virtual method */

    virtual bool IsInitialized (void); /* Specializes virtual method */

    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size); /* Specializes virtual method */
    virtual void* GetTopicHeader (void);   /* Specializes virtual method */ /* The data payload and not the instance of the class */
    virtual void* GetTopicInstance (void); /* Specializes virtual method */ /* The data payload and not the instance of the class */

    virtual RET_STATUS SetCallback (void (* cb)(void*)); /* Specializes virtual method */ /* Routine called before message publication */
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr); /* Specializes virtual method */ /* Routine called before message publication */

    virtual int GetSocket (void) const; /* Specializes virtual method */

    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) { return this->Publish(); }; /* Specializes virtual method */
    virtual RET_STATUS Configure (void); /* Specializes virtual method */

    RET_STATUS Publish (void);

    /* Constructor methods */
    PublisherIface (void);
    PublisherIface (Metadata_t& mdata);
    // Deleted - PublisherIface (char* name);
    PublisherIface (Topic& topic); /* WARNING - Topic externally instantiated should not be destroyed upon Participant destruction */ 

    /* Destructor method */
    virtual ~PublisherIface (void); /* Note - virtual destructor */

} Publisher; /* For backward compatibility purposes with v1.0 */ 

typedef class SubscriberIface : public ParticipantIface
{

  private:

    SubscriberImpl* p_impl;

  public:

    /* Initializer methods */

    /* Accessor methods */
    virtual char* GetInterface (void); /* Specializes virtual method */
    virtual char* GetTopicName (void); /* Specializes virtual method */
    virtual uint_t GetTopicSize (void); /* Specializes virtual method */

    virtual RET_STATUS SetInterface (const char* iface); /* Specializes virtual method */
    // Deleted - virtual RET_STATUS SetTopicName (char* name); /* Specializes virtual method */
    virtual RET_STATUS SetMetadata (Metadata_t& mdata); /* Specializes virtual method */

    virtual bool IsInitialized (void); /* Specializes virtual method */

    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size); /* Specializes virtual method */
    virtual void* GetTopicHeader (void);   /* Specializes virtual method */ /* The data payload and not the instance of the class */
    virtual void* GetTopicInstance (void); /* Specializes virtual method */ /* The data payload and not the instance of the class */

    virtual RET_STATUS SetCallback (void (* cb)(void*)); /* Specializes virtual method */ /* Routine called before message publication */
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr); /* Specializes virtual method */ /* Routine called before message publication */

    virtual int GetSocket (void) const; /* Specializes virtual method */

    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) { return this->Receive(); }; /* Specializes virtual method */
    virtual RET_STATUS Configure (void); /* Specializes virtual method */

    RET_STATUS Receive (uint64_t timeout); /* Blocking wait with configurable timeout - Buffer and size defined using sdn::mcast::Participant::SetBuffer() */
    RET_STATUS Receive (void);             /* Blocking wait - Buffer and size defined using sdn::mcast::Participant::SetBuffer() */

    /* Constructor methods */
    SubscriberIface (void);
    SubscriberIface (Metadata_t& mdata);
    // Deleted - SubscriberIface (char* name);
    SubscriberIface (Topic& topic);

    /* Destructor method */
    virtual ~SubscriberIface (void); /* Note - virtual destructor */

} Subscriber; /* For backward compatibility purposes with v1.0 */

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

#endif /* SDN_CORE_H */
