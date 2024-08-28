/*
 * utility.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: eanform
 */

#include "common/utility.h"
#include "imm/imm.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_csreader.h"
#include <ACS_CS_API.h>
#include <ACS_CS_API_Set.h>


#include "ACS_APGCC_Util.H"

#include <fstream>
#include <boost/regex.hpp>
#include <ACS_APGCC_CommonLib.h>
#include <ACS_APGCC_CLibTypes.h>

namespace common
{

	namespace utility
	{
		namespace parserTag
		{
			const char minus = '-';
			const char plus = '+';
			const char comma = ',';
			const char equal = '=';
			const char underscore = '_';
			const char dot = '.';
			const char colon = ':';
			const char singleQuote = '\'';
			const char slash = '/';
		};

		bool getRDNValue(const std::string& fullDN, const char* RDN, std::string& RDNValue)
		{
			std::string subpartOfDN;

			bool result = getDNbyTag(fullDN, RDN, subpartOfDN);

			if(result)
			{
				result = getLastFieldValue(subpartOfDN, RDNValue);
			}

			return result;
		}

		bool getDNbyTag(const std::string& fullDN, const char* tagOfDN, std::string& outDN)
		{
			bool result = false;
			size_t tagStartPos;
			outDN.clear();
			tagStartPos = fullDN.find(tagOfDN);

			// Check if the tag is present
			if( std::string::npos != tagStartPos )
			{
				// get the DN
				outDN = fullDN.substr(tagStartPos);
				result= true;
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ParserTag:<%s> not found into DN:<%s>", tagOfDN, fullDN.c_str() );
			}

			return result;
		}

		bool getLastFieldValue(const std::string& stringToParser, std::string& value)
		{
			bool result = false;

			value.clear();

			// Get the last value from DN e.g: class1Id=xyz,class2Id=lmk,....
			// Split the field in RDN and Value e.g. : class1Id xyz
			size_t equalPos = stringToParser.find_first_of(parserTag::equal);
			size_t commaPos = stringToParser.find_first_of(parserTag::comma);

			// Check if some error happens
			if( (std::string::npos != equalPos) )
			{
				// check for a single field case
				if( std::string::npos == commaPos )
					value = stringToParser.substr(equalPos + 1);
				else
					value = stringToParser.substr(equalPos + 1, (commaPos - equalPos - 1) );

				result = true;
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ParserTag:<%c> not found into the string:<%s>", parserTag::equal, stringToParser.c_str() );
			}

			return result;
		}

		std::string getRDNValue(const std::string& objectDN, const std::string& rdName)
		{
			std::string rdnValue;

			// Get a rdn value from DN e.g: class1Id=xyz,class2Id=lmk,class3Id=abc
			// Split the field in RDN and Value e.g. : class1Id xyz
			size_t rdnPos = objectDN.find(rdName);

			if(std::string::npos == rdnPos) return rdnValue;

			size_t equalPos = objectDN.find(parserTag::equal, rdnPos);

			if( std::string::npos == equalPos ) return rdnValue;

			size_t commaPos = objectDN.find(parserTag::comma, rdnPos);

			// check for a single field case
			if( std::string::npos == commaPos )
				rdnValue.assign(objectDN.substr(equalPos + 1));
			else
				rdnValue.assign(objectDN.substr(equalPos + 1, (commaPos - equalPos - 1) ));

			return rdnValue;
		}

		std::string getIdValueFromRdn(const std::string & rdn)
		{
			std::string rdnId = rdn;

			size_t index = rdnId.find_first_of(parserTag::comma);
			if (index != std::string::npos)
				rdnId = rdnId.substr(0, index);

			index = rdnId.find_first_of(parserTag::equal);
			if (index != std::string::npos)
				rdnId = rdnId.substr(index+1);

			return rdnId;
		}

		std::string getParentDN(const std::string & dn)
		{
			return stringSplit::after(dn, parserTag::comma);
		}

		std::string getRDNAttribute(const std::string & dn)
		{
			return stringSplit::before(dn, parserTag::equal);
		}

		std::string getParentRDNAttribute(const std::string & dn)
		{
			return getRDNAttribute(getParentDN(dn));
		}

		/*	Utils to retrieve the SmxId from DN	*/
		std::string getSmxIdFromBridgeDN (const std::string & dn)
		{
			return stringSplit::beforeLast(getIdValueFromRdn(dn), parserTag::underscore);
		}

