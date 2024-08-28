/*
 * imm.h
 *
 *  Created on: Feb 22, 2016
 *      Author: eanform
 */

#include "imm/imm.h"
#include "common/utility.h"

namespace imm
{
	namespace regular_expression
	{
		const std::string VALID_OBJECT_ID = "^([A-Za-z0-9]*)$";
	}

	namespace comcli_errormessage
	{
		const std::string GENERIC 							= "Generic Error";
		const std::string TOO_MANY_OPERATIONS 				= "Too many operations in the same transaction";
		const std::string MEMORY_BAD_ALLOC 					= "Memory Allocation Failure";
		const std::string COMMUNICATION_FAILURE 			= "Infrastructure Communication Failure";
		const std::string PROTECTED_OBJECT 					= "System-protected object cannot be modified or deleted";
		const std::string INVALID_BRIDGE_ID 				= "Invalid Bridge Identifier";
		const std::string INVALID_BRIDGE_PORT_ID 			= "Invalid BridgePort Identifier";
		const std::string INVALID_ETHERNET_PORT_ID 			= "Invalid EthernetPort Identifier";
		const std::string ETH_AUTO_NEG_CHANGE_NOT_ALLOWED 	= "auto-negotiation configuration not allowed on this Ethernet port";
		const std::string ETH_AUTO_NEG_CHANGE_NOT_SUPPORTED = "auto-negotiation is not supported by the Ethernet port";
		const std::string INVALID_AGGREGATOR_ID 			= "Invalid Aggregator Identifier";
		const std::string INVALID_VLAN_ID 					= "Invalid VLAN ID";
		const std::string INVALID_SUBNET_VLAN_ID 			= "Invalid VLAN ID";
		const std::string INVALID_SUBNET_VLAN_PLANE 		= "Unsupported Bridge for Subnet VLAN";
		const std::string INVALID_SUBNET_IP 				= "Invalid IPv4 subnet";
		const std::string DUPLICATED_VLAN_ID 				= "Specified VLAN ID is already used";
		const std::string RESTRICTED_VLAN_ID 				= "Cannot modify VLAN ID";
		const std::string PROTECTED_VLAN 					= "Cannot delete a VLAN used by a Subnet VLAN";
		const std::string INVALID_TAGGED_PORT 				= "Invalid tagged BridgePort specified";
		const std::string INVALID_UNTAGGED_PORT 			= "Invalid untagged BridgePort specified";
		const std::string AGG_MEMBER_PORT 					= "Specified BridgePort is member of an Aggregator";
		const std::string DUPLICATED_PORT 					= "Same BridgePort cannot be set as both tagged and untagged member of a VLAN";
		const std::string DUPLICATED_SUBNET_VLAN_IP 		= "Specified Ipv4Subnet is already used";
		const std::string INVALID_ROUTER_NAME 				= "Invalid Router name";
		const std::string INVALID_INTERFACE_ID 				= "Invalid InterfaceIPv4 identifier";
		const std::string INVALID_INTERFACE_NAME 			= "Invalid InterfaceIPv4 name. It shall not contain special characters";
		const std::string INVALID_VRRP_INTERFACE_NAME 		= "Invalid Vrrpv3Interface name";
		const std::string INVALID_BFDPROFILE_NAME 			= "Invalid BfdProfile name";
		const std::string INVALID_LOOPBACK 					= "One of the attributes encapsulation and loopback must be set, but not both at the same time";
		const std::string INVALID_ENCAPSULATION 			= "Invalid encapsulation specified";
		const std::string DUPLICATED_ENCAPSULATION 			= "Specified encapsulation VLAN is already used";
		const std::string MISSING_ENCAPSULATION_VID 		= "Specified encapsulation VLAN does not have a valid VLAN ID";
		const std::string DUPLICATED_LOOPBACK 				= "Cannot define multiple loopback interfaces on same Router";
		const std::string INVALID_ADDRESS_ID 				= "Invalid AddressIPv4 identifier";
		const std::string INVALID_ADDRESS 					= "Invalid IPv4 address";
		const std::string INVALID_DST_ID 					= "Invalid Dst identifier";
		const std::string DUPLICATED_DST 					= "Specified dst is already used";
		const std::string INVALID_NEXTHOP_ID 				= "Invalid NextHop identifier";
		const std::string INVALID_DISCARD 					= "One of the attributes address and discard must be set, but not both at the same time";
		const std::string REFERENCED_OBJECT 				= "Cannot remove an object referenced by other object(s)";
		const std::string INVALID_VRRPSESSION_STATE 		= "VRRP Session must be in LOCKED administrative state";
		const std::string INVALID_VRRP_ADVERTISE_INTERVAL 	= "Advertisement Interval is allowed only in multiple of 5 Centi Secs";
		const std::string MISSING_VRRP_ADDRESS 				= "Missing VRRP Interface address";
		const std::string MISSING_INTERFACE_ADDRESS 		= "Missing Router Interface address";
		const std::string INVALID_VRRP_PRIORITIZED_VALUE 	= "Prioritized VRRP session already defined";
		const std::string INVALID_VRRP_OWNER 				= "VRRP session in owner mode must be prioritized";
		const std::string NOT_UNIQUE_VRID 					= "Specified VR ID is already used";
		const std::string NOT_UNIQUE_ACLENTRY_PRIORITY 		= "Specified ACL entry priority is already used";
		const std::string ACLENTRY_ICMP_IP_MISMATCH 		= "icmpType can be non-empty only if ipProtocol contains one single-value element equal to ICMP (1)";
		const std::string ACLENTRY_PORT_IP_MISMATCH 		= "portDst and portSrc can be non-empty only if ipProtocol contains one single-value element equal to UDP (17), TCP (6), or SCTP (132)";
		const std::string PROTECTED_ADDRESS 				= "Cannot delete/modify an AddressIPv4 used by a Vrrpv3Session";
		const std::string ADDRESS_CONFLICT 					= "Specified address conflicts with another address on the same Router";
		const std::string CONFLICTING_ADDRESS 				= "Specified address conflicts with another address on the same Router";
		const std::string CONFLICTING_NEXTHOP 				= "Specified address conflicts with own address";
		const std::string CONFLICTING_VRRPSESSION 			= "Specified VRRP interface already used from another VRRP session on the same Router";
		const std::string CONFLICTING_VRRPSESSION_OWNER 	= "Specified VRRP interface is referenced by another VRRP session with the same value of prioritizedSession attribute";
		const std::string BFD_CONFLICT 						= "BFD cannot be enabled without a valid BFD profile";
		const std::string BACKUPONGOING_OPNOTPERMITTED 		= "Command not executed, AP backup in progress";
		const std::string INVALID_BRIDGEPORT_STATE 			= "Cannot change LOCKED administrative state for the specified BridgePort";
	}

