/*
 * nexthop.cpp
 *
 *  Created on: May 9, 2016
 *      Author: estevol
 */

#include "engine/nexthop.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

	NextHop::NextHop(const operation::nextHopInfo& data):
		m_data(data),
		m_routerName(common::utility::getRouterNameFromNextHopDN(data.getDN())),
		m_dstName(common::utility::getDstNameFromNextHopDN(data.getDN()))
	{

	}

	NextHop::NextHop(const boost::shared_ptr<NextHop>& rhs):
		m_data(rhs->m_data),
		m_routerName(rhs->m_routerName),
		m_dstName(rhs->m_dstName)
	{

	}

	NextHop::~NextHop()
	{

	}

	uint16_t NextHop::modify(const operation::nextHopInfo& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::nextHop_attribute::ADMIN_DISTANCE_CHANGE) && (m_data.getAdminDistance() != data.getAdminDistance()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] NextHop: <%s>, modifying adminDistance\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getAdminDistance(), data.getAdminDistance());

			m_data.adminDistance = data.getAdminDistance();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::nextHop_attribute::ADDRESS_CHANGE) && (m_data.address.compare(data.getAddress()) != 0) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] NextHop: <%s>, modifying Address\n"
					"old value: <%s>, new value: <%s>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getAddress(), data.getAddress());

			m_data.address = data.getAddress();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::nextHop_attribute::DISCARD_CHANGE) && (m_data.getDiscard() != data.getDiscard()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] NextHop: <%s>, modifying discard\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getDiscard(), data.getDiscard());

			m_data.discard = data.getDiscard();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::nextHop_attribute::BFD_MONITORING_CHANGE) && (m_data.getBfdMonitoring() != data.getBfdMonitoring()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] NextHop: <%s>, modifying bfdMonitoring\n"
					"old value: <%d>, new value: <%d>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getBfdMonitoring(), data.getBfdMonitoring());

			m_data.bfdMonitoring = data.getBfdMonitoring();
			++modifiedAttributes;
		}

		m_data.changeMask |= data.changeMask;

		return modifiedAttributes;
	}


} /* namespace engine */
