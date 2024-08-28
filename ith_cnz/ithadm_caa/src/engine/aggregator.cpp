/*
 * aggregator.cpp
 *
 *  Created on: Mar 16, 2016
 *      Author: estevol
 */

#include "engine/aggregator.h"
#include "engine/ethernetport.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

Aggregator::Aggregator(const operation::aggregatorInfo& data):
		m_data(data)
{

}

Aggregator::Aggregator(const boost::shared_ptr<Aggregator>& rhs):
		m_data(rhs->m_data),
		m_ethernetPorts(rhs->m_ethernetPorts)
{

}

Aggregator::~Aggregator() {
	// TODO Auto-generated destructor stub
}

uint16_t Aggregator::modify(const operation::aggregatorInfo& data)
{
	uint16_t modifiedAttributes = 0;

	if( (data.changeMask & imm::aggregator_attribute::SNMP_INDEX_CHANGE) &&  (m_data.getSnmpIndex() != data.getSnmpIndex()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Aggregator: <%s>, modifying SNMP index\n"
				"old value: <%d>, new value: <%d>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getSnmpIndex(), data.getSnmpIndex());

		m_data.snmpIndex = data.getSnmpIndex();
		++modifiedAttributes;
	}

	if ((data.changeMask & imm::aggregator_attribute::ADMIN_AGG_MEMBER_CHANGE) && (m_data.getAdminAggMember() != data.getAdminAggMember()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Aggregator: <%s>, modifying admin agg member"
				, data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		m_data.aggMember = data.aggMember;
		++modifiedAttributes;
	}

	return modifiedAttributes;
}

std::set<uint16_t> Aggregator::getAdminAggMemebrIndex()
{
	std::set<uint16_t> indexes;

	for (std::set<boost::shared_ptr<EthernetPort> >::iterator it = m_ethernetPorts.begin(); it != m_ethernetPorts.end(); ++it)
	{
		indexes.insert((*it)->getSnmpIndex());
	}

	return indexes;
}

void Aggregator::setOperAggMember(const std::set<std::string> DNs)
{
	m_data.operAggMember = DNs;
}

void Aggregator::setOperAggMember(const std::set<uint16_t> indexes)
{
	m_data.operAggMember.clear();

	for (std::set<boost::shared_ptr<EthernetPort> >::iterator it = m_ethernetPorts.begin(); it != m_ethernetPorts.end(); ++it)
	{
		if (indexes.end() != indexes.find((*it)->getSnmpIndex()))
		{
			m_data.operAggMember.insert((*it)->getDN());
		}
	}
}

bool Aggregator::hasAdminAggMemberIndex(const uint16_t& index)
{
	std::set<uint16_t> indexes = getAdminAggMemebrIndex();
	return indexes.end() != indexes.find(index);
}

} /* namespace engine */