	/**
	 * @namespace oi_name
	 *
	 * @brief
	 * Contains the names of all Object Implementers registered into IMM.
	 *
	 */
	namespace oi_name
	{
		//L2
		const std::string IMPL_BRIDGE = "IthBridgeOi";
		const std::string IMPL_BRIDGEPORT = "IthBridgePortOi";
		const std::string IMPL_ETHERNETPORT = "IthEthernetPortOi";
		const std::string IMPL_AGGREGATOR = "IthAggregatorOi";
		const std::string IMPL_VLAN = "IthVlanOi";
		const std::string IMPL_SUBNETVLAN = "IthSubnetVlanOi";

		//L3
		const std::string IMPL_ROUTER = "IthRouterOi";
		const std::string IMPL_ACL = "IthAclOi";
		const std::string IMPL_ACLENTRY = "IthAclEntryOi";
		const std::string IMPL_BFDPROFILE = "IthBfdProfileOi";
		const std::string IMPL_BFDSESSION = "IthBfdSessionOi";
		const std::string IMPL_INTERFACE = "IthInterfaceOi";
		const std::string IMPL_ADDRESS = "IthAddressOi";
		const std::string IMPL_VRRPINTERFACE = "IthVrrpInterfaceOi";
		const std::string IMPL_VRRPSESSION = "IthVrrpSessionOi";
		const std::string IMPL_ROUTETABLE = "IthRouteTableOi";
		const std::string IMPL_DST = "IthDstOi";
		const std::string IMPL_NEXTHOP = "IthNextHopOi";
	}

