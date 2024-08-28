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
 *  Created on: May 11, 2016
 *      Author: xvincon
 */

#ifndef NETCONF_PARSER_COLLECTOR_H_
#define NETCONF_PARSER_COLLECTOR_H_

#include <string>
#include <utility>
#include <list>
#include <vector>
#include <map>

#include <boost/noncopyable.hpp>

namespace netconf
{
	namespace parser
	{
		class ManagedObject;

		class Collector : private boost::noncopyable
		{
			public:

				Collector();

				virtual ~Collector();

				bool addManagedObject(const char* name);

				void backToParent();

				bool setAttributeName(const char* name);

				bool setAttributeValue(const char* value);

				void addAttribute();

				typedef std::pair<std::string, std::string> ManagedObjectKey_t;
				typedef std::map<std::string, std::string> ManagedObjectAttributes_t;

				// Get the value of a specific attribute of a specific managed object
				bool getAttribute(const std::string& name, const ManagedObjectKey_t& key, const std::string& attributeName, std::string& attributeValue);

				bool getAttribute(const std::string& name, const std::string& attributeName, std::string& attributeValue);

				bool getAttributes(const std::string& ManagedObjectName, const ManagedObjectKey_t& key, ManagedObjectAttributes_t& attributes);

				bool getAttributes(const std::string& ManagedObjectName, const ManagedObjectKey_t& key, std::vector<ManagedObjectKey_t>& attributes);

				void clear();

				void print();

			private:

				bool addRootMo(const char* name);

				bool findFirstOf(ManagedObject*& start, const std::string& name);

				void findAllOf(ManagedObject*& start, const std::string& name, std::list<ManagedObject*>& elements);

				ManagedObject* search(const std::string& ManagedObjectName, const ManagedObjectKey_t& key);

				ManagedObject* m_root;

				ManagedObject* m_currentMO;

		};

	} /* namespace parser */
} /* namespace netconf */

#endif /* NETCONF_PARSER_COLLECTOR_H_ */
