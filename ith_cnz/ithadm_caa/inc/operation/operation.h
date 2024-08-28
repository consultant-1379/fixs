//	********************************************************
//
//	 COPYRIGHT Ericsson 2015
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2015.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2015 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************

#ifndef ITHADM_CAA_INC_FIXS_ITH_OPERATION_H_
#define ITHADM_CAA_INC_FIXS_ITH_OPERATION_H_

#include <string>
#include <set>
#include <stdint.h>

#include "fixs_ith_programconstants.h"
#include "fixs_ith_snmpconstants.h"
#include "imm/imm.h"
#include "common/utility.h"
#include "ACS_APGCC_Util.H"

namespace operation
{

	/// Operation Identifiers
	enum identifier_t {
		INVALID_OP = -1,
		NOOP,
		START,  ///< Activate
		STOP,	///< Deactivate
		SHUTDOWN,	///< Terminate

		CS_SUBSCRIBE,
		CS_UNSUBSCRIBE,

		TRAPDS_SUBSCRIBE,
		TRAPDS_UNSUBSCRIBE,
		BNC_OBJECTS_INSTANTIATE,
		BNC_OBJECT_REMOVE,
		REGISTER_IMM_IMPLEMENTER,
		UNREGISTER_IMM_IMPLEMENTER,

		RELOAD_ALL_FROM_IMM,
		RELOAD_CONTEXT_FROM_IMM,
		RELOAD_CONTEXT_AFTER_COLD_START_TRAP,

		ADD_BFD_SESSIONS,
		ADD_BFD_SESSION_FOR_NEXTHOP,
		REMOVE_BFD_SESSIONS,
		REMOVE_BFD_SESSION_FOR_NEXTHOP,
		CHECK_CONFIG_CONSISTENCY,
		MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE,
		CEASE_ALARM,

		REGISTER_TO_BRF,
		UNREGISTER_FROM_BRF,
		CHECK_SFPPORT_CONSISTENCY,
		SET_PORT_ADMIN_STATE,
		PIM_ADAPTATION
	};

	// Operation result
	struct result
	{
		int errorCode;
		std::string errorMessage;

		result(): errorCode(fixs_ith::ERR_NO_ERRORS), errorMessage() {}

		void set(const int& errorValue, const std::string& errMsg)
		{
			errorCode = errorValue;
			errorMessage.assign(errMsg);
		}

		const char* getErrorMessage() const { return errorMessage.c_str(); }

		int getErrorCode() const { return errorCode; }

		void setErrorCode(const int& errCode) {  errorCode = errCode; }

		bool good() { return (fixs_ith::ERR_NO_ERRORS == errorCode); }

		bool fail() { return (fixs_ith::ERR_NO_ERRORS != errorCode); }
	};

	/** @brief	holds Bridge MO data
	 */
	struct bridgeInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;
		fixs_ith::SwitchBoardPlaneConstants plane;

		std::string bridgeAddress;
		std::string numberOfPorts;
		std::string numberOfVlans;
		std::set<std::string> ipv4SubnetVlan;
		std::string userLabel;

		uint16_t changeMask;

		bridgeInfo() : name(), moDN(), smxId(), plane(), bridgeAddress(), ipv4SubnetVlan(), userLabel(), changeMask(0U) {};

		bridgeInfo(const bridgeInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			bridgeAddress = rhs.bridgeAddress;
			numberOfPorts = rhs.numberOfPorts;
			numberOfVlans = rhs.numberOfVlans;
			ipv4SubnetVlan = rhs.ipv4SubnetVlan;
			userLabel = rhs.userLabel;
			changeMask = rhs.changeMask;
		}

		bridgeInfo& operator=(const bridgeInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			bridgeAddress = rhs.bridgeAddress;
			ipv4SubnetVlan = rhs.ipv4SubnetVlan;
			userLabel = rhs.userLabel;
			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getBridgeDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const fixs_ith::SwitchBoardPlaneConstants getPlane() const { return plane; };
		const char* getBridgeAddress() const { return bridgeAddress.c_str(); };
		const char* getnumberOfPorts() const { return numberOfPorts.c_str(); };
		const char* getnumberOfVlans() const { return numberOfVlans.c_str(); };
		const std::set<std::string> getSubnetVlanReference() const { return ipv4SubnetVlan; };

	};

