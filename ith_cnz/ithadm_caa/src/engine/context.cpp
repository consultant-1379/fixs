/*
 * context.cpp
 *
 *  Created on: Mar 15, 2016
 *      Author: eanform
 */

#include "engine/context.h"
#include "engine/vrrpsession.h"
#include "engine/bfdsessionipv4.h"
#include "engine/bfdprofile.h"
#include "engine/aclentryipv4.h"
#include "engine/alcipv4.h"

#include "engine/basicNetworkConfiguration.h"
#include "engine/configurationManager.h"
#include "imm/imm.h"

#include "operation/pim_adaptation.h"

#include "common/utility.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

#include "fixs_ith_programconstants.h"
//#include "fixs_ith_switchInterface.h"
#include "switch_interface/full_adapter.h"


namespace engine
{

	Context::Context(const std::string& smxId)
	:m_removeOngoing(false),
	 m_smxId(smxId),
	 m_bridgeMap(),
	 m_ethernetPortMap(),
	 m_bridgePortMap(),
	 m_aggregatorMap(),
	 m_vlanMap(),
	 m_subnetVlanMap(),
	 m_routerSet(),
	 m_interfaceMap(),
	 m_vrrpSessions(),
	 m_addressMap(),
	 m_dstMap(),
	 m_nextHopMap(),
	 m_vrrpInterfaceSet(),
	 m_bfdProfileMap(),
	 m_bfdSessions(),
	 m_aclEntries(),
	 m_aclIpV4(),
	 m_switchInterface(),
	 m_contextMutex(),
	 m_contextLock()
	{

	}

	Context::~Context()
	{
		clear();
	}

	fixs_ith::ErrorConstants Context::setBridge(boost::shared_ptr<Bridge>& item)
	{
		if (NULL != getBridgeByName(item->getPlane(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceBridge(item);
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridge_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		if(fixs_ith::TRANSPORT_PLANE == item->getPlane())
		{
			fixs_ith_switchInterface::op_result vrrpNotifyEnable = m_switchInterface.enable_vrrp_new_master_notification(m_smxId);
			if(fixs_ith_switchInterface::SBIA_OK != vrrpNotifyEnable)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to enable VRRP new master notification. Bridge <%s>. Switch Interface Error <%d>",
									m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), vrrpNotifyEnable);
				op_result = vrrpNotifyEnable;
			}

			fixs_ith_switchInterface::op_result bfdNotifyEnable = m_switchInterface.enable_bfd_session_state_notification(m_smxId);
			if(fixs_ith_switchInterface::SBIA_OK !=  bfdNotifyEnable)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to enable BFD session state notification. Bridge <%s>. Switch Interface Error <%d>",
									m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(),  bfdNotifyEnable);
				op_result = bfdNotifyEnable;
			}

			fixs_ith_switchInterface::op_result aclHandlingEnable = m_switchInterface.enable_Acl(m_smxId);
			if(fixs_ith_switchInterface::SBIA_OK !=  aclHandlingEnable)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to enable ACL handling. Bridge <%s>. Switch Interface Error <%d>",
									m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(),  aclHandlingEnable);
				op_result = aclHandlingEnable;
			}
		}

		if(fixs_ith_switchInterface::SBIA_OK != op_result)
		{
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}
		else
		{
			m_bridgeMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Bridge> >(item->getPlane(), item));
		}

		return result;
	}

