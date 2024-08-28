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
 *  Created on: Jun 15, 2016
 *      Author: xvincon
 */

#include "engine/vrrpsession.h"
#include "common/utility.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

namespace engine
{

	VrrpSession::VrrpSession(const operation::vrrpSessionInfo& data)
	: m_data(data),
	  m_routerName( common::utility::getRouterNameFromVrrpSessionDN( data.getDN() ) ),
	  m_interfaceName( common::utility::getInterfaceNameFromVrrpSessionDN( data.getDN() ) ),
	  m_vrrpInterfaceName(common::utility::getVrrpInstanceNameFromDN(data.getVrrpInterfaceRef()))
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpSession created : MOC:<%s>", m_data.getDN());
	}

	VrrpSession::VrrpSession(const boost::shared_ptr<VrrpSession>& rhs)
	: m_data(rhs->m_data),
	  m_routerName(rhs->m_routerName),
	  m_interfaceName(rhs->m_interfaceName),
	  m_vrrpInterfaceName(rhs->m_vrrpInterfaceName)
	{

	}

	VrrpSession::~VrrpSession()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpSession delete : MOC:<%s>", m_data.getDN());
	}

	uint16_t VrrpSession::modify(const operation::vrrpSessionInfo& newData)
	{
		uint16_t modifiedAttributes = 0;

		if( (newData.changeMask & imm::vrrpSession_attribute::ADMIN_STATE_CHANGE)
				&& (m_data.getAdminState() != newData.getAdminState() ) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] VrrpSession: <%s>, modifying Admin State\n"
					"old value: <%d>, new value: <%d>", newData.getSmxId(), m_routerName.c_str(), newData.getName(),
					m_data.getAdminState(), newData.getAdminState());

			m_data.administrativeState = newData.getAdminState();
			m_data.changeMask |= imm::vrrpSession_attribute::ADMIN_STATE_CHANGE;
			++modifiedAttributes;
		}

		return modifiedAttributes;
	}

	void VrrpSession::setVrrpInterfaceInfo(const operation::vrrpInterfaceInfo& data)
	{
		m_data.setVrrpInterfaceInfo(data);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VRRP Interface parameter set on VrrpSession:<%s>" , m_data.getDN());
	}

	void VrrpSession::resetChangeMask()
	{
		m_data.changeMask = 0U;
		m_data.interfaceInfo.changeMask = 0U;
	}

} /* namespace engine */
