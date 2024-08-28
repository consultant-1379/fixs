/*
 * bridgePort.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */

#include "imm/bridgePort_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "engine/context.h"
#include "fixs_ith_workingset.h"

namespace imm
{

	BridgePort_OI::BridgePort_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	BridgePort_OI::~BridgePort_OI()
	{

	}

	ACS_CC_ReturnType BridgePort_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		BridgePort bridgePortObj;
		bridgePortObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == bridgePort_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string bridgePortRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				bridgePortObj.info.moDN.assign(bridgePortRDN);
				bridgePortObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				bridgePortObj.info.name = common::utility::getIdValueFromRdn(bridgePortObj.info.moDN);

				//get smxId and plane value
				bridgePortObj.info.smxId = common::utility::getSmxIdFromBridgeDN(parentname);
				bridgePortObj.info.plane = common::utility::getPlaneFromBridgeDN(parentname);

				result = ACS_CC_SUCCESS;
				continue;
			}

			// check for admin state attribute
			if( 0 == bridgePort_attribute::ADM_STATE.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					bridgePortObj.info.adminstate = *reinterpret_cast<admin_State*>(attr[idx]->attrValues[0]);
					bridgePortObj.info.changeMask |= bridgePort_attribute::ADM_STATE_CHANGE;
				}
				continue;
			}

			if( 0 == bridgePort_attribute::DEFAULT_VLAN_ID.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					bridgePortObj.info.defaultVlanId = *reinterpret_cast<uint16_t*>(attr[idx]->attrValues[0]);
					bridgePortObj.info.changeMask |= bridgePort_attribute::DEFAULT_VLAN_ID_CHANGE;
				}
				continue;
			}

			if( 0 == bridgePort_attribute::PORT_ADDRESS.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					bridgePortObj.info.portAddress = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
					bridgePortObj.info.changeMask |= bridgePort_attribute::PORT_ADDRESS_CHANGE;
				}

				continue;
			}

		}

		// Operation is exclusive
		bridgePortObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedul the operation
			operationTable_t::iterator element = m_bridgePortOperationTable.insert(std::make_pair(ccbId, bridgePortObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,BRIDGEPORT_T,&(element->second));
		}
		else
		{
			// Error
		}
		return result;
	}


	ACS_CC_ReturnType BridgePort_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		BridgePort bridgePortObj;
		bridgePortObj.action = Delete;
		bridgePortObj.info.moDN.assign(objName);

		//get RDN value
		bridgePortObj.info.name = common::utility::getIdValueFromRdn(bridgePortObj.info.moDN);

		//get smxId and plane value
		bridgePortObj.info.smxId = common::utility::getSmxIdFromBridgeDN(common::utility::getParentDN(bridgePortObj.info.moDN));
		bridgePortObj.info.plane = common::utility::getPlaneFromBridgeDN(common::utility::getParentDN(bridgePortObj.info.moDN));

		// Operation is exclusive
		bridgePortObj.exclusive = true;

		//add into the map and schedul the operation
		operationTable_t::iterator element = m_bridgePortOperationTable.insert(std::make_pair(ccbId, bridgePortObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,BRIDGEPORT_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType BridgePort_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		UNUSED(attrMods);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		BridgePort bridgePortObj;
		bridgePortObj.action = Modify;
		bridgePortObj.info.moDN.assign(objName);

		//get RDN value
		bridgePortObj.info.name = common::utility::getIdValueFromRdn(bridgePortObj.info.moDN);

		//get smxId and plane value
		bridgePortObj.info.smxId = common::utility::getSmxIdFromBridgeDN(common::utility::getParentDN(bridgePortObj.info.moDN));
		bridgePortObj.info.plane = common::utility::getPlaneFromBridgeDN(common::utility::getParentDN(bridgePortObj.info.moDN));

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;
			// check for vid attribute
			if( 0 == bridgePort_attribute::ADM_STATE.compare(modAttribute.attrName) )
			{
				bridgePortObj.info.changeMask |= bridgePort_attribute::ADM_STATE_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					bridgePortObj.info.adminstate = *reinterpret_cast<admin_State*>(modAttribute.attrValues[0]);
				}
				continue;
			}

			if( 0 == bridgePort_attribute::DEFAULT_VLAN_ID.compare(modAttribute.attrName) )
			{
				bridgePortObj.info.changeMask |= bridgePort_attribute::DEFAULT_VLAN_ID_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					bridgePortObj.info.defaultVlanId = *reinterpret_cast<uint16_t*>(modAttribute.attrValues[0]);
				}
				continue;
			}

			if( 0 == bridgePort_attribute::PORT_ADDRESS.compare(modAttribute.attrName) )
			{
				bridgePortObj.info.changeMask |= bridgePort_attribute::PORT_ADDRESS_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					bridgePortObj.info.portAddress = reinterpret_cast<char *>(modAttribute.attrValues[0]);
				}
				continue;
			}

		}

		if (bridgePortObj.info.changeMask & (bridgePort_attribute::DEFAULT_VLAN_ID_CHANGE | bridgePort_attribute::ADM_STATE_CHANGE))
		{
			// Operation is exclusive if we changed default VLAN ID or adminState
			bridgePortObj.exclusive = true;
		}


		result = checkTransactionAugmentation(bridgePortObj, oiHandle, ccbId);

		if (ACS_CC_SUCCESS == result)
		{
			if (bridgePortObj.info.changeMask)
			{
				//add into the map and schedul the operation
				operationTable_t::iterator element = m_bridgePortOperationTable.insert(std::make_pair(ccbId, bridgePortObj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,BRIDGEPORT_T,&(element->second));
			}
		}
		else
		{
			//Error
		}

		return result;
	}

	ACS_CC_ReturnType BridgePort_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  BridgePort_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void BridgePort_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void BridgePort_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}

	ACS_CC_ReturnType BridgePort_OI::updateRuntime(const char* p_objName, const char** p_attrName)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		engine::contextAccess_t access(common::utility::getSmxIdFromBridgePortDN(p_objName), engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = access.getContext();


		if (context)
		{
			boost::shared_ptr<engine::BridgePort> bridgePort = context->getBridgePortByName(common::utility::getPlaneFromBridgePortDN(p_objName), common::utility::getIdValueFromRdn(p_objName), true);

			if (bridgePort)
			{
				for (uint16_t i = 0; p_attrName[i] != 0; ++i)
				{
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Updating attribute <%s> on DN:<%s>", p_attrName[i], p_objName);

					if ( bridgePort_attribute::OPER_STATE.compare(p_attrName[i]) == 0)
					{
						imm::oper_State operState = bridgePort->getOperState();

						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%d> for attribute <%s> on DN:<%s>",operState, p_attrName[i], p_objName);

						ACS_CC_ImmParameter operStateParameter;

						char tmpAttrName[64] = {0};
						strcpy(tmpAttrName,  bridgePort_attribute::OPER_STATE.c_str());
						operStateParameter.attrName = tmpAttrName;
						operStateParameter.attrType = ATTR_INT32T;
						operStateParameter.attrValuesNum = (operState != imm::EMPTY_OPER_STATE)? 1U: 0;
						void *value[1] = {reinterpret_cast<void*>(&operState)};
						operStateParameter.attrValues = (operState != imm::EMPTY_OPER_STATE)? value: 0;

						modifyRuntimeObj(p_objName, &operStateParameter);
					}
				}
			}
		}

		return result;
	}

	void BridgePort_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_bridgePortOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_bridgePortOperationTable.erase(operationRange.first, operationRange.second);
	}

	ACS_CC_ReturnType BridgePort_OI::checkTransactionAugmentation(const BridgePort& bridgePortObj, ACS_APGCC_OiHandle& oiHandle, ACS_APGCC_CcbId& ccbId)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		if ( (Modify == bridgePortObj.action) &&
				(bridgePortObj.info.changeMask & bridgePort_attribute::ADM_STATE_CHANGE) &&
				(imm::LOCKED == bridgePortObj.info.getAdminState()))
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BridgePort augmentation might be needed for object <%s>", bridgePortObj.info.getBridgePortDN());

			engine::contextAccess_t access(bridgePortObj.info.getSmxId(), engine::GET_EXISTING, engine::SHARED_ACCESS);
			engine::Context* context = access.getContext();

			if (engine::CONTEXT_ACCESS_ACQUIRED == access.getAccessResult() && context)
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Context Acquired");
				boost::shared_ptr<engine::BridgePort> bridgePort = context->getBridgePortByName(bridgePortObj.info.getPlane(), bridgePortObj.info.getName());

				if (0 != bridgePort && bridgePort->isAggregator())
				{
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BridgePort <%s> is aggregated. Augmentation is required", bridgePortObj.info.getBridgePortDN());

					std::vector<BridgePort> memberBridgePorts;
					std::string portName = common::utility::buildPortName(bridgePortObj.info.getSmxId(), bridgePortObj.info.getPlane(), bridgePortObj.info.getName());
					boost::shared_ptr<engine::Aggregator> aggregator = context->getAggregatorByName(bridgePortObj.info.getPlane(), portName);

					if (0 != aggregator)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Aggregator <%s> found", aggregator->getName());

						std::set<uint16_t> aggMemberIndex = aggregator->getAdminAggMemebrIndex();

						for (std::set<uint16_t>::iterator index = aggMemberIndex.begin(); index != aggMemberIndex.end(); ++index)
						{
							boost::shared_ptr<engine::BridgePort> aggMember = context->getBridgePortByIndex(bridgePortObj.info.getPlane(), *index);
							if (0 != aggMember)
							{
								BridgePort memberData;
								memberData.action = Modify;
								memberData.info.smxId = bridgePortObj.info.getSmxId();
								memberData.info.plane = bridgePortObj.info.getPlane();
								memberData.info.name.assign(aggMember->getName());
								memberData.info.moDN.assign(aggMember->getBridgePortDN());

								memberBridgePorts.push_back(memberData);
							}
						}

						if (!memberBridgePorts.empty())
						{
							ACS_APGCC_CcbHandle ccbHandleVal;
							ACS_APGCC_AdminOwnerHandle adminOwnerHandlerVal;
							if ( (result = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnerHandlerVal)) != ACS_CC_SUCCESS)
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR, "getCcbAugmentationInitialize failed. Error %d: <%s>", getInternalLastError(), getInternalLastErrorText());
								return result;
							}

							for (std::vector<BridgePort>::iterator memberIterator = memberBridgePorts.begin(); memberIterator != memberBridgePorts.end(); ++memberIterator)
							{
								ACS_CC_ImmParameter admStateParameter;

								char tmpAttrName[64] = {0};
								strcpy(tmpAttrName,  imm::bridgePort_attribute::ADM_STATE.c_str());
								admStateParameter.attrName = tmpAttrName;
								admStateParameter.attrType = ATTR_INT32T;
								admStateParameter.attrValuesNum = 1;
								imm::admin_State admState = bridgePortObj.info.getAdminState();
								void *value[1] = {reinterpret_cast<void*>(&admState)};
								admStateParameter.attrValues = value;

								FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Modifying attribute <%s> for object <%s>", admStateParameter.attrName, memberIterator->info.getBridgePortDN());

								if ( (result = modifyObjectAugmentCcb(ccbHandleVal, adminOwnerHandlerVal, memberIterator->info.getBridgePortDN(), &admStateParameter)) != ACS_CC_SUCCESS)
								{
									FIXS_ITH_LOG(LOG_LEVEL_ERROR, "modifyObjectAugmentCcb failed for DN %s. Error %d: <%s>", memberIterator->info.getBridgePortDN(), getInternalLastError(), getInternalLastErrorText());
									return result;
								}

								//Since we won't receive the callback for this, we need to schedule the operation as well
								memberIterator->info.changeMask = bridgePort_attribute::ADM_STATE_CHANGE;
								memberIterator->info.adminstate = bridgePortObj.info.getAdminState();

								//add into the map and schedule the operation
								operationTable_t::iterator element = m_bridgePortOperationTable.insert(std::make_pair(ccbId, *memberIterator));
								configurationHelper_t::instance()->scheduleModifyOperation(ccbId,BRIDGEPORT_T,&(element->second));

							}

							if ( (result = applyAugmentCcb(ccbHandleVal)) != ACS_CC_SUCCESS)
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR, "applyAugmentCcb failed. Error %d: <%s>", getInternalLastError(), getInternalLastErrorText());
								return result;
							}
						}
					}
				}
			}
		}

		return result;
	}

}


