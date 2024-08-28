/*
 * configurationManager.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: eanform
 */

#include "engine/configurationManager.h"
#include "engine/configurationReloadHelper.h"

#include "engine/vrrpsession.h"
#include "engine/bfdprofile.h"
#include "engine/bfdsessionipv4.h"
#include "engine/aclentryipv4.h"
#include "engine/alcipv4.h"
#include "operation/pim_adaptation.h"
#include "imm/configurationHelper.h"

#include "common/utility.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"


#include <string>

namespace engine
{

	ContextAccess::ContextAccess(const std::string& id, ContextAccessMode accessM, ContextLockMode lockM): context(0), accessMode(accessM), lockMode(lockM), accessResult(), smxId(id)
	{
		fixs_ith::workingSet_t::instance()->getConfigurationManager().getContext(*this, smxId);
	}

	ContextAccess::~ContextAccess()
	{
		if(context)
		{
			(lockMode ==  EXCLUSIVE_ACCESS)? context->unlock(): context->unlock_shared();
		}
	}

	void ContextAccess::destroyContext()
	{
		if (EXCLUSIVE_ACCESS == lockMode)
		{
			fixs_ith::workingSet_t::instance()->getConfigurationManager().resetContext(smxId);

			context->unlock();
			delete context;
			context = 0;
		}

	}

	void ContextAccess::setRemovContextOngoing()
	{
		if (EXCLUSIVE_ACCESS == lockMode)
		{
			if (context)
			{
				context->setRemoveOngoing();
			}
		}

	}


//////////////////////////////////////////////////////////////////////////////////////////////////

ConfigurationManager::ConfigurationManager() {
	//restricted object name
	{
		m_protectedObjs.insert(
				std::make_pair<fixs_ith::switchboard_plane_t, std::string>(
						fixs_ith::CONTROL_PLANE,
						engine::defaultVlan::control::left::NAME));
		m_protectedObjs.insert(
				std::make_pair<fixs_ith::switchboard_plane_t, std::string>(
						fixs_ith::CONTROL_PLANE,
						engine::defaultVlan::control::right::NAME));
		m_protectedObjs.insert(
				std::make_pair<fixs_ith::switchboard_plane_t, std::string>(
						fixs_ith::TRANSPORT_PLANE,
						engine::defaultVlan::transport::left::NAME));
		m_protectedObjs.insert(
				std::make_pair<fixs_ith::switchboard_plane_t, std::string>(
						fixs_ith::TRANSPORT_PLANE,
						engine::defaultVlan::transport::right::NAME));
		m_protectedObjs.insert(
				std::make_pair<fixs_ith::switchboard_plane_t, std::string>(
						fixs_ith::CONTROL_PLANE,
						engine::tipcVlan::control::NAME));
		m_protectedObjs.insert(
				std::make_pair<fixs_ith::switchboard_plane_t, std::string>(
						fixs_ith::TRANSPORT_PLANE,
						engine::drbdVlan::transport::NAME));
		m_protectedObjs.insert(
				std::make_pair<fixs_ith::switchboard_plane_t, std::string>(
						fixs_ith::TRANSPORT_PLANE,
						engine::apgOamVlan::transport::NAME));
		m_protectedObjs.insert(
				std::make_pair<fixs_ith::switchboard_plane_t, std::string>(
						fixs_ith::TRANSPORT_PLANE,
						engine::evoEtIngress::transport::NAME));
		if (!common::utility::is_hwtype_gep5()) //TR_HY37073
		{

			if (common::utility::isSwitchBoardSMX()) {
				//GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.
				m_protectedObjs.insert(
						std::make_pair<fixs_ith::switchboard_plane_t,
								std::string>(fixs_ith::TRANSPORT_PLANE,
								engine::updVlan::transport::left::NAME));
				m_protectedObjs.insert(
						std::make_pair<fixs_ith::switchboard_plane_t,
								std::string>(fixs_ith::TRANSPORT_PLANE,
								engine::updVlan::transport::right::NAME));
				//GEP2_GEP5 to GEP7 Dynamic UPD VLAN END
			}
			if (common::utility::isSwitchBoardCMX()) {
				//GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.
				m_protectedObjs.insert(
						std::make_pair<fixs_ith::switchboard_plane_t,
								std::string>(fixs_ith::TRANSPORT_PLANE,
								engine::updVlan::transport::cmxleft::NAME));
				m_protectedObjs.insert(
						std::make_pair<fixs_ith::switchboard_plane_t,
								std::string>(fixs_ith::TRANSPORT_PLANE,
								engine::updVlan::transport::cmxright::NAME));
				//GEP2_GEP5 to GEP7 Dynamic UPD VLAN END
			}

		}
	}
}

ConfigurationManager::~ConfigurationManager() {

}

	fixs_ith::ErrorConstants ConfigurationManager::loadConfiguration(const std::string & smxId)
	{
		fixs_ith::ErrorConstants  result = fixs_ith::ERR_NO_ERRORS;

		fixs_ith_switchboardinfo switch_board;
		// Skip the check for common objects
		if(!smxId.empty() && (fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(switch_board, smxId) != fixs_ith_sbdatamanager::SBM_OK)  )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Switch board with key <%s> found in CS configuration", smxId.c_str());
			return fixs_ith::ERR_CS_NO_BOARD_FOUND;
		}

		ConfigurationReloadHelper loader(smxId);

		//Create and lock the context
		contextAccess_t access(smxId, CREATE_IF_NOT_FOUND, EXCLUSIVE_ACCESS);
		if (CONTEXT_ACCESS_ACQUIRED != access.getAccessResult())
		{
			//Internal error
			result = fixs_ith::ERR_GENERIC;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get exclusive access to context <%s>", smxId.c_str());
		}
		else
		{
			if (!loader.reloadConfigurationFromIMM())
			{
				//TODO: Reload error is not propagated. Using an internal error...
				result = fixs_ith::ERR_GENERIC;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Load from IMM failed, error: <%d>", result);

				//Reload from IMM failed. Destroying the context.
				access.destroyContext();
			}
			else
			{
				if (!smxId.empty())
				{
					std::string error_message;
					int op_result = loader.loadConfigurationPatch(error_message);

					if (fixs_ith::ERR_NO_PATCH != op_result && fixs_ith::ERR_NO_ERRORS !=  op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Load of configuration patch for %s failed, error: <%d>\n"
								"error message: %s", smxId.c_str(), result, error_message.c_str());
					}
					else if (fixs_ith::ERR_NO_ERRORS ==  op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_INFO, "Configuration patch successfully loaded for %s\n", smxId.c_str());
					}
				}
			}
		}


		return result;
	}

	fixs_ith::ErrorConstants ConfigurationManager::reloadConfiguration(const std::string & smxId)
	{
		fixs_ith::ErrorConstants  result = fixs_ith::ERR_NO_ERRORS;
		ConfigurationReloadHelper loader(smxId);

		{
			//Get access and lock the context
			contextAccess_t access(smxId, GET_EXISTING, EXCLUSIVE_ACCESS);

			if (CONTEXT_ACCESS_ACQUIRED == access.getAccessResult())
			{
				access.destroyContext();
			}
			else if (CONTEXT_ACCESS_DENIED == access.getAccessResult())
			{
				//TODO
				result = fixs_ith::ERR_GENERIC;
			}
			//else CONTEXT_NOT_FOUND... nothing to do in this case

		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			result = loadConfiguration(smxId);
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationManager::createPatchDirectory()
	{
		if (mkdir(common::PATCH_FOLDER_NAME.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) && (errno != EEXIST)) {
			// ERROR: Creating the directory. Check errno to understand how ith server was restarted
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to 'mkdir()' failed: creating configuration patch "
					"storage directory <%s>: errno == <%d>", common::PATCH_FOLDER_NAME.c_str(), errno);

			return fixs_ith::ERR_MKDIR;
		}
		else if (chmod(common::PATCH_FOLDER_NAME.c_str(), S_IRWXU | S_IRWXG | S_IRWXO))
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to 'chmod()' failed: creating configuration patch "
					"storage directory <%s>: errno == <%d>", common::PATCH_FOLDER_NAME.c_str(), errno);

			return fixs_ith::ERR_MKDIR;
		}

		return fixs_ith::ERR_NO_ERRORS;
	}
	
	////////// HZ29732

	fixs_ith::ErrorConstants ConfigurationManager::setPimAdaptionOfPorts(const std::string& smxId) {

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		if (smxId.empty()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"setPimAdaptionOfPorts failed: no switch board specified");
			return fixs_ith::ERR_GENERIC;

		}

		fixs_ith_switchboardinfo switch_board;
		if (fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
				switch_board, smxId) != fixs_ith_sbdatamanager::SBM_OK) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Switch board with key <%s> not found in CS configuration",
					smxId.c_str());
			return fixs_ith::ERR_CS_NO_BOARD_FOUND;
		}

		contextAccess_t access(smxId.c_str(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if (!context) {
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR,
					"Context '%s' not found or not accessible.", smxId.c_str());
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		fixs_ith::SwitchBoardPlaneConstants planes[] = {fixs_ith::CONTROL_PLANE, fixs_ith::TRANSPORT_PLANE};
		std::list<fixs_ith::SwitchBoardPlaneConstants> planes_list(planes, planes + sizeof(planes)/sizeof(fixs_ith::SwitchBoardPlaneConstants));

		uint16_t ifIndex;
		fixs_ith::switchboard_plane_t sb_plane;

		for(std::list<fixs_ith::SwitchBoardPlaneConstants>::iterator p_it = planes_list.begin(); p_it != planes_list.end(); ++p_it)
		{
			std::set<uint16_t> ifIndexes = context->getEthernetPortsIndexes(*p_it);

			for(std::set<uint16_t>::iterator if_it = ifIndexes.begin(); if_it != ifIndexes.end(); ++if_it)
			{

				ifIndex = *if_it;
				sb_plane = *p_it;

				if((sb_plane == fixs_ith::TRANSPORT_PLANE) && ((ifIndex > ITH_SNMP_COMMUNITY_PIM_PORT_START && ifIndex < ITH_SNMP_COMMUNITY_PIM_PORT_NOT_EXIST)
						|| (ifIndex > ITH_SNMP_COMMUNITY_PIM_PORT_NOT_EXIST && ifIndex <= ITH_SNMP_COMMUNITY_PIM_PORT_END)))
				{
					result = operation::pim_adaptation::m_pim_adaptation(smxId, sb_plane, ifIndex);
				}
				else
				{
					result = operation::pim_adaptation::m_set_auto_neg_admin_status(smxId, sb_plane, ifIndex);
				}
				if(fixs_ith::ERR_NO_ERRORS != result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Call 'm_pim_adaptation()' failed for ifIndex:<%d>",
							smxId.c_str(), common::utility::planeToString(sb_plane).c_str(), ifIndex);
					return result;
				}
			}
		}
		return fixs_ith::ERR_NO_ERRORS;
	}

