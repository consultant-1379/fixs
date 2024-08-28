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
 *  Created on: Jun 10, 2016
 *      Author: xclaele
 */

#include "imm/bfdprofile_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_transportBNChandler.h"
#include "engine/context.h"
#include "engine/configurationManager.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

namespace imm {

BfdProfile_OI::BfdProfile_OI()
	: Base_OI(imm::moc_name::CLASS_BFD_PROFILE, imm::oi_name::IMPL_BFDPROFILE),
	  m_bfdProfileOperationTable()
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BfdProfile_OI created");
}

BfdProfile_OI::~BfdProfile_OI() {

}

ACS_CC_ReturnType BfdProfile_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
{
	UNUSED(oiHandle);
	UNUSED(className);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "BFD Profile create: ccbId<%llu>", ccbId);

	ACS_CC_ReturnType result = ACS_CC_FAILURE;

	BfdProfile BfdProfileObj;
	BfdProfileObj.action = Create;

	// extract the attributes
	for(size_t idx = 0U; NULL != attr[idx]; ++idx)
	{
		// if this is the RDN attribute then get RDN
		if( 0 == bfdProfile_attribute::RDN.compare(attr[idx]->attrName) )
		{
			//get DN
			std::string bfdProfileRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
			BfdProfileObj.info.moDN.assign(bfdProfileRDN);
			BfdProfileObj.info.moDN += "," + std::string(parentname);

			//get RDN value
			BfdProfileObj.info.name = common::utility::getIdValueFromRdn(BfdProfileObj.info.moDN);

			//get smxId
			BfdProfileObj.info.smxId = common::utility::getSmxIdFromBfdProfileDN(BfdProfileObj.info.moDN);

			result = ACS_CC_SUCCESS;
			continue;
		}

		if( 0 == bfdProfile_attribute::DETECTION_MULTI.compare(attr[idx]->attrName) )
		{
			BfdProfileObj.info.changeMask |= imm::bfdProfile_attribute::DETECTION_MULTI_CHANGE;

			if(0U != attr[idx]->attrValuesNum)
			{
				BfdProfileObj.info.detectionMultiplier = *(reinterpret_cast<uint8_t *>(attr[idx]->attrValues[0]));
			}
			continue;
		}

		if( 0 == bfdProfile_attribute::INTERVAL_TX_MIN.compare(attr[idx]->attrName) )
		{
			BfdProfileObj.info.changeMask |= imm::bfdProfile_attribute::INTERVAL_TX_MIN_CHANGE;

			if(0U != attr[idx]->attrValuesNum)
			{
				// Conversion from milliseconds to microseconds
				BfdProfileObj.info.intervalTxMinDesired =
						common::MS_TO_MICROSEC * ( *(reinterpret_cast<uint32_t*>(attr[idx]->attrValues[0])) );
			}
			continue;
		}

		if( 0 == bfdProfile_attribute::INTERVAL_RX_MIN.compare(attr[idx]->attrName) )
		{
			BfdProfileObj.info.changeMask |= imm::bfdProfile_attribute::INTERVAL_RX_MIN_CHANGE;

			if (0U != attr[idx]->attrValuesNum)
			{
				// Conversion from milliseconds to microseconds
				BfdProfileObj.info.intervalRxMinRequired =
						common::MS_TO_MICROSEC * ( *(reinterpret_cast<uint32_t*>(attr[idx]->attrValues[0])) );
			}
			continue;
		}

	}

	// Operation is exclusive
	BfdProfileObj.exclusive = true;

	if(ACS_CC_SUCCESS == result)
	{
		//add into the map and schedule the operation
		operationTable_t::iterator element = m_bfdProfileOperationTable.insert(std::make_pair(ccbId, BfdProfileObj));
		configurationHelper_t::instance()->scheduleCreateOperation(ccbId, BFDPROFILE_T, &(element->second));
	}

	return result;
}

ACS_CC_ReturnType BfdProfile_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	UNUSED(oiHandle);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "BFD Profile delete: ccbId<%llu>", ccbId);

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	BfdProfile BfdProfileObj;
	BfdProfileObj.action = Delete;
	BfdProfileObj.info.moDN.assign(objName);

	//get RDN value
	BfdProfileObj.info.name = common::utility::getIdValueFromRdn(BfdProfileObj.info.moDN);

	//get smxId and plane value
	BfdProfileObj.info.smxId = common::utility::getSmxIdFromBfdProfileDN(BfdProfileObj.info.moDN);

	// Operation is exclusive
	BfdProfileObj.exclusive = true;

	//add into the map and schedule the operation
	operationTable_t::iterator element = m_bfdProfileOperationTable.insert(std::make_pair(ccbId, BfdProfileObj));
	configurationHelper_t::instance()->scheduleDeleteOperation(ccbId, BFDPROFILE_T, &(element->second));

	return result;
}

