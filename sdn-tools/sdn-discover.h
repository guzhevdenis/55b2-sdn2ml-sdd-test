#ifndef SDN_DISCOVER_H
#define SDN_DISCOVER_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-tools/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-discover.h $
* $Id: sdn-discover.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include <common/LookUpTable.h> /* LookUpTable class definition */
#include <common/any-thread.h> /* Thread management class definition */ 

#include <sdn-core/sdn-mcast.h> /* SDN core library - API definition (sdn::mcast) */

/* Local header files */

/* Constants */

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sdn-discover"

#define DEFAULT_DISCOVERY_TIMEOUT 100000000L /* 10Hz */
#define MAX_DISCOVERY_MSG_LENGTH 1024

/* Type definition */

typedef struct MCastInfo {

  char   group [MAX_IP_ADDR_LENGTH];
  uint_t port;

} MCastInfo_t;

typedef struct ParticipantInfo : public sdn::Metadata_t {

  bool live;
  bool role;
  char host [STRING_MAX_LENGTH]; /* Host machine */
  char part [STRING_MAX_LENGTH]; /* Participant */

} ParticipantInfo_t;

class sdn_discover
{

  private:

    char   m_iface_name [STRING_MAX_LENGTH]; /* SDN interface name */
    MCastInfo_t m_mcast; /* SDN discovery channel */

    /* Topic list */
    ccs::base::LookUpTable<ParticipantInfo_t>* m_part_list;

    /* Notification callback */
    void (* m_cb) (ParticipantInfo_t&, void*); void* m_attr;

    ccs::base::AnyThread* m_sub_thread;

    /* Initializer methods */
    void Initialize (void);

    /* Accessor methods */
    char* GetMCastGroup (void) { return (char*) this->m_mcast.group; };
    uint16_t GetMCastPort (void) { return this->m_mcast.port; };

    char* GetInterface (void) { return (char*) this->m_iface_name; };
    RET_STATUS SetInterface (char* name) { strncpy((char*) this->m_iface_name, name, STRING_MAX_LENGTH); return STATUS_SUCCESS; };

   public:

    sdn::message::Discovery* m_msg;
    sdn::mcast::Subscriber* m_sub;

    /* Initializer methods */

    /* Accessor methods */
    RET_STATUS SetCallback(void (* cb) (ParticipantInfo_t&, void*)) { this->m_cb = cb; return STATUS_SUCCESS; };
    RET_STATUS SetCallback(void (* cb) (ParticipantInfo_t&, void*), void* attr) { this->m_cb = cb; this->m_attr = attr; return STATUS_SUCCESS; };

    /* Miscellaneous methods */
    void Notify (void);

    /* Constructor methods */
    sdn_discover (void) { this->SetInterface((char*) DEFAULT_IFACE_NAME); this->Initialize(); return; };
    sdn_discover (char* name) { this->SetInterface((char*) name); this->Initialize(); return; };

    /* Destructor method */
   ~sdn_discover (void);

    /* Display methods */

};

/* Global variables */

/* Function declaration */

/* Function definition */

void SDNDiscover_SUBThread_CB (sdn_discover* self)
{

  log_trace("Executing '%s' routine", __FUNCTION__);

  (self->m_msg)->ClearInstance();

  uint_t recvsize = (self->m_msg)->GetSize();

  if (self->m_sub == NULL)
    {    
      wait(DEFAULT_DISCOVERY_TIMEOUT); 
      return;
    }

  if ((self->m_sub)->Receive((self->m_msg)->GetInstance(), recvsize, DEFAULT_DISCOVERY_TIMEOUT) != STATUS_SUCCESS) /* Blocking call with timeout */
    {
      log_debug("SDN Discovery - No message over '%d' time", DEFAULT_DISCOVERY_TIMEOUT);
      return;
    }
  else
    {
      log_debug("SDN Discovery - Received message '%s' of size '%u'", (self->m_msg)->GetInstance(), recvsize);
    }

  if ((self->m_msg)->IsValid() != true)
    {
      log_warning("SDN Discovery - Received unknown message '%s'", (self->m_msg)->GetInstance());
      return;
    }
  else
    {
      log_debug("SDN Discovery - Message is valid");
    }

  if ((self->m_msg)->IsQuery() == true)
    {
      log_info("SDN Discovery - Discard query message on '%s'", (self->m_msg)->GetTopicName());
      return;
    }
  else
    {
      log_debug("SDN Discovery - Not a query message");
    }

    {
      /* Notify, as necessary */
      self->Notify();
    }

  log_trace("Leaving '%s' routine", __FUNCTION__);

};

/* Initializer methods */