	/**
	 * @namespace moc_name
	 *
	 * @brief
	 * Contains the names of all Managed Object Classes.
	 *
	 */
	namespace moc_name
	{
		const std::string NAMESPACE_ITH = "AxeInfrastructureTransportM";

		const std::string CLASS_TRANSPORT = NAMESPACE_ITH + "TransportM";

		//L2 CLASSES
		const std::string CLASS_AGGREGATOR = NAMESPACE_ITH + "Aggregator";
		const std::string CLASS_BRIDGE = NAMESPACE_ITH + "Bridge";
		const std::string CLASS_BRIDGE_PORT = NAMESPACE_ITH + "BridgePort";
		const std::string CLASS_VLAN = NAMESPACE_ITH + "Vlan";
		const std::string CLASS_SUBNET_VLAN = NAMESPACE_ITH + "Ipv4SubnetVlan";
		const std::string CLASS_ETH_PORT = NAMESPACE_ITH + "EthernetPort";

		//L3 CLASSES
		const std::string CLASS_ROUTER = NAMESPACE_ITH + "Router";
		const std::string CLASS_ACL = NAMESPACE_ITH + "AclIpv4";
		const std::string CLASS_ACL_ENTRY = NAMESPACE_ITH + "AclEntryIpv4";
		const std::string CLASS_BFD_PROFILE = NAMESPACE_ITH + "BfdProfile";
		const std::string CLASS_BFD_SESSION = NAMESPACE_ITH + "BfdSessionIPv4";
		const std::string CLASS_INTERFACE = NAMESPACE_ITH + "InterfaceIPv4";
		const std::string CLASS_ADDRESS = NAMESPACE_ITH + "AddressIPv4";
		const std::string CLASS_VRRP_SESSION = NAMESPACE_ITH + "Vrrpv3Session";
		const std::string CLASS_VRRP_INTERFACE = NAMESPACE_ITH + "Vrrpv3Interface";
		const std::string CLASS_ROUTE_TABLE = NAMESPACE_ITH + "RouteTableIPv4Static";
		const std::string CLASS_DST = NAMESPACE_ITH + "Dst";
		const std::string CLASS_NEXTHOP = NAMESPACE_ITH + "NextHop";
	}

	namespace mom_dn
	{
		const char ROOT_MOC_DN [] = "AxeInfrastructureTransportMtransportMId=1";
	}

	namespace root_moc_attribute
	{
		const std::string RDN = "AxeInfrastructureTransportMtransportMId";
	}

	/**
	 * @namespace vlan_attribute
	 *
	 * @brief
	 * Contains the names of all attributes of MOC Vlan.
	 *
	 */
	namespace bridge_attribute
	{
		const std::string RDN = "bridgeId";
		const std::string ADDRESS = "bridgeAddress";
		const std::string SUBNET_VLAN_REF = "ipv4SubnetVlan";
		const std::string NUM_OF_VLANS = "numberOfVlans";
		const std::string NUM_OF_PORTS = "numberOfPorts";
	}

	namespace bridgePort_attribute
	{
		const std::string RDN = "bridgePortId";
		const std::string ADM_STATE = "adminState";
		const std::string DEFAULT_VLAN_ID = "defaultVlanId";
		const std::string PORT_ADDRESS = "portAddress";

		//Runtime
		const std::string OPER_STATE = "operState";
	}

	namespace ethernetPort_attribute
	{
		const std::string RDN = "ethernetPortId";
		const std::string MAC_ADDRESS = "macAddress";
		const std::string SNMP_INDEX = "snmpIndex";
		const std::string MAX_FRAME_SIZE = "maxFrameSize";
		const std::string AUTO_NEGOTIATE = "autoNegotiateAdminState";

		//Runtime
		const std::string OPERATING_MODE = "operOperatingMode";
		const std::string AUTO_NEGOTIATION_STATE = "autoNegotiateOperState";
	}

