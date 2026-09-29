#ifndef SDN_RECORD_H
#define SDN_RECORD_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-tools/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-record.h $
* $Id: sdn-record.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include <log.h> /* CCS logging library */

#include <common/tools.h> /* Misc. helper functions, e.g. hash, etc. */
#include <common/types.h> /* Global type definition */
#include <common/buffer.h> /* Memory management class definition */
#include <common/any-thread.h> /* Thread management class definition */ 

#include <sdn-core/sdn-api.h> /* SDN core library - API definition */
#include <sdn-core/sdn-packet.h>
#include <sdn-core/sdn-topic-xml.h> /* SDN core library - XML topic definition file support */

/* Local header files */

/* Constants */

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sdn-record"

#define DEFAULT_FIFO_DEPTH 20
#define DEFAULT_RECORD_COUNT 1000

#undef DEFAULT_THREAD_PERIOD
#define DEFAULT_THREAD_PERIOD 1000000L /* 1kHz */

#define DEFAULT_RECORD_PATH "/tmp/sdn-record"

#define SDN_RECORD_PATH_ENVAR "SDN_RECORD_PATH"

/* Type definition */

typedef enum sdn_record_state {

  SDNRECORD_STATE_UNDEF = 0,
  SDNRECORD_STATE_ERROR,
  SDNRECORD_STATE_IDLE,
  SDNRECORD_STATE_INIT,
  SDNRECORD_STATE_RUNNING,

} sdn_record_state_t;

class sdn_record
{

  private:

    char m_file_name [PATH_MAX_LENGTH];

    sdn_record_state_t m_state;
    sdn::Metadata_t m_topic_meta; /* To collect all metadata information */

    ccs::base::AnyThread* m_dpc_thread;
    ccs::base::AnyThread* m_sub_thread;

    FILE* m_file;

    /* Initializer methods */
    void Initialize (void);

    /* Accessor methods */
    void SetName (char* name) { sstrncpy((char*) this->m_topic_meta.name, name, STRING_MAX_LENGTH); sstrncpy((char*) this->m_topic_name, name, STRING_MAX_LENGTH); return; };

    /* Miscellaneous methods */
    RET_STATUS Open (void);
    RET_STATUS Close (void);

   public:

    char m_topic_name [STRING_MAX_LENGTH]; /* Topic name */
    char m_iface_name [STRING_MAX_LENGTH]; /* SDN interface name */

    BlockMemory_FIFO* m_fifo; /* FIFO holding the payload to be recorded */

    sdn::Packet* m_packet;
    sdn::Header* m_header; 
    sdn::Topic*  m_topic;

    int m_count;

    sdn::Subscriber* m_sub;

    /* Initializer methods */

    /* Accessor methods */
    sdn_record_state_t GetState (void) { return this->m_state; };
    RET_STATUS SetState (sdn_record_state_t state) { this->m_state = state; return STATUS_SUCCESS; };

    RET_STATUS SetCount (int count) { this->m_count = count; return STATUS_SUCCESS; };

    RET_STATUS SetInterface (char* name) { sstrncpy((char*) this->m_iface_name, name, STRING_MAX_LENGTH); return STATUS_SUCCESS; };
    RET_STATUS SetTopic (char* name) { this->SetName(name); /* if (this->m_topic != NULL) delete this->m_topic; this->m_topic = new sdn_topic (this->m_topic_name); */ return STATUS_SUCCESS; };
    RET_STATUS SetTopic (sdn::Metadata_t& mdata) { this->m_topic_meta = mdata; this->SetName(mdata.name); return STATUS_SUCCESS; };

    /* Miscellaneous methods */
    RET_STATUS Configure (void);
    RET_STATUS Start (void) { RET_STATUS status = STATUS_ERROR; if (this->GetState() == SDNRECORD_STATE_IDLE) status = this->SetState(SDNRECORD_STATE_RUNNING); return status; };
    RET_STATUS Stop (void) { RET_STATUS status = STATUS_ERROR; if (this->GetState() == SDNRECORD_STATE_RUNNING) status = this->SetState(SDNRECORD_STATE_IDLE); return status; };

    RET_STATUS Write (void); /* Public required since called from within a thread */
    RET_STATUS Write (char* buffer);

    /* Constructor methods */
    sdn_record (void) { this->SetInterface((char*) DEFAULT_IFACE_NAME); this->SetTopic((char*) DEFAULT_TOPIC_NAME); this->Initialize(); return; };
    sdn_record (char* topic_name) { this->SetInterface((char*) DEFAULT_IFACE_NAME); this->SetTopic((char*) topic_name); this->Initialize(); return; };
    sdn_record (char* iface_name, char* topic_name) { this->SetInterface((char*) iface_name); this->SetTopic((char*) topic_name); this->Initialize(); return; };
    sdn_record (char* iface_name, sdn::Metadata& topic_mdata);

