/*
 * vlan.cpp
 *
 *  Created on: Mar 15, 2016
 *      Author: estevol
 */

#include "engine/vlan.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

Vlan::Vlan(const operation::vlanInfo& data):
		m_data(data)
{

}

Vlan::Vlan(const boost::shared_ptr<Vlan>& rhs):
		m_data(rhs->m_data),
		m_taggedBridgePorts(rhs->m_taggedBridgePorts),
		m_untaggedBridgePorts(rhs->m_untaggedBridgePorts)
{

}


Vlan::~Vlan()
{

}

void Vlan::setTaggedBridgePorts(std::set< boost::shared_ptr<BridgePort> > ports)
{
	m_taggedBridgePorts = ports;
	m_data.taggedBridgePorts.clear();

	addPortsDN(m_taggedBridgePorts, m_data.taggedBridgePorts);

}

void Vlan::setUntaggedBridgePorts(std::set< boost::shared_ptr<BridgePort> > ports)
{
	m_untaggedBridgePorts = ports;
	m_data.untaggedBridgePorts.clear();

	addPortsDN(m_untaggedBridgePorts, m_data.untaggedBridgePorts);

}

void Vlan::addTaggedBridgePort(std::set< boost::shared_ptr<BridgePort> > ports)
{
	m_taggedBridgePorts.insert(ports.begin(), ports.end());

	addPortsDN(ports, m_data.taggedBridgePorts);
}

void Vlan::addUntaggedBridgePort(std::set< boost::shared_ptr<BridgePort> > ports)
{
	m_untaggedBridgePorts.insert(ports.begin(), ports.end());

	addPortsDN(ports, m_data.untaggedBridgePorts);
}

void Vlan::removeTaggedBridgePorts(std::set< boost::shared_ptr<BridgePort> > ports)
{
	m_taggedBridgePorts.erase(ports.begin(), ports.end());

	removePortsDN(ports, m_data.taggedBridgePorts);
}

void Vlan::removeUntaggedBridgePorts(std::set< boost::shared_ptr<BridgePort> > ports)
{
	m_untaggedBridgePorts.erase(ports.begin(), ports.end());

	removePortsDN(ports, m_data.untaggedBridgePorts);
}

void Vlan::replaceTaggedBridgePort(boost::shared_ptr<BridgePort> port)
{
	for (std::set< boost::shared_ptr<BridgePort> >::iterator it = m_taggedBridgePorts.begin(); it != m_taggedBridgePorts.end(); ++it)
	{
		if (strcmp((*it)->getName(), port->getName()) == 0)
		{
			m_taggedBridgePorts.erase(it);
			break;
		}
	}

	m_taggedBridgePorts.insert(port);
}

void Vlan::replaceUntaggedBridgePort(boost::shared_ptr<BridgePort> port)
{
	for (std::set< boost::shared_ptr<BridgePort> >::iterator it = m_taggedBridgePorts.begin(); it != m_taggedBridgePorts.end(); ++it)
	{
		if (strcmp((*it)->getName(), port->getName()) == 0)
		{
			m_untaggedBridgePorts.erase(it);
			break;
		}
	}

	m_untaggedBridgePorts.insert(port);
}


uint16_t Vlan::modify(const operation::vlanInfo& data)
{
	uint16_t modifiedAttributes = 0;

	if( (data.changeMask & imm::vlan_attribute::VLAN_ID_CHANGE) && (m_data.getVlanId() != data.getVlanId()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Vlan: <%s>, modifying vid\n"
				"old value: <%d>, new value: <%d>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getVlanId(), data.getVlanId());

		m_data.vId = data.getVlanId();
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::vlan_attribute::TAGGED_PORT_CHANGE) && (m_data.getVlanTaggedPorts() != data.getVlanTaggedPorts()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Vlan: <%s>, modifying tagged ports\n"
				, data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		m_data.taggedBridgePorts = data.getVlanTaggedPorts();
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::vlan_attribute::UNTAGGED_PORT_CHANGE) && (m_data.getVlanUntaggedPorts() != data.getVlanUntaggedPorts()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Vlan: <%s>, modifying untagged ports\n"
				, data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		m_data.untaggedBridgePorts = data.getVlanUntaggedPorts();
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::vlan_attribute::RESERVED_BY_CHANGE) && (m_data.getReservedBy() != data.getReservedBy()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Vlan: <%s>, modifying reservedBy\n"
				, data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		m_data.reservedBy = data.getReservedBy();
		++modifiedAttributes;
	}

	return modifiedAttributes;
}

std::set<uint16_t> Vlan::getTaggedBridgePortsIndex()
{
	std::set<uint16_t> indexes;

	for (std::set< boost::shared_ptr<BridgePort> >::iterator it = m_taggedBridgePorts.begin(); it != m_taggedBridgePorts.end(); ++it)
	{
		indexes.insert((*it)->getSnmpIndex());
	}

	return indexes;

}

std::set<uint16_t>  Vlan::getUntaggedBridgePortsIndex()
{
	std::set<uint16_t> indexes;

	for (std::set< boost::shared_ptr<BridgePort> >::iterator it = m_untaggedBridgePorts.begin(); it != m_untaggedBridgePorts.end(); ++it)
	{
		indexes.insert((*it)->getSnmpIndex());
	}

	return indexes;
}

void Vlan::addPortsDN(const std::set< boost::shared_ptr<BridgePort> >& ports, std::set< std::string >& portsDN)
{
	for (std::set<boost::shared_ptr<BridgePort> >::iterator it = ports.begin(); it != ports.end(); ++it)
	{
		portsDN.insert((*it)->getBridgePortDN());
	}
}

void Vlan::removePortsDN(const std::set< boost::shared_ptr<BridgePort> >& ports, std::set< std::string >& portsDN)
{
	for (std::set<boost::shared_ptr<BridgePort> >::iterator it = ports.begin(); it != ports.end(); ++it)
	{
		portsDN.erase((*it)->getBridgePortDN());
	}
}

} /* namespace engine */
