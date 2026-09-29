#ifndef SDN_BASE_H
#define SDN_BASE_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-base.h $
* $Id: sdn-base.h 179115 2026-01-09 15:15:20Z kimh19 $
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

/* Documentation - This file is used to simplify the declaration of foundation classes for compiling the 
   SDN core library. It is not intended to be used by client applications, hence can not be included in
   public header files.
*/

/* Global header files */

#include <common/LookUpTable.h> /* LUTable template class definition */
#include <common/any-thread.h> /* Thread management class definition */
//#include <common/any-type.h> /* Runtime datatype definition */
#include <common/Buffer.h> /* Memory management template classes definition */
#include <common/CircularBuffer.h> /* Memory management template classes definition */
#include <common/Statistics.h> /* Statistics template class definition */

#include <common/AnyObject.h> /* Root object class definition */

//#include <common/base-api.h> /* Interface definition common for ccs::mcast and ccs::ucast participants */
//#include <common/mcast-api.h> /* Interface definition ccs::mcast participants */
//#include <common/ucast-api.h> /* Interface definition ccs::ucast participants */

/* Local header files */

#include "sdn-common.h" /* Common header file */
#include "any-type-ext.h" /* Extended type definition, description, units, etc. */

#include "base-api.h" /* Interface definition common for ccs::mcast and ccs::ucast participants */
#include "mcast-api.h" /* Interface definition ccs::mcast participants */
#include "ucast-api.h" /* Interface definition ccs::ucast participants */

#include "base-participant.h" /* Interface definition common for sdn::mcast and sdn::ucast participants */
#include "base-publisher.h"   /* Interface definition common for sdn::mcast and sdn::ucast participants */
#include "base-subscriber.h"  /* Interface definition common for sdn::mcast and sdn::ucast participants */

/* Constants */

/* Type definition */

namespace sdn {

namespace base {

using ::ccs::base::AnyObject;
using ::ccs::base::AnyThread;
using ::ccs::base::Statistics;
using ::ccs::base::LookUpTable;

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace base */

} /* namespace sdn */

#endif /* SDN_BASE_H */
