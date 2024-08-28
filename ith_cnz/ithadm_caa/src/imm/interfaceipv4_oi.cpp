/*
 * interfaceipv4oi.cpp
 *
 *  Created on: May 6, 2016
 *      Author: estevol
 */

#include "imm/interfaceipv4_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_transportBNChandler.h"
#include "engine/context.h"
#include "engine/configurationManager.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

namespace imm {

	Interfaceipv4_OI::Interfaceipv4_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
						Base_OI(managedObjectClassName, objectImplementerName)
	{

	}

	Interfaceipv4_OI::~Interfaceipv4_OI() {

	}

	ACS_CC_ReturnType Interfaceipv4_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		InterfaceIPv4 interfaceIPv4Obj;
		interfaceIPv4Obj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == interfaceIpv4_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string interfaceRDN = reinterpret_cast<char *>(attr[idx]->attrValues[0]);
				interfaceIPv4Obj.info.moDN.assign(interfaceRDN);
				interfaceIPv4Obj.info.moDN += "," + std::string(parentname);

				//get RDN value
				interfaceIPv4Obj.info.name = common::utility::getIdValueFromRdn(interfaceIPv4Obj.info.moDN);

				//get smxId
				interfaceIPv4Obj.info.smxId = common::utility::getSmxIdFromInterfaceDN(interfaceIPv4Obj.info.moDN);

				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == interfaceIpv4_attribute::MTU.compare(attr[idx]->attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::MTU_CHANGE;

				if(0U != attr[idx]->attrValuesNum)
				{
					interfaceIPv4Obj.info.mtu = *(reinterpret_cast<uint32_t *>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::ENCAPSULATION.compare(attr[idx]->attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE;

				if(0U != attr[idx]->attrValuesNum)
				{
					interfaceIPv4Obj.info.encapsulation.assign(reinterpret_cast<char*>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::BFD_STATIC_ROUTES.compare(attr[idx]->attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					interfaceIPv4Obj.info.bfdStaticRoutes = *(reinterpret_cast<imm::oper_State*>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::BFD_PROFILE.compare(attr[idx]->attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					interfaceIPv4Obj.info.bfdProfile.assign(reinterpret_cast<char*>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::ACL_IPV4.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE;
					interfaceIPv4Obj.info.aclIngress.assign(reinterpret_cast<char*>(attr[idx]->attrValues[0]));
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ACL set to:<%s>", interfaceIPv4Obj.info.getAclIngress());
				}

				continue;
			}

			if( 0 == interfaceIpv4_attribute::ADMIN_STATE.compare(attr[idx]->attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::ADMIN_STATE_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					interfaceIPv4Obj.info.adminstate = *(reinterpret_cast<imm::admin_State*>(attr[idx]->attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::LOOPBACK.compare(attr[idx]->attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::LOOPBACK_CHANGE;

				if (0U != attr[idx]->attrValuesNum)
				{
					interfaceIPv4Obj.info.loopback = *(reinterpret_cast<bool*>(attr[idx]->attrValues[0]));
				}

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LoopBack Interface create");
				continue;
			}
		}

		// Operation is exclusive
		interfaceIPv4Obj.exclusive = true;

		result = checkTransactionAugmentation(interfaceIPv4Obj, oiHandle, ccbId);

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_interfaceIPv4OperationTable.insert(std::make_pair(ccbId, interfaceIPv4Obj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId,INTERFACEIPV4_T,&(element->second));
		}

		return result;
	}

	ACS_CC_ReturnType Interfaceipv4_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		InterfaceIPv4 interfaceIPv4Obj;
		interfaceIPv4Obj.action = Delete;
		interfaceIPv4Obj.info.moDN.assign(objName);

		//get RDN value
		interfaceIPv4Obj.info.name = common::utility::getIdValueFromRdn(interfaceIPv4Obj.info.moDN);

		//get smxId and plane value
		interfaceIPv4Obj.info.smxId = common::utility::getSmxIdFromInterfaceDN(interfaceIPv4Obj.info.moDN);

		// Operation is exclusive
		interfaceIPv4Obj.exclusive = true;

		// Set bfdStaticRoutes value in order to schedule BfdSessionIPv4 removal at apply callback
		{
			engine::contextAccess_t access(interfaceIPv4Obj.info.smxId, engine::GET_EXISTING, engine::SHARED_ACCESS);
			engine::Context* context = access.getContext();

			if (NULL != context)
			{
				std::string routerName = common::utility::getRouterNameFromInterfaceDN(interfaceIPv4Obj.info.moDN);
				boost::shared_ptr<engine::InterfaceIPv4> currentInterfaceIPv4Obj = context->getInterfaceByName(routerName, interfaceIPv4Obj.info.name);
				interfaceIPv4Obj.info.bfdStaticRoutes = currentInterfaceIPv4Obj->getData().getBfdStaticRoute();
			}
		}

		result = checkTransactionAugmentation(interfaceIPv4Obj, oiHandle, ccbId);

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_interfaceIPv4OperationTable.insert(std::make_pair(ccbId, interfaceIPv4Obj));
			configurationHelper_t::instance()->scheduleDeleteOperation(ccbId,INTERFACEIPV4_T,&(element->second));
		}

		return result;
	}

	ACS_CC_ReturnType Interfaceipv4_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		UNUSED(objName);
		UNUSED(ccbId);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		InterfaceIPv4 interfaceIPv4Obj;
		interfaceIPv4Obj.action = Modify;
		interfaceIPv4Obj.info.moDN.assign(objName);

		//get RDN value
		interfaceIPv4Obj.info.name = common::utility::getIdValueFromRdn(interfaceIPv4Obj.info.moDN);

		//get smxId and plane value
		interfaceIPv4Obj.info.smxId = common::utility::getSmxIdFromInterfaceDN(interfaceIPv4Obj.info.moDN);


		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]) && (ACS_CC_SUCCESS == result); idx++)
		{
			if( 0 == interfaceIpv4_attribute::MTU.compare(attrMods[idx]->modAttr.attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::MTU_CHANGE;

				if(0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					interfaceIPv4Obj.info.mtu = *(reinterpret_cast<uint32_t *>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::ENCAPSULATION.compare(attrMods[idx]->modAttr.attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE;

				if(0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					interfaceIPv4Obj.info.encapsulation.assign(reinterpret_cast<char*>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::BFD_STATIC_ROUTES.compare(attrMods[idx]->modAttr.attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES_CHANGE;

				if (0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					interfaceIPv4Obj.info.bfdStaticRoutes = *(reinterpret_cast<imm::oper_State*>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::BFD_PROFILE.compare(attrMods[idx]->modAttr.attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE;

				if (0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					interfaceIPv4Obj.info.bfdProfile.assign(reinterpret_cast<char*>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::ACL_IPV4.compare(attrMods[idx]->modAttr.attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE;

				if (0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					interfaceIPv4Obj.info.aclIngress.assign(reinterpret_cast<char*>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::ADMIN_STATE.compare(attrMods[idx]->modAttr.attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::ADMIN_STATE_CHANGE;

				if (0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					interfaceIPv4Obj.info.adminstate = *(reinterpret_cast<imm::admin_State*>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}

			if( 0 == interfaceIpv4_attribute::LOOPBACK.compare(attrMods[idx]->modAttr.attrName) )
			{
				interfaceIPv4Obj.info.changeMask |= imm::interfaceIpv4_attribute::LOOPBACK_CHANGE;

				if (0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					interfaceIPv4Obj.info.loopback = *(reinterpret_cast<bool*>(attrMods[idx]->modAttr.attrValues[0]));
				}
				continue;
			}
		}

		// Operation is exclusive
		interfaceIPv4Obj.exclusive = true;

		result = checkTransactionAugmentation(interfaceIPv4Obj, oiHandle, ccbId);

		if (ACS_CC_SUCCESS == result)
		{
			if (interfaceIPv4Obj.info.changeMask)
			{
				//add into the map and schedule the operation
				operationTable_t::iterator element = m_interfaceIPv4OperationTable.insert(std::make_pair(ccbId, interfaceIPv4Obj));
				configurationHelper_t::instance()->scheduleModifyOperation(ccbId,INTERFACEIPV4_T,&(element->second));
			}
		}

		return result;
	}

	ACS_CC_ReturnType Interfaceipv4_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

	void  Interfaceipv4_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		operationTable_t::iterator element = m_interfaceIPv4OperationTable.find(ccbId);

		if ( element != m_interfaceIPv4OperationTable.end() )
		{
			std::string interfaceIPv4DN = element->second.info.getDN();
			std::string smxId = element->second.info.getSmxId();

			if ( ( imm::Modify == element->second.action ) &&
					( element->second.info.changeMask & imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES_CHANGE ) )
			{
				if ( imm::ENABLED == element->second.info.bfdStaticRoutes )
				{
					fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::ADD_BFD_SESSIONS, 0, 3, interfaceIPv4DN.c_str(), (interfaceIPv4DN.length() + 1)/sizeof(uint8_t));
				}
				else if ( imm::DISABLED == element->second.info.bfdStaticRoutes )
				{
					fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::REMOVE_BFD_SESSIONS, 0, 3, interfaceIPv4DN.c_str(), (interfaceIPv4DN.length() + 1)/sizeof(uint8_t));
				}
			}
			else if ( imm::Delete == element->second.action &&
					!fixs_ith::workingSet_t::instance()->getConfigurationManager().isRemoveContextOngoing(element->second.info.getSmxId()) )
			{
				if ( imm::ENABLED == element->second.info.getBfdStaticRoute() )
				{
					fixs_ith::workingSet_t::instance()->schedule_until_completion(smxId, operation::REMOVE_BFD_SESSIONS, 0, 3, interfaceIPv4DN.c_str(), (interfaceIPv4DN.length() + 1)/sizeof(uint8_t));
				}
			}
		}

		resetOperationTable(ccbId);
	}

	void Interfaceipv4_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void Interfaceipv4_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}


	ACS_CC_ReturnType Interfaceipv4_OI::updateRuntime(const char* p_objName, const char** p_attrName)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "UpdateRuntime callback for %s", p_objName);


		operation::interfaceInfo interfaceIPv4Info;
		interfaceIPv4Info.moDN.assign(p_objName);

		//get RDN value
		interfaceIPv4Info.name = common::utility::getIdValueFromRdn(interfaceIPv4Info.moDN);

		//get smxId and plane value
		interfaceIPv4Info.smxId = common::utility::getSmxIdFromInterfaceDN(interfaceIPv4Info.moDN);


		for (uint16_t i = 0; p_attrName[i] != 0; ++i)
		{
			if ( interfaceIpv4_attribute::OPER_STATE.compare(p_attrName[i]) == 0)
			{
				interfaceIPv4Info.changeMask |= interfaceIpv4_attribute::OPER_STATE_CHANGE;
			}
		}

		if (interfaceIPv4Info.changeMask)
		{
			engine::contextAccess_t access(interfaceIPv4Info.getSmxId(), engine::GET_EXISTING, engine::SHARED_ACCESS);
			engine::Context* context = access.getContext();

			if (context)
			{
				std::string routerName(common::utility::getRouterNameFromInterfaceDN(interfaceIPv4Info.getDN()));
				boost::shared_ptr<engine::InterfaceIPv4> interface = context->getInterfaceByName(routerName, interfaceIPv4Info.getName(), interfaceIPv4Info.changeMask);

				if (interface)
				{
					if (interfaceIPv4Info.changeMask & interfaceIpv4_attribute::OPER_STATE_CHANGE)
					{
						imm::oper_State operState = interface->getOperationalState();

						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting value <%d> for attribute <%s> on DN:<%s>",operState, interfaceIpv4_attribute::OPER_STATE.c_str(), p_objName);

						ACS_CC_ImmParameter operStateParameter;

						char tmpAttrName[64] = {0};
						strcpy(tmpAttrName,  interfaceIpv4_attribute::OPER_STATE.c_str());
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

	void Interfaceipv4_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_interfaceIPv4OperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_interfaceIPv4OperationTable.erase(operationRange.first, operationRange.second);

	}

	ACS_CC_ReturnType Interfaceipv4_OI::checkTransactionAugmentation(const InterfaceIPv4& interfaceObj, ACS_APGCC_OiHandle& oiHandle, ACS_APGCC_CcbId& ccbId)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		bool setVlanReservedBy = false;
		bool resetVlanReservedBy = false;

		bool setBfdProfileReservedBy = false;
		bool resetBfdProfileReservedBy = false;

		bool setAclIngress = false;
		bool resetAclIngress = false;

		bool doAugmentation = false;

		std::string oldBfdProfile;
		std::set<std::string> interfaceDNsOfOldBfdProfile;
		std::set<std::string> interfaceDNsOfNewBfdProfile;

		std::string oldAcl;
		std::set<std::string> interfaceDNsOfOldAcl;
		std::set<std::string> interfaceDNsOfNewAcl;

		switch (interfaceObj.action)
		{
			case Create:
			{
				setVlanReservedBy = strlen(interfaceObj.info.getEncapsulation()) > 0;



				if( getInterfaceIPv4DNsReferencingBfdProfile(interfaceObj.info.bfdProfile, interfaceObj, interfaceDNsOfNewBfdProfile) )
				{
					setBfdProfileReservedBy = !interfaceObj.info.bfdProfile.empty();
					interfaceDNsOfNewBfdProfile.insert(interfaceObj.info.moDN);
				}
				else
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to act on BFD profile");
					return ACS_CC_FAILURE;
				}

				if( getInterfaceIPv4DNsReferencingAclIpv4(interfaceObj.info.aclIngress, interfaceObj, interfaceDNsOfNewAcl))
				{
					setAclIngress = !interfaceObj.info.isAclIngressEmpty();
					interfaceDNsOfNewAcl.insert(interfaceObj.info.moDN);
				}
				else
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to act on ACL");
					return ACS_CC_FAILURE;
				}
			}
			break;

			case Delete:
			{
				resetVlanReservedBy = true;

				if( getCurrentBfdProfile(interfaceObj, oldBfdProfile)
						&& getInterfaceIPv4DNsReferencingBfdProfile(oldBfdProfile, interfaceObj, interfaceDNsOfOldBfdProfile) )
				{
					resetBfdProfileReservedBy = !oldBfdProfile.empty();
					interfaceDNsOfOldBfdProfile.erase(interfaceObj.info.moDN);
				}
				else
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to act on old BFD profile");
					return ACS_CC_FAILURE;
				}

				if( getCurrentAcl(interfaceObj, oldAcl)
						&& getInterfaceIPv4DNsReferencingAclIpv4(oldAcl, interfaceObj, interfaceDNsOfOldAcl) )
				{
					resetAclIngress = !oldAcl.empty();
					interfaceDNsOfOldAcl.erase(interfaceObj.info.moDN);
				}
				else
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to act on old ACL");
					return ACS_CC_FAILURE;
				}
			}
			break;

			case Modify:
			{
				if ((interfaceObj.info.changeMask & imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE))
				{
					setVlanReservedBy = strlen(interfaceObj.info.getEncapsulation()) > 0;
					resetVlanReservedBy = !setVlanReservedBy;
				}

				if ((interfaceObj.info.changeMask & imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE))
				{
					if( getCurrentBfdProfile(interfaceObj, oldBfdProfile)
							&& getInterfaceIPv4DNsReferencingBfdProfile(oldBfdProfile, interfaceObj, interfaceDNsOfOldBfdProfile) )
					{
						resetBfdProfileReservedBy = !oldBfdProfile.empty();
						interfaceDNsOfOldBfdProfile.erase(interfaceObj.info.moDN);
					}
					else
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to act on old BFD profile");
						return ACS_CC_FAILURE;
					}

					if( getInterfaceIPv4DNsReferencingBfdProfile(interfaceObj.info.bfdProfile, interfaceObj, interfaceDNsOfNewBfdProfile) )
					{
						setBfdProfileReservedBy = !interfaceObj.info.bfdProfile.empty();
						interfaceDNsOfNewBfdProfile.insert(interfaceObj.info.moDN);
					}
					else
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to act on current BFD profile");
						return ACS_CC_FAILURE;
					}
				}

				if ((interfaceObj.info.changeMask & imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE))
				{

					if( getCurrentAcl(interfaceObj, oldAcl)
							&& getInterfaceIPv4DNsReferencingAclIpv4(oldAcl, interfaceObj, interfaceDNsOfOldAcl) )
					{
						resetAclIngress = !oldAcl.empty();
						interfaceDNsOfOldAcl.erase(interfaceObj.info.moDN);
					}
					else
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to act on old ACL profile");
						return ACS_CC_FAILURE;
					}

					if( getInterfaceIPv4DNsReferencingAclIpv4(interfaceObj.info.aclIngress, interfaceObj, interfaceDNsOfNewAcl) )
					{
						setAclIngress = !interfaceObj.info.isAclIngressEmpty();
						interfaceDNsOfNewAcl.insert(interfaceObj.info.moDN);
					}
					else
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to act on current ACL profile");
						return ACS_CC_FAILURE;
					}
				}
			}
			break;

			default:
				break;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create flags: \n"
							"Vlan flag:<%s>\n"
							"BFD flag:<%s>\n"
							"ACL flag:<%s>", common::utility::boolToString(setVlanReservedBy)
					, common::utility::boolToString(setBfdProfileReservedBy)
					, common::utility::boolToString(setAclIngress));


		doAugmentation = (setVlanReservedBy || resetVlanReservedBy ||
							setBfdProfileReservedBy || resetBfdProfileReservedBy ||
							setAclIngress || resetAclIngress);

		if(doAugmentation)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Interface augmentation object <%s>", interfaceObj.info.getDN());

			ACS_APGCC_CcbHandle ccbHandleVal;
			ACS_APGCC_AdminOwnerHandle adminOwnerHandlerVal;

			if ((result = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnerHandlerVal)) != ACS_CC_SUCCESS)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "getCcbAugmentationInitialize failed. Error %d: <%s>", getInternalLastError(), getInternalLastErrorText());
				return result;
			}

			{//Augmentation for Vlan reservedBy attribute
				if (setVlanReservedBy)
				{
					result = setVlanReservedByAttribute(interfaceObj.info, ccbHandleVal, adminOwnerHandlerVal);
				}
				else if(resetVlanReservedBy)
				{
					result = resetVlanReservedByAttribute(interfaceObj.info, ccbHandleVal, adminOwnerHandlerVal);
				}
			}

			{//Augmentation for BfdProfile reservedBy attribute

				if(resetBfdProfileReservedBy)
				{
					// update the old BFD profile
					result = modifyReservedByAttribute(ccbHandleVal, adminOwnerHandlerVal, oldBfdProfile, interfaceDNsOfOldBfdProfile);
				}

				if(setBfdProfileReservedBy)
				{
					// update the new BFD profile
					result = modifyReservedByAttribute(ccbHandleVal, adminOwnerHandlerVal, interfaceObj.info.bfdProfile, interfaceDNsOfNewBfdProfile);
				}
			}

			{//Augmentation for alcIpv4 reservedBy attribute

				if(resetAclIngress)
				{
					result = modifyReservedByAttribute(ccbHandleVal, adminOwnerHandlerVal, oldAcl, interfaceDNsOfOldAcl);
				}

				if(setAclIngress)
				{
					result = modifyReservedByAttribute(ccbHandleVal, adminOwnerHandlerVal, interfaceObj.info.aclIngress, interfaceDNsOfNewAcl);
				}
			}

			if ((result = applyAugmentCcb(ccbHandleVal)) != ACS_CC_SUCCESS)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "applyAugmentCcb failed. Error[%d]: <%s>", getInternalLastError(), getInternalLastErrorText());
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Augmentation on InterfaceIPv4:<%s> result:<%s>", interfaceObj.info.getName(), ((ACS_CC_SUCCESS == result) ? "TRUE" : "FALSE"));
		}
		//else nothing to do...

		return result;
	}

	ACS_CC_ReturnType Interfaceipv4_OI::setVlanReservedByAttribute(const operation::interfaceInfo& interfaceObjInfo, ACS_APGCC_CcbHandle& ccbHandleVal, ACS_APGCC_AdminOwnerHandle& adminOwnerHandlerVal)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		ACS_CC_ImmParameter vlanReservedByParameter;

		char tmpAttrName[64] = {0};
		strcpy(tmpAttrName,  imm::vlan_attribute::RESERVED_BY.c_str());
		vlanReservedByParameter.attrName = tmpAttrName;
		vlanReservedByParameter.attrType = ATTR_NAMET;
		vlanReservedByParameter.attrValuesNum = 1;
		char reservedByValue[IMM_DN_PATH_SIZE_MAX] = {0};
		::snprintf(reservedByValue,IMM_DN_PATH_SIZE_MAX,interfaceObjInfo.getDN());
		void *value[1] = {reinterpret_cast<void*>(reservedByValue)};
		vlanReservedByParameter.attrValues = value;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Modifying attribute <%s> for object <%s>", vlanReservedByParameter.attrName, interfaceObjInfo.getEncapsulation());

		if ((result = modifyObjectAugmentCcb(ccbHandleVal, adminOwnerHandlerVal, interfaceObjInfo.getEncapsulation(), &vlanReservedByParameter)) != ACS_CC_SUCCESS)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "modifyObjectAugmentCcb failed for DN %s. Error %d: <%s>", interfaceObjInfo.getEncapsulation(), getInternalLastError(), getInternalLastErrorText());
		}

		return result;
	}

	ACS_CC_ReturnType Interfaceipv4_OI::resetVlanReservedByAttribute(const operation::interfaceInfo& interfaceObjInfo, ACS_APGCC_CcbHandle& ccbHandleVal, ACS_APGCC_AdminOwnerHandle& adminOwnerHandlerVal)
	{
		ACS_CC_ReturnType result = ACS_CC_SUCCESS;


		engine::contextAccess_t access(interfaceObjInfo.getSmxId(), engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = access.getContext();

		if (engine::CONTEXT_ACCESS_ACQUIRED == access.getAccessResult() && context && !context->isContextRemoveOngoing() )
		{
			std::string routerName(common::utility::getRouterNameFromInterfaceDN(interfaceObjInfo.getDN()));
			boost::shared_ptr<engine::InterfaceIPv4> interface = context->getInterfaceByName(routerName, interfaceObjInfo.getName());

			if (NULL != interface && interface->hasEncapsulation())
			{
				ACS_CC_ImmParameter vlanReservedByParameter;

				char tmpAttrName[64] = {0};
				strcpy(tmpAttrName,  imm::vlan_attribute::RESERVED_BY.c_str());
				vlanReservedByParameter.attrName = tmpAttrName;
				vlanReservedByParameter.attrType = ATTR_NAMET;
				vlanReservedByParameter.attrValuesNum = 0;
				vlanReservedByParameter.attrValues = 0;

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Modifying attribute <%s> for object <%s>", vlanReservedByParameter.attrName, interfaceObjInfo.getEncapsulation());

				if ( (result = modifyObjectAugmentCcb(ccbHandleVal, adminOwnerHandlerVal, interface->getEncapsulationDN(), &vlanReservedByParameter)) != ACS_CC_SUCCESS)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "modifyObjectAugmentCcb failed for DN %s. Error %d: <%s>", interfaceObjInfo.getEncapsulation(), getInternalLastError(), getInternalLastErrorText());
				}
			}
		}

		return result;
	}

	bool Interfaceipv4_OI::getCurrentBfdProfile(const InterfaceIPv4& interfaceIPv4Obj, std::string& bfdProfile)
	{
		bool result = false;

		std::string smxId(interfaceIPv4Obj.info.getSmxId());
		engine::contextAccess_t access(smxId, engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = access.getContext();

		if(NULL != context)
		{
			std::string routerName = common::utility::getRouterNameFromInterfaceDN(interfaceIPv4Obj.info.getDN());
			std::string interfaceName = interfaceIPv4Obj.info.getName();

			boost::shared_ptr<engine::InterfaceIPv4> currentInterfaceIPv4Obj = context->getInterfaceByName(routerName, interfaceName);

			if(currentInterfaceIPv4Obj)
			{
				bfdProfile.assign(currentInterfaceIPv4Obj->getData().getBfdProfile());
				result = true;
			}
			else
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Interface <%s> not found", smxId.c_str(), routerName.c_str(), interfaceName.c_str());
			}
		}
		else
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Context not found or not accessible.", smxId.c_str());
		}

		return result;
	}

	bool Interfaceipv4_OI::getCurrentAcl(const InterfaceIPv4& interfaceIPv4Obj, std::string& aclDn)
	{
		bool result = false;

		std::string smxId = interfaceIPv4Obj.info.getSmxId();
		engine::contextAccess_t access(smxId, engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = access.getContext();

		if(NULL != context)
		{
			std::string routerName = common::utility::getRouterNameFromInterfaceDN(interfaceIPv4Obj.info.getDN());
			std::string interfaceName = interfaceIPv4Obj.info.getName();

			boost::shared_ptr<engine::InterfaceIPv4> currentInterfaceIPv4Obj = context->getInterfaceByName(routerName, interfaceName);

			if(currentInterfaceIPv4Obj)
			{
				aclDn = currentInterfaceIPv4Obj->getData().getAclIngress();
				result = true;
			}
			else
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Interface <%s> not found", smxId.c_str(), routerName.c_str(), interfaceName.c_str());
			}
		}
		else
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Context not found or not accessible.", smxId.c_str());

		}

		return result;
	}