fixs_ith::ErrorConstants ConfigurationManager::setAdminStateOfDelayedPorts(
		const std::string& smxId, imm::admin_State adminstate) {

	bool isSMX = common::utility::isSwitchBoardSMX();
	bool isCMX = common::utility::isSwitchBoardCMX();
	if (smxId.empty()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"setAdminStateOfDelayedPorts failed: no switch board specified");
		return fixs_ith::ERR_GENERIC;

	}

	fixs_ith_switchboardinfo switch_board;
	if (fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
			switch_board, smxId) != fixs_ith_sbdatamanager::SBM_OK) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Switch board with key <%s> not found in CS configuration",
				smxId.c_str());
		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
	}

	contextAccess_t access(smxId.c_str(), GET_EXISTING, SHARED_ACCESS);

	Context* context = access.getContext();

	if (!context) {
		FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR,
				"Context '%s' not found or not accessible.", smxId.c_str());
		return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
	}

	bool failure_on_control = false, failure_on_transport = false;

	if (isSMX) {

		std::set<boost::shared_ptr<BridgePort> > cp_bridgePortsSet =
				context->getBridgePorts(fixs_ith::CONTROL_PLANE);
		for (std::set<boost::shared_ptr<BridgePort> >::iterator it =
				cp_bridgePortsSet.begin(); it != cp_bridgePortsSet.end();
				++it) {
			imm::admin_State admin_state =
					(adminstate == imm::EMPTY_ADMIN_STATE) ?
							(*it)->getAdminState() : adminstate;

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Setting admin_state for DELAYED port  %s [%d] on Switch LS plane",
					(*it)->getName(), (*it)->getSnmpIndex());

			fixs_ith_switchInterface::op_result op_result =
					context->m_switchInterface.set_if_adminState(smxId,
							(*it)->getPlane(), (*it)->getSnmpIndex(),
							(admin_state == imm::UNLOCKED) ?
									switch_interface::BP_UNLOCKED :
									switch_interface::BP_LOCKED);
			if (fixs_ith_switchInterface::SBIA_OK != op_result) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"[%s][%s] Failed to set admin state on BridgePort <%s>. Switch Interface Error <%d>",
						smxId.c_str(),
						common::utility::planeToString((*it)->getPlane()).c_str(),
						(*it)->getName(), op_result);
				//result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				failure_on_control = true;
			}
		}

	} //End of SMX

	std::set<boost::shared_ptr<BridgePort> > tp_bridgePortsSet =
			context->getBridgePorts(fixs_ith::TRANSPORT_PLANE);
	for (std::set<boost::shared_ptr<BridgePort> >::iterator it =
			tp_bridgePortsSet.begin(); it != tp_bridgePortsSet.end(); ++it) {
		uint16_t snmpIndex = (*it)->getSnmpIndex();

		if (is_early_enabled_if(fixs_ith::TRANSPORT_PLANE, snmpIndex))
			continue;

		imm::admin_State admin_state =
				(adminstate == imm::EMPTY_ADMIN_STATE) ?
						(*it)->getAdminState() : adminstate;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"Setting admin_state for DELAYED port  %s [%d] on Switch HS plane",
				(*it)->getName(), snmpIndex);

		fixs_ith_switchInterface::op_result op_result =
				context->m_switchInterface.set_if_adminState(smxId,
						(*it)->getPlane(), snmpIndex,
						(admin_state == imm::UNLOCKED) ?
								switch_interface::BP_UNLOCKED :
								switch_interface::BP_LOCKED);
		if (fixs_ith_switchInterface::SBIA_OK != op_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"[%s][%s] Failed to set admin state on BridgePort <%s>. Switch Interface Error <%d>",
					smxId.c_str(),
					common::utility::planeToString((*it)->getPlane()).c_str(),
					(*it)->getName(), op_result);
			//result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			failure_on_transport = true;
		}

		if (isCMX) {
			fixs_ith_switchInterface::op_result op_result =
					context->m_switchInterface.set_if_priority(smxId,
							(*it)->getPlane(), snmpIndex);
			if (fixs_ith_switchInterface::SBIA_OK != op_result) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"[%s][%s] Failed to set port priority on BridgePort <%s>. Switch Interface Error <%d>",
						smxId.c_str(),
						common::utility::planeToString((*it)->getPlane()).c_str(),
						(*it)->getName(), op_result);
				//result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				failure_on_transport = true;

			}
		}
	}

	if (isSMX) {
		if (!failure_on_control && !failure_on_transport)
			return fixs_ith::ERR_NO_ERRORS;
		if (failure_on_control)
			return fixs_ith::ERR_SET_CONTROL_ADMIN_STATE;
		if (failure_on_transport)
			return fixs_ith::ERR_SET_TRANSPORT_ADMIN_STATE;
		return fixs_ith::ERR_SET_ADMIN_STATE;
	} else if (isCMX) {
		if (!failure_on_transport)
			return fixs_ith::ERR_NO_ERRORS;
		if (failure_on_transport)
			return fixs_ith::ERR_SET_TRANSPORT_ADMIN_STATE;
		return fixs_ith::ERR_SET_ADMIN_STATE;
	} else
		return fixs_ith::ERR_NO_ERRORS;
}


	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::bridgeInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<Bridge> bridge = boost::make_shared<Bridge>(data);

		if ( NULL == bridge )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Bridge '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}


		if (NULL != context && NULL != bridge)
		{
			fixs_ith::ErrorConstants result = context->setBridge(bridge);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to add Bridge <%s> to context. Error <%d> ",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added Bridge: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::bridgePortInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<BridgePort> bridgePort = boost::make_shared<BridgePort>(data);

		if ( NULL == bridgePort )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Bridge '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != bridgePort)
		{
			std::string portName = common::utility::buildPortName(data.getSmxId(), data.getPlane(), data.getName());

			boost::shared_ptr<EthernetPort> ethernetPort = context->getEthernetPortByName(data.getPlane(), portName);

			boost::shared_ptr<Aggregator> aggregator = context->getAggregatorByName(data.getPlane(), portName);

			if (NULL != ethernetPort)
			{
				bridgePort->setEthernetPort(ethernetPort);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Associated EthernetPort <%s> to BridgePort <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(),
						ethernetPort->getName(), data.getName());

				fixs_ith::ErrorConstants result = context->setBridgePort(bridgePort);

				if (fixs_ith::ERR_NO_ERRORS != result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to add BridgePort <%s> to context. Error <%d>",
							data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
					return result;
				}

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added BridgePort: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
			}
			else if (NULL != aggregator)
			{
				bridgePort->setAggregator(aggregator);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Associated Aggregator <%s> to BridgePort <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(),
										aggregator->getName(), data.getName());

				fixs_ith::ErrorConstants result = context->setBridgePort(bridgePort);

				if (fixs_ith::ERR_NO_ERRORS != result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to add BridgePort <%s> to context. Error <%d>",
							data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
					return result;
				}

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added BridgePort: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] No EthernetPort or Aggregator found for BridgePort <%s>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
				return fixs_ith::ERR_CONFIG_INVALID_BRIDGE_PORT_ID;
			}

		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::ethernetPortInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<EthernetPort> ethernetPort = boost::make_shared<EthernetPort>(data);

		if ( NULL == ethernetPort )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for EthernetPort '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != ethernetPort)
		{
			if (!context->hasBridge(ethernetPort->getPlane()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] No Bridge found for EthernetPort <%s>",
										data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
				return fixs_ith::ERR_CONFIG_INVALID_BRIDGE_ID;
			}

			fixs_ith::ErrorConstants result = context->setEthernetPort(ethernetPort);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to add EthernetPort <%s> to context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}


			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added EthernetPort: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::aggregatorInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<Aggregator> aggregator = boost::make_shared<Aggregator>(data);

		if ( NULL == aggregator )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Aggregator '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != aggregator)
		{
			fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

			if (fixs_ith::ERR_NO_ERRORS != (result = assignEthernetPorts(aggregator, context)))
			{
				return result;
			}

			result = context->setAggregator(aggregator);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to add Aggregator <%s> to context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added Aggregator: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::vlanInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<Vlan> vlan = boost::make_shared<Vlan>(data);

		if ( NULL == vlan )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Vlan '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != vlan)
		{
			fixs_ith::ErrorConstants result  = fixs_ith::ERR_NO_ERRORS;

			if (imm::vlanId_range::UNDEFINED != vlan->getVlanId() && context->hasVlan(vlan->getPlane(), vlan->getVlanId()))
			{

                           if ((vlan->getName() == engine::updVlan::transport::left::NAME) || (vlan->getName() == engine::updVlan::transport::right::NAME))
                           {
                              if (!(context->hasVlan(vlan->getPlane(),"APZ-UPD")))
                              {

                                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot set vlan ID on Vlan <%s>: ID <%d> is already used",
                                                data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), data.getVlanId());
                                return fixs_ith::ERR_CONFIG_DUPLICATED_VLAN_ID;
                        
                              }
		           }								
                           else
                           {
                        
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot set vlan ID on Vlan <%s>: ID <%d> is already used",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), data.getVlanId());
				return fixs_ith::ERR_CONFIG_DUPLICATED_VLAN_ID;
                            }
		       }

                        //GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature and TR_HY37073 
                        if ( common::utility::is_hwtype_gep5())
                        {

                                if ( (vlan->getName() == engine::updVlan::transport::left::NAME) && (vlan->getPlane() == fixs_ith::TRANSPORT_PLANE))
                                {
                                        FIXS_ITH_LOG(LOG_LEVEL_WARN, "[%s][%s] Cannot create vlan APZ-UPD-A <%s>: ID <%d> is protected",
                                                        data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), data.getVlanId());
                                        return fixs_ith::ERR_GENERIC;
                                }

                                if ( (vlan->getName() == engine::updVlan::transport::right::NAME) && (vlan->getPlane() == fixs_ith::TRANSPORT_PLANE))
                                {
                                        FIXS_ITH_LOG(LOG_LEVEL_WARN, "[%s][%s] Cannot create vlan APZ-UPD-B <%s>: ID <%d> is protected",
                                                        data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), data.getVlanId());
                                        return fixs_ith::ERR_GENERIC;
                                }

                                if ((vlan->getName() == engine::updVlan::transport::cmxleft::NAME) && (vlan->getPlane() == fixs_ith::TRANSPORT_PLANE))
                                {
                                        FIXS_ITH_LOG(LOG_LEVEL_WARN, "Cannot create vlan APZ-UPD-A <%s>: ID <%d> is protected",
                                                        data.getName(), data.getVlanId());
                                        return fixs_ith::ERR_GENERIC;
                                }

                                if ( (vlan->getName() == engine::updVlan::transport::cmxright::NAME) && (vlan->getPlane() == fixs_ith::TRANSPORT_PLANE))
                                {
                                        FIXS_ITH_LOG(LOG_LEVEL_WARN, "Cannot create vlan APZ-UPD-B <%s>: ID <%d> is protected",
                                                        data.getName(), data.getVlanId());
                                        return fixs_ith::ERR_GENERIC;
                                }

                        }
                        //GEP2_GEP5 to GEP7 Dynamic UPD VLAN END and TR_HY37073 

			if (fixs_ith::ERR_NO_ERRORS != (result = assignTaggedPorts(vlan, context)))
			{
				return result;
			}

			if (fixs_ith::ERR_NO_ERRORS != (result = assignUntaggedPorts(vlan, context)))
			{
				return result;
			}

			/* HX22060 BEGIN */
			if (vlan->getPlane() == fixs_ith::TRANSPORT_PLANE)
			{
				FIXS_ITH_LOG(LOG_LEVEL_WARN,"This VLAN is created on transport plane");
				std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts_added = vlan->getUntaggedBridgePorts();
				for ( std::set< boost::shared_ptr<BridgePort> >::iterator it = untaggedBridgePorts_added.begin (); it != untaggedBridgePorts_added.end(); ++it)
				{
					std::cout << " fixs_ith::ErrorConstants ConfigurationManager::add added untagged port " << (*it)-> getName () << std::endl;
					FIXS_ITH_LOG(LOG_LEVEL_WARN, " Added untagged port : is <%s>", (*it)-> getName ()); 

					if ((*it)->getSnmpIndex() >= ITH_SNMP_COMMUNITY_PIM_PORT_START && (*it)->getSnmpIndex() <= ITH_SNMP_COMMUNITY_PIM_PORT_END )
					{
						if (context->IsUntaggedPort((*it)->getPlane(), (*it)->getBridgePortDN()))
						{
							std::cout << "fixs_ith::ErrorConstants ConfigurationManager::add added untagged port is SFP port and have already a VLAN " << std::endl;
							FIXS_ITH_LOG(LOG_LEVEL_WARN, "SFP Port <%s> have untagged Vlan",(*it)-> getName ());
							return fixs_ith::ERR_CONFIG_INVALID_UNTAGGED_PORT;
						}		    
					}
				}
			}
			/* HX22060 END */
			 
