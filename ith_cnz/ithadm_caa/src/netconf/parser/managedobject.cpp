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

#include "netconf/parser/managedobject.h"

#include <iostream>

namespace netconf
{
	namespace parser
	{

		ManagedObject::ManagedObject(const char* name, ManagedObject* parent)
		: m_name(name),
		  m_parent(parent),
		  m_currentAttribute(),
		  m_attributes(),
		  m_children()
		{

		}

		ManagedObject::~ManagedObject()
		{
			Children_t::const_iterator childrenIterator;

			for(childrenIterator = m_children.begin(); childrenIterator != m_children.end(); ++childrenIterator )
			{
				delete (*childrenIterator);
			}

			m_children.clear();
			m_attributes.clear();
		}

		ManagedObject* ManagedObject::addChildren(const char* name)
		{
			ManagedObject* newMO = new (std::nothrow) ManagedObject(name, this);

			if(newMO)
			{
				m_children.push_back(newMO);
			}

			return newMO;
		}

		void ManagedObject::setCurrentAttributeName(const char* name)
		{
			m_currentAttribute.first.assign(name);
		}

		void ManagedObject::setCurrentAttributeValue(const char* value)
		{
			m_currentAttribute.second.assign(value);
		}

		void ManagedObject::addCurrentAttribute()
		{
			m_attributes.insert(m_currentAttribute);
			m_currentAttribute.first.clear();
			m_currentAttribute.second.clear();
		}

		bool ManagedObject::getAttribute(const std::string& attributeName, std::string& attributeValue)
		{
			bool result = false;
			Attributes_t::const_iterator attributeIterator = m_attributes.find(attributeName);

			if(m_attributes.end() != attributeIterator)
			{
				result = true;
				attributeValue.assign(attributeIterator->second);
			}

			return result;
		}

		void ManagedObject::print()
		{
			std::cout << "\n MO : " << m_name << std::endl;
			Attributes_t::const_iterator attributeIterator;

			for(attributeIterator = m_attributes.begin(); attributeIterator != m_attributes.end(); ++attributeIterator)
			{
				std::cout << "\n\t" << attributeIterator->first << " = "<< attributeIterator->second << std::endl;
			}

			Children_t::const_iterator childrenIterator;

			for(childrenIterator = m_children.begin(); childrenIterator != m_children.end(); ++childrenIterator )
			{
				(*childrenIterator)->print();
			}
		}

	} /* namespace parser */
} /* namespace netconf */