ACS_CC_ReturnType BfdProfile_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	UNUSED(oiHandle);
	UNUSED(objName);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "BFD Profile modify: ccbId<%llu>", ccbId);

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	BfdProfile BfdProfileObj;
	BfdProfileObj.action = Modify;
	BfdProfileObj.info.moDN.assign(objName);

	//get RDN value
	BfdProfileObj.info.name = common::utility::getIdValueFromRdn(BfdProfileObj.info.moDN);

	//get smxId and plane value
	BfdProfileObj.info.smxId = common::utility::getSmxIdFromBfdProfileDN(BfdProfileObj.info.moDN);


	// extract the attributes to modify
	for(size_t idx = 0U; (NULL != attrMods[idx]); ++idx)
	{
		if( 0 == bfdProfile_attribute::DETECTION_MULTI.compare(attrMods[idx]->modAttr.attrName) )
		{
			BfdProfileObj.info.changeMask |= imm::bfdProfile_attribute::DETECTION_MULTI_CHANGE;

			if(0U != attrMods[idx]->modAttr.attrValuesNum)
			{
				BfdProfileObj.info.detectionMultiplier = *(reinterpret_cast<uint8_t *>(attrMods[idx]->modAttr.attrValues[0]));
			}
			continue;
		}

		if( 0 == bfdProfile_attribute::INTERVAL_TX_MIN.compare(attrMods[idx]->modAttr.attrName) )
		{
			BfdProfileObj.info.changeMask |= imm::bfdProfile_attribute::INTERVAL_TX_MIN_CHANGE;

			if(0U != attrMods[idx]->modAttr.attrValuesNum)
			{
				// Conversion from milliseconds to microseconds
				BfdProfileObj.info.intervalTxMinDesired =
						common::MS_TO_MICROSEC * ( *(reinterpret_cast<uint32_t*>(attrMods[idx]->modAttr.attrValues[0])) );
			}
			continue;
		}

		if( 0 == bfdProfile_attribute::INTERVAL_RX_MIN.compare(attrMods[idx]->modAttr.attrName) )
		{
			BfdProfileObj.info.changeMask |= imm::bfdProfile_attribute::INTERVAL_RX_MIN_CHANGE;

			if (0U != attrMods[idx]->modAttr.attrValuesNum)
			{
				// Conversion from milliseconds to microseconds
				BfdProfileObj.info.intervalRxMinRequired =
						common::MS_TO_MICROSEC * ( *(reinterpret_cast<uint32_t*>(attrMods[idx]->modAttr.attrValues[0])) );
			}
			continue;
		}

		if( 0 == bfdProfile_attribute::RESERVED_BY.compare(attrMods[idx]->modAttr.attrName) )
		{
			BfdProfileObj.info.changeMask |= imm::bfdProfile_attribute::RESERVED_BY_CHANGE;

			BfdProfileObj.info.reservedBy.clear();
			for (uint32_t j = 0; j <  attrMods[idx]->modAttr.attrValuesNum; j++)
			{
				BfdProfileObj.info.reservedBy.insert(reinterpret_cast<char *>( attrMods[idx]->modAttr.attrValues[j]));
			}
			continue;
		}
	}

	//ReservedBy change is not an exclusive operation
	BfdProfileObj.exclusive = (BfdProfileObj.info.changeMask != bfdProfile_attribute::RESERVED_BY_CHANGE);

	if (BfdProfileObj.info.changeMask)
	{
		//add into the map and schedule the operation
		operationTable_t::iterator element = m_bfdProfileOperationTable.insert(std::make_pair(ccbId, BfdProfileObj));
		configurationHelper_t::instance()->scheduleModifyOperation(ccbId, BFDPROFILE_T, &(element->second));
	}

	return result;
}

ACS_CC_ReturnType BfdProfile_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	UNUSED(oiHandle);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "BFD Profile complete: ccbId<%llu>", ccbId);

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	fixs_ith::ErrorConstants operationResult = configurationHelper_t::instance()->executeMoOperation(ccbId);

	if(fixs_ith::ERR_NO_ERRORS != operationResult)
	{
		result = ACS_CC_FAILURE;
		// Error
		setExitCode(operationResult, configurationHelper_t::instance()->getErrorText(operationResult) );
	}

	return result;
}

void  BfdProfile_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	UNUSED(oiHandle);

	resetOperationTable(ccbId);
}

void BfdProfile_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	UNUSED(oiHandle);

	resetOperationTable(ccbId);
}

void BfdProfile_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
{
	std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
	operationRange = m_bfdProfileOperationTable.equal_range(ccbId);

	// Erase all elements from the table of the operations
	m_bfdProfileOperationTable.erase(operationRange.first, operationRange.second);

}

} /* namespace imm */