//			for (std::set<boost::shared_ptr<BridgePort> >::iterator element = untaggedPorts.begin(); element != untaggedPorts.end(); ++element)
//			{
//				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Set Default VLAN ID <%u> on BridgePort <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(),
//						data.getVlanId(), (*element)->getName());
//
//				(*element)->setDefaultVlanId(data.getVlanId());
//			}


			result = context->setVlan(vlan);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to add Vlan <%s> to context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added Vlan: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());


                       //GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.

                       if ((vlan->getName() == engine::updVlan::transport::left::NAME) || (vlan->getName() == engine::updVlan::transport::right::NAME) || (vlan->getName() == engine::updVlan::transport::cmxleft::NAME) || (vlan->getName() == engine::updVlan::transport::cmxright::NAME))
                      {
		        FIXS_ITH_LOG(LOG_LEVEL_WARN, "Calling createVlanEntryInAxeequipment");
                        if(createVlanEntryInAxeequipment(data.name,data.vId)){
                            FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Vlan instance for vlanID %d is created under AxeEquipment",data.vId);
                        }
		        else{
		            FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Failed to create Vlan instance for vlanID %d under AxeEquipment",data.vId);
		        }


                        if (replaceVlanIdInAxeequipment(data.name,data.getPlane(),data.vId))
                        {
                                FIXS_ITH_LOG(LOG_LEVEL_WARN,"Replaced VLAN in AxeEquipment %s", data.getName());
                        }
                        else
                        {
                                FIXS_ITH_LOG(LOG_LEVEL_WARN,"Failed to replace VLAN in AxeEquipment %s", data.getName());
                        }


                      }
                       //GEP2_GEP5 to GEP7 Dynamic UPD VLAN END

		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::subnetVlanInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<SubnetVlan> subnetVlan = boost::make_shared<SubnetVlan>(data);

		if ( NULL == subnetVlan )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for SubnetVlan '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != subnetVlan)
		{
			boost::shared_ptr<Vlan> vlan = context->getVlanById(data.getPlane(), data.getSubnetVlanId());

			if (NULL == vlan)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Vlan ID <%d> not found for Subnet Based Vlan <%s>@<%s>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getSubnetVlanId(), data.getName(), data.getIpSubnet());
				return fixs_ith::ERR_CONFIG_INVALID_VLAN_ID;
			}

			if (context->hasIpv4SubnetBasedVlan(data.getPlane(), data.getIpSubnet()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot set Ipv4Subnet on SubnetVlan <%s>: Ipv4Subnet <%s> is already used",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), data.getIpSubnet());
				return fixs_ith::ERR_CONFIG_DUPLICATED_SUBNET_VLAN_IP;
			}

			subnetVlan->setVlan(vlan);

			fixs_ith::ErrorConstants result = context->setSubnetVlan(subnetVlan);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to add SubnetVlan <%s> to context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added SubnetVlan: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::routerInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<Router> router = boost::make_shared<Router>(data);

		if ( NULL == router )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Router '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}


		if (NULL != context && NULL != router)
		{
			fixs_ith::ErrorConstants result = context->setRouter(router);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to add Router <%s> to context. Error <%d> ",
						data.getSmxId(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Added Router: <%s>", data.getSmxId(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}


	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::interfaceInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<InterfaceIPv4> interface = boost::make_shared<InterfaceIPv4>(data);

		if ( NULL == interface )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for InterfaceIPv4 '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != interface)
		{
			boost::shared_ptr<Router> router = context->getRouterByName(interface->getRouterName());

			if (NULL == router)
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Router <%s> not found for interface <%s>",
						data.getSmxId(), interface->getRouterName(), data.getName());
				return fixs_ith::ERR_CONFIG_INVALID_ROUTER_NAME;
			}

			if (interface->isLoopback() == interface->hasEncapsulation())
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Loopback or encapsulation must be set for Interface <%s>",
						interface->getSmxId(), interface->getRouterName(), interface->getName());
				return fixs_ith::ERR_CONFIG_INVALID_LOOPBACK;

			}

			if (interface->isLoopback() && context->hasLoopback(interface->getRouterName()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot define multiple loopback interfaces on the same Router",
						interface->getSmxId(), interface->getRouterName());
				return fixs_ith::ERR_CONFIG_DUPLICATED_LOOPBACK;
			}

			if (interface->hasEncapsulation())
			{

				boost::shared_ptr<Vlan> encapsulationVlan = context->getVlanByName(fixs_ith::TRANSPORT_PLANE, common::utility::getIdValueFromRdn(interface->getEncapsulationDN()));

				if (NULL == encapsulationVlan)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] VLAN not found for SMX ID <%s> for encapsulation of Interface <%s>",
							interface->getSmxId(), interface->getRouterName(), interface->getSmxId(),
							interface->getName());
					return fixs_ith::ERR_CONFIG_INVALID_ENCAPSULATION;
				}

				if (imm::vlanId_range::UNDEFINED == encapsulationVlan->getVlanId())
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Specified VLAN encapsulation for interface <%s> does not have a valid VLAN ID",
							interface->getSmxId(), interface->getRouterName(),
							interface->getName());
					return fixs_ith::ERR_CONFIG_MISSING_ENCAPSULATION_VID;
				}

				if (context->isEncapsulation(interface->getEncapsulationDN()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Specified VLAN encapsulation for interface <%s> is already used as encapsulation",
							interface->getSmxId(), interface->getRouterName(),
							interface->getName());
					return fixs_ith::ERR_CONFIG_DUPLICATED_ENCAPSULATION;
				}

				interface->setEncapsulation(encapsulationVlan);

			}

			if ( data.bfdProfile.empty() && ( imm::ENABLED == data.bfdStaticRoutes ) )
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to add InterfaceIPv4 <%s> to context. bfdStaticRoutes cannot be ENABLED "
						"while bfdProfile is NULL", data.getSmxId(), data.getName());
				return fixs_ith::ERR_CONFIG_BFD_CONFLICT;
			}

			fixs_ith::ErrorConstants result = context->setInterface(interface);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to add InterfaceIPv4 <%s> to context. Error <%d> ",
						data.getSmxId(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Added InterfaceIPv4: <%s>", data.getSmxId(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::addressInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<Address> address = boost::make_shared<Address>(data);

		if ( NULL == address )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Address '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != address)
		{

			if (!address->isVRRPInterfaceAddress())
			{
				boost::shared_ptr<Router> router = context->getRouterByName(address->getRouterName());

				if (NULL == router)
				{
					//Error
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Router <%s> not found for address <%s>",
							data.getSmxId(), address->getRouterName(), data.getName());
					return fixs_ith::ERR_CONFIG_INVALID_ROUTER_NAME;
				}

				boost::shared_ptr<InterfaceIPv4> interface = context->getInterfaceByName(address->getRouterName(), address->getInterfaceName());

				if (NULL == interface)
				{
					//Error
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Interface <%s> not found for address <%s>",
							data.getSmxId(), address->getRouterName(), address->getInterfaceName(), data.getName());
					return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_ID;
				}

				if (context->hasConflictingRouterInterfaceAddress(address->getRouterName(), address->getAddress()))
				{
					//Error
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Address <%s> confilcts with another address already configured on the same Router",
							data.getSmxId(), address->getRouterName(), address->getAddress());
					return fixs_ith::ERR_CONFIG_CONFLICTING_ADDRESS;

				}

				fixs_ith::ErrorConstants result = context->setRouterInterfaceAddress(address);

				if (fixs_ith::ERR_NO_ERRORS != result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to add Address <%s> to context. Error <%d> ",
							data.getSmxId(), data.getName(), result);
					return result;
				}

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Added AddressIPv4: <%s>", data.getSmxId(), data.getName());
			}
			else
			{
				boost::shared_ptr<VrrpInterface> vrrpInterface = context->getVrrpInterfaceByName(address->getInterfaceName());

				if (NULL == vrrpInterface)
				{
					//Error
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "VRRP Interface <%s> not found for address <%s>",
							address->getInterfaceName(), data.getName());
					return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_ID;
				}

				fixs_ith::ErrorConstants result = context->setVrrpInterfaceAddress(address);

				if (fixs_ith::ERR_NO_ERRORS != result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add Address <%s> to context. Error <%d> ",
							data.getSmxId(), data.getName(), result);
					return result;
				}

				// Set vrrpInterfaceInfo address attribute
				vrrpInterface->setAddressIPv4(address->getData().address);
			}
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::dstInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<Dst> dst = boost::make_shared<Dst>(data);

		if ( NULL == dst )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Dst '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != dst)
		{

			boost::shared_ptr<Router> router = context->getRouterByName(dst->getRouterName());

			if (NULL == router)
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Router <%s> not found for dst <%s>",
						data.getSmxId(), dst->getRouterName(), data.getName());
				return fixs_ith::ERR_CONFIG_INVALID_ROUTER_NAME;
			}

			if ((data.changeMask & imm::dst_attribute::DST_CHANGE) &&
					context->hasDst(dst->getRouterName(), dst->getDst()))
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Another Dst with dst <%s> already exists",
						data.getSmxId(), dst->getRouterName(), data.getDst());
				return fixs_ith::ERR_CONFIG_DUPLICATED_DST;
			}

			fixs_ith::ErrorConstants result = context->setDst(dst);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to add Dst <%s> to context. Error <%d> ",
						data.getSmxId(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Added Dst: <%s>", data.getSmxId(), data.getName());

		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::nextHopInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<NextHop> nextHop = boost::make_shared<NextHop>(data);

		if ( NULL == nextHop )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for NextHop '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != nextHop)
		{
			boost::shared_ptr<Router> router = context->getRouterByName(nextHop->getRouterName());

			if (NULL == router)
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Router <%s> not found for NextHop <%s>",
						data.getSmxId(), nextHop->getRouterName(), data.getName());
				return fixs_ith::ERR_CONFIG_INVALID_ROUTER_NAME;
			}

			if (nextHop->isDiscard() == nextHop->hasAddress())
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s][%s] Discard or address must be set for NextHop <%s>",
						nextHop->getSmxId(), nextHop->getRouterName(), nextHop->getDstName(), nextHop->getName());
				return fixs_ith::ERR_CONFIG_INVALID_DISCARD;

			}

			if (nextHop->hasAddress() &&
					context->hasRouterInterfaceAddress(nextHop->getRouterName(), nextHop->getAddress()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s][%s] NextHop address <%s> matches an IP address of the same router",
						nextHop->getSmxId(), nextHop->getRouterName(), nextHop->getDstName(), nextHop->getAddress());
				return fixs_ith::ERR_CONFIG_CONFLICTING_NEXTHOP;

			}

			fixs_ith::ErrorConstants result = context->setNextHop(nextHop);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to add NextHop <%s> to context. Error <%d> ",
						data.getSmxId(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s][%s] Added NextHop: <%s>", data.getSmxId(), nextHop->getRouterName(), nextHop->getDstName(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::vrrpInterfaceInfo& data )
	{
		contextAccess_t access(data.getSmxId(), CREATE_IF_NOT_FOUND, SHARED_ACCESS);

		Context* context = access.getContext();

		boost::shared_ptr<VrrpInterface> vrrpInterface = boost::make_shared<VrrpInterface>(data);

		if ( NULL == vrrpInterface )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Cannot allocate memory for VrrpInterface '%s'", data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if (NULL != context && NULL != vrrpInterface)
		{
			fixs_ith::ErrorConstants result = context->setVrrpInterface(vrrpInterface);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add VrrpInterface <%s> to context. Error <%d> ", data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added VrrpInterface: <%s>", data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::vrrpSessionInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<VrrpSession> vrrpSession = boost::make_shared<VrrpSession>(data);

		if(NULL == vrrpSession )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for VRRP SESSION:<%s> DN:<%s>", data.getSmxId(), data.getName(), data.getDN() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		boost::shared_ptr<Router> router = context->getRouterByName(vrrpSession->getRouterName());

		if(NULL == router)
		{
			//Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Router <%s> not found for VrrpSession:<%s>",
					data.getSmxId(), vrrpSession->getRouterName(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_ROUTER_NAME;
		}

		// get the reference to the Vrrp Interface
		boost::shared_ptr<VrrpInterface> vrrpInterface;

		{
			//Empty value
			std::string commonElementId;
			engine::ContextAccess access(commonElementId, GET_EXISTING, SHARED_ACCESS);
			Context* context = access.getContext();

			vrrpInterface = context->getVrrpInterfaceByName(vrrpSession->getVrrpInterfaceName());
		}

		if(NULL == vrrpInterface)
		{
			//Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] VRRP Interface <%s> not found for VrrpSession:<%s>",
					data.getSmxId(), vrrpSession->getVrrpInterfaceName(), vrrpSession->getName());
			return fixs_ith::ERR_CONFIG_INVALID_VRRP_INTERFACE_NAME;
		}

		// set the Vrrp Interface attributes
		vrrpSession->setVrrpInterfaceInfo(vrrpInterface->getData());

		// Check for unique prioritized session
		std::set<std::string> otherVrrpSessionDn = vrrpInterface->getData().getReservedBy();

		for (std::set<std::string>::iterator it = otherVrrpSessionDn.begin(); it != otherVrrpSessionDn.end(); ++it)
		{
			std::string otherSmxId = common::utility::getSmxIdFromVrrpv3SessionDN(*it);
			contextAccess_t otherContextaccess(otherSmxId, GET_EXISTING, SHARED_ACCESS);

			Context* otherContext = otherContextaccess.getContext();

			if (otherContext)
			{
				std::string otherRouterName = common::utility::getRouterNameFromVrrpSessionDN(*it);
				std::string otherInterfaceName = common::utility::getInterfaceNameFromVrrpSessionDN(*it);
				std::string otherVrrpSessionName = common::utility::getIdValueFromRdn(*it);

				boost::shared_ptr<VrrpSession> otherVrrpSession = otherContext->getVrrpSessionByName(otherRouterName, otherInterfaceName, otherVrrpSessionName);

				if (NULL != otherVrrpSession)
				{
					if(vrrpSession->getData().getPrioritizedSession() == otherVrrpSession->getData().getPrioritizedSession())
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] VRRP prioritized Session already defined or missing for VRRP Interface:<%s>",
								data.getSmxId(), vrrpInterface->getName());
						return fixs_ith::ERR_CONFIG_CONFLICTING_VRRPSESSION_PRIORITIZED;
					}
				}
			}

		}


		fixs_ith::ErrorConstants result = context->setVrrpSession(vrrpSession);

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to add Vrrp Session <%s> to context. Error <%d> ",
					data.getSmxId(), data.getName(), result);
			return result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s][%s] Added Vrrp Session: <%s>", data.getSmxId(), vrrpSession->getRouterName(), vrrpSession->getInterfaceName(), vrrpSession->getName());

		return fixs_ith::ERR_NO_ERRORS;

	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::bfdProfileInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<BfdProfile> bfdProfile = boost::make_shared<BfdProfile>(data);

		if(NULL == bfdProfile )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for BFD Profile:<%s> DN:<%s>", data.getSmxId(), data.getName(), data.getDN() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (NULL != context && NULL != bfdProfile)
		{
			boost::shared_ptr<Router> router = context->getRouterByName(bfdProfile->getRouterName());

			if (NULL == router)
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Router <%s> not found for bfd profile <%s>",
						data.getSmxId(), bfdProfile->getRouterName(), data.getName());
				return fixs_ith::ERR_CONFIG_INVALID_ROUTER_NAME;
			}

			fixs_ith::ErrorConstants result = context->setBfdProfile(bfdProfile);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to add BfdProfile <%s> to context. Error <%d> ",
						data.getSmxId(), bfdProfile->getRouterName(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added BfdProfile <%s>", data.getSmxId(), bfdProfile->getRouterName(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::bfdSessionInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context:<%s> not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<BfdSessionIPv4> bfdSession = boost::make_shared<BfdSessionIPv4>(data);

		if( NULL == bfdSession )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for BFD Session:<%s> DN:<%s>", data.getSmxId(), data.getName(), data.getDN() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		return context->setBfdSessionIPv4(bfdSession);
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::aclEntryInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context:<%s> not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<AclEntryIpv4> aclEntry = boost::make_shared<AclEntryIpv4>(data);

		if( NULL == aclEntry )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for AclEntryIpv4:<%s> DN:<%s>", data.getSmxId(), data.getName(), data.getDN());
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		return context->setAclEntryIpv4(aclEntry);
	}

	fixs_ith::ErrorConstants ConfigurationManager::add(const operation::aclIpV4Info& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context:<%s> not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<AclIpv4> aclIpV4 = boost::make_shared<AclIpv4>(data);

		if( NULL == aclIpV4 )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for ACL IpV4:<%s> DN:<%s>", data.getSmxId(), data.getName(), data.getDN());
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		return context->setAclIpV4(aclIpV4);
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::bridgeInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<Bridge> oldObject = context->getBridgeByName(data.getPlane(), data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Bridge to modify not found: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_BRIDGE_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<Bridge> modifiedObject = boost::make_shared<Bridge>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Bridge '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			fixs_ith::ErrorConstants result = context->replaceBridge(modifiedObject);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace Bridge <%s> in context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced Bridge: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::bridgePortInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<BridgePort> oldObject = context->getBridgePortByName(data.getPlane(), data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] BridgePort to modify not found: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_BRIDGE_PORT_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<BridgePort> modifiedObject = boost::make_shared<BridgePort>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for BridgePort '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			fixs_ith::ErrorConstants result = context->replaceBridgePort(modifiedObject);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace BridgePort <%s> in context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced BridgePort: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

			std::set<boost::shared_ptr<Vlan> > vlans = context->getVlans(data.getPlane());

			for (std::set<boost::shared_ptr<Vlan> >::iterator it = vlans.begin(); it != vlans.end(); ++it)
			{
				if ((*it)->hasTaggedPortDN(modifiedObject->getBridgePortDN()) )
				{
					(*it)->replaceTaggedBridgePort(modifiedObject);
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced TAGGED BridgePort: <%s> in Vlan <%s>",
							data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), (*it)->getName());
				}
				else if ((*it)->hasUntaggedPortDN(modifiedObject->getBridgePortDN()) )
				{
					(*it)->replaceUntaggedBridgePort(modifiedObject);
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced UNTAGGED BridgePort: <%s> in Vlan <%s>",
							data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), (*it)->getName());
				}
			}
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::ethernetPortInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<EthernetPort> oldObject = context->getEthernetPortByName(data.getPlane(), data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] EthernetPort to modify not found: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_ETHERNET_PORT_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<EthernetPort> modifiedObject = boost::make_shared<EthernetPort>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for EthernetPort '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			fixs_ith::ErrorConstants result = context->replaceEthernetPort(modifiedObject);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace EthernetPort <%s> in context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced EthernetPort: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

			//Update BridgePort
			std::string portName = common::utility::getBridgePortNameFromPortName(data.getName());

			boost::shared_ptr<BridgePort> bridgePort = context->getBridgePortByName(data.getPlane(), portName);

			if (NULL != bridgePort)
			{
				bridgePort->setEthernetPort(modifiedObject);
			}
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::aggregatorInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<Aggregator> oldObject = context->getAggregatorByName(data.getPlane(), data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Aggregator to modify not found: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_AGGREGATOR_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<Aggregator> modifiedObject = boost::make_shared<Aggregator>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Aggregator '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		//Apply modification
		if (modifiedObject->modify(data))
		{
			//Validate Aggregator
			if (data.changeMask & imm::aggregator_attribute::ADMIN_AGG_MEMBER_CHANGE)
			{
				fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

				if (fixs_ith::ERR_NO_ERRORS != (result = assignEthernetPorts(modifiedObject, context)))
				{
					return result;
				}
			}

			fixs_ith::ErrorConstants result = context->replaceAggregator(modifiedObject);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace Aggregator <%s> in context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced Aggregator: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

			//Update BridgePort
			std::string portName = common::utility::getBridgePortNameFromPortName(data.getName());

			boost::shared_ptr<BridgePort> bridgePort = context->getBridgePortByName(data.getPlane(), portName);

			if (NULL != bridgePort)
			{
				bridgePort->setAggregator(modifiedObject);
			}
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::vlanInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<Vlan> oldObject = context->getVlanByName(data.getPlane(), data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Vlan to modify not found: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_VLAN_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<Vlan> modifiedObject = boost::make_shared<Vlan>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Vlan '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			//Validate Vlan
			fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;			
			
			if (data.changeMask &
					imm::vlan_attribute::VLAN_ID_CHANGE )
			{
				if (isProtectedObject(oldObject->getPlane(), oldObject->getName()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot delete/modify Vlan <%s>. It's a protected Vlan object.",
							oldObject->getSmxId(), common::utility::planeToString(oldObject->getPlane()).c_str(), oldObject->getName());
					return fixs_ith::ERR_CONFIG_PROTECTED_OBJECT;
				}
			}


			if ((data.changeMask & imm::vlan_attribute::VLAN_ID_CHANGE)
					&& modifiedObject->getVlanId() != oldObject->getVlanId())
			{
                                std::string vlanName = data.getName();

				if (imm::vlanId_range::UNDEFINED != oldObject->getVlanId())
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot assign new vlan ID <%d> to Vlan <%s> which already has vlan ID <%d>",
							data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getVlanId(), data.getName(), oldObject->getVlanId());
					return fixs_ith::ERR_CONFIG_RESTRICTED_VLAN_ID;
                                 }     

				if (context->hasVlan(modifiedObject->getPlane(), modifiedObject->getVlanId()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot set vlan ID on Vlan <%s>: ID <%d> is already used",
							data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), data.getVlanId());
					return fixs_ith::ERR_CONFIG_DUPLICATED_VLAN_ID;
				}
			}

			if (data.changeMask & imm::vlan_attribute::TAGGED_PORT_CHANGE)
			{
				if (fixs_ith::ERR_NO_ERRORS != (result = assignTaggedPorts(modifiedObject, context)))
				{
					return result;
				}
			}

			if (data.changeMask & imm::vlan_attribute::UNTAGGED_PORT_CHANGE)
			{
				if (fixs_ith::ERR_NO_ERRORS != (result = assignUntaggedPorts(modifiedObject, context)))
				{
					return result;
				}

				/* HX22060 BEGIN
				 * 
				 */
				if (modifiedObject->getPlane() == fixs_ith::TRANSPORT_PLANE)
				{
					FIXS_ITH_LOG(LOG_LEVEL_WARN,"This VLAN is created on transport plane");
					std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts_old = oldObject->getUntaggedBridgePorts();
					std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts_modified = modifiedObject->getUntaggedBridgePorts();

					std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts_added;
					std::set_difference(untaggedBridgePorts_modified.begin(), untaggedBridgePorts_modified.end(), untaggedBridgePorts_old.begin(), untaggedBridgePorts_old.end(), std::inserter(untaggedBridgePorts_added, untaggedBridgePorts_added.begin()));

					for ( std::set< boost::shared_ptr<BridgePort> >::iterator it = untaggedBridgePorts_added.begin (); it != untaggedBridgePorts_added.end(); ++it)
					{
						std::cout << " fixs_ith::ErrorConstants ConfigurationManager::change added untagged port " << (*it)-> getName () << std::endl;
						FIXS_ITH_LOG(LOG_LEVEL_WARN, " Added untagged port : is <%s>", (*it)-> getName ()); 

						if ((*it)->getSnmpIndex() >= ITH_SNMP_COMMUNITY_PIM_PORT_START && (*it)->getSnmpIndex() <= ITH_SNMP_COMMUNITY_PIM_PORT_END )
						{
							if (context->IsUntaggedPort((*it)->getPlane(), (*it)->getBridgePortDN()))
							{
								std::cout << "fixs_ith::ErrorConstants ConfigurationManager::change This SFP port have already a VLAN " << std::endl;
								FIXS_ITH_LOG(LOG_LEVEL_WARN, "SFP Port <%s> have untagged Vlan",(*it)-> getName ());
								return fixs_ith::ERR_CONFIG_INVALID_UNTAGGED_PORT;
							}		    
						}
					}
				}
				/* HX22060 END
				 * 
				 */
			}

			result = context->replaceVlan(modifiedObject);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace Vlan <%s> in context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced Vlan: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
                        
                        FIXS_ITH_LOG(LOG_LEVEL_WARN,"Replacing VLAN in AxeEquipment %s", data.getName());
                        std::string modifiedVlanName = data.getName();
                        int modifiedVlanId = modifiedObject->getVlanId();
                        if (replaceVlanIdInAxeequipment(modifiedVlanName,data.getPlane(),modifiedVlanId))
                        {
                          FIXS_ITH_LOG(LOG_LEVEL_WARN,"Replaced VLAN in AxeEquipment %s", data.getName());
                        }
                        else
                        {
                          FIXS_ITH_LOG(LOG_LEVEL_WARN,"Failed to replace VLAN in AxeEquipment %s", data.getName());
                        }
			//Update SubnetVlan
			boost::shared_ptr<SubnetVlan> subnetVlan = context->getSubnetVlanById(data.getPlane(), data.getVlanId());

			if (NULL != subnetVlan)
			{
				subnetVlan->setVlan(modifiedObject);
			}

			//update InterfaceIPv4
			if (modifiedObject->isReserved())
			{
				std::set<std::string> reservedByDNs = modifiedObject->getReservedBy();
				for (std::set<std::string>::iterator it = reservedByDNs.begin(); it != reservedByDNs.end(); ++it)
				{
					std::string routerName = common::utility::getRouterNameFromInterfaceDN(*it);
					std::string interfaceName = common::utility::getIdValueFromRdn(*it);

					boost::shared_ptr<InterfaceIPv4> interface = context->getInterfaceByName(routerName, interfaceName);

					if (NULL != interface)
					{
						interface->setEncapsulation(modifiedObject);
					}
				}
			}

		}
		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::subnetVlanInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<SubnetVlan> oldObject = context->getSubnetVlanByName(data.getPlane(), data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] SubnetVlan to modify not found: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_SUBNET_VLAN_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<SubnetVlan> modifiedObject = boost::make_shared<SubnetVlan>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for SubnetVlan '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			//Validate SubnetVlan

			if (data.changeMask & imm::subnetVlan_attribute::VLAN_ID_CHANGE)
			{
				boost::shared_ptr<Vlan> vlan = context->getVlanById(data.getPlane(), data.getSubnetVlanId());

				if (NULL == vlan)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Vlan ID <%d> not found for Subnet Based Vlan <%s>@<%s>",
							data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getSubnetVlanId(), data.getName(), data.getIpSubnet());
					return fixs_ith::ERR_CONFIG_INVALID_VLAN_ID;
				}

				modifiedObject->setVlan(vlan);
			}

			fixs_ith::ErrorConstants result = context->replaceSubnetVlan(modifiedObject);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace SubnetVlan <%s> in context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced SubnetVlan: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::interfaceInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		std::string routerName(common::utility::getRouterNameFromInterfaceDN(data.getDN()));

		boost::shared_ptr<InterfaceIPv4> oldObject = context->getInterfaceByName(routerName, data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] InterfaceIPv4 to modify not found: <%s>", data.getSmxId(),
					routerName.c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<InterfaceIPv4> modifiedObject = boost::make_shared<InterfaceIPv4>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for InterfaceIPv4 '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{

			if (data.changeMask & (imm::interfaceIpv4_attribute::LOOPBACK_CHANGE | imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE) )
			{
				if (modifiedObject->isLoopback() == modifiedObject->hasEncapsulation())
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Loopback or encapsulation must be set for Interface <%s>",
							modifiedObject->getSmxId(), modifiedObject->getRouterName(), modifiedObject->getName());
					return fixs_ith::ERR_CONFIG_INVALID_LOOPBACK;

				}
			}

			if (data.changeMask & imm::interfaceIpv4_attribute::LOOPBACK_CHANGE)
			{
				if (modifiedObject->isLoopback() && context->hasLoopback(modifiedObject->getRouterName()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot define multiple loopback interfaces on the same Router",
							modifiedObject->getSmxId(), modifiedObject->getRouterName());
					return fixs_ith::ERR_CONFIG_DUPLICATED_LOOPBACK;
				}
			}


			if (data.changeMask &  imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE)
			{
				if (modifiedObject->hasEncapsulation())
				{
					boost::shared_ptr<Vlan> encapsulationVlan = context->getVlanByName(fixs_ith::TRANSPORT_PLANE, common::utility::getIdValueFromRdn(modifiedObject->getEncapsulationDN()));

					if (NULL == encapsulationVlan)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] VLAN not found for SMX ID <%s> for encapsulation of Interface <%s>",
								modifiedObject->getSmxId(), modifiedObject->getRouterName(), modifiedObject->getSmxId(),
								modifiedObject->getName());
						return fixs_ith::ERR_CONFIG_INVALID_ENCAPSULATION;
					}

					if (imm::vlanId_range::UNDEFINED == encapsulationVlan->getVlanId())
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Specified VLAN encapsulation for interface <%s> does not have a valid VLAN ID",
								modifiedObject->getSmxId(), modifiedObject->getRouterName(),
								modifiedObject->getName());
						return fixs_ith::ERR_CONFIG_MISSING_ENCAPSULATION_VID;
					}

					if (context->isEncapsulation(modifiedObject->getEncapsulationDN()))
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Specified VLAN encapsulation for interface <%s> is already used as encapsulation",
								modifiedObject->getSmxId(), modifiedObject->getRouterName(),
								modifiedObject->getName());
						return fixs_ith::ERR_CONFIG_DUPLICATED_ENCAPSULATION;
					}

					modifiedObject->setEncapsulation(encapsulationVlan);
				}
			}

			// Copy data
			operation::interfaceInfo modifiedData = data;

			if(data.changeMask & (imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE | imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES_CHANGE))
			{
				if ( modifiedObject->getData().bfdProfile.empty() && ( imm::ENABLED == modifiedObject->getData().bfdStaticRoutes ) )
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to replace InterfaceIPv4 <%s> to context. bfdStaticRoutes cannot be ENABLED "
							"while bfdProfile is NULL", data.getSmxId(), data.getName());
					return fixs_ith::ERR_CONFIG_BFD_CONFLICT;
				}

				if( !(data.changeMask & imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE) )
				{
					modifiedData.bfdProfile.assign(modifiedObject->getData().getBfdProfile());
				}

				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] modifiedData: bfdProfile = <%s>, BfdStaticRoutes = <%d>",
						data.getSmxId(), routerName.c_str(), modifiedData.getBfdProfile(), modifiedData.getBfdStaticRoute());

			}

			if(data.changeMask & imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE)
			{
				modifiedData.aclData = modifiedObject->getData().aclData;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] modifiedData: aclIngress = <%s>",
										data.getSmxId(), routerName.c_str(), modifiedData.aclData.getAclName());
			}

			fixs_ith::ErrorConstants result = context->replaceInterface(modifiedObject, modifiedData);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace InterfaceIPv4 <%s> in context. Error <%d>",
						data.getSmxId(), routerName.c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced InterfaceIPv4: <%s>", data.getSmxId(), routerName.c_str(), data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::addressInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<Address> oldObject;
		std::string routerName(common::utility::getRouterNameFromAddressDN(data.getDN()));
		std::string interfaceName(common::utility::getInterfaceNameFromAddressDN(data.getDN()));

		if (!data.isVRRPInterfaceAddress())
		{
			if(context->hasVrrpSession(routerName, interfaceName))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot delete/modify an AddressIPv4 used by a Vrrpv3Session");
				return fixs_ith::ERR_CONFIG_PROTECTED_ADDRESS;
			}

			oldObject =	context->getRouterInterfaceAddressByName(routerName,interfaceName, data.getName());
		}
		else
		{
			std::set<std::string> vrrpSessions;
			fixs_ith::ErrorConstants result = getVrrpSessionsOfVrrpInterface(interfaceName, vrrpSessions);
			if(fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to retrieve reservedBy list from Vrrpv3Interface: <%s>", interfaceName.c_str());
				return result;
			}
			else if(!vrrpSessions.empty())
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot modify an AddressIPv4 used by a Vrrpv3Session");
				return fixs_ith::ERR_CONFIG_PROTECTED_ADDRESS;
			}

			oldObject =	context->getVrrpInterfaceAddressByName(interfaceName, data.getName());
		}


		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Address to modify not found: <%s>", data.getSmxId(), routerName.c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_ADDRESS_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<Address> modifiedObject = boost::make_shared<Address>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Address '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			if (!modifiedObject->isVRRPInterfaceAddress())
			{
				fixs_ith::ErrorConstants result = context->replaceRouterInterfaceAddress(modifiedObject, data);

				if (fixs_ith::ERR_NO_ERRORS != result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace Address <%s> in context. Error <%d>",
							data.getSmxId(), routerName.c_str(), data.getName(), result);
					return result;
				}
			}
			else
			{
				fixs_ith::ErrorConstants result = context->replaceVrrpInterfaceAddress(modifiedObject, data);

				if (fixs_ith::ERR_NO_ERRORS != result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to replace Address <%s> in context. Error <%d>",
							data.getName(), result);
					return result;
				}

				boost::shared_ptr<VrrpInterface> vrrpInterface = context->getVrrpInterfaceByName(modifiedObject->getInterfaceName());

				if (NULL == vrrpInterface)
				{
					//Error
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "VRRP Interface <%s> not found for address <%s>",
							modifiedObject->getInterfaceName(), data.getName());
					return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_ID;
				}

				// Set vrrpInterfaceInfo address attribute
				vrrpInterface->setAddressIPv4(modifiedObject->getData().address);
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced Address: <%s>", data.getSmxId(), routerName.c_str(), data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;
	}


	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::dstInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<Dst> oldObject;

		std::string routerName(common::utility::getRouterNameFromDstDN(data.getDN()));

		oldObject =	context->getDstByName(routerName, data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Dst to modify not found: <%s>", data.getSmxId(), routerName.c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_DST_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<Dst> modifiedObject = boost::make_shared<Dst>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Dst '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			if ((data.changeMask & imm::dst_attribute::DST_CHANGE) &&
					context->hasDst(modifiedObject->getRouterName(), modifiedObject->getDst()))
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Another Dst with dst <%s> already exists",
						data.getSmxId(), modifiedObject->getRouterName(), data.getDst());
				return fixs_ith::ERR_CONFIG_DUPLICATED_DST;
			}

			fixs_ith::ErrorConstants result = context->replaceDst(modifiedObject, data);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace Dst <%s> in context. Error <%d>",
						data.getSmxId(), routerName.c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced Dst: <%s>", data.getSmxId(),routerName.c_str(), data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::nextHopInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		std::string routerName(common::utility::getRouterNameFromNextHopDN(data.getDN()));
		std::string dstName(common::utility::getDstNameFromNextHopDN(data.getDN()));

		boost::shared_ptr<NextHop> oldObject = context->getNextHopByName(routerName.c_str(), dstName.c_str(), data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s][%s] NextHop to modify not found: <%s>", data.getSmxId(), routerName.c_str(), dstName.c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_NEXTHOP_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<NextHop> modifiedObject = boost::make_shared<NextHop>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for NextHop '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{

			if (data.changeMask & (imm::nextHop_attribute::DISCARD_CHANGE | imm::nextHop_attribute::ADDRESS_CHANGE) )
			{
				if (modifiedObject->isDiscard() == modifiedObject->hasAddress())
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s][%s] Discard or address must be set for NextHop <%s>",
							modifiedObject->getSmxId(), modifiedObject->getRouterName(), modifiedObject->getDstName(), modifiedObject->getName());
					return fixs_ith::ERR_CONFIG_INVALID_DISCARD;

				}

				if (modifiedObject->hasAddress() &&
						context->hasRouterInterfaceAddress(modifiedObject->getRouterName(), modifiedObject->getAddress()))
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s][%s] NextHop address <%s> matches an IP address of the same router",
							modifiedObject->getSmxId(), modifiedObject->getRouterName(), modifiedObject->getDstName(), modifiedObject->getAddress());
					return fixs_ith::ERR_CONFIG_CONFLICTING_NEXTHOP;

				}
			}

			fixs_ith::ErrorConstants result = context->replaceNextHop(modifiedObject, data);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s][%s] Failed to replace NextHop <%s> in context. Error <%d>",
						data.getSmxId(), routerName.c_str(), dstName.c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s][%s] Replaced NextHop: <%s>", data.getSmxId(), modifiedObject->getRouterName(), modifiedObject->getDstName(),  data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::vrrpInterfaceInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<VrrpInterface> oldObject = context->getVrrpInterfaceByName(data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "VrrpInterface to modify not found: <%s>", data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<VrrpInterface> modifiedObject = boost::make_shared<VrrpInterface>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Cannot allocate memory for VrrpInterface '%s'", data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			fixs_ith::ErrorConstants result = context->replaceVrrpInterface(modifiedObject, data);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to replace VrrpInterface <%s> in context. Error <%d>", data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Replaced VrrpInterface: <%s>", data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::vrrpSessionInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		std::string routerName( common::utility::getRouterNameFromVrrpSessionDN( data.getDN()) );
		std::string interfaceName( common::utility::getInterfaceNameFromVrrpSessionDN( data.getDN()) );

		boost::shared_ptr<VrrpSession> oldObject = context->getVrrpSessionByName(routerName, interfaceName, data.getName());

		if(NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] VRRP Session to modify not found: <%s>",
					data.getSmxId(), routerName.c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_ADDRESS_ID;
		}

		//Get a copy of existing object
		boost::shared_ptr<VrrpSession> modifiedObject = boost::make_shared<VrrpSession>(oldObject);

		if(NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for VRRP Session:<%s>", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if(modifiedObject->modify(data))
		{
			// some attribute is changed
			fixs_ith::ErrorConstants result = context->replaceVrrpSession(modifiedObject);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to replace VRRP Session <%s> in context. Error <%d>",
						data.getSmxId(), routerName.c_str(), data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Replaced VRRP Session: <%s>", data.getSmxId(), routerName.c_str(), data.getName());
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::bfdProfileInfo& data)
	{

		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		std::string routerName(common::utility::getRouterNameFromInterfaceDN(data.getDN()));

		boost::shared_ptr<BfdProfile> oldObject = context->getBfdProfileByName(routerName, data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] BfdProfile to modify not found: <%s>", data.getSmxId(),
					routerName.c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_BFDPROFILE_NAME;
		}

		//Get a copy of existing object
		boost::shared_ptr<BfdProfile> modifiedObject = boost::make_shared<BfdProfile>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for BfdProfile '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			fixs_ith::ErrorConstants result = context->replaceBfdProfile(modifiedObject, data);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to replace BfdProfile <%s> in context. Error <%d>", data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Replaced BfdProfile: <%s>", data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;

	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::aclEntryInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		std::string routerName(common::utility::getRouterNameFromChildDN(data.getDN()));

		boost::shared_ptr<AclEntryIpv4> oldObject = context->getAclEntryIpv4ByName(routerName, data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] AclEntry:<%s> to be modified NOT found!", data.getSmxId(),
					routerName.c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_ACLENTRY_NAME;
		}

		//Get a copy of existing object
		boost::shared_ptr<AclEntryIpv4> modifiedObject = boost::make_shared<AclEntryIpv4>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for AclEntry:<%s>", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			fixs_ith::ErrorConstants validationResult = imm::configurationHelper_t::instance()->validate(modifiedObject->getData(), data.changeMask);

			if (fixs_ith::ERR_NO_ERRORS != validationResult)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Validation failed for AclEntry:<%s>. Error:<%d>", data.getName(), validationResult);
				return validationResult;
			}

			fixs_ith::ErrorConstants result = context->replaceAclEntryIpv4(modifiedObject, data);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to replace AclEntry:<%s> in context. Error:<%d>", data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Replaced AclEntry:<%s>", data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::change(const operation::aclIpV4Info& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		std::string routerName(common::utility::getRouterNameFromChildDN(data.getDN()));

		boost::shared_ptr<AclIpv4> oldObject = context->getAclIpV4ByName(routerName, data.getName());

		if (NULL == oldObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Acl Ipv4 to modify not found: <%s>", data.getSmxId(),
					routerName.c_str(), data.getName());
			return fixs_ith::ERR_CONFIG_INVALID_ACLIPV4_NAME;
		}

		//Get a copy of existing object
		boost::shared_ptr<AclIpv4> modifiedObject = boost::make_shared<AclIpv4>(oldObject);

		if ( NULL == modifiedObject )
		{
			//Error
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "[%s] Cannot allocate memory for Acl Ipv4 '%s'", data.getSmxId(), data.getName() );
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		if (modifiedObject->modify(data))
		{
			fixs_ith::ErrorConstants result = context->replaceAclIpv4(modifiedObject);

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to replace Acl Ipv4 <%s> in context. Error <%d>", data.getName(), result);
				return result;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Replaced Acl Ipv4: <%s>", data.getName());
		}
		//else... Nothing is changed

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::bridgeInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		fixs_ith::ErrorConstants result = context->resetBridgeByName(data.getPlane(), data.getName());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to remove Bridge <%s> from context. Error <%d>",
					data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
			return result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Removed Bridge: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::ethernetPortInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		fixs_ith::ErrorConstants result = context->resetEthernetPortByName(data.getPlane(), data.getName());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to remove EthernetPort <%s> from context. Error <%d>",
					data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
			return result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Removed EthernetPort: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::aggregatorInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		fixs_ith::ErrorConstants result = context->resetAggregatorByName(data.getPlane(), data.getName());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to remove Aggregator <%s> from context. Error <%d>",
					data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
			return result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Removed Aggregator: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::bridgePortInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		fixs_ith::ErrorConstants result = context->resetBridgePortByName(data.getPlane(), data.getName());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to remove BridgePort <%s> from context. Error <%d>",
					data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
			return result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Removed BridgePort: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::vlanInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		boost::shared_ptr<Vlan> vlan = context->getVlanByName(data.getPlane(), data.getName());

		if (NULL != vlan)
		{
			if (isProtectedObject(vlan->getPlane(), vlan->getName()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot delete Vlan <%s>. It's a protected object.",
						vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(), vlan->getName());
				return fixs_ith::ERR_CONFIG_PROTECTED_OBJECT;

			}

			if (context->hasSubnetBasedVlan(vlan->getPlane(),vlan->getVlanId()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot remove Vlan <%s> with VID <%d> having Subnet Based Vlan",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), vlan->getName(), vlan->getVlanId());
				return fixs_ith::ERR_CONFIG_PROTECTED_VLAN;
			}

			if (vlan->isReserved())
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot remove Vlan <%s> with VID <%d> referenced by other object",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), vlan->getName(), vlan->getVlanId());
				return fixs_ith::ERR_CONFIG_REFERENCED_OBJECT;
			}
		}

		fixs_ith::ErrorConstants result = context->resetVlanByName(data.getPlane(), data.getName());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to remove Vlan <%s> from context. Error <%d>",
					data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
			return result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Removed Vlan: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::subnetVlanInfo& data )
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;


		{
			fixs_ith::ErrorConstants result = context->resetSubnetVlanByName(data.getPlane(), data.getName());


			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to remove SubnetVlan <%s> from context. Error <%d>",
						data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName(), result);
				return result;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Removed SubnetVlan: <%s>", data.getSmxId(), common::utility::planeToString(data.getPlane()).c_str(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::routerInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		{
			boost::shared_ptr<Router> router = context->getRouterByName(data.getName());

			if (NULL != router)
			{
				if ((engine::defaultRouter::NAME).compare(common::utility::getRouterIdFromRouterName(data.getName())) == 0)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Cannot delete Router <%s>. It's a protected object.",
							router->getSmxId(), router->getName());
					return fixs_ith::ERR_CONFIG_PROTECTED_OBJECT;
				}
			}

			fixs_ith::ErrorConstants result = context->resetRouterByName(data.getName());


			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to remove Router <%s> from context. Error <%d>",
						data.getSmxId(), data.getName(), result);
				return result;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Removed Router: <%s>", data.getSmxId(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::interfaceInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		std::string routerName(common::utility::getRouterNameFromInterfaceDN(data.getDN()));

		{
			if (data.getLoopback() && NULL != context->getRouterByName(routerName))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s]Cannot remove loopback InterfaceIPv4 <%s>",
						data.getSmxId(), routerName.c_str(), data.getName());
				return fixs_ith::ERR_CONFIG_PROTECTED_OBJECT;
			}

			fixs_ith::ErrorConstants result = context->resetInterfaceByName(routerName, data.getName());

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to remove InterfaceIPv4 <%s> from context. Error <%d>",
						data.getSmxId(), data.getName(), result);
				return result;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Removed InterfaceIPv4: <%s>", data.getSmxId(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::addressInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		std::string routerName(common::utility::getRouterNameFromAddressDN(data.getDN()));
		std::string interfaceName(common::utility::getInterfaceNameFromAddressDN(data.getDN()));


		if (!data.isVRRPInterfaceAddress())
		{
			if(context->hasVrrpSession(routerName, interfaceName))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot delete an AddressIPv4 used by a Vrrpv3Session");
				return fixs_ith::ERR_CONFIG_PROTECTED_ADDRESS;
			}

			boost::shared_ptr<InterfaceIPv4> interface = context->getInterfaceByName(routerName, interfaceName);
			if (interface != NULL && interface->isLoopback() && NULL != context->getRouterByName(routerName))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s]Cannot remove address <%s> from loopback InterfaceIPv4 <%s>",
						data.getSmxId(), routerName.c_str(), data.getName(), interfaceName.c_str());
				return fixs_ith::ERR_CONFIG_PROTECTED_OBJECT;
			}

			fixs_ith::ErrorConstants result = context->resetRouterInterfaceAddressByName(routerName, interfaceName, data.getName());

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to remove Address <%s> on Interface <%s> from context. Error <%d>",
						data.getSmxId(),  routerName.c_str(), data.getName(), interfaceName.c_str(), result);
				return result;
			}
		}
		else
		{
			std::set<std::string> vrrpSessions;
			fixs_ith::ErrorConstants result = getVrrpSessionsOfVrrpInterface(interfaceName, vrrpSessions);
			if(fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to retrieve reservedBy list from Vrrpv3Interface: <%s>", interfaceName.c_str());
				return result;
			}
			else if(!vrrpSessions.empty())
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot modify an AddressIPv4 used by a Vrrpv3Session");
				return fixs_ith::ERR_CONFIG_PROTECTED_ADDRESS;
			}

			result = context->resetVrrpInterfaceAddressByName(interfaceName, data.getName());

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to remove Address <%s> on VRRP Interface <%s> from context. Error <%d>",
						data.getName(), interfaceName.c_str(), result);
				return result;
			}

			boost::shared_ptr<VrrpInterface> vrrpInterface = context->getVrrpInterfaceByName(interfaceName);

			if (NULL == vrrpInterface)
			{
				//Error
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "VRRP Interface <%s> not found for address <%s>",
						interfaceName.c_str(), data.getName());
				return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_ID;
			}

			// Reset vrrpInterfaceInfo address attribute
			vrrpInterface->setAddressIPv4("");
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Removed Address: <%s> on Interface <%s>", data.getSmxId(),  routerName.c_str(), data.getName(), interfaceName.c_str());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::dstInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		std::string routerName(common::utility::getRouterNameFromDstDN(data.getDN()));

		{
			fixs_ith::ErrorConstants result = context->resetDstByName(routerName, data.getName());

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to remove Dst <%s> from context. Error <%d>",
						data.getSmxId(),  routerName.c_str(), data.getName(), result);
				return result;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Removed Dst: <%s>", data.getSmxId(),  routerName.c_str(), data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::nextHopInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		std::string routerName(common::utility::getRouterNameFromNextHopDN(data.getDN()));
		std::string dstName(common::utility::getDstNameFromNextHopDN(data.getDN()));

		{
			fixs_ith::ErrorConstants result = context->resetNextHopByName(routerName, dstName, data.getName());

			if (fixs_ith::ERR_NO_ERRORS != result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s][%s] Failed to remove NextHop <%s> from context. Error <%d>",
						data.getSmxId(),  routerName.c_str(), dstName.c_str(), data.getName(), result);
				return result;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s][%s] Removed NextHop: <%s>", data.getSmxId(),  routerName.c_str(), dstName.c_str(),  data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::vrrpInterfaceInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		fixs_ith::ErrorConstants result = context->resetVrrpInterfaceByName(data.getName());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to remove VrrpInterface <%s> from context. Error <%d>",
					data.getName(), result);
			return result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Removed VrrpInterface: <%s>", data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::vrrpSessionInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		fixs_ith::ErrorConstants result = context->resetVrrpSessionByDN(data.getDN());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to remove VrrpInterface <%s> from context. Error <%d>",
					data.getName(), result);
			return result;
		}


		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Removed VrrpInterface: <%s>", data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::bfdProfileInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		fixs_ith::ErrorConstants result = context->resetBfdProfileByDN(data.getDN());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to remove BfdProfile <%s> from context. Error <%d>",
					data.getName(), result);
			return result;
		}


		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Removed BfdProfile: <%s>", data.getName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::bfdSessionInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context:<%s> not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		std::string routerName(common::utility::getRouterNameFromBfdSessionIPv4DN( data.getDN()));

		return context->resetBfdSessionIPv4ByName(routerName, data.getName());
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::aclEntryInfo& data)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context:<%s> not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

		std::string routerName(common::utility::getRouterNameFromChildDN(data.getDN()));

		return context->resetAclEntryByName(routerName, data.getName());
	}

	fixs_ith::ErrorConstants ConfigurationManager::remove(const operation::aclIpV4Info& data)
		{
			contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

			Context* context = access.getContext();

			if(!context)
			{
				FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context:<%s> not found or not accessible.", data.getSmxId() );
				return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
			}

			if(context->isContextRemoveOngoing()) return fixs_ith::ERR_NO_ERRORS;

			std::string routerName(common::utility::getRouterNameFromChildDN(data.getDN()));

			return context->resetAclIpV4ByName(routerName, data.getName());
		}

	fixs_ith::ErrorConstants ConfigurationManager::getVrrpInterfaceDNbyVrrpSessionDN(const std::string& vrrpSessionDN, std::string& vrrpInterfaceDN)
	{
		std::string vrrpSessionSmxId(common::utility::getSmxIdFromVrrpv3SessionDN(vrrpSessionDN));
		contextAccess_t access(vrrpSessionSmxId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context of Common Elements not found or not accessible.");
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<VrrpSession> vrrpSession = context->getVrrpSessionByDN(vrrpSessionDN);

		if(!vrrpSession)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "VRRP session not found DN:<%s>", vrrpSessionDN.c_str());
			return fixs_ith::ERR_CONFIG_INVALID_VRRPSESSION;
		}

		vrrpInterfaceDN.assign(vrrpSession->getData().getVrrpInterfaceRef());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::getVrrpSessionsOfVrrpInterface(const std::string& vrrpInstanceName, std::set<std::string>& vrrpSessions)
	{
		vrrpSessions.clear();

		// Empty string to address common part
		std::string commonElementsId;
		contextAccess_t access(commonElementsId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context for common part not found or not accessible.");
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<VrrpInterface> vrrpInstanceObject = context->getVrrpInterfaceByName(vrrpInstanceName);

		if(!vrrpInstanceObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "VrrpInterface not found: <%s>", vrrpInstanceName.c_str() );
			return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_ID;
		}

		vrrpSessions = vrrpInstanceObject->getData().getReservedBy();

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::getIPv4AddressOfVrrpInterface(const std::string& vrrpInstanceName, std::string& address)
	{
		// Empty string to address common part
		std::string commonElementsId;
		contextAccess_t access(commonElementsId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context for common part not found or not accessible.");
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<Address> vrrpInstanceAddressObject = context->getVrrpInterfaceAddress(vrrpInstanceName);

		if(!vrrpInstanceAddressObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Address of VrrpInterface not found: <%s>", vrrpInstanceName.c_str() );
			return fixs_ith::ERR_CONFIG_INVALID_ADDRESS_ID;
		}

		address.assign(vrrpInstanceAddressObject->getAddress());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::getIPv4AddressOfInterface(const operation::vrrpSessionInfo& data, std::string& address)
	{
		contextAccess_t access(data.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", data.getSmxId() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		std::string routerName(common::utility::getRouterNameFromVrrpSessionDN(data.getDN()));
		std::string interfaceName(common::utility::getInterfaceNameFromVrrpSessionDN(data.getDN()));

		boost::shared_ptr<Address> addressObject =	context->getRouterInterfaceAddress(routerName, interfaceName);

		if(!addressObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Address of Interface not found: <%s>, vrrpSession DN:<%s>", interfaceName.c_str(), data.getDN());
			return fixs_ith::ERR_CONFIG_INVALID_ADDRESS_ID;
		}

		address.assign(addressObject->getAddress());

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::getVrrpSessionInfo(const std::string& vrrpSessionDN, operation::vrrpSessionInfo& data)
	{
		std::string vrrpSessionSmxId(common::utility::getSmxIdFromVrrpv3SessionDN(vrrpSessionDN));
		contextAccess_t access(vrrpSessionSmxId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context <%s> not found or not accessible.", vrrpSessionSmxId.c_str());
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		return context->getVrrpSessionInfo(vrrpSessionDN, data);
	}

	fixs_ith::ErrorConstants ConfigurationManager::getInterfacesIPv4ReferencingBfdProfile(const std::string& smxId, const std::string& routerName, const std::string& bfdProfileName, std::set<std::string>& interfaceIPv4DNs)
	{
		interfaceIPv4DNs.clear();

		contextAccess_t access(smxId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context not found or not accessible.");
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<BfdProfile> bfdProfileObject = context->getBfdProfileByName(routerName, bfdProfileName);

		if(!bfdProfileObject)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "BfdProfile not found: <%s>", bfdProfileName.c_str() );
			return fixs_ith::ERR_CONFIG_INVALID_BFDPROFILE_NAME;
		}

		interfaceIPv4DNs = bfdProfileObject->getData().getReservedBy();

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::getBfdSessionInfo(const std::string& bfdSessionDN, operation::bfdSessionInfo& data)
	{
		std::string bfdSessionSmxId(common::utility::getSmxIdFromBfdSessionDN(bfdSessionDN));
		contextAccess_t access(bfdSessionSmxId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context <%s> not found or not accessible.", bfdSessionSmxId.c_str());
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if( context->isContextRemoveOngoing() ) return fixs_ith::ERR_GENERIC;

		return context->getBfdSessionIPv4Info(bfdSessionDN, data);
	}

	fixs_ith::ErrorConstants ConfigurationManager::getInterfaceIPv4byAddress(const operation::addressInfo& address, operation::interfaceInfo& interface)
	{
		contextAccess_t access(address.getSmxId(), GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context not found or not accessible.");
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		std::string addressDN = address.getDN();
		std::string routerName = common::utility::getRouterNameFromAddressDN(addressDN);
		std::string interfaceIPv4Name = common::utility::getInterfaceNameFromAddressDN(addressDN);

		boost::shared_ptr<InterfaceIPv4> interfaceIPv4 = context->getInterfaceByName(routerName, interfaceIPv4Name);

		if(!interfaceIPv4)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "InterfaceIPv4 not found: <%s>", interfaceIPv4Name.c_str() );
			return fixs_ith::ERR_CONFIG_INVALID_INTERFACE_NAME;
		}

		interface = interfaceIPv4->getData();

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::getInterfacesIPv4ReferencingAclIpv4(const std::string& smxId, const std::string& routerName, const std::string& aclIpv4Name, std::set<std::string>& interfaceIPv4DNs)
	{
		interfaceIPv4DNs.clear();

		contextAccess_t access(smxId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context not found or not accessible.");
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<AclIpv4> aclIpv4Object = context->getAclIpV4ByName(routerName, aclIpv4Name);

		if(!aclIpv4Object)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ACL Ipv4 not found: <%s>", aclIpv4Name.c_str() );
			return fixs_ith::ERR_CONFIG_INVALID_ACLIPV4_NAME;
		}

		interfaceIPv4DNs = aclIpv4Object->getData().getReservedBy();

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::getEthPortAutoNegotiateValue(const std::string & smxId, const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& ifIndex, bool& isAutoNegotiateEnabled)
	{
		contextAccess_t access(smxId, GET_EXISTING, SHARED_ACCESS);
		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context not found or not accessible.");
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		if(boost::shared_ptr<engine::EthernetPort> eth_port = context->getEthernetPortByIndex(plane, ifIndex))
		{
			isAutoNegotiateEnabled = eth_port->isAutoNegotiateEnabled();
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "EthernetPort found in the context -> smxId:<%s>, ifIndex:<%d>, AutoNegotiate:<%s>", smxId.c_str(), ifIndex, isAutoNegotiateEnabled ? "ENABLED" : "DISABLED");
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: EthernetPort NOT found in the context -> smxId:<%s>, ifIndex:<%d>", ifIndex, smxId.c_str());
			return fixs_ith::ERR_GENERIC;
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	bool ConfigurationManager::hasNextHopAddress(const std::string& nextHopDn)
	{
		bool result = false;
		std::string smxId(common::utility::getSmxIdFromNextHopDN(nextHopDn));
		contextAccess_t access(smxId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context not found or not accessible for SMX ID:<%s>", smxId.c_str());
			return result;
		}

		std::string routerName( common::utility::getRouterNameFromNextHopDN(nextHopDn) );
		std::string nextHopName( common::utility::getNextHopNameFromDN(nextHopDn) );
		std::string destName(  common::utility::getDstNameFromNextHopDN(nextHopDn) );

		result = context->hasAddressNextHop(routerName, destName, nextHopName);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NextHop:<%s> of Dst:<%s> on Router:<%s>, has Address is <%s>",
				nextHopName.c_str(), destName.c_str(), routerName.c_str(), common::utility::boolToString(result));
		return result;
	}

	bool ConfigurationManager::isBfdProfileDNValid(const std::string& smxId, const std::string& interfaceDN, const std::string& bfdProfileDN)
	{
		bool result = false;
		contextAccess_t access(smxId, GET_EXISTING, SHARED_ACCESS);
		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context <%s> not found or not accessible.", smxId.c_str());
			return result;
		}

		std::string interfaceRouterName = common::utility::getRouterNameFromInterfaceDN(interfaceDN);
		std::string bfdProfileRouterName = common::utility::getRouterNameFromAddressDN(bfdProfileDN);

		if( interfaceRouterName.compare(bfdProfileRouterName) != 0U )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Router Mismatch from BFD profile:<%s> and Interface:<%s>",
					bfdProfileRouterName.c_str(), interfaceRouterName.c_str());
			return result;
		}

		std::string bfdProfileName = common::utility::getBfdProfileNameFromDN(bfdProfileDN);

		boost::shared_ptr<BfdProfile> bfdProfileObj;
		bfdProfileObj = context->getBfdProfileByName(bfdProfileRouterName, bfdProfileName);

		result = ( NULL != bfdProfileObj);

		return result;
	}

	bool ConfigurationManager::isAclIngressDNValid(const std::string& smxId, const std::string& interfaceDN, const std::string& aclIngresseDN)
	{
		bool result = false;
		contextAccess_t access(smxId, GET_EXISTING, SHARED_ACCESS);
		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context <%s> not found or not accessible.", smxId.c_str());
			return result;
		}

		std::string interfaceRouterName = common::utility::getRouterNameFromInterfaceDN(interfaceDN);
		std::string aclRouterName = common::utility::getRouterNameFromChildDN(aclIngresseDN);

		// Maybe we should check also the smxid??
		if( interfaceRouterName.compare(aclRouterName) != 0U )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Router Mismatch from ACLIpv4 :<%s> and Interface:<%s>",
					aclRouterName.c_str(), interfaceRouterName.c_str());
			return result;
		}

		std::string aclIpv4Name = common::utility::getAclIpv4NameFromDN(aclIngresseDN);

		boost::shared_ptr<AclIpv4> aclObject;
		aclObject = context->getAclIpV4ByName(aclRouterName, aclIpv4Name);

		result = ( NULL != aclObject);

		return result;
	}

	bool ConfigurationManager::getContext(contextAccess_t& access, const std::string& smxId)
	{
		ContextAccessMode accessMode = access.accessMode;
		ContextLockMode lockMode = access.lockMode;

		Context* context = NULL;
		{
			boost::lock_guard<boost::recursive_mutex> guard(m_contextMapMutex);
			smxMap_t::const_iterator it = m_smxMap.find(smxId);

			access.accessResult = CONTEXT_NOT_FOUND;

			if (it != m_smxMap.end())
			{
				context = (*it).second;
				access.accessResult = CONTEXT_ACCESS_ACQUIRED;

				if ((EXCLUSIVE_ACCESS == lockMode && !context->try_lock()) //Try_lock failed. Cannot grant EXCLUSIVE access.
						|| (SHARED_ACCESS == lockMode && !context->try_lock_shared())) //context is locked in exclusive access by another thread. No access is possible.
				{
					context = NULL;
					access.accessResult = CONTEXT_ACCESS_DENIED;
				}

			}
			else if (CREATE_IF_NOT_FOUND == accessMode)
			{
				context = new (std::nothrow) Context(smxId);
				if(!context)
				{
					//Memory Error
					FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Cannot allocate memory for Context '%s'", smxId.c_str());
				}
				else
				{
					FIXS_ITH_LOG(LOG_LEVEL_INFO, "[%s] Add context to the MAP.", smxId.c_str());

					if ((EXCLUSIVE_ACCESS == lockMode && !context->try_lock()) 	//Cannot lock the context we just created. This should never happen!!!
							|| (SHARED_ACCESS == lockMode && !context->try_lock_shared())) //The context we just created is already locked by another thread. This should never happen!!!
					{
						//Something wrong happened if we got here...
						delete context;
						context = NULL;
						access.accessResult = CONTEXT_ACCESS_DENIED;
					}

					if (NULL != context)
					{
						//Add context to the map
						m_smxMap.insert(smxMap_t::value_type(smxId, context));
						access.accessResult = CONTEXT_ACCESS_ACQUIRED;
					}

				}
			}
		}

		access.context = context;

		return context != 0;
	}

	void ConfigurationManager::resetContext(const std::string& smxId)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMapMutex);

		smxMap_t::iterator it = m_smxMap.find(smxId);
		if (it != m_smxMap.end())
		{
			m_smxMap.erase(it);
		}
	}


	bool ConfigurationManager::hasContext(const std::string& smxId)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMapMutex);

		return m_smxMap.find(smxId) != m_smxMap.end();
	}
	

	fixs_ith::ErrorConstants ConfigurationManager::assignTaggedPorts(boost::shared_ptr<Vlan>& vlan, Context* context)
	{

		const std::set<std::string> taggedPortsDN = vlan->getTaggedPortsDN();

		std::set<boost::shared_ptr<BridgePort> > taggedPorts;

		//Check if tagged ports exists
		for (std::set<std::string>::iterator portDN = taggedPortsDN.begin(); portDN != taggedPortsDN.end(); ++portDN)
		{
			std::string portName = common::utility::getIdValueFromRdn(*portDN);
			boost::shared_ptr<BridgePort> bridgePort = context->getBridgePortByName(vlan->getPlane(), portName);

			if (NULL == bridgePort)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] BridgePort <%s> not found for Vlan <%s>",
						vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(), portName.c_str(), vlan->getName());
				return fixs_ith::ERR_CONFIG_INVALID_TAGGED_PORT;
			}

			if (context->isAggregatorMember(bridgePort->getPlane(), bridgePort->getSnmpIndex()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] BridgePort <%s> is an AggregatorMember and cannot be individually added to Vlan <%s>",
						vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(), portName.c_str(), vlan->getName());
				return fixs_ith::ERR_CONFIG_AGG_MEMBER_PORT;
			}

			if (vlan->hasUntaggedPortDN(*portDN))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] BridgePort <%s> cannot be added both as tagged and untagged port to Vlan <%s>",
						vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(), portName.c_str(), vlan->getName());
				return fixs_ith::ERR_CONFIG_DUPLICATED_PORT;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added TAGGED BridgePort <%s> to Vlan <%s>", vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(),
					bridgePort->getName(), vlan->getName());

			taggedPorts.insert(bridgePort);
		}

		vlan->setTaggedBridgePorts(taggedPorts);
		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::assignUntaggedPorts(boost::shared_ptr<Vlan>& vlan, Context* context)
	{
		const std::set<std::string> untaggedPortsDN = vlan->getUntaggedPortsDN();

		std::set<boost::shared_ptr<BridgePort> > untaggedPorts;

		//Check if untagged ports exists
		for (std::set<std::string>::iterator portDN = untaggedPortsDN.begin(); portDN != untaggedPortsDN.end(); ++portDN)
		{
			std::string portName = common::utility::getIdValueFromRdn(*portDN);
			boost::shared_ptr<BridgePort> bridgePort = context->getBridgePortByName(vlan->getPlane(), portName);

			if (NULL == bridgePort)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] BridgePort <%s> not found for Vlan <%s>",
						vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(), portName.c_str(), vlan->getName());
				return fixs_ith::ERR_CONFIG_INVALID_UNTAGGED_PORT;
			}

			if (context->isAggregatorMember(bridgePort->getPlane(), bridgePort->getSnmpIndex()))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] BridgePort <%s> is an AggregatorMember and cannot be individually added to Vlan <%s>",
						vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(), portName.c_str(), vlan->getName());
				return fixs_ith::ERR_CONFIG_AGG_MEMBER_PORT;
			}

			if (vlan->hasTaggedPortDN(*portDN))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] BridgePort <%s> cannot be added both as tagged and untagged port to Vlan <%s>",
						vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(), portName.c_str(), vlan->getName());
				return fixs_ith::ERR_CONFIG_DUPLICATED_PORT;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added UNTAGGED BridgePort <%s> to Vlan <%s>", vlan->getSmxId(), common::utility::planeToString(vlan->getPlane()).c_str(),
					bridgePort->getName(), vlan->getName());
			untaggedPorts.insert(bridgePort);
		}

		vlan->setUntaggedBridgePorts(untaggedPorts);
		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants ConfigurationManager::assignEthernetPorts(boost::shared_ptr<Aggregator>& aggregator, Context* context)
	{
		std::set<boost::shared_ptr<EthernetPort> > aggMembers;

		const std::set<std::string> aggMembersName = aggregator->getAdminAggMemberDN();
		for (std::set<std::string>::iterator element = aggMembersName.begin(); element != aggMembersName.end(); ++element)
		{
			boost::shared_ptr<EthernetPort> ethernetPort = context->getEthernetPortByName(aggregator->getPlane(), common::utility::getIdValueFromRdn(*element));

			if (NULL == ethernetPort)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] EthernetPort <%s> not found for Aggregator <%s>",
						aggregator->getSmxId(), common::utility::planeToString(aggregator->getPlane()).c_str(), common::utility::getIdValueFromRdn(*element).c_str(), aggregator->getName());
				return fixs_ith::ERR_CONFIG_INVALID_ETHERNET_PORT_ID;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s][%s] Added EthernetPort <%s> to Aggregator <%s>", aggregator->getSmxId(), common::utility::planeToString(aggregator->getPlane()).c_str(),
					ethernetPort->getName(), aggregator->getName());
			aggMembers.insert(ethernetPort);
		}

		aggregator->setEthernetPorts(aggMembers);
		return fixs_ith::ERR_NO_ERRORS;
	}

	bool ConfigurationManager::addToReloadInProgressSet(const std::string& smxId)
	{
		boost::recursive_mutex::scoped_lock lk(m_reloadInProgressSetMutex);
		std::pair<std::set<std::string>::iterator, bool> insertResult = m_reloadInProgressSet.insert(smxId);
		return insertResult.second;
	}

	bool ConfigurationManager::isReloadInProgress(const std::string& smxId)
	{
		boost::recursive_mutex::scoped_lock lk(m_reloadInProgressSetMutex);
		return m_reloadInProgressSet.find(smxId) != m_reloadInProgressSet.end();
	}

	void ConfigurationManager::removeFromReloadInProgressSet(const std::string& smxId)
	{
		boost::recursive_mutex::scoped_lock lk(m_reloadInProgressSetMutex);
		m_reloadInProgressSet.erase(smxId);
	}

	std::set<std::string> ConfigurationManager::getContextKeys()
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMapMutex);

		std::set<std::string> keys;
		for (smxMap_t::iterator it = m_smxMap.begin(); it != m_smxMap.end(); ++it)
		{
			//Return ONLY valid keys.
			if (!it->first.empty())
			{
				keys.insert(it->first);
			}
		}

		return keys;
	}

	bool ConfigurationManager::isProtectedObject(const fixs_ith::switchboard_plane_t& plane, const std::string& objName)
	{
		std::pair<mapOfProtectedObjs_t::iterator, mapOfProtectedObjs_t::iterator> protectedObjsRange = m_protectedObjs.equal_range(plane);

		for(mapOfProtectedObjs_t::iterator element = protectedObjsRange.first; element != protectedObjsRange.second; ++element)
		{

			if (objName.compare(element->second) == 0)
			{
                                FIXS_ITH_LOG(LOG_LEVEL_WARN, "isProtectedObject '%s'",objName.c_str());
				return true;
			}
		}

		return false;
	}

	bool ConfigurationManager::isRemoveContextOngoing(const std::string& smxId) const
	{
		bool contextRemoveOngoing = false;
		contextAccess_t access(smxId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(context) contextRemoveOngoing = context->isContextRemoveOngoing();

		return contextRemoveOngoing;
	}

	bool ConfigurationManager::is_early_enabled_if(const fixs_ith::switchboard_plane_t & plane, const uint16_t if_index)
	{
		if (fixs_ith::CONTROL_PLANE == plane) return false;


		if (fixs_ith::TRANSPORT_PLANE == plane)
		{
			if(engine::transport::EARLY_ENABLED_IF_INDEXES_SET.count(if_index)> 0)
				return true;
		}

		return false;
	}

	fixs_ith::ErrorConstants  ConfigurationManager::getSnmpIndexfromBridgePortDN(const std::string objectDN, uint16_t & ifIndex)
	{

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BridgePort DN:<%s> ", objectDN.c_str());

		std::string parentRDNvalue =  common::utility::getParentDN(objectDN);
		std::string smxId = common::utility::getSmxIdFromBridgeDN(parentRDNvalue);
		fixs_ith::SwitchBoardPlaneConstants plane = common::utility::getPlaneFromBridgeDN(parentRDNvalue);
		std::string portNameId = common::utility::getIdValueFromRdn(objectDN);

		std::string portName = common::utility::buildPortName(smxId, plane, portNameId);

		contextAccess_t access(smxId, GET_EXISTING, SHARED_ACCESS);

		Context* context = access.getContext();

		if(!context)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Context '%s' not found or not accessible.", smxId.c_str() );
			return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
		}

		boost::shared_ptr<EthernetPort> ethernetPort = context->getEthernetPortByName(plane, portName);
		boost::shared_ptr<Aggregator>  aggregator = context->getAggregatorByName(plane, portName);

		if (NULL != ethernetPort)
			ifIndex = ethernetPort->getSnmpIndex();
		else if (NULL != aggregator)
			ifIndex = aggregator->getSnmpIndex();
		else
			return fixs_ith::ERR_CONFIG_INVALID_ETHERNET_PORT_ID;

		return fixs_ith::ERR_NO_ERRORS;
	}

	int ConfigurationManager::deleteOldSNMPLogs(const std::string & cmxId)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Deleting old snmp logs on APG");

		int index;
		std::string cont_file_name="";
		uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;
		fixs_ith_switchboardinfo switch_board;
		if(!cmxId.empty() && (fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(switch_board, cmxId) !=fixs_ith_sbdatamanager::SBM_OK)  )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Switch board with key <%s> not found in CS configuration", cmxId.c_str());
			return fixs_ith::ERR_CS_NO_BOARD_FOUND;
		}
		for(index=0; index<=10;index++)
		{
			cont_file_name = "";
			//Get the containter file name for each index
			int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_cont_file_name(switch_board, fixs_ith::TRANSPORT_PLANE, index, cont_file_name, &timeout_ms);
			if(op_res < 0)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Failed to fetch container file name for index == %d", index);
				continue;
			}
			//std::string file_name = std::string((char *)cont_file_name);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Container file name == %s",cont_file_name.c_str());
			//FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Container file name in string format == %s", file_name.c_str());

			//Check container name is valid or not
			/*size_t found = cont_file_name.find("Invalid");
			if(found != std::string::npos)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Contailer file name invalid for index == %d",index);
				continue;
			}*/

			// Set to delete each container file with name obtained above
			{
				int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().delete_container_file(switch_board, cont_file_name);
				if(op_res < 0)
				{
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Failed to delete file with name == %s",cont_file_name.c_str());

				}
				else
				{
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Successfully deleted file with name= %s",cont_file_name.c_str());
				}
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"After:File name for index =%d is %s", index,cont_file_name.c_str());

			}
		}

		return 0;
	}

