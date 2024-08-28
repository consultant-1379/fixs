/*
 * fixs_ith_netconfconstants.h
 *
 *  Created on: May 5, 2016
 *      Author: xassore
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_NETCONFCONSTANTS_H_
#define ITHADM_CAA_INC_FIXS_ITH_NETCONFCONSTANTS_H_

#include <vector>
#include <utility>
#include <string>

namespace netconf {
	typedef std::pair<std::string, std::string> configdataElement;
	typedef std::vector<configdataElement> configdataList;



#define XML_PATH_MAX_SIZE 1024

	enum ErrorConstants {
		ERR_NO_ERROR = 0,
		ERR_IP_ADDRESS_NOT_FOUND,
		ERR_NO_ANSWER_RECEIVED,
		ERR_SSH_USERAUTH_FAILED,
		ERR_SEND_REQUEST_FAILED,
		ERR_ERROR_RECEIVED,
		ERR_PARSER_ANSWER_FAILED,
		ERR_NO_CONFIGURATION_DATA,
		ERR_BUILD_ELEMENT_PATH,
		ERR_BUILD_CONFIG_DOM,
		ERR_PREPARE_EDIT_CONFIG_REQUEST,
		ERR_BAD_DN,
		ERR_OBJ_ALREADY_EXISTS,
		ERR_DATA_MISSING,
		ERR_OPERATION_FAILED,
		ERR_BAD_OBJECT_ID
	};

	namespace netconf_errTagMessage {

		extern const char * ERR_DATA_EXISTS;
		extern const char * ERR_DATA_MISSING;
		extern const char * OPERATION_FAILED;

	}

	namespace authority_levels {
		enum UserType {
			BASIC = 0,
			ADVANCED = 1
		};
	}

	namespace operation {
		enum OperationType {
			NO_OPERATION = 0,
			MERGE,
			REPLACE,
			CREATE,
			DELETE
		};

		extern const char * OPERATION_XML_ATTR_NO_OPERATION;
		extern const char * OPERATION_XML_ATTR_MERGE;
		extern const char * OPERATION_XML_ATTR_REPLACE;
		extern const char * OPERATION_XML_ATTR_CREATE;
		extern const char * OPERATION_XML_ATTR_DELETE;
		extern const char * OPERATION_XML_ATTR_UNKNOWN;
	}

	namespace managedElement {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

	namespace ers {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

	namespace ersTransport {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

		namespace ip {
			extern const char* CLASS_NAME;

			namespace attribute {
				extern const char* KEY;
			}
		}

	namespace transport {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

	namespace policies {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

	namespace switch_mom_dn {
		extern const char * BRIDGE_DN;
		extern const char * TRANSPORT_DN;
		extern const char * POLICIES_BASE_DN;
	}

	namespace router {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

	namespace interfaceIpv4 {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * MTU;
			extern const char * ENCAPSULATION;
			extern const char * ENCAPSULATION_VID;
			extern const char * MAC;
			extern const char * OPER_STATE;
			extern const char * BFD_PROFILE;
			extern const char * ADMIN_STATE;
		}
	}

	namespace addressIpv4 {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * ADDRESS;
		}
	}

	namespace bfdSessionInfo {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * YOUR_DISCRIMINATOR;
			extern const char * DETECTION_TIME;
			extern const char * NEGOTIATED_INTERVAL;
			extern const char * STATE;
			extern const char * UDP_PORT_SRC;
			extern const char * PEER_INTERFACE;
			extern const char * PEER_ADDRESS;
			extern const char * MY_DISCRIMINATOR;
			extern const char * DIAGNOSTIC_CODE;
		}
	}

	namespace bfdProfile {
		extern const char * CLASS_NAME;

		namespace  attribute {
			extern const char * KEY;
			extern const char * DETECTION_MULTI;
			extern const char * INTERVAL_TX_MIN;
			extern const char * INTERVAL_RX_MIN;
			extern const char * INTERVAL_ECHO_RX_MIN;
		}
	}

	namespace routeTableStatic {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

	namespace routeTable {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

	namespace dst {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * DST;
		}
	}

	namespace nextHop {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * ADMIN_DISTANCE;
			extern const char * BFD_ENABLE;
			extern const char * ADDRESS;
			extern const char * NEXT_HOP;
			extern const char * DISCARD;
			extern const char * REFERENCE;
		}
	}

	namespace vrrpv3IPv4Session {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * VR_ID;
			extern const char * ADMIN_STATE;
			extern const char * ADV_INTERVAL;
			extern const char * OWNER;
			extern const char * PREEMPT_MODE;
			extern const char * PREEMPT_HOLD_TIME;
			extern const char * PRIORITY;
			extern const char * DISCONTINUITY_TIME;
			extern const char * COUNTER_REFRESH_INTERVAL;
			extern const char * VRRP_STATE;
			extern const char * PROTO_ERROR_REASON;
		}
	}

	namespace protVirtIPv4Address {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * ADDRESS;
		}
	}

	namespace peerIpv4 {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * ADDRESS;
		}
	}

	namespace rule {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * PRIORITY;
			extern const char * POLICY_GROUP;
		}
	}

	namespace ruleIf {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * IPV4_ADDR_DST;
			extern const char * IPV4_ADDR_SRC;
			extern const char * IP_DSCP;
			extern const char * IP_FRAG;
			extern const char * IP_PROTOCOL;
			extern const char * L4_PORT_DST;
			extern const char * L4_PORT_SRC;
			extern const char * IP_VER;
		}
	}

	namespace ruleThen {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
			extern const char * NO_OPERATION;
			extern const char * DROP;
		}
	}

	namespace policy {
		extern const char * CLASS_NAME;

		namespace attribute {
			extern const char * KEY;
		}
	}

	namespace virtualRouter{

		extern const char* CLASS_NAME;

		namespace attribute {
			extern const char* KEY;
			extern const char* NAME;
		}
	}

	namespace ipInterface{

		extern const char* CLASS_NAME;

		namespace attribute {
			extern const char* KEY;
			extern const char* ETHERNET_PORT;
			extern const char* IPV4_ADDR;
			extern const char* IPV6_ADDR;
			extern const char* PBF_INGRESS;

		}
	}
}

#endif /* ITHADM_CAA_INC_FIXS_ITH_NETCONFCONSTANTS_H_ */