	namespace aggregator_attribute
	{
		const std::string RDN = "aggregatorId";
		const std::string AGG_MEMBER = "adminAggMember";
		const std::string SNMP_INDEX = "snmpIndex";

		//Runtime
		const std::string OPER_STATE = "operState";
		const std::string OPER_AGG_MEMBER = "operAggMember";
		const std::string DATA_RATE = "dataRate";
	}

	namespace vlan_attribute
	{
		const std::string RDN = "vlanId";
		const std::string VLAN_ID = "vid";
		const std::string UNTAGGED_PORTS = "untaggedBridgePorts";
		const std::string TAGGED_PORTS = "taggedBridgePorts";
		const std::string RESERVED_BY = "reservedBy";
	}

	namespace subnetVlan_attribute
	{
		const std::string RDN = "id";
		const std::string IP_SUBNET = "ipv4Subnet";
		const std::string SUBNET_VID = "vlanId";
	}

	namespace router_attribute
	{
		const std::string RDN = "routerId";
	}

	namespace bfdSession_attribute
	{
		const std::string RDN = "bfdSessionIPv4Id";

		//runtime
		const std::string ADDR_DST = "addrDst";
		const std::string DIAG_CODE = "diagCode";
		const std::string DETECTION_TIME = "detectionTime";
		const std::string PORT_SRC = "portSrc";
		const std::string LOCAL_DISCRIMINATOR = "localDiscriminator";
		const std::string REMOTE_DISCRIMINATOR = "remoteDiscriminator";
		const std::string INTERVAL_TX = "intervalTxNegotiated";
		const std::string SESSION_STATE = "sessionState";
		const std::string ADDR_SRC = "addrSrc";
	}

	namespace bfdProfile_attribute
	{
		const std::string RDN = "bfdProfileId";
		const std::string DETECTION_MULTI = "detectionMultiplier";
		const std::string INTERVAL_TX_MIN = "intervalTxMinDesired";
		const std::string INTERVAL_RX_MIN = "intervalRxMinRequired";
		const std::string RESERVED_BY = "reservedBy";
	}

	namespace addressIpv4_attribute
	{
		const std::string RDN = "addressIPv4Id";
		const std::string ADDRESS = "address";
	}

	namespace interfaceIpv4_attribute
	{
		const std::string RDN = "interfaceIPv4Id";
		const std::string ADMIN_STATE = "adminState";
		const std::string BFD_PROFILE = "bfdProfile";
		const std::string ACL_IPV4 = "aclIngress";
		const std::string ENCAPSULATION = "encapsulation";
		const std::string MTU = "mtu";
		const std::string BFD_STATIC_ROUTES = "bfdStaticRoutes";
		const std::string LOOPBACK = "loopback";

		//runtime
		const std::string OPER_STATE = "operationalState";
	}

	namespace routeTable_attribute
	{
		const std::string RDN = "routeTableIPv4StaticId";
	}

	namespace dst_attribute
	{
		const std::string RDN = "dstId";
		const std::string DST = "dst";
	}

	namespace nextHop_attribute
	{
		const std::string RDN = "nextHopId";
		const std::string ADMIN_DISTANCE = "adminDistance";
		const std::string BFD_MONITORING = "bfdMonitoring";
		const std::string ADDRESS = "address";
		const std::string DISCARD = "discard";
	}

	namespace vrrpSession_attribute
	{
		const std::string RDN = "vrrpv3SessionId";
		const std::string ADMIN_STATE = "administrativeState";
		const std::string PRIO_SESSION = "prioritizedSession";
		const std::string VRRP_INTERFACE_REF = "vrrpv3InterfaceRef";
		const std::string PRIORITY = "priority";
		const std::string VRRP_STATE = "vrrpState";
	}

	namespace vrrpInterface_attribute
	{
		const std::string RDN = "vrrpv3InterfaceId";
		const std::string IDENTITY = "vrIdentity";
		const std::string ADV_INTERVAL = "advertiseInterval";
		const int32_t ADV_INTERVAL_MULTIPLIER = 50; // ms
		const std::string PREEMPT_MODE = "preemptMode";
		const std::string PREEMPT_HOLD_TIME = "preemptHoldTime";
		const std::string RESERVED_BY = "reservedBy";
	}