int ConfigurationManager::getLogsFromCMX(const std::string & cmxId,
		const char * source_trap_ip) {

	std::string tftpRoot = "/data/apz/data";
	std::string tftpPathCMX = "/boot/cmx/cmx_logs/cmx_";
	std::string clusterIP = "192.168.169.";
	std::string tmp(source_trap_ip);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "temporary string: %s", tmp.c_str());
	int nodeNum = common::utility::getNodeNumber();
	if (clusterIP.compare(tmp.substr(0, 12)) == 0) {
		char c[16];
		sprintf(c, "%d", nodeNum);
		clusterIP += std::string(c);
	} else {
		clusterIP = "192.168.170.";
		char c[16];
		sprintf(c, "%d", nodeNum);
		clusterIP += std::string(c);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cluster IP == %s", clusterIP.c_str());
	int createRes;
	std::string folderName = tftpRoot + "/boot/cmx";
	createRes = common::utility::createDir(folderName);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " createRes == %d ", createRes);
	folderName = tftpRoot + "/boot/cmx/cmx_logs";
	createRes = common::utility::createDir(folderName);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " createRes == %d ", createRes);

	//replace . with _
	//-- Remove this code if not necessary -- BEGIN
	std::string ipFiltered(tmp);
	if (ipFiltered.compare("") != 0) {
		size_t foundDot;
		foundDot = ipFiltered.find_first_of(".");
		while (foundDot != string::npos) {
			ipFiltered[foundDot] = '_';
			foundDot = ipFiltered.find_first_of(".", foundDot + 1);
		}
	}
	//-- Remove this code if not necessary -- END
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CMX Logging: Modified IP address==%s",
			ipFiltered.c_str());
	std::string logPath("");
	logPath = tftpRoot + tftpPathCMX + ipFiltered;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CMX Logging: Log path == %s",
			logPath.c_str());

	createRes = common::utility::createDir(logPath);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CMX Logging: Log path creation result == %d",
			createRes);

	std::string snmplogPath("");
	snmplogPath = tftpPathCMX + ipFiltered;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CMX Logging: snmp log path == %s",
			snmplogPath.c_str());

	int call_result = 0;

	fixs_ith_switchboardinfo switch_board;
	if (!cmxId.empty()
			&& (fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
					switch_board, cmxId) != fixs_ith_sbdatamanager::SBM_OK)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Switch board with key <%s> not found in CS configuration",
				cmxId.c_str());
		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
	}

	do {
		call_result =
				fixs_ith::workingSet_t::instance()->get_snmpmanager().create_cont_transfer_server_ip(
						switch_board, clusterIP.c_str());
		if (call_result < 0) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"create_cont_transfer_server_ip SNMP set request failed");
			break;
		}

		call_result = 0;
		//oidContTransferSrvPath
		call_result =
				fixs_ith::workingSet_t::instance()->get_snmpmanager().create_cont_transfer_server_path(
						switch_board, snmplogPath.c_str());
		if (call_result < 0) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"create_cont_transfer_server_path SNMP set request failed");
			break;
		}

		call_result = 0;
		int control_value = 1;
		//oidContNotificationsCtrl
		call_result =
				fixs_ith::workingSet_t::instance()->get_snmpmanager().set_cont_control_notifications(
						switch_board, control_value);
		if (call_result < 0) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"set_cont_control_notifications SNMP set request failed");
			break;
		}

		call_result = 0;
		//oidContAutomaticTransfer
		call_result =
				fixs_ith::workingSet_t::instance()->get_snmpmanager().set_cont_automatic_transfer(
						switch_board, control_value);
		if (call_result < 0) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"set_cont_automatic_transfer SNMP set request failed");
			break;
		}

		call_result = 0;
		int encryption_setting = 0;
		//oidContFileEncryption
		call_result =
				fixs_ith::workingSet_t::instance()->get_snmpmanager().set_cont_file_encryption(
						switch_board, encryption_setting);
		if (call_result < 0) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"set_cont_file_encryption SNMP set request failed");
			break;
		}

		call_result = 0;
		int value = 1;
		//oidCreateContFile
		call_result =
				fixs_ith::workingSet_t::instance()->get_snmpmanager().create_cont_file(
						switch_board, value);
		if (call_result < 0) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"setCreateContFile SNMP set request failed");
			break;
		}
	} while (0);

	return 0;

}