		std::string getSmxIdFromEthernetPortDN (const std::string & dn)
		{
			return getSmxIdFromBridgeDN(stringSplit::beforeLast(getIdValueFromRdn(dn), parserTag::colon));
		}

		std::string getSmxIdFromAggregatorDN (const std::string & dn)
		{
			return getSmxIdFromEthernetPortDN(dn);
		}

		std::string getSmxIdFromVlanDN (const std::string & dn)
		{
			return getSmxIdFromBridgeDN(getParentDN(dn));
		}

		std::string getSmxIdFromSubnetVlanDN (const std::string & dn)
		{
			return getSmxIdFromBridgeDN(getParentDN(dn));
		}

		std::string getSmxIdFromBridgePortDN (const std::string & dn)
		{
			return getSmxIdFromBridgeDN(getParentDN(dn));
		}

		std::string getSmxIdFromRouterDN (const std::string & dn)
		{
			return stringSplit::beforeLast(getIdValueFromRdn(dn), (parserTag::colon) );
		}

		std::string getSmxIdFromInterfaceDN (const std::string & dn)
		{
			return getSmxIdFromRouterDN(getParentDN(dn));
		}

		std::string getSmxIdFromAddressDN (const std::string & dn)
		{
			return getSmxIdFromRouterDN(getRouterNameFromAddressDN(dn));
		}

		std::string getSmxIdFromDstDN (const std::string & dn)
		{
			return getSmxIdFromRouterDN(getRouterNameFromDstDN(dn));
		}

		std::string getSmxIdFromNextHopDN (const std::string & dn)
		{
			return getSmxIdFromRouterDN(getRouterNameFromChildDN(dn));
		}

		std::string getSmxIdFromBfdProfileDN (const std::string & dn)
		{
			return getSmxIdFromRouterDN(getRouterNameFromChildDN(dn));
		}

		std::string getSmxIdFromBfdSessionDN(const std::string & dn)
		{
			return getSmxIdFromRouterDN(getRouterNameFromChildDN(dn));
		}

		std::string getSmxIdFromAclDN (const std::string & dn)
		{
			return getSmxIdFromRouterDN(getRouterNameFromChildDN(dn));
		}

		std::string getSmxIdFromAclEntryDN (const std::string & dn)
		{
			return getSmxIdFromRouterDN(getRouterNameFromChildDN(dn));
		}

		std::string getSmxIdFromVrrpv3SessionDN (const std::string & dn)
		{
			return getSmxIdFromRouterDN(getRouterNameFromChildDN(dn));
		}

		/*	Utils to retrieve the Plane from DN	 */
		fixs_ith::SwitchBoardPlaneConstants getPlaneFromBridgeDN(const std::string & dn)
		{
			return planeToInt(stringSplit::afterLast(getIdValueFromRdn(dn), (parserTag::underscore)));
		}

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromEthernetPortDN(const std::string & dn)
		{
			std::string splitRdnValue = stringSplit::beforeLast(getIdValueFromRdn(dn), (parserTag::colon));

			return planeToInt(stringSplit::afterLast(splitRdnValue, (parserTag::underscore)));
		}

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromAggregatorDN(const std::string & dn)
		{
			return getPlaneFromEthernetPortDN(dn);
		}

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromVlanDN (const std::string & dn)
		{
			return getPlaneFromBridgeDN(getParentDN(dn));
		}

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromSubnetVlanDN (const std::string & dn)
		{
			return getPlaneFromBridgeDN(getParentDN(dn));
		}

		fixs_ith::SwitchBoardPlaneConstants getPlaneFromBridgePortDN (const std::string & dn)
		{
			return getPlaneFromBridgeDN(getParentDN(dn));
		}


		std::string planeToString(const fixs_ith::SwitchBoardPlaneConstants& plane)
		{
			return ((fixs_ith::CONTROL_PLANE == plane)? imm::plane_type::CONTROL: imm::plane_type::TRANSPORT);
		}

		fixs_ith::SwitchBoardPlaneConstants planeToInt(const std::string& plane)
		{
			return ((imm::plane_type::CONTROL.compare(plane) == 0 )? fixs_ith::CONTROL_PLANE: fixs_ith::TRANSPORT_PLANE);
		}


		/*
		 *	Common utils
		 */
		std::string buildPortName(const std::string& smxId, const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& interfaceName)
		{
			return (smxId + parserTag::underscore + planeToString(plane) + parserTag::colon + interfaceName);
		}

