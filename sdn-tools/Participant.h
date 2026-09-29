/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/Participant.h $
* $Id: Participant.h 179115 2026-01-09 15:15:20Z kimh19 $
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
 * @file Participant.h
 * @brief Header file for sdn::Participant interface class
 * @date 18/04/2018
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 *
 * @detail This header file contains the definition of the Participant
 * interface class.
 */

#ifndef _Participant_h_
#define _Participant_h_

// Global header files

// Local header files

#include "constants.h" // Constants valid for this scope
#include "sdn-types.h" // Misc. type definition, e.g. RET_STATUS

#include "sdn-topic.h" // sdn::Topic and sdn::Metadata_t definition

// Constants

#ifdef __cplusplus

// Type definition

namespace sdn {

class Header; // Forward class declaration
class Packet; // Forward class declaration

/**
 * @brief The class provides the interface definition for sdn::Participants.
 * @detail sdn::Publisher and sdn::Subscriber provide the interface defined
 * here.
 *
 * A sdn::Participant is composed of a sdn::core::Participant and a
 * sdn::disc::Messenger. The sdn::core::Participant handles topic
 * communication while the sdn::disc::Messenger takes care of the discovery
 * protocol over the same network interface.
 */

class ParticipantIface
{
	private:
//		virtual ParticipantIface& operator= (const ParticipantIface& iface) const = 0; // Pure virtual method
//		ParticipantIface& operator= (const ParticipantIface& iface);

  public:

    /**
     * @brief Reference to message header instance.
     */
    Header* m_header; // The instance of the class // For backward compatibility purposes with v1.0

    /**
     * @brief Reference to the topic instance.
     */
    Topic* m_topic;   // The instance of the class // For backward compatibility purposes with v1.0

    /**
     * @brief Reference to the message.
     */
    Packet* m_packet;   // The instance of the class // For backward compatibility purposes with v1.0

    /**
     * @brief Constructor. NOOP.
     * @post
     *   m_header = m_footer = m_topic = NULL
     */
    ParticipantIface (void) { this->m_header = NULL; this->m_topic = NULL; this->m_packet = NULL; return; };

    /**
     * @brief Destructor. NOOP.
     */
    virtual ~ParticipantIface (void) {};

    ParticipantIface& operator= (const ParticipantIface& iface) { (void)iface; return *this; }; // BBR - ???????

    /**
     * @brief Accessor method.
     * @detail Test if the Participant is fully initialised.
     * @return TRUE if initialised, i.e. ready for operation.
     */
    virtual bool IsInitialized (void) const = 0; // Pure virtual method

    /**
     * @brief Accessor method. Refer to Bug 10327.
     * @detail The method is used to detect whether the payload held in memory by the
     * Participant is following the same byte ordering as the host native ordering
     * scheme, and whether application-side conversion is required or not after the
     * payload is accessed by reference or copy operation. 
     * @return TRUE if the message header or payload is directly interpretable by
     * the host without byte ordering conversion required, FALSE otherwise. 
     */
    virtual bool IsPayloadOrdered (void) const = 0; // Pure virtual method

    /**
     * @brief Accessor method.
     * @detail Returns the interface bound to by the Participant.
     * @return Interface name.
     */
    virtual const char* GetInterface (void) const = 0; // Pure virtual method

    /**
     * @brief Accessor method.
     * @detail Returns the topic name used by the Participant.
     * @return Topic name.
     */
    virtual const char* GetTopicName (void) const = 0; // Pure virtual method

    /**
     * @brief Accessor method.
     * @detail Returns the topic size used by the Participant.
     * @return Topic size.
     */
    virtual uint_t GetTopicSize (void) const = 0; // Pure virtual method

    /**
     * @brief Accessor method.
     * @detail Returns the reference to the message header.
     * @return Header reference.
     */
    virtual void* GetTopicHeader (void) const = 0; // Pure virtual method 

    /**
     * @brief Accessor method.
     * @detail Returns the reference to the message payload, i.e. topic instance
     * @return Payload reference.
     */
    virtual void* GetTopicInstance (void) const = 0; // Pure virtual method 

    /**
     * @brief Accessor method. Specifies the network interface to bind to.
     * @detail The method can be called to overwrite the default SDN interface
     * which is defined through SDN_INTERFACE_NAME environment variable or set
     * to 'lo' if undefined.
     */
    virtual RET_STATUS SetInterface (const char* name) = 0; // Pure virtual method

    // Deleted - virtual RET_STATUS SetTopicName (const char* name) = 0; // Pure virtual method

    /**
     * @brief Accessor method. Specifies the associated topic through metadata.
     * @detail The method can be called associate the Participant to a topic by
     * means of its metadata, i.e. name, size, version, (optionally) MCAST mapping.
     * The topic, associated message payload, etc. are instantiated upon calling
     * the Configure() method.
     */
    virtual RET_STATUS SetMetadata (Metadata_t& mdata) = 0; // Pure virtual method

    /**
     * @brief Accessor method. Specifies the associated topic by reference.
     * @detail The method can be called associate the Participant to a topic by
     * means of an previously created instance. The topic must be disposed of by
     * the calling application. The method is useful for applications where the
     * topic data type can be externally defined or introspected.
     */
    virtual RET_STATUS SetTopic (Topic& topic) = 0; // Pure virtual method

    /**
     * @brief Accessor method. Specifies the depth of the socket buffer.
     * @param depth The receive/send buffer depth in number of topic instances.
     */
    virtual RET_STATUS SetBufferDepth (uint_t depth) = 0; // Pure virtual method

    /**
     * @brief Accessor method.
     * @detail The method can be used to provide a function reference to be called
     * as part of the message handling process.
     */
    virtual RET_STATUS SetCallback (void (* cb)(void*)) = 0; // Pure virtual method

    /**
     * @brief SetCallback method.
     * @detail The method can be used to provide a function reference to be called
     * with application-specific attribute as part of the message handling process.
     */
    virtual RET_STATUS SetCallback (void (* cb)(void*), void* attr) = 0; // Pure virtual method

    virtual int GetSocket (void) = 0; // Pure virtual method
    
    /**
     * @brief Configure method. Completes the instance initialisation.
     * @detail The method is called after participant initialisation to
     * complete the configuration phase, e.g. interface defined, callback
     * installed, etc. The method configures the sdn::Participant, allocates
     * memory for message handling, gratuitously announces instantiation, etc.
     */
    virtual RET_STATUS Configure (void) = 0; // Pure virtual method

    /**
     * @brief Copy method. 
     * @detail Copies to topic instance in/out of the message buffer held by
     * the sdn::Participant.
     *
     * @warning In order to enforce appropriate constness of the method and
     * attributes, the method should not be common to all sdn::Participants
     * but defined only for specialised implementations.
     */
    virtual RET_STATUS CopyTopicInstance (void* instance, uint_t size) = 0; // Pure virtual method

    virtual RET_STATUS Do (void) = 0; // Pure virtual method

    /**
     * @brief DoBackgroundActivity method.
     * @detail The method performs house-keeping operations and should be
     * called at regular intervals in order to handle e.g. the discovery
     * protocol and replies to topic introspection requests, etc.
     */
    virtual RET_STATUS DoBackgroundActivity (void) = 0; // Pure virtual method

};

typedef ParticipantIface Participant;

// Global variables

// Function declaration

// Function definition

} // namespace sdn

extern "C" {

#endif // __cplusplus

// ToDo - Insert C API declaration

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _Participant_h_
