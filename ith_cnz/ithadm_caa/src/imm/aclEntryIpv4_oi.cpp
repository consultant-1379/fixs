/*
 * dst_oi.cpp
 *
 *  Created on: Oct 13, 2016
 *      Author: xclaele
 */

#include "imm/aclEntryIpv4_oi.h"
#include "imm/imm.h"
#include "imm/configurationHelper.h"
#include "common/utility.h"
#include "fixs_ith_logger.h"

namespace imm
{

	AclEntry_OI::AclEntry_OI()
	: Base_OI(imm::moc_name::CLASS_ACL_ENTRY, imm::oi_name::IMPL_ACLENTRY),
	  m_aclEntryOperationTable()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry_OI created");
	}

	AclEntry_OI::~AclEntry_OI()
	{

	}

	ACS_CC_ReturnType AclEntry_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		UNUSED(oiHandle);
		UNUSED(className);

		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		AclEntry aclEntryObj;
		aclEntryObj.action = Create;

		// extract the attributes
		for(size_t idx = 0U; NULL != attr[idx]; ++idx)
		{
			// if this is the RDN attribute then get RDN
			if( 0 == aclEntry_attribute::RDN.compare(attr[idx]->attrName) )
			{
				//get DN
				std::string sessionDN( reinterpret_cast<char *>(attr[idx]->attrValues[0]));
				sessionDN.push_back(common::utility::parserTag::comma);
				sessionDN.append(parentname);

				aclEntryObj.info.moDN.assign(sessionDN);

				//get RDN value
				aclEntryObj.info.name.assign(common::utility::getIdValueFromRdn(sessionDN));

				//get smxId
				aclEntryObj.info.smxId = common::utility::getSmxIdFromAclEntryDN(sessionDN);

				result = ACS_CC_SUCCESS;
				continue;
			}

			if( 0 == aclEntry_attribute::ACLACTION.compare(attr[idx]->attrName) )
			{
				// Mandatory attribute
				int value = *reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]);
				aclEntryObj.info.action = ( 0U == value ? PERMIT : DENY);
				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::ACTION_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::ADDR_DST.compare(attr[idx]->attrName) )
			{
				aclEntryObj.info.addrDst.clear();
				bool present = false;
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					aclEntryObj.info.addrDst.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
					present = true;
				}

				if(present)
					aclEntryObj.info.changeMask |= imm::aclEntry_attribute::ADDR_DST_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::ADDR_SRC.compare(attr[idx]->attrName) )
			{
				aclEntryObj.info.addrSrc.clear();
				bool present = false;
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					aclEntryObj.info.addrSrc.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
					present = true;
				}

				if(present)
					aclEntryObj.info.changeMask |= imm::aclEntry_attribute::ADDR_SRC_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::DSCP.compare(attr[idx]->attrName) )
			{
				aclEntryObj.info.dscp.clear();
				bool present = false;
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					aclEntryObj.info.dscp.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
					present = true;
				}

				if(present)
					aclEntryObj.info.changeMask |= imm::aclEntry_attribute::DSCP_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::FRAGMENT_TYPE.compare(attr[idx]->attrName) )
			{
				if (0U != attr[idx]->attrValuesNum)
				{
					int value = *reinterpret_cast<int32_t*>(attr[idx]->attrValues[0]);
					imm::aclFragmentType fragmentType = FRAGMENT;
					switch(value)
					{
					case 3:
						fragmentType = FRAGMENT;
						break;
					case 4:
						fragmentType = INITIAL;
						break;
					case 5:
						fragmentType = NONINITIAL;
						break;
					default:
						break;
					}
					aclEntryObj.info.fragmentType = fragmentType;
					aclEntryObj.info.changeMask |= imm::aclEntry_attribute::FRAGMENT_TYPE_CHANGE;
				}
				continue;
			}

			if( 0 == aclEntry_attribute::ICMP_TYPE.compare(attr[idx]->attrName) )
			{
				aclEntryObj.info.icmpType.clear();
				bool present = false;
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					aclEntryObj.info.icmpType.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
					present = true;
				}

				if(present)
					aclEntryObj.info.changeMask |= imm::aclEntry_attribute::ICMP_TYPE_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::IP_PROTOCOL.compare(attr[idx]->attrName) )
			{
				aclEntryObj.info.ipProtocol.clear();
				bool present = false;
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					aclEntryObj.info.ipProtocol.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
					present = true;
				}

				if(present)
					aclEntryObj.info.changeMask |= imm::aclEntry_attribute::IP_PROTOCOL_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::PORT_DST.compare(attr[idx]->attrName) )
			{
				aclEntryObj.info.portDst.clear();
				bool present = false;
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					aclEntryObj.info.portDst.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
					present = true;
				}

				if(present)
					aclEntryObj.info.changeMask |= imm::aclEntry_attribute::PORT_DST_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::PORT_SRC.compare(attr[idx]->attrName) )
			{
				aclEntryObj.info.portSrc.clear();
				bool present = false;
				for (uint32_t j = 0; j < attr[idx]->attrValuesNum; j++)
				{
					aclEntryObj.info.portSrc.insert(reinterpret_cast<char *>(attr[idx]->attrValues[j]));
					present = true;
				}

				if(present)
					aclEntryObj.info.changeMask |= imm::aclEntry_attribute::PORT_SRC_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::PRIORITY.compare(attr[idx]->attrName) )
			{
				// Mandatory attribute
				uint16_t value = *reinterpret_cast<uint16_t*>(attr[idx]->attrValues[0]);
				aclEntryObj.info.priority = value;
				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::PRIORITY_CHANGE;
				continue;
			}

		}

		// Operation is NOT exclusive
		aclEntryObj.exclusive = false;

		if(ACS_CC_SUCCESS == result)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_aclEntryOperationTable.insert(std::make_pair(ccbId, aclEntryObj));
			configurationHelper_t::instance()->scheduleCreateOperation(ccbId, ACLENTRY_T, &(element->second));
		}

		return result;
	}


	ACS_CC_ReturnType AclEntry_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		UNUSED(oiHandle);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "AclEntry delete: ccbId<%llu>", ccbId);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		AclEntry aclEntryObj;
		aclEntryObj.action = Delete;
		aclEntryObj.info.moDN.assign(objName);

		//get RDN value
		aclEntryObj.info.name = common::utility::getIdValueFromRdn(aclEntryObj.info.moDN);

		//get smxId and plane value
		aclEntryObj.info.smxId = common::utility::getSmxIdFromAclEntryDN(aclEntryObj.info.moDN);

		// Operation is NOT exclusive
		aclEntryObj.exclusive = false;

		//add into the map and schedule the operation
		operationTable_t::iterator element = m_aclEntryOperationTable.insert(std::make_pair(ccbId, aclEntryObj));
		configurationHelper_t::instance()->scheduleDeleteOperation(ccbId, ACLENTRY_T, &(element->second));

		return result;
	}

	ACS_CC_ReturnType AclEntry_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		UNUSED(oiHandle);
		UNUSED(objName);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "VRRP Interface modify: ccbId<%llu>", ccbId);

		ACS_CC_ReturnType result = ACS_CC_SUCCESS;

		AclEntry aclEntryObj;
		aclEntryObj.action = Modify;
		aclEntryObj.info.moDN.assign(objName);

		//get RDN value
		aclEntryObj.info.name = common::utility::getIdValueFromRdn(aclEntryObj.info.moDN);

		//get smxId and plane value - Intentionally empty
		aclEntryObj.info.smxId = common::utility::getSmxIdFromAclEntryDN(aclEntryObj.info.moDN);


		// extract the attributes to modify
		for(size_t idx = 0U; (NULL != attrMods[idx]); ++idx)
		{
			if( 0 == aclEntry_attribute::ACLACTION.compare(attrMods[idx]->modAttr.attrName) )
			{
				int value = *reinterpret_cast<int32_t*>(attrMods[idx]->modAttr.attrValues[0]);
				aclEntryObj.info.action = ( 0U == value ? PERMIT : DENY);
				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::ACTION_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::ADDR_DST.compare(attrMods[idx]->modAttr.attrName) )
			{
				aclEntryObj.info.addrDst.clear();
				for (uint32_t j = 0; j < attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					aclEntryObj.info.addrDst.insert(reinterpret_cast<char *>(attrMods[idx]->modAttr.attrValues[j]));
				}

				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::ADDR_DST_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::ADDR_SRC.compare(attrMods[idx]->modAttr.attrName) )
			{
				aclEntryObj.info.addrSrc.clear();
				for (uint32_t j = 0; j < attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					aclEntryObj.info.addrSrc.insert(reinterpret_cast<char *>(attrMods[idx]->modAttr.attrValues[j]));
				}

				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::ADDR_SRC_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::DSCP.compare(attrMods[idx]->modAttr.attrName) )
			{
				aclEntryObj.info.dscp.clear();
				for (uint32_t j = 0; j < attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					aclEntryObj.info.dscp.insert(reinterpret_cast<char *>(attrMods[idx]->modAttr.attrValues[j]));
				}

				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::DSCP_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::FRAGMENT_TYPE.compare(attrMods[idx]->modAttr.attrName) )
			{
				if (0U != attrMods[idx]->modAttr.attrValuesNum)
				{
					int value = *reinterpret_cast<int32_t*>(attrMods[idx]->modAttr.attrValues[0]);
					imm::aclFragmentType fragmentType = FRAGMENT;
					switch(value)
					{
					case 3:
						fragmentType = FRAGMENT;
						break;
					case 4:
						fragmentType = INITIAL;
						break;
					case 5:
						fragmentType = NONINITIAL;
						break;
					default:
						break;
					}
					aclEntryObj.info.fragmentType = fragmentType;
				}
				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::FRAGMENT_TYPE_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::ICMP_TYPE.compare(attrMods[idx]->modAttr.attrName) )
			{
				aclEntryObj.info.icmpType.clear();
				for (uint32_t j = 0; j < attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					aclEntryObj.info.icmpType.insert(reinterpret_cast<char *>(attrMods[idx]->modAttr.attrValues[j]));
				}

				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::ICMP_TYPE_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::IP_PROTOCOL.compare(attrMods[idx]->modAttr.attrName) )
			{
				aclEntryObj.info.ipProtocol.clear();
				for (uint32_t j = 0; j < attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					aclEntryObj.info.ipProtocol.insert(reinterpret_cast<char *>(attrMods[idx]->modAttr.attrValues[j]));
				}

				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::IP_PROTOCOL_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::PORT_DST.compare(attrMods[idx]->modAttr.attrName) )
			{
				aclEntryObj.info.portDst.clear();
				for (uint32_t j = 0; j < attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					aclEntryObj.info.portDst.insert(reinterpret_cast<char *>(attrMods[idx]->modAttr.attrValues[j]));
				}

				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::PORT_DST_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::PORT_SRC.compare(attrMods[idx]->modAttr.attrName) )
			{
				aclEntryObj.info.portSrc.clear();
				for (uint32_t j = 0; j < attrMods[idx]->modAttr.attrValuesNum; j++)
				{
					aclEntryObj.info.portSrc.insert(reinterpret_cast<char *>(attrMods[idx]->modAttr.attrValues[j]));
				}

				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::PORT_SRC_CHANGE;
				continue;
			}

			if( 0 == aclEntry_attribute::PRIORITY.compare(attrMods[idx]->modAttr.attrName) )
			{
				uint16_t value = *reinterpret_cast<uint16_t*>(attrMods[idx]->modAttr.attrValues[0]);
				aclEntryObj.info.priority = value;
				aclEntryObj.info.changeMask |= imm::aclEntry_attribute::PRIORITY_CHANGE;
				continue;
			}
		}

		// Operation is exclusive
		aclEntryObj.exclusive = true;

		if (aclEntryObj.info.changeMask)
		{
			//add into the map and schedule the operation
			operationTable_t::iterator element = m_aclEntryOperationTable.insert(std::make_pair(ccbId, aclEntryObj));
			configurationHelper_t::instance()->scheduleModifyOperation(ccbId, ACLENTRY_T, &(element->second));
		}

		return result;
	}

	ACS_CC_ReturnType AclEntry_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "AclEntryIpv4 complete: ccbId<%llu>", ccbId);

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

	void  AclEntry_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void AclEntry_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		UNUSED(oiHandle);

		resetOperationTable(ccbId);
	}

	void AclEntry_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		UNUSED(oiHandle);
		UNUSED(invocation);
		UNUSED(p_objName);
		UNUSED(operationId);
		UNUSED(paramList);
	}

	void AclEntry_OI::resetOperationTable(const ACS_APGCC_CcbId& ccbId)
	{
		std::pair<operationTable_t::iterator, operationTable_t::iterator> operationRange;
		operationRange = m_aclEntryOperationTable.equal_range(ccbId);

		// Erase all elements from the table of the operations
		m_aclEntryOperationTable.erase(operationRange.first, operationRange.second);
	}

}
