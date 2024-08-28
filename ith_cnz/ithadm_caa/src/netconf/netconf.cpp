/*
 * fixs_ith_netconfconstants.h
 *
 *  Created on: May 5, 2016
 *      Author: xassore
 */

#include "netconf/netconf.h"

namespace netconf {

	namespace netconf_errTagMessage {

		const char * ERR_DATA_EXISTS = "data-exists";
		const char * ERR_DATA_MISSING = "data-missing";
		const char * OPERATION_FAILED = "operation-failed";

	}

	namespace operation {
		const char * OPERATION_XML_ATTR_NO_OPERATION = "";
		const char * OPERATION_XML_ATTR_MERGE = "xc:operation=\"merge\"";
		const char * OPERATION_XML_ATTR_REPLACE = "xc:operation=\"replace\"";
		const char * OPERATION_XML_ATTR_CREATE = "xc:operation=\"create\"";
		const char * OPERATION_XML_ATTR_DELETE = "xc:operation=\"delete\"";
		const char * OPERATION_XML_ATTR_UNKNOWN = "xc:operation=\"UNKNOWN\"";
	}

	namespace managedElement {
		const char * CLASS_NAME = "ManagedElement";

		namespace attribute {
			const char * KEY = "managedElementId";
		}
	}

	namespace ers {
		const char * CLASS_NAME = "Ers";

		namespace attribute {
			const char * KEY = "ersId";
		}
	}

	namespace ersTransport {
		const char * CLASS_NAME = "ErsTransport";

		namespace attribute {
			const char * KEY = "ersTransportId";
		}
	}

	namespace ip {
		const char * CLASS_NAME = "Ip";

		namespace attribute {
			const char * KEY = "ipId";
		}
	}

	namespace transport {
		const char * CLASS_NAME = "Transport";

		namespace attribute {
			const char * KEY = "transportId";
		}
	}

	namespace policies {
		const char * CLASS_NAME = "Policies";

		namespace attribute {
			const char * KEY = "policiesId";
		}
	}

	namespace switch_mom_dn {
		 const char * BRIDGE_DN = "ManagedElement=1,Ers=1,ErsTransport=1,Ethernet=1,Bridge=1";
		 const char * TRANSPORT_DN = "ManagedElement=1,Transport=1";
		 const char * POLICIES_BASE_DN = "ManagedElement=1,Ers=1,Policies=1";
	}

	namespace router {
		const char * CLASS_NAME = "Router";

		namespace attribute {
			const char * KEY = "routerId";
		}
	}

	namespace interfaceIpv4 {
		const char * CLASS_NAME = "InterfaceIPv4";

		namespace attribute {
			const char * KEY = "interfaceIPv4Id";
			const char * MTU = "mtu";
			const char * ENCAPSULATION = "encapsulation";
			const char * ENCAPSULATION_VID = "encapsulationVid";
			const char * MAC = "mac";
			const char * OPER_STATE = "operState";
			const char * BFD_PROFILE = "bfdProfile";
			const char * ADMIN_STATE = "adminState";
		}
	}

	namespace addressIpv4 {
		const char * CLASS_NAME = "Address";

		namespace attribute {
			const char * KEY = "addressId";
			const char * ADDRESS = "address";
		}
	}

	namespace bfdSessionInfo {
		const char * CLASS_NAME = "BfdSessionInfo";

		namespace attribute {
			const char * KEY = "bfdSessionInfoId";
			const char * YOUR_DISCRIMINATOR = "yourDiscriminator";
			const char * DETECTION_TIME = "calculatedDetectionTime";
			const char * NEGOTIATED_INTERVAL = "negotiatedInterval";
			const char * STATE = "state";
			const char * UDP_PORT_SRC = "srcUdpPort";
			const char * PEER_INTERFACE = "peerInterface";
			const char * PEER_ADDRESS = "peerAddress";
			const char * MY_DISCRIMINATOR = "myDiscriminator";
			const char * DIAGNOSTIC_CODE = "diagnosticCode";
		}
	}

	namespace bfdProfile {
		const char * CLASS_NAME = "BfdProfile";

		namespace  attribute {
			const char * KEY = "bfdProfileId";
			const char * DETECTION_MULTI = "detectionMultiplier";
			const char * INTERVAL_TX_MIN = "desiredMinTxInterval";
			const char * INTERVAL_RX_MIN = "requiredMinRxInterval";
			const char * INTERVAL_ECHO_RX_MIN = "requiredMinEchoRxInterval";
		}
	}

