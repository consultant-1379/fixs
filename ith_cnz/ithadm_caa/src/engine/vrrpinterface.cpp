/*
 * vrrpinterface.cpp
 *
 *  Created on: Jun 1, 2016
 *      Author: estevol
 */

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_workingset.h"
#include "engine/vrrpinterface.h"

namespace engine {

	VrrpInterface::VrrpInterface(const operation::vrrpInterfaceInfo& data):
		m_data(data)
	{

	}

	VrrpInterface::VrrpInterface(const boost::shared_ptr<VrrpInterface>& rhs):
		m_data(rhs->m_data)
	{

	}

	VrrpInterface::~VrrpInterface()
	{

	}

	uint16_t VrrpInterface::modify(const operation::vrrpInterfaceInfo& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::vrrpInterface_attribute::ADV_INTERVAL_CHANGE) && (m_data.getAdvertiseInterval() != data.getAdvertiseInterval()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpInterface: <%s>, modifying advertiseInterval\n"
					"old value: <%d>, new value: <%d>", data.getName(),
					m_data.getAdvertiseInterval(), data.getAdvertiseInterval());

			m_data.advertiseInterval = data.getAdvertiseInterval();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::vrrpInterface_attribute::PREEMPT_HOLD_TIME_CHANGE) && (m_data.getPreemptHoldTime() != data.getPreemptHoldTime()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpInterface: <%s>, modifying preemptHoldTime\n"
					"old value: <%d>, new value: <%d>", data.getName(),
					m_data.getPreemptHoldTime(), data.getPreemptHoldTime());

			m_data.preemptHoldTime = data.getPreemptHoldTime();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::vrrpInterface_attribute::PREEMPT_MODE_CHANGE) && (m_data.getPreemptMode() != data.getPreemptMode()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpInterface: <%s>, modifying preemptMode\n"
					"old value: <%d>, new value: <%d>", data.getName(),
					m_data.getPreemptMode(), data.getPreemptMode());

			m_data.preemptMode = data.getPreemptMode();
			++modifiedAttributes;
		}

		if( (data.changeMask & imm::vrrpInterface_attribute::RESERVED_BY_CHANGE) && (m_data.getReservedBy().size() != data.getReservedBy().size()) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpInterface: <%s>, modifying reservedBy --> old size: <%d>, new size: <%d>",
					data.getName(), m_data.getReservedBy().size(), data.getReservedBy().size());

			m_data.reservedBy.clear();
			m_data.reservedBy = data.getReservedBy();
			++modifiedAttributes;

			if( 0 == data.getReservedBy().size() )
			{
				//Object of reference chosen to be the RDN of Vrrpv3Interface
				std::string vrrpMasterChangeObjOfRef = common::utility::getRDNAttribute(data.getDN());
				vrrpMasterChangeObjOfRef += "=";
				vrrpMasterChangeObjOfRef += common::utility::getVrrpInstanceNameFromDN(data.getDN());

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpInterface: <%s>, reservedBy is empty. Checking alarm to cease for object of reference <%s> ",
						data.getName(), vrrpMasterChangeObjOfRef.c_str());

				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseVrrpMasterChangeAlarm(vrrpMasterChangeObjOfRef);
			}
		}

		m_data.changeMask |= data.changeMask;

		return modifiedAttributes;
	}

} /* namespace engine */
