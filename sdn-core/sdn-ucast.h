#ifndef SDN_UCAST_H
#define SDN_UCAST_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-ucast.h $
* $Id: sdn-ucast.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include "ucast-api.h" /* Interface definition ccs::ucast participants */

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {
    
namespace ucast {
    
/* Maintained for backward compatibility with v1.0 */
typedef ccs::base::ParticipantIface ParticipantIface;
typedef ccs::ucast::PublisherIface PublisherIface;
typedef ccs::ucast::SubscriberIface SubscriberIface;

typedef ParticipantIface Participant;
typedef PublisherIface Publisher;
typedef SubscriberIface Subscriber;

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace ucast */
    
} /* namespace sdn */

extern "C" {
    
#endif /* __cplusplus */
    
/* ToDo - Insert C API declaration */
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SDN_UCAST_H */
