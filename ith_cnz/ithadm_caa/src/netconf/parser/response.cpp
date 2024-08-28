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

#include "netconf/parser/response.h"
#include "netconf/netconf.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"


#include <libxml/parser.h>

#include <memory.h>

namespace netconf
{
	namespace parser
	{
		bool Response::m_libxml_initialized = false;

		void Response::initialize()
		{
			if(!m_libxml_initialized)
			{
				// initialize the library and check potential ABI mismatches
				LIBXML_TEST_VERSION

				m_libxml_initialized = true;
			}
		}

		void Response::finalize()
		{
			if(m_libxml_initialized)
			{
				//clean up resources of xml library
				xmlCleanupParser();
				m_libxml_initialized = false;
			}
		}

		Response::Response()
		: m_managedObjects()
		{

		}

		bool Response::parse(const std::string& xmlResponse)
		{
			bool result = true;

			m_managedObjects.clear();

			if( xmlResponse.empty() )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Empty xml doc nothing to parser!");
				return result;
			}

			// See http://xmlsoft.org/html/libxml-tree.html#xmlSAXHandler
			xmlSAXHandler saxHandler;

			memset( &saxHandler, 0, sizeof(saxHandler) );

			saxHandler.initialized = XML_SAX2_MAGIC;  // so we do this to force parsing as SAX2.

			// set callback functions
			saxHandler.startElement = &(Response::OnManagedObjectStart);
			saxHandler.characters = &(Response::OnAttributeValue);
			saxHandler.endElement =  &(Response::OnManagedObjectEnd);

			saxHandler.warning = &(Response::OnLibraryWarning);
			saxHandler.error = &(Response::OnLibraryError);

			// parsing the xml document
			int parserResult = xmlSAXUserParseMemory( &saxHandler, &m_managedObjects, xmlResponse.c_str(), static_cast<int>(xmlResponse.length()) );

			if ( parserResult != 0 )
			{
				result = false;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to parser doc:<%s>, error:<%d>", xmlResponse.c_str(), parserResult);
			}

			return result;
		}

		bool Response::getAttributeOf(const std::string& ManagedObjectName, const Collector::ManagedObjectKey_t& key, const std::string& attributeName, std::string& attributeValue)
		{
			return m_managedObjects.getAttribute(ManagedObjectName, key, attributeName, attributeValue);
		}

		bool Response::getAllAttributeOf(const std::string& ManagedObjectName, const Collector::ManagedObjectKey_t& key, std::map<std::string, std::string>& attributes)
		{
			return m_managedObjects.getAttributes(ManagedObjectName, key, attributes);
		}

		bool Response::getAllAttributeOf(const std::string& ManagedObjectName, const configdataElement& key, configdataList& attributes)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Element:<%s>, Key:<%s=%s>", ManagedObjectName.c_str(), key.first.c_str(), key.second.c_str() );
			return m_managedObjects.getAttributes(ManagedObjectName, key, attributes);
		}

		void Response::OnManagedObjectStart(void* userData, const xmlChar* name, const xmlChar** attrs)
		{
			// tag properties
			UNUSED(attrs);

			Collector* managedObjects = reinterpret_cast<Collector*>(userData);

			// tag name founded
			const char* elementName = reinterpret_cast<const char*>(name);

			if( isaManagedObject(elementName) )
			{
				// Managed object name, add a new element
				managedObjects->addManagedObject(elementName);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG," Found Element :<%s>", elementName);
			}
			else
			{
				// Attribute name add it to the current managed object
				managedObjects->setAttributeName(elementName);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG," Found Attribute :<%s>", elementName);
			}
		}

		void Response::OnAttributeValue(void* userData, const xmlChar* value, int length)
		{
			Collector* managedObjects = reinterpret_cast<Collector*>(userData);

			char AttributeValue[length + 1];
			strncpy(AttributeValue, reinterpret_cast<const char*>(value), length);
			AttributeValue[length] = 0;

			managedObjects->setAttributeValue(AttributeValue);
		}

		void Response::OnManagedObjectEnd(void* userData, const xmlChar* name)
		{
			Collector* managedObjects = reinterpret_cast<Collector*>(userData);

			// tag name founded
			const char* elementName = reinterpret_cast<const char*>(name);

			if( isaManagedObject(elementName) )
			{
				// Managed object end, move to the previous element
				managedObjects->backToParent();
			}
			else
			{
				// add the pair:<attributeName, attributeValue> to the current managed object
				managedObjects->addAttribute();
			}
		}

		void Response::OnLibraryError(void* userData, const char* format, ... )
		{
			va_list ap;
			va_start(ap, format);

			UNUSED(userData);

			char log_buffer[8 * 1024];
			size_t written = ::snprintf(log_buffer, FIXS_ITH_ARRAY_SIZE(log_buffer), "XML2_LIBRARY_ERROR: ");
			::vsnprintf((log_buffer + written), (FIXS_ITH_ARRAY_SIZE(log_buffer) - written), format, ap);

			FIXS_ITH_LOG(LOG_LEVEL_ERROR, log_buffer);

			va_end(ap);
		}

		void Response::OnLibraryWarning(void* userData, const char* format, ... )
		{
			va_list ap;
			va_start(ap, format);

			UNUSED(userData);

			char log_buffer[8 * 1024];
			size_t written = ::snprintf(log_buffer, FIXS_ITH_ARRAY_SIZE(log_buffer), "XML2_LIBRARY_WARNING: ");
			::vsnprintf((log_buffer + written), (FIXS_ITH_ARRAY_SIZE(log_buffer) - written), format, ap);

			FIXS_ITH_LOG(LOG_LEVEL_WARN, log_buffer);

			va_end(ap);
		}

		bool Response::isaManagedObject(const char* elementName)
		{
			bool isMO = ( ( strcmp(elementName, managedElement::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, ers::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, ersTransport::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, ip::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, virtualRouter::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, ipInterface::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, router::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, interfaceIpv4::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, addressIpv4::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, bfdSessionInfo::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, bfdProfile::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, routeTable::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, routeTableStatic::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, dst::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, nextHop::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, vrrpv3IPv4Session::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, protVirtIPv4Address::CLASS_NAME) == 0 ) ||
						  ( strcmp(elementName, peerIpv4::CLASS_NAME) == 0 )

						);

			return isMO;
		}

	} /* namespace parser */
} /* namespace netconf */
