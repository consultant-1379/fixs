/*
 * ethernetPort.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */


#include "imm/ethernetPort_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "engine/context.h"
#include "fixs_ith_workingset.h"

namespace imm
{

	EthernetPort_OI::EthernetPort_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	EthernetPort_OI::~EthernetPort_OI()
	{

	}

	ACS_CC_ReturnType EthernetPort_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		EthernetPort ethernetPortObj;
		ethernetPortObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == ethernetPort_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string ethernetPortRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				ethernetPortObj.info.moDN.assign(ethernetPortRDN);
				ethernetPortObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				ethernetPortObj.info.name = common::utility::getIdValueFromRdn(ethernetPortObj.info.moDN);

				//get smxId and plane value
				ethernetPortObj.info.smxId = common::utility::getSmxIdFromEthernetPortDN(ethernetPortObj.info.moDN);
				ethernetPortObj.info.plane = common::utility::getPlaneFromEthernetPortDN(ethernetPortObj.info.moDN);

				result = ACS_CC_SUCCESS;
				continue;
			}

			// check for address attribute
			if( 0 == ethernetPort_attribute::MAC_ADDRESS.compare(attr[idx]->attrName) )
			{

				if (0U != attr[idx]->attrValuesNum)
				{
					ethernetPortObj.info.macAddress = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				}
				continue;
			}

			//check for SNMP index attribute
			if( 0 == ethernetPort_attribute::SNMP_INDEX.compare(attr[idx]->attrName) )
			{

				if (0U != attr[idx]->attrValuesNum)
				{
					ethernetPortObj.info.snmpIndex = *(reinterpret_cast<uint16_t *>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == ethernetPort_attribute::MAX_FRAME_SIZE.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					ethernetPortObj.info.maxFrameSize = *(reinterpret_cast<uint32_t *>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( (0 == ethernetPort_attribute::AUTO_NEGOTIATE.compare(attr[idx]->attrName))
					&& (0U != attr[idx]->attrValuesNum) )
			{
				int value = *reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]);
				ethernetPortObj.info.autoNegotiate = ( 0U == value ? AN_LOCKED : AN_UNLOCKED );
				ethernetPortObj.info.changeMask |= imm::ethernetPort_attribute::AUTO_NEGOTIATE_CHANGE;

				continue;
			}

		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ETH_PORT create callback: IF_INDEX: %u, MAX FRAME SIZE: %u, AUTO-NEGOTIATE: %s", ethernetPortObj.info.snmpIndex,
				ethernetPortObj.info.maxFrameSize, (AN_LOCKED == ethernetPortObj.info.autoNegotiate) ? "LOCKED" : "UNLOCKED");

		// Operation is exclusive
		ethernetPortObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_ethernetPortOperationTable.insert(std::make_pair(ccbId, ethernetPortObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,ETHERNETPORT_T,&(element->second));
		}

		return result;
	}


	ACS_CC_ReturnType EthernetPort_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		EthernetPort ethernetPortObj;
		ethernetPortObj.action = Delete;
		ethernetPortObj.info.moDN.assign(objName);

		//get RDN value
		ethernetPortObj.info.name = common::utility::getIdValueFromRdn(ethernetPortObj.info.moDN);

		//get smxId and plane value
		ethernetPortObj.info.smxId = common::utility::getSmxIdFromEthernetPortDN(ethernetPortObj.info.moDN);
		ethernetPortObj.info.plane = common::utility::getPlaneFromEthernetPortDN(ethernetPortObj.info.moDN);

		// Operation is exclusive
		ethernetPortObj.exclusive = true;

		//add into the map and schedule the operation
		operationTable_t::iterator element = m_ethernetPortOperationTable.insert(std::make_pair(ccbId, ethernetPortObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,ETHERNETPORT_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType EthernetPort_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		UNUSED(objName);
		UNUSED(ccbId);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		EthernetPort ethernetPortObj;
		ethernetPortObj.action = Modify;
		ethernetPortObj.info.moDN.assign(objName);

		//get RDN value
		ethernetPortObj.info.name = common::utility::getIdValueFromRdn(ethernetPortObj.info.moDN);

		//get smxId and plane value
		ethernetPortObj.info.smxId = common::utility::getSmxIdFromEthernetPortDN(ethernetPortObj.info.moDN);
		ethernetPortObj.info.plane = common::utility::getPlaneFromEthernetPortDN(ethernetPortObj.info.moDN);

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;
			// check for address attribute
			if( 0 == ethernetPort_attribute::MAC_ADDRESS.compare(modAttribute.attrName) )
			{
				ethernetPortObj.info.changeMask |= imm::ethernetPort_attribute::MAC_ADDRESS_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					ethernetPortObj.info.macAddress = reinterpret_cast<char *>(modAttribute.attrValues[0]);
				}
				continue;
			}

			//check for SNMP index attribute
			if( 0 == ethernetPort_attribute::SNMP_INDEX.compare(modAttribute.attrName) )
			{
				ethernetPortObj.info.changeMask |= imm::ethernetPort_attribute::SNMP_INDEX_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					ethernetPortObj.info.snmpIndex = *(reinterpret_cast<uint16_t *>(modAttribute.attrValues[0]));
				}
				continue;
			}

			if( 0 == ethernetPort_attribute::MAX_FRAME_SIZE.compare(modAttribute.attrName) )
			{
				ethernetPortObj.info.changeMask |= imm::ethernetPort_attribute::MAX_FRAME_SIZE_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					ethernetPortObj.info.maxFrameSize = *(reinterpret_cast<uint32_t *>(modAttribute.attrValues[0]));
				}
				continue;
			}

			if( (0 == ethernetPort_attribute::AUTO_NEGOTIATE.compare(modAttribute.attrName))
					&& (0U != modAttribute.attrValuesNum) )
			{
				int value = *reinterpret_cast<int32_t*>(modAttribute.attrValues[0]);
				ethernetPortObj.info.autoNegotiate = ( 0U == value ? AN_LOCKED : AN_UNLOCKED );
				ethernetPortObj.info.changeMask |= imm::ethernetPort_attribute::AUTO_NEGOTIATE_CHANGE;

				continue;
			}

		}

		// Operation is exclusive
		ethernetPortObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			if (ethernetPortObj.info.changeMask)
			{
				//add into the map and schedule the operation
				operationTable_t::iterator element = m_ethernetPortOperationTable.insert(std::make_pair(ccbId, ethernetPortObj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,ETHERNETPORT_T,&(element->second));
			}
		}


		return result;
	}

	ACS_CC_ReturnType EthernetPort_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  EthernetPort_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void EthernetPort_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void EthernetPort_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);;
	}

	ACS_CC_ReturnType EthernetPort_OI::updateRuntime(const char* p_objName, const char** p_attrName)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		engine::contextAccess_t access(common::utility::getSmxIdFromEthernetPortDN(p_objName), engine::GET_EXISTING, engine::SHARED_ACCESS);

		engine::Context* context = access.getContext();

		if (context)
		{
			boost::shared_ptr<engine::EthernetPort> ethernePort = context->getEthernetPortByName(common::utility::getPlaneFromEthernetPortDN(p_objName), common::utility::getIdValueFromRdn(p_objName), true);

			if (ethernePort)
			{
				for (uint16_t i = 0; p_attrName[i] != 0; ++i)
				{
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Updating attribute <%s> on DN:<%s>", p_attrName[i], p_objName);

					if (ethernetPort_attribute::OPERATING_MODE.compare(p_attrName[i]) == 0)
					{
						int32_t operOperatingMode = ethernePort->getOperOperatingMode();

						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%i> for attribute <%s> on DN:<%s>", operOperatingMode, p_attrName[i], p_objName);

						ACS_CC_ImmParameter parameter;

						char tmpAttrName[64] = {0};
						strcpy(tmpAttrName, ethernetPort_attribute::OPERATING_MODE.c_str());
						parameter.attrName = tmpAttrName;
						parameter.attrType = ATTR_INT32T;
						parameter.attrValuesNum = (operOperatingMode != imm::OPER_MODE_UNKNOWN) ? 1U : 0;
						void *value[1] = {reinterpret_cast<void*>(&operOperatingMode)};
						parameter.attrValues = (operOperatingMode != imm::OPER_MODE_UNKNOWN) ? value : 0;

						modifyRuntimeObj(p_objName, &parameter);
					}
					else if (ethernetPort_attribute::AUTO_NEGOTIATION_STATE.compare(p_attrName[i]) == 0)
					{
						imm::EthAutoNegotiationState autoNegotiationState = ethernePort->getAutoNegotiationState();
						imm::oper_State imm_auto_neg_value;

						switch(autoNegotiationState)
						{
							case imm::AN_STATE_UNKNOWN:
								imm_auto_neg_value = imm::EMPTY_OPER_STATE;
								break;

							case  imm::AN_ENABLED:
								imm_auto_neg_value = imm::ENABLED;
								break;

							case  imm::AN_DISABLED:
								imm_auto_neg_value = imm::DISABLED;
								break;
						}

						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%d> for attribute <%s> on DN:<%s>", imm_auto_neg_value, p_attrName[i], p_objName);

						ACS_CC_ImmParameter parameter;

						char tmpAttrName[64] = {0};
						strcpy(tmpAttrName, ethernetPort_attribute::AUTO_NEGOTIATION_STATE.c_str());
						parameter.attrName = tmpAttrName;
						parameter.attrType = ATTR_INT32T;
						parameter.attrValuesNum = (imm_auto_neg_value != imm::EMPTY_OPER_STATE) ? 1U : 0;
						void *value[1] = {reinterpret_cast<void*>(&imm_auto_neg_value)};
						parameter.attrValues = (imm_auto_neg_value != imm::EMPTY_OPER_STATE) ? value : 0;

						modifyRuntimeObj(p_objName, &parameter);
					}
				}
			}
		}

		return result;
	}

	void EthernetPort_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_ethernetPortOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_ethernetPortOperationTable.erase(operationRange.first, operationRange.second);
	}

}