	bool Interfaceipv4_OI::getInterfaceIPv4DNsReferencingBfdProfile(const std::string& bfdProfileDN, const InterfaceIPv4& interfaceIPv4Obj, std::set<std::string>& interfaceIPv4DNs)
	{
		bool result = true;

		if( bfdProfileDN.empty() )
		{
			return result;
		}

		std::string smxId = interfaceIPv4Obj.info.getSmxId();
		std::string routerName = common::utility::getRouterNameFromInterfaceDN(interfaceIPv4Obj.info.getDN());
		std::string bfdProfileName = common::utility::getBfdProfileNameFromDN(bfdProfileDN);
		fixs_ith::ErrorConstants getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getInterfacesIPv4ReferencingBfdProfile(smxId, routerName, bfdProfileName, interfaceIPv4DNs);

		if(fixs_ith::ERR_NO_ERRORS != getResult)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Interfaces IPv4 referencing BfdProfile:<%s>", bfdProfileName.c_str() );
			result = false;
		}

		return result;
	}

	bool Interfaceipv4_OI::getInterfaceIPv4DNsReferencingAclIpv4(const std::string& aclIpv4DN, const InterfaceIPv4& interfaceIPv4Obj, std::set<std::string>& interfaceIPv4DNs)
	{
		bool result = true;

		if( aclIpv4DN.empty() )
		{
			return result;
		}

		std::string smxId = interfaceIPv4Obj.info.getSmxId();
		std::string routerName = common::utility::getRouterNameFromInterfaceDN(interfaceIPv4Obj.info.getDN());
		std::string aclIpv4Name = common::utility::getAclIpv4NameFromDN(aclIpv4DN);
		fixs_ith::ErrorConstants getResult = fixs_ith::workingSet_t::instance()->getConfigurationManager().getInterfacesIPv4ReferencingAclIpv4(smxId, routerName, aclIpv4Name, interfaceIPv4DNs);

		if(fixs_ith::ERR_NO_ERRORS != getResult)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Interfaces IPv4 referencing Acl Ipv4:<%s>", aclIpv4Name.c_str() );
			return false;
		}

		switch (interfaceIPv4Obj.action)
		{
			case Create:
			{
				// add the current InterfaceIPv4 DN to the previous ones
				interfaceIPv4DNs.insert(interfaceIPv4Obj.info.moDN);
			}
			break;

			case Delete:
			{
				// remove the current InterfaceIPv4 DN to the previous ones
				interfaceIPv4DNs.erase(interfaceIPv4Obj.info.moDN);
			}
			break;

			case Modify:
			{
				if( interfaceIPv4Obj.info.isAclIngressEmpty() )
				{
					// remove the current InterfaceIPv4 DN to the previous ones
					interfaceIPv4DNs.erase(interfaceIPv4Obj.info.moDN);
				}
				else
				{
					// add the current InterfaceIPv4 DN to the previous ones
					interfaceIPv4DNs.insert(interfaceIPv4Obj.info.moDN);
				}
			}
			break;
			default:
				result = false;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Invalid action on InterfaceIPv4:<%s>", interfaceIPv4Obj.info.getDN());
		}

		return result;
	}

	ACS_CC_ReturnType Interfaceipv4_OI::modifyReservedByAttribute(ACS_APGCC_CcbHandle ccbHandle, ACS_APGCC_AdminOwnerHandle adminOwnerHandler, const std::string& objectDN, const std::set<std::string>& interfaceIPv4DNs )
	{
		char tmpAttrName[64] = {0};

		if( std::string::npos != objectDN.find(imm::bfdProfile_attribute::RDN.c_str()) )
		{
			imm::bfdProfile_attribute::RESERVED_BY.copy(tmpAttrName, imm::bfdProfile_attribute::RESERVED_BY.length());
		}
		else
		{
			imm::aclIpv4_attribute::RESERVED_BY.copy(tmpAttrName, imm::aclIpv4_attribute::RESERVED_BY.length());
		}


		ACS_CC_ImmParameter reservedByAttribute;
		reservedByAttribute.attrName = tmpAttrName;
		reservedByAttribute.attrType = ATTR_NAMET;
		reservedByAttribute.attrValuesNum = 0;
		void* arrayOfInterfaceIPv4DNs[interfaceIPv4DNs.size()];

		std::set<std::string>::const_iterator sessionIterator = interfaceIPv4DNs.begin();

		const int MAX_DN_LENGTH = 512U;

		for(; sessionIterator != interfaceIPv4DNs.end(); ++sessionIterator )
		{
			char* reservedByValue = new char[MAX_DN_LENGTH];
			::snprintf(reservedByValue, MAX_DN_LENGTH, sessionIterator->c_str());

			arrayOfInterfaceIPv4DNs[reservedByAttribute.attrValuesNum++] = reinterpret_cast<void*>(reservedByValue);
		}

		reservedByAttribute.attrValues = arrayOfInterfaceIPv4DNs;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Modifying attribute <%s> for object <%s>", reservedByAttribute.attrName, objectDN.c_str());

		ACS_CC_ReturnType immResult = modifyObjectAugmentCcb(ccbHandle, adminOwnerHandler, objectDN.c_str(), &reservedByAttribute);

		if( ACS_CC_SUCCESS != immResult)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "modifyObjectAugmentCcb failed for DN <%s>. Error %d: <%s>", objectDN.c_str(), getInternalLastError(), getInternalLastErrorText() );
		}

		// clean allocate memory
		for(unsigned index = 0U; index < reservedByAttribute.attrValuesNum; ++index)
		{
			delete[] reinterpret_cast<char*>(reservedByAttribute.attrValues[index]);
		}

		return immResult;
	}



} /* namespace imm */