bool ConfigurationManager::createVlanEntryInAxeequipment(
		const std::string vlanName, int vlanTag) {

	bool res = false;
	ACS_CC_ReturnType result;
	OmHandler omHandler;
	vector<ACS_CC_ValuesDefinitionType> attrList;
	std::string fullRDN = "vlanId=" + vlanName;
	int pcpValue = 6, reservedValue = 0, stackValue = 0, vlanTypeValue = 0;
	int vlantag = vlanTag;

	ACS_CC_ValuesDefinitionType attributeRDN = defineAttributeString("vlanId",
			ATTR_STRINGT, fullRDN.c_str(), 1);
	attrList.push_back(attributeRDN);

	ACS_CC_ValuesDefinitionType attributeName = defineAttributeString("name",
			ATTR_STRINGT, vlanName.c_str(), 1);
	attrList.push_back(attributeName);

	ACS_CC_ValuesDefinitionType attributeStack = defineAttributeInt("stack",
			ATTR_INT32T, &stackValue, 1);
	attrList.push_back(attributeStack);

	ACS_CC_ValuesDefinitionType attributeReserved = defineAttributeInt(
			"reserved", ATTR_INT32T, &reservedValue, 1);
	attrList.push_back(attributeReserved);

	ACS_CC_ValuesDefinitionType attributePcp = defineAttributeInt("pcp",
			ATTR_INT32T, &pcpValue, 1);
	attrList.push_back(attributePcp);

	ACS_CC_ValuesDefinitionType attributeVlanType = defineAttributeInt(
			"vlanType", ATTR_INT32T, &vlanTypeValue, 1);
	attrList.push_back(attributeVlanType);

	ACS_CC_ValuesDefinitionType attributeVlanTag = defineAttributeInt("vlanTag",
			ATTR_UINT32T, &vlantag, 1);
	attrList.push_back(attributeVlanTag);

	char* classname = "AxeEquipmentVlan";
	const char* parentName = "vlanCategoryId=1,AxeEquipmentequipmentMId=1";

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS) {
		res = false;
	}

	else {
		int id = 0;
		char* text;
		result = omHandler.createObject(classname, parentName, attrList);
		id = omHandler.getInternalLastError();
		text = omHandler.getInternalLastErrorText();
		if (result != ACS_CC_SUCCESS) {
			res = false;
			//FIXS_ITH_LOG_ERRNO( "createImmObject_NO_OI() failed ***********%d::::%s ",id,text);
		} else {
			res = true;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS) {
			res = false;
		}
	}

	return res;
}

