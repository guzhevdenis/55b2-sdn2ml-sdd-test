#ifndef SDN_QUERY_H
#define SDN_QUERY_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-tools/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-query.h $
* $Id: sdn-query.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include <common/log-api.h>  /* Logging API definition */

#include <common/tools.h> /* Misc. helper functions, e.g. hash, etc. */
#include <common/types.h> /* Global type definition */
#include <common/any-thread.h> /* Thread management class definition */ 

#include <sdn-core/sdn-mcast.h> /* SDN core library - API definition (sdn::mcast) */

/* Local header files */

/* Constants */

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sdn-query"

#define DEFAULT_QUERY_TIMEOUT 120000000000L /* 120 sec */

#define MAX_DISCOVERY_MSG_LENGTH 1024

#define sdn_query sdn::tool::Querier /* Maintained for backward compatibility */

/* Type definition */

namespace sdn {

namespace tool {

class Querier
{

  private:

    char   m_iface_name [STRING_MAX_LENGTH]; /* SDN interface name */
    char   m_targt_name [STRING_MAX_LENGTH]; /* SDN target to query */
    char   m_topic_name [STRING_MAX_LENGTH]; /* SDN topic name */
    char   m_mcast_group [MAX_IP_ADDR_LENGTH]; /* SDN discovery protocol */
    uint_t m_mcast_port;

    sdn::message::Query* m_msg;
    sdn::mcast::Publisher* m_pub;

    /* Initializer methods */
    void Initialize (void);

    /* Accessor methods */
    const char* GetMCastGroup (void) { return this->m_mcast_group; };
    uint_t GetMCastPort (void) { return this->m_mcast_port; };

   public:

    /* Initializer methods */

    /* Accessor methods */
    RET_STATUS SetInterface (const char* iface) { sstrncpy((char*) this->m_iface_name, iface, STRING_MAX_LENGTH); return STATUS_SUCCESS; };
    RET_STATUS SetHostName (const char* host) { sstrncpy((char*) this->m_targt_name, host, STRING_MAX_LENGTH); return STATUS_SUCCESS; };
    RET_STATUS SetTopic (const char* topic) { sstrncpy((char*) this->m_topic_name, topic, STRING_MAX_LENGTH); return STATUS_SUCCESS; };

    const char* GetInterface (void) { return (char*) this->m_iface_name; };
    const char* GetHostName (void) { return (char*) this->m_targt_name; };
    const char* GetTopic (void) { return (char*) this->m_topic_name; };

    /* Miscellaneous methods */
    RET_STATUS Configure (void);
    RET_STATUS Publish (void);

    /* Constructor methods */
    Querier (void) { this->Initialize(); this->SetInterface(DEFAULT_IFACE_NAME); this->SetHostName(STRING_UNDEFINED); this->SetTopic(STRING_UNDEFINED); return; };
    Querier (const char* iface) { this->Initialize(); this->SetInterface(iface); this->SetHostName(STRING_UNDEFINED); this->SetTopic(STRING_UNDEFINED); this->Configure(); return; };
    Querier (const char* iface, const char* topic) { this->Initialize(); this->SetInterface(iface); this->SetHostName(STRING_UNDEFINED); this->SetTopic(topic); this->Configure(); return; };
    Querier (const char* iface, const char* host, const char* topic) { this->Initialize(); this->SetInterface(iface); this->SetHostName(host); this->SetTopic(topic); this->Configure(); return; };

    /* Destructor method */
    virtual ~Querier (void);

};

 class QuerierThread : public Querier
{

  private:

    ccs::base::AnyThread* m_pub_thread;

    /* Initializer methods */
    void Initialize (void);

   public:

    /* Initializer methods */

    /* Accessor methods */

    /* Miscellaneous methods */
    RET_STATUS Configure (void);

    /* Constructor methods */
    QuerierThread (void) { this->Initialize(); return; };