fixs_ith::ErrorConstants Context::setBridgePort(
		boost::shared_ptr<BridgePort>& item) {
	if (NULL != getBridgePortByName(item->getPlane(), item->getName())) {
		//Item already exists. Try to replace it
		return replaceBridgePort(item);
	}

	fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

	boost::lock_guard<boost::recursive_mutex> guard(
			m_contextMutex[bridgePort_idx]);

	fixs_ith_switchInterface::op_result op_result =
			fixs_ith_switchInterface::SBIA_OK;

	uint16_t port_snmpIndex = item->getSnmpIndex();
	fixs_ith::switchboard_plane_t switch_plane = item->getPlane();

	if (fixs_ith::TRANSPORT_PLANE == switch_plane) {
		if (isBackPlanePort(switch_plane, port_snmpIndex)
				&& (op_result = m_switchInterface.configure_bp_port_as_edgeport(
						m_smxId, switch_plane, port_snmpIndex))
						!= fixs_ith_switchInterface::SBIA_OK) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"[%s][%s] Failed to CONFIGURE Port %s as EDGE PORT on Bridge <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(),
					common::utility::planeToString(switch_plane).c_str(),
					item->getSmxId(), item->getName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}

		if (op_result == fixs_ith_switchInterface::SBIA_OK
				&& (!(isAggregatorMember(switch_plane, port_snmpIndex))
						&& (op_result = m_switchInterface.disable_rstp(m_smxId,
								switch_plane, port_snmpIndex))
								!= fixs_ith_switchInterface::SBIA_OK)) {

			FIXS_ITH_LOG(LOG_LEVEL_WARN,
					"[%s][%s] Failed to Disable RSTP of BridgePort <%s> Bridge <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(),
					common::utility::planeToString(switch_plane).c_str(),
					item->getSmxId(), item->getName(), op_result);

			/*
			 * TR HV79790: in some cases (for example after Maiden Installation of APG on a node where the SMX boards are already configured, including the LAGs),
			 * it may occur that the Bridge port is NOT member of any AGGREGATOR in the AxeInfrastructureTransportM MOM, but it is REALLY a member of an AGGREGATOR
			 * on the SMX board. In such cases, the "disable_rstp" operation fails but it is not a real problem.
			 * On the other hand, even if the Bridge Port is not currently a member of any LAG on the SMX board, it is likely that a "disable_rstp" failure does not compromise SMX board work.
			 * So, we  log the failure but to not consider it a blocking issue.
			 */
			op_result = fixs_ith_switchInterface::SBIA_OK;
			//result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}
	}
	//Disable RSTP on TRASPORT PORTS of Control switches
	else if (isTransportPort(port_snmpIndex)
			&& (op_result = m_switchInterface.disable_rstp(m_smxId,
					switch_plane, port_snmpIndex))
					!= fixs_ith_switchInterface::SBIA_OK) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"[%s][%s] Failed to Disable RSTP of BridgePort <%s> Bridge <%s>. Switch Interface Error <%d>",
				m_smxId.c_str(),
				common::utility::planeToString(switch_plane).c_str(),
				item->getSmxId(), item->getName(), op_result);
		result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
	}
	//TR HV35619
	//TR_HX92638 BEGIN
	// We need to set admin state for backplane ports corresponding to APUBs
	int32_t apubslot(0);
	int32_t partnerapubslot(0);
	string apubslot_bpportname;
	string partner_apubslot_bpportname;
	if (fixs_ith::ERR_NO_ERRORS
			== fixs_ith::workingSet_t::instance()->get_cs_reader().get_my_slot(
					apubslot)) {
		apubslot_bpportname = "BP_" + common::utility::slot_to_string(apubslot);
	}
	if (fixs_ith::ERR_NO_ERRORS
			== fixs_ith::workingSet_t::instance()->get_cs_reader().get_my_partner_slot(
					partnerapubslot)) {
		partner_apubslot_bpportname = "BP_"
				+ common::utility::slot_to_string(partnerapubslot);
	}
	if (fixs_ith_switchInterface::SBIA_OK == op_result
			&& ((item->getChangeMask()
					& imm::bridgePort_attribute::ADM_STATE_CHANGE)
					|| ((strcmp(apubslot_bpportname.c_str(), item->getName())
							== 0)
							|| (strcmp(partner_apubslot_bpportname.c_str(),
									item->getName()) == 0)))) {
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "apub slot %s",
				apubslot_bpportname.c_str());
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "apub partner slot %s",
				partner_apubslot_bpportname.c_str());
		FIXS_ITH_LOG(LOG_LEVEL_WARN,
				"Setting admin_state for EARLY CONFIGURED port %d on %s plane",
				port_snmpIndex,
				common::utility::planeToString(switch_plane).c_str());
		op_result =
				m_switchInterface.set_if_adminState(m_smxId, switch_plane,
						item->getSnmpIndex(),
						(fixs_ith_switchInterface::BP_admin_state) item->getAdminState());
		if (op_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"[%s][%s] Failed to set admin state on BridgePort <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(),
					common::utility::planeToString(switch_plane).c_str(),
					item->getName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}

		// Setting default priority to ports where apub is inserted in case of CMX as switchboard
		if (common::utility::isSwitchBoardCMX()) {
			if (op_result == fixs_ith_switchInterface::SBIA_OK) {
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"Setting default port priority for EARLY CONFIGURED port %d on %s plane",
						port_snmpIndex,
						common::utility::planeToString(switch_plane).c_str());
				op_result = m_switchInterface.set_if_priority(m_smxId,
						switch_plane, item->getSnmpIndex());
				if (op_result) {
					FIXS_ITH_LOG(LOG_LEVEL_ERROR,
							"[%s][%s] Failed to set default port priority of BridgePort <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(),
							common::utility::planeToString(switch_plane).c_str(),
							item->getName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;

				}
			} else {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Not Setting the default port priority for EARLY CONFIGURED port %d on %s plane as the admin status of that port is not UP",
						port_snmpIndex,
						common::utility::planeToString(switch_plane).c_str());
			}

		}

	}
	//TR_HX92638 END
	//TODO: ELIMINARE
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				">>>> SKIPPED setting admin_state for port %d on %s plane",
				port_snmpIndex,
				common::utility::planeToString(switch_plane).c_str());

	if (fixs_ith_switchInterface::SBIA_OK == op_result) {
		//Check if the bridgePort is part of an aggregator or is an aggregated port
		bool isAggMember = isAggregatorMember(switch_plane,
				item->getSnmpIndex());

		bool isAggPort = isAggregatedPort(switch_plane, item->getSnmpIndex());

		if (!isAggPort && !isAggMember) {
			//It's an ethernet port which doesn't belong to any aggregator
			//Clean the aggregator admin key
			std::set<uint16_t> members;
			members.insert(item->getSnmpIndex());

			fixs_ith_switchInterface::op_result op_result =
					m_switchInterface.set_aggregator_members(m_smxId,
							switch_plane, 0, members);

			if (fixs_ith_switchInterface::SBIA_OK != op_result) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"[%s][%s] Failed to set  dot3adAggPortActorAdminKey on BridgePort <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(),
						common::utility::planeToString(switch_plane).c_str(),
						item->getName(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}
		}

		if (!isAggMember) {
			//TODO:: Temporary WA: Create VLAN (if not existing) otherwise setting default VLAN ID would fail.
			if (imm::vlanId_range::DEFAULT != item->getDefaultVlanId()) {
				m_switchInterface.create_vlan(m_smxId, switch_plane,
						item->getDefaultVlanId(), "-");
			}

			fixs_ith_switchInterface::op_result op_result =
					m_switchInterface.set_port_default_vlan(m_smxId,
							switch_plane, item->getSnmpIndex(),
							item->getDefaultVlanId());

			if (fixs_ith_switchInterface::SBIA_OK != op_result) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"[%s][%s] Failed to set default VLAN ID on BridgePort <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(),
						common::utility::planeToString(switch_plane).c_str(),
						item->getName(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}
		}

		/*	Temporary patch for CP. To be removed.
		 //Enable OAMPDU exchange on monitored link ports
		 if (isMonitoredPortLink(switch_plane, item->getSnmpIndex()))
		 {
		 //Enable OAM on monitored port
		 m_switchInterface.set_eth_oam_adminState(m_smxId, switch_plane, item->getSnmpIndex(), switch_interface::OAM_ENABLED);
		 }
		 */

	}

	if (fixs_ith::ERR_NO_ERRORS == result) {
		m_bridgePortMap.insert(
				std::pair<fixs_ith::SwitchBoardPlaneConstants,
						boost::shared_ptr<BridgePort> >(switch_plane, item));
	}

	return result;
}

	fixs_ith::ErrorConstants Context::setEthernetPort(boost::shared_ptr<EthernetPort>& item)
	{
		if (NULL != getEthernetPortByName(item->getPlane(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceEthernetPort(item);
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[ethernetPort_idx]);

		int32_t agg_actor_admin_key = 0;

		uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;
		fixs_ith_switchInterface::op_result op_result = m_switchInterface.get_agg_actor_admin_key(agg_actor_admin_key, item->getSnmpIndex(), m_smxId, item->getPlane(), &timeout_ms);


		if (fixs_ith_switchInterface::SBIA_OK != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set max frame size on EthernetPort <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}
		else
		{
			if (0 == agg_actor_admin_key)
			{
				op_result = m_switchInterface.set_eth_max_frame_size(m_smxId, item->getPlane(), item->getSnmpIndex(), item->getMaxFrameSize());

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set max frame size on EthernetPort <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
			}
		}
		//BEGIN : TR HY85746
		if(common::utility::isSwitchBoardCMX())
		{
			int ifIndex = item->getSnmpIndex();
			if(ifIndex == 131 || ifIndex == 136 || ifIndex == 141 || ifIndex == 146)
			{
				// Fetch the Auto-negotiation status in IMM
				if(item->getAutoNegotiate() == imm::AN_LOCKED)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Setting the eth mau type to 10G for port %s", m_smxId.c_str());
					fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_eth_if_mau_type(m_smxId,item->getPlane(),item->getSnmpIndex(),fixs_ith_snmp::IF_MAU_TYPE_10G_BASE_CX4);
					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to set if mau type for EthernetPort <%s>. Switch Interface Error <%d>",m_smxId.c_str(),common::utility::planeToString(item->getPlane()).c_str(),item->getName(), op_result);
						return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}

					FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Setting the auto nego status to disabled for port %s",m_smxId.c_str());
					op_result = m_switchInterface.set_eth_auto_negotiation(m_smxId, item->getPlane(), item->getSnmpIndex(), false);
					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to disable auto negotiation on EthernetPort <%s>. Switch Interface Error <%d>",m_smxId.c_str(),common::utility::planeToString(item->getPlane()).c_str(),item->getName(), op_result);

						 FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Restoring the mau type set on ethernet port %s  to 40G, as auto-negotiation disabled setting got failed",item->getName());

						op_result = m_switchInterface.set_eth_if_mau_type(m_smxId,item->getPlane(),item->getSnmpIndex(),fixs_ith_snmp::IF_MAU_TYPE_40G_BASE_CR4);

						if (fixs_ith_switchInterface::SBIA_OK != op_result)
						{
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Failed to set the mau type on ethernet port %s  to 40G",item->getName());
							return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
						}
						else
						{
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Restored the mau type to 40G on ethernet port %s  to 40G",item->getName());
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Returning failure as the auto-negotiation setting to disable on ethernet port %s got failed",item->getName());
							return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
						}

					}
				
				}	

			}

		}
		//END : TR HY85746

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_ethernetPortMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<EthernetPort> >(item->getPlane(), item));
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setAggregator(boost::shared_ptr<Aggregator>& item)
	{
		if (NULL != getAggregatorByName(item->getPlane(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceAggregator(item);
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aggregator_idx]);

		// Set the proper ports in default vlan BEFORE creating the aggregator
		if (fixs_ith::CONTROL_PLANE == item->getPlane())
		{
			m_switchInterface.set_vlan_members(item->getSmxId(), item->getPlane(), imm::vlanId_range::DEFAULT, engine::defaultVlan::control::TAGGED_PORTS_INDEXES_SET, engine::defaultVlan::control::UNTAGGED_PORTS_INDEXES_SET);
		}
		else if (fixs_ith::TRANSPORT_PLANE == item->getPlane())
		{
			m_switchInterface.set_vlan_members(item->getSmxId(), item->getPlane(), imm::vlanId_range::DEFAULT, engine::defaultVlan::transport::TAGGED_PORTS_INDEXES_SET, engine::defaultVlan::transport::UNTAGGED_PORTS_INDEXES_SET);
			
		}

		fixs_ith_switchInterface::op_result op_result = m_switchInterface.create_aggregator(m_smxId, item->getPlane(), item->getSnmpIndex(), item->getName());

		if (fixs_ith_switchInterface::SBIA_OK != op_result &&
				fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to create aggregator <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}
		else
		{ //Aggregator created or already existing
			std::set<uint16_t > aggregatorIndexes = item->getAdminAggMemebrIndex();
//			std::set<uint16_t > allIndexes = getEthernetPortsIndexes(item->getPlane());
//
//			for (std::set<uint16_t >::iterator it = aggregatorIndexes.begin(); it != aggregatorIndexes.end(); ++it)
//			{
//				allIndexes.erase(*it);
//			}
//			//Clean other ports
//			op_result = m_switchInterface.set_aggregator_members(m_smxId, item->getPlane(), 0, allIndexes);
//
//			if (fixs_ith_switchInterface::SBIA_OK != op_result)
//			{
//				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set admin members on aggregator <%s>. Switch Interface Error <%d>",
//						m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), op_result);
//				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
//			}
//			else
//			{

			op_result = m_switchInterface.set_aggregator_members(m_smxId, item->getPlane(), item->getSnmpIndex(), aggregatorIndexes);

			if (fixs_ith_switchInterface::SBIA_OK != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set admin members on aggregator <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}
//			}
		}

		if (fixs_ith_switchInterface::SBIA_OK == op_result)
		{
			m_aggregatorMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Aggregator> >(item->getPlane(), item));
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setVlan(boost::shared_ptr<Vlan>& item)
	{
		if (NULL != getVlanByName(item->getPlane(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceVlan(item);
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		if (item->getVlanId() != imm::vlanId_range::UNDEFINED)
		{
			fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

			op_result =	m_switchInterface.create_vlan(m_smxId, item->getPlane(), item->getVlanId(), item->getName());

			if (fixs_ith_switchInterface::SBIA_OK != op_result &&
					fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to create Vlan <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}
			else
			{// VLAN created or already existing
				op_result =	m_switchInterface.set_vlan_members(m_smxId, item->getPlane(), item->getVlanId(), item->getTaggedBridgePortsIndex(), item->getUntaggedBridgePortsIndex());
				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set tagged/untagged of members of Vlan <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					/* TR HX22060 BEGIN */
					if (item->getPlane() == fixs_ith::TRANSPORT_PLANE)
					{
						FIXS_ITH_LOG(LOG_LEVEL_WARN,"This VLAN is created on transport plane");
						std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts = item->getUntaggedBridgePorts();

						for ( std::set< boost::shared_ptr<BridgePort> >::iterator it = untaggedBridgePorts.begin (); it != untaggedBridgePorts.end(); ++it)
						{
							fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_port_default_vlan(m_smxId, (*it)->getPlane(), (*it)->getSnmpIndex(), item->getVlanId());
							std::cout << "addded untagged port is:"<< (*it)->getName () << std::endl;
							FIXS_ITH_LOG(LOG_LEVEL_WARN, "Addded untagged port is: <%s>", (*it)->getName ());
							if (fixs_ith_switchInterface::SBIA_OK != op_result )
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set default Vlan of untagged port <%s>. Switch Interface Error <%d>",
										m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getName(), op_result);
								result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
								break;
							}
						}
					}
					/* TR HX22060 END */
				}
			}
		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_vlanMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Vlan> >(item->getPlane(), item));
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setSubnetVlan(boost::shared_ptr<SubnetVlan>& item)
	{
		if (NULL != getSubnetVlanByName(item->getPlane(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceSubnetVlan(item);
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[subnetVlan_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		op_result =	m_switchInterface.create_ipv4subnet(m_smxId, item->getPlane(),item->getIpSubnet());
		if (fixs_ith_switchInterface::SBIA_OK != op_result &&
				fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to create SubnetVlan <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getIpSubnet(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}
		else
		{// SUBNET VLAN created or already existing

			fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_ipv4subnet_vlanid(m_smxId, item->getPlane(), item->getIpSubnet(), item->getVlanId());
			if (fixs_ith_switchInterface::SBIA_OK != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set vid <%d> for SubnetVlan <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(), common::utility::planeToString(item->getPlane()).c_str(), item->getVlanId(), item->getIpSubnet(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}
		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_subnetVlanMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<SubnetVlan> >(item->getPlane(), item));
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setRouter(boost::shared_ptr<Router>& item)
	{
		if (NULL != getRouterByName(item->getName()))
		{
			//Item already exists. Try to replace it
			//return replaceRouter(item);
			return fixs_ith::ERR_NO_ERRORS;
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[router_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		op_result = m_switchInterface.create_router(item->getData());

		if (fixs_ith_switchInterface::SBIA_OK != op_result &&
				fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to create Router <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), item->getName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_routerSet.insert(item);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setInterface(boost::shared_ptr<InterfaceIPv4>& item)
	{
		if (NULL != getInterfaceByName(item->getRouterName(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceInterface(item, item->getData());
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[interface_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		//TODO: Loopback interface is created by default...
		if (!item->isLoopback())
		{
			item->setAclInfo();

			operation::interfaceInfo info(item->getData());

			op_result = m_switchInterface.create_interfaceIPv4(info);

			if (fixs_ith_switchInterface::SBIA_OK != op_result &&
					fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
			{

				//TODO: Creation "merge" failed. It might be because the object already exists.
				//Let's try to get the remote object.
				//Perhaps we should check if the object already exists before retrying

				operation::interfaceInfo remote_info(info);
				remote_info.changeMask = imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE;
				op_result = m_switchInterface.get_interfaceIPv4_info(remote_info);

				if (op_result != fixs_ith_switchInterface::SBIA_OK)
				{
					//TODO: Proper error code
					return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}

				//Local encapsulation is in the format: vlanId=<value>,...
				char localVlanId[8] = {0};
				snprintf(localVlanId, 8, "%d", item->getEncaspulationVid());
				std::string local_encapsulation(localVlanId);


				//Remote encapsulation is in the format: ManagedElement=1,...,Vlan=<value>
				std::string remote_encapsulation;
				size_t pos = remote_info.encapsulation.find_last_of("=");
				if (pos != std::string::npos)
				{
					remote_encapsulation.assign(remote_info.encapsulation.substr(pos + 1));
				}

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Local Encapsulation <%s>.", local_encapsulation.c_str());
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Remote Encapsulation <%s>.", remote_encapsulation.c_str());
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Remote DN <%s>.", remote_info.encapsulation.c_str());

				if (local_encapsulation.compare(remote_encapsulation) == 0)
				{
					//Remote restricted attributes match the ones we want to set. It should be possible to set unrestricted attributes only
					//Let's try to remove restricted attribute and create it again
					info.changeMask &= ~imm::interfaceIpv4_attribute::RESTRICTED_ATTRIBUTES;
					op_result = m_switchInterface.create_interfaceIPv4(info);
				}
				else
				{
					//Restricted attributes DO NOT match. We shall delete the existing object and create a new one.
					//But we need to delete all its children first.
					//How can we do this?
					//TODO
					return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}

				op_result = m_switchInterface.create_interfaceIPv4(info);
				if (fixs_ith_switchInterface::SBIA_OK != op_result &&
						fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
				{
					//Definitely failed...
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to create Interface <%s> on Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), item->getName(), item->getRouterName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
			}
		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_interfaceMap.insert(std::pair<std::string, boost::shared_ptr<InterfaceIPv4> >(item->getRouterName(),item));
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Interface:<%s> on Router:<%s> created.",
					m_smxId.c_str(), item->getName(), item->getRouterName());
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setRouterInterfaceAddress(boost::shared_ptr<Address>& item)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "setRouterInterfaceAddress() entering");

		if (NULL != getRouterInterfaceAddressByName(item->getRouterName(), item->getInterfaceName(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceRouterInterfaceAddress(item, item->getData());
		}


		if(checkRouterNextHopsAddress(item->getRouterName(), item->getData().address))
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Specified address in <%s> conflicts with another address on the same Router <%s>",
					m_smxId.c_str(), item->getName(), item->getRouterName());
			return fixs_ith::ERR_CONFIG_ADDRESS_CONFLICT;
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		boost::shared_ptr<InterfaceIPv4> interface = getInterfaceByName(item->getRouterName(), item->getInterfaceName());

		//TODO: Interface loopback is automatically created..
		if (NULL != interface && !interface->isLoopback())
		{

			operation::addressInfo addressIPv4Info(item->getData());
			addressIPv4Info.address.clear();
			op_result = m_switchInterface.get_addressIPv4(addressIPv4Info);

			if (fixs_ith_switchInterface::SBIA_OK != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to get addressIPv4 <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(),  item->getRouterName(), addressIPv4Info.getName(), op_result);
				//result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}

			std::string smx_address = addressIPv4Info.getAddress();
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "setRouterInterfaceAddress():: smx_address == %s", smx_address.c_str());
			if (smx_address.compare(item->getData().address))
			{
				op_result = m_switchInterface.create_addressIPv4(item->getData());

				if (fixs_ith_switchInterface::SBIA_OK != op_result &&
						fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to create Address <%s> on Interface <%s> of Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), item->getName(), item->getInterfaceName(), item->getRouterName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
			}
		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_addressMap.insert(std::pair<std::string, boost::shared_ptr<Address> >(item->getRouterName(),item));
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Address:<%s> on Interface:<%s> of Router:<%s> created.",
					m_smxId.c_str(), item->getName(), item->getInterfaceName(), item->getRouterName());
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setVrrpSession(boost::shared_ptr<VrrpSession>& newVrrpSession)
	{
		if(NULL != getVrrpSessionByName(newVrrpSession->getRouterName(),  newVrrpSession->getInterfaceName(), newVrrpSession->getName()))
		{
			//Item already exists. Try to replace it
			return replaceVrrpSession(newVrrpSession);
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);

		fixs_ith_switchInterface::op_result op_result = m_switchInterface.create_vrrpv3Session(newVrrpSession->getData());

		if (fixs_ith_switchInterface::SBIA_OK != op_result &&
				fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to create VrrpSession:<%s> on Interface:<%s> of Router:<%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), newVrrpSession->getName(), newVrrpSession->getInterfaceName(), newVrrpSession->getRouterName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			newVrrpSession->resetChangeMask();
			m_vrrpSessions.insert(std::make_pair(newVrrpSession->getRouterName(), newVrrpSession));
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] VrrpSession:<%s> on Interface:<%s> of Router:<%s> created.",
					m_smxId.c_str(), newVrrpSession->getName(), newVrrpSession->getInterfaceName(), newVrrpSession->getRouterName());
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setDst(boost::shared_ptr<Dst>& item)
	{
		if (NULL != getDstByName(item->getRouterName(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceDst(item, item->getData());
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[dst_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		// Check if DST is already configured on SMX
		operation::dstInfo dst_info(item->getData());

		op_result = m_switchInterface.get_Dst(dst_info);

		if(op_result != fixs_ith_switchInterface::SBIA_OK )
		{
			// DST is not configured on SMX or we were unable to verify it.
			// Try to create it.
			op_result = m_switchInterface.create_Dst(item->getData());
		}
		else
		{
			if( dst_info.dst != item->getData().dst )
			{
				// DST is configured on SMX but with a different destination. Try to modify it.
				m_switchInterface.modify_Dst(item->getData());
			}
		}

		if (fixs_ith_switchInterface::SBIA_OK != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to create Dst <%s> on Router <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), item->getName(), item->getRouterName(), op_result);

			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}


		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_dstMap.insert(std::pair<std::string, boost::shared_ptr<Dst> >(item->getRouterName(),item));
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setNextHop(boost::shared_ptr<NextHop>& item)
	{
		if (NULL != getNextHopByName(item->getRouterName(), item->getDstName(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceNextHop(item, item->getData());
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		// Check if NextHop is already configured on SMX
		operation::nextHopInfo nextHop_info(item->getData());
		op_result = m_switchInterface.get_NextHopInfo(nextHop_info);

		if(op_result == fixs_ith_switchInterface::SBIA_OK)
		{
			bool disalignment_found = false;
			// NextHop is already configured.
			// Now check if all attribute's value are the same.
			(!disalignment_found) && (nextHop_info.address != item->getData().address) &&
					(disalignment_found = true);

			(!disalignment_found) && (nextHop_info.adminDistance != item->getData().adminDistance) &&
					(disalignment_found = true);

			(!disalignment_found) && (nextHop_info.bfdMonitoring != item->getData().bfdMonitoring) &&
					(disalignment_found = true);

			(!disalignment_found) && (nextHop_info.discard != item->getData().discard) &&
					(disalignment_found = true);

			//NEXTHOP is already configured with the desired attribute's values. No need to create it on the board!
			if (!disalignment_found)
			{
				m_nextHopMap.insert(std::pair<std::string, boost::shared_ptr<NextHop> >(item->getRouterName(),item));
				return result;
			}
		}

		// NEXTHOP is not configured on SMX or we were unable to verify it.
		// Try to create it.

		op_result = m_switchInterface.create_NextHopInfo(item->getData());

		if (fixs_ith_switchInterface::SBIA_OK != op_result &&
				fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
		{
			//TODO: NextHop creation failed... We need to figure out the proper error
			//Temporary WA: let's try to delete it and create it again
			m_switchInterface.delete_NextHopInfo(item->getData());

			op_result = m_switchInterface.create_NextHopInfo(item->getData());
			if (fixs_ith_switchInterface::SBIA_OK != op_result &&
					fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to create NextHop <%s> on Dst <%s> of Router <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(), item->getName(), item->getDstName(), item->getRouterName(), op_result);
				return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;

			}
		}

		m_nextHopMap.insert(std::pair<std::string, boost::shared_ptr<NextHop> >(item->getRouterName(),item));

		return result;
	}

	fixs_ith::ErrorConstants Context::setVrrpInterface(boost::shared_ptr<VrrpInterface>& item)
	{
		if (NULL != getVrrpInterfaceByName(item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceVrrpInterface(item, item->getData());
		}

		fixs_ith::ErrorConstants result = checkVrIdentityUniqueness(item->getData().vrIdentity);

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "vrIdentity <%d> is not unique.", item->getData().vrIdentity);
			return result;
		}

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpInterface_idx]);

		m_vrrpInterfaceSet.insert(item);

		return result;
	}

	fixs_ith::ErrorConstants Context::setVrrpInterfaceAddress(boost::shared_ptr<Address>& item)
	{
		if (NULL != getVrrpInterfaceAddressByName(item->getInterfaceName(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceVrrpInterfaceAddress(item, item->getData());
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		m_addressMap.insert(std::pair<std::string, boost::shared_ptr<Address> >(item->getRouterName(),item));

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Inserted address <%s> into addressMap, Router name <%s>, Interface name <%s>",
				item->getData().address.c_str(), item->getRouterName(), item->getInterfaceName());

		return result;
	}

	fixs_ith::ErrorConstants Context::setBfdProfile(boost::shared_ptr<BfdProfile>& item)
	{
		if (NULL != getBfdProfileByName(item->getRouterName(), item->getName()))
		{
			//Item already exists. Try to replace it
			return replaceBfdProfile(item, item->getData());
		}

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdProfile_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		op_result = m_switchInterface.create_BfdProfile(item->getData());

		if (fixs_ith_switchInterface::SBIA_OK != op_result &&
				fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to create BfdProfile <%s> on Router <%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), item->getName(), item->getRouterName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_bfdProfileMap.insert(std::pair<std::string, boost::shared_ptr<BfdProfile> >(item->getRouterName(),item));
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Inserted BfdProfile <%s> into bfdProfileMap, Router name <%s>",
					item->getName(), item->getRouterName());
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setBfdSessionIPv4(boost::shared_ptr<BfdSessionIPv4>& newBfdSession)
	{
		// check if it is already defined
		if(NULL != getBfdSessionIPv4ByName(newBfdSession->getRouterName(), newBfdSession->getName()))
		{
			//Item already exists. Try to replace it
			return replaceBfdSessionIPv4(newBfdSession);
		}

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdSession_idx]);

		m_bfdSessions.insert(std::make_pair(newBfdSession->getRouterName(), newBfdSession));

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] BFD Session:<%s> of Router:<%s> added.",
				m_smxId.c_str(), newBfdSession->getName(),  newBfdSession->getRouterName() );

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants Context::setAclEntryIpv4(boost::shared_ptr<AclEntryIpv4>& newAclEntry)
	{
		// check if it is already defined
		if(NULL != getAclEntryIpv4ByName(newAclEntry->getRouterName(), newAclEntry->getName()))
		{
			//Item already exists. Try to replace it
			return replaceAclEntryIpv4(newAclEntry, newAclEntry->getData());
		}

		std::string routerName = newAclEntry->getRouterName();
		std::string aclIpv4Name = newAclEntry->getAclIpv4Name();
		std::string aclEntryName = newAclEntry->getName();
		fixs_ith::ErrorConstants result = checkAclEntryPriorityUniqueness(routerName, aclIpv4Name, aclEntryName, newAclEntry->getData().getPriority());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "priority <%d> is not unique.", newAclEntry->getData().getPriority());
			return result;
		}

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclEntry_idx]);

		fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

		op_result = m_switchInterface.create_AclEntry(newAclEntry->getData());

		if (fixs_ith_switchInterface::SBIA_OK != op_result &&
				fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to create AclEntryIpv4:<%s> on AclIpv4:<%s> of Router:<%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), newAclEntry->getName(), newAclEntry->getAclIpv4Name(), newAclEntry->getRouterName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}

		if (fixs_ith::ERR_NO_ERRORS == result)
		{
			m_aclEntries.insert(std::make_pair(newAclEntry->getRouterName(), newAclEntry));
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] AclEntryIpv4:<%s> on AclIpv4:<%s> of Router:<%s> added.",
					m_smxId.c_str(), newAclEntry->getName(), newAclEntry->getAclIpv4Name(), newAclEntry->getRouterName());
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::setAclIpV4(boost::shared_ptr<AclIpv4>& newAclIpV4)
	{
		// check if it is already defined
		if(NULL != getAclIpV4ByName(newAclIpV4->getRouterName(), newAclIpV4->getName()))
		{
			//Item already exists. Try to replace it
			return replaceAclIpv4(newAclIpV4);
		}

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclIpv4_idx]);


		m_aclIpV4.insert(std::make_pair(newAclIpV4->getRouterName(), newAclIpV4));

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] AclIpV4:<%s> of Router:<%s> added.",
				m_smxId.c_str(), newAclIpV4->getName(), newAclIpV4->getRouterName());

		return fixs_ith::ERR_NO_ERRORS;
	}

	bool Context::hasBridge(const fixs_ith::SwitchBoardPlaneConstants& plane)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridge_idx]);

		std::pair<bridgeMap_t::iterator, bridgeMap_t::iterator> bridgeRange = m_bridgeMap.equal_range(plane);

		return bridgeRange.first != m_bridgeMap.end();
	}

	bool Context::hasVlan(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& vlanId)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		bool result = false;

		std::pair<vlanMap_t::iterator, vlanMap_t::iterator> vlanRange = m_vlanMap.equal_range(plane);

		for (vlanMap_t::iterator element = vlanRange.first; element != vlanRange.second && !result; ++element)
		{
			result = (element->second->getVlanId() == vlanId);
		}

		return result;
	}

	bool Context::hasVlan(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		bool result = false;

		std::pair<vlanMap_t::iterator, vlanMap_t::iterator> vlanRange = m_vlanMap.equal_range(plane);

		for (vlanMap_t::iterator element = vlanRange.first; element != vlanRange.second && !result; ++element)
		{
			result = (name.compare(element->second->getName()) == 0);
		}

		return result;
	}

	bool Context::hasSubnetBasedVlan(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& vlanId)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[subnetVlan_idx]);

		bool result = false;

		std::pair<subnetVlanMap_t::iterator, subnetVlanMap_t::iterator> subnetVlanRange = m_subnetVlanMap.equal_range(plane);

		for (subnetVlanMap_t::iterator element = subnetVlanRange.first; element != subnetVlanRange.second && !result; ++element)
		{
			result = (element->second->getVlanId() == vlanId);
		}

		return result;
	}

	bool Context::hasSubnetBasedVlan(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[subnetVlan_idx]);

		bool result = false;

		std::pair<subnetVlanMap_t::iterator, subnetVlanMap_t::iterator> subnetVlanRange = m_subnetVlanMap.equal_range(plane);

		for (subnetVlanMap_t::iterator element = subnetVlanRange.first; element != subnetVlanRange.second && !result; ++element)
		{
			result = (name.compare(element->second->getName()) == 0);
		}

		return result;
	}

	bool Context::hasIpv4SubnetBasedVlan(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& subnet)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[subnetVlan_idx]);

		bool result = false;

		std::pair<subnetVlanMap_t::iterator, subnetVlanMap_t::iterator> subnetVlanRange = m_subnetVlanMap.equal_range(plane);

		for (subnetVlanMap_t::iterator element = subnetVlanRange.first; element != subnetVlanRange.second && !result; ++element)
		{
			result = (subnet.compare(element->second->getIpSubnet()) == 0);
		}

		return result;
	}

	boost::shared_ptr<Bridge> Context::getBridgeByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridge_idx]);

		boost::shared_ptr<Bridge> elementFound;

		std::pair<bridgeMap_t::iterator, bridgeMap_t::iterator> bridgeRange;
		bridgeRange = m_bridgeMap.equal_range(plane);

		for (bridgeMap_t::iterator element = bridgeRange.first; element != bridgeRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		return elementFound;
	}

	boost::shared_ptr<BridgePort> Context::getBridgePortByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name, bool loadRuntime)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridgePort_idx]);

		boost::shared_ptr<BridgePort> elementFound;

		std::pair<bridgePortMap_t::iterator, bridgePortMap_t::iterator> bridgePortRange;
		bridgePortRange = m_bridgePortMap.equal_range(plane);

		for (bridgePortMap_t::iterator element = bridgePortRange.first; element != bridgePortRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		if( (NULL != elementFound) && loadRuntime)
		{
			loadRuntimeInfo(elementFound);
		}

		return elementFound;
	}

	boost::shared_ptr<EthernetPort> Context::getEthernetPortByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name, bool loadRuntime)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[ethernetPort_idx]);

		boost::shared_ptr<EthernetPort> elementFound;

		std::pair<ethernetPortMap_t::iterator, ethernetPortMap_t::iterator> ethernetPortRange;
		ethernetPortRange = m_ethernetPortMap.equal_range(plane);

		for (ethernetPortMap_t::iterator element = ethernetPortRange.first; element != ethernetPortRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		if( (NULL != elementFound) && loadRuntime)
		{
			loadRuntimeInfo(elementFound);
		}

		return elementFound;

	}

	boost::shared_ptr<Aggregator> Context::getAggregatorByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name, bool loadRuntime)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aggregator_idx]);

		boost::shared_ptr<Aggregator> elementFound;

		std::pair<aggregatorMap_t::iterator, aggregatorMap_t::iterator> aggregatorRange;
		aggregatorRange = m_aggregatorMap.equal_range(plane);

		for (aggregatorMap_t::iterator element = aggregatorRange.first; element != aggregatorRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		if( (NULL != elementFound) && loadRuntime)
		{
			loadRuntimeInfo(elementFound);
		}

		return elementFound;

	}

	boost::shared_ptr<Vlan> Context::getVlanByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name, bool loadRuntimeInfo)
	{
		UNUSED(loadRuntimeInfo);
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		boost::shared_ptr<Vlan> elementFound;

		std::pair<vlanMap_t::iterator, vlanMap_t::iterator> vlanRange;
		vlanRange = m_vlanMap.equal_range(plane);

		for (vlanMap_t::iterator element = vlanRange.first; element != vlanRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		return elementFound;

	}
	
	/* HX22060 BEGIN
	 * 
	 */

	bool Context::IsUntaggedPort (const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& portDN, bool loadRuntimeInfo)
	{
		UNUSED(loadRuntimeInfo);
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		std::pair<vlanMap_t::iterator, vlanMap_t::iterator> vlanRange;
		vlanRange = m_vlanMap.equal_range(plane);

		bool result = false;
		std::cout << "Context::IsUntaggedPort port DN added" << portDN << std::endl;

		for (vlanMap_t::iterator element = vlanRange.first; element != vlanRange.second; ++element)
		{
			std::cout << "Context::IsUntaggedPort VLAN name:::" << element->second->getName() << std::endl;
			if (element->second->hasUntaggedPortDN(portDN))
			{
				std::cout << "Context::IsUntaggedPort port DN is assosiated with untagged VLAN" << portDN << std::endl;
				result = true;
				break;
			}
		}
		return result;
	}

	/* HX22060 END
	 * 
	 */

	boost::shared_ptr<SubnetVlan> Context::getSubnetVlanByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name, bool loadRuntimeInfo)
	{
		UNUSED(loadRuntimeInfo);
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[subnetVlan_idx]);

		boost::shared_ptr<SubnetVlan> elementFound;

		std::pair<subnetVlanMap_t::iterator, subnetVlanMap_t::iterator> subnetVlanRange;
		subnetVlanRange = m_subnetVlanMap.equal_range(plane);

		for (subnetVlanMap_t::iterator element = subnetVlanRange.first; element != subnetVlanRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		return elementFound;

	}

	boost::shared_ptr<Router> Context::getRouterByName(const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[router_idx]);

		boost::shared_ptr<Router> elementFound;

		for (routerSet_t::iterator element = m_routerSet.begin(); element != m_routerSet.end() && NULL == elementFound; ++element)
		{
			if (name.compare((*element)->getName()) == 0)
			{
				elementFound = *element;
			}
		}

		return elementFound;

	}

	boost::shared_ptr<InterfaceIPv4> Context::getInterfaceByName(const std::string routerName, const std::string& name, uint16_t runtimeAttributeMask)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[interface_idx]);

		std::pair<interfaceMap_t::iterator, interfaceMap_t::iterator> interfaceRange;
		interfaceRange = m_interfaceMap.equal_range(routerName);

		boost::shared_ptr<InterfaceIPv4> elementFound;

		for (interfaceMap_t::iterator element = interfaceRange.first; element != interfaceRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		if(NULL != elementFound)
		{
			loadRuntimeInfo(elementFound, runtimeAttributeMask);
		}


		return elementFound;

	}

	boost::shared_ptr<VrrpSession> Context::getVrrpSessionByName(const std::string& routerName, const std::string& interfaceName, const std::string& vrrpSessionName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);

		std::pair<vrrpSessions_t::iterator, vrrpSessions_t::iterator> vrrpSessionRange;
		vrrpSessionRange = m_vrrpSessions.equal_range(routerName);

		boost::shared_ptr<VrrpSession> elementFound;

		for(vrrpSessions_t::const_iterator element = vrrpSessionRange.first; vrrpSessionRange.second != element; ++element)
		{
			if(interfaceName.compare(element->second->getInterfaceName()) == 0 &&
					vrrpSessionName.compare(element->second->getName()) == 0 )
			{
				elementFound = element->second;
				break;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] VrrpSession:<%s> on Interface:<%s> of Router:<%s> %s.",
				m_smxId.c_str(), vrrpSessionName.c_str(), interfaceName.c_str(), routerName.c_str(), (elementFound ? "FOUND" : "NOT FOUND"));

		return elementFound;
	}

	boost::shared_ptr<VrrpSession> Context::getVrrpSessionByDN(const std::string& vrrpSessionDN)
	{
		std::string routerName( common::utility::getRouterNameFromVrrpSessionDN( vrrpSessionDN ) );
		std::string interfaceName( common::utility::getInterfaceNameFromVrrpSessionDN( vrrpSessionDN ) );
		std::string vrrpSessionName(common::utility::getVrrpSessionNameFromDN(vrrpSessionDN));
		return getVrrpSessionByName(routerName, interfaceName, vrrpSessionName);
	}

	boost::shared_ptr<Address> Context::getRouterInterfaceAddressByName(const std::string& routerName, const std::string interfaceName, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		std::pair<addressMap_t::iterator, addressMap_t::iterator> addressRange;
		addressRange = m_addressMap.equal_range(routerName);

		boost::shared_ptr<Address> elementFound;

		for (addressMap_t::iterator element = addressRange.first; element != addressRange.second && NULL == elementFound; ++element)
		{
			if (interfaceName.compare(element->second->getInterfaceName()) == 0 &&
					name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		return elementFound;
	}

	boost::shared_ptr<Address> Context::getRouterInterfaceAddress(const std::string& routerName, const std::string interfaceName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		std::pair<addressMap_t::iterator, addressMap_t::iterator> addressRange;
		addressRange = m_addressMap.equal_range(routerName);

		boost::shared_ptr<Address> elementFound;

		for(addressMap_t::const_iterator element = addressRange.first; addressRange.second != element; ++element)
		{
			if(interfaceName.compare(element->second->getInterfaceName()) == 0 )
			{
				elementFound = element->second;
				break;
			}
		}

		return elementFound;
	}

	boost::shared_ptr<Dst> Context::getDstByName(const std::string routerName, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[dst_idx]);

		std::pair<dstMap_t::iterator, dstMap_t::iterator> dstRange;
		dstRange = m_dstMap.equal_range(routerName);

		boost::shared_ptr<Dst> elementFound;

		for (dstMap_t::iterator element = dstRange.first; element != dstRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		return elementFound;

	}

	boost::shared_ptr<NextHop>  Context::getNextHopByName(const std::string& routerName, const std::string dstName, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		std::pair<nextHopMap_t::iterator, nextHopMap_t::iterator> range;
		range = m_nextHopMap.equal_range(routerName);

		boost::shared_ptr<NextHop> elementFound;

		for (nextHopMap_t::iterator element = range.first; element != range.second && NULL == elementFound; ++element)
		{
			if (dstName.compare(element->second->getDstName()) == 0 &&
					name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		return elementFound;
	}

	boost::shared_ptr<VrrpInterface> Context::getVrrpInterfaceByName(const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpInterface_idx]);

		boost::shared_ptr<VrrpInterface> elementFound;

		for (vrrpInterfaceSet_t::iterator element = m_vrrpInterfaceSet.begin(); element != m_vrrpInterfaceSet.end() && NULL == elementFound; ++element)
		{
			if (name.compare((*element)->getName()) == 0)
			{
				elementFound = *element;
			}
		}

		return elementFound;

	}

	boost::shared_ptr<VrrpInterface> Context::getVrrpInterfaceById(const int32_t& vrId)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpInterface_idx]);

		boost::shared_ptr<VrrpInterface> elementFound;

		for (vrrpInterfaceSet_t::iterator element = m_vrrpInterfaceSet.begin(); element != m_vrrpInterfaceSet.end() && NULL == elementFound; ++element)
		{
			if (vrId == (*element)->getData().getVrIdentity())
			{
				elementFound = *element;
			}
		}

		return elementFound;

	}

	boost::shared_ptr<Address> Context::getVrrpInterfaceAddressByName(const std::string& vrrpInterfaceName, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		std::pair<addressMap_t::iterator, addressMap_t::iterator> addressRange;
		addressRange = m_addressMap.equal_range("");

		boost::shared_ptr<Address> elementFound;

		for(addressMap_t::iterator element = addressRange.first; element != addressRange.second; ++element)
		{
			if(vrrpInterfaceName.compare(element->second->getInterfaceName()) == 0U &&
					name.compare(element->second->getName()) == 0U )
			{
				elementFound = element->second;
				break;
			}
		}

		return elementFound;
	}

	boost::shared_ptr<Address> Context::getVrrpInterfaceAddress(const std::string& vrrpInterfaceName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		std::pair<addressMap_t::iterator, addressMap_t::iterator> addressRange;
		addressRange = m_addressMap.equal_range("");

		boost::shared_ptr<Address> elementFound;

		for(addressMap_t::iterator element = addressRange.first; element != addressRange.second; ++element)
		{
			if( vrrpInterfaceName.compare(element->second->getInterfaceName()) == 0U )
			{
				elementFound = element->second;
				break;
			}
		}

		return elementFound;
	}

	boost::shared_ptr<Vlan> Context::getVlanById(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& vlanId, bool loadRuntimeInfo)
	{
		UNUSED(loadRuntimeInfo);
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		boost::shared_ptr<Vlan> elementFound;

		std::pair<vlanMap_t::iterator, vlanMap_t::iterator> vlanRange = m_vlanMap.equal_range(plane);

		for (vlanMap_t::iterator element = vlanRange.first; element != vlanRange.second && NULL == elementFound; ++element)
		{
			if (element->second->getVlanId() == vlanId)
			{
				elementFound = element->second;
			}
		}

		return elementFound;

	}

	boost::shared_ptr<SubnetVlan> Context::getSubnetVlanById(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& vlanId, bool loadRuntimeInfo)
	{
		UNUSED(loadRuntimeInfo);
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[subnetVlan_idx]);

		boost::shared_ptr<SubnetVlan> elementFound;

		std::pair<subnetVlanMap_t::iterator, subnetVlanMap_t::iterator> vlanRange = m_subnetVlanMap.equal_range(plane);

		for (subnetVlanMap_t::iterator element = vlanRange.first; element != vlanRange.second && NULL == elementFound; ++element)
		{
			if (element->second->getVlanId() == vlanId)
			{
				elementFound = element->second;
			}
		}

		return elementFound;

	}

	boost::shared_ptr<BridgePort> Context::getBridgePortByIndex(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& index, bool loadRuntime)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridgePort_idx]);

		boost::shared_ptr<BridgePort> elementFound;

		std::pair<bridgePortMap_t::iterator, bridgePortMap_t::iterator> bridgePortRange;
		bridgePortRange = m_bridgePortMap.equal_range(plane);

		for (bridgePortMap_t::iterator element = bridgePortRange.first; element != bridgePortRange.second && NULL == elementFound; ++element)
		{
			if (element->second->getSnmpIndex() == index)
			{
				elementFound = element->second;
			}
		}

		if( (NULL != elementFound) && loadRuntime)
		{
			loadRuntimeInfo(elementFound);
		}

		return elementFound;
	}

	boost::shared_ptr<EthernetPort> Context::getEthernetPortByIndex(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& index, bool loadRuntime)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[ethernetPort_idx]);

		boost::shared_ptr<EthernetPort> elementFound;

		std::pair<ethernetPortMap_t::iterator, ethernetPortMap_t::iterator> ethernetPortRange;
		ethernetPortRange = m_ethernetPortMap.equal_range(plane);

		for (ethernetPortMap_t::iterator element = ethernetPortRange.first; element != ethernetPortRange.second && NULL == elementFound; ++element)
		{
			if (element->second->getSnmpIndex() == index)
			{
				elementFound = element->second;
			}
		}

		if( (NULL != elementFound) && loadRuntime)
		{
			loadRuntimeInfo(elementFound);
		}

		return elementFound;

	}

	boost::shared_ptr<Aggregator> Context::getAggregatorByIndex(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& index, bool loadRuntime)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aggregator_idx]);

		boost::shared_ptr<Aggregator> elementFound;

		std::pair<aggregatorMap_t::iterator, aggregatorMap_t::iterator> aggregatorRange;
		aggregatorRange = m_aggregatorMap.equal_range(plane);

		for (aggregatorMap_t::iterator element = aggregatorRange.first; element != aggregatorRange.second && NULL == elementFound; ++element)
		{
			if (element->second->getSnmpIndex() == index)
			{
				elementFound = element->second;
			}
		}

		if( (NULL != elementFound) && loadRuntime)
		{
			loadRuntimeInfo(elementFound);
		}

		return elementFound;

	}

	std::set< boost::shared_ptr<Vlan> > Context::getVlans(const fixs_ith::SwitchBoardPlaneConstants& plane)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		std::set<boost::shared_ptr<Vlan> > elementFound;

		std::pair<vlanMap_t::iterator, vlanMap_t::iterator> vlanRange = m_vlanMap.equal_range(plane);

		for (vlanMap_t::iterator element = vlanRange.first; element != vlanRange.second; ++element)
		{
			elementFound.insert(element->second);
		}

		return elementFound;
	}

	std::set< boost::shared_ptr<BridgePort> > Context::getBridgePorts(const fixs_ith::SwitchBoardPlaneConstants& plane)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridgePort_idx]);

		std::set<boost::shared_ptr<BridgePort> > elementFound;

		std::pair<bridgePortMap_t::iterator, bridgePortMap_t::iterator> range = m_bridgePortMap.equal_range(plane);

		for (bridgePortMap_t::iterator element = range.first; element != range.second; ++element)
		{
			elementFound.insert(element->second);
		}

		return elementFound;
	}

	std::set< boost::shared_ptr<Aggregator> > Context::getAggregators(const fixs_ith::SwitchBoardPlaneConstants& plane)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aggregator_idx]);

		std::set<boost::shared_ptr<Aggregator> > elementFound;

		std::pair<aggregatorMap_t::iterator, aggregatorMap_t::iterator> range = m_aggregatorMap.equal_range(plane);

		for (aggregatorMap_t::iterator element = range.first; element != range.second; ++element)
		{
			elementFound.insert(element->second);
		}

		return elementFound;

	}

	std::set< boost::shared_ptr<InterfaceIPv4> > Context::getInterfaces()
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[interface_idx]);

		std::set<boost::shared_ptr<InterfaceIPv4> > elementFound;

		for (interfaceMap_t::iterator element = m_interfaceMap.begin(); element != m_interfaceMap.end(); ++element)
		{
			elementFound.insert(element->second);
		}

		return elementFound;

	}

	std::set< boost::shared_ptr<InterfaceIPv4> > Context::getInterfaces(const std::string& routerName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[interface_idx]);

		std::set<boost::shared_ptr<InterfaceIPv4> > elementFound;

		std::pair<interfaceMap_t::iterator, interfaceMap_t::iterator> range = m_interfaceMap.equal_range(routerName);

		for (interfaceMap_t::iterator element = range.first; element != range.second; ++element)
		{
			elementFound.insert(element->second);
		}

		return elementFound;
	}

	std::set< boost::shared_ptr<Address> > Context::getRouterInterfaceAddresses(const std::string& routerName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		std::set<boost::shared_ptr<Address> > elementFound;

		std::pair<addressMap_t::iterator, addressMap_t::iterator> range = m_addressMap.equal_range(routerName);

		for (addressMap_t::iterator element = range.first; element != range.second; ++element)
		{
			elementFound.insert(element->second);
		}

		return elementFound;
	}

	std::set< boost::shared_ptr<Dst> > Context::getDsts(const std::string& routerName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[dst_idx]);

		std::set<boost::shared_ptr<Dst> > elementFound;

		std::pair<dstMap_t::iterator, dstMap_t::iterator> range = m_dstMap.equal_range(routerName);

		for (dstMap_t::iterator element = range.first; element != range.second; ++element)
		{
			elementFound.insert(element->second);
		}

		return elementFound;
	}

	std::set<std::string> Context::getNextHopsAddressWithBFDEnabled(const std::string& routerName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		std::set<std::string> elementsFound;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Get NextHop Address on Router:<%s> ", m_smxId.c_str(), routerName.c_str());

		std::pair<nextHopMap_t::const_iterator, nextHopMap_t::const_iterator> range = m_nextHopMap.equal_range(routerName);

		for(nextHopMap_t::const_iterator element = range.first; element != range.second; ++element)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Found NextHop Address:<%s>, IP:<%s> BFD is <%s>",
					m_smxId.c_str(), element->second->getName(), element->second->getAddress(), common::utility::boolToString(element->second->isBFDEnabled()) );

			if(element->second->hasAddress() && element->second->isBFDEnabled() )
			{
				elementsFound.insert(element->second->getAddress());
			}
		}

		// remove already defined address
		std::set<std::string> addressOfBfdSessions = getAddressOfBfdSessions(routerName);
		std::set<std::string>::iterator element = addressOfBfdSessions.begin();

		for(; addressOfBfdSessions.end() != element; ++element )
		{
			if( elementsFound.find(*element) != elementsFound.end() )
			{
				elementsFound.erase(*element);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] BFD session for address:<%s> on Router:<%s> already created",
						 m_smxId.c_str(), routerName.c_str(), element->c_str() );
			}
		}

		return elementsFound;
	}

	std::set<std::string> Context::getNextHopsAddress(const std::string& routerName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		std::set<std::string> elementsFound;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Get NextHop Address on Router:<%s> ", m_smxId.c_str(), routerName.c_str());

		std::pair<nextHopMap_t::const_iterator, nextHopMap_t::const_iterator> range = m_nextHopMap.equal_range(routerName);

		for(nextHopMap_t::const_iterator element = range.first; element != range.second; ++element)
		{
			if(element->second->hasAddress() )
			{
				elementsFound.insert(element->second->getAddress());
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Found <%zu> NextHops with address",
					m_smxId.c_str(), elementsFound.size());

		return elementsFound;
	}


	std::set< uint16_t > Context::getBridgePortsIndexes(const fixs_ith::SwitchBoardPlaneConstants& plane)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridgePort_idx]);

		std::set<uint16_t > elementFound;

		std::pair<bridgePortMap_t::iterator, bridgePortMap_t::iterator> range = m_bridgePortMap.equal_range(plane);

		for (bridgePortMap_t::iterator element = range.first; element != range.second; ++element)
		{
			elementFound.insert(element->second->getSnmpIndex());
		}

		return elementFound;
	}

	std::set< uint16_t > Context::getEthernetPortsIndexes(const fixs_ith::SwitchBoardPlaneConstants& plane)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[ethernetPort_idx]);

		std::set<uint16_t > elementFound;

		std::pair<ethernetPortMap_t::iterator, ethernetPortMap_t::iterator> range = m_ethernetPortMap.equal_range(plane);

		for (ethernetPortMap_t::iterator element = range.first; element != range.second; ++element)
		{
			elementFound.insert(element->second->getSnmpIndex());
		}

		return elementFound;
	}

	std::set< boost::shared_ptr<VrrpSession> > Context::getVrrpSessions(const std::string& vrrpInterfaceName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);
		std::set<boost::shared_ptr<VrrpSession> > elementsFound;

		vrrpSessions_t::const_iterator element;

		for(element = m_vrrpSessions.begin(); m_vrrpSessions.end() != element; ++element)
		{
			if(vrrpInterfaceName.compare(element->second->getVrrpInterfaceName()) == 0 )
			{
				// add the vrrp session with same vrrp interface
				elementsFound.insert(element->second);
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Found <%d> VRRP Session with same VRRP Interface:<%s>",
				m_smxId.c_str(), elementsFound.size(), vrrpInterfaceName.c_str() );

		return elementsFound;
	}

	fixs_ith::ErrorConstants Context::getVrrpSessionInfo(const std::string& vrrpSessionDN, operation::vrrpSessionInfo& data)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);

		boost::shared_ptr<VrrpSession> vrrpSession = getVrrpSessionByDN(vrrpSessionDN);

		if(NULL == vrrpSession)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] VRRP session not found DN:<%s>", m_smxId.c_str(), vrrpSessionDN.c_str());
			return fixs_ith::ERR_CONFIG_INVALID_VRRPSESSION;
		}

		data = vrrpSession->getData();

		fixs_ith_switchInterface::op_result op_result = m_switchInterface.get_vrrpv3Session_info(data);

		if (fixs_ith_switchInterface::SBIA_OK != op_result &&
				fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to get VrrpSession:<%s> INFO on Interface:<%s> of Router:<%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), vrrpSession->getName(), vrrpSession->getInterfaceName(), vrrpSession->getRouterName(), op_result);
			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}

		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] VRRP session:<%s> vrrpState:<%d>",
				m_smxId.c_str(), vrrpSession->getName(), data.getVrrpState());

		return result;
	}

	boost::shared_ptr<BfdProfile> Context::getBfdProfileByName(const std::string& routerName, const std::string& name)
	{

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdProfile_idx]);

		std::pair<bfdProfileMap_t::iterator, bfdProfileMap_t::iterator> bfdProfileRange;
		bfdProfileRange = m_bfdProfileMap.equal_range(routerName);

		boost::shared_ptr<BfdProfile> elementFound;

		for (bfdProfileMap_t::iterator element = bfdProfileRange.first; element != bfdProfileRange.second && NULL == elementFound; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
			}
		}

		return elementFound;
	}

	boost::shared_ptr<BfdSessionIPv4> Context::getBfdSessionIPv4ByName(const std::string& routerName, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdSession_idx]);

		std::pair<bfdSessions_t::const_iterator, bfdSessions_t::const_iterator> bfdSessionsRange;
		bfdSessionsRange = m_bfdSessions.equal_range(routerName);

		boost::shared_ptr<BfdSessionIPv4> elementFound;

		for(bfdSessions_t::const_iterator element = bfdSessionsRange.first; bfdSessionsRange.second != element; ++element)
		{
			if( name.compare(element->second->getName()) == 0)
			{
				// BFD session found
				elementFound = element->second;
				break;
			}
		}

		return elementFound;
	}

	fixs_ith::ErrorConstants Context::getBfdSessionIPv4Info(const std::string& bfdSessionDN, operation::bfdSessionInfo& data)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdSession_idx]);

		std::string routerName(common::utility::getRouterNameFromBfdSessionIPv4DN( bfdSessionDN));
		std::string bfdSessionName(common::utility::getIdValueFromRdn(bfdSessionDN));

		boost::shared_ptr<BfdSessionIPv4> bfdSession = getBfdSessionIPv4ByName(routerName, bfdSessionName);

		if(NULL == bfdSession)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] BFD Session not found DN:<%s>", m_smxId.c_str(), bfdSessionDN.c_str());
			return fixs_ith::ERR_CONFIG_INVALID_BFDSESSION;
		}

		data = bfdSession->getData();

		if( !bfdSession->isOutOfDate() ) return result;

		fixs_ith_switchInterface::op_result op_result = m_switchInterface.get_BfdSessionIPv4_info(data);

		if(fixs_ith_switchInterface::SBIA_OK == op_result )
		{
			// replace interfaceIPv4 reference with its IP address
			std::string interfaceName = common::utility::getRDNValue(data.getAddrSrc(), netconf::interfaceIpv4::CLASS_NAME);
			boost::shared_ptr<Address> interfaceAddress = getRouterInterfaceAddress(routerName, interfaceName);

			if(interfaceAddress)
				data.addrSrc.assign(interfaceAddress->getAddress());

			// refresh bfd session info
			bfdSession->refreshAttribute(data);
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to get BFDSession:<%s> INFO of Router:<%s>. Switch Interface Error <%d>",
					m_smxId.c_str(), bfdSession->getName(), bfdSession->getRouterName(), op_result);

			result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
		}

		return result;
	}

	std::set<std::string> Context::getAddressOfBfdSessions(const std::string& routerName)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdSession_idx]);

		std::set<std::string> addressOfBfdSessions;
		std::pair<bfdSessions_t::const_iterator, bfdSessions_t::const_iterator> bfdSessionsRange;

		bfdSessionsRange = m_bfdSessions.equal_range(routerName);

		for(bfdSessions_t::const_iterator element = bfdSessionsRange.first; bfdSessionsRange.second != element; ++element)
		{
			addressOfBfdSessions.insert(element->second->getAddress());
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Found <%zu> BFD sessions on Router:<%s>", m_smxId.c_str(), addressOfBfdSessions.size(), routerName.c_str() );

		return addressOfBfdSessions;
	}

	std::set<std::string> Context::getAddressOfInterfaceWithBFD(const std::string& routerName)
	{
		std::set<std::string> addressOfInterfaceWithBFD;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[interface_idx]);
		std::pair<interfaceMap_t::const_iterator, interfaceMap_t::const_iterator> interfaceRange;

		interfaceRange = m_interfaceMap.equal_range(routerName);

		for(interfaceMap_t::const_iterator element = interfaceRange.first; interfaceRange.second != element; ++element)
		{
			// get only interface with BFD enabled
			if(element->second->isBfdEnabled())
			{
				std::string interfaceName(element->second->getName());
				boost::shared_ptr<Address> interfaceAddress = getRouterInterfaceAddress(routerName, interfaceName);

				if(interfaceAddress)
				{
					addressOfInterfaceWithBFD.insert(interfaceAddress->getAddress());
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Interface:<%s> Address:<%s> with BFD enabled",
							m_smxId.c_str(), interfaceName.c_str(), interfaceAddress->getAddress() );
				}
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Found <%zu> Interface with BFD enabled on Router:<%s>", m_smxId.c_str(), addressOfInterfaceWithBFD.size(), routerName.c_str() );

		return addressOfInterfaceWithBFD;
	}

	std::string Context::getNextHopAddress(const std::string& routerName, const std::string& dstName, const std::string& nextHopName)
	{
		std::string address;
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		std::pair<nextHopMap_t::const_iterator, nextHopMap_t::const_iterator> nextHopRange;
		nextHopRange = m_nextHopMap.equal_range(routerName);

		for(nextHopMap_t::const_iterator element = nextHopRange.first; nextHopRange.second != element; ++element)
		{
			if( ( dstName.compare(element->second->getDstName()) == 0U )
					&& ( nextHopName.compare(element->second->getName()) == 0U ) )
			{
				address.assign(element->second->getAddress());
				break;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] NextHop<%s> address:<%s> on Dst:<%s>, Router:<%s>",
				m_smxId.c_str(), nextHopName.c_str(), address.c_str(), dstName.c_str(), routerName.c_str() );

		return address;
	}

	boost::shared_ptr<AclEntryIpv4> Context::getAclEntryIpv4ByName(const std::string& routerName, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclEntry_idx]);

		std::pair<aclEntries_t::iterator, aclEntries_t::iterator> aclEntriesRange;
		aclEntriesRange = m_aclEntries.equal_range(routerName);

		boost::shared_ptr<AclEntryIpv4> elementFound;

		for(aclEntries_t::iterator element = aclEntriesRange.first; element != aclEntriesRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
				break;
			}
		}

		return elementFound;
	}

	boost::shared_ptr<AclIpv4> Context::getAclIpV4ByName(const std::string& routerName, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclIpv4_idx]);

		std::pair<aclIpV4_t::const_iterator, aclIpV4_t::const_iterator> aclRange;
		aclRange = m_aclIpV4.equal_range(routerName);

		boost::shared_ptr<AclIpv4> elementFound;

		for(aclIpV4_t::const_iterator element = aclRange.first; element != aclRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				elementFound = element->second;
				break;
			}
		}

		return elementFound;
	}

	fixs_ith::ErrorConstants Context::resetBridgeByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridge_idx]);

		std::pair<bridgeMap_t::iterator, bridgeMap_t::iterator> bridgeRange;
		bridgeRange = m_bridgeMap.equal_range(plane);

		for (bridgeMap_t::iterator element = bridgeRange.first; element != bridgeRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				m_bridgeMap.erase(element);
				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::resetBridgePortByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridgePort_idx]);

		std::pair<bridgePortMap_t::iterator, bridgePortMap_t::iterator> bridgePortRange;
		bridgePortRange = m_bridgePortMap.equal_range(plane);

		for (bridgePortMap_t::iterator element = bridgePortRange.first; element != bridgePortRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				m_bridgePortMap.erase(element);
				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::resetEthernetPortByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[ethernetPort_idx]);

		std::pair<ethernetPortMap_t::iterator, ethernetPortMap_t::iterator> ethernetPortRange;
		ethernetPortRange = m_ethernetPortMap.equal_range(plane);

		for (ethernetPortMap_t::iterator element = ethernetPortRange.first; element != ethernetPortRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				m_ethernetPortMap.erase(element);
				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::resetAggregatorByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aggregator_idx]);

		std::pair<aggregatorMap_t::iterator, aggregatorMap_t::iterator> aggregatorRange;
		aggregatorRange = m_aggregatorMap.equal_range(plane);

		for (aggregatorMap_t::iterator element = aggregatorRange.first; element != aggregatorRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

				op_result =	m_switchInterface.delete_aggregator(m_smxId, element->second->getPlane(), element->second->getSnmpIndex());

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to delete Aggregator <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), common::utility::planeToString(element->second->getPlane()).c_str(), element->second->getName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					m_aggregatorMap.erase(element);
				}

				break;
			}
		}

		return result;
	}


	fixs_ith::ErrorConstants Context::resetVlanByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		std::pair<vlanMap_t::iterator, vlanMap_t::iterator> vlanRange;
		vlanRange = m_vlanMap.equal_range(plane);

		for (vlanMap_t::iterator element = vlanRange.first; element != vlanRange.second ; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				if (element->second->getVlanId() == imm::vlanId_range::DEFAULT)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Cannot delete default Vlan <%s>.",
							m_smxId.c_str(), common::utility::planeToString(element->second->getPlane()).c_str(), element->second->getName());
					result = fixs_ith::ERR_CONFIG_PROTECTED_OBJECT;
				}
				else if (element->second->getVlanId() != imm::vlanId_range::UNDEFINED)
				{
					fixs_ith_switchInterface::op_result op_result =	m_switchInterface.delete_vlan(m_smxId, element->second->getPlane(), element->second->getVlanId());

					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to delete Vlan <%s>. Switch Interface Error <%d>",
								m_smxId.c_str(), common::utility::planeToString(element->second->getPlane()).c_str(), element->second->getName(), op_result);
						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
				}

				if (fixs_ith::ERR_NO_ERRORS == result)
				{
					m_vlanMap.erase(element);
				}

				break;
			}
		}

		return result;

	}

	fixs_ith::ErrorConstants Context::resetSubnetVlanByName(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[subnetVlan_idx]);

		std::pair<subnetVlanMap_t::iterator, subnetVlanMap_t::iterator> subnetVlanRange;
		subnetVlanRange = m_subnetVlanMap.equal_range(plane);

		for (subnetVlanMap_t::iterator element = subnetVlanRange.first; element != subnetVlanRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				fixs_ith_switchInterface::op_result op_result =	m_switchInterface.delete_ipv4subnet(m_smxId, element->second->getPlane(), element->second->getIpSubnet());

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to delete SubnetVlan <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), common::utility::planeToString(element->second->getPlane()).c_str(), element->second->getIpSubnet(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					m_subnetVlanMap.erase(element);
				}

				break;
			}
		}

		return result;

	}

	fixs_ith::ErrorConstants  Context::resetRouterByName(const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[router_idx]);

		for (routerSet_t::iterator element = m_routerSet.begin(); element != m_routerSet.end(); ++element)
		{
			if (name.compare((*element)->getName()) == 0)
			{
				operation::routerInfo info((*element)->getData());
				fixs_ith_switchInterface::op_result op_result =	m_switchInterface.delete_router(info);

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Failed to delete Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), name.c_str(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					m_routerSet.erase(element);
				}

				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::resetInterfaceByName(const std::string& routerName, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[interface_idx]);

		std::pair<interfaceMap_t::iterator, interfaceMap_t::iterator> interfaceRange;
		interfaceRange = m_interfaceMap.equal_range(routerName);

		for (interfaceMap_t::iterator element = interfaceRange.first; element != interfaceRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{
				//TODO: Loopback interface is automatically deleted
				fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;
				if (!element->second->isLoopback())
				{
					op_result =	m_switchInterface.delete_interfaceIPv4(element->second->getData());
				}

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Failed to delete Interface <%s> on Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), name.c_str(), element->second->getRouterName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					m_interfaceMap.erase(element);
				}

				break;
			}
		}

		return result;

	}

	fixs_ith::ErrorConstants Context::resetVrrpSessionByName(const std::string& routerName, const std::string interfaceName, const std::string& vrrpSessionName)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);

		std::pair<vrrpSessions_t::iterator, vrrpSessions_t::iterator> vrrpSessionRange;
		vrrpSessionRange = m_vrrpSessions.equal_range(routerName);

		for(vrrpSessions_t::iterator element = vrrpSessionRange.first; vrrpSessionRange.second != element; ++element)
		{
			if( (interfaceName.compare(element->second->getInterfaceName()) == 0) &&
					(vrrpSessionName.compare(element->second->getName()) == 0) )
			{
				// Vrrp session found
				// check state admin
				if( imm::LOCKED != element->second->getData().getAdminState() )
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Failed to delete VrrpSession:<%s> on Interface:<%s> of Router:<%s>. ADMIN STATE must be LOCKED",
							m_smxId.c_str(), vrrpSessionName.c_str(), interfaceName.c_str(), routerName.c_str());

					result = fixs_ith::ERR_CONFIG_INVALID_VRRPSESSION_STATE;
					break;
				}

				fixs_ith_switchInterface::op_result op_result = m_switchInterface.delete_vrrpv3Session(element->second->getData());

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Failed to delete VrrpSession:<%s> on Interface:<%s> of Router:<%s>. Switch Interface Error:<%d>",
							m_smxId.c_str(), vrrpSessionName.c_str(), interfaceName.c_str(), routerName.c_str(), op_result);

					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					m_vrrpSessions.erase(element);

					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] VrrpSession:<%s> on Interface:<%s> of Router:<%s> removed.",
							m_smxId.c_str(), vrrpSessionName.c_str(), interfaceName.c_str(), routerName.c_str());
				}

				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::resetVrrpSessionByDN(const std::string& vrrpSessionDN)
	{
		std::string routerName( common::utility::getRouterNameFromVrrpSessionDN( vrrpSessionDN ) );
		std::string interfaceName( common::utility::getInterfaceNameFromVrrpSessionDN( vrrpSessionDN ) );
		std::string vrrpSessionName(common::utility::getVrrpSessionNameFromDN(vrrpSessionDN));
		return resetVrrpSessionByName(routerName, interfaceName, vrrpSessionName);
	}

	fixs_ith::ErrorConstants Context::resetDstByName(const std::string& routerName, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[dst_idx]);

		std::pair<dstMap_t::iterator, dstMap_t::iterator> dstRange;
		dstRange = m_dstMap.equal_range(routerName);

		for (dstMap_t::iterator element = dstRange.first; element != dstRange.second; ++element)
		{
			if (name.compare(element->second->getName()) == 0)
			{

				fixs_ith_switchInterface::op_result op_result =	m_switchInterface.delete_Dst(element->second->getData());

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Failed to delete Dst <%s> on Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), name.c_str(), element->second->getRouterName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					m_dstMap.erase(element);
				}

				break;
			}
		}

		return result;

	}

	fixs_ith::ErrorConstants  Context::resetRouterInterfaceAddressByName(const std::string& routerName, const std::string interfaceName, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		std::pair<addressMap_t::iterator, addressMap_t::iterator> addressRange;
		addressRange = m_addressMap.equal_range(routerName);

		for (addressMap_t::iterator element = addressRange.first; element != addressRange.second; ++element)
		{
			if (interfaceName.compare(element->second->getInterfaceName()) == 0 &&
					name.compare(element->second->getName()) == 0)
			{
				fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

				boost::shared_ptr<InterfaceIPv4> interface = getInterfaceByName(element->second->getRouterName(), element->second->getInterfaceName());

				//TODO: Interface loopback is automatically deleted..
				if (NULL != interface && !interface->isLoopback())
				{
					op_result =	m_switchInterface.delete_addressIPv4(element->second->getData());
				}

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Failed to delete Address <%s> on Interface <%s> of Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), name.c_str(), element->second->getInterfaceName(), element->second->getRouterName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					m_addressMap.erase(element);
				}

				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants  Context::resetNextHopByName(const std::string& routerName, const std::string dstName, const std::string& name)
	{

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		std::pair<nextHopMap_t::iterator, nextHopMap_t::iterator> range;
		range = m_nextHopMap.equal_range(routerName);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Entering in resetNextHopByName: [<%s> <%s> <%s>] ", routerName.c_str(), dstName.c_str(),  name.c_str());
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " There are m_nextHopMap<%d> element with key <%s> in 'm_nextHopMap'" , m_nextHopMap.count(routerName), routerName.c_str());


		nextHopMap_t::iterator elementToErase;
		bool elementFound = false;

		for (nextHopMap_t::iterator element = range.first; element != range.second && elementFound == false; ++element)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Found NextHop <%s> of Router <%s>", element->second->getName(), element->second->getRouterName());
			if (dstName.compare(element->second->getDstName()) == 0 &&
					name.compare(element->second->getName()) == 0)
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Calling switchInetface.delete_NextHopInfo to delete NextHop <%s> of Router <%s> [DN == <%s>]", element->second->getName(), element->second->getRouterName(), element->second->getData().getDN());
				fixs_ith_switchInterface::op_result op_result =	m_switchInterface.delete_NextHopInfo(element->second->getData());

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s]Failed to delete NextHop <%s> on Dst <%s> of Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), name.c_str(), element->second->getDstName(), element->second->getRouterName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					elementFound = true;
					//m_nextHopMap.erase(element);
					elementToErase = element;
				}
			}
		}

		if(elementFound) {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ERASING from m_nextHopMap NextHop <%s> of Router <%s>", elementToErase->second->getName(), elementToErase->second->getRouterName());
			m_nextHopMap.erase(elementToErase);
		}

		return result;
	}

	fixs_ith::ErrorConstants  Context::resetVrrpInterfaceByName(const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpInterface_idx]);

		for (vrrpInterfaceSet_t::iterator element = m_vrrpInterfaceSet.begin(); element != m_vrrpInterfaceSet.end(); ++element)
		{
			if (name.compare((*element)->getName()) == 0)
			{
				if((*element)->getData().getReservedBy().empty())
					m_vrrpInterfaceSet.erase(element);
				else
					result = fixs_ith::ERR_CONFIG_REFERENCED_OBJECT;

				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants  Context::resetVrrpInterfaceAddressByName(const std::string interfaceName, const std::string& name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		std::pair<addressMap_t::iterator, addressMap_t::iterator> addressRange;
		addressRange = m_addressMap.equal_range("");

		for (addressMap_t::iterator element = addressRange.first; element != addressRange.second; ++element)
		{
			if (interfaceName.compare(element->second->getInterfaceName()) == 0 &&
					name.compare(element->second->getName()) == 0)
			{
				m_addressMap.erase(element);
				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants  Context::resetBfdSessionIPv4ByName(const std::string& routerName, const std::string& name)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdSession_idx]);

		std::pair<bfdSessions_t::iterator, bfdSessions_t::iterator> range;
		range = m_bfdSessions.equal_range(routerName);

		// for each bfd session on this router
		for(bfdSessions_t::iterator element = range.first; range.second != element; ++element)
		{
			if( name.compare(element->second->getName()) == 0 )
			{
				// Bfd session found remove it
				m_bfdSessions.erase(element);

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] BFD Session:<%s> of Router:<%s> removed.",
						m_smxId.c_str(), name.c_str(),  routerName.c_str());

				break;
			}
		}

		return fixs_ith::ERR_NO_ERRORS;
	}

	fixs_ith::ErrorConstants Context::resetBfdProfileByDN(const std::string& bfdProfileDN)
	{
		std::string routerName( common::utility::getRouterNameFromInterfaceDN( bfdProfileDN ) );
		std::string bfdProfileName(common::utility::getBfdProfileNameFromDN(bfdProfileDN));
		return resetBfdProfileByName(routerName, bfdProfileName);
	}

	fixs_ith::ErrorConstants Context::resetBfdProfileByName(const std::string& routerName, const std::string& bfdProfileName)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdProfile_idx]);

		std::pair<bfdProfileMap_t::iterator, bfdProfileMap_t::iterator> bfdProfileRange;
		bfdProfileRange = m_bfdProfileMap.equal_range(routerName);

		for(bfdProfileMap_t::iterator element = bfdProfileRange.first; bfdProfileRange.second != element; ++element)
		{
			if(	(bfdProfileName.compare(element->second->getName()) == 0) )
			{
				// BfdProfile found

				if (element->second->getData().getReservedBy().empty())
				{
					fixs_ith_switchInterface::op_result op_result = m_switchInterface.delete_BfdProfile(element->second->getData());

					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to delete BfdProfile:<%s> of Router:<%s>. Switch Interface Error:<%d>",
								m_smxId.c_str(), bfdProfileName.c_str(), routerName.c_str(), op_result);

						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
					else
					{
						m_bfdProfileMap.erase(element);

						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] VrrpSBfdProfileession:<%s> of Router:<%s> removed.",
								m_smxId.c_str(), bfdProfileName.c_str(), routerName.c_str());
					}
				}
				else
				{
					result = fixs_ith::ERR_CONFIG_REFERENCED_OBJECT;
				}

				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::resetAclEntryByName(const std::string& routerName, const std::string& aclEntryName)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclEntry_idx]);

		std::pair<aclEntries_t::iterator, aclEntries_t::iterator> aclEntriesRange;
		aclEntriesRange = m_aclEntries.equal_range(routerName);

		// for each acl entry on this router
		for(aclEntries_t::iterator element = aclEntriesRange.first; aclEntriesRange.second != element; ++element)
		{
			if( aclEntryName.compare(element->second->getName()) == 0 )
			{
				// AclEntry found
				fixs_ith_switchInterface::op_result op_result = fixs_ith_switchInterface::SBIA_OK;

				// AclEntryIpv4 is already removed SMX side if we are here triggered by AclIpv4 delete
				if( NULL != getAclIpV4ByName(routerName, element->second->getAclIpv4Name()) )
				{
					op_result = m_switchInterface.delete_AclEntry(element->second->getData());
				}

				if( fixs_ith_switchInterface::SBIA_OK != op_result )
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to delete AclEntryIpv4:<%s> of Router:<%s>. Switch Interface Error:<%d>",
							m_smxId.c_str(), aclEntryName.c_str(), routerName.c_str(), op_result);

					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					m_aclEntries.erase(element);
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] AclEntryIpv4:<%s> of Router:<%s> removed.",
							m_smxId.c_str(), aclEntryName.c_str(), routerName.c_str());
				}
				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::resetAclIpV4ByName(const std::string& routerName, const std::string& aclIpV4Name)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclIpv4_idx]);

		std::pair<aclIpV4_t::iterator, aclIpV4_t::iterator> aclIpv4Range;
		aclIpv4Range = m_aclIpV4.equal_range(routerName);

		// for each acl ipv4 on this router
		for(aclIpV4_t::iterator element = aclIpv4Range.first; aclIpv4Range.second != element; ++element)
		{
			if( aclIpV4Name.compare(element->second->getName()) == 0 )
			{
				// AclIpv4 found remove it
				if (element->second->getData().getReservedBy().empty())
				{
					fixs_ith_switchInterface::op_result op_result = m_switchInterface.delete_AclIpv4(element->second->getData());

					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to delete AclIpv4:<%s> of Router:<%s>. Switch Interface Error:<%d>",
								m_smxId.c_str(), aclIpV4Name.c_str(), routerName.c_str(), op_result);

						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
					else
					{
						m_aclIpV4.erase(element);
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] AclIpV4:<%s> of Router:<%s> removed.",
								m_smxId.c_str(), aclIpV4Name.c_str(), routerName.c_str());
					}
				}
				else
				{
					result = fixs_ith::ERR_CONFIG_REFERENCED_OBJECT;
				}
				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceBridge(boost::shared_ptr<Bridge>& modifiedBridge)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridge_idx]);

		boost::shared_ptr<Bridge> oldBridge;
		bridgeMap_t::iterator elementFound;

		std::pair<bridgeMap_t::iterator, bridgeMap_t::iterator> bridgeRange;
		bridgeRange = m_bridgeMap.equal_range(modifiedBridge->getPlane());

		for (bridgeMap_t::iterator element = bridgeRange.first; element != bridgeRange.second && NULL == oldBridge; ++element)
		{
			if (strcmp(modifiedBridge->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldBridge = element->second;
			}
		}

		if (NULL != oldBridge)
		{
			m_bridgeMap.erase(elementFound);
			m_bridgeMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Bridge> >(modifiedBridge->getPlane(), modifiedBridge));
		}
		else
		{
			//Object was missing in context... Create it
			result = setBridge(modifiedBridge);
		}

		return result;

	}

	fixs_ith::ErrorConstants Context::replaceBridgePort(boost::shared_ptr<BridgePort>& modifiedBridgePort)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bridgePort_idx]);

