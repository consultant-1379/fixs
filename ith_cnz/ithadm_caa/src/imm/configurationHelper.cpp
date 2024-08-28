/*
 * configurationHelper.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */

#include "imm/configurationHelper.h"
#include "imm/imm.h"

#include "common/utility.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_logger.h"

#include "engine/basicNetworkConfiguration.h"

namespace imm
{

	ConfigurationHelper::ConfigurationHelper()
	: m_createOperations(),m_modifyOperations(),m_deleteOperations(),m_exclusiveMoOperations(),
	  m_createOrderedMoOperations(),m_modifyOrderedMoOperations(),m_deleteOrderedMoOperations()
	{
		//exclusive operation
		{
			m_exclusiveMoOperations.insert(VLAN_T);
			m_exclusiveMoOperations.insert(ROUTER_T);
			m_exclusiveMoOperations.insert(BFDPROFILE_T);
			m_exclusiveMoOperations.insert(INTERFACEIPV4_T);
			m_exclusiveMoOperations.insert(ADDRESS_T);
			m_exclusiveMoOperations.insert(DST_T);
			m_exclusiveMoOperations.insert(NEXTHOP_T);
			m_exclusiveMoOperations.insert(VRRPINTERFACE_T);
			m_exclusiveMoOperations.insert(VRRPSESSION_T);
			m_exclusiveMoOperations.insert(BFDSESSION_T);
			m_exclusiveMoOperations.insert(ACLENTRY_T);
			m_exclusiveMoOperations.insert(ACLIPV4_T);
		}

		//Define order of create operations
		{
			//L2
			m_createOrderedMoOperations.push_back(BRIDGE_T);
			m_createOrderedMoOperations.push_back(ETHERNETPORT_T);
			m_createOrderedMoOperations.push_back(AGGREGATOR_T);
			m_createOrderedMoOperations.push_back(BRIDGEPORT_T);
			m_createOrderedMoOperations.push_back(VLAN_T);
			m_createOrderedMoOperations.push_back(SUBNETVLAN_T);

			//L3
			m_createOrderedMoOperations.push_back(VRRPINTERFACE_T);
			m_createOrderedMoOperations.push_back(ROUTER_T);
			m_createOrderedMoOperations.push_back(BFDPROFILE_T);
			m_createOrderedMoOperations.push_back(INTERFACEIPV4_T);
			m_createOrderedMoOperations.push_back(ADDRESS_T);
			m_createOrderedMoOperations.push_back(DST_T);
			m_createOrderedMoOperations.push_back(NEXTHOP_T);
			m_createOrderedMoOperations.push_back(VRRPSESSION_T);
			m_createOrderedMoOperations.push_back(BFDSESSION_T);
			m_createOrderedMoOperations.push_back(ACLENTRY_T);
			m_createOrderedMoOperations.push_back(ACLIPV4_T);
		}

		//Define order of modify operations
		{
			//L2
			m_modifyOrderedMoOperations.push_back(BRIDGE_T);
			m_modifyOrderedMoOperations.push_back(ETHERNETPORT_T);
			m_modifyOrderedMoOperations.push_back(AGGREGATOR_T);
			m_modifyOrderedMoOperations.push_back(BRIDGEPORT_T);
			m_modifyOrderedMoOperations.push_back(VLAN_T);
			m_modifyOrderedMoOperations.push_back(SUBNETVLAN_T);

			//L3
			m_modifyOrderedMoOperations.push_back(ROUTER_T);
			m_modifyOrderedMoOperations.push_back(BFDPROFILE_T);
			m_modifyOrderedMoOperations.push_back(INTERFACEIPV4_T);
			m_modifyOrderedMoOperations.push_back(ADDRESS_T);
			m_modifyOrderedMoOperations.push_back(DST_T);
			m_modifyOrderedMoOperations.push_back(NEXTHOP_T);
			m_modifyOrderedMoOperations.push_back(VRRPINTERFACE_T);
			m_modifyOrderedMoOperations.push_back(VRRPSESSION_T);
			m_modifyOrderedMoOperations.push_back(ACLIPV4_T);
			m_modifyOrderedMoOperations.push_back(ACLENTRY_T);
		}

		//Define order of delete operations
		{
			//L3
			m_deleteOrderedMoOperations.push_back(ROUTER_T);
			m_deleteOrderedMoOperations.push_back(ADDRESS_T);
			m_deleteOrderedMoOperations.push_back(INTERFACEIPV4_T);
			m_deleteOrderedMoOperations.push_back(BFDPROFILE_T);
			m_deleteOrderedMoOperations.push_back(NEXTHOP_T);
			m_deleteOrderedMoOperations.push_back(DST_T);
			m_deleteOrderedMoOperations.push_back(VRRPINTERFACE_T);
			m_deleteOrderedMoOperations.push_back(VRRPSESSION_T);
			m_deleteOrderedMoOperations.push_back(BFDSESSION_T);
			m_deleteOrderedMoOperations.push_back(ACLIPV4_T);
			m_deleteOrderedMoOperations.push_back(ACLENTRY_T);

			//L2
			m_deleteOrderedMoOperations.push_back(SUBNETVLAN_T);
			m_deleteOrderedMoOperations.push_back(VLAN_T);
			m_deleteOrderedMoOperations.push_back(BRIDGEPORT_T);
			m_deleteOrderedMoOperations.push_back(AGGREGATOR_T);
			m_deleteOrderedMoOperations.push_back(ETHERNETPORT_T);
			m_deleteOrderedMoOperations.push_back(BRIDGE_T);

		}
	}

	ConfigurationHelper::~ConfigurationHelper()
	{

	}

