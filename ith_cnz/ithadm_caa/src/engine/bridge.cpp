/*
 * bridge.cpp
 *
 *  Created on: Mar 15, 2016
 *      Author: estevol
 */

#include "engine/bridge.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

	Bridge::Bridge(const operation::bridgeInfo& data):
			m_data(data)
	{

	}

	Bridge::Bridge(const boost::shared_ptr<Bridge>& rhs):
			m_data(rhs->m_data)
	{

	}

	Bridge::~Bridge() {
		// TODO Auto-generated destructor stub
	}

	uint16_t Bridge::modify(const operation::bridgeInfo& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::bridge_attribute::BRIDGE_ADDRESS_CHANGE) && (strcmp(data.getBridgeAddress(), m_data.getBridgeAddress()) != 0) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Bridge: <%s>, modifying bridgeAddress\n"
					"old value: <%s>, new value: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(),
					m_data.getBridgeAddress(), data.getBridgeAddress());

			m_data.bridgeAddress = data.getBridgeAddress();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::bridge_attribute::IPV4_SUBNET_CHANGE) && (data.getSubnetVlanReference() !=  m_data.getSubnetVlanReference()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Bridge: <%s>, modifying IPv4SubnetVlan",
					data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

			m_data.ipv4SubnetVlan = data.getSubnetVlanReference();
			++modifiedAttributes;
		}

		return modifiedAttributes;
	}

} /* namespace engine */