		std::string buildLoopbackInterfaceName(const std::string& routerName)
		{
			return "lo." + common::utility::getRouterIdFromRouterName(routerName);
		}

		std::string getBridgePortNameFromPortName(const std::string& portName)
		{
			return stringSplit::afterLast(portName, parserTag::colon);
		}

		std::string getRouterIdFromRouterName(const std::string& name)
		{
			return stringSplit::afterLast(name, parserTag::colon);
		}

		std::string getRouterNameFromInterfaceDN(const std::string& dn)
		{
			return getIdValueFromRdn(getParentDN(dn));
		}

		std::string getRouterNameFromRdnValue (const std::string & rdn_value, char separator) {
			// Find the separator
			std::string::size_type separator_pos = rdn_value.find_last_of(separator);

			return ((separator_pos == std::string::npos) ? rdn_value : rdn_value.substr(separator_pos + 1));
		}

		std::string getRouterNameFromChildDN(const std::string& dn)
		{
			std::string separator = imm::router_attribute::RDN + parserTag::equal;
			std::string splitRdnValue = stringSplit::afterLast(dn,separator);

			return stringSplit::before(splitRdnValue, parserTag::comma);
		}

		std::string getRouterNameFromAddressDN(const std::string& dn)
		{
			return getRouterNameFromChildDN(dn);
		}

		std::string getRouterNameFromDstDN(const std::string& dn)
		{
			return getRouterNameFromChildDN(dn);
		}

		std::string getRouterNameFromNextHopDN(const std::string& dn)
		{
			return getRouterNameFromChildDN(dn);
		}

		std::string getRouterNameFromVrrpSessionDN(const std::string& dn)
		{
			return getRouterNameFromChildDN(dn);
		}

		std::string getRouterNameFromBfdSessionIPv4DN(const std::string& dn)
		{
			return getIdValueFromRdn(getParentDN(dn));
		}

		std::string getInterfaceNameFromVrrpSessionDN(const std::string& dn)
		{
			return getIdValueFromRdn(getParentDN(dn));
		}

		std::string getVrrpSessionNameFromDN(const std::string& dn)
		{
			return getIdValueFromRdn(dn);
		}

		std::string getInterfaceNameFromAddressDN(const std::string& dn)
		{
			return getIdValueFromRdn(getParentDN(dn));
		}

		std::string getDstNameFromNextHopDN(const std::string& dn)
		{
			return getIdValueFromRdn(getParentDN(dn));
		}

		std::string getVrrpInstanceNameFromDN(const std::string& dn)
		{
			std::string vrrpInstanceName;
			getRDNValue(dn, imm::vrrpInterface_attribute::RDN.c_str(), vrrpInstanceName);
			return vrrpInstanceName;
		}

		int getEthernetPortPositionFromRdnValue (
				std::string & magazine,
				std::string & slot,
				std::string & port_name,
				const std::string & rdn_value) {

			/*
			 *           1
			 * 0123456789012345678
			 * 0.0.0.0_25_HS:BP_22
			 * ethernetPortId=0.0.0.0_0_HS:BP_22,AxeInfrastructureTransportMtransportMId=1
			 */


			std::string::size_type underscore_first_pos = rdn_value.find_first_of(parserTag::underscore);
			if ((underscore_first_pos == std::string::npos) || !underscore_first_pos) {
				// ERROR: Malformed ethernet port RDN value: First underscore not found
				return -1;
			}

			std::string::size_type underscore_second_pos = rdn_value.find_first_of(parserTag::underscore, underscore_first_pos + 1);
			if ((underscore_second_pos == std::string::npos) || !underscore_second_pos) {
				// ERROR: Malformed ethernet port RDN value: Second underscore not found or there is only one underscore separator
				return -1;
			}

			std::string::size_type colon_pos = rdn_value.find_first_of(parserTag::colon, underscore_second_pos + 1);
			if ((colon_pos == std::string::npos) || !colon_pos) {
				// ERROR: Malformed ethernet port RDN value: Colon not found or it is not the last separator
				return -1;
			}

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "rdn_value == <%s>, underscore_first_pos == <%u>, underscore_second_pos == <%u>, colon_pos == <%u>",
					rdn_value.c_str(), underscore_first_pos, underscore_second_pos, colon_pos);

