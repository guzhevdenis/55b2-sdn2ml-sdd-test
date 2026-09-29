/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/base-api.h $
* $Id: base-api.h 179115 2026-01-09 15:15:20Z kimh19 $
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

/**
 * @file base-api.h
 * @brief Header file for common participants interface class definition
 * @date 25/04/2017
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 * @detail This header file contains the definition of the ccs::base::Participant, etc.
 * interface classes. These interfaces must be provided by any particular participant
 * implementation.
 * @todo Doxygen documentation for the interface definition.
 */

#ifndef _BaseAPI_h_
#define _BaseAPI_h_

// Global header files

// Local header files

#include "sdn-types.h" // Misc. type definition, e.g. RET_STATUS

// Constants

#ifdef __cplusplus

// Type definition

namespace ccs {

namespace base {

typedef class ParticipantIface // Interface definition
{

  public:

    // Initializer methods

    // Accessor methods
    virtual const char* GetInterfaceAddress (void) const = 0; // Pure virtual method
    virtual const char* GetInterface (void) const = 0;        // Pure virtual method
    virtual const char* GetAddress (void) const = 0;          // Pure virtual method
    virtual uint_t GetPort (void) const = 0;                  // Pure virtual method

    virtual void SetInterface (const char* iface) = 0; // Pure virtual method
    virtual void SetAddress (const char* addr) = 0;    // Pure virtual method
    virtual void SetPort (const uint_t port) = 0;      // Pure virtual method

    virtual void SetBufferDepth (const uint_t depth) = 0; // Bug 10920 - Socket depth in bytes

    virtual void* GetBuffer (void) const = 0; // Pure virtual method
    virtual uint_t GetSize (void) const = 0;  // Pure virtual method
    virtual void SetBuffer (void* buffer, const uint_t size) = 0; // Pure virtual method

    virtual RET_STATUS SetCallback (void (* cb)(void*)) = 0; // Pure virtual method - Callback used as part of the message handling mechanism
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr) = 0; // Pure virtual method - Callback used as part of the message handling mechanism

    virtual int GetSocket (void) const = 0; // Pure virtual method

    // Miscellaneous methods
    virtual RET_STATUS Do (void) = 0;    // Pure virtual method
    virtual RET_STATUS Open (void) = 0;  // Pure virtual method
    virtual RET_STATUS Close (void) = 0; // Pure virtual method

    // Constructor methods
    ParticipantIface (void) {};

    // Destructor method
    virtual ~ParticipantIface (void) {};

} Participant;

typedef class PublisherIface : public ParticipantIface
{

  public:

    // Initializer methods

    // Accessor methods
    virtual void SetSourcePort (const uint_t port) = 0; // Bug 10809 - Publisher-side port
    virtual void SetTimeToLive (const uint8_t ttl) = 0; // Bug 14996 - TTL needs to be configurable

    // Miscellaneous methods
    virtual RET_STATUS Do (void) { return this->Publish(); }; // Specializes virtual method

    virtual RET_STATUS Publish (const void* buffer, const uint_t size) = 0; // Pure virtual method
    virtual RET_STATUS Publish (void) = 0;                                  // Pure virtual method - Publish buffer passed using ccs::base::Participant::SetBuffer()

    // Constructor methods
    PublisherIface (void) {};

    // Destructor method
    virtual ~PublisherIface (void) {};

} Publisher;

typedef class SubscriberIface : public ParticipantIface
{

  public:

    // Initializer methods

    // Accessor methods

    // Miscellaneous methods
    virtual RET_STATUS Do (void) { return this->Receive(); }; // Specializes virtual method

    virtual RET_STATUS Receive (void* buffer, uint_t& size, const uint64_t timeout) = 0; // Pure virtual method - Blocking wait with configurable timeout - Buffer size updated with received size
    virtual RET_STATUS Receive (void* buffer, uint_t& size) = 0;                         // Pure virtual method - Blocking wait - Buffer size updated with received size
    virtual RET_STATUS Receive (const uint64_t timeout) = 0; // Pure virtual method - Blocking wait with configurable timeout using buffer passed with ccs::base::Participant::SetBuffer()
    virtual RET_STATUS Receive (void) = 0;                   // Pure virtual method - Blocking wait using buffer passed with ccs::base::Participant::SetBuffer()

    // Constructor methods
    SubscriberIface (void) {};

    // Destructor method
    virtual ~SubscriberIface (void) {};

} Subscriber;

typedef class MessengerIface : public ParticipantIface
{

  public:

    // Initializer methods

    // Accessor methods

    // Miscellaneous methods
    virtual RET_STATUS Do (void) { return this->Receive(); }; // Specializes virtual method

    virtual RET_STATUS Publish (const void* buffer, const uint_t size) = 0; // Pure virtual method
    virtual RET_STATUS Publish (void) = 0;                                  // Pure virtual method - Publish buffer passed using ccs::base::Participant::SetBuffer()

    virtual RET_STATUS Receive (void* buffer, uint_t& size, const uint64_t timeout) = 0; // Pure virtual method - Blocking wait with configurable timeout - Buffer size updated with received size
    virtual RET_STATUS Receive (void* buffer, uint_t& size) = 0;                         // Pure virtual method - Blocking wait - Buffer size updated with received size
    virtual RET_STATUS Receive (const uint64_t timeout) = 0; // Pure virtual method - Blocking wait with configurable timeout using buffer passed with ccs::base::Participant::SetBuffer()
    virtual RET_STATUS Receive (void) = 0;                   // Pure virtual method - Blocking wait using buffer passed with ccs::base::Participant::SetBuffer()

    virtual RET_STATUS Reply (const void* buffer, const uint_t size) = 0; // Pure virtual method
    virtual RET_STATUS Reply (void) = 0;                                  // Pure virtual method - Reply using buffer passed with ccs::base::Participant::SetBuffer()

    // Constructor methods
    MessengerIface (void) {};

    // Destructor method
    virtual ~MessengerIface (void) {};

} Messenger;

// Global variables

// Function declaration

// Function definition

} // namespace base

} // namespace ccs

#endif // __cplusplus

#endif // _BaseAPI_H
