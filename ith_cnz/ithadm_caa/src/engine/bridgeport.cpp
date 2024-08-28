/*
 * bridgeport.cpp
 *
 *  Created on: Mar 15, 2016
 *      Author: estevol
 */

#include "engine/bridgeport.h"
#include "engine/aggregator.h"
#include "engine/ethernetport.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

BridgePort::BridgePort(const operation::bridgePortInfo& data):
		m_data(data)
{

}

BridgePort::BridgePort(const boost::shared_ptr<BridgePort>& rhs):
		m_data(rhs->m_data),
		m_ethernetPort(rhs->m_ethernetPort),
		m_aggregator(rhs->m_aggregator)
{

}

const uint16_t BridgePort::getSnmpIndex() const
{
	if (isEthernet())
	{
		return m_ethernetPort->getSnmpIndex();
	}
	else if (isAggregator())
	{
		return m_aggregator->getSnmpIndex();
	}

	return fixs_ith::DEFAULT_SNMP_INDEX;
}

uint16_t BridgePort::modify(const operation::bridgePortInfo& data)
{
	uint16_t modifiedAttributes = 0;
	m_data.changeMask= 0;

	if( (data.changeMask & imm::bridgePort_attribute::ADM_STATE_CHANGE) /* && (m_data.getAdminState() != data.getAdminState())*/ )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] BridgePort: <%s>, modifying administrative state\n"
				"old value: <%d>, new value: <%d>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getAdminState(), data.getAdminState());

		m_data.adminstate = data.getAdminState();
		m_data.changeMask |= imm::bridgePort_attribute::ADM_STATE_CHANGE;
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::bridgePort_attribute::DEFAULT_VLAN_ID_CHANGE) && (m_data.getDefaultVlanId() != data.getDefaultVlanId()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] BridgePort: <%s>, modifying default vlan ID\n"
				"old value: <%d>, new value: <%d>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getDefaultVlanId(), data.getDefaultVlanId());

		m_data.defaultVlanId = data.getDefaultVlanId();
		m_data.changeMask |= imm::bridgePort_attribute::DEFAULT_VLAN_ID_CHANGE;
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::bridgePort_attribute::PORT_ADDRESS_CHANGE) && (strcmp(m_data.getPortAddress(), data.getPortAddress()) != 0) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] BridgePort: <%s>, modifying port address\n"
				"old value: <%s>, new value: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getPortAddress(), data.getPortAddress());

		m_data.portAddress = data.getPortAddress();
		m_data.changeMask |= imm::bridgePort_attribute::PORT_ADDRESS_CHANGE;
		++modifiedAttributes;
	}

	return modifiedAttributes;
}

BridgePort::~BridgePort()
{
	// TODO Auto-generated destructor stub
}

} /* namespace engine */
