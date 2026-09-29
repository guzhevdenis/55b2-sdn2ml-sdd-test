#ifndef SDN_MCAST_H
#define SDN_MCAST_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-mcast.h $
* $Id: sdn-mcast.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include "mcast-api.h" /* Interface definition common for ccs::mcast participants */

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {

namespace mcast {

/* Maintained for backward compatibility with v1.0 */
typedef ccs::base::ParticipantIface ParticipantIface;
typedef ccs::mcast::PublisherIface PublisherIface;
typedef ccs::mcast::SubscriberIface SubscriberIface;

typedef ParticipantIface Participant;
typedef PublisherIface Publisher;
typedef SubscriberIface Subscriber;

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace mcast */

} /* namespace sdn */

extern "C" {
    
#endif /* __cplusplus */
    
/* ToDo - Insert C API declaration */
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SDN_MCAST_H */
