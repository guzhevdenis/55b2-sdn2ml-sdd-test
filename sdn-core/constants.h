#ifndef CONSTANTS_H
#define CONSTANTS_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/constants.h $
* $Id: constants.h 179115 2026-01-09 15:15:20Z kimh19 $
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

/* Constants */

#define UNIT_VERSION_UID(major, minor, maint) (100*(major)+10*(minor)+(maint))

#ifndef UNIT_VERSION
#define UNIT_VERSION UNIT_VERSION_UID(1,2,0)
#endif

#ifndef UNIT_VERSION_HR
#define UNIT_VERSION_HR "1.2.1"
#endif

#define USE_PACKET_FOOTER
#if UNIT_VERSION < UNIT_VERSION_UID(2,0,0)
#undef USE_PACKET_FOOTER
#endif

#define DEFAULT_TOPIC_NAME "undefined"
#define DEFAULT_TOPIC_SIZE 1024

#define DEFAULT_MCAST_GROUP "239.0.0.1"
#define DEFAULT_MCAST_PORT  60000
#define DEFAULT_UCAST_ADDR "127.0.0.1"
#define DEFAULT_UCAST_PORT  60000
#define DEFAULT_IFACE_NAME "lo"

#define DEFAULT_DISCOVERY_MSG_LENGTH 2048 /* SDN discovery message size */
#define DEFAULT_DISCOVERY_MCAST_GROUP "239.0.1.2" /* SDN discovery protocol */
#define DEFAULT_DISCOVERY_MCAST_PORT 10002 /* SDN discovery protocol */

/* Documentation - The packet size is derived from the specification of the IP protocol suite. The UDP header 
   includes size as 16bits field, and corresponds to header + data. Similarly, the IPv4 header has size as 16bits
   field corresponding to header + data. It is worthwhile noting that the IPv4 header includes options and a header
   size 4bits field corresponding to number of 32bits words composing the header. I.e.

   Max UDP/IPv4 data payload = (2^16 - 1) - 4 * (2^4 - 1) - 8; 16bits - IP header - UDP header
                             = 65535 - 60 - 8 = 65467;

   Max topic data payload = 65467 - 48 - 20; Max UDP/IPv4 data payload - SDN header - SDN footer;
                          = 65399;

*/

#define MAX_IPv4_PACKET_SIZE 65535  
#define MAX_PACKET_SIZE 65467 /* UDP/IPv4 payload size, i.e. maximum size for sdn::mcast or sdn::ucast participants */
#define MAX_TOPIC_SIZE  65399 /* SDN payload size, i.e. maximum size for topic data */

#define MAX_TOPIC_NUMBER 1024
#define MAX_IP_ADDR_LENGTH 16

#define SDN_INTERFACE_ENVVAR "SDN_INTERFACE_NAME"

/* Type definition */

/* Global variables */

/* Function declaration */

/* Function definition */

#endif