	/** @brief	holds BridgePort MO data
	 */
	struct bridgePortInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;
		fixs_ith::SwitchBoardPlaneConstants plane;

		imm::admin_State adminstate;
		imm::oper_State operState; //Runtime
		std::string userLabel;
		uint16_t defaultVlanId;
		std::string portAddress;

		uint16_t changeMask;

		bridgePortInfo() : name(), moDN(),smxId(), plane(), adminstate(), operState(), userLabel(), defaultVlanId(), portAddress(), changeMask(0U) {};

		bridgePortInfo(const bridgePortInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			adminstate = rhs.adminstate;
			operState = rhs.operState;
			userLabel = rhs.userLabel;
			defaultVlanId = rhs.defaultVlanId;
			portAddress = rhs.portAddress;

			changeMask = rhs.changeMask;
		}

		bridgePortInfo& operator=(const bridgePortInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			adminstate = rhs.adminstate;
			operState = rhs.operState;
			userLabel = rhs.userLabel;
			defaultVlanId = rhs.defaultVlanId;
			portAddress = rhs.portAddress;

			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getBridgePortDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const fixs_ith::SwitchBoardPlaneConstants getPlane() const { return plane; };
		const imm::admin_State getAdminState() const { return adminstate; };
		const imm::admin_State getBridgePortState() const {return adminstate; };
		const uint16_t getDefaultVlanId() const {return defaultVlanId; };
		const char* getPortAddress() const { return portAddress.c_str(); };
		const imm::oper_State getOperState() const { return operState; };
		const uint16_t getChangeMask() const { return changeMask; };


	};

	/** @brief	holds EthernetPort MO data
	 */
	struct ethernetPortInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;
		fixs_ith::SwitchBoardPlaneConstants plane;

		std::string macAddress;
		std::string userLabel;
		int32_t operatingMode;
		uint16_t snmpIndex;
		uint32_t maxFrameSize;

		imm::EthAutoNegotiate autoNegotiate;
		imm::EthAutoNegotiationState autoNegotiationState;

		uint16_t changeMask;

		ethernetPortInfo() : name(), moDN(), smxId(), plane(), macAddress(), userLabel(), operatingMode(), snmpIndex(), maxFrameSize(), autoNegotiate(imm::AN_LOCKED), autoNegotiationState(imm::AN_STATE_UNKNOWN), changeMask(0U) {};

		ethernetPortInfo(const ethernetPortInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			macAddress = rhs.macAddress;
			userLabel = rhs.userLabel;
			operatingMode = rhs.operatingMode;
			snmpIndex = rhs.snmpIndex;
			maxFrameSize = rhs.maxFrameSize;
			autoNegotiate = rhs.autoNegotiate;
			autoNegotiationState = rhs.autoNegotiationState;
			changeMask = rhs.changeMask;

		}

		ethernetPortInfo& operator=(const ethernetPortInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			macAddress = rhs.macAddress;
			userLabel = rhs.userLabel;
			operatingMode = rhs.operatingMode;
			snmpIndex = rhs.snmpIndex;
			maxFrameSize = rhs.maxFrameSize;
			autoNegotiate = rhs.autoNegotiate;
			autoNegotiationState = rhs.autoNegotiationState;
			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getEthernetPortDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const fixs_ith::SwitchBoardPlaneConstants getPlane() const { return plane; };
		const char* getMacAddress() const {return macAddress.c_str();};
		const uint16_t getSnmpIndex() const {return snmpIndex; };
		const int32_t getOperatingMode() const {return operatingMode; };
		const uint32_t getMaxFrameSize() const {return maxFrameSize; };
		imm::EthAutoNegotiate getAutoNegotiate() const { return autoNegotiate; };
		imm::EthAutoNegotiationState getAutoNegotiationState() const { return autoNegotiationState; };
		bool isSFPport() const {return (snmpIndex >= ITH_SNMP_COMMUNITY_PIM_PORT_START && snmpIndex <= ITH_SNMP_COMMUNITY_PIM_PORT_END);};
		bool isAutoNegotiateEnabled() const { return (imm::AN_UNLOCKED == autoNegotiate); };
	};

