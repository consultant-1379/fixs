/*
 * erthernetport.cpp
 *
 *  Created on: Mar 16, 2016
 *      Author: estevol
 */

#include "engine/ethernetport.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

std::vector<fixs_ith_snmp::IfMauTypeListBitsConstants> not_auto_neg_vector_init() {
	std::vector<fixs_ith_snmp::IfMauTypeListBitsConstants> tmp;
	// Insert indexes of not auto negotiable mau types
	tmp.push_back(fixs_ith_snmp::IF_MAU_TYPE_10G_BASE_LR);
	tmp.push_back(fixs_ith_snmp::IF_MAU_TYPE_10G_BASE_SR);
	// ...
	return tmp;
}

const std::vector<fixs_ith_snmp::IfMauTypeListBitsConstants> EthernetPort::not_auto_negotiable_mau_types(not_auto_neg_vector_init());

EthernetPort::EthernetPort(const operation::ethernetPortInfo& data):
		m_data(data)
{

}

EthernetPort::EthernetPort(const boost::shared_ptr<EthernetPort>& rhs):
		m_data(rhs->m_data)
{

}

EthernetPort::~EthernetPort() {

}

uint16_t EthernetPort::modify(const operation::ethernetPortInfo& data)
{
	uint16_t modifiedAttributes = 0;

	if( (data.changeMask & imm::ethernetPort_attribute::SNMP_INDEX_CHANGE) && (m_data.getSnmpIndex() != data.getSnmpIndex()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] EthernetPort: <%s>, modifying SNMP index\n"
				"old value: <%d>, new value: <%d>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getSnmpIndex(), data.getSnmpIndex());

		m_data.snmpIndex = data.getSnmpIndex();
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::ethernetPort_attribute::MAC_ADDRESS_CHANGE) && (strcmp(m_data.getMacAddress(), data.getMacAddress()) != 0) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] EthernetPort: <%s>, modifying MAC address\n"
				"old value: <%s>, new value: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getMacAddress(), data.getMacAddress());

		m_data.macAddress = data.getMacAddress();
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::ethernetPort_attribute::MAX_FRAME_SIZE_CHANGE) && (m_data.getMaxFrameSize() != data.getMaxFrameSize()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] EthernetPort: <%s>, modifying MAX frame size\n"
				"old value: <%d>, new value: <%d>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getMaxFrameSize(), data.getMaxFrameSize());

		m_data.maxFrameSize = data.getMaxFrameSize();
		++modifiedAttributes;
	}

	if( (data.changeMask & imm::ethernetPort_attribute::AUTO_NEGOTIATE_CHANGE) && (m_data.getAutoNegotiate() != data.getAutoNegotiate()) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] EthernetPort: <%s>, modifying AutoNegotiate\n"
				"old value: <%d>, new value: <%d>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
				m_data.getAutoNegotiate(), data.getAutoNegotiate());

		m_data.autoNegotiate = data.getAutoNegotiate();
		++modifiedAttributes;
	}

	return modifiedAttributes;

}

bool EthernetPort::isAutoNegotiationSupported(std::vector<bool> & if_mau_type) const
{
	std::vector<fixs_ith_snmp::IfMauTypeListBitsConstants>::const_iterator it;
	for(it = not_auto_negotiable_mau_types.begin(); it != not_auto_negotiable_mau_types.end(); ++it)
	{
		if(if_mau_type[*it])
		{
			return false;
		}
	}
	return true;
}

} /* namespace engine */
