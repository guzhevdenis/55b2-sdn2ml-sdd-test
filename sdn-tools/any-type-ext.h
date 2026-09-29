#ifndef ANY_TYPE_EXT_H
#define ANY_TYPE_EXT_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/any-type-ext.h $
* $Id: any-type-ext.h 179115 2026-01-09 15:15:20Z kimh19 $
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

#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */
#include "any-type.h"

#include "common/CompoundType.h"
#include "common/AnyValueHelper.h" // .. associated helper routines
#include <common/LookUpTable.h> /* LookUpTable class definition, etc. */

/* Type definition */

namespace sdn {

namespace base {

typedef struct ExtTypeInfo { /* For each attribute within a structure, the name will be defined externally as keyword in LUTable */

  uint32_t rank; /* Attribute rank within the structure */
  char desc [STRING_MAX_LENGTH]; /* Attribute free description string */
  char qual [STRING_MAX_LENGTH]; /* Attribute qualifier */
  char unit [STRING_MAX_LENGTH]; /* Attribute unit */
  char mult [STRING_MAX_LENGTH]; /* Attribute multiplicity string */

} ExtTypeInfo_t;


class AnyType : public ccs::types::CompoundType
{

  private:

    ccs::base::LookUpTable<ExtTypeInfo_t>* m_type_ext; /* Extended attribute look-up table */

    uint8_t* m_instance;

    bool m_defined, m_created;

    /* Initializer methods */
    void Initialize (void);

  public:

    /* Initializer methods */
    RET_STATUS AddAttribute (uint_t rank, const char* name, const char* type, uint_t mult=1, const char* value=NULL);
    RET_STATUS AddAttribute (const char* rank, const char* name, const char* type, const char* mult = NULL, const char* value = NULL); // Bug 12528 - Restore missing method
    RET_STATUS AddExtAttribute (const char* rank, const char* name, const char* desc, const char* qual, const char* unit, const char* mult);

    /* Accessor methods */
    void* GetInstance (void) const { return (void*)this->m_instance; };
    void SetInstance (void* instance) { this->m_instance = (uint8_t*)instance; return; };
    void ClearInstance (void) { if((this->m_instance) != NULL) memset((this->m_instance), 0, this->ccs::types::AnyType::GetSize()); return; };
    RET_STATUS CreateInstance (void);
    RET_STATUS CreateInstance (uint_t size);
    void DeleteInstance (void);

    bool IsDefined (void) const { return this->m_defined; }
    bool IsInitialized (void) const { return this->m_defined; }
    bool IsCreated (void) const { return this->m_created; }

    uint_t GetRank (void) const { return this->ccs::types::CompoundType::GetAttributeNumber(); } /* Number of attributes */

    bool HasAttribute (const char * const name) const;

    bool GetAttribute (uint_t index, void * const p_value) const { return this->GetAttribute(this->ccs::types::CompoundType::GetAttributeName(index), p_value); };
    bool GetAttribute (const char * const name, void * const p_value) const;

    bool SetAttribute (uint_t index, const void * const p_value) { return this->SetAttribute(this->ccs::types::CompoundType::GetAttributeName(index), p_value); };
    bool SetAttribute (const char * const name, const void * const p_value);

    uint_t GetAttributeRank (const char* name) const;

    uint32_t GetAttributeOffset (const char * const name) const; // Bug 13335

    void* GetAttributeReference (const uint32_t index) const { return this->GetAttributeReference(this->ccs::types::CompoundType::GetAttributeName(index)); };
    void* GetAttributeReference (const char * const name) const; // Bug 12521

    uint_t GetAttributeSize (const uint32_t index) const { return this->GetAttributeSize(this->ccs::types::CompoundType::GetAttributeName(index)); };
    uint_t GetAttributeSize (const char * const name) const;

    TypeIdentifier_t GetAttributeType (uint_t index) const;
    TypeIdentifier_t GetAttributeType (const char* name) const {return this->GetAttributeType(this->ccs::types::CompoundType::GetAttributeIndex(name));};
	
    const char* HasInstanceIndex (void) const;
    const char* HasInstanceTimestamp (void) const;

    const char* GetAttributeDescription (uint_t rank) const;
    const char* GetAttributeDescription (const char* name) const { return this->GetAttributeDescription(this->GetAttributeRank(name)); };

    const char* GetAttributeQualifier (uint_t rank) const;
    const char* GetAttributeQualifier (const char* name) const { return this->GetAttributeQualifier(this->GetAttributeRank(name)); };

    const char* GetAttributeUnit (uint_t rank) const;
    const char* GetAttributeUnit (const char* name) const { return this->GetAttributeUnit(this->GetAttributeRank(name)); };

    const char* GetAttributeMultiplicity (uint_t rank) const;
    const char* GetAttributeMultiplicity (const char* name) const { return this->GetAttributeMultiplicity(this->GetAttributeRank(name)); };

    // Miscellaneous methods
    RET_STATUS Compress (void); // Would the type definition be incomplete

    /**
     * @brief Copy assignment operator.
     */

    AnyType& operator= (const ccs::types::AnyType& type);
    AnyType& operator= (const ccs::types::CompoundType& type);

    // Constructor methods
    AnyType (void); // Type definition using the class API

    // Destructor method
    virtual ~AnyType (void);

    // Display methods

    RET_STATUS ParseInstance (const char* buffer);

};

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace base */

} /* namespace sdn */

#endif /* ANY_TYPE_EXT_H */
