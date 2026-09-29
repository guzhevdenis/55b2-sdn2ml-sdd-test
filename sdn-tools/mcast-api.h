#ifndef MCAST_API_H
#define MCAST_API_H

/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/mcast-api.h $
* $Id: mcast-api.h 179115 2026-01-09 15:15:20Z kimh19 $
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

// Global header files

// Local header files

#include "sdn-types.h" // Misc. type definition, e.g. RET_STATUS

#include "base-api.h" // Interface class definition

// Constants

#ifdef __cplusplus

// Type definition

namespace ccs {

namespace mcast {

class MessengerImpl;  // Forward class declaration
class PublisherImpl;  // Forward class declaration
class SubscriberImpl; // Forward class declaration

typedef base::ParticipantIface Participant;

typedef class PublisherIface : public base::PublisherIface
{

  private:

    PublisherImpl* p_impl;

  public:

    // Initializer methods

    // Accessor methods
    virtual const char* GetInterfaceAddress (void) const; // Specializes virtual method
    virtual const char* GetInterface (void) const;        // Specializes virtual method
    virtual const char* GetAddress (void) const { return this->GetMCastAddr(); }; // Specializes virtual method
    virtual uint_t GetPort (void) const { return this->GetMCastPort(); };         // Specializes virtual method
    const char* GetMCastAddr (void) const;
    uint_t GetMCastPort (void) const;

    virtual void SetInterface (const char* iface); // Specializes virtual method
    virtual void SetAddress (const char* addr) { return this->SetMCastAddr(addr); }; // Specializes virtual method
    virtual void SetPort (const uint_t port) { return this->SetMCastPort(port); }; // Specializes virtual method
    void SetMCastAddr (const char* addr);
    void SetMCastPort (const uint_t port);
    virtual void SetSourcePort (const uint_t port); // Bug 10809 - Publisher-side port
    virtual void SetTimeToLive (const uint8_t ttl); // Bug 14996 - TTL needs to be configurable
    virtual void SetBufferDepth (const uint_t depth); // Bug 10920 - Socket depth in bytes

    virtual void* GetBuffer (void) const; // Specializes virtual method
    virtual uint_t GetSize (void) const; // Specializes virtual method
    virtual void SetBuffer (void* buffer, const uint_t size); // Specializes virtual method

    virtual RET_STATUS SetCallback (void (* cb)(void*)); // Specializes virtual method // Routine called before message publication
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr); // Specializes virtual method // Routine called before message publication

    virtual int GetSocket (void) const; // Specializes virtual method

    // Miscellaneous methods
    virtual RET_STATUS Open (void);  // Specializes virtual method
    virtual RET_STATUS Close (void); // Specializes virtual method

    virtual RET_STATUS Publish (const void* buffer, const uint_t size); // Specializes virtual method
    virtual RET_STATUS Publish (void); // Specializes virtual method

    // Constructor methods
    PublisherIface (void);
    PublisherIface (const char* iface, const char* addr, const uint_t port);

    // Destructor method
    virtual ~PublisherIface (void);

} Publisher;

typedef class SubscriberIface : public base::SubscriberIface
{

  private:

    SubscriberImpl* p_impl;

  public:

    // Initializer methods

    // Accessor methods
    virtual const char* GetInterfaceAddress (void) const; // Specializes virtual method
    virtual const char* GetInterface (void) const;        // Specializes virtual method
    virtual const char* GetAddress (void) const { return this->GetMCastAddr(); }; // Specializes virtual method
    virtual uint_t GetPort (void) const { return this->GetMCastPort(); };         // Specializes virtual method
    const char* GetMCastAddr (void) const;
    uint_t GetMCastPort (void) const;

    virtual void SetInterface (const char* iface); // Specializes virtual method
    virtual void SetAddress (const char* addr) { return this->SetMCastAddr(addr); }; // Specializes virtual method
    virtual void SetPort (const uint_t port) { return this->SetMCastPort(port); }; // Specializes virtual method
    void SetMCastAddr (const char* addr);
    void SetMCastPort (const uint_t port);
    virtual void SetBufferDepth (const uint_t depth); // Bug 10920 - Socket depth in bytes

    virtual void* GetBuffer (void) const; // Specializes virtual method
    virtual uint_t GetSize (void) const; // Specializes virtual method
    virtual void SetBuffer (void* buffer, const uint_t size); // Specializes virtual method

    virtual RET_STATUS SetCallback (void (* cb)(void*)); // Specializes virtual method // Routine called upon message reception
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr); // Specializes virtual method // Routine called upon message reception
#if 0
    bool IsUpdated (void); // Successful Receive

