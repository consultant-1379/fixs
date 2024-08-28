/*
 * bridge_oi.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: eanform
 */

#include "imm/bridge_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "engine/context.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_transportBNChandler.h"

namespace imm
{

	Bridge_OI::Bridge_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	Bridge_OI::~Bridge_OI()
	{

	}

	ACS_CC_ReturnType Bridge_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		Bridge bridgeObj;
		bridgeObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == bridge_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string bridgeRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				bridgeObj.info.moDN.assign(bridgeRDN);
				bridgeObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				bridgeObj.info.name = common::utility::getIdValueFromRdn(bridgeObj.info.moDN);

				//get smxId and plane value
				bridgeObj.info.smxId = common::utility::getSmxIdFromBridgeDN(bridgeObj.info.moDN);
				bridgeObj.info.plane = common::utility::getPlaneFromBridgeDN(bridgeObj.info.moDN);

				result = ACS_CC_SUCCESS;
				continue;
			}

			// check for address attribute
			if( 0 == bridge_attribute::ADDRESS.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					bridgeObj.info.bridgeAddress = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				}
				continue;
			}

			// check for subnetvlan reference attribute
			if( 0 == bridge_attribute::SUBNET_VLAN_REF.compare(attr[idx]->attrName) )
			{
				bridgeObj.info.ipv4SubnetVlan.clear();
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					bridgeObj.info.ipv4SubnetVlan.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
				}

				continue;
			}
		}

		// Operation is exclusive
		bridgeObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedul the operation
			operationTable_t::iterator element = m_bridgeOperationTable.insert(std::make_pair(ccbId, bridgeObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,BRIDGE_T,&(element->second));
		}
		else
		{
			// Error
		}
		return result;
	}


	ACS_CC_ReturnType Bridge_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		Bridge bridgeObj;
		bridgeObj.action = Delete;
		bridgeObj.info.moDN.assign(objName);

		//get RDN value
		bridgeObj.info.name = common::utility::getIdValueFromRdn(bridgeObj.info.moDN);

		//get smxId and plane value
		bridgeObj.info.smxId = common::utility::getSmxIdFromBridgeDN(bridgeObj.info.moDN);
		bridgeObj.info.plane = common::utility::getPlaneFromBridgeDN(bridgeObj.info.moDN);

		// Operation is exclusive
		bridgeObj.exclusive = true;

		//add into the map and schedul the operation
		operationTable_t::iterator element = m_bridgeOperationTable.insert(std::make_pair(ccbId, bridgeObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,BRIDGE_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType Bridge_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		Bridge bridgeObj;
		bridgeObj.action = Modify;
		bridgeObj.info.moDN.assign(objName);

		//get RDN value
		bridgeObj.info.name = common::utility::getIdValueFromRdn(bridgeObj.info.moDN);

		//get smxId and plane value
		bridgeObj.info.smxId = common::utility::getSmxIdFromBridgeDN(bridgeObj.info.moDN);
		bridgeObj.info.plane = common::utility::getPlaneFromBridgeDN(bridgeObj.info.moDN);

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;
			// check for address attribute
			if( 0 == bridge_attribute::ADDRESS.compare(modAttribute.attrName) )
			{
				bridgeObj.info.changeMask |= bridge_attribute::BRIDGE_ADDRESS_CHANGE;

				if (0 != modAttribute.attrValuesNum)
				{
					bridgeObj.info.bridgeAddress = reinterpret_cast<char *>(modAttribute.attrValues[0]);
				}

				continue;
			}

			// check for subnetvlan reference attribute
			if( 0 == bridge_attribute::SUBNET_VLAN_REF.compare(modAttribute.attrName) )
			{
				bridgeObj.info.changeMask |= bridge_attribute::IPV4_SUBNET_CHANGE;

				bridgeObj.info.ipv4SubnetVlan.clear();
				for (uint32_t j = 0; j < modAttribute.attrValuesNum; j++)
				{
					bridgeObj.info.ipv4SubnetVlan.insert(reinterpret_cast<char *>(modAttribute.attrValues[j]));
				}


				continue;
			}
		}

		// Operation is exclusive
		bridgeObj.exclusive = true;

		if (ACS_CC_SUCCESS == result)
		{
			if (bridgeObj.info.changeMask)
			{
				//add into the map and schedul the operation
				operationTable_t::iterator element = m_bridgeOperationTable.insert(std::make_pair(ccbId, bridgeObj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,BRIDGE_T,&(element->second));
			}
		}
		else
		{
			//Error
		}

		return result;
	}

	ACS_CC_ReturnType Bridge_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  Bridge_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Bridge_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Bridge_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}

	ACS_CC_ReturnType Bridge_OI::updateRuntime(const char* p_objName, const char** p_attrName)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		engine::contextAccess_t access(common::utility::getSmxIdFromBridgeDN(p_objName), engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = access.getContext();


		if (context)
		{
			for (uint16_t i = 0; p_attrName[i] != 0; ++i)
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Updating attribute <%s> on DN:<%s>", p_attrName[i], p_objName);

				if ( bridge_attribute::NUM_OF_PORTS.compare(p_attrName[i]) == 0)
				{
					uint16_t num_of_ports = context->getBridgePorts(common::utility::getPlaneFromBridgeDN(p_objName)).size();
					char attrValue[8] = {0};
					::snprintf(attrValue, 8, "%u", num_of_ports);

					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%s> for attribute <%s> on DN:<%s>", attrValue, p_attrName[i], p_objName);

					ACS_CC_ImmParameter numOfPortsParameter;

					char tmpAttrName[IMM_ATTR_NAME_MAX_SIZE] = {0};
					strcpy(tmpAttrName,  bridge_attribute::NUM_OF_PORTS.c_str());
					numOfPortsParameter.attrName = tmpAttrName;
					numOfPortsParameter.attrType = ATTR_STRINGT;
					numOfPortsParameter.attrValuesNum = 1U;
					void *value[1] = {reinterpret_cast<void*>(attrValue)};
					numOfPortsParameter.attrValues = value;

					modifyRuntimeObj(p_objName, &numOfPortsParameter);
				}

				if ( bridge_attribute::NUM_OF_VLANS.compare(p_attrName[i]) == 0)
				{
					uint16_t num_of_vlans = context->getVlans(common::utility::getPlaneFromBridgeDN(p_objName)).size();
					char attrValue[8] = {0};
					::snprintf(attrValue, 8, "%u", num_of_vlans);

					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%s> for attribute <%s> on DN:<%s>", attrValue, p_attrName[i], p_objName);

					ACS_CC_ImmParameter numOfVlansParameter;

					char tmpAttrName[IMM_ATTR_NAME_MAX_SIZE] = {0};
					strcpy(tmpAttrName,  bridge_attribute::NUM_OF_VLANS.c_str());
					numOfVlansParameter.attrName = tmpAttrName;
					numOfVlansParameter.attrType = ATTR_STRINGT;
					numOfVlansParameter.attrValuesNum = 1U;
					void *value[1] = {reinterpret_cast<void*>(attrValue)};
					numOfVlansParameter.attrValues = value;

					modifyRuntimeObj(p_objName, &numOfVlansParameter);
				}
			}
		}

		return result;
	}

	void Bridge_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_bridgeOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_bridgeOperationTable.erase(operationRange.first, operationRange.second);
	}

}

