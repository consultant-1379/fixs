/*
 * nextHop_oi.cpp
 *
 *  Created on: May 17, 2016
 *      Author: estevol
 */

#include "imm/nexthop_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "engine/configurationManager.h"
#include "common/utility.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_workingset.h"

namespace imm
{

	NextHop_OI::NextHop_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	NextHop_OI::~NextHop_OI()
	{

	}

	ACS_CC_ReturnType NextHop_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		NextHop nextHopObj;
		nextHopObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == nextHop_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string nextHopDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				nextHopDN.push_back(common::utility::parserTag::comma);
				nextHopDN.append(parentname);

				nextHopObj.info.moDN.assign(nextHopDN);

				//get RDN value
				nextHopObj.info.name.assign( common::utility::getIdValueFromRdn(nextHopDN) );

				//get smxId
				nextHopObj.info.smxId.assign( common::utility::getSmxIdFromNextHopDN(nextHopDN));

				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == nextHop_attribute::ADDRESS.compare(attr[idx]->attrName) )
			{
				nextHopObj.info.changeMask |= imm::nextHop_attribute::ADDRESS_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					nextHopObj.info.address = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				}
				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == nextHop_attribute::ADMIN_DISTANCE.compare(attr[idx]->attrName) )
			{
				nextHopObj.info.changeMask |= imm::nextHop_attribute::ADMIN_DISTANCE_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					nextHopObj.info.adminDistance = *(reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]));
				}
				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == nextHop_attribute::BFD_MONITORING.compare(attr[idx]->attrName) )
			{
				nextHopObj.info.changeMask |= imm::nextHop_attribute::BFD_MONITORING_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					nextHopObj.info.bfdMonitoring = (*(reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]))) > 0;
				}
				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == nextHop_attribute::DISCARD.compare(attr[idx]->attrName) )
			{
				nextHopObj.info.changeMask |= imm::nextHop_attribute::DISCARD_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					nextHopObj.info.discard = (*(reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]))) > 0;
				}
				result = ACS_CC_SUCCESS;
				continue;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CREATE NextHop:"
				"\nDN:<%s>"
				"\nSMX ID:<%s>"
				"\nAddress:<%s>"
				"\nDiscard:<%s>",
				nextHopObj.info.getDN(), nextHopObj.info.getSmxId(), nextHopObj.info.getAddress(), common::utility::boolToString(nextHopObj.info.getDiscard()) );

		// Operation is exclusive
		nextHopObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_nextHopOperationTable.insert(std::make_pair(ccbId, nextHopObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,NEXTHOP_T,&(element->second));
		}

		return result;
	}


	ACS_CC_ReturnType NextHop_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		NextHop nextHopObj;
		nextHopObj.action = Delete;
		nextHopObj.info.moDN.assign(objName);

		//get RDN value
		nextHopObj.info.name = common::utility::getIdValueFromRdn(nextHopObj.info.moDN);

		//get smxId value
		nextHopObj.info.smxId = common::utility::getSmxIdFromNextHopDN(nextHopObj.info.moDN);

		// Operation is exclusive
		nextHopObj.exclusive = true;

		//add into the map and schedule the operation
		operationTable_t::iterator element = m_nextHopOperationTable.insert(std::make_pair(ccbId, nextHopObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,NEXTHOP_T,&(element->second));

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Delete NextHop:"
						"\nDN:<%s>"
						"\nSMX ID:<%s>",
						nextHopObj.info.getDN(), nextHopObj.info.getSmxId() );

		return result;
	}

	ACS_CC_ReturnType NextHop_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		NextHop nextHopObj;
		nextHopObj.action = Modify;
		nextHopObj.info.moDN.assign(objName);

		//get RDN value
		nextHopObj.info.name = common::utility::getIdValueFromRdn(nextHopObj.info.moDN);

		//get smxId
		nextHopObj.info.smxId = common::utility::getSmxIdFromNextHopDN(nextHopObj.info.moDN);

		// extract the attributes to modify
		for(size_t idx = 0U; NULL != attrMods[idx]; ++idx)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;

			//check for nextHop attribute
			if( 0U == nextHop_attribute::ADMIN_DISTANCE.compare(modAttribute.attrName) )
			{
				nextHopObj.info.changeMask |= imm::nextHop_attribute::ADMIN_DISTANCE_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					nextHopObj.info.adminDistance = *(reinterpret_cast<int16_t*>(modAttribute.attrValues[0]));
				}

				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0U == nextHop_attribute::BFD_MONITORING.compare(modAttribute.attrName) )
			{
				nextHopObj.info.changeMask |= imm::nextHop_attribute::BFD_MONITORING_CHANGE;

				if(0U != modAttribute.attrValuesNum)
				{
					nextHopObj.info.bfdMonitoring = (*(reinterpret_cast<int32_t*>(modAttribute.attrValues[0]))) > 0;
				}

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Modify NextHopDN:<%s>, bfdMonitoring set to <%s>",
						objName, common::utility::boolToString(nextHopObj.info.getBfdMonitoring()));

				result = ACS_CC_SUCCESS;
				continue;
			}

		}

		// Operation is exclusive
		nextHopObj.exclusive = true;

		if(ACS_CC_SUCCESS == result )
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_nextHopOperationTable.insert(std::make_pair(ccbId, nextHopObj));
			configurationHelper_t::instance()->scheduleModifyOperation(ccbId, NEXTHOP_T, &(element->second));

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Modify NextHop:"
									"\nDN:<%s>"
									"\nSMX ID:<%s>",
									nextHopObj.info.getDN(), nextHopObj.info.getSmxId() );
		}

		return result;
	}

	ACS_CC_ReturnType NextHop_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		fixs_ith::ErrorConstants operationResult = configurationHelper_t::instance()->executeMoOperation(ccbId);
		if (fixs_ith::ERR_NO_ERRORS == operationResult)
		{
			result = ACS_CC_SUCCESS;
		}
		else
		{
			// Error
			setExitCode(operationResult, configurationHelper_t::instance()->getErrorText(operationResult) );
		}
		return result;
	}

	void  NextHop_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		operationTable_t::const_iterator element = m_nextHopOperationTable.find(ccbId);

		if(m_nextHopOperationTable.end() != element)
		{
			std::string nextHopDN(element->second.info.getDN());
			std::string smxId = element->second.info.getSmxId();

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NextHopDN:<%s>, changeMask:<%d>",
					nextHopDN.c_str(), element->second.info.changeMask);

			switch(element->second.action )
			{

				case imm::Create:
				{
					if( (element->second.info.changeMask & imm::nextHop_attribute::BFD_MONITORING_CHANGE) &&
							element->second.info.bfdMonitoring &&
							(element->second.info.changeMask & imm::nextHop_attribute::ADDRESS_CHANGE) )
					{
						fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::ADD_BFD_SESSION_FOR_NEXTHOP, 0, 3, nextHopDN.c_str(), (nextHopDN.length() + 1U));
					}
				}
				break;

				case imm::Modify:
				{
					if( (element->second.info.changeMask & imm::nextHop_attribute::BFD_MONITORING_CHANGE) &&
						element->second.info.bfdMonitoring &&
						fixs_ith::workingSet_t::instance()->getConfigurationManager().hasNextHopAddress(nextHopDN) )
					{
						fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::ADD_BFD_SESSION_FOR_NEXTHOP, 0, 3, nextHopDN.c_str(), (nextHopDN.length() + 1U));
					}
				}
				break;

				case imm::Delete :
				{
					if( !fixs_ith::workingSet_t::instance()->getConfigurationManager().isRemoveContextOngoing(smxId) )
						fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::REMOVE_BFD_SESSION_FOR_NEXTHOP, 0, 3, nextHopDN.c_str(), (nextHopDN.length() + 1U));
				}
				break;

				default:
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unknown action <%d> NextHopDN:<%s>",
							element->second.action, nextHopDN.c_str() );
			}
		}

		resetOperationTable(ccbId);
	}

	void NextHop_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void NextHop_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_nextHopOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_nextHopOperationTable.erase(operationRange.first, operationRange.second);
	}

}