	/** @brief	holds Aggregator MO data
	 */
	struct aggregatorInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;
		fixs_ith::SwitchBoardPlaneConstants plane;

		std::string userLabel;
		std::set<std::string> aggMember;
		imm::oper_State operState; //Runtime
		std::set<std::string> operAggMember; //Runtime
		uint32_t dataRate; //Runtime
		uint16_t snmpIndex;

		uint16_t changeMask;

		aggregatorInfo() : name(), moDN(), smxId(), plane(),userLabel(), aggMember(), operState(), operAggMember(), dataRate(), snmpIndex(), changeMask(0U) {};

		aggregatorInfo(const aggregatorInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			userLabel = rhs.userLabel;
			aggMember = rhs.aggMember;
			operState = rhs.operState;
			operAggMember = rhs.operAggMember;
			dataRate = rhs.dataRate;
			snmpIndex = rhs.snmpIndex;

			changeMask = rhs.changeMask;
		}

		aggregatorInfo& operator=(const aggregatorInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			userLabel = rhs.userLabel;
			aggMember = rhs.aggMember;
			operState = rhs.operState;
			operAggMember = rhs.operAggMember;
			dataRate = rhs.dataRate;
			snmpIndex = rhs.snmpIndex;

			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getAggregatorDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const fixs_ith::SwitchBoardPlaneConstants getPlane() const { return plane; };
		const std::set<std::string> getAdminAggMember() const { return aggMember; };
		const uint32_t getDataRate() const {return dataRate; };
		const uint16_t getSnmpIndex() const {return snmpIndex; };
		const imm::oper_State getOperState() const { return operState; };
		const std::set<std::string> getOperAggMember() const { return operAggMember; };

	};

	/** @brief	holds Vlan MO data
	 */
	struct vlanInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;
		fixs_ith::SwitchBoardPlaneConstants plane;

		std::string userLabel;
		uint16_t vId;
		std::set<std::string> taggedBridgePorts;
		std::set<std::string> untaggedBridgePorts;

		std::set<std::string> reservedBy;

		uint16_t changeMask;

		vlanInfo() : name(), moDN(), smxId(), plane(), userLabel(), vId(), taggedBridgePorts(), untaggedBridgePorts(), reservedBy(), changeMask(0U) {};

		vlanInfo(const vlanInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			userLabel = rhs.userLabel;
			vId = rhs.vId;
			taggedBridgePorts = rhs.taggedBridgePorts;
			untaggedBridgePorts = rhs.untaggedBridgePorts;
			reservedBy = rhs.reservedBy;

			changeMask = rhs.changeMask;
		}

		vlanInfo& operator=(const vlanInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			userLabel = rhs.userLabel;
			vId = rhs.vId;
			taggedBridgePorts = rhs.taggedBridgePorts;
			untaggedBridgePorts = rhs.untaggedBridgePorts;
			reservedBy = rhs.reservedBy;

			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getVlanDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const fixs_ith::SwitchBoardPlaneConstants getPlane() const { return plane; };
		const uint16_t getVlanId() const { return vId; };
		const std::set<std::string> getVlanTaggedPorts() const { return taggedBridgePorts; };
		const std::set<std::string> getVlanUntaggedPorts() const { return untaggedBridgePorts; };
		const std::set<std::string> getReservedBy() const { return reservedBy; };

	};

	/** @brief	holds SubnetVlan MO data
	 */
	struct subnetVlanInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;
		fixs_ith::SwitchBoardPlaneConstants plane;

		std::string ipv4Subnet;
		uint16_t vId;
		std::string userLabel;

		uint16_t changeMask;

