/*
 * subnetVlan.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */

#include "imm/subnetVlan_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_logger.h"

namespace imm
{

	SubnetVlan_OI::SubnetVlan_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	SubnetVlan_OI::~SubnetVlan_OI()
	{

	}

	ACS_CC_ReturnType SubnetVlan_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;
		SubnetVlan subnetVlanObj;
		subnetVlanObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == subnetVlan_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string subnetVlanRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				subnetVlanObj.info.moDN.assign(subnetVlanRDN);
				subnetVlanObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				subnetVlanObj.info.name = common::utility::getIdValueFromRdn(subnetVlanObj.info.moDN);

				//get smxId and plane value
				subnetVlanObj.info.smxId = common::utility::getSmxIdFromBridgeDN(parentname);
				subnetVlanObj.info.plane = common::utility::getPlaneFromBridgeDN(parentname);

				result = ACS_CC_SUCCESS;
				continue;
			}

			// check for vid attribute
			if( 0 == subnetVlan_attribute::SUBNET_VID.compare(attr[idx]->attrName) )
			{

				if (0U != attr[idx]->attrValuesNum)
				{
					subnetVlanObj.info.vId = *reinterpret_cast<uint16_t*>(attr[idx]->attrValues[0]);
				}
				continue;
			}

			// check for tagged ports attribute
			if( 0 == subnetVlan_attribute::IP_SUBNET.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					subnetVlanObj.info.ipv4Subnet = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				}
				continue;
			}
		}

		// Operation is exclusive
		subnetVlanObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedul the operation
			operationTable_t::iterator element = m_subnetVlanOperationTable.insert(std::make_pair(ccbId, subnetVlanObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,SUBNETVLAN_T,&(element->second));
		}
		else
		{
			// Error
		}


		return result;
	}


	ACS_CC_ReturnType SubnetVlan_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		SubnetVlan subnetVlanObj;
		subnetVlanObj.action = Delete;
		subnetVlanObj.info.moDN.assign(objName);

		//get RDN value
		subnetVlanObj.info.name = common::utility::getIdValueFromRdn(subnetVlanObj.info.moDN);

		//get smxId and plane value
		subnetVlanObj.info.smxId = common::utility::getSmxIdFromBridgeDN(common::utility::getParentDN(subnetVlanObj.info.moDN));
		subnetVlanObj.info.plane = common::utility::getPlaneFromBridgeDN(common::utility::getParentDN(subnetVlanObj.info.moDN));

		// Operation is exclusive
		subnetVlanObj.exclusive = true;

		//add into the map and schedul the operation
		operationTable_t::iterator element = m_subnetVlanOperationTable.insert(std::make_pair(ccbId, subnetVlanObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,SUBNETVLAN_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType SubnetVlan_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		SubnetVlan subnetVlanObj;
		subnetVlanObj.action = Modify;
		subnetVlanObj.info.moDN.assign(objName);

		//get RDN value
		subnetVlanObj.info.name = common::utility::getIdValueFromRdn(subnetVlanObj.info.moDN);

		//get smxId and plane value
		subnetVlanObj.info.smxId = common::utility::getSmxIdFromBridgeDN(common::utility::getParentDN(subnetVlanObj.info.moDN));
		subnetVlanObj.info.plane = common::utility::getPlaneFromBridgeDN(common::utility::getParentDN(subnetVlanObj.info.moDN));

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;

			// check if vId attribute
			if( 0 == subnetVlan_attribute::SUBNET_VID.compare(modAttribute.attrName) )
			{
				subnetVlanObj.info.changeMask |= subnetVlan_attribute::VLAN_ID_CHANGE;

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Modifying <%s> on <%s>", modAttribute.attrName, objName );

				if (0 != modAttribute.attrValues)
				{
					subnetVlanObj.info.vId = *reinterpret_cast<uint16_t*>(modAttribute.attrValues[0]);
				}

				continue;
			}

			// check for tagged ports attribute
			if( 0 == subnetVlan_attribute::IP_SUBNET.compare(modAttribute.attrName) )
			{
				subnetVlanObj.info.changeMask |= subnetVlan_attribute::IP_SUBNET_CHANGE;

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Modifying <%s> on <%s>", modAttribute.attrName, objName );

				if (0 != modAttribute.attrValues)
				{
					subnetVlanObj.info.ipv4Subnet = reinterpret_cast<char *>(modAttribute.attrValues[0]);
				}
				continue;
			}

		}

		// Operation is exclusive
		subnetVlanObj.exclusive = true;

		if (ACS_CC_SUCCESS == result)
		{
			if (subnetVlanObj.info.changeMask)
			{
				//add into the map and schedul the operation
				operationTable_t::iterator element = m_subnetVlanOperationTable.insert(std::make_pair(ccbId, subnetVlanObj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,SUBNETVLAN_T,&(element->second));
			}
		}
		else
		{
			//Error
		}

		return result;
	}

	ACS_CC_ReturnType SubnetVlan_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  SubnetVlan_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void SubnetVlan_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);

	}

	void SubnetVlan_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(paramList);
		UNUSED(operationId);
		UNUSED(p_objName);
		UNUSED(invocation);
		UNUSED(oiHandle);
	}

	void SubnetVlan_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_subnetVlanOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_subnetVlanOperationTable.erase(operationRange.first, operationRange.second);
	}

}


