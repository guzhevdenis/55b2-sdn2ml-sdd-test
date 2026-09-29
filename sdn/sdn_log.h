/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn_log.h $
* $Id: sdn_log.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project		: CODAC Core System
*
* Description	: SDN Software - Communication API SdnLogBuf header file.
*
* Author        : Eui Jae LEE, Hyung Gi KIM (Mobiis Co., Ltd)
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

/*! \file log_buffer.h 
 *   \brief SDN Log Functions Header File
 */
#ifndef SDN_LOG_H
#define SDN_LOG_H


#if 1

//#define LOG_TRACE_ENABLE
//#define LOG_DEBUG_ENABLE
#include <common/log-api.h> /* Syslog wrapper routines */

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sdnapi"

#else                           /* Port to log-lib */

#define SDN_PORT
#include <log-port.h>
#undef SDN_PORT
		
#endif /* Port to loglib */

#endif
