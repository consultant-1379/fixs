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

#include "imm/bfdsession_oi.h"

#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_workingset.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_programconstants.h"

namespace imm
{

	BfdSession_OI::BfdSession_OI()
	: Base_OI(imm::moc_name::CLASS_BFD_SESSION, imm::oi_name::IMPL_BFDSESSION),
	  m_bfdSessionOperationTable()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BFD_Session_OI created");
	}

	BfdSession_OI::~BfdSession_OI()
	{

	}

	ACS_CC_ReturnType BfdSession_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(className);
		UNUSED(oiHandle);
		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		BfdSession bfdSessionObj;
		bfdSessionObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0U == bfdSession_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string sessionDN( reinterpret_cast<char *>(attr[idx]->attrValues[0]));
				sessionDN.push_back(common::utility::parserTag::comma);
				sessionDN.append(parentname);

				bfdSessionObj.info.moDN.assign(sessionDN);

				//get RDN value
				bfdSessionObj.info.name.assign(common::utility::getIdValueFromRdn(sessionDN));

				//get smxId
				bfdSessionObj.info.smxId.assign( common::utility::getSmxIdFromBfdSessionDN(sessionDN));


				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CREATE IPv4 BFD SESSION :"
						"\nDN:<%s>"
						"\nName:<%s>"
						"\nSMX ID:<%s>",
						bfdSessionObj.info.getDN(), bfdSessionObj.info.getName(), bfdSessionObj.info.getSmxId());

				// Operation is exclusive
				bfdSessionObj.exclusive = true;

				//add into the map and schedule the operation
				operationTable_t::iterator element = m_bfdSessionOperationTable.insert(std::make_pair(ccbId, bfdSessionObj));
				configurationHelper_t::instance()->scheduleCreateOperation( ccbId, BFDSESSION_T, &(element->second));

				result = ACS_CC_SUCCESS;

				break;
			}
		}

		return result;
	}

	ACS_CC_ReturnType BfdSession_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		BfdSession bfdSessionObj;
		bfdSessionObj.action = Delete;

		// set Session DN
		bfdSessionObj.info.moDN.assign(objName);

		//get RDN value
		bfdSessionObj.info.name.assign(common::utility::getIdValueFromRdn(bfdSessionObj.info.moDN));

		//get smxId
		bfdSessionObj.info.smxId.assign( common::utility::getSmxIdFromBfdSessionDN(bfdSessionObj.info.moDN));

		// Operation is exclusive
		bfdSessionObj.exclusive = true;
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Delete IPv4 BFD SESSION scheduled:<%d>",
				"\nDN:<%s>"
				"\nName:<%s>"
				"\nSMX ID:<%s>",
				ccbId, bfdSessionObj.info.getDN(), bfdSessionObj.info.getName(), bfdSessionObj.info.getSmxId());

		//add into the map and schedule the operation
		operationTable_t::iterator element = m_bfdSessionOperationTable.insert(std::make_pair(ccbId, bfdSessionObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId, BFDSESSION_T, &(element->second));

		return result;
	}

	ACS_CC_ReturnType BfdSession_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		fixs_ith::ErrorConstants operationResult = configurationHelper_t::instance()->executeMoOperation(ccbId);

		if(fixs_ith::ERR_NO_ERRORS != operationResult)
		{
			// Error
			result = ACS_CC_FAILURE;
			setExitCode(operationResult, configurationHelper_t::instance()->getErrorText(operationResult) );
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "COMPLETE BFD SESSION Operation, result:<%d>", operationResult);
		return result;
	}

	void  BfdSession_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void BfdSession_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	ACS_CC_ReturnType BfdSession_OI::updateRuntime(const char* p_objName, const char** p_attrName)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Update run-time attribute of BFD session:<%s>", p_objName);

		operation::bfdSessionInfo bfdSessionInfo;
		fixs_ith::ErrorConstants getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getBfdSessionInfo(p_objName, bfdSessionInfo);

		if(fixs_ith::ERR_NO_ERRORS != getResult)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Failed to get INFO on BFD Session:<%s>", p_objName);
			return result;
		}

		// search attribute to update
		for(uint16_t index = 0; p_attrName[index] != 0; ++index)
		{

			if( bfdSession_attribute::ADDR_DST.compare(p_attrName[index]) == 0U )
			{
				updateAttribute(p_objName, bfdSession_attribute::ADDR_DST.c_str(), bfdSessionInfo.getAddrDst());
				continue;
			}

			if( bfdSession_attribute::ADDR_SRC.compare(p_attrName[index]) == 0U )
			{
				updateAttribute(p_objName, bfdSession_attribute::ADDR_SRC.c_str(), bfdSessionInfo.getAddrSrc() );
				continue;
			}

			if( bfdSession_attribute::INTERVAL_TX.compare(p_attrName[index]) == 0U )
			{
				uint32_t intervalTXinMs = static_cast<uint32_t>(bfdSessionInfo.getIntervalTx() / common::MS_TO_MICROSEC);
				updateAttribute(p_objName, bfdSession_attribute::INTERVAL_TX.c_str(), intervalTXinMs );
				continue;
			}

			if( bfdSession_attribute::REMOTE_DISCRIMINATOR.compare(p_attrName[index]) == 0U )
			{
				updateAttribute(p_objName, bfdSession_attribute::REMOTE_DISCRIMINATOR.c_str(), bfdSessionInfo.getRemoteDiscr() );
				continue;
			}

			if( bfdSession_attribute::LOCAL_DISCRIMINATOR.compare(p_attrName[index]) == 0U )
			{
				updateAttribute(p_objName, bfdSession_attribute::LOCAL_DISCRIMINATOR.c_str(), bfdSessionInfo.getLocalDiscr() );
				continue;
			}

			if( bfdSession_attribute::PORT_SRC.compare(p_attrName[index]) == 0U )
			{
				updateAttribute(p_objName, bfdSession_attribute::PORT_SRC.c_str(), bfdSessionInfo.getPortSrc() );
				continue;
			}

			if( bfdSession_attribute::DETECTION_TIME.compare(p_attrName[index]) == 0U )
			{
				uint32_t detectionTimeinMs = static_cast<uint32_t>(bfdSessionInfo.getDetectionTime() / common::MS_TO_MICROSEC);
				updateAttribute(p_objName, bfdSession_attribute::DETECTION_TIME.c_str(), detectionTimeinMs );
				continue;
			}

			if( bfdSession_attribute::DIAG_CODE.compare(p_attrName[index]) == 0U )
			{
				updateAttribute(p_objName, bfdSession_attribute::DIAG_CODE.c_str(), bfdSessionInfo.getDiagCode() );
				continue;
			}

			if( bfdSession_attribute::SESSION_STATE.compare(p_attrName[index]) == 0U )
			{
				updateAttribute(p_objName, bfdSession_attribute::SESSION_STATE.c_str(), bfdSessionInfo.getSessionState() );
				continue;
			}
		}

		return result;
	}

	void BfdSession_OI::updateAttribute(const char* objectDN, const char* attributeName, const char* attributeValue)
	{
		ACS_CC_ImmParameter bfdMOAttribute;

		char tmpAttributeName[64] = {0};
		strcpy(tmpAttributeName, attributeName);
		bfdMOAttribute.attrName = tmpAttributeName;
		bfdMOAttribute.attrType = ATTR_STRINGT;
		bfdMOAttribute.attrValuesNum = 1;

		const uint16_t bufferSize = 256U;
		char tmpAttributeValue[bufferSize] = {0};
		snprintf(tmpAttributeValue, bufferSize, "%s", attributeValue );
		void* attributeValues[1] = { reinterpret_cast<void*>(tmpAttributeValue) };
		bfdMOAttribute.attrValues = attributeValues;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%s> for attribute <%s> on DN:<%s>",
				attributeValue, attributeName, objectDN);

		modifyRuntimeObj(objectDN, &bfdMOAttribute);
	}

	void BfdSession_OI::updateAttribute(const char* objectDN, const char* attributeName, uint32_t attributeValue)
	{
		ACS_CC_ImmParameter bfdMOAttribute;

		char tmpAttributeName[64] = {0};
		strcpy(tmpAttributeName, attributeName);
		bfdMOAttribute.attrName = tmpAttributeName;
		bfdMOAttribute.attrType = ATTR_UINT32T;
		bfdMOAttribute.attrValuesNum = 1;

		void* attributeValues[1] = { reinterpret_cast<void*>(&attributeValue) };
		bfdMOAttribute.attrValues = attributeValues;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%u> for attribute <%s> on DN:<%s>",
				attributeValue, attributeName, objectDN);

		modifyRuntimeObj(objectDN, &bfdMOAttribute);
	}

	void BfdSession_OI::updateAttribute(const char* objectDN, const char* attributeName, int32_t attributeValue)
	{
		ACS_CC_ImmParameter bfdMOAttribute;

		char tmpAttributeName[64] = {0};
		strcpy(tmpAttributeName, attributeName);
		bfdMOAttribute.attrName = tmpAttributeName;
		bfdMOAttribute.attrType = ATTR_INT32T;
		bfdMOAttribute.attrValuesNum = 1;

		void* attributeValues[1] = { reinterpret_cast<void*>(&attributeValue) };
		bfdMOAttribute.attrValues = attributeValues;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%d> for attribute <%s> on DN:<%s>",
				attributeValue, attributeName, objectDN);

		modifyRuntimeObj(objectDN, &bfdMOAttribute);
	}

	void BfdSession_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_bfdSessionOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_bfdSessionOperationTable.erase(operationRange.first, operationRange.second);
	}

} /* namespace imm */
