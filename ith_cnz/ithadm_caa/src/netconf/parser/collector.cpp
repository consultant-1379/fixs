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

#include "netconf/parser/collector.h"
#include "netconf/parser/managedobject.h"

namespace netconf
{
	namespace parser
	{

		Collector::Collector()
		: m_root(0),
		  m_currentMO(0)
		{


		}

		Collector::~Collector()
		{
			delete m_root;
		}

		bool Collector::addManagedObject(const char* name)
		{
			bool result = false;
			if(m_root)
			{
				ManagedObject* current = m_currentMO->addChildren(name);

				if(current)
				{
					result = true;
					m_currentMO = current;
				}
			}
			else
			{
				result = addRootMo(name);
			}

			return result;

		}

		void Collector::backToParent()
		{
			ManagedObject* current = m_currentMO->getParent();

			if(current)
			{
				m_currentMO = current;
			}
		}

		bool Collector::setAttributeName(const char* name)
		{
			bool result = false;

			if(m_currentMO)
			{
				result = true;
				m_currentMO->setCurrentAttributeName(name);
			}

			return result;
		}

		bool Collector::setAttributeValue(const char* value)
		{
			bool result = false;

			if(m_currentMO)
			{
				result = true;
				m_currentMO->setCurrentAttributeValue(value);
			}

			return result;
		}

		void Collector::addAttribute()
		{
			if(m_currentMO)
			{
				m_currentMO->addCurrentAttribute();
			}
		}

		bool Collector::getAttribute(const std::string& name, const ManagedObjectKey_t& key, const std::string& attributeName, std::string& attributeValue)
		{
			bool result = false;

			ManagedObject* element = search(name, key);

			if(element)
			{
				result = element->getAttribute(attributeName, attributeValue);
			}

			return result;

		}

		bool Collector::getAttribute(const std::string& name, const std::string& attributeName, std::string& attributeValue)
		{
			bool result = false;

			ManagedObject* start = m_root;

			if( findFirstOf(start, name))
			{
				result = start->getAttribute(attributeName, attributeValue);
			}

			return result;
		}

		bool Collector::getAttributes(const std::string& ManagedObjectName, const Collector::ManagedObjectKey_t& key, ManagedObjectAttributes_t& attributes)
		{
			bool result = false;

			ManagedObject* element = search(ManagedObjectName, key);

			if(element)
			{
				result = true;
				attributes = element->getAttributes();
			}

			return result;
		}

		bool Collector::getAttributes(const std::string& ManagedObjectName, const ManagedObjectKey_t& key, std::vector<ManagedObjectKey_t>& attributes)
		{
			ManagedObjectAttributes_t moAttributes;
			bool result = getAttributes(ManagedObjectName, key, moAttributes);

			attributes.clear();

			ManagedObjectAttributes_t::const_iterator attributeIterator = moAttributes.begin();

			for(; moAttributes.end() != attributeIterator; ++attributeIterator)
			{
				attributes.push_back(*attributeIterator);
			}

			return result;
		}

		void Collector::clear()
		{
			if(m_root) delete m_root;

			m_root = m_currentMO = 0;
		}

		void Collector::print()
		{
			if(m_root) m_root->print();
		}

		bool Collector::addRootMo(const char* name)
		{
			bool result = false;

			m_root = new (std::nothrow) ManagedObject(name, NULL);

			if(m_root)
			{
				result = true;
				m_currentMO = m_root;
			}

			return result;
		}

		bool Collector::findFirstOf(ManagedObject*& start, const std::string& name)
		{
			bool found = false;

			if(0 == start) return false;

			if(name.compare(start->getName()) == 0)
			{
				found = true;
				return found;
			}

			ManagedObject::Children_t childrenMO = start->getChildren();
			ManagedObject::Children_t::const_iterator childrenIterator;

			for(childrenIterator = childrenMO.begin(); !found && childrenIterator != childrenMO.end(); ++childrenIterator )
			{
				start = *childrenIterator;
				found = findFirstOf(start, name);
			}

			return found;
		}

		void Collector::findAllOf(ManagedObject*& start, const std::string& name, std::list<ManagedObject*>& elements)
		{
			if(start)
			{

				if(name.compare(start->getName()) == 0)
				{
					elements.push_back(start);
				}
				else
				{
					ManagedObject::Children_t childrenMO = start->getChildren();
					ManagedObject::Children_t::const_iterator childrenIterator;

					for(childrenIterator = childrenMO.begin(); childrenIterator != childrenMO.end(); ++childrenIterator )
					{
						start = *childrenIterator;
						findAllOf(start, name, elements);
					}
				}
			}
		}

		ManagedObject* Collector::search(const std::string& ManagedObjectName, const ManagedObjectKey_t& key)
		{
			ManagedObject* found = NULL;

			ManagedObject* start = m_root;

			ManagedObject::Children_t elements;

			// get all MOs of same kind
			findAllOf(start, ManagedObjectName, elements);

			ManagedObject::Children_t::const_iterator moIterator;

			// search the specified object
			for(moIterator = elements.begin(); moIterator != elements.end(); ++moIterator)
			{
				std::string keyValue;
				if( (*moIterator)->getAttribute(key.first, keyValue) && (keyValue.compare(key.second) == 0 ))
				{
					found = (*moIterator);
					break;
				}
			}

			return found;
		}

	} /* namespace parser */
} /* namespace netconf */