/*============================================================================
 ROUTINE: defineAttributeString
 ============================================================================ */
ACS_CC_ValuesDefinitionType ConfigurationManager::defineAttributeString(
		const char* attributeName, ACS_CC_AttrValueType type, const char *value,
		unsigned int numValue) {
	ACS_CC_ValuesDefinitionType attribute;
	char* stringValue = const_cast<char *>(value);

	attribute.attrName = const_cast<char*>(attributeName);
	attribute.attrType = type;
	attribute.attrValuesNum = numValue;

	if (attribute.attrValuesNum == 0)
		attribute.attrValues = 0;
	else {
		attribute.attrValues = new void*[attribute.attrValuesNum];
		attribute.attrValues[0] = reinterpret_cast<void*>(stringValue);
	}

	return attribute;
}

/*============================================================================
 ROUTINE: defineAttributeInt
 ============================================================================ */

ACS_CC_ValuesDefinitionType ConfigurationManager::defineAttributeInt(
		const char* attributeName, ACS_CC_AttrValueType type, int *value,
		unsigned int numValue) {
	ACS_CC_ValuesDefinitionType attribute;

	attribute.attrName = const_cast<char*>(attributeName);
	attribute.attrType = type;
	attribute.attrValuesNum = numValue;

	if (attribute.attrValuesNum == 0)
		attribute.attrValues = 0;
	else {
		attribute.attrValues = new void*[attribute.attrValuesNum];
		attribute.attrValues[0] = reinterpret_cast<void*>(value);
	}

	return attribute;
}


