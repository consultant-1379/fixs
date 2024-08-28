/*
 * subnetvlan.cpp
 *
 *  Created on: Mar 16, 2016
 *      Author: estevol
 */

#include "engine/subnetvlan.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

SubnetVlan::SubnetVlan(const operation::subnetVlanInfo& data):
		m_data(data)
{

}

SubnetVlan::SubnetVlan(const boost::shared_ptr<SubnetVlan>& rhs):
		m_data(rhs->m_data),
		m_associatedVlan(rhs->m_associatedVlan)
{

}


SubnetVlan::~SubnetVlan() {
	// TODO Auto-generated destructor stub
}

uint16_t SubnetVlan::modify(const operation::subnetVlanInfo& data)
{
	uint16_t modifiedAttributes = 0;

	if( (data.changeMask & imm::subnetVlan_attribute::VLAN_ID_CHANGE) && (m_data.getSubnetVlanId() != data.getSubnetVlanId()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] SubnetVlan: <%s>, modifying vid\n"
				"old value: <%d>, new value: <%d>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getSubnetVlanId(), data.getSubnetVlanId());

		m_data.vId = data.getSubnetVlanId();
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::subnetVlan_attribute::IP_SUBNET_CHANGE) && (strcmp(m_data.getIpSubnet(), data.getIpSubnet()) != 0) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] SubnetVlan: <%s>, modifying IP subnet\n"
				"old value: <%s>, new value: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getIpSubnet(), data.getIpSubnet());

		m_data.ipv4Subnet.assign(data.getIpSubnet());
		++modifiedAttributes;
	}

	return modifiedAttributes;
}

} /* namespace engine */
