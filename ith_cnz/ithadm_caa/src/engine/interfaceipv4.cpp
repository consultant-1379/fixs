/*
 * interfaceipv4.cpp
 *
 *  Created on: May 9, 2016
 *      Author: estevol
 */

#include "engine/interfaceipv4.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

	InterfaceIPv4::InterfaceIPv4(const operation::interfaceInfo& data):
		m_data(data), m_routerName(common::utility::getRouterNameFromInterfaceDN(data.getDN())), m_encapsulation()
	{

	}

	InterfaceIPv4::InterfaceIPv4(const boost::shared_ptr<InterfaceIPv4>& rhs):
		m_data(rhs->m_data), m_routerName(rhs->m_routerName), m_encapsulation(rhs->m_encapsulation)
	{

	}

	InterfaceIPv4::~InterfaceIPv4()
	{

	}

	void InterfaceIPv4::setEncapsulation(const boost::shared_ptr<Vlan>& encapsulation)
	{
		if (encapsulation != m_encapsulation)
		{
			m_encapsulation = encapsulation;
			m_data.encapsulation.assign(encapsulation->getDN());
			m_data.encapsulationVid = encapsulation->getVlanId();
		}
	}

	uint16_t InterfaceIPv4::modify(const operation::interfaceInfo& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::interfaceIpv4_attribute::MTU_CHANGE) && (m_data.getMTU() != data.getMTU()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] InterfaceIPv4: <%s>, modifying mtu\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getMTU(), data.getMTU());

			m_data.mtu = data.getMTU();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::interfaceIpv4_attribute::ADMIN_STATE_CHANGE) && (m_data.getAdmState() != data.getAdmState()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] InterfaceIPv4: <%s>, modifying admin state\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getAdmState(), data.getAdmState());

			m_data.adminstate = data.getAdmState();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE) && (m_data.bfdProfile.compare(data.getBfdProfile()) != 0) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] InterfaceIPv4: <%s>, modifying BFD Profile\n"
					"old value: <%s>, new value: <%s>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getBfdProfile(), data.getBfdProfile());

			m_data.bfdProfile = data.getBfdProfile();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE) && (m_data.aclIngress.compare(data.getAclIngress()) != 0) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] InterfaceIPv4: <%s>, modifying ACL Ingress\n"
					"old value: <%s>, new value: <%s>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getAclIngress(), data.getAclIngress());

			m_data.aclIngress = data.getAclIngress();
			setAclInfo();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES_CHANGE) && (m_data.getBfdStaticRoute() != data.getBfdStaticRoute()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] InterfaceIPv4: <%s>, modifying BFD Static Route\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getBfdStaticRoute(), data.getBfdStaticRoute());

			m_data.bfdStaticRoutes = data.getBfdStaticRoute();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE) && (m_data.encapsulation.compare(data.getEncapsulation()) != 0) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] InterfaceIPv4: <%s>, modifying Encapsulation\n"
					"old value: <%s>, new value: <%s>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getEncapsulation(), data.getEncapsulation());

			m_data.encapsulation = data.getEncapsulation();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::interfaceIpv4_attribute::LOOPBACK_CHANGE) && (m_data.getLoopback() != data.getLoopback()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] InterfaceIPv4: <%s>, modifying loopback\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getLoopback(), data.getLoopback());

			m_data.loopback = data.getLoopback();
			++modifiedAttributes;
		}

		m_data.changeMask |= data.changeMask;

		return modifiedAttributes;
	}

	void InterfaceIPv4::setRuntimeInfo(const operation::interfaceInfo& runtimeInfo)
	{
		if (runtimeInfo.changeMask & imm::interfaceIpv4_attribute::OPER_STATE_CHANGE)
		{
			m_data.operationalState = runtimeInfo.getOperationalState();
		}
	}

	bool InterfaceIPv4::isBfdEnabled()
	{
		return (imm::ENABLED == m_data.bfdStaticRoutes);
	}

	bool InterfaceIPv4::isAclDefined() const
	{
		return !m_data.isAclIngressEmpty();
	}

	void InterfaceIPv4::setAclInfo()
	{
		m_data.aclData.setRouterName(common::utility::getRouterIdFromRouterName(m_routerName));

		m_data.aclData.aclName.clear();
		if( isAclDefined() )
		{
			// ACL name
			m_data.aclData.setAclName(common::utility::getIdValueFromRdn(m_data.getAclIngress()));
		}

		if(m_encapsulation)
		{
			m_data.aclData.setVlanId(m_encapsulation->getVlanId());
		}

	}


} /* namespace engine */
