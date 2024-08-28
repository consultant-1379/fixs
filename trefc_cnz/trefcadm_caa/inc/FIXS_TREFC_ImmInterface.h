/*
 * FIXS_TREFC_ImmInterface.h
 *
 *  Created on: Mar 23, 2012
 *      Author: estevol
 */

#ifndef FIXS_TREFC_IMMINTERFACE_H_
#define FIXS_TREFC_IMMINTERFACE_H_

#include "ACS_APGCC_OiHandler.h"
#include "ACS_CC_Types.h"
#include "ACS_APGCC_Util.H"
#include "acs_apgcc_omhandler.h"
#include "FIXS_TREFC_IMM_Util.h"
#include "FIXS_TREFC_Util.h"
#include "FIXS_TREFC_OI_TimeService.h"
#include "ACS_CS_API.h"


using namespace TREFC_Util;

namespace ImmInterface {

	int getAPZShelfAddress(std::string &address);
	int getAPZShelfAddress(uint32_t &address);
	bool setServiceStatus(IMM_Util::TimeServiceStatus servStatus);
	bool getServiceStatus(int &status);
	bool getAdminStatus(int &adminState);
	bool getNwTimeServerName(std::string &name);
	bool getSCXAtrefIP(std::string &scxIP);
	bool getSCXAtrefNetMask(std::string &scxMask);
	bool getSCXAtrefGatewayIP(std::string &scxGIP);
	bool getSCXBtrefIP(std::string &scxIP);
	bool getSCXBtrefNetMask(std::string &scxMask);
	bool getSCXBtrefGatewayIP(std::string &scxGIP);
	int  getTimeReferencesConfigList(std::map<std::string,std::string> &tRefList);
	/*******setting*************/
	bool setSwitchServiceRefType(IMM_Util::ExternalReferenceType SxServRefType);
	bool setSwitchServiceType(IMM_Util::TimeServiceType SxServType);
	/*********************************/

//	int getNwTimeServerIP(std::string &ntsIP);
//	ACS_CC_ReturnType setServiceType(IMM_Util::TimeServiceType type);
	bool getServiceType(int &type, bool isSMX);
	bool getReferenceType(int &type, bool isSMX);
	int getAPZSwitchMagazineSlot (unsigned char &shelf, bool &slot0, bool &slot25,bool isSMX);
	int getTimeReferencesList(std::map<std::string,std::string> &tRefList, bool isSMX);
	bool getAptEthInPort(uint32_t magazine, unsigned short slot, int &port_out);
	bool getAptEthInPort(std::string magazine, unsigned short slot, int &port_out);
	int getExternalIpAddressConfiguration (ExtIpSwitch &extIpSwitch0, ExtIpSwitch &extIpSwitch25, bool isSMX);
	int getSWITCHBoardAddress(vector<SWITCHAPTIPAddress> &switchAPTVector,vector<SWITCHAPZIPAddress> &switchAPZVector,bool isSMX);
	int getEnvironment(EnvironmentType &env);
	bool createSwitchTimeServiceClasses(bool isSMX);
	bool deleteSwitchTimeServiceClasses();
	bool configureNetworkConfiguration(int slot,std::string IP,std::string NetMask,std::string Gateway);
	bool setServiceAdmState(IMM_Util::TimeServiceAdm servStatus);
	bool setServiceResultState(IMM_Util::ChangeAdmStateResultType result);
	int deleteScxTimeServiceClasses();
//	int getAPZShelfDN(std::string &shelfAPZ_dn);
//	bool isAPZMagazine(std::string SCXBoard_dn);
}


#endif /* FIXS_TREFC_IMMINTERFACE_H_ */
