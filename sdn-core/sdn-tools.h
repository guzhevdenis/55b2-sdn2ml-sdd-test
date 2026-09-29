#ifndef SDN_TOOLS_H
#define SDN_TOOLS_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-tools.h $
* $Id: sdn-tools.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project	: CODAC Core System
*
* Description	: SDN - Infrastructure tools - Prototype
*
* Author        : Bertrand Bauvir
*
* Copyright (c) : 2010-2026 ITER Organization,
*                 CS 90 046
*                 13067 St. Paul-lez-Durance Cedex
*                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

/* Global header files */

#include <common/tools.h> /* Misc. helper functions, e.g. hash, etc. */

#include <common/Base64.h> /* Misc. helper functions, e.g. base64 encoding, etc. */
#include <common/CyclicRedundancyCheck.h> /* Misc. helper functions, e.g. crc, etc. */
#include <common/Hash.h> /* Misc. helper functions, e.g. hash, etc. */

/* Bug 9343 - Enable log_trace by default during development phases - Disable for released versions */
//#define LOG_TRACE_ENABLE
//#define LOG_DEBUG_ENABLE
#include <common/log-api.h> /* Syslog wrapper routines */

#include <common/NetTools.h> /* Misc. network functions */
#include <common/SysTools.h> /* Misc. system functions */

/* Local header files */

//#include "tools.h" /* Misc. helper functions, e.g. hash, etc. */
//#include "base64.h" /* Misc. helper functions, e.g. base64 encoding, etc. */
//#include "crc.h" /* Misc. helper functions, e.g. crc, etc. */
//#include "log-api.h" /* Syslog wrapper routines */
//#include "net-tools.h" /* Misc. network functions */

#include "constants.h"
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

/* Constants */

#ifndef UNIT_VERSION_HR
#define UNIT_VERSION_HR "undefined"
#endif

/* Type definition */

namespace sdn {

namespace HelperTools {

/* Global variables */

/* Function declaration */

/* Function definition */

static inline const char* GetVersion (void) { return UNIT_VERSION_HR; }

using ccs::HelperTools::IsAddressValid;
using ccs::HelperTools::IsMCastAddress;
using ccs::HelperTools::IsUCastAddress;

using ccs::HelperTools::IsInterfaceValid;
using ccs::HelperTools::IsLinkDetected;
using ccs::HelperTools::GetMACAddress;
using ccs::HelperTools::GetIPAddress;

static inline bool IsTTLValid (long ttl) // Bug 14996 - Test validity of the TTL value
{

  return ttl >= -1 && ttl <= 255;

}

static inline bool IsInterfaceValid (void) 
{ 

  char iface_name [STRING_MAX_LENGTH] = DEFAULT_IFACE_NAME; 

  bool ret = (get_env_variable(SDN_INTERFACE_ENVVAR, static_cast<char*>(iface_name)) == STATUS_SUCCESS);

  if (ret)
    {
      ret = sdn::HelperTools::IsInterfaceValid(iface_name);
    }

  return ret;

}

using ccs::HelperTools::IsBigEndian;
using ccs::HelperTools::IsLittleEndian;
using ccs::HelperTools::GetNativeByteOrder;

template <typename Type> inline Type FromNetworkByteOrder (Type value) { return ccs::HelperTools::FromNetworkByteOrder<Type>(value); }
template <typename Type> inline Type ToNetworkByteOrder (Type value) { return ccs::HelperTools::ToNetworkByteOrder<Type>(value); }

template <typename Type> inline Type CyclicRedundancyCheck (const uint8_t* buffer, const uint_t size) { return ccs::HelperTools::CyclicRedundancyCheck<Type>(buffer, size); }
template <typename Type> inline Type Hash (const char* key) { return ccs::HelperTools::Hash<Type>(key); }

} // namespace HelperTools

} // namespace sdn

static inline RET_STATUS net_get_ip_address (const char* iface_name, char* iface_addr, uint_t max_size = STRING_MAX_LENGTH) { return sdn::HelperTools::GetIPAddress(iface_name, iface_addr, max_size); }
static inline RET_STATUS net_get_mac_address (const char* iface_name, char* iface_addr, uint_t max_size = STRING_MAX_LENGTH) { return sdn::HelperTools::GetMACAddress(iface_name, iface_addr, max_size); }
static inline bool net_is_link_detected (const char* iface_name) { return sdn::HelperTools::IsLinkDetected(iface_name); }
/* Bug 8722 - Test validity of network interface */
static inline bool net_is_interface_valid (const char* iface_name) { return sdn::HelperTools::IsInterfaceValid(iface_name); }
static inline bool net_is_address_valid (const char* addr) { return sdn::HelperTools::IsAddressValid(addr); }
static inline bool net_is_mcast_address (const char* addr) { return sdn::HelperTools::IsMCastAddress(addr); }
static inline bool net_is_ucast_address (const char* addr) { return sdn::HelperTools::IsUCastAddress(addr); }

static inline bool sdn_is_link_detected (void)
{

  char iface_name [STRING_MAX_LENGTH] = DEFAULT_IFACE_NAME;

  /* Try and retrieve interface identifier from environment */
  return ((get_env_variable((char*) "SDN_INTERFACE_NAME", (char*) iface_name) != STATUS_SUCCESS) ? false : sdn::HelperTools::IsLinkDetected(iface_name));

}

static inline RET_STATUS sdn_get_mac_address (char* iface_addr, uint_t max_size = STRING_MAX_LENGTH)
{

  char iface_name [STRING_MAX_LENGTH] = DEFAULT_IFACE_NAME;

  /* Try and retrieve interface identifier from environment */
  return ((get_env_variable((char*) "SDN_INTERFACE_NAME", (char*) iface_name) != STATUS_SUCCESS) ? STATUS_ERROR : sdn::HelperTools::GetMACAddress(iface_name, iface_addr, max_size));

}

/* Bug 8722 - Test validity of SDN_INTERFACE_NAME */
static inline bool sdn_is_interface_valid (void) { return sdn::HelperTools::IsInterfaceValid(); } /* Maintained for backward compatibility */

static inline bool sdn_is_address_valid (const char* addr) { return sdn::HelperTools::IsAddressValid(addr); } /* Maintained for backward compatibility */
static inline bool sdn_is_mcast_address (const char* addr) { return sdn::HelperTools::IsMCastAddress(addr); } /* Maintained for backward compatibility */
static inline bool sdn_is_ucast_address (const char* addr) { return sdn::HelperTools::IsUCastAddress(addr); } /* Maintained for backward compatibility */

#endif /* SDN_TOOLS_H */
