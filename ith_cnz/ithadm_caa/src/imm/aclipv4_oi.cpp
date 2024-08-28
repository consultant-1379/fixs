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
 *  Created on: Oct 18, 2016
 *      Author: xvincon
 */

#include "imm/aclipv4_oi.h"

#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_workingset.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_programconstants.h"

namespace imm
{

	AclIpv4_OI::AclIpv4_OI():
			Base_OI(imm::moc_name::CLASS_ACL, imm::oi_name::IMPL_ACL),
			m_aclIpv4OperationTable()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ACL IPv4 OI created");
	}



	AclIpv4_OI::~AclIpv4_OI()
	{

	}

	ACS_CC_ReturnType AclIpv4_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "ACL IpV4 create: ccbId<%llu>", ccbId);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		AclIpv4 aclIpV4Obj;
		aclIpV4Obj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == aclIpv4_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string aclIpv4RDN = reinterpret_cast<char*>(attr[idx]->attrValues[0]);
				aclIpv4RDN.push_back(common::utility::parserTag::comma);
				aclIpv4RDN.append(parentname);

				aclIpV4Obj.info.moDN.assign(aclIpv4RDN);

				//get RDN value
				aclIpV4Obj.info.name = common::utility::getIdValueFromRdn(aclIpV4Obj.info.moDN);

				//get smxId
				aclIpV4Obj.info.smxId = common::utility::getSmxIdFromAclDN(aclIpV4Obj.info.moDN);

				result = ACS_CC_SUCCESS;

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CREATE ACL IPv4 :"
										"\nDN:<%s>"
										"\nName:<%s>"
										"\nSMX ID:<%s>",
										aclIpV4Obj.info.getDN(), aclIpV4Obj.info.getName(), aclIpV4Obj.info.getSmxId());

				continue;
			}
		}

		// Operation is exclusive
		aclIpV4Obj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_aclIpv4OperationTable.insert(std::make_pair(ccbId, aclIpV4Obj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId, ACLIPV4_T, &(element->second));
		}

		return result;
	}


	ACS_CC_ReturnType AclIpv4_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "ACL IpV4 delete: ccbId<%llu>", ccbId);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		AclIpv4 aclIpV4Obj;
		aclIpV4Obj.action = Delete;
		aclIpV4Obj.info.moDN.assign(objName);

		//get RDN value
		aclIpV4Obj.info.name = common::utility::getIdValueFromRdn(aclIpV4Obj.info.moDN);

		//get smxId and plane value
		aclIpV4Obj.info.smxId = common::utility::getSmxIdFromAclDN(aclIpV4Obj.info.moDN);

		// Operation is exclusive
		aclIpV4Obj.exclusive = true;

		//add into the map and schedule the operation
		operationTable_t::iterator element = m_aclIpv4OperationTable.insert(std::make_pair(ccbId, aclIpV4Obj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId, ACLIPV4_T, &(element->second));

		return result;
	}


	ACS_CC_ReturnType AclIpv4_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		UNUSED(objName);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "ACL IpV4 modify: ccbId<%llu>", ccbId);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		AclIpv4 aclIpV4Obj;
		aclIpV4Obj.action = Modify;
		aclIpV4Obj.info.moDN.assign(objName);

		//get RDN value
		aclIpV4Obj.info.name = common::utility::getIdValueFromRdn(aclIpV4Obj.info.moDN);

		//get smxId and plane value
		aclIpV4Obj.info.smxId = common::utility::getSmxIdFromBfdProfileDN(aclIpV4Obj.info.moDN);

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]); ++idx)
		{

			if( 0 == aclIpv4_attribute::RESERVED_BY.compare(attrMods[idx]->modAttr.attrName) )
			{
				aclIpV4Obj.info.changeMask |= imm::aclIpv4_attribute::RESERVED_BY_CHANGE;

				aclIpV4Obj.info.reservedBy.clear();

				for (uint32_t j = 0; j <  attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					aclIpV4Obj.info.reservedBy.insert(reinterpret_cast<char *>( attrMods[idx]->modAttr.attrValues[j]));
				}
				continue;
			}
		}

		if(aclIpV4Obj.info.changeMask)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_aclIpv4OperationTable.insert(std::make_pair(ccbId, aclIpV4Obj));
			configurationHelper_t::instance()->scheduleModifyOperation(ccbId, ACLIPV4_T, &(element->second));
		}

		return result;
	}


	ACS_CC_ReturnType AclIpv4_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "ACL IpV4 complete: ccbId<%llu>", ccbId);

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

	void  AclIpv4_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void AclIpv4_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void AclIpv4_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_aclIpv4OperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_aclIpv4OperationTable.erase(operationRange.first, operationRange.second);

	}
} /* namespace imm */