//		if(modifiedBridgePort->getChangeMask() & imm::bridgePort_attribute::ADM_STATE_CHANGE){
//			imm::admin_State modifiedAdminState= modifiedBridgePort->getAdminState();
//			uint16_t snmpIndex = modifiedBridgePort->getSnmpIndex();
//			fixs_ith::switchboard_plane_t plane = modifiedBridgePort->getPlane();
//
//			if (modifiedAdminState == imm::UNLOCKED &&
//					((snmpIndex == TRANSPORT_PLANE_CONTROL2_PORT_INDEX && plane == fixs_ith::TRANSPORT_PLANE )||
//							(snmpIndex == CONTROL_PLANE_TRANSPORT2_PORT_INDEX  && plane == fixs_ith::CONTROL_PLANE)))
//			{
//				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] IS NOT ALLOWED TO SET ADMIN STATE to <%s> for BridgePort <%s>",
//						modifiedBridgePort->getSmxId(), common::utility::planeToString(plane).c_str(), "UNLOCKED", modifiedBridgePort->getName());
//				return fixs_ith::ERR_CONFIG_INVALID_BRIDGEPORT_STATE;
//			}
//		}

		boost::shared_ptr<BridgePort> oldBridgePort;
		bridgePortMap_t::iterator elementFound;

		std::pair<bridgePortMap_t::iterator, bridgePortMap_t::iterator> bridgePortRange;
		bridgePortRange = m_bridgePortMap.equal_range(modifiedBridgePort->getPlane());

		for (bridgePortMap_t::iterator element = bridgePortRange.first; element != bridgePortRange.second && NULL == oldBridgePort; ++element)
		{
			if (strcmp(modifiedBridgePort->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldBridgePort = element->second;
			}
		}

		if (NULL != oldBridgePort)
		{
			if((modifiedBridgePort->getChangeMask() & imm::bridgePort_attribute::ADM_STATE_CHANGE) /*&&
					(oldBridgePort->getAdminState() != modifiedBridgePort->getAdminState())*/)
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting admin_state for EARLY CONFIGURED port %d on %s plane", modifiedBridgePort->getSnmpIndex(), common::utility::planeToString(modifiedBridgePort->getPlane()).c_str());
				fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_if_adminState(m_smxId, modifiedBridgePort->getPlane(), modifiedBridgePort->getSnmpIndex(), (fixs_ith_switchInterface::BP_admin_state) modifiedBridgePort->getAdminState());

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set admin state on BridgePort <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), common::utility::planeToString(modifiedBridgePort->getPlane()).c_str(), modifiedBridgePort->getName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
			}
			//TODO: ELIMINARE
			else FIXS_ITH_LOG(LOG_LEVEL_DEBUG, ">>>> SKIPPED setting admin_state for port %d on %s plane", modifiedBridgePort->getSnmpIndex(), common::utility::planeToString(modifiedBridgePort->getPlane()).c_str());

			if (fixs_ith::ERR_NO_ERRORS == result &&
					(modifiedBridgePort->getChangeMask() & imm::bridgePort_attribute::DEFAULT_VLAN_ID_CHANGE) &&
					(oldBridgePort->getDefaultVlanId() != modifiedBridgePort->getDefaultVlanId()) )
			{
				//Check if the bridgePort is part of an aggregator
				bool isAggMemebr = isAggregatorMember(modifiedBridgePort->getPlane(), modifiedBridgePort->getSnmpIndex());

				if (!isAggMemebr)
				{
					fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_port_default_vlan(m_smxId, modifiedBridgePort->getPlane(), modifiedBridgePort->getSnmpIndex(), modifiedBridgePort->getDefaultVlanId());

					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set default VLAN ID on BridgePort <%s>. Switch Interface Error <%d>",
								m_smxId.c_str(), common::utility::planeToString(modifiedBridgePort->getPlane()).c_str(), modifiedBridgePort->getName(), op_result);
						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
				}
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_bridgePortMap.erase(elementFound);
				m_bridgePortMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<BridgePort> >(modifiedBridgePort->getPlane(), modifiedBridgePort));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setBridgePort(modifiedBridgePort);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceEthernetPort(boost::shared_ptr<EthernetPort>& modifiedEthernetPort)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[ethernetPort_idx]);

		boost::shared_ptr<EthernetPort> oldEthernetPort;
		ethernetPortMap_t::iterator elementFound;

		std::pair<ethernetPortMap_t::iterator, ethernetPortMap_t::iterator> range;
		range = m_ethernetPortMap.equal_range(modifiedEthernetPort->getPlane());

		for (ethernetPortMap_t::iterator element = range.first; element != range.second && NULL == oldEthernetPort; ++element)
		{
			if (strcmp(modifiedEthernetPort->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldEthernetPort = element->second;
			}
		}

		if (NULL != oldEthernetPort)
		{
			if (oldEthernetPort->getMaxFrameSize() != modifiedEthernetPort->getMaxFrameSize())
			{
				fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_eth_max_frame_size(m_smxId, modifiedEthernetPort->getPlane(), modifiedEthernetPort->getSnmpIndex(), modifiedEthernetPort->getMaxFrameSize());

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set max frame size on EthernetPort <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(), modifiedEthernetPort->getName(), op_result);
					return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
			}

			if(oldEthernetPort->getAutoNegotiate() != modifiedEthernetPort->getAutoNegotiate())
			{
				std::vector<bool> if_mau_type;
				uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;

			// Autonegotiation modification should be rejected in case of below ports on CMX
			// E5-E8 , GE1-GE4 and BP1-BP24
			if (common::utility::isSwitchBoardCMX())
			{
				int modifiedPortIndex = modifiedEthernetPort->getSnmpIndex();
				if (modifiedPortIndex == 131 || modifiedPortIndex == 136 || modifiedPortIndex == 141 || modifiedPortIndex == 146)
				{
					// Fetch the autonegotiation state change
					// If autoNegotiate state is changed from UNLOCKED TO LOCKED
					if (modifiedEthernetPort->getAutoNegotiate() == imm::AN_LOCKED)
					{
						// Set the default type of port to 10GigBaseCX4
						fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_eth_if_mau_type(m_smxId,modifiedEthernetPort->getPlane(),modifiedEthernetPort->getSnmpIndex(),fixs_ith_snmp::IF_MAU_TYPE_10G_BASE_CX4);

						if (fixs_ith_switchInterface::SBIA_OK != op_result)
						{
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to set if mau type for EthernetPort <%s>. Switch Interface Error <%d>",m_smxId.c_str(),common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(),modifiedEthernetPort->getName(), op_result);
							return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
						}

						// Disable the auto negotiation status
						op_result = m_switchInterface.set_eth_auto_negotiation(m_smxId, modifiedEthernetPort->getPlane(),modifiedEthernetPort->getSnmpIndex(), false);

						if (fixs_ith_switchInterface::SBIA_OK != op_result)
						{
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to disable auto negotiation on EthernetPort <%s>. Switch Interface Error <%d>",m_smxId.c_str(),common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(),modifiedEthernetPort->getName(), op_result);
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Restoring the if mau type for EthernetPort <%s> to 40G",m_smxId.c_str());

							op_result = m_switchInterface.set_eth_if_mau_type(m_smxId,modifiedEthernetPort->getPlane(),modifiedEthernetPort->getSnmpIndex(),fixs_ith_snmp::IF_MAU_TYPE_40G_BASE_CR4);
							if (fixs_ith_switchInterface::SBIA_OK != op_result)
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to restore the if mau type for EthernetPort <%s> to 40G",m_smxId.c_str());
								return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
							}
							else
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Restored the if mau type for EthernetPort <%s> to 40G",m_smxId.c_str());
								// Returning ERROR as the auto-negotiation setting failed.
								return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
							}

						}

					}
					else if (modifiedEthernetPort->getAutoNegotiate() == imm::AN_UNLOCKED)
					{
						// Set the default type of port to 40GbaseCR4
						fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_eth_if_mau_type(m_smxId,modifiedEthernetPort->getPlane(),modifiedEthernetPort->getSnmpIndex(),fixs_ith_snmp::IF_MAU_TYPE_40G_BASE_CR4);

						if (fixs_ith_switchInterface::SBIA_OK != op_result)
						{
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to set if mau type for EthernetPort <%s>. Switch Interface Error <%d>",m_smxId.c_str(),common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(),modifiedEthernetPort->getName(), op_result);
							return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
						}

						// Enable the auto negotiation status
						op_result = m_switchInterface.set_eth_auto_negotiation(m_smxId, modifiedEthernetPort->getPlane(),modifiedEthernetPort->getSnmpIndex(), true);

						if (fixs_ith_switchInterface::SBIA_OK != op_result)
						{
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to enable auto negotiation on EthernetPort <%s>. Switch Interface Error <%d>",m_smxId.c_str(),common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(),modifiedEthernetPort->getName(), op_result);
							FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Restoring the if mau type for EthernetPort <%s> to 10G",m_smxId.c_str());

							op_result = m_switchInterface.set_eth_if_mau_type(m_smxId,modifiedEthernetPort->getPlane(),modifiedEthernetPort->getSnmpIndex(),fixs_ith_snmp::IF_MAU_TYPE_10G_BASE_CX4);

							if (fixs_ith_switchInterface::SBIA_OK != op_result)
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to restore the if mau type for EthernetPort <%s> to 10G",m_smxId.c_str());
								return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
							}
							else
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Restored the if mau type for EthernetPort <%s> to 10G",m_smxId.c_str());
								// Returning ERROR as the auto-negotiation setting failed.
								return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
							}

						}
					}

				}
				else
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] ERROR auto-negotiation is not supported by the Ethernet Port:<%s> with ifIndex:<%d>",m_smxId.c_str(),common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(),modifiedEthernetPort->getName(),modifiedEthernetPort->getSnmpIndex());
					return fixs_ith::ERR_CONFIG_ETH_AUTO_NEG_CHANGE_NOT_ALLOWED;

				}

			}
			else if (common::utility::isSwitchBoardSMX())
			{

				fixs_ith_switchInterface::op_result op_result =m_switchInterface.get_eth_if_mau_type(if_mau_type,modifiedEthernetPort->getSnmpIndex(), m_smxId,modifiedEthernetPort->getPlane(), &timeout_ms);

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to get if mau type for EthernetPort <%s>. Switch Interface Error <%d>",m_smxId.c_str(),common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(),modifiedEthernetPort->getName(), op_result);
					return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}

				if (!modifiedEthernetPort->isAutoNegotiationSupported(if_mau_type) && modifiedEthernetPort->isAutoNegotiateEnabled()) {
					FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] ERROR auto-negotiation is not supported by the Ethernet Port:<%s> with ifIndex:<%d>",m_smxId.c_str(),common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(),modifiedEthernetPort->getName(),modifiedEthernetPort->getSnmpIndex());
					return fixs_ith::ERR_CONFIG_ETH_AUTO_NEG_CHANGE_NOT_ALLOWED;
				}

				if (modifiedEthernetPort->isSFPport())
				{
					result = operation::pim_adaptation::m_pim_adaptation(m_smxId, modifiedEthernetPort->getPlane(),modifiedEthernetPort->getSnmpIndex(), true,modifiedEthernetPort->isAutoNegotiateEnabled());
				}
				else
				{
					fixs_ith_switchInterface::op_result op_result =m_switchInterface.set_eth_auto_negotiation(m_smxId,modifiedEthernetPort->getPlane(),modifiedEthernetPort->getSnmpIndex(),modifiedEthernetPort->isAutoNegotiateEnabled());

					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR,"[%s][%s] Failed to set auto negotiation on EthernetPort <%s>. Switch Interface Error <%d>",m_smxId.c_str(),common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(),modifiedEthernetPort->getName(), op_result);
						return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
				}
			}
		}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_ethernetPortMap.erase(elementFound);
				m_ethernetPortMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<EthernetPort> >(modifiedEthernetPort->getPlane(), modifiedEthernetPort));
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] ERROR setting AutoNegAdminStaus for Ethernet Port:<%s> with ifIndex:<%d>. "
						"Check if either auto negotiation is NOT supported or default MAU type value is NOT set or for some other SMX/ITH error",
						m_smxId.c_str(), common::utility::planeToString(modifiedEthernetPort->getPlane()).c_str(), modifiedEthernetPort->getName(), modifiedEthernetPort->getSnmpIndex());
				return fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setEthernetPort(modifiedEthernetPort);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceAggregator(boost::shared_ptr<Aggregator>& modifiedAggregator)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aggregator_idx]);

		boost::shared_ptr<Aggregator> oldAggregator;
		aggregatorMap_t::iterator elementFound;

		std::pair<aggregatorMap_t::iterator, aggregatorMap_t::iterator> range;
		range = m_aggregatorMap.equal_range(modifiedAggregator->getPlane());

		for (aggregatorMap_t::iterator element = range.first; element != range.second && NULL == oldAggregator; ++element)
		{
			if (strcmp(modifiedAggregator->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldAggregator = element->second;
			}
		}

		if (NULL != oldAggregator)
		{
			if (oldAggregator->getAdminAggMemberDN() != modifiedAggregator->getAdminAggMemberDN())
			{
				std::set<uint16_t > aggregatorIndexes = modifiedAggregator->getAdminAggMemebrIndex();
				std::set<uint16_t > allIndexes = getEthernetPortsIndexes(modifiedAggregator->getPlane());

				for (std::set<uint16_t >::iterator it = aggregatorIndexes.begin(); it != aggregatorIndexes.end(); ++it)
				{
					allIndexes.erase(*it);
				}
				//Clean other ports
				fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_aggregator_members(m_smxId, modifiedAggregator->getPlane(), 0, allIndexes);

				std::set<uint16_t > oldAaggregatorIndexes = oldAggregator->getAdminAggMemebrIndex();
				for (std::set<uint16_t >::iterator it = oldAaggregatorIndexes.begin(); it != oldAaggregatorIndexes.end(); ++it)
				{
					op_result = m_switchInterface.disable_rstp(m_smxId, oldAggregator->getPlane(), *it);

				}

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set admin members on aggregator <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), common::utility::planeToString(modifiedAggregator->getPlane()).c_str(), modifiedAggregator->getName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{
					op_result = m_switchInterface.set_aggregator_members(m_smxId, modifiedAggregator->getPlane(), modifiedAggregator->getSnmpIndex(), modifiedAggregator->getAdminAggMemebrIndex());

					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set admin members on aggregator <%s>. Switch Interface Error <%d>",
								m_smxId.c_str(), common::utility::planeToString(modifiedAggregator->getPlane()).c_str(), modifiedAggregator->getName(), op_result);
						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
				}
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_aggregatorMap.erase(elementFound);
				m_aggregatorMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Aggregator> >(modifiedAggregator->getPlane(), modifiedAggregator));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setAggregator(modifiedAggregator);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceVlan(boost::shared_ptr<Vlan>& modifiedVlan)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vlan_idx]);

		boost::shared_ptr<Vlan> oldVlan;
		vlanMap_t::iterator elementFound;

		std::pair<vlanMap_t::iterator, vlanMap_t::iterator> range;
		range = m_vlanMap.equal_range(modifiedVlan->getPlane());

		for (vlanMap_t::iterator element = range.first; element != range.second && NULL == oldVlan; ++element)
		{
			if (strcmp(modifiedVlan->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldVlan = element->second;
			}
		}

		if (NULL != oldVlan)
		{
			if (modifiedVlan->getVlanId() != imm::vlanId_range::UNDEFINED)
			{
				if (oldVlan->getVlanId() != modifiedVlan->getVlanId())
				{
					fixs_ith_switchInterface::op_result op_result =	m_switchInterface.create_vlan(m_smxId, modifiedVlan->getPlane(), modifiedVlan->getVlanId(), modifiedVlan->getName());

					if (fixs_ith_switchInterface::SBIA_OK != op_result &&
							fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to create Vlan <%s>. Switch Interface Error <%d>",
								m_smxId.c_str(), common::utility::planeToString(modifiedVlan->getPlane()).c_str(), modifiedVlan->getName(), op_result);
						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
				}


				if ((oldVlan->getVlanId() != modifiedVlan->getVlanId()) ||
						(oldVlan->getTaggedPortsDN() != modifiedVlan->getTaggedPortsDN()) ||
						(oldVlan->getUntaggedPortsDN() != modifiedVlan->getUntaggedPortsDN()))
				{
					fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_vlan_members(m_smxId, modifiedVlan->getPlane(), modifiedVlan->getVlanId(), modifiedVlan->getTaggedBridgePortsIndex(), modifiedVlan->getUntaggedBridgePortsIndex());

					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set tagged/untagged members of Vlan <%s>. Switch Interface Error <%d>",
								m_smxId.c_str(), common::utility::planeToString(modifiedVlan->getPlane()).c_str(), modifiedVlan->getName(), op_result);
						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
					/* HX22060  BEGIN */
					
					if ( modifiedVlan->getPlane() == fixs_ith::TRANSPORT_PLANE)
					{
						FIXS_ITH_LOG(LOG_LEVEL_WARN,"This VLAN is created on transport plane");
						std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts_old = oldVlan->getUntaggedBridgePorts();
						std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts_modified = modifiedVlan->getUntaggedBridgePorts();

						std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts_added;
						std::set_difference(untaggedBridgePorts_modified.begin(), untaggedBridgePorts_modified.end(), untaggedBridgePorts_old.begin(), untaggedBridgePorts_old.end(), std::inserter(untaggedBridgePorts_added, untaggedBridgePorts_added.begin()));

						std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts_removed; 
						std::set_difference( untaggedBridgePorts_old.begin(), untaggedBridgePorts_old.end(), untaggedBridgePorts_modified.begin(), untaggedBridgePorts_modified.end(), std::inserter(untaggedBridgePorts_removed, untaggedBridgePorts_removed.begin()));

						for ( std::set< boost::shared_ptr<BridgePort> >::iterator it = untaggedBridgePorts_added.begin (); it != untaggedBridgePorts_added.end(); ++it)
						{
							fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_port_default_vlan(m_smxId, (*it)->getPlane(), (*it)->getSnmpIndex(), modifiedVlan->getVlanId());
							std::cout << " added untagged part " << (*it)-> getName () << std::endl;
							FIXS_ITH_LOG(LOG_LEVEL_WARN, " Added untagged port : is <%s>", (*it)-> getName ()); 

							if (fixs_ith_switchInterface::SBIA_OK != op_result )
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set default Vlan of untagged port <%s>. Switch Interface Error <%d>",
										m_smxId.c_str(), common::utility::planeToString(modifiedVlan->getPlane()).c_str(), modifiedVlan->getName(), op_result);
								result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
								break;
							}
						}

						for ( std::set< boost::shared_ptr<BridgePort> >::iterator it = untaggedBridgePorts_removed.begin (); it != untaggedBridgePorts_removed.end(); ++it)
						{
							fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_port_default_vlan(m_smxId, (*it)->getPlane(), (*it)->getSnmpIndex(), (*it)->getDefaultVlanId());
							std::cout << " removed untagged part " << (*it)-> getName () << std::endl;
							FIXS_ITH_LOG(LOG_LEVEL_WARN, " Removed untagged port : is <%s>", (*it)-> getName ()); 

							if (fixs_ith_switchInterface::SBIA_OK != op_result )
							{
								FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set default Vlan of untagged port <%s>. Switch Interface Error <%d>",
										m_smxId.c_str(), common::utility::planeToString(modifiedVlan->getPlane()).c_str(), modifiedVlan->getName(), op_result);
								result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
								break;
							}
						}

						FIXS_ITH_LOG(LOG_LEVEL_WARN, "old VLAN ID is: <%d>", oldVlan->getVlanId());
						FIXS_ITH_LOG(LOG_LEVEL_WARN, "modified VLAN ID is: <%d>", modifiedVlan->getVlanId());

						if (oldVlan->getVlanId() != modifiedVlan->getVlanId())
						{
							std::set < boost::shared_ptr<BridgePort> > untaggedBridgePorts = modifiedVlan->getUntaggedBridgePorts();

							for ( std::set< boost::shared_ptr<BridgePort> >::iterator it = untaggedBridgePorts.begin (); it != untaggedBridgePorts.end(); ++it)
							{
								fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_port_default_vlan(m_smxId, (*it)->getPlane(), (*it)->getSnmpIndex(), modifiedVlan->getVlanId());
								std::cout << "addded untagged port is:"<< (*it)->getName () << std::endl;
								FIXS_ITH_LOG(LOG_LEVEL_WARN, "Addded untagged port is: <%s>", (*it)->getName ());
								if (fixs_ith_switchInterface::SBIA_OK != op_result )
								{
									FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set default Vlan of untagged port <%s>. Switch Interface Error <%d>",
											m_smxId.c_str(), common::utility::planeToString(modifiedVlan->getPlane()).c_str(), modifiedVlan->getName(), op_result);
									result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
									break;
								}
							}
						}
					}
					/* TR HX22060 END */

				}
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_vlanMap.erase(elementFound);
				m_vlanMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Vlan> >(modifiedVlan->getPlane(), modifiedVlan));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setVlan(modifiedVlan);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceSubnetVlan(boost::shared_ptr<SubnetVlan>& modifiedSubnetVlan)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[subnetVlan_idx]);

		boost::shared_ptr<SubnetVlan> oldSubnetVlan;
		subnetVlanMap_t::iterator elementFound;

		std::pair<subnetVlanMap_t::iterator, subnetVlanMap_t::iterator> range;
		range = m_subnetVlanMap.equal_range(modifiedSubnetVlan->getPlane());

		for (subnetVlanMap_t::iterator element = range.first; element != range.second && NULL == oldSubnetVlan; ++element)
		{
			if (strcmp(modifiedSubnetVlan->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldSubnetVlan = element->second;
			}
		}

		if (NULL != oldSubnetVlan)
		{
			if (oldSubnetVlan->getIpSubnet() != modifiedSubnetVlan->getIpSubnet())
			{
				//create SubnetVlan
				fixs_ith_switchInterface::op_result op_result =	m_switchInterface.create_ipv4subnet(m_smxId, modifiedSubnetVlan->getPlane(),modifiedSubnetVlan->getIpSubnet());
				if (fixs_ith_switchInterface::SBIA_OK != op_result &&
						fixs_ith_switchInterface::SBIA_OBJ_ALREADY_EXIST != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to create SubnetVlan <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), common::utility::planeToString(modifiedSubnetVlan->getPlane()).c_str(), modifiedSubnetVlan->getName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
				else
				{// SUBNET VLAN created or already existing

					fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_ipv4subnet_vlanid(m_smxId, modifiedSubnetVlan->getPlane(), modifiedSubnetVlan->getIpSubnet(), modifiedSubnetVlan->getVlanId());
					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set vid <%d> for SubnetVlan <%s>. Switch Interface Error <%d>",
								m_smxId.c_str(), common::utility::planeToString(modifiedSubnetVlan->getPlane()).c_str(), modifiedSubnetVlan->getVlanId(), modifiedSubnetVlan->getName(), op_result);
						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
					else
					{
						fixs_ith_switchInterface::op_result op_result =	m_switchInterface.delete_ipv4subnet(m_smxId, oldSubnetVlan->getPlane(), oldSubnetVlan->getIpSubnet());

						if (fixs_ith_switchInterface::SBIA_OK != op_result)
						{
							FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to delete SubnetVlan <%s>. Switch Interface Error <%d>",
									m_smxId.c_str(), common::utility::planeToString(oldSubnetVlan->getPlane()).c_str(), oldSubnetVlan->getName(), op_result);
							result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
						}
					}
				}

			}
			else if (modifiedSubnetVlan->getVlanId() != imm::vlanId_range::UNDEFINED)
			{
				if (oldSubnetVlan->getVlanId() != modifiedSubnetVlan->getVlanId())
				{
					fixs_ith_switchInterface::op_result op_result = m_switchInterface.set_ipv4subnet_vlanid(m_smxId, modifiedSubnetVlan->getPlane(), modifiedSubnetVlan->getIpSubnet(), modifiedSubnetVlan->getVlanId());
					if (fixs_ith_switchInterface::SBIA_OK != op_result)
					{
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to set vid of SubnetVlan <%s>. Switch Interface Error <%d>",
								m_smxId.c_str(), common::utility::planeToString(modifiedSubnetVlan->getPlane()).c_str(), modifiedSubnetVlan->getName(), op_result);
						result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
					}
				}
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_subnetVlanMap.erase(elementFound);
				m_subnetVlanMap.insert(std::pair<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<SubnetVlan> >(modifiedSubnetVlan->getPlane(), modifiedSubnetVlan));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setSubnetVlan(modifiedSubnetVlan);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceInterface(boost::shared_ptr<InterfaceIPv4>& modifiedInterface, const operation::interfaceInfo& modifiedInfo)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[interface_idx]);

		boost::shared_ptr<InterfaceIPv4> oldInterface;
		interfaceMap_t::iterator elementFound;

		std::pair<interfaceMap_t::iterator, interfaceMap_t::iterator> range;
		range = m_interfaceMap.equal_range(modifiedInterface->getRouterName());

		for (interfaceMap_t::iterator element = range.first; element != range.second && NULL == oldInterface; ++element)
		{
			if (strcmp(modifiedInterface->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldInterface = element->second;
			}
		}

		if (NULL != oldInterface)
		{
			fixs_ith_switchInterface::op_result op_result = m_switchInterface.modify_interfaceIPv4(modifiedInfo);

			if (fixs_ith_switchInterface::SBIA_OK != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to modify Interface <%s> of Router <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(), modifiedInterface->getName(), modifiedInterface->getRouterName(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_interfaceMap.erase(elementFound);
				m_interfaceMap.insert(std::pair<std::string, boost::shared_ptr<InterfaceIPv4> >(modifiedInterface->getRouterName(), modifiedInterface));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setInterface(modifiedInterface);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceVrrpSession(boost::shared_ptr<VrrpSession>& newVrrpSession)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);

		boost::shared_ptr<VrrpSession> oldVrrpSession;
		vrrpSessions_t::iterator elementFound;

		std::pair<vrrpSessions_t::iterator, vrrpSessions_t::iterator> range;
		range = m_vrrpSessions.equal_range(newVrrpSession->getRouterName());

		for (vrrpSessions_t::iterator element = range.first; range.second != element; ++element)
		{
			if( (strcmp(newVrrpSession->getName(), element->second->getName()) == 0)
					&& (strcmp(newVrrpSession->getInterfaceName(), element->second->getInterfaceName()) == 0) )
			{
				elementFound = element;
				oldVrrpSession = element->second;
				break;
			}
		}

		if(oldVrrpSession)
		{
			fixs_ith_switchInterface::op_result op_result = m_switchInterface.modify_vrrpv3Session(newVrrpSession->getData());

			if(fixs_ith_switchInterface::SBIA_OK != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to modify VrrpSession:<%s> of [Router:<%s>, Interface:<%s>]. Switch Interface Error <%d>",
						m_smxId.c_str(), newVrrpSession->getName(), newVrrpSession->getRouterName(), newVrrpSession->getInterfaceName(), op_result);

				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_vrrpSessions.erase(elementFound);
				newVrrpSession->resetChangeMask();
				m_vrrpSessions.insert(std::make_pair(newVrrpSession->getRouterName(), newVrrpSession));

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] VrrpSession:<%s> on Interface:<%s> of Router:<%s> replaced.",
						m_smxId.c_str(), newVrrpSession->getName(), newVrrpSession->getInterfaceName(), newVrrpSession->getRouterName() );
			}
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "[%s] Replace VrrpSession:<%s> of [Router:<%s>, Interface:<%s>] not found.",
					m_smxId.c_str(), newVrrpSession->getName(), newVrrpSession->getRouterName(), newVrrpSession->getInterfaceName());

			//Object was missing in context... Create it
			result = setVrrpSession(newVrrpSession);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceRouterInterfaceAddress(boost::shared_ptr<Address>& modifiedAddress, const operation::addressInfo& modifiedAddressInfo)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		boost::shared_ptr<Address> oldAddress;
		addressMap_t::iterator elementFound;

		std::pair<addressMap_t::iterator, addressMap_t::iterator> addressRange;
		addressRange = m_addressMap.equal_range(modifiedAddress->getRouterName());

		for (addressMap_t::iterator element = addressRange.first; element != addressRange.second && NULL == oldAddress; ++element)
		{
			if (strcmp(modifiedAddress->getInterfaceName(), element->second->getInterfaceName()) == 0 &&
					strcmp(modifiedAddress->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldAddress = element->second;

				break;
			}
		}

		if (NULL != oldAddress)
		{
//			operation::addressInfo addressIPv4Info(modifiedAddressInfo);
//			addressIPv4Info.address.clear();
//			fixs_ith_switchInterface::op_result op_result = m_switchInterface.get_addressIPv4(addressIPv4Info);
//
//			if (fixs_ith_switchInterface::SBIA_OK != op_result)
//			{
//				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Failed to get addressIPv4 <%s>. Switch Interface Error <%d>",
//						m_smxId.c_str(),  addressIPv4Info.getName(), op_result);
//				//result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
//			}
//
//		 	std::string smx_address = addressIPv4Info.getAddress();
//			std::string newAddress_str(modifiedAddress->getAddress());
//			if (smx_address.compare(newAddress_str))

			if (strcmp(oldAddress->getAddress(), modifiedAddress->getAddress()) != 0)
			{
				fixs_ith_switchInterface::op_result op_result = m_switchInterface.modify_addressIPv4(modifiedAddressInfo);

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to modify Address <%s> on Interface <%s> of Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), modifiedAddress->getName(), modifiedAddress->getInterfaceName(), modifiedAddress->getRouterName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_addressMap.erase(elementFound);
				m_addressMap.insert(std::pair<std::string, boost::shared_ptr<Address> >(modifiedAddress->getRouterName(), modifiedAddress));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setRouterInterfaceAddress(modifiedAddress);
		}


		return result;
	}

	fixs_ith::ErrorConstants Context::replaceDst(boost::shared_ptr<Dst>& modifiedDst, const operation::dstInfo& modifiedInfo)
	{
		UNUSED(modifiedInfo);

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[dst_idx]);

		boost::shared_ptr<Dst> oldDst;
		dstMap_t::iterator elementFound;

		std::pair<dstMap_t::iterator, dstMap_t::iterator> range;
		range = m_dstMap.equal_range(modifiedDst->getRouterName());

		for (dstMap_t::iterator element = range.first; element != range.second && NULL == oldDst; ++element)
		{
			if (strcmp(modifiedDst->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldDst = element->second;
			}
		}

		if (NULL != oldDst)
		{
			fixs_ith_switchInterface::op_result op_result = m_switchInterface.modify_Dst(modifiedInfo);

			if (fixs_ith_switchInterface::SBIA_OK != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to modify Dst <%s> of Router <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(), modifiedDst->getName(), modifiedDst->getRouterName(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_dstMap.erase(elementFound);
				m_dstMap.insert(std::pair<std::string, boost::shared_ptr<Dst> >(modifiedDst->getRouterName(), modifiedDst));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setDst(modifiedDst);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceNextHop(boost::shared_ptr<NextHop>& modifiedNextHop, const operation::nextHopInfo& modifiedNextHopInfo)
	{
		UNUSED(modifiedNextHopInfo);

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		boost::shared_ptr<NextHop> oldObject;
		nextHopMap_t::iterator elementFound;

		std::pair<nextHopMap_t::iterator, nextHopMap_t::iterator> range;
		range = m_nextHopMap.equal_range(modifiedNextHop->getRouterName());

		for (nextHopMap_t::iterator element = range.first; element != range.second && NULL == oldObject; ++element)
		{
			if (strcmp(modifiedNextHop->getDstName(), element->second->getDstName()) == 0 &&
					strcmp(modifiedNextHop->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldObject = element->second;

				break;
			}
		}

		if (NULL != oldObject)
		{
			fixs_ith_switchInterface::op_result op_result = m_switchInterface.modify_NextHopInfo(modifiedNextHopInfo);

			if (fixs_ith_switchInterface::SBIA_OK != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to modify NextHop <%s> on Dst <%s> of Router <%s>. Switch Interface Error <%d>",
						m_smxId.c_str(), modifiedNextHop->getName(), modifiedNextHop->getDstName(), modifiedNextHop->getRouterName(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_nextHopMap.erase(elementFound);
				m_nextHopMap.insert(std::pair<std::string, boost::shared_ptr<NextHop> >(modifiedNextHop->getRouterName(), modifiedNextHop));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setNextHop(modifiedNextHop);
		}


		return result;
	}

	fixs_ith::ErrorConstants Context::replaceVrrpInterface(boost::shared_ptr<VrrpInterface>& modifiedInterface, const operation::vrrpInterfaceInfo& modifiedInterfaceInfo)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpInterface_idx]);

		boost::shared_ptr<VrrpInterface> oldVrrpInterface;
		vrrpInterfaceSet_t::iterator elementFound;

		for (vrrpInterfaceSet_t::iterator element = m_vrrpInterfaceSet.begin(); element != m_vrrpInterfaceSet.end() && NULL == oldVrrpInterface; ++element)
		{
			if (strcmp(modifiedInterface->getName(), (*element)->getName()) == 0)
			{
				elementFound = element;
				oldVrrpInterface = *element;

				break;
			}
		}

		if (NULL != oldVrrpInterface)
		{
			//Update the VrrpSessions referencing this object
			if(!(modifiedInterfaceInfo.changeMask & imm::vrrpInterface_attribute::RESERVED_BY_CHANGE))
			{
				std::set<std::string>::const_iterator it;
				std::set<std::string> reservedByList = modifiedInterface->getData().getReservedBy();
				std::set<std::string>::iterator firstSessionIt = reservedByList.begin();
				std::set<std::string>::iterator lastSessionIt = reservedByList.end();
				for (it = firstSessionIt; it != lastSessionIt; ++it)
				{
					std::string vrrpv3SessionSmxId = common::utility::getSmxIdFromVrrpv3SessionDN(*it);
					engine::ContextAccess access(vrrpv3SessionSmxId, GET_EXISTING, SHARED_ACCESS);
					Context* context = access.getContext();
					boost::shared_ptr<VrrpSession> vrrpSession = context->getVrrpSessionByDN(*it);
					vrrpSession->setVrrpInterfaceInfo(modifiedInterfaceInfo);
					result = context->replaceVrrpSession(vrrpSession);

					if (fixs_ith::ERR_NO_ERRORS != result)
					{
						//TODO rollback operation for VrrpSession successfull notified
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to replace VRRP Session <%s> in the context <%s> "
								"with the modified VRRP Interface Info <%s>",
								(*it).c_str(), vrrpv3SessionSmxId.c_str(), modifiedInterfaceInfo.getName());
						break;
					}
				}
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_vrrpInterfaceSet.erase(elementFound);
				m_vrrpInterfaceSet.insert(modifiedInterface);
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setVrrpInterface(modifiedInterface);
		}


		return result;
	}

	fixs_ith::ErrorConstants Context::replaceVrrpInterfaceAddress(boost::shared_ptr<Address>& modifiedAddress, const operation::addressInfo& modifiedAddressInfo)
	{
		UNUSED(modifiedAddressInfo);

		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[address_idx]);

		boost::shared_ptr<Address> oldAddress;
		addressMap_t::iterator elementFound;

		std::pair<addressMap_t::iterator, addressMap_t::iterator> addressRange;
		addressRange = m_addressMap.equal_range(modifiedAddress->getRouterName());

		for (addressMap_t::iterator element = addressRange.first; element != addressRange.second && NULL == oldAddress; ++element)
		{
			if (strcmp(modifiedAddress->getInterfaceName(), element->second->getInterfaceName()) == 0 &&
					strcmp(modifiedAddress->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldAddress = element->second;

				break;
			}
		}

		if (NULL != oldAddress)
		{
			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_addressMap.erase(elementFound);
				m_addressMap.insert(std::pair<std::string, boost::shared_ptr<Address> >(modifiedAddress->getRouterName(), modifiedAddress));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setRouterInterfaceAddress(modifiedAddress);
		}


		return result;
	}

	fixs_ith::ErrorConstants Context::replaceBfdProfile(boost::shared_ptr<BfdProfile>& newBfdProfile, const operation::bfdProfileInfo& newBfdProfileInfo)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdProfile_idx]);

		boost::shared_ptr<BfdProfile> oldBfdProfile;
		bfdProfileMap_t::iterator elementFound;

		std::pair<bfdProfileMap_t::iterator, bfdProfileMap_t::iterator> range;
		range = m_bfdProfileMap.equal_range(newBfdProfile->getRouterName());

		for (bfdProfileMap_t::iterator element = range.first; element != range.second && NULL == oldBfdProfile; ++element)
		{
			if (strcmp(newBfdProfile->getName(), element->second->getName()) == 0)
			{
				elementFound = element;
				oldBfdProfile = element->second;
			}
		}

		if (NULL != oldBfdProfile)
		{
			if(!(newBfdProfileInfo.changeMask & imm::bfdProfile_attribute::RESERVED_BY_CHANGE))
			{
				fixs_ith_switchInterface::op_result op_result = m_switchInterface.modify_BfdProfile(newBfdProfileInfo);

				if (fixs_ith_switchInterface::SBIA_OK != op_result)
				{
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to modify BfdProfile <%s> of Router <%s>. Switch Interface Error <%d>",
							m_smxId.c_str(), newBfdProfile->getName(), newBfdProfile->getRouterName(), op_result);
					result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
				}
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_bfdProfileMap.erase(elementFound);
				m_bfdProfileMap.insert(std::pair<std::string, boost::shared_ptr<BfdProfile> >(newBfdProfile->getRouterName(), newBfdProfile));
			}
		}
		else
		{
			//Object was missing in context... Create it
			result = setBfdProfile(newBfdProfile);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceBfdSessionIPv4(boost::shared_ptr<BfdSessionIPv4>& newBfdSession)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[bfdSession_idx]);

		std::pair<bfdSessions_t::iterator, bfdSessions_t::iterator> range;
		range = m_bfdSessions.equal_range(newBfdSession->getRouterName());

		// for each bfd session on this router
		for(bfdSessions_t::iterator element = range.first; range.second != element; ++element)
		{
			if( strcmp(newBfdSession->getName(), element->second->getName()) == 0 )
			{
				// Bfd session found replace it
				m_bfdSessions.erase(element);
				m_bfdSessions.insert(std::make_pair(newBfdSession->getRouterName(), newBfdSession));

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] BFD Session:<%s> of Router:<%s> replaced.",
						m_smxId.c_str(), newBfdSession->getName(),  newBfdSession->getRouterName() );

				return fixs_ith::ERR_NO_ERRORS;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_WARN, "[%s] Replace BFD Session:<%s> of Router:<%s> not found.",
					m_smxId.c_str(), newBfdSession->getName(), newBfdSession->getRouterName());

		//Object was missing in context... Create it
		return setBfdSessionIPv4(newBfdSession);
	}

	fixs_ith::ErrorConstants Context::replaceAclEntryIpv4(boost::shared_ptr<AclEntryIpv4>& newAclEntry, const operation::aclEntryInfo& newAclEntryInfo)
	{
		std::string routerName = newAclEntry->getRouterName();
		std::string aclIpv4Name = newAclEntry->getAclIpv4Name();
		std::string aclEntryName = newAclEntry->getName();
		fixs_ith::ErrorConstants result = checkAclEntryPriorityUniqueness(routerName, aclIpv4Name, aclEntryName, newAclEntry->getData().getPriority());

		if (fixs_ith::ERR_NO_ERRORS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "priority <%d> is not unique.", newAclEntry->getData().getPriority());
			return result;
		}

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclEntry_idx]);

		boost::shared_ptr<AclEntryIpv4> oldAclEntry;
		aclEntries_t::iterator elementFound;

		std::pair<aclEntries_t::iterator, aclEntries_t::iterator> range;
		range = m_aclEntries.equal_range(newAclEntry->getRouterName());

		// for each ACL entry on this router
		for(aclEntries_t::iterator element = range.first; range.second != element && NULL == oldAclEntry; ++element)
		{
			if( strcmp(newAclEntry->getName(), element->second->getName()) == 0 )
			{
				elementFound = element;
				oldAclEntry = element->second;
			}
		}

		if (NULL != oldAclEntry)
		{
			fixs_ith_switchInterface::op_result op_result = m_switchInterface.modify_AclEntry(newAclEntryInfo);

			if (fixs_ith_switchInterface::SBIA_OK != op_result)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] Failed to modify AclEntryIpv4:<%s> on AclIpv4:<%s> of Router:<%s>. Switch Interface Error <%d>",
						m_smxId.c_str(), newAclEntry->getName(), newAclEntry->getAclIpv4Name(), newAclEntry->getRouterName(), op_result);
				result = fixs_ith::ERR_CONFIG_SB_INTERFACE_FAILURE;
			}

			if (fixs_ith::ERR_NO_ERRORS == result)
			{
				m_aclEntries.erase(elementFound);
				m_aclEntries.insert(std::make_pair(newAclEntry->getRouterName(), newAclEntry));

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] AclEntryIpv4:<%s> on AclIpv4:<%s> of Router:<%s> replaced.",
						m_smxId.c_str(), newAclEntry->getName(), newAclEntry->getAclIpv4Name(), newAclEntry->getRouterName());
			}
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "[%s] AclEntryIpv4:<%s> on AclIpv4:<%s> of Router:<%s> not found.",
					m_smxId.c_str(), newAclEntry->getName(), newAclEntry->getAclIpv4Name(), newAclEntry->getRouterName());

			//Object was missing in context... Create it
			result = setAclEntryIpv4(newAclEntry);
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::replaceAclIpv4(boost::shared_ptr<AclIpv4>& newAclIpv4)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclIpv4_idx]);

		std::pair<aclIpV4_t::iterator, aclIpV4_t::iterator> range;
		range = m_aclIpV4.equal_range(newAclIpv4->getRouterName());

		// for each acl entry on this router
		for(aclIpV4_t::iterator element = range.first; range.second != element; ++element)
		{
			if( strcmp(newAclIpv4->getName(), element->second->getName()) == 0 )
			{
				// Acl Ipv4 found replace it
				m_aclIpV4.erase(element);
				m_aclIpV4.insert(std::make_pair(newAclIpv4->getRouterName(), newAclIpv4));

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "[%s] Acl IpV4:<%s> of Router:<%s> replaced.",
						m_smxId.c_str(), newAclIpv4->getName(), newAclIpv4->getRouterName() );

				return fixs_ith::ERR_NO_ERRORS;
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_WARN, "[%s] Acl IpV4:<%s> of Router:<%s> not found.",
				m_smxId.c_str(), newAclIpv4->getName(), newAclIpv4->getRouterName());

		//Object was missing in context... Create it
		return setAclIpV4(newAclIpv4);
	}

	fixs_ith::ErrorConstants Context::checkVrIdentityUniqueness(int32_t vrIdentity)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpInterface_idx]);

		for(vrrpInterfaceSet_t::const_iterator it = m_vrrpInterfaceSet.begin(); it != m_vrrpInterfaceSet.end(); ++it)
		{
			if(vrIdentity == (*it)->getData().vrIdentity)
			{
				result = fixs_ith::ERR_CONFIG_NOT_UNIQUE_VRID;
				break;
			}
		}

		return result;
	}

	fixs_ith::ErrorConstants Context::checkAclEntryPriorityUniqueness(const std::string& routerName, const std::string& aclIpv4Name, const std::string& aclEntryName, const uint16_t priority)
	{
		fixs_ith::ErrorConstants result = fixs_ith::ERR_NO_ERRORS;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[aclEntry_idx]);

		std::pair<aclEntries_t::iterator, aclEntries_t::iterator> range;
		range = m_aclEntries.equal_range(routerName);

		for(aclEntries_t::const_iterator it = range.first; it != range.second; ++it)
		{
			if((strcmp(aclIpv4Name.c_str(), it->second->getAclIpv4Name()) == 0) &&
					(strcmp(aclEntryName.c_str(), it->second->getName()) != 0) &&
					(priority == it->second->getData().getPriority()))
			{
				result = fixs_ith::ERR_CONFIG_NOT_UNIQUE_ACLENTRY_PRIORITY;
				break;
			}
		}

		return result;
	}

	const bool Context::checkRouterNextHopsAddress(const std::string& routerName, const std::string& address)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		bool elementFound = false;

		std::pair<nextHopMap_t::iterator, nextHopMap_t::iterator> range;
		range = m_nextHopMap.equal_range(routerName);

		for (nextHopMap_t::const_iterator element = range.first; element != range.second; ++element)
		{
			if (strcmp(address.c_str(), element->second->getAddress()) == 0 )
			{
				elementFound = true;
				break;
			}
		}

		return elementFound;
	}

	void Context::loadRuntimeInfo(boost::shared_ptr<BridgePort>& item)
	{
		//Get operative state
		fixs_ith_switchInterface::IF_oper_state if_op_state = fixs_ith_switchInterface::IF_STATE_UNKNOWN;

		uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;
		m_switchInterface.get_if_operState(if_op_state, item->getSnmpIndex(), m_smxId, item->getPlane(), &timeout_ms);

		item->setOperState((imm::oper_State) if_op_state);
	}

	void Context::loadRuntimeInfo(boost::shared_ptr<EthernetPort>& item)
	{
		// Get operOperatingMode
		fixs_ith_switchInterface::IF_oper_mode if_oper_mode = fixs_ith_switchInterface::IF_OPERMODE_UNKNOWN;

		uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;
		m_switchInterface.get_if_operOperating_mode(if_oper_mode, item->getSnmpIndex(), m_smxId, item->getPlane(), &timeout_ms);

		item->setOperOperatingMode((imm::EthOperOperatingMode) if_oper_mode);

		// Get auto negotiation admin status
		int autoNegotiationState = 0;
		m_switchInterface.get_if_auto_negotiation_state(autoNegotiationState, item->getSnmpIndex(), m_smxId, item->getPlane(), &timeout_ms);

		// ENABLED(1), DIASBLE(2)
		if(autoNegotiationState)
		{
			if(item->isSFPport())
			{
				// Get mau type
				std::vector<bool> if_mau_type;
				m_switchInterface.get_eth_if_mau_type(if_mau_type, item->getSnmpIndex(), m_smxId, item->getPlane(), &timeout_ms);
				bool isAutoNegDisabled = autoNegotiationState - 1;
				autoNegotiationState = (isAutoNegDisabled != if_mau_type[fixs_ith_snmp::IF_MAU_TYPE_1000_BASE_TFD]) ? 2 : 1;
			}
		}

		// Set autoNegotiationState
		item->setAutoNegotiationState((imm::EthAutoNegotiationState) autoNegotiationState);
	}

	void Context::loadRuntimeInfo(boost::shared_ptr<Aggregator>& item)
	{
		//Get operative state
		fixs_ith_switchInterface::IF_oper_state if_op_state = fixs_ith_switchInterface::IF_STATE_UNKNOWN;

		uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;
		m_switchInterface.get_if_operState(if_op_state, item->getSnmpIndex(), m_smxId, item->getPlane(), &timeout_ms);

		item->setOperState((imm::oper_State) if_op_state);

		//Get operAggMember
		std::set<uint16_t> indexes;
		m_switchInterface.get_aggregator_operMembers(indexes, item->getSnmpIndex(), m_smxId, item->getPlane(), &timeout_ms);

		item->setOperAggMember(indexes);

		//Get dataRate
		uint32_t dataRate = 0;
		m_switchInterface.get_if_highSpeed(dataRate, item->getSnmpIndex(), m_smxId, item->getPlane(), &timeout_ms);

		item->setDataRate(dataRate);
	}

	void Context::loadRuntimeInfo(boost::shared_ptr<InterfaceIPv4>& item, const uint16_t& runtimeAttributeMask)
	{
		if (runtimeAttributeMask)
		{
			operation::interfaceInfo info(item->getData());

			info.changeMask = runtimeAttributeMask;

			m_switchInterface.get_interfaceIPv4_info(info);
			item->setRuntimeInfo(info);
		}
	}


	const bool Context::isAggregatorMember(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& snmpIndex)
	{
		std::set< boost::shared_ptr<Aggregator> > aggregators = getAggregators(plane);
		bool isAggregatorMember = false;

		for (std::set< boost::shared_ptr<Aggregator> >::iterator it = aggregators.begin(); it != aggregators.end() && !isAggregatorMember; ++it)
		{
			isAggregatorMember = (*it)->hasAdminAggMemberIndex(snmpIndex);
		}

		return isAggregatorMember;
	}

	const bool Context::isAggregatedPort(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& snmpIndex)
	{
		std::set< boost::shared_ptr<Aggregator> > aggregators = getAggregators(plane);
		bool isAggregatedPort = false;

		for (std::set< boost::shared_ptr<Aggregator> >::iterator it = aggregators.begin(); it != aggregators.end() && !isAggregatedPort; ++it)
		{
			isAggregatedPort = (*it)->getSnmpIndex() == snmpIndex;
		}

		return isAggregatedPort;
	}

	const bool Context::isEncapsulation(const std::string& encapsulationDN)
	{
		std::set< boost::shared_ptr<InterfaceIPv4> > interfaces = getInterfaces();
		bool isEncapsulation = false;

		for (std::set< boost::shared_ptr<InterfaceIPv4> >::iterator it = interfaces.begin(); it != interfaces.end() && !isEncapsulation; ++it)
		{
			isEncapsulation = encapsulationDN.compare((*it)->getEncapsulationDN()) == 0;
		}

		return isEncapsulation;
	}

	const bool Context::hasLoopback(const std::string& routerName)
	{
		std::set< boost::shared_ptr<InterfaceIPv4> > interfaces = getInterfaces(routerName);
		bool isLoopback = false;

		for (std::set< boost::shared_ptr<InterfaceIPv4> >::iterator it = interfaces.begin(); it != interfaces.end() && !isLoopback; ++it)
		{
			isLoopback = (*it)->isLoopback();
		}

		return isLoopback;
	}

	const bool Context::hasDst(const std::string& routerName, const std::string& dst)
	{
		std::set< boost::shared_ptr<Dst> > dstSet = getDsts(routerName);
		bool dstFound = false;

		for (std::set< boost::shared_ptr<Dst> >::iterator it = dstSet.begin(); it != dstSet.end() && !dstFound; ++it)
		{
			dstFound = (dst.compare((*it)->getDst()) == 0);
		}

		return dstFound;
	}

	const bool Context::hasRouterInterfaceAddress(const std::string& routerName, const std::string& address)
	{
		std::set< boost::shared_ptr<Address> > addressSet = getRouterInterfaceAddresses(routerName);
		bool conflictFound = false;

		for (std::set< boost::shared_ptr<Address> >::iterator it = addressSet.begin(); it != addressSet.end() && !conflictFound; ++it)
		{
			conflictFound = common::utility::compareIpv4Addresses(address, (*it)->getAddress());
		}

		return conflictFound;
	}

	const bool Context::hasConflictingRouterInterfaceAddress(const std::string& routerName, const std::string& address)
	{
		std::set< boost::shared_ptr<Address> > addressSet = getRouterInterfaceAddresses(routerName);
		bool conflictFound = false;

		for (std::set< boost::shared_ptr<Address> >::iterator it = addressSet.begin(); it != addressSet.end() && !conflictFound; ++it)
		{
			conflictFound = common::utility::compareIpv4Subnets(address, (*it)->getAddress());
		}

		return conflictFound;
	}

	bool Context::hasVrrpSession(const std::string& routerName, const std::string& interfaceName)
	{
		bool result = false;
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);

		std::pair<vrrpSessions_t::const_iterator, vrrpSessions_t::const_iterator> vrrpSessionRange;
		vrrpSessionRange = m_vrrpSessions.equal_range(routerName);

		for(vrrpSessions_t::const_iterator element = vrrpSessionRange.first; vrrpSessionRange.second != element; ++element)
		{
			if( (interfaceName.compare(element->second->getInterfaceName()) == 0) )
			{
				result = true;
				break;
			}
		}

		return result;
	}

	bool Context::hasVrrpSessionOwner(const std::string& vrrpInterfaceName)
	{
		bool result = false;
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);

		std::set< boost::shared_ptr<VrrpSession> > otherVrrpSessions = getVrrpSessions(vrrpInterfaceName);

		std::set< boost::shared_ptr<VrrpSession> >::const_iterator element;

		for(element = otherVrrpSessions.begin(); otherVrrpSessions.end() != element; ++element )
		{
			if( (*element)->getData().getPriority() == imm::vrrpSessionPriority::OWNER )
			{
				result = true;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] VRRP Session OWNER already defined <%s> for VRRP Interface:<%s>",
							m_smxId.c_str(), (*element)->getDN(), vrrpInterfaceName.c_str());
				break;
			}
		}

		return result;
	}

	bool Context::hasPrioritizedVrrpSession(const std::string& vrrpInterfaceName)
	{
		bool result = false;
		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[vrrpSession_idx]);

		std::set< boost::shared_ptr<VrrpSession> > otherVrrpSessions = getVrrpSessions(vrrpInterfaceName);

		std::set< boost::shared_ptr<VrrpSession> >::const_iterator element;

		for(element = otherVrrpSessions.begin(); otherVrrpSessions.end() != element; ++element )
		{
			if( (*element)->getData().getPrioritizedSession() == imm::VRRP_TRUE )
			{
				result = true;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s] VRRP Prioritized Session already defined <%s> for VRRP Interface:<%s>",
						m_smxId.c_str(), (*element)->getDN(), vrrpInterfaceName.c_str());
				break;
			}
		}

		return result;
	}

	bool Context::hasAddressNextHop(const std::string& routerName, const std::string& dstName, const std::string& nextHopName)
	{
		bool result = false;

		boost::lock_guard<boost::recursive_mutex> guard(m_contextMutex[nextHop_idx]);

		std::pair<nextHopMap_t::const_iterator, nextHopMap_t::const_iterator> nextHopRange;
		nextHopRange = m_nextHopMap.equal_range(routerName);

		for(nextHopMap_t::const_iterator element = nextHopRange.first; nextHopRange.second != element; ++element)
		{
			if( ( dstName.compare(element->second->getDstName()) == 0U )
					&& ( nextHopName.compare(element->second->getName()) == 0U ) )
			{
				result = element->second->hasAddress();
				break;
			}
		}

		return result;
	}

	void Context::clear()
	{
		//Lock all...
		boost::lock_guard<boost::recursive_mutex> guard_1(m_contextMutex[bridge_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_2(m_contextMutex[ethernetPort_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_3(m_contextMutex[aggregator_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_4(m_contextMutex[bridgePort_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_5(m_contextMutex[vlan_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_6(m_contextMutex[subnetVlan_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_7(m_contextMutex[router_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_8(m_contextMutex[interface_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_9(m_contextMutex[address_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_10(m_contextMutex[dst_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_11(m_contextMutex[nextHop_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_12(m_contextMutex[vrrpInterface_idx]);
		boost::lock_guard<boost::recursive_mutex> guard_13(m_contextMutex[vrrpSession_idx]);
		// ...and clear all!
		m_bridgeMap.clear();
		m_ethernetPortMap.clear();
		m_bridgePortMap.clear();
		m_aggregatorMap.clear();
		m_vlanMap.clear();
		m_subnetVlanMap.clear();
		m_routerSet.clear();
		m_interfaceMap.clear();
		m_addressMap.clear();
		m_dstMap.clear();
		m_nextHopMap.clear();
		m_vrrpInterfaceSet.clear();
		m_vrrpSessions.clear();
		m_bfdSessions.clear();
	}

	bool Context::isMonitoredPortLink(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& snmpIndex)
	{
		if (fixs_ith::CONTROL_PLANE == plane)
		{
			return engine::control::MONITORED_INTERFACES_INDEXES_SET.count(snmpIndex) > 0;
		}
		else if (fixs_ith::TRANSPORT_PLANE == plane)
		{
			return engine::transport::MONITORED_INTERFACES_INDEXES_SET.count(snmpIndex) > 0;
		}

		return false;
	}

	bool Context::isBackPlanePort(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t if_index)
	{
		if (fixs_ith::CONTROL_PLANE == plane)
		{
			return engine::control::BP_INTERFACES_INDEXES_SET.count(if_index) > 0;
		}
		else if (fixs_ith::TRANSPORT_PLANE == plane)
		{
			if (common::utility::isSwitchBoardSMX())
			{
				return engine::transport::BP_INTERFACES_INDEXES_SET.count(if_index) > 0;
			}
			else if (common::utility::isSwitchBoardCMX())
			{
                                return engine::transport::BP_INTERFACES_INDEXES_SET_CMX.count(if_index) > 0;

			}
		}
		return false;
	}

	bool Context::isControlPort(const uint16_t if_index)
	{

		return engine::transport::CONTROLPORTS_INTERFACES_INDEXES_SET.count(if_index) > 0;

	}

	bool Context::isTransportPort(const uint16_t if_index)
	{

		return engine::control::TRANSPORTPORTS_INTERFACES_INDEXES_SET.count(if_index) > 0;

	}
}