    /* Destructor method */
   ~sdn_record (void);

    /* Display methods */

};

/* Global variables */

/* Function declaration */

/* Function definition */

void SDNRecord_DPCThread_CB (sdn_record* self)
{

  log_trace("Executing '%s' routine", __FUNCTION__);

  while ((self->m_fifo)->IsEmpty() != true)
    {

      if ((self->m_fifo)->TryLock() == false)
	{
	  log_debug("FIFO currently locked - Try again later"); continue;
	}

      /* Take a reference to the payload instance */
      void* payload = (self->m_fifo)->GetOutDataBlockReference(); 

      (self->m_topic)->SetInstance(payload); /* WARNING - Overwriting the instance reference - This causes subsequent calls to GetInstance() to return 
						          a pointer to the FIFO buffer rather than the payload 
					     */
      self->Write();

      (self->m_fifo)->ReleaseOutDataBlockReference();
      (self->m_fifo)->ReleaseLock();

    }

  log_trace("Leaving '%s' routine", __FUNCTION__);

};

void SDNRecord_SUBThread_CB (sdn_record* self)
{

  log_trace("Executing '%s' routine", __FUNCTION__);

  if (self->m_sub == NULL) 
    {    
      wait(100000000ul); 
      return;
    }
  else if ((self->m_sub)->Receive(100000000ul) != STATUS_SUCCESS) /* Blocking call with timeout */
    {
      log_debug("Recorder '%s' - Unable to receive instance", self->m_topic_name);
      return;
    }
  else if (self->GetState() == SDNRECORD_STATE_RUNNING) 
    {

      if ((self->m_fifo)->IsFull() != true) 
	{ 
	  (self->m_fifo)->AcquireLock();
	  void* payload = (self->m_fifo)->GetInDataBlockReference(); (self->m_packet)->GetPayload(payload); (self->m_fifo)->ReleaseInDataBlockReference(); 
	  (self->m_fifo)->ReleaseLock();
	}
      else /* FIFO is full */
	{
	  log_warning("Recorder '%s' - FIFO full - Discard instance", self->m_topic_name);
	}

      if (self->m_count != -1) /* Count limit to be reached */
	{
	  self->m_count -= 1; if (self->m_count == 0) self->SetState(SDNRECORD_STATE_IDLE);
	}

    }
  else /* Recorded is not in RUNNING state */
    {
    }

  (self->m_sub)->DoBackgroundActivity();

  log_trace("Leaving '%s' routine", __FUNCTION__);

};

/* Initializer methods */

void sdn_record::Initialize (void) 
{ 

  log_trace("sdn_record::Initialize - Entering method");

  /* Initialize resources */
  this->m_topic = NULL; 
  this->m_packet = NULL;
  this->m_header = NULL;

  this->m_fifo = NULL;
  this->m_file = NULL;
  this->m_sub  = NULL;

  this->m_dpc_thread = NULL;
  this->m_sub_thread = NULL;

  this->m_count = DEFAULT_RECORD_COUNT;
  this->m_state = SDNRECORD_STATE_UNDEF;

  log_trace("sdn_record::Initialize - Leaving method");

  return;

};

/* Accessor methods */

/* Miscellaneous methods */

RET_STATUS sdn_record::Configure (void) 
{ 

  RET_STATUS status = STATUS_ERROR;

  log_trace("sdn_record::Configure - Entering method");

  /* Initialize resources */
  this->SetState(SDNRECORD_STATE_INIT);
#if 0
  this->m_topic = new sdn::Topic (this->m_topic_meta); 
  this->m_packet = new sdn::Packet ((this->m_topic)->GetSize()); (this->m_packet)->CreateInstance(); (this->m_topic)->SetInstance((this->m_packet)->GetPayload());
  this->m_header = (this->m_packet)->GetHeader(); /* Take a reference to the header instance */

  this->m_fifo = new BlockMemory_FIFO ((this->m_topic)->GetSize(), DEFAULT_FIFO_DEPTH);

  this->m_sub = new mcast::Subscriber (this->m_iface_name, (this->m_topic)->GetMCastGroup(), (this->m_topic)->GetMCastPort());
#else
  this->m_topic = sdn::TopicHelper::InstantiateFromDefinitionFile(this->m_topic_name);
  this->m_sub = new sdn::Subscriber(*this->m_topic); 

  if (((this->m_sub)->SetInterface(this->m_iface_name) != STATUS_SUCCESS) || ((this->m_sub)->Configure() != STATUS_SUCCESS))
    {
      log_error("sdn::Subscriber::Configure returns error with '%s' interface", this->m_iface_name);
      return status;
    }

  this->m_packet = (this->m_sub)->m_packet;
  this->m_header = (this->m_packet)->GetHeader(); /* Take a reference to the header instance */
  this->m_fifo = new BlockMemory_FIFO ((this->m_topic)->GetSize(), DEFAULT_FIFO_DEPTH);
#endif
  /* Open file */
  this->Open();

  this->m_dpc_thread = new ccs::base::AnyThread ((char*) "Record DPC"); (this->m_dpc_thread)->SetCallback((void (*)(void*)) &SDNRecord_DPCThread_CB, (void*) this); (this->m_dpc_thread)->SetPeriod(DEFAULT_THREAD_PERIOD); (this->m_dpc_thread)->Launch(); /* 1kHz */
  this->m_sub_thread = new ccs::base::AnyThread ((char*) "Record SUB"); (this->m_sub_thread)->SetCallback((void (*)(void*)) &SDNRecord_SUBThread_CB, (void*) this); (this->m_sub_thread)->SetPeriod(0); (this->m_sub_thread)->Launch(); /* Blocking callback */

  this->SetState(SDNRECORD_STATE_IDLE);

  status = STATUS_SUCCESS;

  log_trace("sdn_record::Configure - Leaving method");

  return status;

};

