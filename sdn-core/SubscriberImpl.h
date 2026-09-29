/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/SubscriberImpl.h $
* $Id: SubscriberImpl.h 179115 2026-01-09 15:15:20Z kimh19 $
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

/**
 * @file SubscriberImpl.h
 * @brief Header file for sdn::Subscriber implementation classes
 * @date 18/04/2018
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 *
 * @detail This header file contains the definition of the Subscriber
 * implementation classes.
 */

#ifndef _SubscriberImpl_h_
#define _SubscriberImpl_h_

/* Global header files */

/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

#include "sdn-base.h" /* Base classes definition */

#include "ParticipantImpl.h"

/* Constants */

#define OBJTYPE_SDN_SUBSCRIBER (char*) "sdn::Subscriber_Base"

#ifdef __cplusplus

/* Type definition */

namespace sdn {

class Subscriber_Base : public ParticipantImpl /* Base class */
{

  private:

  public:

    /* Initialiser methods */

    /* Accessor methods */

    /* Miscellaneous methods */
    virtual RET_STATUS Do (void) { return this->Receive(); }; /* Specialises virtual method */

    virtual RET_STATUS Receive (uint64_t timeout) = 0; /* Pure virtual method */
    virtual RET_STATUS Receive (void) = 0;             /* Pure virtual method */

    /* Constructor methods */
    Subscriber_Base (void) { this->SetInstanceType(OBJTYPE_SDN_SUBSCRIBER); return; };

    /* Destructor method */
    virtual ~Subscriber_Base (void) {};

};

class SubscriberImpl : public Subscriber_Base
{

  private:

    /* Initialiser methods */
    void Initialise (void);

  public:

    /* Initialiser methods */

    /* Accessor methods */

    /* Miscellaneous methods */
    virtual RET_STATUS Configure (void); /* Specialises virtual method */
    virtual RET_STATUS Receive (uint64_t timeout); /* Specialises virtual method */
    virtual RET_STATUS Receive (void); /* Specialises virtual method */

    /* Constructor methods */
    SubscriberImpl (void);
    SubscriberImpl (Metadata_t& mdata); /* Used by AsynchronousSubscriber */
    SubscriberImpl (Topic& topic); /* Used by AsynchronousSubscriber */

    /* Destructor method */
    virtual ~SubscriberImpl (void);

};

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespace sdn */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SubscriberImpl_h_ */