	namespace routeTableStatic {
		const char * CLASS_NAME = "RouteTableIPv4Static";

		namespace attribute {
			const char * KEY = "routeTableIPv4StaticId";
		}
	}

	namespace routeTable {
		const char * CLASS_NAME = "RouteTableIPv4";

		namespace attribute {
			const char * KEY = "routeTableIPv4Id";
		}
	}

	namespace dst {
		const char * CLASS_NAME = "Dst";

		namespace attribute {
			const char * KEY = "dstId";
			const char * DST = "dst";
		}
	}

	namespace nextHop {
		const char * CLASS_NAME = "NextHop";

		namespace attribute {
			const char * KEY = "nextHopId";
			const char * ADMIN_DISTANCE = "adminDistance";
			const char * BFD_ENABLE = "bfdEnable";
			const char * ADDRESS = "address";
			const char * NEXT_HOP = "nextHop";
			const char * DISCARD = "discard";
			const char * REFERENCE = "reference";
		}
	}

	namespace vrrpv3IPv4Session {
		const char * CLASS_NAME = "Vrrpv3IPv4Session";

		namespace attribute {
			const char * KEY = "vrrpv3Ipv4SessionId";
			const char * VR_ID = "vrId";
			const char * ADMIN_STATE = "adminState";
			const char * ADV_INTERVAL = "advertiseInterval";
			const char * OWNER = "owner";
			const char * PREEMPT_MODE = "preemptMode";
			const char * PREEMPT_HOLD_TIME = "preemptHoldTime";
			const char * PRIORITY = "priority";
			const char * DISCONTINUITY_TIME = "discontinuityTime";
			const char * COUNTER_REFRESH_INTERVAL = "counterRefreshInterval";
			const char * VRRP_STATE = "vrrpState";
			const char * PROTO_ERROR_REASON = "protoErrorReason";
		}
	}

	namespace protVirtIPv4Address {
		const char * CLASS_NAME = "ProtVirtIPv4Address";

		namespace attribute {
			const char * KEY = "protVirtIPv4AddressId";
			const char * ADDRESS = "address";
		}
	}

	namespace peerIpv4 {
		const char * CLASS_NAME = "PeerIPv4";

		namespace attribute {
			const char * KEY = "peerIPv4Id";
			const char * ADDRESS = "address";
		}
	}

	namespace rule {
		const char * CLASS_NAME = "Rule";

		namespace attribute {
			const char * KEY = "ruleId";
			const char * PRIORITY = "priority";
			const char * POLICY_GROUP = "policy-group";
		}
	}

	namespace ruleIf {
		const char * CLASS_NAME = "If";

		namespace attribute {
			const char * KEY = "ifId";
			const char * IPV4_ADDR_DST = "ipv4-addr-dst";
			const char * IPV4_ADDR_SRC = "ipv4-addr-src";
			const char * IP_DSCP = "ip-dscp";
			const char * IP_FRAG = "ip-frag";
			const char * IP_PROTOCOL = "ip-protocol";
			const char * L4_PORT_DST = "l4-port-dst";
			const char * L4_PORT_SRC = "l4-port-src";
			const char * IP_VER = "ip-ver";
		}
	}

	namespace ruleThen {
		const char * CLASS_NAME = "Then";

		namespace attribute {
			const char * KEY = "thenId";
			const char * NO_OPERATION = "no-operation";
			const char * DROP = "drop";
		}
	}

	namespace policy {
		const char * CLASS_NAME = "Policy";

		namespace attribute {
			const char * KEY = "policyId";
		}
	}

	namespace virtualRouter{

			const char * CLASS_NAME = "VirtualRouter";

			namespace attribute {
				const char * KEY = "virtualRouterId";
				const char * NAME = "name";
			}
		}

	namespace ipInterface{

		const char * CLASS_NAME = "IpInterface";

		namespace attribute {
			const char * KEY = "ipInterfaceId";
			const char * ETHERNET_PORT = "ethernet-port";
			const char * IPV4_ADDR = "ipv4-addr";
			const char * IPV6_ADDR = "ipv6-addr";
			const char * PBF_INGRESS = "pbf-ingress";

		}
	}


}