	namespace aclEntry_attribute
	{
		const std::string RDN = "aclEntryIpv4Id";
		const std::string ACLACTION = "action";
		const std::string ADDR_DST = "addrDst";
		const std::string ADDR_SRC = "addrSrc";
		const std::string DSCP = "dscp";
		const std::string FRAGMENT_TYPE = "fragmentType";
		const std::string ICMP_TYPE = "icmpType";
		const std::string IP_PROTOCOL = "ipProtocol";
		const std::string PORT_DST = "portDst";
		const std::string PORT_SRC = "portSrc";
		const std::string PRIORITY = "priority";
	}

	namespace aclIpv4_attribute
	{
		const std::string RDN = "aclIpv4Id";
		const std::string RESERVED_BY = "reservedBy";
	}

	namespace plane_type
	{
		const std::string CONTROL = "LS";
		const std::string TRANSPORT = "HS";
	};


	namespace brf_mom_dn
	{
		const char BRF_MOC_DN [] = "brfParticipantContainerId=1";
	}

	namespace brfPersistentDataOwner
	{
		const std::string BRF_OBJECT_NAME = "brfPersistentDataOwnerId=ERIC-APG-FIXS-ITH";
		const std::string BRF_CLASS_NAME = "BrfPersistentDataOwner";
		const std::string BRF_RDN = "brfPersistentDataOwnerId";
		const std::string BRF_ATTR_VERSION= "version";
		const std::string BRF_ATTR_BACKPTYPE = "backupType";
	}

	ACS_CC_ReturnType getClassInstances(OmHandler* omHandler, const std::string& className, std::vector<std::string>& dnObjs)
	{
		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		if (omHandler)
		{
			result = ACS_CC_SUCCESS;
			std::vector<std::string> dnList;

			if (omHandler->getClassInstances(className.c_str(), dnList) != ACS_CC_SUCCESS &&
					omHandler->getInternalLastError() != imm::error_code::ERR_IMM_NOT_FOUND)
			{
				return ACS_CC_FAILURE;
			}

			dnObjs.insert(dnObjs.begin(), dnList.begin(), dnList.end());
		}

		return result;

	}


	ACS_CC_ReturnType getClassInstances(OmHandler* omHandler, const std::string& className, const std::string& switchboard_key, std::vector<std::string>& dnObjs)
	{
		ACS_CC_ReturnType result = ACS_CC_FAILURE;

		if (omHandler)
		{
			result = ACS_CC_SUCCESS;

			std::vector<std::string> dnList;

			if (omHandler->getClassInstances(className.c_str(), dnList) != ACS_CC_SUCCESS &&
					omHandler->getInternalLastError() != imm::error_code::ERR_IMM_NOT_FOUND)
			{
				return ACS_CC_FAILURE;
			}

			for(std::vector<std::string>::iterator objectDN = dnList.begin(); objectDN != dnList.end(); ++objectDN)
			{
				if (imm::moc_name::CLASS_BRIDGE.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromBridgeDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_ETH_PORT.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromEthernetPortDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_AGGREGATOR.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromAggregatorDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_BRIDGE_PORT.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromBridgePortDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_VLAN.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromVlanDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_SUBNET_VLAN.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromSubnetVlanDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_ROUTER.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromRouterDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_INTERFACE.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromInterfaceDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_ADDRESS.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromAddressDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_DST.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromDstDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_NEXTHOP.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromNextHopDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_BFD_PROFILE.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromBfdProfileDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_BFD_SESSION.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromBfdSessionDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_ACL.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromAclDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_ACL_ENTRY.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromAclEntryDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
				else if (imm::moc_name::CLASS_VRRP_SESSION.compare(className) == 0)
				{
					if (switchboard_key.compare(common::utility::getSmxIdFromVrrpv3SessionDN(*objectDN)) == 0)
					{
						dnObjs.push_back(*objectDN);
					}
				}
			}
		}

		return result;

	}

}
