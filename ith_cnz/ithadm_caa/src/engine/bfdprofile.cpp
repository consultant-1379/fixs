/*
 * bfdProfile.cpp
 *
 *  Created on: Jun 1, 2016
 *      Author: estevol
 */

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "engine/bfdprofile.h"

namespace engine {

	BfdProfile::BfdProfile(const operation::bfdProfileInfo& data):
		m_data(data),
		m_routerName(common::utility::getRouterNameFromAddressDN(data.getDN()))
	{

	}

	BfdProfile::BfdProfile(const boost::shared_ptr<BfdProfile>& rhs):
		m_data(rhs->m_data),
		m_routerName(rhs->m_routerName)
	{

	}

	BfdProfile::~BfdProfile()
	{

	}

	uint16_t BfdProfile::modify(const operation::bfdProfileInfo& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::bfdProfile_attribute::DETECTION_MULTI_CHANGE) && (m_data.getDetectionMultiplier() != data.getDetectionMultiplier()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] bfdProfile: <%s>, modifying detectionMultiplier\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getDetectionMultiplier(), data.getDetectionMultiplier());

			m_data.detectionMultiplier = data.getDetectionMultiplier();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::bfdProfile_attribute::INTERVAL_TX_MIN_CHANGE) && (m_data.getIntervalTx() != data.getIntervalTx()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] bfdProfile: <%s>, modifying intervalTxMinDesired\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getIntervalTx(), data.getIntervalTx());

			m_data.intervalTxMinDesired = data.getIntervalTx();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::bfdProfile_attribute::INTERVAL_RX_MIN_CHANGE) && (m_data.getIntervalRx() != data.getIntervalRx()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] bfdProfile: <%s>, modifying intervalRxMinRequired\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getIntervalRx(), data.getIntervalRx());

			m_data.intervalRxMinRequired = data.getIntervalRx();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::bfdProfile_attribute::RESERVED_BY_CHANGE) && (m_data.getReservedBy().size() != data.getReservedBy().size()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BfdProfile: <%s>, modifying reservedBy\n"
					"old size: <%d>, new size: <%d>", data.getName(),
					m_data.getReservedBy().size(), data.getReservedBy().size());

			m_data.reservedBy.clear();
			m_data.reservedBy = data.getReservedBy();
			++modifiedAttributes;
		}

		m_data.changeMask |= data.changeMask;

		return modifiedAttributes;
	}

} /* namespace engine */