ACS_CC_ImmParameter ConfigurationManager::defineParameterInt(const char* attributeName, ACS_CC_AttrValueType type, int *value, unsigned int numValue)
{
        ACS_CC_ImmParameter parToModify;

        char *name_attrUnsInt32 = const_cast<char*>(attributeName);
        parToModify.attrName = name_attrUnsInt32;
        parToModify.attrType = type;
        parToModify.attrValuesNum = numValue;

        if (parToModify.attrValuesNum == 0)     parToModify.attrValues = 0;
        else
        {
                parToModify.attrValues=new void*[parToModify.attrValuesNum];
                parToModify.attrValues[0] =reinterpret_cast<void*>(value);
        }

return parToModify;

}

bool ConfigurationManager::replaceVlanIdInAxeequipment(
		const std::string vlanName, const fixs_ith::switchboard_plane_t & plane,
		int vlanTag) {
	bool result = false;

	if (((vlanName == engine::updVlan::transport::left::NAME)
			|| (vlanName == engine::updVlan::transport::cmxleft::NAME))
			&& ((plane == fixs_ith::TRANSPORT_PLANE))) {
		char dnValue[IMM_DN_PATH_SIZE_MAX] = { 0 };
		::snprintf(dnValue, IMM_DN_PATH_SIZE_MAX,
				"vlanId=APZ-UPD-A,vlanCategoryId=1,AxeEquipmentequipmentMId=1");

		ACS_CC_ImmParameter attributeVlanTag = defineParameterInt("vlanTag",
				ATTR_UINT32T, &vlanTag, 1);

		if (modifyVlanTag(dnValue, attributeVlanTag)) {
			result = true;
		} else {
			result = false;
		}
		delete[] attributeVlanTag.attrValues;

	}

	if (((vlanName == engine::updVlan::transport::right::NAME)
			|| (vlanName == engine::updVlan::transport::cmxright::NAME))
			&& ((plane == fixs_ith::TRANSPORT_PLANE))) {
		char dnValue[IMM_DN_PATH_SIZE_MAX] = { 0 };
		::snprintf(dnValue, IMM_DN_PATH_SIZE_MAX,
				"vlanId=APZ-UPD-B,vlanCategoryId=1,AxeEquipmentequipmentMId=1");

		ACS_CC_ImmParameter attributeVlanTag = defineParameterInt("vlanTag",
				ATTR_UINT32T, &vlanTag, 1);
		if (modifyVlanTag(dnValue, attributeVlanTag)) {
			result = true;
		} else {
			result = false;
		}
		delete[] attributeVlanTag.attrValues;

	}

	return result;
}

bool ConfigurationManager::modifyVlanTag(const char *object,
		ACS_CC_ImmParameter attributeVlanTag) {
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS) {
		res = false;
	} else {
		result = omHandler.modifyAttribute(object, &attributeVlanTag);
		if (result != ACS_CC_SUCCESS) {
			int id = 0;
			char * text;
			id = omHandler.getInternalLastError();
			text = omHandler.getInternalLastErrorText();
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS) {
			res = false;
		}
	}

	return res;
}

}//namespace