			magazine = rdn_value.substr(0, underscore_first_pos);
			slot = rdn_value.substr(underscore_first_pos + 1, (underscore_second_pos - underscore_first_pos - 1));
			port_name = rdn_value.substr(colon_pos + 1);

			return ((magazine.empty() || slot.empty() || port_name.empty()) ? -1 : 0);
		}

		std::string getBfdProfileNameFromDN(const std::string& dn)
		{
			std::string bfdProfileName;
			getRDNValue(dn, imm::bfdProfile_attribute::RDN.c_str(), bfdProfileName);
			return bfdProfileName;
		}

		std::string getAclIpv4NameFromDN(const std::string& dn)
		{
			std::string alcIpv4Name;
			getRDNValue(dn, imm::aclIpv4_attribute::RDN.c_str(), alcIpv4Name);
			return alcIpv4Name;
		}

		std::string getAclIpv4NameFromChildDN(const std::string& dn)
		{
			std::string separator = imm::aclIpv4_attribute::RDN + parserTag::equal;
			std::string splitRdnValue = stringSplit::afterLast(dn,separator);

			return stringSplit::before(splitRdnValue, parserTag::comma);
		}

		std::string getNextHopNameFromDN(const std::string& dn)
		{
			return getRDNValue(dn, imm::nextHop_attribute::RDN);
		}

		bool getRouterDn(const std::string& objectDN, std::string& routerDN)
		{
			return getDNbyTag(objectDN, imm::router_attribute::RDN.c_str(), routerDN);
		}

		bool isValidVlanIdRange(uint16_t vId){return (vId <= imm::vlanId_range::UPPER_BOUND)? true : false; };

bool isSwitchBoardCMX() {
	//This function returns true if the Switchboard is CMX
	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue nodeArchitecture;
	ACS_CS_API_NS::CS_API_Result cs_call_result =
			ACS_CS_API_NetworkElement::getNodeArchitecture(nodeArchitecture);

	if (cs_call_result != ACS_CS_API_NS::Result_Success) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'getNodeArchitecture' failed: cannot get the NodeArchitecture from CS: cs_call_result == %d",
				cs_call_result);
	}

	return (nodeArchitecture == ACS_CS_API_CommonBasedArchitecture::SCX ?
			true : false);
}

bool isSwitchBoardSMX() {
	//This function returns true if the Switchboard is SMX
	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue nodeArchitecture;
	ACS_CS_API_NS::CS_API_Result cs_call_result =
			ACS_CS_API_NetworkElement::getNodeArchitecture(nodeArchitecture);

	if (cs_call_result != ACS_CS_API_NS::Result_Success) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'getNodeArchitecture' failed: cannot get the NodeArchitecture from CS: cs_call_result == %d",
				cs_call_result);
	}

	return (nodeArchitecture == ACS_CS_API_CommonBasedArchitecture::SMX ?
			true : false);

}

		int getNodeNumber()
		{
			std::string node_id_path = "/etc/cluster/nodes/this/id";
			fstream nodeFile;
			nodeFile.open(node_id_path.c_str(), ios::in);
			if (!nodeFile)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Unable to read APG node id from /etc/cluster/nodes/this/id ");
				return -1;
			}

			char val[256];
			char *c = val;
			nodeFile.getline(c, 256);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"APG node id: %s", c);
			nodeFile.close();
			return ::atoi(c);
		}

int createDir(std::string dirName) {
	/*HW55782 - changed chdir to stat */
	struct stat st;
	if (stat(dirName.c_str(), &st) == -1) { //ERROR
		if (int resMk = ::mkdir(dirName.c_str(), 0777)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Directory creation failed==%s",
					dirName.c_str());
			return -1;
		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Directory %s successfully created",
					dirName.c_str());
			if (int resChmod = ::chmod(dirName.c_str(), 0777)) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Failed to do chmod on directory %s", dirName.c_str());
				return -1;
			} else {
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Chmod to direcotry %s is OK",
						dirName.c_str());
			}
		}
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " Directory %s already created ",
				dirName.c_str());

		if (int resChmod = ::chmod(dirName.c_str(), 0777)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					" Unable to change permissions on directory %s",
					dirName.c_str());
			return -1;
		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Able to change permissions on direcotry %s",
					dirName.c_str());
		}
	}
	return 0;
}