    /* Destructor method */
    virtual ~QuerierThread (void);

};

/* Global variables */

/* Function declaration */

/* Function definition */

void QuerierThread_Publish_CB (Querier* self)
{

  log_trace("Executing '%s' routine", __FUNCTION__);

  if (self->Publish() != STATUS_SUCCESS)
    {
      log_warning("Unable to publish on '%s'", self->GetInterface());
    }
  else
    {
      log_info("Renew SDN topic membership information on '%s'", self->GetInterface());
    }

  log_trace("Leaving '%s' routine", __FUNCTION__);

};

/* Initializer methods */

void Querier::Initialize (void) 
{ 

  log_trace("Querier::Initialize - Entering method");

  /* Define default attributes */
  sstrncpy(this->m_mcast_group, DEFAULT_DISCOVERY_MCAST_GROUP, MAX_IP_ADDR_LENGTH);
  this->m_mcast_port = DEFAULT_DISCOVERY_MCAST_PORT;

  /* Initialize resources */
  this->m_msg = NULL;
  this->m_pub = NULL;

  log_trace("Querier::Initialize - Leaving method");

  return;

};

void QuerierThread::Initialize (void) 
{ 

  log_trace("QuerierThread::Initialize - Entering method");

  /* Initialize resources */
  this->m_pub_thread = NULL; 

  log_trace("QuerierThread::Initialize - Leaving method");

  return;

};

/* Accessor methods */

/* Miscellaneous methods */

RET_STATUS Querier::Configure (void) 
{ 

  log_trace("Querier::Configure - Entering method");

  RET_STATUS status = STATUS_ERROR;

  /* Instantiate resources */
  this->m_msg = new sdn::message::Query ();

  if (IsUndefined(this->GetHostName()) != true) (this->m_msg)->SetHostName(this->GetHostName());
  if (IsUndefined(this->GetTopic()) != true) (this->m_msg)->SetTopicName(this->GetTopic());

  (this->m_msg)->CreateInstance();

  this->m_pub = new sdn::mcast::Publisher (this->m_iface_name, this->GetMCastGroup(), this->GetMCastPort()); 
  (this->m_pub)->SetBuffer((this->m_msg)->GetInstance(), (this->m_msg)->GetSize());

  status = STATUS_SUCCESS;

  log_trace("Querier::Configure - Leaving method");

  return status;

};

RET_STATUS QuerierThread::Configure (void) 
{ 

  log_trace("QuerierThread::Configure - Entering method");

  RET_STATUS status = this->Querier::Configure();

  /* Instantiate resources */
  this->m_pub_thread = new ccs::base::AnyThread ("SDN Query");

  (this->m_pub_thread)->SetCallback((void (*)(void*)) &QuerierThread_Publish_CB, (void*) this); 
  (this->m_pub_thread)->SetAccuracy(10000000L); 
  (this->m_pub_thread)->SetPeriod(DEFAULT_QUERY_TIMEOUT); 
  (this->m_pub_thread)->Launch(); 

  status = STATUS_SUCCESS;

  log_trace("QuerierThread::Configure - Leaving method");

  return status;

};

RET_STATUS Querier::Publish (void) 
{ 

  RET_STATUS status = STATUS_ERROR; 

  log_info("Querier::Publish - Publish message '%s'", (this->m_msg)->GetInstance()); 

  if (this->m_pub != NULL) status = (this->m_pub)->Publish(); 

  return status; 

};

/* Constructor methods */

/* Destructor method */

Querier::~Querier (void) 
{ 

  log_trace("Querier::~Querier - Entering method");

  /* Release resources */
  //if (this->m_pub_thread != NULL) delete this->m_pub_thread; 
  if (this->m_pub != NULL) delete this->m_pub;
  if (this->m_msg != NULL) delete this->m_msg;

  log_trace("Querier::~Querier - Leaving method");

  return; 

};

QuerierThread::~QuerierThread (void) 
{ 

  log_trace("Querier::~Querier - Entering method");

  /* Release resources */
  if (this->m_pub_thread != NULL) delete this->m_pub_thread; 

  log_trace("QuerierThread::~QuerierThread - Leaving method");

  return; 

};

}; /* namespace tool */

}; /* namespace sdn */

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC NULL

#endif
