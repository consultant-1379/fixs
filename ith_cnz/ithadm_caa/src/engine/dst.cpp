/*
 * dst.cpp
 *
 *  Created on: May 17, 2016
 *      Author: estevol
 */

#include "engine/dst.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

	Dst::Dst(const operation::dstInfo& data):
			m_data(data),
			m_routerName(common::utility::getRouterNameFromDstDN(m_data.getDN()))
	{

	}

	Dst::Dst(const boost::shared_ptr<Dst>& rhs):
			m_data(rhs->m_data),
			m_routerName(rhs->m_routerName)
	{

	}

	Dst::~Dst()
	{

	}

	uint16_t Dst::modify(const operation::dstInfo& data)
	{
		uint16_t modifiedAttributes = 0;

		if( (data.changeMask & imm::dst_attribute::DST_CHANGE) && (m_data.dst.compare(data.getDst()) != 0) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Dst: <%s>, modifying dst\n"
					"old value: <%s>, new value: <%s>", data.getSmxId(), m_routerName.c_str(), data.getName(),
					m_data.getDst(), data.getDst());

			m_data.dst.assign(data.getDst());
			++modifiedAttributes;
		}

		m_data.changeMask |= data.changeMask;

		return modifiedAttributes;
	}


} /* namespace engine */
