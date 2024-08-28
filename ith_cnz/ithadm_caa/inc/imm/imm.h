/*
 * imm.h
 *
 *  Created on: Feb 22, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_IMM_IMM_H_
#define ITHADM_CAA_INC_IMM_IMM_H_

#include <stdint.h>

#include <string>

#include "acs_apgcc_omhandler.h"

namespace imm
{

#define IMM_DN_PATH_SIZE_MAX 512
#define IMM_RDN_PATH_SIZE_MAX 256
#define IMM_ATTR_NAME_MAX_SIZE 128
#define IMM_CLASS_NAME_MAX_SIZE 128
#define IMM_MAGAZINE_ADDRESS_SEPARATOR '.'
#define IMM_BRIDGE_NAME_SIZE_MAX  128
#define IMM_MACADDRESS_SIZE_MAX  17

#define EXIT_IF_ISBACKUPONGOING()  { if(fixs_ith::workingSet_t::instance()->isBackupOngoing()){ \
setExitCode(fixs_ith::ERR_BACKUP_IN_PROGRESS, configurationHelper_t::instance()->getErrorText(fixs_ith::ERR_BACKUP_IN_PROGRESS) );\
return ACS_CC_FAILURE;} }

#if 0
#define EXIT_IF_ISBACKUPONGOING() {};
#endif

	namespace regular_expression
	{
		extern const std::string VALID_OBJECT_ID;
	}

	namespace error_code
	{
		const int ERR_IMM_TRY_AGAIN = -6;
		const int ERR_IMM_BAD_HANDLE = -9;
		const int ERR_IMM_ALREADY_EXIST = -14;
		const int ERR_IMM_NOT_FOUND = -41;
	};

	namespace comcli_errormessage
	{
		extern const std::string GENERIC;
		extern const std::string TOO_MANY_OPERATIONS;
		extern const std::string MEMORY_BAD_ALLOC;
		extern const std::string COMMUNICATION_FAILURE;
		extern const std::string PROTECTED_OBJECT;
		extern const std::string INVALID_BRIDGE_ID;
		extern const std::string INVALID_BRIDGE_PORT_ID;
		extern const std::string INVALID_ETHERNET_PORT_ID;
		extern const std::string ETH_AUTO_NEG_CHANGE_NOT_ALLOWED;
		extern const std::string ETH_AUTO_NEG_CHANGE_NOT_SUPPORTED;
		extern const std::string INVALID_AGGREGATOR_ID;
		extern const std::string INVALID_VLAN_ID;
		extern const std::string INVALID_SUBNET_VLAN_ID;
		extern const std::string INVALID_SUBNET_VLAN_PLANE;
		extern const std::string INVALID_SUBNET_IP;
		extern const std::string DUPLICATED_VLAN_ID;
		extern const std::string RESTRICTED_VLAN_ID;
		extern const std::string PROTECTED_VLAN;
		extern const std::string INVALID_TAGGED_PORT;
		extern const std::string INVALID_UNTAGGED_PORT;
		extern const std::string AGG_MEMBER_PORT;
		extern const std::string DUPLICATED_PORT;
		extern const std::string DUPLICATED_SUBNET_VLAN_IP;

		extern const std::string INVALID_ROUTER_NAME;
		extern const std::string INVALID_INTERFACE_ID;
		extern const std::string INVALID_INTERFACE_NAME;
		extern const std::string INVALID_VRRP_INTERFACE_NAME;
		extern const std::string INVALID_BFDPROFILE_NAME;
		extern const std::string INVALID_LOOPBACK;
		extern const std::string INVALID_ENCAPSULATION;
		extern const std::string DUPLICATED_ENCAPSULATION;
		extern const std::string MISSING_ENCAPSULATION_VID;
		extern const std::string DUPLICATED_LOOPBACK;
		extern const std::string INVALID_ADDRESS_ID;
		extern const std::string INVALID_ADDRESS;
		extern const std::string INVALID_DST_ID;
		extern const std::string DUPLICATED_DST;
		extern const std::string INVALID_NEXTHOP_ID;
		extern const std::string INVALID_DISCARD;

		extern const std::string REFERENCED_OBJECT;
		extern const std::string INVALID_VRRPSESSION_STATE;
		extern const std::string INVALID_VRRP_ADVERTISE_INTERVAL;

		extern const std::string MISSING_VRRP_ADDRESS;
		extern const std::string MISSING_INTERFACE_ADDRESS;
		extern const std::string INVALID_VRRP_PRIORITIZED_VALUE;
		extern const std::string INVALID_VRRP_OWNER;

		extern const std::string NOT_UNIQUE_VRID;
		extern const std::string NOT_UNIQUE_ACLENTRY_PRIORITY;
		extern const std::string ACLENTRY_ICMP_IP_MISMATCH;
		extern const std::string ACLENTRY_PORT_IP_MISMATCH;
		extern const std::string PROTECTED_ADDRESS;
		extern const std::string ADDRESS_CONFLICT;

		extern const std::string CONFLICTING_ADDRESS;
		extern const std::string CONFLICTING_NEXTHOP;
		extern const std::string CONFLICTING_VRRPSESSION;
		extern const std::string CONFLICTING_VRRPSESSION_OWNER;
		extern const std::string BFD_CONFLICT;
		extern const std::string BACKUPONGOING_OPNOTPERMITTED;
		extern const std::string INVALID_BRIDGEPORT_STATE;
	}

	enum ImmAction
	{
		Nothing = 0,
		Create,
		Delete,
		Modify
	};

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
		extern const std::string IMPL_BRIDGE;
		extern const std::string IMPL_BRIDGEPORT;
		extern const std::string IMPL_ETHERNETPORT;
		extern const std::string IMPL_AGGREGATOR;
		extern const std::string IMPL_VLAN;
		extern const std::string IMPL_SUBNETVLAN;

		//L3
		extern const std::string IMPL_ROUTER;
		extern const std::string IMPL_ACL;
		extern const std::string IMPL_ACLENTRY;
		extern const std::string IMPL_BFDPROFILE;
		extern const std::string IMPL_BFDSESSION;
		extern const std::string IMPL_INTERFACE;
		extern const std::string IMPL_ADDRESS;
		extern const std::string IMPL_VRRPINTERFACE;
		extern const std::string IMPL_VRRPSESSION;
		extern const std::string IMPL_ROUTETABLE;
		extern const std::string IMPL_DST;
		extern const std::string IMPL_NEXTHOP;
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
		extern const std::string NAMESPACE_ITH;

		extern const std::string CLASS_TRANSPORT;

		//L2 CLASSES
		extern const std::string CLASS_AGGREGATOR;
		extern const std::string CLASS_BRIDGE;
		extern const std::string CLASS_BRIDGE_PORT;
		extern const std::string CLASS_VLAN;
		extern const std::string CLASS_SUBNET_VLAN;
		extern const std::string CLASS_ETH_PORT;

		//L3 CLASSES
		extern const std::string CLASS_ROUTER;
		extern const std::string CLASS_ACL;
		extern const std::string CLASS_ACL_ENTRY;
		extern const std::string CLASS_BFD_PROFILE;
		extern const std::string CLASS_BFD_SESSION;
		extern const std::string CLASS_INTERFACE;
		extern const std::string CLASS_ADDRESS;
		extern const std::string CLASS_VRRP_SESSION;
		extern const std::string CLASS_VRRP_INTERFACE;
		extern const std::string CLASS_ROUTE_TABLE;
		extern const std::string CLASS_DST;
		extern const std::string CLASS_NEXTHOP;
	}

	namespace mom_dn
	{
		extern const char ROOT_MOC_DN [];
	}

	namespace root_moc_attribute
	{
		extern const std::string RDN;
	}

	namespace change_mask
	{
		const uint16_t ALL_ATTRIBUTES = 0xffff;
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
		extern const std::string RDN;
		extern const std::string ADDRESS;
		extern const std::string SUBNET_VLAN_REF;
		extern const std::string NUM_OF_VLANS;
		extern const std::string NUM_OF_PORTS;

		enum ChangeMask
		{
			BRIDGE_ADDRESS_CHANGE 	= 0x0001,
			IPV4_SUBNET_CHANGE		= 0x0002
		};
	}

	namespace bridgePort_attribute
	{
		extern const std::string RDN;
		extern const std::string ADM_STATE;
		extern const std::string DEFAULT_VLAN_ID;
		extern const std::string PORT_ADDRESS;

		//Runtime
		extern const std::string OPER_STATE;

		enum ChangeMask
		{
			ADM_STATE_CHANGE 		= 0x0001,
			DEFAULT_VLAN_ID_CHANGE	= 0x0002,
			PORT_ADDRESS_CHANGE		= 0x0004
		};
	}

	namespace ethernetPort_attribute
	{
		extern const std::string RDN;
		extern const std::string MAC_ADDRESS;
		extern const std::string OPERATING_MODE;
		extern const std::string SNMP_INDEX;

		extern const std::string AUTO_NEGOTIATE;
		extern const std::string AUTO_NEGOTIATION_STATE;

		//Runtime
		extern const std::string MAX_FRAME_SIZE;

		enum ChangeMask
		{
			MAC_ADDRESS_CHANGE 		= 0x0001,
			SNMP_INDEX_CHANGE		= 0x0002,
			MAX_FRAME_SIZE_CHANGE	= 0x0004,
			AUTO_NEGOTIATE_CHANGE	= 0x0008
		};
	}

	namespace aggregator_attribute
	{
		extern const std::string RDN;
		extern const std::string AGG_MEMBER;
		extern const std::string SNMP_INDEX;

		//Runtime
		extern const std::string OPER_STATE;
		extern const std::string OPER_AGG_MEMBER;
		extern const std::string DATA_RATE;

		enum ChangeMask
		{
			ADMIN_AGG_MEMBER_CHANGE 		= 0x0001,
			SNMP_INDEX_CHANGE				= 0x0002
		};
	}

	namespace vlan_attribute
	{
		extern const std::string RDN;
		extern const std::string VLAN_ID;
		extern const std::string UNTAGGED_PORTS;
		extern const std::string TAGGED_PORTS;
		extern const std::string RESERVED_BY;

		enum ChangeMask
		{
			VLAN_ID_CHANGE 			= 0x0001,
			TAGGED_PORT_CHANGE 		= 0x0002,
			UNTAGGED_PORT_CHANGE 	= 0x0004,
			RESERVED_BY_CHANGE	 	= 0x0008
		};
	}

	namespace subnetVlan_attribute
	{
		extern const std::string RDN;
		extern const std::string IP_SUBNET;
		extern const std::string SUBNET_VID;

		enum ChangeMask
		{
			VLAN_ID_CHANGE			= 0x0001,
			IP_SUBNET_CHANGE 		= 0x0002
		};
	}

	namespace router_attribute
	{
		extern const std::string RDN;
	}

	namespace bfdSession_attribute
	{
		extern const std::string RDN;

		//runtime
		extern const std::string ADDR_DST;
		extern const std::string DIAG_CODE;
		extern const std::string DETECTION_TIME;
		extern const std::string PORT_SRC;
		extern const std::string LOCAL_DISCRIMINATOR;
		extern const std::string REMOTE_DISCRIMINATOR;
		extern const std::string INTERVAL_TX;
		extern const std::string SESSION_STATE;
		extern const std::string ADDR_SRC;
	}

	namespace bfdProfile_attribute
	{
		extern const std::string RDN;
		extern const std::string DETECTION_MULTI;
		extern const std::string INTERVAL_TX_MIN;
		extern const std::string INTERVAL_RX_MIN;
		extern const std::string RESERVED_BY;

		enum ChangeMask
		{
			DETECTION_MULTI_CHANGE			= 0x0001,
			INTERVAL_TX_MIN_CHANGE 			= 0x0002,
			INTERVAL_RX_MIN_CHANGE 			= 0x0004,
			RESERVED_BY_CHANGE				= 0x0008
		};
	}

	namespace addressIpv4_attribute
	{
		extern const std::string RDN;
		extern const std::string ADDRESS;

		enum ChangeMask
		{
			ADDRESS_CHANGE				= 0x0001
		};
	}

	namespace interfaceIpv4_attribute
	{
		extern const std::string RDN;
		extern const std::string ADMIN_STATE;
		extern const std::string BFD_PROFILE;
		extern const std::string ACL_IPV4;
		extern const std::string ENCAPSULATION;
		extern const std::string MTU;
		extern const std::string BFD_STATIC_ROUTES;
		extern const std::string LOOPBACK;

		//runtime
		extern const std::string OPER_STATE;

		enum ChangeMask
		{
			ADMIN_STATE_CHANGE			= 0x0001,
			BFD_PROFILE_CHANGE 			= 0x0002,
			ENCAPSULATION_CHANGE 		= 0x0004,
			MTU_CHANGE 					= 0x0008,
			BFD_STATIC_ROUTES_CHANGE 	= 0x0010,
			LOOPBACK_CHANGE 			= 0x0020,

			//runtime
			OPER_STATE_CHANGE 			= 0x0040,

			ACL_IPV4_CHANGE				= 0x0080,


			RESTRICTED_ATTRIBUTES = ENCAPSULATION_CHANGE | LOOPBACK_CHANGE
		};
	}

	namespace routeTable_attribute
	{
		extern const std::string RDN;
	}

	namespace dst_attribute
	{
		extern const std::string RDN;
		extern const std::string DST;

		enum ChangeMask
		{
			DST_CHANGE			= 0x0001
		};
	}

	namespace nextHop_attribute
	{
		extern const std::string RDN;
		extern const std::string ADMIN_DISTANCE;
		extern const std::string BFD_MONITORING;
		extern const std::string ADDRESS;
		extern const std::string DISCARD;

		enum ChangeMask
		{
			ADMIN_DISTANCE_CHANGE	= 0x0001,
			BFD_MONITORING_CHANGE 	= 0x0002,
			ADDRESS_CHANGE 			= 0x0004,
			DISCARD_CHANGE 			= 0x0008
		};
	}

	namespace vrrpSession_attribute
	{
		extern const std::string RDN;
		extern const std::string ADMIN_STATE;
		extern const std::string PRIO_SESSION;
		extern const std::string VRRP_INTERFACE_REF;
		extern const std::string PRIORITY;
		extern const std::string VRRP_STATE;

		enum ChangeMask
		{
			ADMIN_STATE_CHANGE			= 0x0001,
			PRIO_SESSION_CHANGE 		= 0x0002,
			VRRP_INTERFACE_REF_CHANGE 	= 0x0004,
			PRIORITY_CHANGE 			= 0x0008
		};
	}

	namespace vrrpInterface_attribute
	{
		extern const std::string RDN;
		extern const std::string IDENTITY;
		extern const std::string ADV_INTERVAL;
		extern const int32_t ADV_INTERVAL_MULTIPLIER;
		extern const std::string PREEMPT_MODE;
		extern const std::string PREEMPT_HOLD_TIME;
		extern const std::string RESERVED_BY;

		enum ChangeMask
		{
			IDENTITY_CHANGE				= 0x0001,
			ADV_INTERVAL_CHANGE 		= 0x0002,
			PREEMPT_MODE_CHANGE		 	= 0x0004,
			PREEMPT_HOLD_TIME_CHANGE 	= 0x0008,
			RESERVED_BY_CHANGE			= 0x0010
		};
	}

	namespace aclEntry_attribute
	{
		extern const std::string RDN;
		extern const std::string ACLACTION;
		extern const std::string ADDR_DST;
		extern const std::string ADDR_SRC;
		extern const std::string DSCP;
		extern const std::string FRAGMENT_TYPE;
		extern const std::string ICMP_TYPE;
		extern const std::string IP_PROTOCOL;
		extern const std::string PORT_DST;
		extern const std::string PORT_SRC;
		extern const std::string PRIORITY;

		enum ChangeMask
		{
			ACTION_CHANGE				= 0x0001,
			ADDR_DST_CHANGE 			= 0x0002,
			ADDR_SRC_CHANGE 			= 0x0004,
			DSCP_CHANGE 				= 0x0008,
			FRAGMENT_TYPE_CHANGE		= 0x0010,
			ICMP_TYPE_CHANGE			= 0x0020,
			IP_PROTOCOL_CHANGE			= 0x0040,
			PORT_DST_CHANGE				= 0x0080,
			PORT_SRC_CHANGE				= 0x0100,
			PRIORITY_CHANGE				= 0x0200
		};
	}

	namespace aclIpv4_attribute
	{
		extern const std::string RDN;
		extern const std::string RESERVED_BY;

		enum ChangeMask
		{
			RESERVED_BY_CHANGE		= 0x0001,
		};
	}

	enum aclAction
	{
		PERMIT = 0,
		DENY
	};

	enum aclFragmentType
	{
		FRAGMENT = 3,
		INITIAL,
		NONINITIAL
	};

	namespace vlanId_range
	{
		const uint16_t UNDEFINED = 0;
		const uint16_t DEFAULT = 1;
		const uint16_t UPPER_BOUND = 4093;
	};

	namespace plane_type
	{
		extern const std::string CONTROL;
		extern const std::string TRANSPORT;
	};

	enum admin_State
	{
		LOCKED = 0,
		UNLOCKED,
		EMPTY_ADMIN_STATE
	};

	enum oper_State
	{
		DISABLED = 0,
		ENABLED,
		EMPTY_OPER_STATE
	};

	enum vrrpvPrioSession
	{
		VRRP_TRUE = 0,
		VRRP_FALSE
	};

	enum vrrpSessionState
	{
		INIT = 1,
		BACKUP = 2,
		MASTER = 3
	};

	namespace vrrpSessionPriority
	{
		const uint32_t OWNER = 255U;
		const uint32_t DEFAULT_MASTER = 254U;
		const uint32_t DEFAULT_BACKUP = 100U;
	};

	enum BfdSessionDiagnCode
	{
		BFD_DC_NO_DIAG = 0,
		BFD_DC_CONTR_DETECTION_TIME_EXP = 1,
		BFD_DC_ECHO_FUNCTION_FAILED = 2,
		BFD_DC_NEIGHBOR_SIG_SESSION_DOWN = 3,
		BFD_DC_FORWARDING_PLANE_RESET = 4,
		BFD_DC_PATH_DOWN = 5,
		BFD_DC_CONCAT_PATH_DOWN = 6,
		BFD_DC_ADMIN_DOWN = 7,
		BFD_DC_REVERSE_CONCAT_PATH_DOWN = 8,
	};

	enum BfdSessionOperState
	{
		BFD_OPERSTATE_ADMIN_DOWN = 0,
		BFD_OPERSTATE_DOWN = 1,
		BFD_OPERSTATE_INIT = 2,
		BFD_OPERSTATE_FAILING = 3,
		BFD_OPERSTATE_UP = 4,
	};

	enum EthOperOperatingMode {
		OPER_MODE_UNKNOWN = 1,
		OPER_MODE_10M_HALF = 2,
		OPER_MODE_10M_FULL = 3,
		OPER_MODE_100M_HALF = 4,
		OPER_MODE_100M_FULL = 5,
		OPER_MODE_1G_FULL = 6,
		OPER_MODE_1G_FULL_SLAVE = 7,
		OPER_MODE_1G_FULL_MASTER = 8,
		OPER_MODE_10G_FULL = 9,
		OPER_MODE_10G_FULL_MASTER = 10,
		OPER_MODE_10G_FULL_SLAVE = 11,
		OPER_MODE_40G_FULL = 12,
		OPER_MODE_100G_FULL = 13,
	};

	enum EthAutoNegotiate
	{
		AN_LOCKED = 0,
		AN_UNLOCKED
	};

	enum EthAutoNegotiationState
	{
		AN_STATE_UNKNOWN = 0,
		AN_ENABLED,
		AN_DISABLED
	};

	namespace brf_mom_dn
	{
		extern const char BRF_MOC_DN [];
	};

	namespace brfPersistentDataOwner
	{
		extern const std::string BRF_OBJECT_NAME;
		extern const std::string BRF_CLASS_NAME;
		extern const std::string BRF_RDN;
		extern const std::string BRF_ATTR_VERSION;
		extern const std::string BRF_ATTR_BACKPTYPE;
	};

	ACS_CC_ReturnType getClassInstances(OmHandler* omHandler, const std::string& className, std::vector<std::string>& dnObjs);

	ACS_CC_ReturnType getClassInstances(OmHandler* omHandler, const std::string& className, const std::string& switchboard_key, std::vector<std::string>& dnObjs);
}

#endif /* ITHADM_CAA_INC_IMM_IMM_H_ */