RET_STATUS sdn_record::Open (void) 
{ 

  RET_STATUS status = STATUS_ERROR;

  log_trace("sdn_record::Open - Entering method");

  char file_path [PATH_MAX_LENGTH]; snprintf(file_path, PATH_MAX_LENGTH, "%s/%s", (((getenv(SDN_RECORD_PATH_ENVAR) == NULL) || (strlen(getenv(SDN_RECORD_PATH_ENVAR)) == 0)) ? DEFAULT_RECORD_PATH : getenv(SDN_RECORD_PATH_ENVAR)), this->m_topic_name);

  /* Try and create path */
  log_info("Try and create '%s' path", file_path); 
  create_path(file_path);

  uint64_t curr_time = ccs::HelperTools::Floor(ccs::HelperTools::GetCurrentTime(), 1000000000ul); // With second resolution
  char timestamp [STRING_MAX_LENGTH]; ccs::HelperTools::ToISO8601(curr_time, timestamp, STRING_MAX_LENGTH); 

  snprintf(this->m_file_name, PATH_MAX_LENGTH, "%s/%s_%s.record", file_path, this->m_topic_name, timestamp);

  /* Try and open file */
  log_info("Try and open '%s' file", this->m_file_name);
  if ((this->m_file = fopen(this->m_file_name, "w")) == NULL)
    {
      log_error("Unable to open '%s' file - %d, %s", this->m_file_name, errno, strerror(errno));
      return status;
    }

  /* Write metadata */
  char type_def [1024] = STRING_UNDEFINED; if (this->m_topic != NULL) (this->m_topic)->SerializeType((char*) type_def, 1024); else log_warning("Topic instance is 'NULL'");
 
  fprintf(this->m_file, "%% TDEF - %s\n", type_def);
  fprintf(this->m_file, "%% TIME - %s (%ld)\n", timestamp, curr_time);

  status = STATUS_SUCCESS;

  log_trace("sdn_record::Open - Leaving method");

  return status; 

};

RET_STATUS sdn_record::Close (void) { if (this->m_file != NULL) fclose(this->m_file); this->m_file = NULL; return STATUS_SUCCESS; };

RET_STATUS sdn_record::Write (void) { char buffer [1024]; (this->m_topic)->SerializeInstance((char*) buffer, 1024); return this->Write((char*) buffer); };
RET_STATUS sdn_record::Write (char* buffer) { RET_STATUS status = STATUS_ERROR; if (this->m_file != NULL) { fprintf(this->m_file, "%s\n", buffer); status = STATUS_SUCCESS; } return status; };

/* Constructor methods */

sdn_record::sdn_record (char* iface_name, sdn::Metadata_t& topic_mdata)
{ 

  log_trace("sdn_record::sdn_record - Entering method");

  this->SetInterface((char*) iface_name); this->SetTopic((sdn::Metadata_t&) topic_mdata); this->Initialize(); 

  log_trace("sdn_record::sdn_record - Leaving method");

  return; 

};

/* Destructor method */

sdn_record::~sdn_record (void) 
{ 

  log_trace("sdn_record::~sdn_record - Entering method");

  this->Stop();

  /* Release resources */
  if (this->m_dpc_thread != NULL) delete this->m_dpc_thread; 
  if (this->m_sub_thread != NULL) delete this->m_sub_thread; 

  if (this->m_sub != NULL) delete this->m_sub;
  if (this->m_fifo != NULL) delete this->m_fifo; 
#if 0
  if (this->m_packet != NULL) delete this->m_packet; 
  if (this->m_topic != NULL) delete this->m_topic;
#endif
  this->Close();

  log_trace("sdn_record::~sdn_record - Leaving method");

  return; 

};

/* Display methods */

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC NULL

#endif
