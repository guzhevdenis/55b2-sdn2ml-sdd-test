#ifndef SDN_MSG_H
#define SDN_MSG_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-msg.h $
* $Id: sdn-msg.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include <stdio.h> /* sscanf, printf, etc. */
#include <string.h> /* strncpy, etc. */

/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

/* Constants */

#define DISCOVERY_MAX_LENGTH 1024

#define sdn_msg sdn::Message /* For backward compatibility purposes with CCSv5.2 */
#define sdn_discovery_msg sdn::message::Discovery /* For backward compatibility purposes with CCSv5.2 */
#define sdn_query_msg sdn::message::Query /* For backward compatibility purposes with CCSv5.2 */

/* Type definition */

namespace sdn {

class Message
{

  private:

    bool m_allocated;
    uint8_t* m_instance;
    uint_t m_size;

    /* Initializer methods */
    void Initialize (void) { this->m_allocated = false; this->m_instance = NULL; this->m_size = 0u; return; };

  public:

    /* Initializer methods */
    virtual void ClearInstance (void) { if (this->m_instance != NULL) memset(this->m_instance, 0, this->m_size); return; };
    virtual RET_STATUS CopyInstance (void* instance) { if (this->m_instance != NULL) memcpy(this->m_instance, instance, this->m_size); return STATUS_SUCCESS; };
    virtual RET_STATUS CreateInstance (void) { this->DeleteInstance(); this->m_instance = new uint8_t [this->m_size]; this->m_allocated = true; return STATUS_SUCCESS; };
#if 0 /* Warning - Recursion risk - Specializing CreateInstance (void) and calling CreateInstance (void*, uint_t) */
    virtual RET_STATUS CreateInstance (void* instance, uint_t size) { this->SetSize(size); this->CreateInstance(); this->CopyInstance(instance); return STATUS_SUCCESS; };
#else
    virtual RET_STATUS CreateInstance (void* instance, uint_t size) { this->SetSize(size); this->DeleteInstance(); this->m_instance = new uint8_t [this->m_size]; this->m_allocated = true; this->CopyInstance(instance); return STATUS_SUCCESS; };
#endif
    virtual void DeleteInstance (void) { if ((this->m_allocated == true) && (this->m_instance != NULL)) delete [] this->m_instance; this->m_allocated = false; this->m_instance = NULL; return; };

    /* Accessor methods */
    virtual void* GetInstance (void) { return this->m_instance; };
    virtual void SetInstance (void* instance) { if (this->m_instance != instance) this->DeleteInstance(); this->m_instance = (uint8_t*) instance; return; };
    virtual void SetInstance (void* instance, uint_t size) { this->SetSize(size); this->SetInstance(instance); return; };

    uint_t GetSize (void) { return this->m_size; };
    void SetSize (uint_t size) { this->m_size = size; return; };

    /* Miscellaneous methods */

    /* Constructor methods */
    Message (void) { this->Initialize(); return; };

    /* Destructor method */
    virtual ~Message (void) { this->DeleteInstance(); return; };

};

namespace message {

class Discovery : public Message
{

  private:

    char m_copy [DISCOVERY_MAX_LENGTH];

    char* m_group;
    char* m_qual;
    char* m_host;
    char* m_part;
    char* m_role;
    char* m_topic;
    char* m_size;
    char* m_version;
    char* m_mcast_group;
    char* m_mcast_port;

  public:

    /* Initializer methods */

    /* Accessor methods */
    char* GetMsgGroup (void);
    char* GetQualifier (void);
    char* GetHostName (void);
    char* GetParticipant (void);
    char* GetRole (void);
    char* GetTopicName (void);
    //char* GetTopicVersion (void);
    uint_t GetTopicVersion (void);
    uint_t GetTopicSize (void);
    char* GetMCastGroup (void);
    uint_t GetMCastPort (void);

    /* Miscellaneous methods */
    bool IsValid (void);

    bool IsJoin (void);
    bool IsLeave (void);
    bool IsUpdate (void);
    bool IsQuery (void);
    bool IsPublisher (void);
    bool IsSubscriber (void);
    bool IsResponse (void);

    RET_STATUS Parse (const char* buffer);
    RET_STATUS Parse (void) { return this->Parse((const char*) this->GetInstance()); };

    /* Constructor methods */
    Discovery (void);

    /* Destructor method */
    virtual ~Discovery (void) {};

};

class Query : public Message
{

  private:

    char m_host_name [STRING_MAX_LENGTH]; /* SDN participant host name */
    char m_topic_name [STRING_MAX_LENGTH]; /* SDN topic name */

  public:

    /* Initializer methods */
    RET_STATUS CreateInstance (void); /* Warning - Recursion risk */

    /* Accessor methods */
    RET_STATUS SetHostName (const char* host);
    RET_STATUS SetTopicName (const char* topic);

    /* Miscellaneous methods */

    /* Constructor methods */
    Query (void);

    /* Destructor method */
    virtual ~Query (void) {};

};

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace message */

} /* namespace sdn */

#endif /* SDN_MSG_H */
