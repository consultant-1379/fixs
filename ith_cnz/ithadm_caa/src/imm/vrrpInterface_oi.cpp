/*
 * vrrpInterface_oi.cpp
 *
 *  Created on: Jun 1, 2016
 *      Author: estevol
 */

#include "imm/vrrpInterface_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_transportBNChandler.h"
#include "engine/context.h"
#include "engine/configurationManager.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_workingset.h"

namespace imm {

	VrrpInterface_OI::VrrpInterface_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
						Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	VrrpInterface_OI::~VrrpInterface_OI() {

	}

	ACS_CC_ReturnType VrrpInterface_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		VrrpInterface vrrpInterfaceObj;
		vrrpInterfaceObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == vrrpInterface_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string interfaceRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				vrrpInterfaceObj.info.moDN.assign(interfaceRDN);
				vrrpInterfaceObj.info.moDN += "," + std::string(parentname);

				//get RDN value
				vrrpInterfaceObj.info.name = common::utility::getIdValueFromRdn(vrrpInterfaceObj.info.moDN);

				//get smxId - Intentionally empty
				//vrrpInterfaceObj.info.smxId

				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == vrrpInterface_attribute::ADV_INTERVAL.compare(attr[idx]->attrName) )
			{
				vrrpInterfaceObj.info.changeMask |= imm::vrrpInterface_attribute::ADV_INTERVAL_CHANGE;

				if(0U != attr[idx]->attrValuesNum)
				{
					vrrpInterfaceObj.info.advertiseInterval = *(reinterpret_cast<int32_t *>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == vrrpInterface_attribute::PREEMPT_HOLD_TIME.compare(attr[idx]->attrName) )
			{
				vrrpInterfaceObj.info.changeMask |= imm::vrrpInterface_attribute::PREEMPT_HOLD_TIME_CHANGE;

				if(0U != attr[idx]->attrValuesNum)
				{
					vrrpInterfaceObj.info.preemptHoldTime = *(reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == vrrpInterface_attribute::PREEMPT_MODE.compare(attr[idx]->attrName) )
			{
				vrrpInterfaceObj.info.changeMask |= imm::vrrpInterface_attribute::PREEMPT_MODE_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					vrrpInterfaceObj.info.preemptMode = *(reinterpret_cast<bool*>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == vrrpInterface_attribute::IDENTITY.compare(attr[idx]->attrName) )
			{
				vrrpInterfaceObj.info.changeMask |= imm::vrrpInterface_attribute::IDENTITY_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					vrrpInterfaceObj.info.vrIdentity = *(reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]));
				}
				continue;
			}

		}

		// Operation is exclusive
		vrrpInterfaceObj.exclusive = true;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_vrrpInterfaceOperationTable.insert(std::make_pair(ccbId, vrrpInterfaceObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId, VRRPINTERFACE_T, &(element->second));
		}

		return result;
	}

	ACS_CC_ReturnType VrrpInterface_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		VrrpInterface vrrpInterfaceObj;
		vrrpInterfaceObj.action = Delete;
		vrrpInterfaceObj.info.moDN.assign(objName);

		//get RDN value
		vrrpInterfaceObj.info.name = common::utility::getIdValueFromRdn(vrrpInterfaceObj.info.moDN);

		//get smxId and plane value
		vrrpInterfaceObj.info.smxId = common::utility::getSmxIdFromInterfaceDN(vrrpInterfaceObj.info.moDN);

		// Operation is exclusive
		vrrpInterfaceObj.exclusive = true;

		//add into the map and schedule the operation
		operationTable_t::iterator element = m_vrrpInterfaceOperationTable.insert(std::make_pair(ccbId, vrrpInterfaceObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId, VRRPINTERFACE_T, &(element->second));

		return result;
	}

	ACS_CC_ReturnType VrrpInterface_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		UNUSED(objName);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "VRRP Interface modify: ccbId<%llu>", ccbId);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		VrrpInterface vrrpInterfaceObj;
		vrrpInterfaceObj.action = Modify;
		vrrpInterfaceObj.info.moDN.assign(objName);

		//get RDN value
		vrrpInterfaceObj.info.name = common::utility::getIdValueFromRdn(vrrpInterfaceObj.info.moDN);

		//get smxId and plane value - Intentionally empty
		//vrrpInterfaceObj.info.smxId


		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]); ++idx)
		{
			if( 0 == vrrpInterface_attribute::ADV_INTERVAL.compare(attrMods[idx]->modAttr.attrName) )
			{
				vrrpInterfaceObj.info.changeMask |= imm::vrrpInterface_attribute::ADV_INTERVAL_CHANGE;

				if(0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					vrrpInterfaceObj.info.advertiseInterval = *(reinterpret_cast<int32_t *>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == vrrpInterface_attribute::PREEMPT_HOLD_TIME.compare(attrMods[idx]->modAttr.attrName) )
			{
				vrrpInterfaceObj.info.changeMask |= imm::vrrpInterface_attribute::PREEMPT_HOLD_TIME_CHANGE;

				if(0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					vrrpInterfaceObj.info.preemptHoldTime = *(reinterpret_cast<int32_t*>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == vrrpInterface_attribute::PREEMPT_MODE.compare(attrMods[idx]->modAttr.attrName) )
			{
				vrrpInterfaceObj.info.changeMask |= imm::vrrpInterface_attribute::PREEMPT_MODE_CHANGE;

				if (0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					vrrpInterfaceObj.info.preemptMode = *(reinterpret_cast<bool*>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == vrrpInterface_attribute::RESERVED_BY.compare(attrMods[idx]->modAttr.attrName) )
			{
				vrrpInterfaceObj.info.changeMask |= imm::vrrpInterface_attribute::RESERVED_BY_CHANGE;

				vrrpInterfaceObj.info.reservedBy.clear();
				for (uint32_t j = 0; j <  attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					vrrpInterfaceObj.info.reservedBy.insert(reinterpret_cast<char *>( attrMods[idx]->modAttr.attrValues[j]));
				}

				// Only for abort handling
				fixs_ith::ErrorConstants getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getVrrpSessionsOfVrrpInterface(vrrpInterfaceObj.info.name, vrrpInterfaceObj.previousVrrpSessionsDN);

				if(fixs_ith::ERR_NO_ERRORS != getResult)
				{
					setExitCode(static_cast<int>(fixs_ith::ERR_CONFIG_INVALID_VRRP_INTERFACE_NAME), imm::comcli_errormessage::INVALID_VRRP_INTERFACE_NAME);
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get vrrpSessions for vrrpInterface:<%s>", vrrpInterfaceObj.info.getName() );
					return ACS_CC_FAILURE;
				}

				continue;
			}
		}

		//ReservedBy change is not an exclusive operation
		vrrpInterfaceObj.exclusive = (vrrpInterfaceObj.info.changeMask != vrrpInterface_attribute::RESERVED_BY_CHANGE);

		if (vrrpInterfaceObj.info.changeMask)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_vrrpInterfaceOperationTable.insert(std::make_pair(ccbId, vrrpInterfaceObj));
			configurationHelper_t::instance()->scheduleModifyOperation(ccbId, VRRPINTERFACE_T, &(element->second));
		}

		return result;
	}

	ACS_CC_ReturnType VrrpInterface_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "VRRP Interface Complete: ccbId<%llu>", ccbId);

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

	void  VrrpInterface_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void VrrpInterface_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);
		operationTable_t::iterator element = m_vrrpInterfaceOperationTable.find(ccbId);

		// check for ReservedBy attribute rollback to previous value
		if( (m_vrrpInterfaceOperationTable.end() != element) &&  (imm::Modify == element->second.action) &&
				(imm::vrrpInterface_attribute::RESERVED_BY_CHANGE & element->second.info.changeMask) )
		{
			element->second.info.reservedBy = element->second.previousVrrpSessionsDN;
			int result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(element->second.info);
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Rollback of ReservedBy attribute of the VRRP Instance DN:<%s>, result<%d>",
					element->second.info.getDN(), result);
		}

		resetOperationTable(ccbId);
	}

	void VrrpInterface_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_vrrpInterfaceOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_vrrpInterfaceOperationTable.erase(operationRange.first, operationRange.second);

	}

} /* namespace imm */
