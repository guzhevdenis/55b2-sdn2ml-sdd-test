/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/AsynchronousSubscriberImpl.h $
* $Id: AsynchronousSubscriberImpl.h 179115 2026-01-09 15:15:20Z kimh19 $
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
 * @file AsynchronousSubscriberImpl.h
 * @brief Header file for sdn::AsynchronousSubscriber implementation classes
 * @date 18/04/2018
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 *
 * @detail This header file contains the definition of the AsynchronousSubscriber
 * implementation classes.
 */

#ifndef _AsynchronousSubscriberImpl_h_
#define _AsynchronousSubscriberImpl_h_

/* Global header files */
#if __cplusplus > 199711L
#include <functional> /* std::function, etc. */
#endif
/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */

#include "sdn-base.h" /* Base classes definition */

#include "SubscriberImpl.h"

/* Constants */

#ifdef __cplusplus

/* Type definition */

namespace sdn {
#if __cplusplus > 199711L
#else
class AsynchronousSubscriberIface; /* Forward class declaration */
#endif
class AsynchronousSubscriberImpl : public SubscriberImpl
{

  private:

    base::AnyThread* m_thread; /* Thread to manage subscriber instance */
    base::Statistics<uint64_t>* m_stats;

    uint64_t m_timeout;
#if __cplusplus > 199711L
    std::function<void(void)> m_recv_cb; /* Routine called upon topic reception */
#else
    AsynchronousSubscriberIface* m_iface;
#endif
    /* Initializer methods */
    void Initialize (void);

  public:

    /* Initializer methods */
    RET_STATUS Launch (void) { RET_STATUS status = STATUS_ERROR; if (this->m_thread != NULL) status = (this->m_thread)->Launch(); return status; };
    RET_STATUS Terminate (void) { RET_STATUS status = STATUS_ERROR; if (this->m_thread != NULL) status = (this->m_thread)->Terminate(); return status; };

    /* Accessor methods */
    RET_STATUS SetAffinity (uint_t core) { RET_STATUS status = STATUS_ERROR; if (this->m_thread != NULL) status = (this->m_thread)->SetAffinity(core); return status; };
    RET_STATUS SetPriority (uint_t policy, uint_t priority) { RET_STATUS status = STATUS_ERROR; if (this->m_thread != NULL) status = (this->m_thread)->SetPriority(policy, priority); return status; };
    RET_STATUS SetTimeout (uint64_t timeout) { this->m_timeout = timeout; return STATUS_SUCCESS; };

    /* Miscellaneous methods */
    RET_STATUS Preamble (void) { return this->Configure(); };
    RET_STATUS Do (void);
#if __cplusplus > 199711L
    RET_STATUS RegisterHandler (std::function<void(void)> cb) { this->m_recv_cb = cb; return STATUS_SUCCESS; };
#else
    RET_STATUS RegisterHandler (AsynchronousSubscriberIface* iface) { this->m_iface = iface; return STATUS_SUCCESS; };
#endif
    /* Constructor methods */
    AsynchronousSubscriberImpl (void) : SubscriberImpl() { /* Initialize resources */ this->Initialize(); return; };
    AsynchronousSubscriberImpl (Metadata_t& mdata) : SubscriberImpl(mdata) { /* Initialize resources */ this->Initialize(); return; };
    AsynchronousSubscriberImpl (Topic& topic) : SubscriberImpl(topic) { /* Initialize resources */ this->Initialize(); return; };

    /* Destructor method */
    virtual ~AsynchronousSubscriberImpl (void) { if (this->m_thread != NULL) delete this->m_thread; if (this->m_stats != NULL) delete this->m_stats; };

};

/* Global variables */

/* Function declaration */

/* Function definition */

} /* namespane sdn */

extern "C" {

#endif /* __cplusplus */

/* ToDo - Insert C API declaration */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AsynchronousSubscriberImpl_h_ */
