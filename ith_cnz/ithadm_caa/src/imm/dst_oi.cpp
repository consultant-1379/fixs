/*
 * dst_oi.cpp
 *
 *  Created on: May 17, 2016
 *      Author: estevol
 */

#include "imm/dst_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"

namespace imm
{

	Dst_OI::Dst_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	Dst_OI::~Dst_OI()
	{

	}

	ACS_CC_ReturnType Dst_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		Dst dstObj;
		dstObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == dst_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string dstRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				dstObj.info.moDN.assign(dstRDN);
				dstObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				dstObj.info.name = common::utility::getIdValueFromRdn(dstObj.info.moDN);

				//get smxId
				dstObj.info.smxId = common::utility::getSmxIdFromDstDN(dstObj.info.moDN);

				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == dst_attribute::DST.compare(attr[idx]->attrName) )
			{
				dstObj.info.changeMask |= imm::dst_attribute::DST_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					dstObj.info.dst = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				}
				result = ACS_CC_SUCCESS;
				continue;
			}
		}

		// Operation is exclusive
		dstObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedul the operation
			operationTable_t::iterator element = m_dstOperationTable.insert(std::make_pair(ccbId, dstObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,DST_T,&(element->second));
		}
		else
		{
			// Error
		}
		return result;
	}


	ACS_CC_ReturnType Dst_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		Dst dstObj;
		dstObj.action = Delete;
		dstObj.info.moDN.assign(objName);

		//get RDN value
		dstObj.info.name = common::utility::getIdValueFromRdn(dstObj.info.moDN);

		//get smxId value
		dstObj.info.smxId = common::utility::getSmxIdFromDstDN(dstObj.info.moDN);

		// Operation is exclusive
		dstObj.exclusive = true;

		//add into the map and schedul the operation
		operationTable_t::iterator element = m_dstOperationTable.insert(std::make_pair(ccbId, dstObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,DST_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType Dst_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);


		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		Dst dstObj;
		dstObj.action = Modify;
		dstObj.info.moDN.assign(objName);

		//get RDN value
		dstObj.info.name = common::utility::getIdValueFromRdn(dstObj.info.moDN);

		//get smxId
		dstObj.info.smxId = common::utility::getSmxIdFromDstDN(dstObj.info.moDN);

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;

			//check for dst attribute
			if( 0 == dst_attribute::DST.compare(modAttribute.attrName) )
			{
				dstObj.info.changeMask |= imm::dst_attribute::DST_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					dstObj.info.dst = reinterpret_cast<char *>(modAttribute.attrValues[0]);
				}
				continue;
			}
		}

		// Operation is exclusive
		dstObj.exclusive = true;

		if (ACS_CC_SUCCESS == result)
		{
			if (dstObj.info.changeMask)
			{
				//add into the map and schedul the operation
				operationTable_t::iterator element = m_dstOperationTable.insert(std::make_pair(ccbId, dstObj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,DST_T,&(element->second));
			}
		}
		else
		{
			//Error
		}

		return result;
	}

	ACS_CC_ReturnType Dst_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  Dst_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Dst_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Dst_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}

	void Dst_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_dstOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_dstOperationTable.erase(operationRange.first, operationRange.second);
	}

}
