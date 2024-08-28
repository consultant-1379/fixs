/*
 * aggregator.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */

#include "imm/aggregator_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "engine/context.h"
#include "fixs_ith_workingset.h"

namespace imm
{

	Aggregator_OI::Aggregator_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	Aggregator_OI::~Aggregator_OI()
	{

	}

	ACS_CC_ReturnType Aggregator_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		Aggregator aggregatorObj;
		aggregatorObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == aggregator_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string aggregatorRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				aggregatorObj.info.moDN.assign(aggregatorRDN);
				aggregatorObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				aggregatorObj.info.name = common::utility::getIdValueFromRdn(aggregatorObj.info.moDN);

				//get smxId and plane value
				aggregatorObj.info.smxId = common::utility::getSmxIdFromAggregatorDN(aggregatorObj.info.moDN);
				aggregatorObj.info.plane = common::utility::getPlaneFromAggregatorDN(aggregatorObj.info.moDN);

				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == aggregator_attribute::AGG_MEMBER.compare(attr[idx]->attrName) )
			{
				aggregatorObj.info.aggMember.clear();
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					aggregatorObj.info.aggMember.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
				}

				continue;
			}

//			if( 0 == aggregator_attribute::DATA_RATE.compare(attr[idx]->attrName) )
//			{
//				if (0U != attr[idx]->attrValuesNum)
//				{
//					aggregatorObj.info.dataRate = *reinterpret_cast<uint32_t *>(attr[idx]->attrValues[0]);
//				}
//				continue;
//			}

			//check for SNMP index attribute
			if( 0 == aggregator_attribute::SNMP_INDEX.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					aggregatorObj.info.snmpIndex = *(reinterpret_cast<uint16_t *>(attr[idx]->attrValues[0]));
				}
				continue;
			}

		}

		// Operation is exclusive
		aggregatorObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedul the operation
			operationTable_t::iterator element = m_aggregatorOperationTable.insert(std::make_pair(ccbId, aggregatorObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,AGGREGATOR_T,&(element->second));
		}
		else
		{
			// Error
		}
		return result;
	}


	ACS_CC_ReturnType Aggregator_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		Aggregator aggregatorObj;
		aggregatorObj.action = Delete;
		aggregatorObj.info.moDN.assign(objName);

		//get RDN value
		aggregatorObj.info.name = common::utility::getIdValueFromRdn(aggregatorObj.info.moDN);

		//get smxId and plane value
		aggregatorObj.info.smxId = common::utility::getSmxIdFromAggregatorDN(aggregatorObj.info.moDN);
		aggregatorObj.info.plane = common::utility::getPlaneFromAggregatorDN(aggregatorObj.info.moDN);

		// Operation is exclusive
		aggregatorObj.exclusive = true;

		//add into the map and schedul the operation
		operationTable_t::iterator element = m_aggregatorOperationTable.insert(std::make_pair(ccbId, aggregatorObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,AGGREGATOR_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType Aggregator_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);


		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		Aggregator aggregatorObj;
		aggregatorObj.action = Modify;
		aggregatorObj.info.moDN.assign(objName);

		//get RDN value
		aggregatorObj.info.name = common::utility::getIdValueFromRdn(aggregatorObj.info.moDN);

		//get smxId and plane value
		aggregatorObj.info.smxId = common::utility::getSmxIdFromAggregatorDN(aggregatorObj.info.moDN);
		aggregatorObj.info.plane = common::utility::getPlaneFromAggregatorDN(aggregatorObj.info.moDN);

		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			ACS_APGCC_AttrValues modAttribute = attrMods[idx]->modAttr;

			if( 0 == aggregator_attribute::AGG_MEMBER.compare(modAttribute.attrName) )
			{
				aggregatorObj.info.changeMask |= imm::aggregator_attribute::ADMIN_AGG_MEMBER_CHANGE;

				aggregatorObj.info.aggMember.clear();
				for (uint32_t j = 0; j < modAttribute.attrValuesNum; j++)
				{
					aggregatorObj.info.aggMember.insert(reinterpret_cast<char *>(modAttribute.attrValues[j]));
				}

				continue;
			}

			//check for SNMP index attribute
			if( 0 == aggregator_attribute::SNMP_INDEX.compare(modAttribute.attrName) )
			{
				aggregatorObj.info.changeMask |= imm::aggregator_attribute::SNMP_INDEX_CHANGE;

				if (0U != modAttribute.attrValuesNum)
				{
					aggregatorObj.info.snmpIndex = *(reinterpret_cast<uint16_t *>(modAttribute.attrValues[0]));
				}
				continue;
			}
		}

		// Operation is exclusive
		aggregatorObj.exclusive = true;

		if (ACS_CC_SUCCESS == result)
		{
			if (aggregatorObj.info.changeMask)
			{
				//add into the map and schedul the operation
				operationTable_t::iterator element = m_aggregatorOperationTable.insert(std::make_pair(ccbId, aggregatorObj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,AGGREGATOR_T,&(element->second));
			}
		}
		else
		{
			//Error
		}

		return result;
	}

	ACS_CC_ReturnType Aggregator_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  Aggregator_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Aggregator_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Aggregator_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}

	ACS_CC_ReturnType Aggregator_OI::updateRuntime(const char* p_objName, const char** p_attrName)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		engine::contextAccess_t access(common::utility::getSmxIdFromAggregatorDN(p_objName), engine::GET_EXISTING, engine::SHARED_ACCESS);

		engine::Context* context = access.getContext();

		if (context)
		{
			boost::shared_ptr<engine::Aggregator> aggregator = context->getAggregatorByName(common::utility::getPlaneFromAggregatorDN(p_objName), common::utility::getIdValueFromRdn(p_objName), true);

			if (aggregator)
			{
				for (uint16_t i = 0; p_attrName[i] != 0; ++i)
				{
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Updating attribute <%s> on DN:<%s>", p_attrName[i], p_objName);

					if ( aggregator_attribute::OPER_STATE.compare(p_attrName[i]) == 0)
					{
						imm::oper_State operState = aggregator->getOperState();

						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%d> for attribute <%s> on DN:<%s>",operState, p_attrName[i], p_objName);

						ACS_CC_ImmParameter operStateParameter;

						char tmpAttrName[64] = {0};
						strcpy(tmpAttrName,  aggregator_attribute::OPER_STATE.c_str());
						operStateParameter.attrName = tmpAttrName;
						operStateParameter.attrType = ATTR_INT32T;
						operStateParameter.attrValuesNum = (operState != imm::EMPTY_OPER_STATE)? 1U: 0;
						void *value[1] = {reinterpret_cast<void*>(&operState)};
						operStateParameter.attrValues = (operState != imm::EMPTY_OPER_STATE)? value: 0;

						modifyRuntimeObj(p_objName, &operStateParameter);
					}

					else if ( aggregator_attribute::OPER_AGG_MEMBER.compare(p_attrName[i]) == 0)
					{
						std::set<std::string> operAggMember = aggregator->getOperAggMemberDN();

						ACS_CC_ImmParameter parameter;

						char tmpAttrName[64] = {0};
						strcpy(tmpAttrName,  aggregator_attribute::OPER_AGG_MEMBER.c_str());
						parameter.attrName = tmpAttrName;
						parameter.attrType = ATTR_NAMET;
						parameter.attrValuesNum = operAggMember.size();

						void **value = new void*[operAggMember.size()];

						std::vector<char*> tmpValue(operAggMember.size(), 0);



						uint16_t j = 0;
						for (std::set<std::string>::iterator it = operAggMember.begin(); it != operAggMember.end();  ++it )
						{
							tmpValue[j] = new char[512];
							memset(tmpValue[j], 0, 512);
							strcpy(tmpValue[j],it->c_str());

							FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%s> for attribute <%s> on DN:<%s>", it->c_str(), p_attrName[i], p_objName);
							value[j] = reinterpret_cast<void*>(tmpValue[j]);
							++j;
						}

						parameter.attrValues = value;


						modifyRuntimeObj(p_objName, &parameter);

						for (std::vector<char*>::iterator it = tmpValue.begin(); it != tmpValue.end(); ++it)
						{
							delete [] *it;
						}
						delete [] value;

					}
					else if ( aggregator_attribute::DATA_RATE.compare(p_attrName[i]) == 0)
					{
						uint32_t dataRate = aggregator->getDataRate();

						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%d> for attribute <%s> on DN:<%s>",dataRate, p_attrName[i], p_objName);

						ACS_CC_ImmParameter dataRateParameter;

						char tmpAttrName[64] = {0};
						strcpy(tmpAttrName,  aggregator_attribute::DATA_RATE.c_str());
						dataRateParameter.attrName = tmpAttrName;
						dataRateParameter.attrType = ATTR_UINT32T;
						dataRateParameter.attrValuesNum = 1U;
						void *value[1] = {reinterpret_cast<void*>(&dataRate)};
						dataRateParameter.attrValues = value;

						modifyRuntimeObj(p_objName, &dataRateParameter);
					}

				}
			}
		}

		return result;
	}

	void Aggregator_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_aggregatorOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_aggregatorOperationTable.erase(operationRange.first, operationRange.second);

	}

}



