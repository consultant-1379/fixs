/*
 * address.cpp
 *
 *  Created on: May 17, 2016
 *      Author: estevol
 */

#include "engine/address.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

	Address::Address(const operation::addressInfo& data):
			m_data(data),
			m_routerName(),
			m_interfaceName(common::utility::getInterfaceNameFromAddressDN(data.getDN()))
	{
		if(!isVRRPInterfaceAddress())
		{
			m_routerName.assign(common::utility::getRouterNameFromAddressDN(data.getDN()));
		}

	}

	Address::Address(const boost::shared_ptr<Address>& rhs):
			m_data(rhs->m_data),
			m_routerName(rhs->m_routerName),
			m_interfaceName(rhs->m_interfaceName)
	{

	}

	Address::~Address()
	{

	}

	uint16_t Address::modify(const operation::addressInfo& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::addressIpv4_attribute::ADDRESS_CHANGE) && (m_data.address.compare(data.getAddress()) != 0) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s][%s] Address: <%s>, modifying address\n"
					"old value: <%s>, new value: <%s>", data.getSmxId(), m_routerName.c_str(), m_interfaceName.c_str(), data.getName(),
					m_data.getAddress(), data.getAddress());

			m_data.address.assign(data.getAddress());
			++modifiedAttributes;
		}

		m_data.changeMask |= data.changeMask;

		return modifiedAttributes;
	}


} /* namespace engine */
