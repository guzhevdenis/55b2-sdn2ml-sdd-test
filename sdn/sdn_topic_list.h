//+============================================================================
// $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn_topic_list.h $
// $Id: sdn_topic_list.h 179115 2026-01-09 15:15:20Z kimh19 $
//
// Project       : CODAC Core System
//
// Description   : SDN Software
//
// Author        : Hyung Gi KIM
//
// Copyright (c) : 2010-2026 ITER Organization,
//                 CS 90 046
//                 13067 St. Paul-lez-Durance Cedex
//                 France
//
// This file is part of ITER CODAC software.
// For the terms and conditions of redistribution or use of this software
// refer to the file ITER-LICENSE.TXT located in the top level directory
// of the distribution package.
//
//-============================================================================

#ifndef SDN_TOPIC_LIST_H
#define SDN_TOPIC_LIST_H

/*! \file sdn_topic_list.h 
 *  \brief Class TopicList Definition 
 */

#include <string>
#include <vector>


class PubsubData
{
public:
  std::string m_strHostName;
  std::string m_strAppName;
  std::string m_strIpAddr;
  int m_bPublisher;
};


class Metadata
{
public:
  std::string m_strName;
  std::string m_strDescription;
  unsigned short m_version;
  unsigned short m_payloadSize;
  char m_bEvent;
};


/// class TopicData
/// Manages Topic related information, which are metadata, publisher, and list of subscribers. 
///
class TopicData
{
public:
  std::string m_strMcastAddr;
  std::string m_strMcastPort;

  TopicData()
  {
    m_metadata.m_strName = "";
    m_metadata.m_strDescription = "";
    m_metadata.m_version = 0;
    m_metadata.m_payloadSize = 0;
    m_metadata.m_bEvent = 0;
  };

  int isEmpty();
  int isExist(const char *host_name, const char *app_name, int is_publisher,
              const char *ip_addr = NULL);
  int add(const char *host_name, const char *app_name, int is_publisher,
          const char *ip_addr = NULL);
  int remove(const char *host_name, const char *app_name, int is_publisher,
             const char *ip_addr = NULL);
  char *getTopicName();
  int setMetadata(Metadata metadata);
  int getMetadata(Metadata * p_metadata);
  int getPublisher(PubsubData * p_pubdata);
  int getSubscriberList(std::vector < PubsubData > *p_list_subdata);

private:
  Metadata m_metadata;

  std::vector < PubsubData > m_vectorPub;
  std::vector < PubsubData > m_vectorSub;
};


/// class TopicList
/// Manages TopicData list. 
///
class TopicList
{
public:
  TopicData * newTopicData(const char *topic_name, const char *topic_desc,
                           const char *topic_version,
                           const char *payload_size, const char *is_event);
  int removeTopicData(const char *topic_name);
  TopicData *getTopicData(const char *topic_name);
    std::vector < TopicData * >*getTopicDataList();

private:
    std::vector < TopicData * >m_vectorTopicData;
};

#endif