void sdn_discover::Initialize (void) 
{ 

  log_trace("sdn_discover::Initialize - Entering method");

  /* Define default attributes */
  strncpy(this->m_mcast.group, (char*) DEFAULT_DISCOVERY_MCAST_GROUP, MAX_IP_ADDR_LENGTH);
  this->m_mcast.port = DEFAULT_DISCOVERY_MCAST_PORT;

  this->m_cb = NULL; this->m_attr = NULL;

  /* Initialize resources */
  this->m_msg = new sdn::message::Discovery; (this->m_msg)->CreateInstance(); // Bug 10226 - Default size part of the class
  this->m_sub = new sdn::mcast::Subscriber (this->m_iface_name, this->GetMCastGroup(), this->GetMCastPort());
  this->m_part_list = new ccs::base::LookUpTable<ParticipantInfo_t> ();
  this->m_sub_thread = new ccs::base::AnyThread ((char*) "SDN Discovery"); (this->m_sub_thread)->SetCallback((void (*)(void*)) &SDNDiscover_SUBThread_CB, (void*) this); (this->m_sub_thread)->SetPeriod(0); (this->m_sub_thread)->Launch(); /* Blocking callback */

  log_trace("sdn_discover::Initialize - Leaving method");

  return;

};

/* Accessor methods */

/* Miscellaneous methods */

void sdn_discover::Notify (void) 
{

  log_trace("sdn_discover::Notify - Entering method");

  ParticipantInfo_t info; info.live = false; 
#if 1
  bool notify = false;
#else
  bool notify = true; /* Bug 7817 - Notify in all cases for the time being - Use case is multiple start of sdn-sample application (same process, same topic, same role) */
#endif
  char keyword [STRING_MAX_LENGTH] = STRING_UNDEFINED;

  snprintf((char*) keyword, STRING_MAX_LENGTH, "%s:%s@%s", (this->m_msg)->GetParticipant(), (this->m_msg)->GetTopicName(), (this->m_msg)->GetHostName());

  /* Newly discovered topic */
  if (!(this->m_part_list)->IsValid(keyword))
    {
      log_info("Register '%s' participant for future reference", (char*) keyword); 
      (this->m_part_list)->Register(keyword, info);
    }
    
  (this->m_part_list)->GetElement(keyword, info);

  if (((this->m_msg)->IsJoin() == true) || ((this->m_msg)->IsResponse() == true))
    {
      /* The message includes interesting information */
      sstrncpy(info.name, (this->m_msg)->GetTopicName(), STRING_MAX_LENGTH); 
      //sstrncpy(info.vers, (this->m_msg)->GetTopicVersion(), STRING_MAX_LENGTH); 
      info.vers = (this->m_msg)->GetTopicVersion();
      info.size = (this->m_msg)->GetTopicSize();
      sstrncpy(info.mcast_group, (this->m_msg)->GetMCastGroup(), MAX_IP_ADDR_LENGTH); info.mcast_port = (this->m_msg)->GetMCastPort();

      info.role = (this->m_msg)->IsPublisher();
      sstrncpy(info.host, (this->m_msg)->GetHostName(), STRING_MAX_LENGTH); 
      sstrncpy(info.part, (this->m_msg)->GetParticipant(), STRING_MAX_LENGTH); 
    }

  /* Check change of state */
  if ((info.live == false) && (((this->m_msg)->IsJoin() == true) || ((this->m_msg)->IsResponse() == true)))
    { 
      log_info("Participant '%s' joined '%s' as '%s'", (this->m_msg)->GetParticipant(), (this->m_msg)->GetTopicName(), (((this->m_msg)->IsPublisher() == true) ? "publisher" : "subscriber")); 
      info.live = true; notify = true; (this->m_part_list)->SetElement(keyword, info); 
    }
#if 1
  else if ((info.live == true) && ((this->m_msg)->IsJoin() == true) && ((this->m_msg)->IsResponse() == false)) 
    { 
      log_warning("Participant joined '%s' - Probably crashed before", (this->m_msg)->GetTopicName()); 
      if (this->m_cb != NULL) { log_info("Notify silent termination"); info.live = false; (*(this->m_cb))(info, this->m_attr); }
      info.live = true; notify = true; (this->m_part_list)->SetElement(keyword, info); 
    }
#endif
  else if ((info.live == true) && ((this->m_msg)->IsLeave() == true)) 
    { 
      log_info("Participant '%s' has left '%s'", (this->m_msg)->GetParticipant(), (this->m_msg)->GetTopicName()); 
      info.live = false; notify = true; (this->m_part_list)->SetElement(keyword, info); 
    }

  if ((notify == true) && (this->m_cb != NULL)) { log_info("Notify registered parties"); (*(this->m_cb))(info, this->m_attr); }

  log_trace("sdn_discover::Notify - Leaving method");

  return;

};

/* Constructor methods */

/* Destructor method */

sdn_discover::~sdn_discover (void) 
{ 

  log_trace("sdn_discover::~sdn_discover - Entering method");

  /* Release resources */
  if (this->m_sub_thread != NULL) delete this->m_sub_thread; 
  if (this->m_part_list != NULL) delete this->m_part_list;
  if (this->m_sub != NULL) delete this->m_sub;
  if (this->m_msg != NULL) delete this->m_msg;

  log_trace("sdn_discover::~sdn_discover - Leaving method");

  return; 

};

/* Display methods */

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC NULL

#endif