		subnetVlanInfo() : name(), moDN(),smxId(), plane(), ipv4Subnet(), vId(), userLabel(), changeMask(0U) {};

		subnetVlanInfo(const subnetVlanInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			ipv4Subnet = rhs.ipv4Subnet;
			vId = rhs.vId;
			userLabel = rhs.userLabel;

			changeMask = rhs.changeMask;
		}

		subnetVlanInfo& operator=(const subnetVlanInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			plane = rhs.plane;

			ipv4Subnet = rhs.ipv4Subnet;
			vId = rhs.vId;
			userLabel = rhs.userLabel;

			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getSubnetVlanDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const fixs_ith::SwitchBoardPlaneConstants getPlane() const { return plane; };
		const char* getIpSubnet() const { return ipv4Subnet.c_str(); };
		const uint16_t getSubnetVlanId() const { return vId; };

	};

	/** @brief	holds Router MO data
	 */
	struct routerInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		routerInfo() : name(), moDN(),smxId() {};

		routerInfo(const routerInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
		}

		routerInfo& operator=(const routerInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
	};

	/** @brief	holds BFD Session MO data
	 */
	struct bfdSessionInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		std::string addrDst;
		std::string addrSrc;
		uint32_t detectionTime;
		uint32_t interval_tx;
		uint32_t localDiscriminator;
		uint32_t portSrc;
		uint32_t remoteDiscriminator;
		imm::BfdSessionDiagnCode diagCode;
		imm::BfdSessionOperState sessionState;

		bfdSessionInfo() : name(), moDN(), smxId(), addrDst(), addrSrc(), detectionTime(0U),
				interval_tx(0U), localDiscriminator(0U), portSrc(0U),remoteDiscriminator(0U),
				diagCode(imm::BFD_DC_NO_DIAG),sessionState(imm::BFD_OPERSTATE_ADMIN_DOWN){};

		bfdSessionInfo(const bfdSessionInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
			addrDst = rhs.addrDst;
			addrSrc = rhs.addrSrc;
			detectionTime = rhs.detectionTime;
			interval_tx = rhs.interval_tx;
			localDiscriminator = rhs.localDiscriminator;
			portSrc = rhs.portSrc;
			remoteDiscriminator = rhs.remoteDiscriminator;
			diagCode = rhs.diagCode;
			sessionState = rhs.sessionState;
		}

		bfdSessionInfo& operator=(const bfdSessionInfo& rhs)
		{
			if(this != &rhs)
			{
				name = rhs.name;
				moDN = rhs.moDN;
				smxId = rhs.smxId;
				addrDst = rhs.addrDst;
				addrSrc = rhs.addrSrc;
				detectionTime = rhs.detectionTime;
				interval_tx = rhs.interval_tx;
				localDiscriminator = rhs.localDiscriminator;
				portSrc = rhs.portSrc;
				remoteDiscriminator = rhs.remoteDiscriminator;
				diagCode = rhs.diagCode;
				sessionState = rhs.sessionState;

			}

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };

