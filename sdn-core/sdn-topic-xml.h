/******************************************************************************
* $HeadURL: https://svnpub.codac.iter.org/codac/iter/codac/dev/units/m-sdn-core/tags/CODAC-CORE-7.4.0/src/main/c++/include/sdn-topic-xml.h $
* $Id: sdn-topic-xml.h 179115 2026-01-09 15:15:20Z kimh19 $
*
* Project       : CODAC Core System
*
* Description   : SDN - Infrastructure tools - Prototype
*
* Author        : Bertrand Bauvir
*
* Copyright (c) : 2010-2026 ITER Organization,
*                 CS 90 046
*                 13067 St. Paul-lez-Durance Cedex
*                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

#ifndef _TopicXML_h
#define _TopicXML_h

/**
 * @file TopicXML.h
 * @brief Header file for TopicHelper routines
 * @date 14/09/2017
 * @author Bertrand Bauvir (IO)
 * @copyright 2010-2017 ITER Organization
 * @detail This header file contains the definition of various sdn::Topic helper
 * routines supporting the definition of topic and data type by means of XML
 * definition files. The code herein has dependency to libxml2 v2.7.6 installed with
 * CODAC Core System v5.4 or above. Inclusion of this header files requires availability
 * of <libxml/tree.h> and linking the application code with the appropriate libraries.
 * @todo InstantiateFromStream to support as well using discovery messages on-the-wire.
 */

/* Global header files */

#include <libxml/tree.h> 
//#include <libxml/parser.h>

#include <common/AnyTypeDatabase.h>

/* Local header files */

#include "constants.h" /* Constants valid for this scope */
#include "sdn-types.h" /* Misc. type definition, e.g. RET_STATUS */
#include "sdn-tools.h"

#include "sdn-topic.h" /* sdn::Topic class definition */

/* Constants */

/* Type definition */