	void ConfigurationHelper::scheduleCreateOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, const void *moInfo)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_operationMutex);
		m_createOperations.insert(std::make_pair(configID, std::make_pair(moType, moInfo)));
	}

	void ConfigurationHelper::scheduleModifyOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, const void *moInfo)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_operationMutex);
		m_modifyOperations.insert(std::make_pair(configID, std::make_pair(moType, moInfo)));
	}

	void ConfigurationHelper::scheduleDeleteOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, const void *moInfo)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_operationMutex);
		m_deleteOperations.insert(std::make_pair(configID, std::make_pair(moType, moInfo)));

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Delete operation:<%d>: ccbId<%llu>", m_deleteOperations.size(), configID);
	}

	bool ConfigurationHelper::existMoOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType)
	{
		std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> createCommandRange = m_createOperations.equal_range(configID);
		std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> modifyCommandRange = m_modifyOperations.equal_range(configID);
		std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> deleteCommandRange = m_deleteOperations.equal_range(configID);

		//for each operation found
		for(mapOfOperation_t::iterator element = createCommandRange.first; element != createCommandRange.second; ++element)
		{
			operation_t operation = element->second;
			if (moType == operation.first)
			{
				return true;
			}
		}

		for(mapOfOperation_t::iterator element = modifyCommandRange.first; element != modifyCommandRange.second; ++element)
		{
			operation_t operation = element->second;
			if (moType == operation.first)
			{
				return true;
			}
		}

		for(mapOfOperation_t::iterator element = deleteCommandRange.first; element != deleteCommandRange.second; ++element)
		{
			operation_t operation = element->second;
			if (moType == operation.first)
			{
				return true;
			}
		}

		return false;
	}

	unsigned int ConfigurationHelper::countMoOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, mapOfOperation_t& mapOfOperations)
	{
		unsigned int count = 0;
		std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> commandRange;
		commandRange = mapOfOperations.equal_range(configID);

		//for each operation found
		for(mapOfOperation_t::iterator element = commandRange.first; element != commandRange.second; ++element)
		{
			operation_t operation = element->second;
			if (moType == operation.first)
			{
				++count;
			}
		}

		//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%d] Found <%u> operation(s) for MOType <%d>",configID, count, moType);
		return count;
	}

	unsigned int ConfigurationHelper::countExclusiveMoOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, mapOfOperation_t& mapOfOperations)
	{
		unsigned int count = 0;
		std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> commandRange;
		commandRange = mapOfOperations.equal_range(configID);

		//for each operation found
		for(mapOfOperation_t::iterator element = commandRange.first; element != commandRange.second; ++element)
		{
			operation_t operation = element->second;
			if (moType == operation.first)
			{
				const Base_OI::Operation* op = reinterpret_cast<const Base_OI::Operation*> (operation.second);
				if (op && op->isExclusive())
				{
					++count;
				}
			}
		}

		//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%d] Found <%u> operation(s) for MOType <%d>",configID, count, moType);
		return count;
	}

	bool ConfigurationHelper::isMoOperationAllowed(const ACS_APGCC_CcbId& configID)
	{
		int exclusiveOperationCount = 0;

		//for each operation found
		for(std::set<MOtype_t>::iterator operationType = m_exclusiveMoOperations.begin(); operationType != m_exclusiveMoOperations.end(); ++operationType)
		{
			//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%d] Counting Create operations for MOType <%d>",configID, *operationType);
			unsigned int createOperations = countExclusiveMoOperation(configID, *operationType, m_createOperations);
			exclusiveOperationCount += createOperations;
			//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Found <%d> create operations for MO %d", createOperations, *operationType);

			if ( ROUTER_T == *operationType  && createOperations > 0)
			{

				//When a Router is created, loopback interface and corresponding address are created as well.
				//Do not include those operations in the count
				exclusiveOperationCount -= 2 * createOperations;
			}

			unsigned int modifyOperations = countExclusiveMoOperation(configID, *operationType, m_modifyOperations);
			exclusiveOperationCount += modifyOperations;
			//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Found <%d> modify operations for MO %d", modifyOperations, *operationType);

			unsigned int deleteOperations = countExclusiveMoOperation(configID, *operationType, m_deleteOperations);
			exclusiveOperationCount += deleteOperations;
			//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Found <%d> delete operations for MO %d", deleteOperations, *operationType);

			if ( ROUTER_T == *operationType  && deleteOperations > 0)
			{
				//When a Router is deleted, loopback interface and corresponding address are deleted as well.
				//Do not include those operations in the count
				exclusiveOperationCount -= 2 * deleteOperations;
			}

		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Total operations %d", exclusiveOperationCount);
		return (exclusiveOperationCount <= 1);
	}

	fixs_ith::ErrorConstants ConfigurationHelper::executeMoOperation(const ACS_APGCC_CcbId& configID)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_operationMutex);
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if(fixs_ith::workingSet_t::instance()->isBackupOngoing())
		{
			result = fixs_ith::ERR_BACKUP_IN_PROGRESS;
		}
		else if(!isMoOperationAllowed(configID))
		{
			//More than one operation in the same transaction
			result = fixs_ith::ERR_IMM_TOO_MANY_OP;
		}
		else
		{
			//Execute Delete operations first...
			{
				std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> commandRange;
				commandRange = m_deleteOperations.equal_range(configID);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Execute <%d> Delete Operations, Id:<%u>", m_deleteOperations.size(), configID);
				//find and perform operation
				result = findAndExecute(m_deleteOrderedMoOperations, commandRange);
			}

			// ... Then Modify operations ...
			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> commandRange;
				commandRange = m_modifyOperations.equal_range(configID);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Execute <%d> Modify Operations, Id:<%u>", m_modifyOperations.size(), configID);
				//find and perform operation
				result = findAndExecute(m_modifyOrderedMoOperations, commandRange);
			}

			//... Finally create operations.
			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> commandRange;
				commandRange = m_createOperations.equal_range(configID);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Execute <%d> Create Operations, Id:<%u>", m_createOperations.size(), configID);
				//find and perform operation
				result = findAndExecute(m_createOrderedMoOperations, commandRange);
			}

		}
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "executeMoOperation returns with result:<%d>", result);

		//clean operations from the map at first execution triggered by complete callback
		cleanMoOperation(configID);

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::validateAndExecuteMoOperation(const MOtype_t& moType, const void *moInfo)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		//L2
		const Bridge_OI::Bridge* bridge = 0;
		const BridgePort_OI::BridgePort* bridgePort = 0;
		const EthernetPort_OI::EthernetPort* ethernetPort = 0;
		const Aggregator_OI::Aggregator* aggregator = 0;
		const Vlan_OI::Vlan* vlan = 0;
		const SubnetVlan_OI::SubnetVlan* subnetVlan = 0;

		//L3
		const Router_OI::Router* router = 0;
		const Interfaceipv4_OI::InterfaceIPv4* interface = 0;
		const Address_OI::Address* address = 0;
		const Dst_OI::Dst* dst = 0;
		const NextHop_OI::NextHop* nextHop = 0;
		const VrrpInterface_OI::VrrpInterface* vrrpInterface = 0;
		const VrrpSession_OI::VrrpSession* vrrpSession = 0;
		const BfdProfile_OI::BfdProfile* bfdProfile = 0;
		const BfdSession_OI::BfdSession* bfdSession = 0;
		const AclEntry_OI::AclEntry* aclEntry = 0;
		const AclIpv4_OI::AclIpv4* aclIpv4 = 0;


		switch(moType)
		{
			case BRIDGE_T:
				bridge = reinterpret_cast<const Bridge_OI::Bridge*>(moInfo);
				result = verifyAndExecuteMoOperation(bridge);
				break;

			case BRIDGEPORT_T:
				bridgePort = reinterpret_cast<const BridgePort_OI::BridgePort*>(moInfo);
				result = verifyAndExecuteMoOperation(bridgePort);
				break;

			case ETHERNETPORT_T:
				ethernetPort = reinterpret_cast<const EthernetPort_OI::EthernetPort*>(moInfo);
				result = verifyAndExecuteMoOperation(ethernetPort);
				break;

			case AGGREGATOR_T:
				aggregator = reinterpret_cast<const Aggregator_OI::Aggregator*>(moInfo);
				result = verifyAndExecuteMoOperation(aggregator);
				break;

			case VLAN_T:
				vlan = reinterpret_cast<const Vlan_OI::Vlan*>(moInfo);
				result = verifyAndExecuteMoOperation(vlan);
				break;

			case SUBNETVLAN_T:
				subnetVlan = reinterpret_cast<const SubnetVlan_OI::SubnetVlan*>(moInfo);
				result = verifyAndExecuteMoOperation(subnetVlan);
				break;

			case ROUTER_T:
				router = reinterpret_cast<const Router_OI::Router*>(moInfo);
				result = verifyAndExecuteMoOperation(router);
				break;

			case INTERFACEIPV4_T:
				interface = reinterpret_cast<const Interfaceipv4_OI::InterfaceIPv4*>(moInfo);
				result = verifyAndExecuteMoOperation(interface);
				break;

			case ADDRESS_T:
				address = reinterpret_cast<const Address_OI::Address*>(moInfo);
				result = verifyAndExecuteMoOperation(address);
				break;

			case DST_T:
				dst = reinterpret_cast<const Dst_OI::Dst*>(moInfo);
				result = verifyAndExecuteMoOperation(dst);
				break;

			case NEXTHOP_T:
				nextHop = reinterpret_cast<const NextHop_OI::NextHop*>(moInfo);
				result = verifyAndExecuteMoOperation(nextHop);
				break;

			case VRRPINTERFACE_T:
				vrrpInterface = reinterpret_cast<const VrrpInterface_OI::VrrpInterface*>(moInfo);
				result = verifyAndExecuteMoOperation(vrrpInterface);
				break;

			case VRRPSESSION_T:
				vrrpSession = reinterpret_cast<const VrrpSession_OI::VrrpSession*>(moInfo);
				result = verifyAndExecuteMoOperation(vrrpSession);
				break;

			case BFDPROFILE_T:
				bfdProfile = reinterpret_cast<const BfdProfile_OI::BfdProfile*>(moInfo);
				result = verifyAndExecuteMoOperation(bfdProfile);
				break;

			case BFDSESSION_T:
				bfdSession = reinterpret_cast<const BfdSession_OI::BfdSession*>(moInfo);
				result = verifyAndExecuteMoOperation(bfdSession);
				break;

			case ACLENTRY_T:
				aclEntry = reinterpret_cast<const AclEntry_OI::AclEntry*>(moInfo);
				result = verifyAndExecuteMoOperation(aclEntry);
				break;

			case ACLIPV4_T:
				aclIpv4 = reinterpret_cast<const AclIpv4_OI::AclIpv4*>(moInfo);
				result = verifyAndExecuteMoOperation(aclIpv4);
				break;

			default:
				result = fixs_ith::ERR_GENERIC;
				break;
		}


		return result;
	}

	void ConfigurationHelper::cleanMoOperation(const ACS_APGCC_CcbId& configID)
	{
		std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> createCommandRange = m_createOperations.equal_range(configID);
		std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> modifyCommandRange = m_modifyOperations.equal_range(configID);
		std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator> deleteCommandRange = m_deleteOperations.equal_range(configID);

		// Erase all commands from the commands map
		m_createOperations.erase(createCommandRange.first, createCommandRange.second);
		m_modifyOperations.erase(modifyCommandRange.first, modifyCommandRange.second);
		m_deleteOperations.erase(deleteCommandRange.first, deleteCommandRange.second);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%d] Cleaning Create operations. Size <%d>",configID, m_createOperations.size());
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%d] Cleaning Modify operations. Size <%d>",configID, m_modifyOperations.size());
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%d] Cleaning Delete operations. Size <%d>",configID, m_deleteOperations.size());
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const Bridge_OI::Bridge* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;
		if (Create == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;

	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const BridgePort_OI::BridgePort* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			operation::bridgePortInfo bridgePortInfoObj(object->info);
			fixs_ith::ErrorConstants set_result = reset_adminstate_changeMask(bridgePortInfoObj);

			result = (fixs_ith::ERR_NO_ERRORS == set_result)? fixs_ith::workingSet_t::instance()->getConfigurationManager().add(bridgePortInfoObj): set_result;
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info, object->info.changeMask);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info):validationResult;
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;

	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const EthernetPort_OI::EthernetPort* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;

	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const Aggregator_OI::Aggregator* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;

	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const Vlan_OI::Vlan* object )
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info, imm::change_mask::ALL_ATTRIBUTES);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info): validationResult;
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info, object->info.changeMask);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info): validationResult;
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;

	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const SubnetVlan_OI::SubnetVlan* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info, imm::change_mask::ALL_ATTRIBUTES);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info): validationResult;
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{

			fixs_ith::ErrorConstants validationResult = validate(object->info, object->info.changeMask);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info): validationResult;

		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const Router_OI::Router* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			if (!fixs_ith::workingSet_t::instance()->getConfigurationManager().hasContext(object->info.getSmxId()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Context:<%s> no Found for the Router :<%s>",
						object->info.getSmxId(), object->info.getName());
				return fixs_ith::ERR_CONFIG_INVALID_ROUTER_NAME;
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);

		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			return fixs_ith::ERR_NO_ERRORS;
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const Interfaceipv4_OI::InterfaceIPv4* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info,  object->info.changeMask);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info): validationResult;

		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info, object->info.changeMask);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info): validationResult;

		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const Address_OI::Address* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info,  object->info.changeMask);
			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info): validationResult;
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info, object->info.changeMask);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info): validationResult;
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const Dst_OI::Dst* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info,  object->info.changeMask);
			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info): validationResult;
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info, object->info.changeMask);

			result = (fixs_ith::ERR_NO_ERRORS == validationResult)? fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info): validationResult;
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const NextHop_OI::NextHop* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const VrrpInterface_OI::VrrpInterface* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (Create == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info,  object->info.changeMask);
			result = (fixs_ith::ERR_NO_ERRORS == validationResult) ? fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info) : validationResult;
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info,  object->info.changeMask);
			result = (fixs_ith::ERR_NO_ERRORS == validationResult) ? fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info) : validationResult;
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const VrrpSession_OI::VrrpSession* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Vrrp Session operation:<%d>", object->action);

		if (Create == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const BfdProfile_OI::BfdProfile* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Bfd Profile operation:<%d>", object->action);

		if (Create == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const BfdSession_OI::BfdSession* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BFD Session operation:<%d>", object->action);

		if (Create == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const AclEntry_OI::AclEntry* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "AclEntry operation:<%d>", object->action);

		if (Create == object->action)
		{
			fixs_ith::ErrorConstants validationResult = validate(object->info,  object->info.changeMask);
			result = (fixs_ith::ERR_NO_ERRORS == validationResult) ? fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info) : validationResult;
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::verifyAndExecuteMoOperation(const AclIpv4_OI::AclIpv4* object)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Acl IpV4 operation:<%d>", object->action);

		if (Create == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(object->info);
		}
		else if (Delete == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().remove(object->info);
		}
		else if (Modify == object->action)
		{
			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().change(object->info);
		}
		else
		{
			result = fixs_ith::ERR_GENERIC;
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::findAndExecute(const std::vector<MOtype_t> & listOperations, const std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator>& commandRange)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		//for each operation found
		for(std::vector<MOtype_t>::const_iterator operationType = listOperations.begin(); operationType != listOperations.end() && result == fixs_ith::ERR_NO_ERRORS; ++operationType)
		{
			//for each operation found
			for(mapOfOperation_t::iterator element = commandRange.first; element != commandRange.second && result == fixs_ith::ERR_NO_ERRORS; ++element)
			{
				operation_t operation = element->second;

				if (*operationType == operation.first)
				{
					//validate operation
					result = validateAndExecuteMoOperation(operation.first, operation.second);
				}
			}

		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::reset_adminstate_changeMask(operation::bridgePortInfo& info)
	{
		uint16_t IfIndex ;
		fixs_ith::ErrorConstants result = fixs_ith::workingSet_t::instance()->getConfigurationManager().getSnmpIndexfromBridgePortDN(info.moDN, IfIndex);
		if(result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "set_adminstate_changeMask failed! BridgePort:<%s>, DN:<%s>: errorCode:<%d>", info.getName(),
					info.getBridgePortDN(), result);
			return result;
		}
		// If IfIndex is an interface to be ENABLED as last step of SMX configuration reset changeMask to skip operation towards SMX
		if (!fixs_ith::workingSet_t::instance()->getConfigurationManager().is_early_enabled_if(info.plane, IfIndex)){
			info.changeMask ^= imm::bridgePort_attribute::ADM_STATE_CHANGE;
		}
		return fixs_ith::ERR_NO_ERRORS;
	}


	fixs_ith::ErrorConstants ConfigurationHelper::validate(const operation::bridgePortInfo& info, const uint16_t& changeMask)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if ((changeMask & imm::bridgePort_attribute::ADM_STATE_CHANGE ) && info.adminstate == imm::UNLOCKED )
		{
			//Verify that ADMIN state value is valid for the specified interface
			uint16_t IfIndex ;
			std::string objectDN = info.moDN;
			fixs_ith::switchboard_plane_t plane = info.plane;
			fixs_ith::ErrorConstants result = fixs_ith::workingSet_t::instance()->getConfigurationManager().getSnmpIndexfromBridgePortDN(objectDN, IfIndex);
			if(result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid BridgePort <%s> belonging to SmxId <%s>",
											info.getSmxId(), common::utility::planeToString(plane).c_str(), info.name.c_str(), info.smxId.c_str());
				return result;
			}

			if ((IfIndex == TRANSPORT_PLANE_CONTROL2_PORT_INDEX && info.plane == fixs_ith::TRANSPORT_PLANE )  ||
						(IfIndex == CONTROL_PLANE_TRANSPORT2_PORT_INDEX  && plane == fixs_ith::CONTROL_PLANE))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] IS NOT ALLOWED TO SET ADMIN STATE to <%s> for BridgePort <%s>",
									info.getSmxId(), common::utility::planeToString(plane).c_str(), "UNLOCKED", info.name.c_str());

				return fixs_ith::ERR_CONFIG_INVALID_BRIDGEPORT_STATE;
			}
		}

		return result;
	}


	fixs_ith::ErrorConstants ConfigurationHelper::validate(const operation::vlanInfo& info, const uint16_t& changeMask)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (changeMask & imm::vlan_attribute::TAGGED_PORT_CHANGE)
		{
			//Verify Tagged Ports
			for (std::set<std::string>::iterator portDN = info.taggedBridgePorts.begin(); portDN != info.taggedBridgePorts.end(); ++portDN)
			{
				std::string portName = common::utility::getIdValueFromRdn(*portDN);

				std::string bridgePortSmxId = common::utility::getSmxIdFromBridgeDN(common::utility::getParentDN(*portDN));
				if (0 != bridgePortSmxId.compare(info.getSmxId()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid BridgePort <%s> belonging to SmxId <%s> for Vlan <%s> belonging to SmxId <%s>",
							info.getSmxId(), common::utility::planeToString(info.getPlane()).c_str(), portName.c_str(), bridgePortSmxId.c_str(),
							info.getName(), info.getSmxId());
					return fixs_ith::ERR_CONFIG_INVALID_TAGGED_PORT;
				}

				fixs_ith::SwitchBoardPlaneConstants bridgePortPlane = common::utility::getPlaneFromBridgeDN(common::utility::getParentDN(*portDN));
				if (info.getPlane() != bridgePortPlane)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid BridgePort <%s> on plane <%s> for Vlan <%s> on plane <%s>",
							info.getSmxId(), common::utility::planeToString(info.getPlane()).c_str(), portName.c_str(), common::utility::planeToString(bridgePortPlane).c_str(),
							info.getName(), common::utility::planeToString(info.getPlane()).c_str());
					return fixs_ith::ERR_CONFIG_INVALID_TAGGED_PORT;
				}
			}
		}

		if (changeMask & imm::vlan_attribute::UNTAGGED_PORT_CHANGE)
		{
			//Verify Untagged Ports
			for (std::set<std::string>::iterator portDN = info.untaggedBridgePorts.begin(); portDN != info.untaggedBridgePorts.end(); ++portDN)
			{
				std::string portName = common::utility::getIdValueFromRdn(*portDN);

				std::string bridgePortSmxId = common::utility::getSmxIdFromBridgeDN(common::utility::getParentDN(*portDN));
				if (0 != bridgePortSmxId.compare(info.getSmxId()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid BridgePort <%s> belonging to SmxId <%s> for Vlan <%s> belonging to SmxId <%s>",
							info.getSmxId(), common::utility::planeToString(info.getPlane()).c_str(), portName.c_str(), bridgePortSmxId.c_str(),
							info.getName(), info.getSmxId());
					return fixs_ith::ERR_CONFIG_INVALID_UNTAGGED_PORT;
				}

				fixs_ith::SwitchBoardPlaneConstants bridgePortPlane = common::utility::getPlaneFromBridgeDN(common::utility::getParentDN(*portDN));
				if (info.getPlane() != bridgePortPlane)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid BridgePort <%s> on plane <%s> for Vlan <%s> on plane <%s>",
							info.getSmxId(), common::utility::planeToString(info.getPlane()).c_str(), portName.c_str(), common::utility::planeToString(bridgePortPlane).c_str(),
							info.getName(), common::utility::planeToString(info.getPlane()).c_str());
					return fixs_ith::ERR_CONFIG_INVALID_UNTAGGED_PORT;
				}
			}
		}

		if (changeMask & imm::vlan_attribute::VLAN_ID_CHANGE)
		{
			if (!common::utility::isValidVlanIdRange(info.getVlanId()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid vid <%u> for Vlan <%s>",
						info.getSmxId(), common::utility::planeToString(info.getPlane()).c_str(), info.getVlanId(), info.getName());
				return fixs_ith::ERR_CONFIG_INVALID_VLAN_ID;
			}
		}


		return result;
	}


	fixs_ith::ErrorConstants ConfigurationHelper::validate(const operation::subnetVlanInfo& info, const uint16_t& changeMask)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (fixs_ith::CONTROL_PLANE != info.getPlane())
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] plane not supported for SubnetVlan <%s>",
					info.getSmxId(), common::utility::planeToString(info.getPlane()).c_str(), info.getName());
			return fixs_ith::ERR_CONFIG_INVALID_SUBNET_VLAN_PLANE;
		}

		if (changeMask & imm::subnetVlan_attribute::VLAN_ID_CHANGE)
		{
			if (!common::utility::isValidVlanIdRange(info.getSubnetVlanId()) ||
					imm::vlanId_range::UNDEFINED == info.getSubnetVlanId())
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid vid <%u> for SubnetVlan <%s>",
						info.getSmxId(), common::utility::planeToString(info.getPlane()).c_str(), info.getSubnetVlanId(), info.getName());
				return fixs_ith::ERR_CONFIG_INVALID_VLAN_ID;
			}
		}
		if (changeMask & imm::subnetVlan_attribute::IP_SUBNET_CHANGE)
		{
			if (!common::utility::isValidIpv4Subnet(info.getIpSubnet()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid Ipv4Subnet <%s> for SubnetVlan <%s>",
						info.getSmxId(), common::utility::planeToString(info.getPlane()).c_str(), info.getIpSubnet(), info.getName());
				return fixs_ith::ERR_CONFIG_INVALID_SUBNET_VLAN_IP;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::validate(const operation::interfaceInfo& info, const uint16_t& changeMask)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		//Validate interface name
		std::string routerName = common::utility::getRouterNameFromInterfaceDN(info.getDN());
		std::string loopback_ifname = common::utility::buildLoopbackInterfaceName(routerName);

		//Loopback interface is an exception to Interface naming rule
		if (loopback_ifname.compare(info.getName()) != 0)
		{
            if(!common::utility::isValidObjectId(info.getName()))
            {
            	FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid Interface name <%s>",
            			info.getSmxId(),
						common::utility::getRouterNameFromInterfaceDN(info.getDN()).c_str(),
						info.getName());
            	return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_NAME;
            }

		}

		if (changeMask & imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE)
		{
			//Encapsulation is set
			if (strlen(info.getEncapsulation()) > 0)
			{
				if (fixs_ith::TRANSPORT_PLANE != common::utility::getPlaneFromVlanDN(info.getEncapsulation()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid plane %s for encapsulation of Interface <%s>",
							info.getSmxId(),
							common::utility::getRouterNameFromInterfaceDN(info.getDN()).c_str(),
							common::utility::planeToString(common::utility::getPlaneFromVlanDN(info.getEncapsulation())).c_str(),
							 info.getName());
					return fixs_ith::ERR_CONFIG_INVALID_ENCAPSULATION;
				}

				if (common::utility::getSmxIdFromVlanDN(info.getEncapsulation()).compare(info.getSmxId()) != 0)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid SMX ID <%s> for encapsulation of Interface <%s> with SMX ID <%s>",
							info.getSmxId(),
							common::utility::getRouterNameFromInterfaceDN(info.getDN()).c_str(),
							common::utility::getSmxIdFromVlanDN(info.getEncapsulation()).c_str(),
							info.getName(), info.getSmxId());
					return fixs_ith::ERR_CONFIG_INVALID_ENCAPSULATION;
				}
			}
		}

		if (changeMask & imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE)
		{
			//bfdProfile is set
			if (strlen(info.getBfdProfile()) > 0)
			{
				if (!fixs_ith::workingSet_t::instance()->getConfigurationManager().isBfdProfileDNValid(info.getSmxId(), info.getDN(), info.getBfdProfile()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid BfdProfile DN <%s> for InterfaceIPv4 <%s>",
							info.getSmxId(),
							common::utility::getRouterNameFromInterfaceDN(info.getDN()).c_str(),
							info.getBfdProfile(),
							info.getName());
					return fixs_ith::ERR_CONFIG_INVALID_BFDPROFILE_NAME;
				}
			}
		}

		if (changeMask & imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE)
		{
			//aclIngress is set
			if(!info.isAclIngressEmpty())
			{
				if (!fixs_ith::workingSet_t::instance()->getConfigurationManager().isAclIngressDNValid(info.getSmxId(), info.getDN(), info.getAclIngress()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid aclIngress DN <%s> for InterfaceIPv4 <%s>",
							info.getSmxId(),
							common::utility::getRouterNameFromInterfaceDN(info.getDN()).c_str(),
							info.getAclIngress(),
							info.getName());
					return fixs_ith::ERR_CONFIG_INVALID_ACLINGRESS_NAME;
				}
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::validate(const operation::addressInfo& info, const uint16_t& changeMask)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (changeMask & imm::addressIpv4_attribute::ADDRESS_CHANGE)
		{
			if (!common::utility::isValidIpv4Address(info.getAddress()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid address <%s> for AddressIPv4 <%s> on interface <%s>",
						info.getSmxId(), common::utility::getRouterNameFromAddressDN(info.getDN()).c_str(), info.getAddress(), info.getName(),
						common::utility::getInterfaceNameFromAddressDN(info.getDN()).c_str());
				return fixs_ith::ERR_CONFIG_INVALID_ADDRESS;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::validate(const operation::dstInfo& info, const uint16_t& changeMask)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (changeMask & imm::dst_attribute::DST_CHANGE)
		{
			if (!common::utility::isValidIpv4Subnet(info.getDst()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Invalid dst <%s> for Dst <%s>",
						info.getSmxId(), common::utility::getRouterNameFromDstDN(info.getDN()).c_str(), info.getDst(), info.getName());
				return fixs_ith::ERR_CONFIG_INVALID_DST_IP;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::validate(const operation::vrrpInterfaceInfo& info, const uint16_t& changeMask)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (changeMask & imm::vrrpInterface_attribute::ADV_INTERVAL_CHANGE)
		{
			if ((info.getAdvertiseInterval()%(imm::vrrpInterface_attribute::ADV_INTERVAL_MULTIPLIER)) != 0)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Invalid advertiseInterval value <%i> for Vrrpv3Interface <%s>",
						info.getAdvertiseInterval(), info.getName());
				return fixs_ith::ERR_CONFIG_INVALID_VRRP_ADVERTISE_INTERVAL;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationHelper::validate(const operation::aclEntryInfo& info, const uint16_t& changeMask)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if ((changeMask & imm::aclEntry_attribute::ICMP_TYPE_CHANGE) ||
				(changeMask & imm::aclEntry_attribute::PORT_DST_CHANGE) ||
					(changeMask & imm::aclEntry_attribute::PORT_SRC_CHANGE) ||
						(changeMask & imm::aclEntry_attribute::IP_PROTOCOL_CHANGE))
		{
			// icmpType can be non-empty only if ipProtocol contains one single-value element, equal to ICMP (1).
			if (!info.getIcmpType().empty())
			{
				if (!((1 == info.getIpProtocol().size()) && (0 == std::strcmp((*info.getIpProtocol().begin()).c_str(),"1"))))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Mismatch between imcpType and ipProtocol attribute for AclEntryIpv4:<%s>", info.getName());
					return fixs_ith::ERR_CONFIG_ACLENTRY_ICMP_IP_MISMATCH;
				}
			}

			// portDst and portSrc can be non-empty only if the protocol is non-empty and if it contains
			// elements that are single values equal to UDP (17), TCP (6), or SCTP (132).
			if (!(info.getPortDst().empty() && info.getPortSrc().empty()))
			{
				if (!((1 == info.getIpProtocol().size()) &&
						(0 == std::strcmp((*info.getIpProtocol().begin()).c_str(),"6") ||
								0 == std::strcmp((*info.getIpProtocol().begin()).c_str(),"17") ||
									0 == std::strcmp((*info.getIpProtocol().begin()).c_str(),"132"))))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Mismatch between portDst or portSrc and ipProtocol attribute for AclEntryIpv4:<%s>", info.getName());
					return fixs_ith::ERR_CONFIG_ACLENTRY_PORT_IP_MISMATCH;
				}
			}
		}

		return result;
	}

	std::string ConfigurationHelper::getErrorText(const fixs_ith::ErrorConstants errorcode)
	{
		std::string errorText;

		switch(errorcode)
		{
			case fixs_ith::ERR_IMM_TOO_MANY_OP:								errorText = comcli_errormessage::TOO_MANY_OPERATIONS;				break;
			case fixs_ith::ERR_MEMORY_BAD_ALLOC:							errorText = comcli_errormessage::MEMORY_BAD_ALLOC;					break;
			case fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE:				errorText = comcli_errormessage::COMMUNICATION_FAILURE;				break;
			case fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE:					errorText = comcli_errormessage::COMMUNICATION_FAILURE;				break;
			case fixs_ith::ERR_CONFIG_PROTECTED_OBJECT:						errorText = comcli_errormessage::PROTECTED_OBJECT;					break;
			case fixs_ith::ERR_CONFIG_INVALID_BRIDGE_ID:					errorText = comcli_errormessage::INVALID_BRIDGE_ID; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_ETHERNET_PORT_ID:				errorText = comcli_errormessage::INVALID_ETHERNET_PORT_ID; 			break;
			case fixs_ith::ERR_CONFIG_ETH_AUTO_NEG_CHANGE_NOT_ALLOWED:		errorText = comcli_errormessage::ETH_AUTO_NEG_CHANGE_NOT_ALLOWED; 	break;
			case fixs_ith::ERR_CONFIG_ETH_AUTO_NEG_CHANGE_NOT_SUPPORTED:	errorText = comcli_errormessage::ETH_AUTO_NEG_CHANGE_NOT_SUPPORTED; break;
			case fixs_ith::ERR_CONFIG_INVALID_AGGREGATOR_ID:				errorText = comcli_errormessage::INVALID_AGGREGATOR_ID; 			break;
			case fixs_ith::ERR_CONFIG_INVALID_BRIDGE_PORT_ID:				errorText = comcli_errormessage::INVALID_BRIDGE_PORT_ID; 			break;
			case fixs_ith::ERR_CONFIG_INVALID_VLAN_ID:						errorText = comcli_errormessage::INVALID_VLAN_ID; 					break;
			case fixs_ith::ERR_CONFIG_INVALID_SUBNET_VLAN_ID:				errorText = comcli_errormessage::INVALID_SUBNET_VLAN_ID; 			break;
			case fixs_ith::ERR_CONFIG_INVALID_SUBNET_VLAN_PLANE:			errorText = comcli_errormessage::INVALID_SUBNET_VLAN_PLANE; 		break;
			case fixs_ith::ERR_CONFIG_INVALID_SUBNET_VLAN_IP:	 			errorText = comcli_errormessage::INVALID_SUBNET_IP; 				break;
			case fixs_ith::ERR_CONFIG_DUPLICATED_VLAN_ID:					errorText = comcli_errormessage::DUPLICATED_VLAN_ID; 				break;
			case fixs_ith::ERR_CONFIG_RESTRICTED_VLAN_ID:					errorText = comcli_errormessage::RESTRICTED_VLAN_ID;				break;
			case fixs_ith::ERR_CONFIG_PROTECTED_VLAN:						errorText = comcli_errormessage::PROTECTED_VLAN; 					break;
			case fixs_ith::ERR_CONFIG_INVALID_TAGGED_PORT:					errorText = comcli_errormessage::INVALID_TAGGED_PORT; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_UNTAGGED_PORT:				errorText = comcli_errormessage::INVALID_UNTAGGED_PORT; 			break;
			case fixs_ith::ERR_CONFIG_DUPLICATED_PORT:						errorText = comcli_errormessage::DUPLICATED_PORT; 					break;
			case fixs_ith::ERR_CONFIG_AGG_MEMBER_PORT:						errorText = comcli_errormessage::AGG_MEMBER_PORT; 					break;
			case fixs_ith::ERR_CONFIG_DUPLICATED_SUBNET_VLAN_IP:			errorText = comcli_errormessage::DUPLICATED_SUBNET_VLAN_IP; 		break;
			case fixs_ith::ERR_CONFIG_INVALID_ROUTER_NAME:					errorText = comcli_errormessage::INVALID_ROUTER_NAME; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_INTERFACE_ID:					errorText = comcli_errormessage::INVALID_INTERFACE_ID; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_INTERFACE_NAME:				errorText = comcli_errormessage::INVALID_INTERFACE_NAME; 			break;
			case fixs_ith::ERR_CONFIG_INVALID_BFDPROFILE_NAME:				errorText = comcli_errormessage::INVALID_BFDPROFILE_NAME; 			break;
			case fixs_ith::ERR_CONFIG_INVALID_LOOPBACK:						errorText = comcli_errormessage::INVALID_LOOPBACK; 					break;
			case fixs_ith::ERR_CONFIG_INVALID_ENCAPSULATION:				errorText = comcli_errormessage::INVALID_ENCAPSULATION;				break;
			case fixs_ith::ERR_CONFIG_DUPLICATED_ENCAPSULATION:				errorText = comcli_errormessage::DUPLICATED_ENCAPSULATION; 			break;
			case fixs_ith::ERR_CONFIG_MISSING_ENCAPSULATION_VID:			errorText = comcli_errormessage::MISSING_ENCAPSULATION_VID; 		break;
			case fixs_ith::ERR_CONFIG_DUPLICATED_LOOPBACK:					errorText = comcli_errormessage::DUPLICATED_LOOPBACK; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_ADDRESS_ID:					errorText = comcli_errormessage::INVALID_ADDRESS_ID; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_ADDRESS:						errorText = comcli_errormessage::INVALID_ADDRESS;					break;
			case fixs_ith::ERR_CONFIG_CONFLICTING_ADDRESS:					errorText = comcli_errormessage::CONFLICTING_ADDRESS; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_DST_ID:						errorText = comcli_errormessage::INVALID_DST_ID; 					break;
			case fixs_ith::ERR_CONFIG_INVALID_DST_IP:						errorText = comcli_errormessage::INVALID_SUBNET_IP; 				break;
			case fixs_ith::ERR_CONFIG_DUPLICATED_DST:						errorText = comcli_errormessage::DUPLICATED_DST; 					break;
			case fixs_ith::ERR_CONFIG_INVALID_NEXTHOP_ID:					errorText = comcli_errormessage::INVALID_NEXTHOP_ID; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_DISCARD:						errorText = comcli_errormessage::INVALID_DISCARD; 					break;
			case fixs_ith::ERR_CONFIG_CONFLICTING_NEXTHOP:					errorText = comcli_errormessage::CONFLICTING_NEXTHOP;				break;
			case fixs_ith::ERR_CONFIG_REFERENCED_OBJECT:					errorText = comcli_errormessage::REFERENCED_OBJECT; 				break;
			case fixs_ith::ERR_CONFIG_INVALID_VRRPSESSION_STATE:			errorText = comcli_errormessage::INVALID_VRRPSESSION_STATE; 		break;
			case fixs_ith::ERR_CONFIG_INVALID_VRRP_ADVERTISE_INTERVAL:		errorText = comcli_errormessage::INVALID_VRRP_ADVERTISE_INTERVAL;	break;
			case fixs_ith::ERR_CONFIG_NOT_UNIQUE_VRID:						errorText = comcli_errormessage::NOT_UNIQUE_VRID; 					break;
			case fixs_ith::ERR_CONFIG_NOT_UNIQUE_ACLENTRY_PRIORITY: 		errorText = comcli_errormessage::NOT_UNIQUE_ACLENTRY_PRIORITY; 		break;
			case fixs_ith::ERR_CONFIG_ACLENTRY_ICMP_IP_MISMATCH: 			errorText = comcli_errormessage::ACLENTRY_ICMP_IP_MISMATCH; 		break;
			case fixs_ith::ERR_CONFIG_ACLENTRY_PORT_IP_MISMATCH:			errorText = comcli_errormessage::ACLENTRY_PORT_IP_MISMATCH; 		break;
			case fixs_ith::ERR_CONFIG_PROTECTED_ADDRESS:					errorText = comcli_errormessage::PROTECTED_ADDRESS;					break;
			case fixs_ith::ERR_CONFIG_ADDRESS_CONFLICT:						errorText = comcli_errormessage::ADDRESS_CONFLICT; 					break;
			case fixs_ith::ERR_CONFIG_CONFLICTING_VRRPSESSION_PRIORITIZED: 	errorText = comcli_errormessage::CONFLICTING_VRRPSESSION_OWNER; 	break;
			case fixs_ith::ERR_CONFIG_BFD_CONFLICT:							errorText = comcli_errormessage::BFD_CONFLICT; 						break;
			case fixs_ith::ERR_BACKUP_IN_PROGRESS:							errorText = comcli_errormessage::BACKUPONGOING_OPNOTPERMITTED; 		break;
			case fixs_ith::ERR_CONFIG_INVALID_BRIDGEPORT_STATE:				errorText = comcli_errormessage::INVALID_BRIDGEPORT_STATE; 			break;

			default:
				errorText = comcli_errormessage::GENERIC;
		}

		return errorText;
	}


} /* namespace */

