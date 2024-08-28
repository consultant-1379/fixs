/*
 * 
 * COPYRIGHT Ericsson 2016
 *	All rights reserved.
 *
 *	The Copyright to the computer program(s) herein
 *	is the property of Ericsson 2016.
 *	The program(s) may be used and/or copied only with
 *	the written permission from Ericsson 2016 or in
 *	accordance with the terms and conditions stipulated in
 *	the agreement/contract under which the program(s) have
 *	been supplied.
 *
 *	
 *  Created on: Jul 1, 2016
 *      Author: xvincon
 */

#include "engine/bfdsessionipv4.h"
#include "common/utility.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

namespace engine
{
	const uint8_t MAX_DATA_OLDNESS = 5U; // after MAX_DATA_OLDNESS the data is out of data

	BfdSessionIPv4::BfdSessionIPv4(const operation::bfdSessionInfo& data)
	: m_data(data),
	  m_routerName( common::utility::getRouterNameFromBfdSessionIPv4DN( data.getDN() ) ),
	  m_lastUpdate(0U)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BfdSession created : MOC:<%s>", m_data.getDN());
	}

	BfdSessionIPv4::BfdSessionIPv4(const boost::shared_ptr<BfdSessionIPv4>& rhs)
	: m_data(rhs->m_data),
	  m_routerName(rhs->m_routerName),
	  m_lastUpdate(0U)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BfdSession created : MOC:<%s>", m_data.getDN());
	}

	BfdSessionIPv4::~BfdSessionIPv4()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BfdSession deleted : MOC:<%s>", m_data.getDN());
	}

	void BfdSessionIPv4::refreshAttribute(const operation::bfdSessionInfo& newData)
	{
		m_data.addrDst.assign(newData.addrDst);
		m_data.addrSrc.assign(newData.addrSrc);
		m_data.detectionTime = newData.detectionTime;
		m_data.interval_tx = newData.interval_tx;
		m_data.localDiscriminator = newData.localDiscriminator;
		m_data.remoteDiscriminator = newData.remoteDiscriminator;
		m_data.diagCode = newData.diagCode;
		m_data.portSrc = newData.portSrc;
		m_data.sessionState = newData.sessionState;

		m_lastUpdate = std::time(NULL);

	}

	bool BfdSessionIPv4::isOutOfDate() const
	{
		std::time_t currentTime = std::time(NULL);

		return ( (currentTime - m_lastUpdate) > MAX_DATA_OLDNESS);
	}
} /* namespace imm */
