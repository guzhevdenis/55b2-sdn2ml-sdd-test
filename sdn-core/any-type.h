#ifndef ANY_TYPE_H
#define ANY_TYPE_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/any-type.h $
* $Id: any-type.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project	: CODAC Core System
*
* Description	: Infrastructure tools - Prototype
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

/* Done:
 *
 *   - Support type-specific accessors through type-casting the instance reference, see void* AnyType::GetInstance (void).
 *   - Support dummy attributed with zero size.
 *   - Retrieve attribute rank by name for future access.
 *   - Parse attribute from string, incl. reference to environment variables.
 *   - Re-structure code, base class distinct from XML parsing class to allow for deciding at application-level
 *   - Support type-specific accessors, e.g. RET_STATUS AnyType::GetAttribute(uint_t, double&).
 *   - Safe implementation of Parse and Serialize methods.
 *
 */

/* ToDo:
 *
 *   - Verify behaviour in case of absence of environment variable definition.
 *   - Code simplification urgently needed.
 *   - Robustify destructor ... still segfaults.
 *
 */

/* Global header files */

#include <common/types.h> /* Condensed integer type definition, RET_STATUS, etc. */

/* Local header files */

/* Constants */

#define DEFAULT_DATATYPE_NAME "undefined"

#define ANYTYPE_ATTR_INT8  ANYTYPE_ATTR_SINT8
#define ANYTYPE_ATTR_INT16 ANYTYPE_ATTR_SINT16
#define ANYTYPE_ATTR_INT32 ANYTYPE_ATTR_SINT32
#define ANYTYPE_ATTR_INT64 ANYTYPE_ATTR_SINT64

#define ANYTYPE_ATTR_UNDEFINED ANYTYPE_ATTR_UNDEF /* Maintained for backward compatibility reason */

/* Type definition */

typedef ::ccs::types::Identifier TypeIdentifier_t; // From BasicTypes.h
enum TypeIdentifier { // Bug 12556 - Maintained for backward compatibility reason

  ANYTYPE_ATTR_BOOL = ccs::types::BOOLEAN,
  ANYTYPE_ATTR_CHAR, 
  ANYTYPE_ATTR_STRING, 
  ANYTYPE_ATTR_SINT8,
  ANYTYPE_ATTR_UINT8,
  ANYTYPE_ATTR_SINT16,
  ANYTYPE_ATTR_UINT16,
  ANYTYPE_ATTR_SINT32,
  ANYTYPE_ATTR_UINT32,
  ANYTYPE_ATTR_SINT64,
  ANYTYPE_ATTR_UINT64,
  ANYTYPE_ATTR_FLOAT,
  ANYTYPE_ATTR_DOUBLE ,
  ANYTYPE_ATTR_UNDEF

};

namespace ccs {

namespace base {

typedef struct AttrInfo { // Rank will be derived from index within the table

  char name [STRING_MAX_LENGTH]; // Attribute name
  char type [STRING_MAX_LENGTH]; // Type identifier
  char mult [STRING_MAX_LENGTH]; // Type multiplicity
  char deflt [STRING_MAX_LENGTH]; // Default value, if any

  // Bug 12644 - Regression on unit
  char unit [STRING_MAX_LENGTH]; // Unit, if any
  char desc [STRING_MAX_LENGTH]; // Description, if any

} AttrInfo_t;

typedef struct TypeInfo { /* For each attribute within a structure, the name will be defined externally as keyword in LUTable */

  uint32_t rank; /* Attribute rank within the structure */
  uint32_t size; /* Attribute memory footprint */
  uint32_t type; /* Type identifier */
  uint32_t mult; /* Type multiplicity */
  uint32_t offset; /* Relative location in instance */
  void* p_value; /* Pointer to location in instance */
  void* p_deflt; /* Pointer to default value */

} TypeInfo_t;

/* Global variables */

/* Function declaration */

static inline TypeIdentifier_t AnyType_GetTypeId (const char* type_name) { return ::ccs::types::GetIdentifier(type_name); }
static inline const char * AnyType_GetTypeName (TypeIdentifier_t type_id) { return ::ccs::types::GetName(type_id); }
static inline uint_t AnyType_GetTypeSize (const char* type_name) { return ::ccs::types::GetSize(type_name); }
static inline uint_t AnyType_GetTypeSize (TypeIdentifier_t type_id) { return ::ccs::types::GetSize(type_id); }

bool AnyType_IsAttrValid (TypeInfo_t& attr);

/* Function definition */

/* Accessor methods */

} /* namespace base */

} /* namespace ccs */

#endif /* ANY_TYPE_H */
