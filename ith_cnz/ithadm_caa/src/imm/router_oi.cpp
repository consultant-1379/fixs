/*
 * router_oi.cpp
 *
 *  Created on: May 3, 2016
 *      Author: eanform
 */

#include "imm/router_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_transportBNChandler.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"


namespace imm
{

	Router_OI::Router_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	Router_OI::~Router_OI()
	{

	}

	ACS_CC_ReturnType Router_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		Router routerObj;
		routerObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == router_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string routerRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				routerObj.info.moDN.assign(routerRDN);
				routerObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				routerObj.info.name = common::utility::getIdValueFromRdn(routerObj.info.moDN);

				//get smxId
				routerObj.info.smxId = common::utility::getSmxIdFromRouterDN(routerObj.info.moDN);

				result = ACS_CC_SUCCESS;
				continue;
			}
		}

		// Operation is exclusive
		routerObj.exclusive = true;

		result = checkTransactionAugmentation(routerObj, oiHandle, ccbId);

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedul the operation
			operationTable_t::iterator element = m_routerOperationTable.insert(std::make_pair(ccbId, routerObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,ROUTER_T,&(element->second));
		}
		else
		{
			// Error
		}
		return result;
	}


	ACS_CC_ReturnType Router_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;
		Router routerObj;
		routerObj.action = Delete;
		routerObj.info.moDN.assign(objName);

		//get RDN value
		routerObj.info.name = common::utility::getIdValueFromRdn(routerObj.info.moDN);

		//get smxId value
		routerObj.info.smxId = common::utility::getSmxIdFromRouterDN(routerObj.info.moDN);

		// Operation is exclusive
		routerObj.exclusive = true;

		//add into the map and schedul the operation
		operationTable_t::iterator element = m_routerOperationTable.insert(std::make_pair(ccbId, routerObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,ROUTER_T,&(element->second));

		return result;
	}

	ACS_CC_ReturnType Router_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		UNUSED(ccbId);
		UNUSED(objName);
		UNUSED(attrMods);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		return result;
	}

	ACS_CC_ReturnType Router_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  Router_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Router_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Router_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}

	void Router_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_routerOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_routerOperationTable.erase(operationRange.first, operationRange.second);

	}

	ACS_CC_ReturnType Router_OI::checkTransactionAugmentation(const Router& routerObj, ACS_APGCC_OiHandle& oiHandle, ACS_APGCC_CcbId& ccbId)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		if (Create == routerObj.action)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Router augmentation object <%s>", routerObj.info.getDN());

			ACS_APGCC_CcbHandle ccbHandleVal;
			ACS_APGCC_AdminOwnerHandle adminOwnerHandlerVal;
			if ( (result = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnerHandlerVal)) != ACS_CC_SUCCESS)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "getCcbAugmentationInitialize failed. Error %d: <%s>", getInternalLastError(), getInternalLastErrorText());
				return result;
			}

			{ //Create RouteTableStatic
				/*The vector of attributes*/
				std::vector<ACS_CC_ValuesDefinitionType> attrList;
				/*the attributes*/
				ACS_CC_ValuesDefinitionType attrRdn;

				char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};

				::snprintf(rdnValue, IMM_RDN_PATH_SIZE_MAX, "%s=1", imm::routeTable_attribute::RDN.c_str());

				char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
				strncpy(attrdn, imm::routeTable_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
				attrRdn.attrName = attrdn;
				attrRdn.attrType = ATTR_STRINGT;
				attrRdn.attrValuesNum = 1;
				void *attrRdnvalue[1] = {reinterpret_cast<void*>(rdnValue)};
				attrRdn.attrValues = attrRdnvalue;

				attrList.push_back(attrRdn);

				if ( (result = createObjectAugmentCcb(ccbHandleVal, adminOwnerHandlerVal, imm::moc_name::CLASS_ROUTE_TABLE.c_str(), routerObj.info.getDN(), attrList))  != ACS_CC_SUCCESS)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObjectAugmentCcb failed for RDN %s. Error %d: <%s>", rdnValue, getInternalLastError(), getInternalLastErrorText());
					return result;
				}
			}

			{ //Create Loopback interface
				/*The vector of attributes*/
				std::vector<ACS_CC_ValuesDefinitionType> attrList;
				/*the attributes*/
				ACS_CC_ValuesDefinitionType attrRdn;
				ACS_CC_ValuesDefinitionType attrLoopback;
				ACS_CC_ValuesDefinitionType attrAdminState;

				char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};

				std::string loopbackIfName(common::utility::buildLoopbackInterfaceName(routerObj.info.getName()));
				::snprintf(rdnValue, IMM_RDN_PATH_SIZE_MAX, "%s=%s", imm::interfaceIpv4_attribute::RDN.c_str(), loopbackIfName.c_str());

				char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
				strncpy(attrdn, imm::interfaceIpv4_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
				attrRdn.attrName = attrdn;
				attrRdn.attrType = ATTR_STRINGT;
				attrRdn.attrValuesNum = 1;
				void *attrRdnvalue[1] = {reinterpret_cast<void*>(rdnValue)};
				attrRdn.attrValues = attrRdnvalue;

				char name_attrLoopback[IMM_ATTR_NAME_MAX_SIZE]= {0};
				strncpy(name_attrLoopback, imm::interfaceIpv4_attribute::LOOPBACK.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
				attrLoopback.attrName = name_attrLoopback;
				attrLoopback.attrType = ATTR_INT32T;
				attrLoopback.attrValuesNum = 1;
				int value_attrLoopback = 1;
				void *attrLoopbackvalue[1] = {reinterpret_cast<void*>(&value_attrLoopback)};
				attrLoopback.attrValues = attrLoopbackvalue;

				char name_attrAdminState[IMM_ATTR_NAME_MAX_SIZE]= {0};
				strncpy(name_attrAdminState, imm::interfaceIpv4_attribute::ADMIN_STATE.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
				attrAdminState.attrName = name_attrAdminState;
				attrAdminState.attrType = ATTR_INT32T;
				attrAdminState.attrValuesNum = 1;
				int value_attrAdminState = imm::UNLOCKED;
				void *attrAdminStatevalue[1] = {reinterpret_cast<void*>(&value_attrAdminState)};
				attrAdminState.attrValues = attrAdminStatevalue;

				attrList.push_back(attrRdn);
				attrList.push_back(attrLoopback);
				attrList.push_back(attrAdminState);

				if ( (result = createObjectAugmentCcb(ccbHandleVal, adminOwnerHandlerVal, imm::moc_name::CLASS_INTERFACE.c_str(), routerObj.info.getDN(), attrList))  !=  ACS_CC_SUCCESS)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObjectAugmentCcb failed for RDN %s. Error %d: <%s>", rdnValue, getInternalLastError(), getInternalLastErrorText());
					return result;
				}

			}

			{ //Create Address on Loopback interface
				/*The vector of attributes*/
				std::vector<ACS_CC_ValuesDefinitionType> attrList;
				/*the attributes*/
				ACS_CC_ValuesDefinitionType attrRdn;
				ACS_CC_ValuesDefinitionType attrAddress;

				char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
				char parentDN[IMM_DN_PATH_SIZE_MAX] = {0};

				std::string loopbackIfName(common::utility::buildLoopbackInterfaceName(routerObj.info.getName()));

				::snprintf(rdnValue, IMM_RDN_PATH_SIZE_MAX, "%s=1", imm::addressIpv4_attribute::RDN.c_str());
				::snprintf(parentDN, IMM_RDN_PATH_SIZE_MAX, "%s=%s,%s", imm::interfaceIpv4_attribute::RDN.c_str(), loopbackIfName.c_str(), routerObj.info.getDN());

				char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
				strncpy(attrdn, imm::addressIpv4_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
				attrRdn.attrName = attrdn;
				attrRdn.attrType = ATTR_STRINGT;
				attrRdn.attrValuesNum = 1;
				void *attrRdnvalue[1] = {reinterpret_cast<void*>(rdnValue)};
				attrRdn.attrValues = attrRdnvalue;

				char name_attrAddress[IMM_ATTR_NAME_MAX_SIZE]= {0};
				strncpy(name_attrAddress, imm::addressIpv4_attribute::ADDRESS.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
				attrAddress.attrName = name_attrAddress;
				attrAddress.attrType = ATTR_STRINGT;
				attrAddress.attrValuesNum = 1;
				char value_attrAddress[IMM_ATTR_NAME_MAX_SIZE] = {0};
				::snprintf(value_attrAddress,IMM_ATTR_NAME_MAX_SIZE,"127.0.0.1/8");
				void *attrAddressvalue[1] = {reinterpret_cast<void*>(value_attrAddress)};
				attrAddress.attrValues = attrAddressvalue;

				attrList.push_back(attrRdn);
				attrList.push_back(attrAddress);

				if ( (result = createObjectAugmentCcb(ccbHandleVal, adminOwnerHandlerVal, imm::moc_name::CLASS_ADDRESS.c_str(), parentDN, attrList))  !=  ACS_CC_SUCCESS)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObjectAugmentCcb failed for RDN %s. Error %d: <%s>", rdnValue, getInternalLastError(), getInternalLastErrorText());
					return result;
				}

			}

			if ( (result = applyAugmentCcb(ccbHandleVal)) != ACS_CC_SUCCESS)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "applyAugmentCcb failed. Error %d: <%s>", getInternalLastError(), getInternalLastErrorText());
				return result;
			}

		}

		return result;
	}

}




