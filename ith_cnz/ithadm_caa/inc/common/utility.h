/*
 * utility.h
 *
 *  Created on: Mar 1, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_COMMON_UTILITY_H_
#define ITHADM_CAA_INC_COMMON_UTILITY_H_

#include "fixs_ith_programconstants.h"
#include "imm/imm.h"

#include "acs_apgcc_omhandler.h"

#include <string>
#include <vector>
#include "stdint.h"



namespace common
{
	const uint16_t MS_TO_MICROSEC = 1000U;
	const uint32_t MAX_NETWORK_PREFIX = 32U;

	const std::string PATCH_FOLDER_NAME = "/cluster/storage/system/config/fixs_ith_patch";

	namespace utility
	{
		namespace parserTag
		{
			extern const char minus;
			extern const char plus;
			extern const char comma;
			extern const char equal;
			extern const char underscore;
			extern const char dot;
			extern const char colon;
			extern const char singleQuote;
			extern const char slash;
		};

		inline const char* boolToString(bool b)
		{
			return b ? "TRUE" : "FALSE";
		}

		/** @brief
		 *
		 *  This method extracts the value of a RDN from a DN
		 *
		 *  @param fullDN : the DN of the object
		 *
		 *  @param RDN : the RDN to get
		 *
		 *  @param RDNValue : the field value
		 *
		 *  @return true on success, false otherwise
		 *
		 *  @remarks Remarks
		 */
		bool getRDNValue(const std::string& fullDN, const char* RDN, std::string& RDNValue);

		/** @brief	getDNbyTag
		 *
		 *  This method gets the DN of a object by its RDN
		 *
		 *  @param fullDN : A full DN
		 *
		 *  @param tagOfDN : The RDN
		 *
		 *  @param outDN :  The DN of the object
		 *
		 *  @return true on success, otherwise false
		 *
		 *  @remarks Remarks
		 */
		bool getDNbyTag(const std::string& fullDN, const char* tagOfDN, std::string& outDN);

		/** @brief	getLastFieldValue
		 *
		 *  This method extracts the last field value from a DN
		 *
		 *  @param stringToParser : the DN of the object
		 *
		 *  @param value : the field value
		 *
		 *  @return true on success, false otherwise
		 *
		 *  @remarks Remarks
		 */
		bool getLastFieldValue(const std::string& stringToParser, std::string& value);

		/** @brief	getRDNValue
		 *
		 *  This method extracts the a rdn value from a DN
		 *
		 *  @param objectDN : the DN of the object
		 *
		 *  @param rdName : the RD name
		 *
		 *  @return field value
		 *
		 *  @remarks Remarks
		 */
		std::string getRDNValue(const std::string& objectDN, const std::string& rdName);

		std::string getIdValueFromRdn(const std::string & rdn);

		std::string getParentDN(const std::string & dn);

		std::string getRDNAttribute(const std::string & dn);

		std::string getParentRDNAttribute(const std::string & dn);


		std::string getSmxIdFromBridgeDN (const std::string & dn);

		std::string getSmxIdFromEthernetPortDN (const std::string & dn);

		std::string getSmxIdFromAggregatorDN (const std::string & dn);

		std::string getSmxIdFromBridgePortDN (const std::string & dn);

		std::string getSmxIdFromVlanDN (const std::string & dn);

		std::string getSmxIdFromSubnetVlanDN (const std::string & dn);

		std::string getSmxIdFromRouterDN (const std::string & dn);

		std::string getSmxIdFromInterfaceDN (const std::string & dn);

		std::string getSmxIdFromAddressDN (const std::string & dn);

		std::string getSmxIdFromDstDN (const std::string & dn);

		std::string getSmxIdFromNextHopDN (const std::string & dn);

		std::string getSmxIdFromBfdProfileDN (const std::string & dn);

		std::string getSmxIdFromBfdSessionDN(const std::string & dn);

		std::string getSmxIdFromAclDN (const std::string & dn);

		std::string getSmxIdFromAclEntryDN (const std::string & dn);

		std::string getSmxIdFromVrrpv3SessionDN (const std::string & dn);

		std::string planeToString(const fixs_ith::SwitchBoardPlaneConstants& plane);

		fixs_ith::SwitchBoardPlaneConstants planeToInt(const std::string& plane);


		fixs_ith::SwitchBoardPlaneConstants getPlaneFromBridgeDN(const std::string & dn);

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromEthernetPortDN(const std::string & dn);

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromAggregatorDN(const std::string & dn);

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromVlanDN (const std::string & dn);

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromSubnetVlanDN (const std::string & dn);

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromBridgePortDN (const std::string & dn);


		std::string buildPortName(const std::string& smxId, const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& interfaceName);

		std::string buildLoopbackInterfaceName(const std::string& routerName);

		std::string getBridgePortNameFromPortName(const std::string& portName);

		std::string getRouterIdFromRouterName(const std::string& name);

		std::string getRouterNameFromRdnValue (const std::string & rdn_value, char separator = parserTag::colon);

		std::string getRouterNameFromChildDN(const std::string& dn);

		std::string getRouterNameFromInterfaceDN(const std::string & dn);

		std::string getRouterNameFromAddressDN(const std::string& dn);

		std::string getRouterNameFromDstDN(const std::string& dn);

		std::string getRouterNameFromNextHopDN(const std::string& dn);

		std::string getRouterNameFromVrrpSessionDN(const std::string& dn);

		std::string getRouterNameFromBfdSessionIPv4DN(const std::string& dn);

		std::string getInterfaceNameFromVrrpSessionDN(const std::string& dn);

		std::string getVrrpSessionNameFromDN(const std::string& dn);

		std::string getInterfaceNameFromAddressDN(const std::string& dn);

		std::string getDstNameFromNextHopDN(const std::string& dn);

		std::string getVrrpInstanceNameFromDN(const std::string& dn);

