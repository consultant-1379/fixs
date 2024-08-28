/*
 * address_oi.cpp
 *
 *  Created on: May 17, 2016
 *      Author: estevol
 */

#include "imm/address_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_workingset.h"

namespace imm
{

	Address_OI::Address_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	Address_OI::~Address_OI()
	{

	}

	ACS_CC_ReturnType Address_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		Address addressObj;
		addressObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == addressIpv4_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string addressRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				addressObj.info.moDN.assign(addressRDN);
				addressObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				addressObj.info.name = common::utility::getIdValueFromRdn(addressObj.info.moDN);

				//get smxId
				addressObj.info.smxId = common::utility::getSmxIdFromAddressDN(addressObj.info.moDN);

				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == addressIpv4_attribute::ADDRESS.compare(attr[idx]->attrName) )
			{
				addressObj.info.changeMask |= imm::addressIpv4_attribute::ADDRESS_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					addressObj.info.address = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				}
				result = ACS_CC_SUCCESS;
				continue;
			}
		}

		// Operation is exclusive
		addressObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_addressOperationTable.insert(std::make_pair(ccbId, addressObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,ADDRESS_T,&(element->second));
		}
		else
		{
			// Error
		}
		return result;
	}


	ACS_CC_ReturnType Address_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		Address addressObj;
		addressObj.action = Delete;
		addressObj.info.moDN.assign(objName);

		//get RDN value
		addressObj.info.name = common::utility::getIdValueFromRdn(addressObj.info.moDN);

		//get smxId value
		addressObj.info.smxId = common::utility::getSmxIdFromAddressDN(addressObj.info.moDN);

		// Operation is exclusive
		addressObj.exclusive = true;

		//add into the map and schedule the operation
		operationTable_t::iterator element = m_addressOperationTable.insert(std::make_pair(ccbId, addressObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,ADDRESS_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType Address_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);


		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		Address addressObj;
		addressObj.action = Modify;
		addressObj.info.moDN.assign(objName);

		//get RDN value
		addressObj.info.name = common::utility::getIdValueFromRdn(addressObj.info.moDN);

		//get smxId
		addressObj.info.smxId = common::utility::getSmxIdFromAddressDN(addressObj.info.moDN);

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;

			//check for address attribute
			if( 0 == addressIpv4_attribute::ADDRESS.compare(modAttribute.attrName) )
			{
				addressObj.info.changeMask |= imm::addressIpv4_attribute::ADDRESS_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					addressObj.info.address = reinterpret_cast<char *>(modAttribute.attrValues[0]);
				}
				continue;
			}
		}

		// Operation is exclusive
		addressObj.exclusive = true;

		if (ACS_CC_SUCCESS == result)
		{
			if (addressObj.info.changeMask)
			{
				//add into the map and schedule the operation
				operationTable_t::iterator element = m_addressOperationTable.insert(std::make_pair(ccbId, addressObj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,ADDRESS_T,&(element->second));
			}
		}
		else
		{
			//Error
		}

		return result;
	}

	ACS_CC_ReturnType Address_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  Address_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		operationTable_t::iterator element = m_addressOperationTable.find(ccbId);

		if ( !element->second.info.isVRRPInterfaceAddress() )
		{
			operation::interfaceInfo interface;
			fixs_ith::workingSet_t::instance()->getConfigurationManager().getInterfaceIPv4byAddress(element->second.info, interface);

			if ( imm::ENABLED == interface.getBfdStaticRoute() )
			{
				std::string interfaceIPv4DN = interface.getDN();
				std::string smxId(element->second.info.getSmxId());

				if (imm::Create == element->second.action)
				{
					fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::ADD_BFD_SESSIONS, 0, 3, interfaceIPv4DN.c_str(), (interfaceIPv4DN.length() + 1)/sizeof(uint8_t));
				}
				else if ( imm::Modify == element->second.action && (element->second.info.changeMask & imm::addressIpv4_attribute::ADDRESS_CHANGE) )
				{
					// Address has been modified. First schedule old BFD sessions removal...
					fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::REMOVE_BFD_SESSIONS, 0, 3, interfaceIPv4DN.c_str(), (interfaceIPv4DN.length() + 1)/sizeof(uint8_t));
					// ...then schedule new BFD sessions creation
					fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::ADD_BFD_SESSIONS, 0, 3, interfaceIPv4DN.c_str(), (interfaceIPv4DN.length() + 1)/sizeof(uint8_t));
				}
				else if ( imm::Delete == element->second.action && (!fixs_ith::workingSet_t::instance()->getConfigurationManager().isRemoveContextOngoing(element->second.info.getSmxId())) )
				{
					fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::REMOVE_BFD_SESSIONS, 0, 3, interfaceIPv4DN.c_str(), (interfaceIPv4DN.length() + 1)/sizeof(uint8_t));
				}
			}
		}

		resetOperationTable(ccbId);
	}

	void Address_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Address_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}

	void Address_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_addressOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_addressOperationTable.erase(operationRange.first, operationRange.second);
	}

}
