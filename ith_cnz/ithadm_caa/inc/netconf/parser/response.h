/*
 * 
 * COPYRIGHT Ericsson 2016
 *	All rights reserved.
 *
 *	The Copyright to the computer program(s) herein
 *	is the property of Ericsson 2016.
 *	The program(s) may be used and/or copied only with
 *	the written permission from Ericsson 2016 or in
 *	accordance with the terms and conditions stipulated in
 *	the agreement/contract under which the program(s) have
 *	been supplied.
 *
 *	
 *  Created on: May 12, 2016
 *      Author: xvincon
 */

#ifndef NETCONF_PARSER_RESPONSEBUILDER_H_
#define NETCONF_PARSER_RESPONSEBUILDER_H_

#include "netconf/netconf.h"
#include "netconf/parser/collector.h"

#include <libxml/xmlmemory.h>

#include <map>
#include <string>

namespace netconf
{
	namespace parser
	{

		class Response
		{
			public:

				Response();

				virtual ~Response() {};

				bool parse(const std::string& xmlResponse);

				// Get the value of a specific attribute of a specific managed object
				bool getAttributeOf(const std::string& ManagedObjectName, const Collector::ManagedObjectKey_t& key, const std::string& attributeName, std::string& attributeValue);

				// Get all attributes as map of <name, value> of a specific managed object
				bool getAllAttributeOf(const std::string& ManagedObjectName, const Collector::ManagedObjectKey_t& key, std::map<std::string, std::string>& attributes);

				// Get all attributes as list of pair <name, value> of a specific managed object
				bool getAllAttributeOf(const std::string& ManagedObjectName, const configdataElement& key, configdataList& attributes);

				// initialize the xml2 library
				static void initialize();

				// free all resources of xml2 library
				static void finalize();

			private:

				//
				// callbacks functions for the Libxml2 SAX handler
				//
				static void OnManagedObjectStart(void* userData, const xmlChar* name, const xmlChar** attrs);

				static void OnAttributeValue(void* userData, const xmlChar* value, int length);

				static void OnManagedObjectEnd(void* userData, const xmlChar* name);

				static void OnLibraryError(void* userData, const char* format, ... );

				static void OnLibraryWarning(void* userData, const char* format, ... );

				static bool isaManagedObject(const char* elementName);

				Collector m_managedObjects;

				static bool m_libxml_initialized;

		};

	} /* namespace parser */
} /* namespace netconf */

#endif /* NETCONF_PARSER_RESPONSEBUILDER_H_ */
