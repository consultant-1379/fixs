/*
 * vlan_oi.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */

#include "imm/vlan_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"

#include <set>

namespace imm
{

	Vlan_OI::Vlan_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	Vlan_OI::~Vlan_OI()
	{

	}

	ACS_CC_ReturnType Vlan_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;
		Vlan vlanObj;
		vlanObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == vlan_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string vlanRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				vlanObj.info.moDN.assign(vlanRDN);
				vlanObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				vlanObj.info.name = common::utility::getIdValueFromRdn(vlanObj.info.moDN);

				//get smxId and plane value
				vlanObj.info.smxId = common::utility::getSmxIdFromBridgeDN(parentname);
				vlanObj.info.plane = common::utility::getPlaneFromBridgeDN(parentname);

				result = ACS_CC_SUCCESS;
				continue;
			}

			// check for vid attribute
			if( 0 == vlan_attribute::VLAN_ID.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					vlanObj.info.vId = *reinterpret_cast<uint16_t*>(attr[idx]->attrValues[0]);
				}

				continue;
			}

			// check for tagged ports attribute
			if( 0 == vlan_attribute::TAGGED_PORTS.compare(attr[idx]->attrName) )
			{
				vlanObj.info.taggedBridgePorts.clear();
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					vlanObj.info.taggedBridgePorts.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
				}

				continue;
			}

			// check for untagged ports attribute
			if( 0 == vlan_attribute::UNTAGGED_PORTS.compare(attr[idx]->attrName) )
			{
				vlanObj.info.untaggedBridgePorts.clear();
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					vlanObj.info.untaggedBridgePorts.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
				}

				continue;
			}

			if( 0 == vlan_attribute::RESERVED_BY.compare(attr[idx]->attrName) )
			{
				vlanObj.info.reservedBy.clear();
				for (uint32_t j = 0; j <  attr[idx]->attrValuesNum; j++)
				{
					vlanObj.info.reservedBy.insert(reinterpret_cast<char *>( attr[idx]->attrValues[j]));
				}

				continue;
			}
		}

		// Operation is exclusive
		vlanObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedul the operation
			operationTable_t::iterator element = m_vlanOperationTable.insert(std::make_pair(ccbId, vlanObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,VLAN_T,&(element->second));
		}
		else
		{
			// Error
		}


		return result;
	}


	ACS_CC_ReturnType Vlan_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		Vlan vlanObj;
		vlanObj.action = Delete;
		vlanObj.info.moDN.assign(objName);

		//get RDN value
		vlanObj.info.name = common::utility::getIdValueFromRdn(vlanObj.info.moDN);

		//get smxId and plane value
		vlanObj.info.smxId = common::utility::getSmxIdFromBridgeDN(common::utility::getParentDN(vlanObj.info.moDN));
		vlanObj.info.plane = common::utility::getPlaneFromBridgeDN(common::utility::getParentDN(vlanObj.info.moDN));

		// Operation is exclusive
		vlanObj.exclusive = true;

		//add into the map and schedul the operation
		operationTable_t::iterator element = m_vlanOperationTable.insert(std::make_pair(ccbId, vlanObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,VLAN_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType Vlan_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		UNUSED(attrMods);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		Vlan vlanObj;
		vlanObj.action = Modify;
		vlanObj.info.moDN.assign(objName);

		//get RDN value
		vlanObj.info.name = common::utility::getIdValueFromRdn(vlanObj.info.moDN);

		//get smxId and plane value
		vlanObj.info.smxId = common::utility::getSmxIdFromBridgeDN(common::utility::getParentDN(vlanObj.info.moDN));
		vlanObj.info.plane = common::utility::getPlaneFromBridgeDN(common::utility::getParentDN(vlanObj.info.moDN));

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;

			// check if vId attribute
			if( 0 == vlan_attribute::VLAN_ID.compare(modAttribute.attrName) )
			{
				vlanObj.info.changeMask |= vlan_attribute::VLAN_ID_CHANGE;

				if (0 != modAttribute.attrValuesNum)
				{
					vlanObj.info.vId = *reinterpret_cast<uint16_t*>(modAttribute.attrValues[0]);
				}
				else
				{
					// don't allow to assign EMPTY value to 'vid' attribute
					setExitCode(fixs_ith::ERR_CONFIG_INVALID_VLAN_ID, configurationHelper_t::instance()->getErrorText(fixs_ith::ERR_CONFIG_INVALID_VLAN_ID) );
					result = ACS_CC_FAILURE;
				}

				continue;
			}

			// check for tagged ports attribute
			if( 0 == vlan_attribute::TAGGED_PORTS.compare(modAttribute.attrName) )
			{
				vlanObj.info.changeMask |= vlan_attribute::TAGGED_PORT_CHANGE;

				vlanObj.info.taggedBridgePorts.clear();
				for (uint32_t j = 0; j < modAttribute.attrValuesNum; j++)
				{
					vlanObj.info.taggedBridgePorts.insert(reinterpret_cast<char *>(modAttribute.attrValues[j]));
				}

				continue;
			}

			// check for untagged ports attribute
			if( 0 == vlan_attribute::UNTAGGED_PORTS.compare(modAttribute.attrName) )
			{
				vlanObj.info.changeMask |= vlan_attribute::UNTAGGED_PORT_CHANGE;

				vlanObj.info.untaggedBridgePorts.clear();
				for (uint32_t j = 0; j < modAttribute.attrValuesNum; j++)
				{
					vlanObj.info.untaggedBridgePorts.insert(reinterpret_cast<char *>(modAttribute.attrValues[j]));
				}

				continue;
			}

			if( 0 == vlan_attribute::RESERVED_BY.compare(modAttribute.attrName) )
			{
				vlanObj.info.changeMask |= vlan_attribute::RESERVED_BY_CHANGE;

				vlanObj.info.reservedBy.clear();
				for (uint32_t j = 0; j < modAttribute.attrValuesNum; j++)
				{
					vlanObj.info.reservedBy.insert(reinterpret_cast<char *>(modAttribute.attrValues[j]));
				}

				continue;
			}
		}

		if (vlanObj.info.changeMask &
				(vlan_attribute::VLAN_ID_CHANGE | vlan_attribute::TAGGED_PORT_CHANGE | vlan_attribute::UNTAGGED_PORT_CHANGE))
		{
			// Operation is exclusive
			vlanObj.exclusive = true;
		}

		if (ACS_CC_SUCCESS == result)
		{
			if (vlanObj.info.changeMask)
			{
				//add into the map and schedul the operation
				operationTable_t::iterator element = m_vlanOperationTable.insert(std::make_pair(ccbId, vlanObj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,VLAN_T,&(element->second));
			}
		}
		else
		{
			//Error
		}

		return result;
	}

	ACS_CC_ReturnType Vlan_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  Vlan_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Vlan_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Vlan_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}

	void Vlan_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_vlanOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_vlanOperationTable.erase(operationRange.first, operationRange.second);
	}

}



