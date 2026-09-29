#ifndef SDN_TYPES_H
#define SDN_TYPES_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-types.h $
* $Id: sdn-types.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project	: CODAC Core System
*
* Description	: SDN - Infrastructure tools - Prototype
*
* Author        : Bertrand Bauvir
*
* Copyright (c) : 2010-2026 ITER Organization,
*		          CS 90 046
*                 13067 St. Paul-lez-Durance Cedex
*                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

/* Global header files */

#include <common/types.h> /* Misc. type definition */
#include <common/BasicTypes.h> /* Misc. type definition */
#include <common/SysTools.h> // Endianness-related types

/* Local header files */

/* Constants */

/* Type definition */

namespace sdn {

namespace types {

using ccs::types::string;

using ccs::types::char8;

using ccs::types::int8;
using ccs::types::uint8;
typedef int8 int8_t; // Backward compatibility
typedef uint8 uint8_t; // Backward compatibility

using ccs::types::int16;
using ccs::types::uint16;
typedef int16 int16_t; // Backward compatibility
typedef uint16 uint16_t; // Backward compatibility

using ccs::types::int32;
using ccs::types::uint32;
typedef int32 int32_t; // Backward compatibility
typedef uint32 uint32_t; // Backward compatibility

using ccs::types::int64;
using ccs::types::uint64;
typedef int64 int64_t; // Backward compatibility
typedef uint64 uint64_t; // Backward compatibility

using ccs::types::float32;
using ccs::types::float64;
typedef float32 float32_t; // Backward compatibility
typedef float64 float64_t; // Backward compatibility

using ccs::types::Endianness;
typedef Endianness Endianness_t; // Backward compatibility

/* Global variables */

static const Endianness BigEndian = ccs::types::BigEndian;
static const Endianness LittleEndian = ccs::types::LittleEndian;
static const Endianness NetworkByteOrder = ccs::types::NetworkByteOrder;

/* Function declaration */

/* Function definition */

} /* Namespace types */

} /* Namespace sdn */

#endif /* SDN_TYPES_H */
