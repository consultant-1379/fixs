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
 *  Created on: Oct 21, 2016
 *      Author: xvincon
 */

#include "engine/alcipv4.h"
#include "common/utility.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

namespace engine
{

	AclIpv4::AclIpv4(const operation::aclIpV4Info& data)
	: m_data(data),
	  m_routerName(common::utility::getRouterNameFromChildDN(data.getDN()))
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"ACL IPv4 object created, DN:<%s>", m_data.getDN() );
	}

	AclIpv4::~AclIpv4()
	{

	}

	AclIpv4::AclIpv4(const boost::shared_ptr<AclIpv4>& rhs)
	: m_data(rhs->m_data),
	  m_routerName(rhs->m_routerName)
	{

	}

	uint16_t AclIpv4::modify(const operation::aclIpV4Info& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::aclIpv4_attribute::RESERVED_BY_CHANGE) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ACL IpV4: <%s>, modifying reservedBy --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getReservedBy().size(), data.getReservedBy().size());

			m_data.reservedBy.clear();
			m_data.reservedBy = data.getReservedBy();
			++modifiedAttributes;
		}

		m_data.changeMask |= data.changeMask;

		return modifiedAttributes;
	}

} /* namespace imm */