    uint64_t GetTimeout (void) const;
    void SetTimeout (const uint64_t timeout);
#endif
    virtual int GetSocket (void) const; // Specializes virtual method

    // Miscellaneous methods
    virtual RET_STATUS Open (void);  // Specializes virtual method
    virtual RET_STATUS Close (void); // Specializes virtual method

    virtual RET_STATUS Receive (void* buffer, uint_t& size, const uint64_t timeout); // Blocking wait with configurable timeout - Size updated with actual number of bytes received
    virtual RET_STATUS Receive (void* buffer, uint_t& size);                         // Blocking wait - Size updated with actual number of bytes received
    virtual RET_STATUS Receive (const uint64_t timeout); // Blocking wait with configurable timeout using buffer passed with ccs::base::Participant::SetBuffer()
    virtual RET_STATUS Receive (void);                   // Blocking wait using buffer passed with ccs::base::Participant::SetBuffer()

//    int GetSocket (void); // Specializes virtual method

    // Constructor methods
    SubscriberIface (void);
    SubscriberIface (const char* iface, const char* addr, const uint_t port);

    // Destructor method
    virtual ~SubscriberIface (void);

} Subscriber;

typedef class MessengerIface : public base::MessengerIface
{

  private:

    MessengerImpl* p_impl;

  public:

    // Initializer methods

    // Accessor methods
    virtual const char* GetInterfaceAddress (void) const; // Specializes virtual method
    virtual const char* GetInterface (void) const;        // Specializes virtual method
    virtual const char* GetAddress (void) const { return this->GetMCastAddr(); }; // Specializes virtual method
    virtual uint_t GetPort (void) const { return this->GetMCastPort(); };         // Specializes virtual method
    const char* GetMCastAddr (void) const;
    uint_t GetMCastPort (void) const;

    virtual void SetInterface (const char* iface); // Specializes virtual method
    virtual void SetAddress (const char* addr) { return this->SetMCastAddr(addr); }; // Specializes virtual method
    virtual void SetPort (const uint_t port) { return this->SetMCastPort(port); }; // Specializes virtual method
    void SetMCastAddr (const char* addr);
    void SetMCastPort (const uint_t port);
    virtual void SetBufferDepth (const uint_t depth); // Bug 10920 - Socket depth in bytes
    virtual void SetTimeToLive (const uint8_t ttl); // Bug 14996 - TTL needs to be configurable

    virtual void* GetBuffer (void) const; // Specializes virtual method
    virtual uint_t GetSize (void) const; // Specializes virtual method
    virtual void SetBuffer (void* buffer, const uint_t size); // Specializes virtual method

    virtual RET_STATUS SetCallback (void (* cb)(void*)); // Specializes virtual method // Routine called upon message reception
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr); // Specializes virtual method // Routine called upon message reception
#if 0
    bool IsUpdated (void); // Successful Receive

    uint64_t GetTimeout (void) const;
    void SetTimeout (const uint64_t timeout);
#endif
    virtual int GetSocket (void) const; // Specializes virtual method

    // Miscellaneous methods
    virtual RET_STATUS Open (void);  // Specializes virtual method
    virtual RET_STATUS Close (void); // Specializes virtual method

    virtual RET_STATUS Publish (const void* buffer, const uint_t size); // Specializes virtual method
    virtual RET_STATUS Publish (void); // Specializes virtual method

    virtual RET_STATUS Receive (void* buffer, uint_t& size, const uint64_t timeout); // Blocking wait with configurable timeout - Size updated with actual number of bytes received
    virtual RET_STATUS Receive (void* buffer, uint_t& size);                         // Blocking wait - Size updated with actual number of bytes received
    virtual RET_STATUS Receive (const uint64_t timeout); // Blocking wait with configurable timeout using buffer passed with ccs::base::Participant::SetBuffer()
    virtual RET_STATUS Receive (void);                   // Blocking wait using buffer passed with ccs::base::Participant::SetBuffer()

    virtual RET_STATUS Reply (const void* buffer, const uint_t size); // Specializes virtual method
    virtual RET_STATUS Reply (void) { return STATUS_ERROR; }; // Specializes virtual method

//    int GetSocket (void); // Specializes virtual method

    // Constructor methods
    MessengerIface (void);
    MessengerIface (const char* iface, const char* addr, const uint_t port);

    // Destructor method
    virtual ~MessengerIface (void);

} Messenger;

// Global variables

// Function declaration

// Function definition

bool IsAddrValid (const char* addr);

} // namespace mcast

} // namespace ccs

extern "C" {

#endif // __cplusplus

// ToDo - Insert C API declaration

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MCAST_API_H
