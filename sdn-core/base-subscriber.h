#ifndef BASE_SUBSCRIBER_H
#define BASE_SUBSCRIBER_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/base-subscriber.h $
* $Id: base-subscriber.h 179115 2026-01-09 15:15:20Z kimh19 $
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

/* Local header files */

#include "base-api.h" /* Interface definition common for ccs::mcast and ccs::ucast participants */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {

namespace base {

/* Maintained for backward compatibility with v1.0 */
typedef ccs::base::SubscriberIface SubscriberIface;
typedef SubscriberIface Subscriber;

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace base */

} /* namespace sdn */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BASE_SUBSCRIBER_H */
