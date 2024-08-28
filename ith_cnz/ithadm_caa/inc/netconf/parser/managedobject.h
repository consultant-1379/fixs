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

#ifndef NETCONF_PARSER_MANAGEDOBJECT_H_
#define NETCONF_PARSER_MANAGEDOBJECT_H_

#include <string>
#include <map>
#include <list>

#include <boost/noncopyable.hpp>


namespace netconf
{
	namespace parser
	{
		class Collector;

		class ManagedObject : private boost::noncopyable
		{
			private:

				typedef std::list<ManagedObject*> Children_t;
				typedef std::map<std::string, std::string> Attributes_t;

				friend class Collector;

				ManagedObject(const char* name, ManagedObject* parent);

				virtual ~ManagedObject();

				ManagedObject* addChildren(const char* name);

				void setCurrentAttributeName(const char* name);

				void setCurrentAttributeValue(const char* value);

				void addCurrentAttribute();

				bool getAttribute(const std::string& attributeName, std::string& attributeValue);

				Attributes_t& getAttributes() { return m_attributes; };

				ManagedObject* getParent() const { return m_parent; };

				std::string getName() const { return m_name; };

				Children_t& getChildren() { return m_children; };

				void print();

			private:

				std::string m_name;

				ManagedObject* m_parent;

				typedef std::pair<std::string, std::string> Attribute_t;
				Attribute_t m_currentAttribute;


				Attributes_t m_attributes;

				Children_t m_children;

		};

	} /* namespace parser */
} /* namespace netconf */

#endif /* NETCONF_PARSER_MANAGEDOBJECT_H_ */