namespace sdn {

namespace TopicHelper {

/* Global variables declaration */
extern uint_t g_parser_init;

/* Function declaration */

/* Bug 8874 - SDN_TOPIC_PATH variable does not allow for values above 64 characters */
static char _path_list [] [PATH_MAX_LENGTH] = 
{

  "${SDN_TOPIC_PATH}",
  "${CODAC_CONF}/sdn", 
  "${CODAC_CONF}/default", 
  "/tmp", 
  "./target/main/resources", 
  "./target/test/resources", 
  "../../../main/resources", 
  "../../resources", 
  "../resources", 
  ".", 
  EOT_KEYWORD

};

static char _suff_list [] [STRING_MAX_LENGTH] = 
{

  /* Bug 10528 - Enforce using topic name and not file name when trying and locate definition file */
  //"",
  ".xml",
  "_typedef.xml",
  EOT_KEYWORD

};


/* Function definition */

/**
 * @brief Retrieves the path to the XML topic definition file by topic name.
 * @detail This method retrieves the path to the XML topic definition file by
 * topic name. The char array is left unmodified in case a topic definition
 * file can not be found. 
 * @return STATUS_SUCCESS if a topic definition file may be found for the named
 * topic, STATUS_ERROR otherwise.
 */

static inline RET_STATUS LocateDefinitionFile (const char* name, char* file_path = NULL, uint_t max_size = STRING_MAX_LENGTH)
{

  log_trace("TopicHelper::LocateDefinitionFile('%s') - Entering method", name);

  RET_STATUS status = STATUS_ERROR;

  if (name == NULL) return status;

  if (file_path == NULL)
    {
      char tmp [1024];
      file_path = (char*) tmp; max_size = 1024;
    }

  // Bug 11538 - SDD generates topic description files with '_' replacing CBS separators '-' and ':'
  std::string topic_name (name);

  if (std::string::npos != topic_name.find(':', 0u))
    {

      {
        std::size_t pos = 0u;
        
        bool ok = (std::string::npos != topic_name.find('-', 0u));
        
        while (ok)
          {
            pos = topic_name.find('-', pos);
            ok = (std::string::npos != pos);
            
            if (ok)
              {
                topic_name.replace(pos, 1u, "_");
              }
          }
      }
      
      {
        std::size_t pos = 0u;
        
        bool ok = (std::string::npos != topic_name.find(':', 0u));
        
        while (ok)
          {
            pos = topic_name.find(':', pos);
            ok = (std::string::npos != pos);
            
            if (ok)
              {
                topic_name.replace(pos, 1u, "_");
              }
          }
      }
      
      log_info("TopicHelper::LocateDefinitionFile('%s') - Will use '%s'", name, topic_name.c_str());

    }

  char file_name [PATH_MAX_LENGTH];

  bool found = false;

  uint_t path_index = 0;

  while (strncmp(_path_list[path_index], EOT_KEYWORD, PATH_MAX_LENGTH) != 0)
    {
      char* conf_path = _path_list[path_index];

      log_debug("Try and resolve path '%s'", conf_path); 

      if (ccs::HelperTools::ResolveVariableString(conf_path, conf_path, PATH_MAX_LENGTH) != STATUS_SUCCESS)
        {
          log_info("Unable to resolve path '%s'", conf_path);
        }
      else
        {
          log_info("Path resolves to '%s'", conf_path);
        }

      uint_t suff_index = 0;

      while (strncmp(_suff_list[suff_index], EOT_KEYWORD, PATH_MAX_LENGTH) != 0)
        {
          char* suffix = _suff_list[suff_index];

          if (ccs::HelperTools::Exist((char*) conf_path, topic_name.c_str(), (char*) suffix) == true)
            {
              found = true;

              snprintf(file_name, PATH_MAX_LENGTH, "%s%s", topic_name.c_str(), (char*) suffix);

              log_debug("Load '%s' topic definition file from '%s'", topic_name.c_str(), conf_path);

              snprintf(file_path, max_size, "%s/%s", conf_path, file_name);

              break;
            }

          suff_index += 1;
        }

      if (found == true) 
        {
          /* No need to search further */
          log_debug("Located '%s' topic definition file", file_path);
          status = STATUS_SUCCESS;
          break;
        }

      path_index += 1;
    }

  if (found != true) log_error("Unable to locate '%s' topic definition file", name);

  log_trace("TopicHelper::LocateDefinitionFile('%s') - Leaving method", name);

  return status;

}

/**
 * @brief Initialize the parser for topic XML definition files.
 * @detail The parser used to define topic through file is not fully re-entrant.
 * In case topics are defined through file introspection in a concurrent manner,
 * this method should be called ahead of topic instantiation.
 * @return STATUS_SUCCESS if the parser is initialized, STATUS_ERROR otherwise.
 */

static inline RET_STATUS InitializeParser (void)
{

  log_trace("TopicHelper::InitializeParser - Entering method");

  RET_STATUS status = STATUS_ERROR;
#if __cplusplus > 197711L
  if (!__atomic_fetch_add(&g_parser_init, 1u, __ATOMIC_SEQ_CST)) 
#else
  if (!__sync_fetch_and_add(&g_parser_init, 1u)) 
#endif
    {
      log_info("TopicHelper::InitializeParser - Call xmlInitParser");
      xmlInitParser();
    }

  status = STATUS_SUCCESS;

  log_trace("TopicHelper::InitializeParser - Leaving method");

  return status;

}

/**
 * @brief Clean-up parser resources.
 * @see InitializeParser.
 * @see LoadDefinitionFile.
 * @return STATUS_SUCCESS.
 */

static inline RET_STATUS ReleaseParser (void)
{

  log_trace("TopicHelper::ReleaseParser - Entering method");

  RET_STATUS status = STATUS_ERROR;
#if __cplusplus > 197711L
  if (!__atomic_sub_fetch(&g_parser_init, 1u, __ATOMIC_SEQ_CST)) 
#else
  if (!__sync_sub_and_fetch(&g_parser_init, 1u)) 
#endif
    {
      log_info("TopicHelper::ReleaseParser - Call xmlCleanupParser");
      xmlCleanupParser();
    }

  status = STATUS_SUCCESS;

  log_trace("TopicHelper::ReleaseParser - Leaving method");

  return status;

}

/**
 * @brief Defines topic metadata from XML definition.
 * @detail This method parses a XML definition string and configures the sdn::Metadata_t reference.
 * @return True if the topic metadata is configured.
 */

static inline bool ParseXMLContent (Metadata_t& mdata, const ccs::types::char8 * const content)
{

  log_trace("TopicHelper::ParseXMLContent - Entering method");

  if (ccs::HelperTools::Contain(content, "dataType format=\"json\""))
    {
      log_info("ParseXMLContent - Found datatype definition ..");
      // Try and register datatypes
      ccs::base::GlobalTypeDatabase::TryAndRegister(content);
      ccs::HelperTools::DumpGTDBToLog();
    }

  // Rely on libxml2 to load and parse type definition file
  InitializeParser();
  LIBXML_TEST_VERSION;

  xmlDocPtr xDoc = xmlReadMemory(content, strlen(content), "undefined.xml", NULL, 0);
  xmlNodePtr xCurNode;

  bool status = (NULL != xDoc);

  if (!status) 
    {
      log_error("TopicHelper::ParseXMLContent - Unable to parse buffer '%s'", content);
      return status;
    }
  else 
    {
      xCurNode = xmlDocGetRootElement(xDoc);
      status = ccs::HelperTools::StringCompare((const char*) xCurNode->name, "topic");
    }

  if (status)
    { // Must have name and version properties
      status = ((NULL != xmlHasProp(xCurNode, (xmlChar*) "name")) && (NULL != xmlHasProp(xCurNode, (xmlChar*) "name")));
    }

  // Topic attributes

  if (status)
    {
      // WARNING - Memory leak using xmlGetProp without systematic xmlFree
      xmlChar* prop; 

      prop = xmlGetProp(xCurNode, (xmlChar*) "name"); (void)ccs::HelperTools::SafeStringCopy(mdata.name, (const char*) prop, ccs::types::MaxStringLength); xmlFree(prop);
      prop = xmlGetProp(xCurNode, (xmlChar*) "version"); mdata.vers = static_cast<ccs::types::uint32>(ccs::HelperTools::ToInteger((const char*) prop)); xmlFree(prop);

      // Optional attributes
      if (NULL != (prop = xmlGetProp(xCurNode, (xmlChar*) "description")))
        { 
          (void)ccs::HelperTools::SafeStringCopy(mdata.desc, (const char*) prop, 128u);
          xmlFree(prop);
        }

      if (NULL != (prop = xmlGetProp(xCurNode, (xmlChar*) "uid")))
        { 
          mdata.hash = static_cast<ccs::types::uint32>(ccs::HelperTools::ToInteger((const char*) prop));
          xmlFree(prop); 
        }

      if (NULL != (prop = xmlGetProp(xCurNode, (xmlChar*) "size")))
        { 
          mdata.size = static_cast<ccs::types::uint32>(ccs::HelperTools::ToInteger((const char*) prop)); 
          xmlFree(prop);
        }

      if (NULL != (prop = xmlGetProp(xCurNode, (xmlChar*) "mapping")))
        {
          ccs::types::string topic_addr = STRING_UNDEFINED;

          (void)ccs::HelperTools::SafeStringCopy(topic_addr, (const char*) prop, ccs::types::MaxStringLength);
          xmlFree(prop);

          // Put a zero in lieu of ':'
          if (ccs::HelperTools::Contain(topic_addr, ':'))
            {
              (void)ccs::HelperTools::SafeStringCopy(mdata.mcast_group, topic_addr, ccs::types::MaxIPv4AddrLength, ':');
              mdata.mcast_port = static_cast<ccs::types::uint32>(ccs::HelperTools::ToInteger(&(topic_addr[ccs::HelperTools::Find(topic_addr, ':')+1])));
            }
        }
      else
        {
          log_notice("Generate topic mapping from hash table");
          GenerateMCastAddress(mdata);
        }

      if (NULL != (prop = xmlGetProp(xCurNode, (xmlChar*) "dataType")))
        { 
          ccs::types::string type_name = STRING_UNDEFINED;
          (void)ccs::HelperTools::SafeStringCopy(type_name, (const char*) prop, ccs::types::MaxStringLength);
          xmlFree(prop);

          // Should be a valid defined type
          status = ccs::base::GlobalTypeDatabase::IsValid(type_name);

          if (status)
            {
              mdata.type.intro = ccs::base::SharedReference<ccs::types::AnyType>(const_cast<ccs::types::AnyType*>(ccs::base::GlobalTypeDatabase::GetType(type_name).GetReference()), ccs::base::GlobalTypeDatabase::GetType(type_name).GetCounter());
              status = static_cast<bool>(mdata.type.intro);
            }
        }
    }

  // Further attributes or datatye definition

  if (status)
    {
      // Test if topic definition includes children elements
      if (xCurNode->children != NULL)
        {
          log_notice("Topic '%s' does provide type definition", mdata.name);

          xCurNode = xCurNode->children; 

          // Warning - '\n' if exists is considered the first child

          if (xCurNode->type != XML_ELEMENT_NODE)
            { 
              log_debug("Discard XML node type '%d'", xCurNode->type); 
              xCurNode = xCurNode->next; 
            }

          if ((xCurNode->type == XML_ELEMENT_NODE) && (strcmp((char*) xCurNode->name, (char*) "dataTypes") == 0)) 
            { 
              log_debug("Datatype definition - Skip 'datatypes'"); 
              xCurNode = xCurNode->next; 
            }

          if ((xCurNode->type == XML_ELEMENT_NODE) && (strcmp((char*) xCurNode->name, (char*) "attributes") == 0)) 
            { 
              log_debug("Nested definition - Skip 'attributes'"); 
              xCurNode = xCurNode->children;
              status = (NULL != xCurNode);
            }
          else if (status)
            {
              log_debug("Nested definition - Test for 'attribute'"); 
              status = ((xCurNode->type == XML_ELEMENT_NODE) && (strcmp((char*) xCurNode->name, (char*) "attribute") == 0));
            }

          // Instantiate class
          sdn::base::AnyType* ref = NULL_PTR_CAST(sdn::base::AnyType*);

          if (status)
            {
              log_debug("Instantiate sdn::base::AnyType"); 
              mdata.type.exten = ccs::base::SharedReference<sdn::base::AnyType>(new (std::nothrow) sdn::base::AnyType);
              ref = mdata.type.exten.GetReference();
              status = (NULL_PTR_CAST(sdn::base::AnyType*) != ref);
            }

          // Parse attributes if exist
          for (; ((xCurNode) && (status)); xCurNode = xCurNode->next) 
            {
              // Only interested in XML elements at this level
              if (xCurNode->type != XML_ELEMENT_NODE) { log_debug("Discard XML node type '%d'", xCurNode->type); continue; }
              // Only interested in 'attribute' elements
              if (strcmp((char*) xCurNode->name, (char*) "attribute") != 0) { log_debug("Discard XML element node '%s'", xCurNode->name); continue; }

              // Must have 'name' and 'type' properties
              if (xmlHasProp(xCurNode, (xmlChar*) "name") == NULL) { log_warning("Discard attribute - Does not appear to have a name"); continue; }
              if (xmlHasProp(xCurNode, (xmlChar*) "dataType") == NULL) { log_warning("Discard attribute - Does not appear to have a type"); continue; }

              char attr_name [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_rank [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_type [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_mult [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              
              xmlChar* prop; 
              
              prop = xmlGetProp(xCurNode, (xmlChar*) "name");     sstrncpy((char*) attr_name, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop);

              if (xmlHasProp(xCurNode, (xmlChar*) "dataType") != NULL)     { prop = xmlGetProp(xCurNode, (xmlChar*) "dataType");     sstrncpy((char*) attr_type, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              
              if (xmlHasProp(xCurNode, (xmlChar*) "rank") != NULL)         { prop = xmlGetProp(xCurNode, (xmlChar*) "rank");         sstrncpy((char*) attr_rank, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              if (xmlHasProp(xCurNode, (xmlChar*) "multiplicity") != NULL) { prop = xmlGetProp(xCurNode, (xmlChar*) "multiplicity"); sstrncpy((char*) attr_mult, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };

              log_debug("Found attribute '%s'", attr_name);
              
              char* attr_defl = NULL;
              
              // May have a default value, i.e. XML text as child
              if ((xCurNode->children != NULL) && (xCurNode->children->type == XML_TEXT_NODE)) 
                { 
                  log_info("Attribute '%s' is provided with a default value", attr_name);
                  attr_defl = (char*) xCurNode->children->content; char* p_buf = attr_defl; while (*p_buf != 0) { if (*p_buf == '\n') *p_buf = ' '; p_buf += 1; } /* Replace characters */
                }

              if (ref->AddAttribute(attr_rank, attr_name, attr_type, attr_mult, attr_defl) != STATUS_SUCCESS)
                {
                  log_error("SDNAnyType::AddAttribute('%s %s %s %s %s') failed", attr_rank, attr_name, attr_type, attr_mult, attr_defl); 
                  status = false;
                }
              else if (ref->HasAttribute(attr_name) == true)
                {
                  log_debug("SDNAnyType::AddAttribute('%s %s %s %s %s') successful", attr_rank, attr_name, attr_type, attr_mult, attr_defl);
                }
              else log_warning("SDNAnyType::AddAttribute successful but AnyType::HasAttribute('%s') failed - '%d'", attr_name, ref->GetAttributeNumber());


              char attr_desc [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_qual [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_unit [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              
              if (xmlHasProp(xCurNode, (xmlChar*) "description") != NULL) { prop = xmlGetProp(xCurNode, (xmlChar*) "description"); sstrncpy((char*) attr_desc, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              if (xmlHasProp(xCurNode, (xmlChar*) "qualifier") != NULL)   { prop = xmlGetProp(xCurNode, (xmlChar*) "qualifier");   sstrncpy((char*) attr_qual, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              if (xmlHasProp(xCurNode, (xmlChar*) "unit") != NULL)        { prop = xmlGetProp(xCurNode, (xmlChar*) "unit");        sstrncpy((char*) attr_unit, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              
              if (IsUndefined((char*) attr_desc) != true) log_debug("Attribute '%s' has 'description' property set to '%s'", attr_name, attr_desc);
              if (IsUndefined((char*) attr_qual) != true) log_debug("Attribute '%s' has 'qualifier' property set to '%s'", attr_name, attr_qual);
              if (IsUndefined((char*) attr_unit) != true) log_debug("Attribute '%s' has 'unit' property set to '%s'", attr_name, attr_unit);
              if (IsUndefined((char*) attr_mult) != true) log_debug("Attribute '%s' has 'multiplicity' property set to '%s'", attr_name, attr_mult);

              if (ref->AddExtAttribute(attr_rank, attr_name, attr_desc, attr_qual, attr_unit, attr_mult) != STATUS_SUCCESS)
                {
                  log_error("AnyType::AddExtAttribute('%s %s %s %s %s %s') failed", attr_rank, attr_name, attr_desc, attr_qual, attr_unit, attr_mult); 
                  status = false;
                }
              else
                {
                  log_debug("AnyType::AddExtAttribute('%s %s %s %s %s %s') successful", attr_rank, attr_name, attr_desc, attr_qual, attr_unit, attr_mult);
                }
            }

          /* Bug 10153 - Regression - Try and compress LUTable */
          ref->Compress();
        }
    }

  // Shutdown libxml
  xmlFreeDoc(xDoc);
  ReleaseParser();
  xmlMemoryDump();

  // Datatype inferred from size

  if ((false == static_cast<bool>(mdata.type.exten)) && (false == static_cast<bool>(mdata.type.intro)) && (0u != mdata.size))
    {
      mdata.type.intro = ccs::base::SharedReference<ccs::types::AnyType>(ccs::HelperTools::NewArrayType(mdata.name, ccs::types::UnsignedInteger8, mdata.size));
    }

  // Datatype inferred from <attributes> list

  if (static_cast<bool>(mdata.type.exten) && (false == static_cast<bool>(mdata.type.intro)))
    {
      mdata.type.intro = ccs::base::SharedReference<ccs::types::AnyType>(dynamic_cast<ccs::types::AnyType*>(mdata.type.exten.GetReference()), mdata.type.exten.GetCounter());
      mdata.type.intro->SetName(mdata.name);
    }

  // Datatype inferred from name

  if (static_cast<bool>(mdata.type.exten) && static_cast<bool>(mdata.type.intro))
    {
      // ToDo
    }

  // Check/force consistency .. size, hash, etc.
  mdata.size = (mdata.type.intro)->GetSize();
  //mdata.hash = 0u;

  if (static_cast<bool>(mdata.type.intro))
    {
      ccs::types::char8 buffer [1024u]; ccs::HelperTools::Serialise(mdata.type.intro, buffer, 1024u);
      log_info("Topic datatype definition '%s' ..", buffer);
    }

  if (static_cast<bool>(mdata.type.exten))
    {
      ccs::base::SharedReference<ccs::types::AnyType> _type = ccs::base::SharedReference<ccs::types::AnyType>(dynamic_cast<ccs::types::AnyType*>(mdata.type.exten.GetReference()), mdata.type.exten.GetCounter());
      ccs::types::char8 buffer [1024u]; ccs::HelperTools::Serialise(_type, buffer, 1024u);
      log_info("Topic datatype definition '%s' ..", buffer);
    }

  if (!status)
    {
      status = TopicHelper::IsMetadataValid(mdata);
    }

  log_trace("TopicHelper::ParseXMLContent - Leaving method");

  return status;

}

/**
 * @brief Defines topic from XML definition.
 * @detail This method parses a XML definition string and uses the sdn::Topic reference
 * and API to configure it.
 * @return STATUS_SUCCESS if the topic is configured, STATUS_ERROR otherwise.
 * @todo Fix re-entrancy issue with libxml2 parser.
 */

static inline RET_STATUS ParseXMLContent (Topic* self, const char* content)
{

  log_trace("TopicHelper::ParseXMLContent - Entering method");

  if (ccs::HelperTools::Contain(content, "dataType format=\"json\""))
    {
      log_info("ParseXMLContent - Found datatype definition ..");
      // Try and register datatypes
      ccs::base::GlobalTypeDatabase::TryAndRegister(content);
    }

  RET_STATUS status = STATUS_ERROR;

  /* Rely on libxml2 to load and parse type definition file */
  InitializeParser();
  LIBXML_TEST_VERSION;

  xmlDocPtr xDoc = xmlReadMemory(content, strlen(content), "undefined.xml", NULL, 0);

  bool ok = true;

  if (xDoc == NULL) 
    {
      log_error("Unable to parse buffer '%s'", content);
      return status;
    }
  else 
    {
      xmlNodePtr xCurNode = xmlDocGetRootElement(xDoc);

      if (strcmp((char*) xCurNode->name, (char*) "topic") != 0) 
        { 
          log_error("Unknown XML element node '%s'", xCurNode->name); 
          return status;
        }

      /* Parse topic attributes */
      char topic_name [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
      char topic_desc [128] = STRING_UNDEFINED; // Bug 10852 - DAN limit above 64 characters
      char topic_iden [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
      char topic_vers [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
      char topic_size [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
      char topic_addr [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
      char topic_type [STRING_MAX_LENGTH] = STRING_UNDEFINED; // Type name which should be registsred somehow

      xmlChar* prop; 

      if (xmlHasProp(xCurNode, (xmlChar*) "name") != NULL)    { prop = xmlGetProp(xCurNode, (xmlChar*) "name");    sstrncpy((char*) topic_name, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); }
      if (xmlHasProp(xCurNode, (xmlChar*) "description") != NULL) { prop = xmlGetProp(xCurNode, (xmlChar*) "description"); sstrncpy((char*) topic_desc, (char*) prop, 128u); xmlFree(prop); } // Bug 10852 - DAN limit above 64 characters
      if (xmlHasProp(xCurNode, (xmlChar*) "uid") != NULL)     { prop = xmlGetProp(xCurNode, (xmlChar*) "uid");     sstrncpy((char*) topic_iden, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); }
      if (xmlHasProp(xCurNode, (xmlChar*) "version") != NULL) { prop = xmlGetProp(xCurNode, (xmlChar*) "version"); sstrncpy((char*) topic_vers, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); }
      if (xmlHasProp(xCurNode, (xmlChar*) "size") != NULL)    { prop = xmlGetProp(xCurNode, (xmlChar*) "size");    sstrncpy((char*) topic_size, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
      if (xmlHasProp(xCurNode, (xmlChar*) "mapping") != NULL) { prop = xmlGetProp(xCurNode, (xmlChar*) "mapping"); sstrncpy((char*) topic_addr, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
      if (xmlHasProp(xCurNode, (xmlChar*) "dataType") != NULL) { prop = xmlGetProp(xCurNode, (xmlChar*) "dataType"); sstrncpy((char*) topic_type, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };

      /* Must have 'name' and 'version' properties */
      if (IsUndefined(topic_name) || IsUndefined(topic_vers))
        {
          log_error("Topic definition does not conform to expected syntax");
          return status;
        }
  
      /* Check topic name */
      if (strcmp(self->GetName(), (char*) topic_name) != 0)
        {
          log_notice("Topic name mismatch '%s %s'", self->GetName(), (char*) topic_name);
          self->SetName((char*) topic_name);
        }

      // Store recovered attributes
      self->SetVersion((char*) topic_vers);
      
      if (IsUndefined(topic_desc) != true) self->SetDescription(topic_desc); // Bug 10852 - Topic description
      if (IsUndefined(topic_iden) != true) self->SetUID(topic_iden);
      if (IsUndefined(topic_size) != true) self->SetSize(topic_size);

      if (IsUndefined(topic_type) != true) self->SetType(topic_type); // Datatype name

      // Bug 10182 - Regression - Topic instantiated from XML definition file uses the file as the source for MCAST mapping
      if (IsUndefined(topic_addr) == true)
        {
          log_notice("Generate topic mapping from hash table");
          Topic_GenerateMCastAddress(self->m_meta);
        }
      else 
        {
          char* p_char = topic_addr;
          char* p_addr = topic_addr;
          /* Put a zero in lieu of ':' */
          while (*p_char != ':') p_char += 1; 
          *p_char = 0; 
          self->SetMCastGroup(p_addr); 
          p_addr = p_char + 1;
          self->SetMCastPort(p_addr); 
        }

      // Test if topic definition includes children elements
      if (xCurNode->children != NULL)
        {
          log_notice("Topic '%s' does provide type definition", self->GetName());

          /* ToDo - Re-design this portion of the code */

          xCurNode = xCurNode->children; 

          /* Warning - '\n' if exists is considered the first child */

          if (xCurNode->type != XML_ELEMENT_NODE)
            { 
              log_debug("Discard XML node type '%d'", xCurNode->type); 
              xCurNode = xCurNode->next; 
            }

          if ((xCurNode->type == XML_ELEMENT_NODE) && (strcmp((char*) xCurNode->name, (char*) "dataTypes") == 0)) 
            { 
              log_info("Datatype definition - Skip 'datatypes'"); 
              xCurNode = xCurNode->next; 
            }

          if ((xCurNode->type == XML_ELEMENT_NODE) && (strcmp((char*) xCurNode->name, (char*) "attributes") == 0)) 
            { 
              log_info("Nested definition - Skip 'attributes'"); 
              xCurNode = xCurNode->children; 
            }
          else if ((xCurNode->type == XML_ELEMENT_NODE) && (strcmp((char*) xCurNode->name, (char*) "attribute") == 0)) 
            {
              log_info("Flat definition - No 'attributes' element"); 
            }

          // Instantiate class
          sdn::base::AnyType* ref = self->GetTypeDefinition();

          // Parse attributes if exist
          for (; ((xCurNode) && (ok)); xCurNode = xCurNode->next) 
            {
              // Only interested in XML elements at this level
              if (xCurNode->type != XML_ELEMENT_NODE) { log_debug("Discard XML node type '%d'", xCurNode->type); continue; }
              // Only interested in 'attribute' elements
              if (strcmp((char*) xCurNode->name, (char*) "attribute") != 0) { log_debug("Discard XML element node '%s'", xCurNode->name); continue; }

              // Must have 'name' and 'type' properties
              if (xmlHasProp(xCurNode, (xmlChar*) "name") == NULL) { log_warning("Discard attribute - Does not appear to have a name"); continue; }
              if (xmlHasProp(xCurNode, (xmlChar*) "dataType") == NULL) { log_warning("Discard attribute - Does not appear to have a type"); continue; }

              char attr_name [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_rank [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_type [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_mult [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              
              xmlChar* prop; 
              
              prop = xmlGetProp(xCurNode, (xmlChar*) "name");     sstrncpy((char*) attr_name, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop);

              if (xmlHasProp(xCurNode, (xmlChar*) "dataType") != NULL)     { prop = xmlGetProp(xCurNode, (xmlChar*) "dataType");     sstrncpy((char*) attr_type, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              
              if (xmlHasProp(xCurNode, (xmlChar*) "rank") != NULL)         { prop = xmlGetProp(xCurNode, (xmlChar*) "rank");         sstrncpy((char*) attr_rank, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              if (xmlHasProp(xCurNode, (xmlChar*) "multiplicity") != NULL) { prop = xmlGetProp(xCurNode, (xmlChar*) "multiplicity"); sstrncpy((char*) attr_mult, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };

              log_debug("Found attribute '%s'", attr_name);
              
              char* attr_defl = NULL;
              
              // May have a default value, i.e. XML text as child
              if ((xCurNode->children != NULL) && (xCurNode->children->type == XML_TEXT_NODE)) 
                { 
                  log_debug("Attribute '%s' is provided with a default value", attr_name);
                  attr_defl = (char*) xCurNode->children->content; char* p_buf = attr_defl; while (*p_buf != 0) { if (*p_buf == '\n') *p_buf = ' '; p_buf += 1; } /* Replace characters */
                }

              if (ref->AddAttribute(attr_rank, attr_name, attr_type, attr_mult, attr_defl) != STATUS_SUCCESS)
                {
                  log_error("SDNAnyType::AddAttribute('%s %s %s %s %s') failed", attr_rank, attr_name, attr_type, attr_mult, attr_defl); 
                  ok = false;
                }
              else if (ref->HasAttribute(attr_name) == true)
                {
                  log_debug("SDNAnyType::AddAttribute('%s %s %s %s %s') successful", attr_rank, attr_name, attr_type, attr_mult, attr_defl);
                }
              else log_warning("SDNAnyType::AddAttribute successful but AnyType::HasAttribute('%s') failed - '%d'", attr_name, ref->GetAttributeNumber());


              char attr_desc [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_qual [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              char attr_unit [STRING_MAX_LENGTH] = STRING_UNDEFINED; 
              
              if (xmlHasProp(xCurNode, (xmlChar*) "description") != NULL) { prop = xmlGetProp(xCurNode, (xmlChar*) "description"); sstrncpy((char*) attr_desc, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              if (xmlHasProp(xCurNode, (xmlChar*) "qualifier") != NULL)   { prop = xmlGetProp(xCurNode, (xmlChar*) "qualifier");   sstrncpy((char*) attr_qual, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              if (xmlHasProp(xCurNode, (xmlChar*) "unit") != NULL)        { prop = xmlGetProp(xCurNode, (xmlChar*) "unit");        sstrncpy((char*) attr_unit, (char*) prop, STRING_MAX_LENGTH); xmlFree(prop); };
              
              if (IsUndefined((char*) attr_desc) != true) log_debug("Attribute '%s' has 'description' property set to '%s'", attr_name, attr_desc);
              if (IsUndefined((char*) attr_qual) != true) log_debug("Attribute '%s' has 'qualifier' property set to '%s'", attr_name, attr_qual);
              if (IsUndefined((char*) attr_unit) != true) log_debug("Attribute '%s' has 'unit' property set to '%s'", attr_name, attr_unit);
              if (IsUndefined((char*) attr_mult) != true) log_debug("Attribute '%s' has 'multiplicity' property set to '%s'", attr_name, attr_mult);

              if (ref->AddExtAttribute(attr_rank, attr_name, attr_desc, attr_qual, attr_unit, attr_mult) != STATUS_SUCCESS)
                {
                  log_error("AnyType::AddExtAttribute('%s %s %s %s %s %s') failed", attr_rank, attr_name, attr_desc, attr_qual, attr_unit, attr_mult); 
                  ok = false;
                }
              else
                {
                  log_debug("AnyType::AddExtAttribute('%s %s %s %s %s %s') successful", attr_rank, attr_name, attr_desc, attr_qual, attr_unit, attr_mult);
                }
            }

          /* Bug 10153 - Regression - Try and compress LUTable */
          ref->Compress();

          /* Provide type definition to topic instance */
          //self->SetTypeDefinition(ref);
        }
      else
        {
          log_notice("Topic '%s' does not provide type definition", self->GetName());
        }
    }

  /* Shutdown libxml */
  xmlFreeDoc(xDoc);
  ReleaseParser();
  xmlMemoryDump();

  if (ok)
    {
      status = STATUS_SUCCESS;
    }

  uint_t hash = 0;
#if 0 // Must be backward compatible with older versions
  if(self->GetTypeDefinition() != NULL)
    {
      char string [1024] = STRING_UNDEFINED;
      const ccs::types::CompoundType *type = new (std::nothrow) ccs::types::CompoundType(*(self->GetTypeDefinition()));
      if(ccs::HelperTools::Serialise((const std::shared_ptr<const ccs::types::CompoundType>)(type), string, 1024u) == true)
        {
          hash = ccs::HelperTools::Hash<ccs::types::uint16>(string);
        }
    }
#endif
  if ((self->GetUID() == 0) && (hash != 0))
    {
      self->SetUID(hash);
      log_notice("Topic '%s' uid inferred from datatype definition '%u'", self->GetName(), self->GetUID());
    }

  if ((self->GetUID() != 0) && (self->GetUID() != hash))
    {
      log_warning("Topic '%s' uid mismatch between metadata '%u' and datatype definition '%u'", self->GetName(), self->GetUID(), hash);
    }

  uint_t size = ((self->GetTypeDefinition() == NULL) ? 0 : (self->GetTypeDefinition())->GetSize());

  if ((self->GetSize() == 0) && (size != 0))
    {
      self->SetSize(size);
      log_notice("Topic '%s' size inferred from datatype definition '%u'", self->GetName(), self->GetSize());
    }

  if ((self->GetSize() != 0) && (size != 0) && (self->GetSize() != size))
    {
      log_warning("Topic '%s' size mismatch between metadata '%u' and datatype definition '%u'", self->GetName(), self->GetSize(), size);
      self->SetSize(size);
      log_notice("Topic '%s' size inferred from datatype definition '%u'", self->GetName(), self->GetSize());
    }

  /* Bug 9249 - Natural payload size limit for SDN over UDP/IPv4 */
  if (self->GetSize() > MAX_TOPIC_SIZE)
    {
      log_warning("Topic '%s' size '%u' is invalid - Payload limited to '%u'", self->GetName(), self->GetSize(), MAX_TOPIC_SIZE); 
    }

  log_info("Loaded topic definition - '%s [%s:%d]' of size '%u' and version '%u'", self->GetName(), self->GetMCastGroup(), self->GetMCastPort(), self->GetSize(), self->GetVersion());

  {
    char buffer [2048]; self->SerializeType(buffer, 2048);
    log_debug("Loaded topic definition '%s'", buffer);
  }

  log_trace("TopicHelper::ParseXMLContent - Leaving method");

  return status;

}

/**
 * @brief Defines topic from XML definition file.
 * @detail This method parses a XML definition file and uses the sdn::Topic reference
 * and API to configure it.
 * @return STATUS_SUCCESS if the topic is configured, STATUS_ERROR otherwise.
 * @todo Fix re-entrancy issue with libxml2 parser.
 */

static inline RET_STATUS LoadDefinitionFile (Topic* self, const char* file_path)
{

  log_trace("TopicHelper::LoadDefinitionFile('%s') - Entering method", file_path);

  RET_STATUS status = STATUS_ERROR;

  /* Check file existence */
  if (exist(file_path) != true)
    {
      log_error("Unable to locate topic definition file '%s'", file_path);
      return status;
    }

  std::ifstream file (file_path);
  std::string content ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  status = ParseXMLContent(self, content.c_str());
  
  log_trace("TopicHelper::LoadDefinitionFile('%s') - Leaving method", file_path);

  return status;

}

/**
 * @brief Instantiates a topic by name, using XML topic definition file.
 * @detail This method locates a XML topic definition file for the 
 * given name, parses it to instantiate and configure a sdn::Topic instance.
 * @param name Topic name, e.g. "MAG-PFCS:STATUS" (SDN variable as declared in Self-Description Data model).
 * @return sdn::Topic* if the topic is instantiated and configured, NULL otherwise.
 * @code 
   #include "sdn-topic.h"
     
   sdn::Topic* topic = sdn::TopicHelper::InstantiateFromDefinitionFile("MAG-PFCS:STATUS");

   bool ok = (static_cast<sdn::Topic*>(NULL) != topic);
   @endcode
 */

static inline Topic* InstantiateFromDefinitionFile (const char* name) // Name, size, destination, address, etc. is all part of the XML file
{ 

  log_trace("TopicHelper::InstantiateFromDefinitionFile('%s') - Entering method", name);

  char file_path [PATH_MAX_LENGTH] = STRING_UNDEFINED;

  Topic* ref = static_cast<Topic*>(NULL);

  /* Search for type definition file at several locations ... */
  if (LocateDefinitionFile(name, (char*) file_path, PATH_MAX_LENGTH) == STATUS_SUCCESS)
    {
      /* Instantiate class */
      ref = new (std::nothrow) Topic;

      /* Try and load datatype definition file */
      if (LoadDefinitionFile(ref, file_path) != STATUS_SUCCESS) 
        {
          delete ref;
          ref = static_cast<Topic*>(NULL);
        }
      else
        {
          /* Finalize configuration */
          ref->Configure();
        }
    }
  else
    {
      log_error("Unable to locate type definition file for '%s'", name);
    }

  log_trace("TopicHelper::InstantiateFromDefinitionFile('%s') - Leaving method", name);

  return ref;

}

/**
 * @brief Instantiates a topic by parsing XML content.
 * @detail This method is passed a null-terminated character array providing
 * topic definition, parses it to instantiate and configure a
 * sdn::Topic instance.
 * @param content Nil-terminated char array with XML content.
 * @return sdn::Topic* if the topic is instantiated and configured, NULL otherwise.
 * @code 
   #include "sdn-topic.h"
     
   char type_def [] = 
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<topic name=\"MyTopicName\" description=\"Topic definition file (see https://bugzilla.iter.org/codac/show_bug.cgi?id=10852)\" version=\"1\" uid=\"0\">"
    "  <attributes>"
    "    <attribute name=\"reserved\" dataType=\"uint8\" multiplicity=\"1024\"/>"
    "  </attributes>"
    "</topic>";

   sdn::Topic* topic = sdn::TopicHelper::InstantiateFromXMLContent(type_def);

   bool ok = (static_cast<sdn::Topic*>(NULL) != topic);
   @endcode
 */

static inline Topic* InstantiateFromXMLContent (const char* content)
{ 

        log_trace("TopicHelper::InstantiateFromXMLContent - Entering method");

        Topic* ref = static_cast<Topic*>(NULL);

        // Instantiate class
        ref = new (std::nothrow) Topic;

        bool ok = (static_cast<Topic*>(NULL) != ref);

        // Try and parse content
        if(ok) ok = (ParseXMLContent(ref, content) == STATUS_SUCCESS);

        if(ok) ok = (ref->Configure() == STATUS_SUCCESS);

        if(!ok)
        {
                if(ref != NULL) delete ref;
                ref = static_cast<Topic*>(NULL);
        }

        log_trace("TopicHelper::InstantiateFromXMLContent - Leaving method");

        return ref;

}

} /* namespace TopicHelper */

} /* namespace sdn */

#endif /* _TopicXML_h */