//		std::string getEthernetPortNameFromDN (const std::string & dn);

		int getEthernetPortPositionFromRdnValue (
				std::string & magazine,
				std::string & slot,
				std::string & port_name,
				const std::string & rdn_value);

		std::string getBfdProfileNameFromDN(const std::string& dn);

		std::string getAclIpv4NameFromDN(const std::string& dn);

		std::string getAclIpv4NameFromChildDN(const std::string& dn);

		std::string getNextHopNameFromDN(const std::string& dn);

		bool getRouterDn(const std::string& objectDN, std::string& routerDN);

		bool isValidVlanIdRange(uint16_t vId);

		bool isValidIpv4Subnet(const std::string& ipv4Subnet);

		bool isValidIpv4Address(const std::string& ipv4Address);

		bool compareIpv4Subnets(const std::string& ipv4Address_a, const std::string& ipv4Address_b);

		bool compareIpv4Addresses(const std::string& ipv4Address_a, const std::string& ipv4Address_b);

		bool isValidObjectId(const std::string& objId);

		bool isSwitchBoardCMX();

		bool isSwitchBoardSMX();

		int getNodeNumber();

		int createDir(std::string dirName);

		void renameCopiedContFile (std::string  contFileName, std::string ipNumber);

		int build_sb_key_from_magazine_and_slot(std::string & key, const std::string & mag_str,  const int32_t& slot_pos);

		std::string  build_sb_key_from_magazine_and_slot(uint32_t & magazine, const int32_t& slot_pos);

		int get_magazine_and_slot_from_sb_key (std::string & mag_str, int32_t & slot_pos, const std::string & key);


		int uint32_to_ip_format (char (& value_ip) [16], uint32_t value);


		uint16_t getVlanIdFromFile(const std::string& filePath);


		bool splitMagazineAndSlotFromSmxKey(const std::string& smxId, std::string& magazine, std::string& slot);


		int build_bridge_name(char (& bridge_name)[IMM_BRIDGE_NAME_SIZE_MAX], int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
		int build_bridge_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
		int build_port_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
		int build_aggregator_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
		int build_ethport_dn(char (& ethport_dn)[IMM_DN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
		int build_bridgePort_dn(char (& bridgePort_dn)[IMM_DN_PATH_SIZE_MAX], const char * port_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
		int build_vlan_dn(char (& vlna_dn)[IMM_DN_PATH_SIZE_MAX], const char * name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);

		std::string slot_to_string(int string); // TR_HX92638 
                bool is_hwtype_gep5(); //TR_HY37073 
		
		namespace stringSplit
		{
			// Description:
			//    Get the string that is before 'pattern'.
			// Parameters:
			//    str            Source string
			//    pattern        Pattern string
			//
			// Return value:
			//    Returns the string that is before 'pattern', or
			//    the whole string if 'pattern' cannot be found.
			//
			// Additional information:
			//    -
			std::string before(const std::string& str, const char pattern);
			std::string before(const std::string& str, const std::string& pattern);


			// Description:
			//    Get the string that is before the last occurrences of 'pattern'.
			// Parameters:
			//    str            Source string
			//    pattern        Pattern string
			// Return value:
			//    Returns the string that is before the last occurence if 'pattern',
			//    or the source string if 'pattern' cannot be found.
			// Additional information:
			//    -
			std::string beforeLast(const std::string& str, const char pattern);
			std::string beforeLast(const std::string& str, const std::string& pattern);


			// Description:
			//    Get the string that is after 'pattern'.
			// Parameters:
			//    str            Source string
			//    pattern        Pattern string
			//
			// Return value:
			//    Returns the string that is after 'pattern', or
			//    an empty string if 'pattern' cannot be found.
			//
			// Additional information:
			//    -
			std::string after(const std::string& str, const char pattern);
			std::string after(const std::string& str, const std::string& pattern);

			// Description:
			//    Get the string that is after the last occurances of 'pattern'.
			// Parameters:
			//    str            Source string
			//    pattern        Pattern string
			//
			// Return value:
			//    Returns the string that is after the last occurances if 'pattern',
			//    or an empty string if 'pattern' cannot be found.
			//
			// Additional information:
			//    -
			std::string afterLast(const std::string& str, const char pattern);
			std::string afterLast(const std::string& str, const std::string& pattern);
		};
	}
}

#endif /* ITHADM_CAA_INC_COMMON_UTILITY_H_ */
