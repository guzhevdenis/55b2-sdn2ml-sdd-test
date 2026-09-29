#ifndef UCAST_CONSTANTS_H
#define UCAST_CONSTANTS_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/ucast-constants.h $
* $Id: ucast-constants.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project	: CODAC Core System
*
* Description	: Infrastructure tools - Prototype
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

#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace ccs {

namespace ucast {

/* Global variables */

const char DefaultIfaceName [] = "lo";
const char DefaultUCastAddr [] = "127.0.0.1";
const uint_t DefaultUCastPort = 60000u;

/* Documentation - The packet size is derived from the specification of the IP protocol suite. The UDP header 
   includes size as 16bits field, and corresponds to header + data. Similarly, the IPv4 header has size as 16bits
   field corresponding to header + data. It is worthwhile noting that the IPv4 header includes options and a header
   size 4bits field corresponding to number of 32bits words composing the header. I.e.

   Max UDP/IPv4 data payload = (2^16 - 1) - 4 * (2^4 - 1) - 8; 16bits - IP header - UDP header
                             = 65535 - 60 - 8 = 65467;

   Max topic data payload = 65467 - 48 - 20; Max UDP/IPv4 data payload - SDN header - SDN footer;
                          = 65399;

*/

const uint_t MaxIPv4PacketSize = 65535u;
const uint_t MaxUCastPacketSize = 65467u; /* UDP/IPv4 payload size, i.e. maximum size for ccs::ucast participants */

const uint_t MaxIPv4AddrLength = 16u;

/* Function declaration */

/* Function definition */

} /* namespace ucast */

} /* namespace ccs */

#endif /* __cplusplus */

#endif /* UCAST_CONSTANTS_H */
