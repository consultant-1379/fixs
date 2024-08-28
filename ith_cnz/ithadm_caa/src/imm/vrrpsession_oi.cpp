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
 *  Created on: Jun 10, 2016
 *      Author: xvincon
 */

#include "imm/vrrpsession_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_programconstants.h"

namespace imm
{

	VrrpSession_OI::VrrpSession_OI()
	: Base_OI(imm::moc_name::CLASS_VRRP_SESSION, imm::oi_name::IMPL_VRRPSESSION),
	  m_vrrpSessionOperationTable()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpSession_OI created");
	}

	VrrpSession_OI::~VrrpSession_OI()
	{

	}

	ACS_CC_ReturnType VrrpSession_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		VrrpSession vrrpSessionObj;
		vrrpSessionObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == vrrpSession_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string sessionDN( reinterpret_cast<char *>(attr[idx]->attrValues[0]));
				sessionDN.push_back(common::utility::parserTag::comma);
				sessionDN.append(parentname);

				vrrpSessionObj.info.moDN.assign(sessionDN);

				//get RDN value
				vrrpSessionObj.info.name.assign(common::utility::getIdValueFromRdn(sessionDN));

				//get smxId
				vrrpSessionObj.info.smxId = common::utility::getSmxIdFromVrrpv3SessionDN(sessionDN);

				continue;
			}

			if( 0 == vrrpSession_attribute::PRIO_SESSION.compare(attr[idx]->attrName) )
			{
				// get prioritizedSession value
				int value = *reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]);
				vrrpSessionObj.info.prioritizedSession = ( 0U == value ? VRRP_TRUE : VRRP_FALSE);

				vrrpSessionObj.info.changeMask |= imm::vrrpSession_attribute::PRIO_SESSION_CHANGE;
				continue;
			}

			if( 0 == vrrpSession_attribute::VRRP_INTERFACE_REF.compare(attr[idx]->attrName) )
			{
				//get vrrpInterface DN
				vrrpSessionObj.info.vrrpv3InterfaceRef.assign( reinterpret_cast<char*>(attr[idx]->attrValues[0]) );

				vrrpSessionObj.info.changeMask |= imm::vrrpSession_attribute::VRRP_INTERFACE_REF_CHANGE;
				continue;
			}

			if( 0 == vrrpSession_attribute::ADMIN_STATE.compare(attr[idx]->attrName) )
			{
				// get Administrative State value
				int value = *reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]);

				vrrpSessionObj.info.administrativeState = ( 0U == value ? LOCKED : UNLOCKED );

				vrrpSessionObj.info.changeMask |= imm::vrrpSession_attribute::ADMIN_STATE_CHANGE;
				continue;
			}

		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CREATE VRRP SESSION : \nDN:<%s>"
				"\nSMX ID:<%s>"
				"\nVRRP INTERFACE DN:<%s>"
				"\nPRIORITIZED SESSION:<%i>"
				"\nSTATE:<%i>",
				vrrpSessionObj.info.getDN(), vrrpSessionObj.info.getSmxId(), vrrpSessionObj.info.getVrrpInterfaceRef(), vrrpSessionObj.info.getPrioritizedSession(), vrrpSessionObj.info.getAdminState());

		// Operation is exclusive
		vrrpSessionObj.exclusive = true;

		if( checkTransactionAugmentation(vrrpSessionObj, oiHandle, ccbId) )
		{
			result = ACS_CC_SUCCESS;
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_vrrpSessionOperationTable.insert(std::make_pair(ccbId, vrrpSessionObj));
			configurationHelper_t::instance()->scheduleCreateOperation( ccbId, VRRPSESSION_T, &(element->second));
		}

		return result;
	}

	ACS_CC_ReturnType VrrpSession_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		FIXS_ITH_LOG(LOG_LEVEL_INFO,"Delete VRRP SESSION :"
						"\nDN:<%s>"
						, objName);

		VrrpSession vrrpSessionObj;
		vrrpSessionObj.action = Delete;
		vrrpSessionObj.info.moDN.assign(objName);

		//get RDN value
		vrrpSessionObj.info.name.assign(common::utility::getVrrpSessionNameFromDN(vrrpSessionObj.info.moDN));

		fixs_ith::ErrorConstants getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getVrrpInterfaceDNbyVrrpSessionDN(vrrpSessionObj.info.moDN, vrrpSessionObj.info.vrrpv3InterfaceRef);

		if(fixs_ith::ERR_NO_ERRORS != getResult)
		{
			setExitCode(static_cast<int>(fixs_ith::ERR_IMM_DELETE_OBJ), imm::comcli_errormessage::GENERIC);
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get VRRP INterface for vrrpSession:<%s>", vrrpSessionObj.info.moDN.c_str() );
			return result;
		}

		//get smxId and plane value
		vrrpSessionObj.info.smxId.assign(common::utility::getSmxIdFromVrrpv3SessionDN(vrrpSessionObj.info.moDN));

		//Ignore augmentation result in order to get the correct error code from the ConfigurationManager
		if( checkTransactionAugmentation(vrrpSessionObj, oiHandle, ccbId) )
		{
			result = ACS_CC_SUCCESS;
			// Operation is exclusive
			vrrpSessionObj.exclusive = true;

			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Delete VRRP SESSION schedule:<%d>", ccbId);
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_vrrpSessionOperationTable.insert(std::make_pair(ccbId, vrrpSessionObj));
			configurationHelper_t::instance()->scheduleDeleteOperation(ccbId, VRRPSESSION_T, &(element->second));
		}

		return result;
	}

	ACS_CC_ReturnType VrrpSession_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"MODIFY VRRP SESSION"
				"\nDN:<%s>"
				, objName);
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		VrrpSession vrrpSessionObj;
		vrrpSessionObj.action = Modify;
		vrrpSessionObj.info.moDN.assign(objName);

		//get Vrrp Session name
		vrrpSessionObj.info.name.assign(common::utility::getVrrpSessionNameFromDN(vrrpSessionObj.info.moDN));

		//get smxId and plane value
		vrrpSessionObj.info.smxId.assign(common::utility::getSmxIdFromVrrpv3SessionDN(vrrpSessionObj.info.moDN));

		// extract the attributes to modify
		for(size_t idx = 0U; NULL != attrMods[idx]; ++idx)
		{
			if( 0 == vrrpSession_attribute::ADMIN_STATE.compare(attrMods[idx]->modAttr.attrName) )
			{
				if(0U == attrMods[idx]->modAttr.attrValuesNum)
				{
					setExitCode(static_cast<int>(fixs_ith::ERR_IMM_MODIFY_OBJ), imm::comcli_errormessage::GENERIC);
					result = ACS_CC_FAILURE;
					break;
				}

				// get Administrative State value
				int value = *reinterpret_cast<int32_t*>(attrMods[idx]->modAttr.attrValues[0]);

				vrrpSessionObj.info.administrativeState = ( 0U == value ? LOCKED : UNLOCKED );

				vrrpSessionObj.info.changeMask |= imm::vrrpSession_attribute::ADMIN_STATE_CHANGE;
				continue;
			}
		}

		// Operation is exclusive
		vrrpSessionObj.exclusive = true;

		if( (vrrpSessionObj.info.changeMask) && (ACS_CC_SUCCESS == result))
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_vrrpSessionOperationTable.insert(std::make_pair(ccbId, vrrpSessionObj));
			configurationHelper_t::instance()->scheduleModifyOperation(ccbId, VRRPSESSION_T, &(element->second));
		}

		return result;
	}

	ACS_CC_ReturnType VrrpSession_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		fixs_ith::ErrorConstants operationResult = configurationHelper_t::instance()->executeMoOperation(ccbId);

		if (fixs_ith::ERR_NO_ERRORS != operationResult)
		{
			// Error
			result = ACS_CC_FAILURE;
			setExitCode(operationResult, configurationHelper_t::instance()->getErrorText(operationResult) );
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"COMPLETE VRRP SESSION Operation, result:<%d>", operationResult);
		return result;
	}

	void  VrrpSession_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void VrrpSession_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	ACS_CC_ReturnType VrrpSession_OI::updateRuntime(const char* p_objName, const char** p_attrName)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		for (uint16_t i = 0; p_attrName[i] != 0; ++i)
		{
			if ( vrrpSession_attribute::VRRP_STATE.compare(p_attrName[i]) == 0)
			{
				operation::vrrpSessionInfo vrrpSessionInfo;
				fixs_ith::ErrorConstants getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getVrrpSessionInfo(p_objName, vrrpSessionInfo);

				if(fixs_ith::ERR_NO_ERRORS == getResult)
				{
					imm::vrrpSessionState vrrpState = vrrpSessionInfo.getVrrpState();

					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%d> for attribute <%s> on DN:<%s>",
							vrrpState, vrrpSession_attribute::VRRP_STATE.c_str(), p_objName);

					ACS_CC_ImmParameter vrrpStateParameter;

					char tmpAttrName[64] = {0};
					strcpy(tmpAttrName,  vrrpSession_attribute::VRRP_STATE.c_str());
					vrrpStateParameter.attrName = tmpAttrName;
					vrrpStateParameter.attrType = ATTR_INT32T;
					vrrpStateParameter.attrValuesNum = 1U;
					void *value[1] = { reinterpret_cast<void*>(&vrrpState)};
					vrrpStateParameter.attrValues = value;

					modifyRuntimeObj(p_objName, &vrrpStateParameter);
				}
			}
		}

		return result;
	}

	bool VrrpSession_OI::checkTransactionAugmentation(VrrpSession& vrrpSessionObj, ACS_APGCC_OiHandle& oiHandle, ACS_APGCC_CcbId& ccbId)
	{
		bool result = false;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Augmentation on VRRP Session:<%s>", vrrpSessionObj.info.getName());

		std::set<std::string> vrrpSessionsDN;

		if( getVrrpSessionsDN(vrrpSessionObj, vrrpSessionsDN) &&
			( (Create != vrrpSessionObj.action ) ||	calculateVrrpSessionPriority(vrrpSessionObj) ) )
		{
			//Ignore augmentation result in order to get the correct error code from the ConfigurationManager
			result = true;

			ACS_APGCC_CcbHandle ccbHandleVal;
			ACS_APGCC_AdminOwnerHandle adminOwnerHandlerVal;

			ACS_CC_ReturnType immResult = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnerHandlerVal);

			if ( ACS_CC_SUCCESS !=  immResult)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "getCcbAugmentationInitialize failed on VrrpSession:<%s>, Error %d: <%s>", vrrpSessionObj.info.getDN(), getInternalLastError(), getInternalLastErrorText());
				return immResult;
			}

			immResult = modifyReservedByAttribute(ccbHandleVal, adminOwnerHandlerVal, vrrpSessionObj.info.getVrrpInterfaceRef(), vrrpSessionsDN);

			if(Create == vrrpSessionObj.action )
			{
				immResult = modifyPriorityAttribute(ccbHandleVal, adminOwnerHandlerVal, vrrpSessionObj.info.getDN(), vrrpSessionObj.info.getPriority());
			}

			immResult = applyAugmentCcb(ccbHandleVal);

			if( ACS_CC_SUCCESS != immResult)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "applyAugmentCcb failed on VrrpSession:<%s>, Error %d: <%s>", vrrpSessionObj.info.getDN(), getInternalLastError(), getInternalLastErrorText());
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Augmentation on VRRP Session:<%s> result:<%s>", vrrpSessionObj.info.getName(), common::utility::boolToString(result) );

		return result;
	}

	bool VrrpSession_OI::getVrrpSessionsDN(const VrrpSession& vrrpSessionObj, std::set<std::string>& vrrpSessionsDN)
	{
		bool result = true;

		std::string vrrpInterfaceName = common::utility::getVrrpInstanceNameFromDN(vrrpSessionObj.info.getVrrpInterfaceRef());

		fixs_ith::ErrorConstants getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getVrrpSessionsOfVrrpInterface(vrrpInterfaceName, vrrpSessionsDN);

		if(fixs_ith::ERR_NO_ERRORS != getResult)
		{
			result = false;
			setExitCode(static_cast<int>(fixs_ith::ERR_CONFIG_INVALID_VRRP_INTERFACE_NAME), imm::comcli_errormessage::INVALID_VRRP_INTERFACE_NAME);
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get vrrpSessions for vrrpInterface:<%s>", vrrpInterfaceName.c_str() );
			return result;
		}

		switch (vrrpSessionObj.action)
		{
			case Create:
			{
				// add the current VrrpSession DN to the previous ones
				result = checkAndAddVrrpSessionDN(vrrpSessionObj.info.moDN, vrrpSessionsDN);
			}
			break;

			case Delete:
			{
				// remove the current VrrpSession DN to the previous ones
				vrrpSessionsDN.erase(vrrpSessionObj.info.moDN);
			}
			break;

			case Modify:
			default:
				result = false;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Invalid action on VrrpSession:<%s>", vrrpSessionObj.info.getDN());
		}

		return result;
	}

	bool VrrpSession_OI::checkAndAddVrrpSessionDN(const std::string& vrrpSessionDN, std::set<std::string>& vrrpSessionsDN)
	{
		std::string routerDN;
		bool result =  common::utility::getRouterDn(vrrpSessionDN, routerDN);

		std::set<std::string>::const_iterator vrrpSessionDNIterator;

		for(vrrpSessionDNIterator = vrrpSessionsDN.begin(); (true == result) && vrrpSessionsDN.end() != vrrpSessionDNIterator; ++vrrpSessionDNIterator)
		{
			std::string otherRouterDN;
			result = common::utility::getRouterDn(*vrrpSessionDNIterator, otherRouterDN);

			if( result )
			{
				// check if already used on this router
				if( routerDN.compare(otherRouterDN) == 0 )
				{
					result = false;
					setExitCode(static_cast<int>(fixs_ith::ERR_IMM_CREATE_OBJ), imm::comcli_errormessage::CONFLICTING_VRRPSESSION);
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "VRRP INTERFACE already used on this router from the VRRP Session<%s>", vrrpSessionDNIterator->c_str() );
					return result;
				}
			}
		}

		if(result)
		{
			// add the current VrrpSession DN to the previous ones
			vrrpSessionsDN.insert(vrrpSessionDN);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added to Reserved By list VRRP Session\nDN<%s>", vrrpSessionDN.c_str());
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get RouterDN from VRRP Session DN");
			setExitCode(static_cast<int>(fixs_ith::ERR_IMM_CREATE_OBJ), imm::comcli_errormessage::GENERIC);
		}

		return result;
	}

	bool VrrpSession_OI::calculateVrrpSessionPriority(VrrpSession& vrrpSessionObj)
	{
		bool result = true;

		std::string vrrpAddress;
		std::string interfaceAddress;

		std::string vrrpInterfaceName = common::utility::getVrrpInstanceNameFromDN(vrrpSessionObj.info.getVrrpInterfaceRef());

		fixs_ith::ErrorConstants getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getIPv4AddressOfVrrpInterface(vrrpInterfaceName, vrrpAddress);

		if(fixs_ith::ERR_NO_ERRORS != getResult)
		{
			result = false;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Address for vrrpInterface:<%s>", vrrpInterfaceName.c_str() );
			setExitCode(getResult, imm::comcli_errormessage::MISSING_VRRP_ADDRESS);
			return result;
		}

		getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getIPv4AddressOfInterface(vrrpSessionObj.info, interfaceAddress);

		if(fixs_ith::ERR_NO_ERRORS != getResult)
		{
			result = false;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Address for VrrpSession:<%s>", vrrpSessionObj.info.getDN() );
			setExitCode(getResult, imm::comcli_errormessage::MISSING_INTERFACE_ADDRESS);
			return result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpInteface Address:<%s>, Interface Address:<%s>", vrrpAddress.c_str(), interfaceAddress.c_str() );

		vrrpSessionObj.info.changeMask |= imm::vrrpSession_attribute::PRIORITY_CHANGE;

		if( compareIPv4Address(interfaceAddress, vrrpAddress) )
		{
			// Both Addresses are equal
			if(VRRP_TRUE == vrrpSessionObj.info.getPrioritizedSession() )
			{
				vrrpSessionObj.info.priority = vrrpSessionPriority::OWNER;
			}
			else
			{
				result = false;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "%s must be TRUE!", imm::vrrpSession_attribute::PRIO_SESSION.c_str() );
				setExitCode(fixs_ith::ERR_CONFIG_INVALID_VRRP_OWNER, imm::comcli_errormessage::INVALID_VRRP_OWNER);
			}
		}
		else
		{
			vrrpSessionObj.info.priority = (VRRP_TRUE == vrrpSessionObj.info.getPrioritizedSession()) ? vrrpSessionPriority::DEFAULT_MASTER : vrrpSessionPriority::DEFAULT_BACKUP;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "%s = <%u>", imm::vrrpSession_attribute::PRIORITY.c_str(), vrrpSessionObj.info.priority );

		return result;
	}

	bool VrrpSession_OI::compareIPv4Address(const std::string& interfaceAddress, const std::string& vrrpAddress)
	{
		// remove subnet part
		std::string interfaceIP(interfaceAddress.substr(0, interfaceAddress.find(common::utility::parserTag::slash)));
		std::string vrrpIP(vrrpAddress.substr(0, vrrpAddress.find(common::utility::parserTag::slash)));

		return (interfaceIP.compare(vrrpIP) == 0 );
	}

	ACS_CC_ReturnType VrrpSession_OI::modifyReservedByAttribute(ACS_APGCC_CcbHandle ccbHandle, ACS_APGCC_AdminOwnerHandle adminOwnerHandler, const char* vrrpInterfaceDN, const std::set<std::string>& vrrpSessionsDN)
	{
		char tmpAttrName[64] = {0};
		imm::vrrpInterface_attribute::RESERVED_BY.copy(tmpAttrName, imm::vrrpInterface_attribute::RESERVED_BY.length());

		ACS_CC_ImmParameter reservedByAttribute;
		reservedByAttribute.attrName = tmpAttrName;
		reservedByAttribute.attrType = ATTR_NAMET;
		reservedByAttribute.attrValuesNum = 0;
		void* arrayOfVrrpSessionDN[vrrpSessionsDN.size()];

		std::set<std::string>::const_iterator sessionIterator = vrrpSessionsDN.begin();

		const int MAX_DN_LENGTH = 512U;

		for(; sessionIterator != vrrpSessionsDN.end(); ++sessionIterator )
		{
			char* reservedByValue = new char[MAX_DN_LENGTH];
			::snprintf(reservedByValue, MAX_DN_LENGTH, sessionIterator->c_str());

			arrayOfVrrpSessionDN[reservedByAttribute.attrValuesNum++] = reinterpret_cast<void*>(reservedByValue);
		}

		reservedByAttribute.attrValues = arrayOfVrrpSessionDN;

		int max_attempts = 5;
		ACS_CC_ReturnType immResult = ACS_CC_FAILURE;

		for (int retry = 0; (retry < max_attempts) && (ACS_CC_SUCCESS != immResult); retry++)
		{
			immResult = modifyObjectAugmentCcb(ccbHandle, adminOwnerHandler, vrrpInterfaceDN, &reservedByAttribute);

			if( ACS_CC_SUCCESS != immResult)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "modifyObjectAugmentCcb failed for DN %s. Error %d: <%s>, attempt number: <%i>", vrrpInterfaceDN, getInternalLastError(), getInternalLastErrorText(), (retry+1) );
				::usleep(10000); //10 ms
			}
		}

		if( ACS_CC_SUCCESS != immResult)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "modifyObjectAugmentCcb failed for DN %s. Max number of attempts reached!", vrrpInterfaceDN);
		}


		// clean allocate memory
		for(unsigned index = 0U; index < reservedByAttribute.attrValuesNum; ++index)
		{
			delete[] reinterpret_cast<char*>(reservedByAttribute.attrValues[index]);
		}

		return immResult;
	}

	ACS_CC_ReturnType VrrpSession_OI::modifyPriorityAttribute(ACS_APGCC_CcbHandle ccbHandle, ACS_APGCC_AdminOwnerHandle adminOwnerHandler, const char* vrrpSessionDN, uint32_t priorityValue)
	{
		char tmpAttrName[64] = {0};
		imm::vrrpSession_attribute::PRIORITY.copy(tmpAttrName, imm::vrrpSession_attribute::PRIORITY.length());

		ACS_CC_ImmParameter priorityAttribute;
		priorityAttribute.attrName = tmpAttrName;
		priorityAttribute.attrType = ATTR_UINT32T;
		priorityAttribute.attrValuesNum = 1;

		void* tmpValue[1] = { reinterpret_cast<void*>(&priorityValue) };
		priorityAttribute.attrValues = tmpValue;

		ACS_CC_ReturnType immResult = modifyObjectAugmentCcb(ccbHandle, adminOwnerHandler, vrrpSessionDN, &priorityAttribute);

		if( ACS_CC_SUCCESS != immResult)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "modifyObjectAugmentCcb failed for DN %s. Error %d: <%s>", vrrpSessionDN, getInternalLastError(), getInternalLastErrorText() );
		}

		return immResult;
	}

	void VrrpSession_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_vrrpSessionOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_vrrpSessionOperationTable.erase(operationRange.first, operationRange.second);
	}

} /* namespace imm */
