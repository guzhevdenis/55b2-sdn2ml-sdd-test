#ifndef SDN_PACKET_H
#define SDN_PACKET_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-packet.h $
* $Id: sdn-packet.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include <stdio.h> /* sscanf, printf, etc. */
#include <string.h> /* strncpy, etc. */

/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

#include "sdn-msg.h"
#include "sdn-header.h"

/* Constants */
#define SERIALIZE_MAX_LENGTH 2048
#define DEFAULT_PAYLOAD_SIZE 1024
#if 0 // Scoped enums put requirements on compiler used to include this header file
typedef enum SDNPacket_AttributeId : uint_t {
#else
typedef enum SDNPacket_AttributeId {
#endif
  SDN_PACKET_HEADER = 0,
  SDN_PACKET_CORPUS,

  SDN_PACKET_UNDEFINED

} SDNPacket_AttributeId_t;

#define SDNPacket sdn::Packet /* For backward compatibility purposes with v1.0 */
#define sdn_packet sdn::Packet /* For backward compatibility purposes with v1.0 */

/* Type definition */

namespace sdn {

class Packet : public Message
{

  private:

    ccs::types::CompoundType* m_buffer; /* Raw allocated memory buffer which is split in regions: */
    Header*  m_header;            /*   - Header,                                            */
    ccs::types::CompoundType* m_corpus; /*   - Body, payload, topic                               */

    uint8_t* m_b_instance;
    uint8_t* m_c_instance;

//	ccs::types::AnyValue m_b_value;
//	ccs::types::AnyValue m_h_value;
//	ccs::types::AnyValue m_c_value;

    /* Initializer methods */
    void Initialize (uint_t size); /* Define payload by size */

  public:

    /* Initializer methods */
    void ClearInstance (void);
    RET_STATUS CreateInstance (void);

    /* Accessor methods */
    void SetInstance (void* instance);

    Header* GetHeader (void) { return this->m_header; };

    void* GetPayload (void) { return (void*) m_c_instance; };
    void GetPayload (void* ref) { memcpy(ref, m_c_instance, (this->m_corpus)->GetSize()); };
    void SetPayload (void* ref) { memcpy(m_c_instance, ref, (this->m_corpus)->GetSize()); };

    uint_t GetHeaderSize (void)  { uint_t size = 0; if (this->m_header != NULL) size = (this->m_header)->GetSize(); return size; };
    uint_t GetPayloadSize (void) { uint_t size = 0; if (this->m_corpus != NULL) size = (this->m_corpus)->GetSize(); return size; };

    /* Miscellaneous methods */

    /* Constructor methods */
    Packet (void);
    Packet (uint_t size); /* Define payload by size */

    /* Destructor method */
   ~Packet (void);

    /* Display methods */
    uint16_t SerializeType (char* buffer, int max_size = SERIALIZE_MAX_LENGTH);
    void SerializeInstance (char* buffer, int max_size = SERIALIZE_MAX_LENGTH);

};

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace sdn */

#endif /* SDN_PACKET_H */