		const char * getAddrDst() const { return addrDst.c_str(); }
		const char * getAddrSrc() const { return addrSrc.c_str(); }
		const uint32_t getDetectionTime() const { return detectionTime; };
		const uint32_t getIntervalTx() const { return interval_tx; };
		const uint32_t getLocalDiscr() const { return localDiscriminator; };
		const uint32_t getRemoteDiscr() const { return remoteDiscriminator; };
		const uint32_t getPortSrc() const { return portSrc; };
		const imm::BfdSessionDiagnCode getDiagCode() const { return diagCode; };
		const imm::BfdSessionOperState getSessionState() const { return sessionState; };
	};

	/** @brief	holds BFD Profile MO data
	 */
	struct bfdProfileInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		uint8_t detectionMultiplier;
		uint32_t intervalTxMinDesired;
		uint32_t intervalRxMinRequired;
		std::set<std::string> reservedBy;

		uint16_t changeMask;

		bfdProfileInfo()
			: name(), moDN(),smxId(), detectionMultiplier(), intervalTxMinDesired(),
			  intervalRxMinRequired(), reservedBy(), changeMask(0U) {};

		bfdProfileInfo(const bfdProfileInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			detectionMultiplier = rhs.detectionMultiplier;
			intervalTxMinDesired = rhs.intervalTxMinDesired;
			intervalRxMinRequired = rhs.intervalRxMinRequired;
			reservedBy = rhs.reservedBy;

			changeMask = rhs.changeMask;
		}

		bfdProfileInfo& operator=(const bfdProfileInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			detectionMultiplier = rhs.detectionMultiplier;
			intervalTxMinDesired = rhs.intervalTxMinDesired;
			intervalRxMinRequired = rhs.intervalRxMinRequired;
			reservedBy = rhs.reservedBy;

			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const uint8_t getDetectionMultiplier() const { return detectionMultiplier; };
		const uint32_t getIntervalTx() const { return intervalTxMinDesired; };
		const uint32_t getIntervalRx() const { return intervalRxMinRequired; };
		std::set<std::string> getReservedBy() const { return reservedBy; };
	};

	/** @brief	holds Address Ipv4 MO data
	 */
	struct addressInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		std::string  address;

		uint16_t changeMask;




		addressInfo() : name(), moDN(),smxId(), address(), changeMask(0U) {};

		addressInfo(const addressInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			address = rhs.address;
			changeMask = rhs.changeMask;
		}

		addressInfo& operator=(const addressInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			address = rhs.address;
			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const char* getAddress() const { return address.c_str(); };
		const bool isVRRPInterfaceAddress() const { return common::utility::getParentRDNAttribute(moDN).compare(imm::vrrpInterface_attribute::RDN) == 0;};
	};

	/** @brief	holds Interface Ipv4 MO data
	 */
	struct interfaceInfo
	{
			struct aclInfo
			{
				std::string aclName;
				std::string routerName;
				uint16_t vlanId;

				aclInfo(): aclName(), routerName(), vlanId(0) {};

				aclInfo(const aclInfo& rhs)
				{
					aclName.assign(rhs.getAclName());
					routerName.assign(rhs.getRouterName());
					vlanId = rhs.getVlanId();
				}

				aclInfo& operator=(const aclInfo& rhs)
				{
					if(this != &rhs)
					{
						aclName.assign(rhs.getAclName());
						routerName.assign(rhs.getRouterName());
						vlanId = rhs.getVlanId();
					}

					return *this;
				}

				const char* getAclName() const { return aclName.c_str(); };
				const char* getRouterName() const { return routerName.c_str(); };
				uint16_t getVlanId() const { return vlanId; };

				void setAclName(const std::string& name) { aclName.assign(name); };
				void setRouterName(const std::string& name) { routerName.assign(name); };
				void setVlanId(const uint16_t id) { vlanId = id; };
			};

			std::string name;
			std::string moDN;
			std::string smxId;

			imm::admin_State adminstate;
			std::string bfdProfile;
			std::string aclIngress;

			std::string encapsulation;
			uint32_t mtu;
			imm::oper_State bfdStaticRoutes;
			bool loopback;

			imm::oper_State operationalState; //runtime

			uint16_t changeMask;

			//Additional attributes
			uint16_t encapsulationVid;

			aclInfo aclData;

			interfaceInfo() : name(), moDN(),smxId(), adminstate(imm::EMPTY_ADMIN_STATE), bfdProfile(), aclIngress(), encapsulation(), mtu(0U), bfdStaticRoutes(imm::EMPTY_OPER_STATE), loopback(false), operationalState(imm::EMPTY_OPER_STATE), changeMask(0U), encapsulationVid(0U), aclData() {};

			interfaceInfo(const interfaceInfo& rhs)
			{
				name = rhs.name;
				moDN = rhs.moDN;
				smxId = rhs.smxId;

				adminstate = rhs.adminstate;
				bfdProfile = rhs.bfdProfile;
				aclIngress = rhs.aclIngress;
				encapsulation = rhs.encapsulation;
				mtu = rhs.mtu;
				bfdStaticRoutes = rhs.bfdStaticRoutes;
				loopback = rhs.loopback;
				operationalState = rhs.operationalState;

				changeMask = rhs.changeMask;

				encapsulationVid = rhs.encapsulationVid;
				aclData = rhs.aclData;
			}

			interfaceInfo& operator=(const interfaceInfo& rhs)
			{
				name = rhs.name;
				moDN = rhs.moDN;
				smxId = rhs.smxId;

				adminstate = rhs.adminstate;
				bfdProfile = rhs.bfdProfile;
				aclIngress = rhs.aclIngress;
				encapsulation = rhs.encapsulation;
				mtu = rhs.mtu;
				bfdStaticRoutes = rhs.bfdStaticRoutes;
				loopback = rhs.loopback;
				operationalState = rhs.operationalState;

				changeMask = rhs.changeMask;

				encapsulationVid = rhs.encapsulationVid;
				aclData = rhs.aclData;

				return *this;
			}

			const char* getName() const { return name.c_str(); };
			const char* getDN() const { return moDN.c_str(); };
			const char* getSmxId() const { return smxId.c_str(); };
			const imm::admin_State  getAdmState() const { return adminstate; };
			const char*  getBfdProfile() const { return bfdProfile.c_str(); };

			const char* getAclIngress() const { return aclIngress.c_str(); };
			bool isAclIngressEmpty() const { return aclIngress.empty(); };

			const char*  getEncapsulation() const { return encapsulation.c_str(); };
			const uint32_t getMTU() const { return mtu; };
			const imm::oper_State getBfdStaticRoute() const { return bfdStaticRoutes; };
			const bool getLoopback() const {return loopback; };
			const imm::oper_State getOperationalState() const {return operationalState; };
			const uint16_t getEncapsulationVid() const {return encapsulationVid; };
	};

	/** @brief	holds RouteTable MO data
	 */
	struct routeTableInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		routeTableInfo() : name(), moDN(),smxId() {};

		routeTableInfo(const routeTableInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;
		}

		routeTableInfo& operator=(const routeTableInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
	};

	/** @brief	holds Dst MO data
	 */
	struct dstInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;


		std::string dst;

		uint16_t changeMask;

		dstInfo() : name(), moDN(),smxId(), dst(), changeMask(0U) {};

		dstInfo(const dstInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			dst = rhs.dst;

			changeMask = rhs.changeMask;
		}

		dstInfo& operator=(const dstInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			dst = rhs.dst;
			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const char* getDst() const { return dst.c_str(); };
	};

	/** @brief	holds NextHop MO data
	 */
	struct nextHopInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		std::string address;
		int16_t adminDistance;
		bool discard;
		bool bfdMonitoring;

		uint16_t changeMask;

		nextHopInfo() : name(), moDN(), smxId(), address(), adminDistance(0U), discard(false), bfdMonitoring(false), changeMask(0U) {};

		nextHopInfo(const nextHopInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			address = rhs.address;
			adminDistance = rhs.adminDistance;
			discard = rhs.discard;
			bfdMonitoring = rhs.bfdMonitoring;

			changeMask = rhs.changeMask;

		}

		nextHopInfo& operator=(const nextHopInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			address = rhs.address;
			adminDistance = rhs.adminDistance;
			discard = rhs.discard;
			bfdMonitoring = rhs.bfdMonitoring;

			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const char* getAddress() const { return address.c_str(); };
		const int16_t getAdminDistance() const { return adminDistance; };
		const bool getDiscard() const { return discard; };
		bool getBfdMonitoring() const { return bfdMonitoring; };
		bool hasAddress() const { return !address.empty(); };

	};

	/** @brief	holds VRRP Interface MO data
	 */
	struct vrrpInterfaceInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		int32_t vrIdentity;
		int32_t advertiseInterval;
		bool preemptMode;
		int32_t preemptHoldTime;
		std::set<std::string> reservedBy;

		std::string address_ipv4;

		uint16_t changeMask;

		vrrpInterfaceInfo ()
		: name(), moDN(), vrIdentity(), advertiseInterval(), preemptMode(),
			preemptHoldTime(), reservedBy(), address_ipv4(), changeMask(0U) {};

		vrrpInterfaceInfo (const vrrpInterfaceInfo & rhs)
		: name(rhs.name), moDN(rhs.moDN), vrIdentity(rhs.vrIdentity),
			advertiseInterval(rhs.advertiseInterval), preemptMode(rhs.preemptMode),
			preemptHoldTime(rhs.preemptHoldTime), reservedBy(rhs.reservedBy),
			address_ipv4(rhs.address_ipv4), changeMask(rhs.changeMask)
		{}

		vrrpInterfaceInfo & operator= (const vrrpInterfaceInfo & rhs)
		{
			if (this != &rhs) {
				name = rhs.name; moDN = rhs.moDN;

				vrIdentity = rhs.vrIdentity;
				advertiseInterval = rhs.advertiseInterval;
				preemptMode = rhs.preemptMode;
				preemptHoldTime = rhs.preemptHoldTime;
				reservedBy = rhs.reservedBy;
				address_ipv4 = rhs.address_ipv4;
				changeMask = rhs.changeMask;
			}

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		int32_t getVrIdentity() const { return vrIdentity; };
		int32_t getAdvertiseInterval() const { return advertiseInterval; };
		bool getPreemptMode() const { return preemptMode; };
		int32_t getPreemptHoldTime() const { return preemptHoldTime; };
		std::set<std::string> getReservedBy() const { return reservedBy; };
		std::string getAddressIPv4 () const { return address_ipv4; }
	};

	/** @brief	holds VRRP Session MO data
	 */
	struct vrrpSessionInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		imm::admin_State administrativeState;
		std::string vrrpv3InterfaceRef;
		imm::vrrpvPrioSession prioritizedSession;
		uint32_t priority;
		imm::vrrpSessionState vrrpState;

		vrrpInterfaceInfo interfaceInfo;

		uint16_t changeMask;

		vrrpSessionInfo()
		: name(), moDN(), smxId(), administrativeState(), vrrpv3InterfaceRef(),
			prioritizedSession(), priority(0U), vrrpState(), interfaceInfo(), changeMask(0U) {};

		vrrpSessionInfo(const vrrpSessionInfo & rhs)
		: name(rhs.name), moDN(rhs.moDN), smxId(rhs.smxId), administrativeState(rhs.administrativeState),
			vrrpv3InterfaceRef(rhs.vrrpv3InterfaceRef), prioritizedSession(rhs.prioritizedSession),
			priority(rhs.priority), vrrpState(rhs.vrrpState), interfaceInfo(rhs.interfaceInfo),
			changeMask(rhs.changeMask)
		{}

		vrrpSessionInfo & operator= (const vrrpSessionInfo & rhs)
		{
			if (this != &rhs) {
				name = rhs.name; moDN = rhs.moDN; smxId = rhs.smxId;

				administrativeState = rhs.administrativeState;
				vrrpv3InterfaceRef = rhs.vrrpv3InterfaceRef;
				prioritizedSession = rhs.prioritizedSession;
				priority = rhs.priority;
				vrrpState = rhs.vrrpState;
				interfaceInfo = rhs.interfaceInfo;
				changeMask = rhs.changeMask;
			}

			return *this;
		}

		void setVrrpInterfaceInfo(const vrrpInterfaceInfo& vrrpInterfaceInfo) {	interfaceInfo = vrrpInterfaceInfo; };
		const vrrpInterfaceInfo& getVrrpInterfaceInfo () const { return interfaceInfo; }

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const char* getVrrpInterfaceRef() const { return vrrpv3InterfaceRef.c_str(); };
		imm::admin_State getAdminState() const { return administrativeState; };
		imm::vrrpvPrioSession getPrioritizedSession() const { return prioritizedSession; };
		uint32_t getPriority() const { return priority; }
		imm::vrrpSessionState getVrrpState() const { return vrrpState; }

	};


	/** @brief	holds AclEntry MO data
	 */
	struct aclEntryInfo
	{
		std::string name;
		std::string moDN;
		std::string smxId;

		imm::aclAction action;
		std::set<std::string> addrDst;
		std::set<std::string> addrSrc;
		std::set<std::string> dscp;
		imm::aclFragmentType fragmentType;
		std::set<std::string> icmpType;
		std::set<std::string> ipProtocol;
		std::set<std::string> portDst;
		std::set<std::string> portSrc;
		uint16_t priority;

		uint16_t changeMask;

		aclEntryInfo() : name(), moDN(),smxId(), action(), addrDst(), addrSrc(), dscp(), fragmentType(),
				icmpType(), ipProtocol(), portDst(), portSrc(), priority(0U), changeMask(0U) {};

		aclEntryInfo(const aclEntryInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			action = rhs.action;
			addrDst = rhs.addrDst;
			addrSrc = rhs.addrSrc;
			dscp = rhs.dscp;
			fragmentType = rhs.fragmentType;
			icmpType = rhs.icmpType;
			ipProtocol = rhs.ipProtocol;
			portDst = rhs.portDst;
			portSrc = rhs.portSrc;
			priority = rhs.priority;

			changeMask = rhs.changeMask;
		}

		aclEntryInfo& operator=(const aclEntryInfo& rhs)
		{
			name = rhs.name;
			moDN = rhs.moDN;
			smxId = rhs.smxId;

			action = rhs.action;
			addrDst = rhs.addrDst;
			addrSrc = rhs.addrSrc;
			dscp = rhs.dscp;
			fragmentType = rhs.fragmentType;
			icmpType = rhs.icmpType;
			ipProtocol = rhs.ipProtocol;
			portDst = rhs.portDst;
			portSrc = rhs.portSrc;
			priority = rhs.priority;

			changeMask = rhs.changeMask;

			return *this;
		}

		const char* getName() const { return name.c_str(); };
		const char* getDN() const { return moDN.c_str(); };
		const char* getSmxId() const { return smxId.c_str(); };
		const imm::aclAction getAclAction() const { return action; };
		const std::set<std::string> getAddrDst() const { return addrDst; };
		const std::set<std::string> getAddrSrc() const { return addrSrc; };
		const std::set<std::string> getDscp() const { return dscp; };
		const imm::aclFragmentType  getFragmentType() const { return fragmentType; };
		const std::set<std::string> getIcmpType() const { return icmpType; };
		const std::set<std::string> getIpProtocol() const { return ipProtocol; };
		const std::set<std::string> getPortDst() const { return portDst; };
		const std::set<std::string> getPortSrc() const { return portSrc; };
		const uint16_t getPriority() const { return priority; }
	};

	/** @brief	holds AclEntry MO data
	 */
	struct aclIpV4Info
	{
			std::string name;
			std::string moDN;
			std::string smxId;

			std::set<std::string> reservedBy;
			uint16_t changeMask;

			aclIpV4Info() : name(), moDN(),smxId(), reservedBy(), changeMask(0U) {};

			aclIpV4Info(const aclIpV4Info& rhs)
			{
				name = rhs.name;
				moDN = rhs.moDN;
				smxId = rhs.smxId;

				reservedBy = rhs.reservedBy;
				changeMask = rhs.changeMask;
			}

			aclIpV4Info& operator=(const aclIpV4Info& rhs)
			{
				if(this != &rhs)
				{
					name = rhs.name;
					moDN = rhs.moDN;
					smxId = rhs.smxId;

					reservedBy = rhs.reservedBy;
					changeMask = rhs.changeMask;
				}

				return *this;
			}

			const char* getName() const { return name.c_str(); };
			const char* getDN() const { return moDN.c_str(); };
			const char* getSmxId() const { return smxId.c_str(); };

			std::set<std::string> getReservedBy() const { return reservedBy; };
	};

}


#endif /* ITHADM_CAA_INC_FIXS_ITH_OPERATION_H_ */