void renameCopiedContFile (std::string  contFileName, std::string ipNumber)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Entered renameCopiedContFile" );
        std::string tftpPathAPG = "/data/apz/data/boot/cmx/cmx_logs";
        std::string clusterIP = "192.168.169.";
        std::string ipFiltered(ipNumber);
        if (ipFiltered.compare("")!=0)
        {
                size_t foundDot;
                foundDot = ipFiltered.find_first_of(".");
                while (foundDot != std::string::npos){
                        ipFiltered[foundDot] = '_';
                        foundDot = ipFiltered.find_first_of(".",foundDot+1 );
                }
        }

        std::string tftpPathAPGOld = tftpPathAPG + "/cmx_" + ipFiltered + "/";
        tftpPathAPGOld.append(contFileName);
        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "File name before rename: %s",tftpPathAPGOld.c_str());

        char byte[512];
        memset(byte, 0xFF, sizeof(byte));

        time_t longTime;
        time(&longTime);  // Get the current time

	struct tm* today = localtime(&longTime);
        int year = 1900 + today->tm_year;
        sprintf(byte, "%u%02u%02u_%02u%02u%02u",year, (today->tm_mon + 1) ,today->tm_mday,today->tm_hour, today->tm_min,today->tm_sec );
        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"FIXS_CMXH_SNMPManager::renameCopiedContFile byte :%s", byte);

        std::string tftpPathAPGNew = tftpPathAPG + "/cmx_" + ipFiltered + "/";
        tftpPathAPGNew.append(contFileName);
        size_t pos;
        pos = tftpPathAPGNew.find(".tar");
        tftpPathAPGNew = tftpPathAPGNew.substr (0,pos);
        tftpPathAPGNew.append("_");
        tftpPathAPGNew.append(byte);
        tftpPathAPGNew.append(".tar.gnupg.gz");
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"File name after rename: %s",tftpPathAPGNew.c_str());

        int retValue = rename(tftpPathAPGOld.c_str(), tftpPathAPGNew.c_str());

        if( retValue == 0)
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"FIXS_CMXH_SNMPManager::renameCopiedContFile File successfully renamed");
        else
                FIXS_ITH_LOG(LOG_LEVEL_ERROR,"FIXS_CMXH_SNMPManager::renameCopiedContFile File rename failed with : %d",strerror(errno));
        }


		int build_sb_key_from_magazine_and_slot(std::string & key, const std::string & mag_str, const int32_t& slot_pos)
		{
			// build the key combining MAGAZINE string and SLOT position
			char mag_slot_str[32]= {0};
			::snprintf(mag_slot_str, FIXS_ITH_ARRAY_SIZE(mag_slot_str), "%s%c%d", mag_str.c_str(), parserTag::underscore, slot_pos);

			// return the key in the output parameter
			key.assign(mag_slot_str);
			return 0;
		}

		int uint32_to_ip_format (char (& value_ip) [16], uint32_t value)
		{
			in_addr addr;
			addr.s_addr = value;

			::inet_ntop(AF_INET, &addr, value_ip, FIXS_ITH_ARRAY_SIZE(value_ip));

			return 0;
		}

		std::string  build_sb_key_from_magazine_and_slot(uint32_t & magazine, const int32_t& slot_pos)
		{
			std::string key;
			char magazine_str [16] = {0};

			uint32_to_ip_format(magazine_str, magazine);
			build_sb_key_from_magazine_and_slot(key,magazine_str, slot_pos);

			return key;
		}

		int get_magazine_and_slot_from_sb_key (std::string & mag_str, int32_t & slot_pos, const std::string & key) {
			std::string::size_type pos = key.find_first_of(parserTag::underscore);
			if (pos == std::string::npos) // ERROR: Underscore character not found into the key string
				return -1;

			mag_str = key.substr(0, pos);
			slot_pos = atoi(key.substr(pos + 1).c_str());

			return 0;
		}

		bool isValidIpv4Subnet(const std::string& ipv4Subnet)
		{
			char ipinput[INET_ADDRSTRLEN] = {0};
			strncpy(ipinput, stringSplit::before(ipv4Subnet, parserTag::slash).c_str(), INET_ADDRSTRLEN);
			uint8_t mask_len = atoi(stringSplit::after(ipv4Subnet, parserTag::slash).c_str());
			uint32_t ipa = 0;

			inet_pton(AF_INET, ipinput, &ipa);
			ipa = ntohl(ipa);

			uint32_t comp = INADDR_BROADCAST >> mask_len;

			// TR HX22060 BEGIN
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "ipa == %d mask_len == %d", ipa, mask_len);
			if (mask_len > 32) return false;
			return ((comp & ipa) == 0) || ((comp & ipa) == ipa);
			// TR HX22060 END
		}

		bool isValidIpv4Address(const std::string& ipv4Address)
		{
			char ipinput[INET_ADDRSTRLEN] = {0};
			strncpy(ipinput, stringSplit::before(ipv4Address, parserTag::slash).c_str(), INET_ADDRSTRLEN);
			uint8_t mask_len = atoi(stringSplit::after(ipv4Address, parserTag::slash).c_str());
			uint32_t ipa = 0;

			inet_pton(AF_INET, ipinput, &ipa);
			ipa = ntohl(ipa);

			uint32_t comp = INADDR_BROADCAST >> mask_len;

			uint32_t last_bits = comp & ipa;

			return (last_bits != 0 && last_bits != comp);

		}

		bool compareIpv4Subnets(const std::string& ipv4Address_a, const std::string& ipv4Address_b)
		{
			char ipinput_a[INET_ADDRSTRLEN] = {0};
			strncpy(ipinput_a, stringSplit::before(ipv4Address_a, parserTag::slash).c_str(), INET_ADDRSTRLEN);
			uint8_t mask_len_a = atoi(stringSplit::after(ipv4Address_a, parserTag::slash).c_str());
			uint32_t ipa = 0;

			inet_pton(AF_INET, ipinput_a, &ipa);
			ipa = ntohl(ipa);

			char ipinput_b[INET_ADDRSTRLEN] = {0};
			strncpy(ipinput_b, stringSplit::before(ipv4Address_b, parserTag::slash).c_str(), INET_ADDRSTRLEN);
			uint8_t mask_len_b = atoi(stringSplit::after(ipv4Address_b, parserTag::slash).c_str());
			uint32_t ipb = 0;

			inet_pton(AF_INET, ipinput_b, &ipb);
			ipb = ntohl(ipb);

			uint32_t shorter_mask_len = std::min(mask_len_a, mask_len_b);
			uint32_t mask = INADDR_BROADCAST << (32 - shorter_mask_len);

			uint32_t subnet_a = ipa & mask;
			uint32_t subnet_b = ipb & mask;

			return subnet_a == subnet_b;

		}

		bool compareIpv4Addresses(const std::string& ipv4Address_a, const std::string& ipv4Address_b)
		{
			char ipinput_a[INET_ADDRSTRLEN] = {0};
			strncpy(ipinput_a, stringSplit::before(ipv4Address_a, parserTag::slash).c_str(), INET_ADDRSTRLEN);
			uint32_t ipa = 0;

			inet_pton(AF_INET, ipinput_a, &ipa);
			ipa = ntohl(ipa);

			char ipinput_b[INET_ADDRSTRLEN] = {0};
			strncpy(ipinput_b, stringSplit::before(ipv4Address_b, parserTag::slash).c_str(), INET_ADDRSTRLEN);
			uint32_t ipb = 0;

			inet_pton(AF_INET, ipinput_b, &ipb);
			ipb = ntohl(ipb);

			return ipa == ipb;

		}

		bool isValidObjectId(const std::string& objId)
		{
			boost::regex expression(imm::regular_expression::VALID_OBJECT_ID);
			return boost::regex_match(objId, expression);
		}


		uint16_t getVlanIdFromFile(const std::string& filePath)
		{
			std::string fileContent,line;
			std::ifstream infile(filePath.c_str());

			if (infile)
			{
				while (std::getline(infile, line))
				{
					fileContent += line;
				}
				infile.close();
			}

			return atoi(fileContent.c_str());
		}


		bool splitMagazineAndSlotFromSmxKey(const std::string& smxId, std::string& magazine, std::string& slot)
		{
			int result = false;

			// Smx Key e.g: 0.0.0.0_25

			std::string::size_type underscore_pos = smxId.find_first_of(parserTag::underscore);
			if( std::string::npos != underscore_pos )
			{
				magazine.assign(smxId.substr(0U, underscore_pos));
				slot.assign(smxId.substr(underscore_pos + 1));
				result = (!magazine.empty() && !slot.empty() );
			}

			return result;
		}


		int build_bridge_name(char (& bridge_name)[IMM_BRIDGE_NAME_SIZE_MAX], int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane){

			char magazine_str[16] = {0};

			if(int call_result = fixs_ith_csreader::uint32_to_ip_format(magazine_str, magazine, IMM_MAGAZINE_ADDRESS_SEPARATOR))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Error building MAGAZINE string in IP format. magazine == 0x%08X, call_result == %d", magazine, call_result);
				return -1;
			}

			snprintf(bridge_name, IMM_BRIDGE_NAME_SIZE_MAX, "%s_%d_%s",magazine_str,slot, (common::utility::planeToString(plane)).c_str());

			return 0;
		}

		int build_port_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane){


			char bridge_name[IMM_BRIDGE_NAME_SIZE_MAX] = {0};

			if(int call_result = build_bridge_name(bridge_name, slot, magazine, plane))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Error building bridge name  magazine == 0x%08X slot == %d plane == %d, call_result == %d", magazine, slot , plane, call_result);
				return -1;
			}
			::snprintf(rdn, IMM_RDN_PATH_SIZE_MAX, "%s=%s:%s", imm::ethernetPort_attribute::RDN.c_str(), bridge_name, if_name);

			return 0;
		}

		int build_bridge_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane){

			char bridge_name[IMM_BRIDGE_NAME_SIZE_MAX] = {0};

			if(int call_result = build_bridge_name(bridge_name, slot, magazine, plane))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Error building bridge name  magazine == 0x%08X slot == %d plane == %d, call_result == %d", magazine, call_result);
				return -1;
			}

			::snprintf(rdn, IMM_RDN_PATH_SIZE_MAX, "%s=%s", imm::bridge_attribute::RDN.c_str(), bridge_name);

			return 0;
		}

		int build_aggregator_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane)
		{
			char bridge_name[IMM_BRIDGE_NAME_SIZE_MAX] = {0};

			if(int call_result = build_bridge_name(bridge_name, slot, magazine, plane))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Error building bridge name  magazine == 0x%08X slot == %d plane == %d, call_result == %d", magazine, slot , plane, call_result);
				return -1;
			}
			::snprintf(rdn, IMM_RDN_PATH_SIZE_MAX, "%s=%s:%s", imm::aggregator_attribute::RDN.c_str(), bridge_name, if_name);

			return 0;
		}



		int build_ethport_dn(char (& ethport_dn)[IMM_DN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane)
		{
			if(!if_name || !(*if_name)) return fixs_ith::ERR_BUILD_OBJECT_DN;

			char ethportRdn[IMM_RDN_PATH_SIZE_MAX] = {0};

			if(build_port_rdn(ethportRdn, if_name, slot, magazine, plane))  return fixs_ith::ERR_BUILD_OBJECT_DN;

			::snprintf(ethport_dn, IMM_DN_PATH_SIZE_MAX, "%s,%s", ethportRdn, imm::mom_dn::ROOT_MOC_DN);

			return fixs_ith::ERR_NO_ERRORS;
		}


		int build_bridgePort_dn(char (& bridgePort_dn)[IMM_DN_PATH_SIZE_MAX], const char * port_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane)
		{
			if(!port_name || !(*port_name)) return fixs_ith::ERR_BUILD_OBJECT_DN;

			char bridgeRdn[IMM_RDN_PATH_SIZE_MAX] = {0};

			if(build_bridge_rdn(bridgeRdn, slot, magazine, plane))  return fixs_ith::ERR_BUILD_OBJECT_DN;

			::snprintf(bridgePort_dn, IMM_RDN_PATH_SIZE_MAX, "%s=%s,%s,%s", imm::bridgePort_attribute::RDN.c_str(), port_name,bridgeRdn,imm::mom_dn::ROOT_MOC_DN);

			return fixs_ith::ERR_NO_ERRORS;
		}

		int build_vlan_dn(char (& vlna_dn)[IMM_DN_PATH_SIZE_MAX], const char * name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane)
		{
			if(!name || !(*name)) return fixs_ith::ERR_BUILD_OBJECT_DN;

			char bridgeRdn[IMM_RDN_PATH_SIZE_MAX] = {0};

			if(build_bridge_rdn(bridgeRdn, slot, magazine, plane))  return fixs_ith::ERR_BUILD_OBJECT_DN;

			::snprintf(vlna_dn, IMM_DN_PATH_SIZE_MAX, "%s=%s,%s,%s", imm::vlan_attribute::RDN.c_str(), name, bridgeRdn, imm::mom_dn::ROOT_MOC_DN);

			return fixs_ith::ERR_NO_ERRORS;
		}

		// TR HX92638 BEGIN
		std::string slot_to_string(int32_t slot)
		{
			std::stringstream ss;
			ss << slot;
			return ss.str();
		}
		// TR HX92638 END
		
		// TR HY37073 BEGIN

		bool is_hwtype_gep5()
		{
			bool hwtype_gep5 = false;
			ACS_APGCC_CommonLib commonlib_obj;
			ACS_APGCC_HWINFO hwInfo;
			ACS_APGCC_HWINFO_RESULT hwInfoResult;

			commonlib_obj.GetHwInfo( &hwInfo, &hwInfoResult, ACS_APGCC_GET_HWVERSION );
			if(hwInfoResult.hwVersionResult == ACS_APGCC_HWINFO_SUCCESS){
				switch(hwInfo.hwVersion){
				case ACS_APGCC_HWVER_GEP1:
				case ACS_APGCC_HWVER_GEP2:
					break;
				case ACS_APGCC_HWVER_GEP5:
					FIXS_ITH_LOG(LOG_LEVEL_WARN, "GetHwInfo() - Hardware Version GEP5");
					hwtype_gep5=true;
					break;
				case ACS_APGCC_HWVER_GEP7:
				case ACS_APGCC_HWVER_VM:
					break;
				default:
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: GetHwInfo() - Undefined Hardware Version");
				}
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: GetHwInfo() - Undefined Hardware Version");
			}
			return hwtype_gep5;
		}
		
		// TR HY37073 END
		
		namespace stringSplit
		{
			//STRING MANIPULATION UTILITIES //////////////////////////////////////////////////////////

			std::string before(const std::string& input_string, const char pattern)
			{
				return before(input_string, std::string(1, pattern));
			}

			std::string before(const std::string& input_string, const std::string& pattern)
			{

				std::string out(input_string);
				std::string::size_type pattern_position = input_string.find(pattern);

				if ( std::string::npos != pattern_position)
				{
					//Pattern found
					out = input_string.substr(0, pattern_position);
				}

				return out;
			}

			//-----------------------------------------------------------------------------
			//-----------------------------------------------------------------------------

			std::string beforeLast(const std::string& str, const char pattern)
			{
				return beforeLast(str, std::string(1, pattern));
			}

			std::string beforeLast(const std::string& input_string, const std::string& pattern)
			{
				std::string out(input_string);
				std::string::size_type next_pattern_position = std::string::npos, last_pattern_position = std::string::npos, start_position = 0;

				do
				{
					next_pattern_position = input_string.find(pattern, start_position);

					if( std::string::npos != next_pattern_position )
					{
						last_pattern_position = next_pattern_position;
						start_position = next_pattern_position + pattern.length();
					}
				} while( std::string::npos != next_pattern_position );


				if( std::string::npos != last_pattern_position )
				{
					//Pattern found
					out = input_string.substr(0, last_pattern_position);
				}

				return out;
			}

			//-----------------------------------------------------------------------------
			//-----------------------------------------------------------------------------
			std::string after(const std::string& input_string, const char pattern)
			{
				return after(input_string, std::string(1, pattern));
			}

			std::string after(const std::string& input_string, const std::string& pattern)
			{
				std::string out;
				std::string::size_type pattern_position = input_string.find(pattern);

				if ( std::string::npos != pattern_position )
				{
					//Pattern found
					out = input_string.substr(pattern_position + pattern.length(), std::string::npos);
				}

				return out;
			}


			//-----------------------------------------------------------------------------
			//-----------------------------------------------------------------------------
			std::string afterLast(const std::string& input_string, const char pattern)
			{
				return afterLast(input_string, std::string(1, pattern));
			}

			std::string afterLast(const std::string& input_string, const std::string& pattern)
			{

				std::string out;
				std::string::size_type next_pattern_position = std::string::npos, last_pattern_position = std::string::npos, start_position = 0;

				do
				{
					next_pattern_position = input_string.find(pattern, start_position);

					if( std::string::npos != next_pattern_position )
					{
						last_pattern_position = next_pattern_position;
						start_position = next_pattern_position + pattern.length();
					}
				} while( std::string::npos != next_pattern_position );

				if( std::string::npos != last_pattern_position )
				{
					//Pattern found
					out = input_string.substr(last_pattern_position + pattern.length(), std::string::npos);
				}

				return out;

			}


		} //namespace string
	} //namespace utility


}
