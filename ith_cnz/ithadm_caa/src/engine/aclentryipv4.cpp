/*
 * address.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: xclaele
 */

#include "engine/aclentryipv4.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

	AclEntryIpv4::AclEntryIpv4(const operation::aclEntryInfo& data):
			m_data(data),
			m_routerName(common::utility::getRouterNameFromChildDN(data.getDN())),
			m_aclIpv4Name(common::utility::getAclIpv4NameFromChildDN(data.getDN()))
	{

	}

	AclEntryIpv4::AclEntryIpv4(const boost::shared_ptr<AclEntryIpv4>& rhs):
			m_data(rhs->m_data),
			m_routerName(rhs->m_routerName),
			m_aclIpv4Name(rhs->m_aclIpv4Name)
	{

	}

	AclEntryIpv4::~AclEntryIpv4()
	{

	}

	uint16_t AclEntryIpv4::modify(const operation::aclEntryInfo& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::aclEntry_attribute::ACTION_CHANGE) && (m_data.getAclAction() != data.getAclAction()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying action --> old value: <%d>, new value: <%d>",
					data.getName(), m_data.getAclAction(), data.getAclAction());

			m_data.action = data.getAclAction();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::ADDR_DST_CHANGE) && (m_data.getAddrDst() != data.getAddrDst()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying addrDst --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getAddrDst().size(), data.getAddrDst().size());

			m_data.addrDst = data.getAddrDst();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::ADDR_SRC_CHANGE) && (m_data.getAddrSrc() != data.getAddrSrc()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying addrSrc --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getAddrSrc().size(), data.getAddrSrc().size());

			m_data.addrSrc = data.getAddrSrc();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::DSCP_CHANGE) && (m_data.getDscp() != data.getDscp()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying dscp --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getDscp().size(), data.getDscp().size());

			m_data.dscp = data.getDscp();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::FRAGMENT_TYPE_CHANGE) && (m_data.getFragmentType() != data.getFragmentType()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying fragmentType --> old value: <%d>, new value: <%d>",
					data.getName(), m_data.getFragmentType(), data.getFragmentType());

			m_data.fragmentType = data.getFragmentType();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::ICMP_TYPE_CHANGE) && (m_data.getIcmpType() != data.getIcmpType()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying icmpType --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getIcmpType().size(), data.getIcmpType().size());

			m_data.icmpType = data.getIcmpType();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::IP_PROTOCOL_CHANGE) && (m_data.getIpProtocol() != data.getIpProtocol()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying ipProtocol --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getIpProtocol().size(), data.getIpProtocol().size());

			m_data.ipProtocol = data.getIpProtocol();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::PORT_DST_CHANGE) && (m_data.getPortDst() != data.getPortDst()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying portDst --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getPortDst().size(), data.getPortDst().size());

			m_data.portDst = data.getPortDst();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::PORT_SRC_CHANGE) && (m_data.getPortSrc() != data.getPortSrc()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying portSrc --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getPortSrc().size(), data.getPortSrc().size());

			m_data.portSrc = data.getPortSrc();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::aclEntry_attribute::PRIORITY_CHANGE) && (m_data.getPriority() != data.getPriority()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry: <%s>, modifying priority --> old value: <%d>, new value: <%d>",
					data.getName(), m_data.getPriority(), data.getPriority());

			m_data.priority = data.getPriority();
			++modifiedAttributes;
		}

		m_data.changeMask |= data.changeMask;

		return modifiedAttributes;
	}


} /* namespace engine */
