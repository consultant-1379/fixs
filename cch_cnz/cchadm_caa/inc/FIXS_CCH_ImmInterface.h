/*
 * FIXS_CCH_ImmInterface.h
 *
 *  Created on: Mar 28, 2012
 *      Author: eanform
 */

#ifndef FIXS_CCH_IMMINTERFACE_H_
#define FIXS_CCH_IMMINTERFACE_H_

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <bits/stl_map.h>
#include <list>

#include "FIXS_CCH_SoftwareMap.h"
#include "FIXS_CCH_BladeSwManagement.h"
#include "FIXS_CCH_Util.h"
#include "FIXS_CCH_SNMPManager.h"

#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"

using namespace CCH_Util;
namespace IMM_Interface
{
	const char * const SCXFBN = "SCXB";
	const char * const IPTFBN = "IPTB";
	const char * const EPB1FBN = "EPB1";
	const char * const CETB1FBN = "CETB1";
	const char * const CMXFBN = "CMXB";
	const char * const EVOETFBN = "EVOET";
	const char * const IPLBFBN = "IPLB";
	const char * const SMXFBN = "SMXB";

	const char * const IN_USE = "IN USE";
	const char * const NOT_USED = "NOT USED";

	//General
	void getEnvironmentType(EnvironmentType &env);
	int getContainersFromDisk ();
	void getEnvironmentTypeDirect(EnvironmentType &env);

	//Other Blade
	bool getBoardDataInfo(const char* dn_blade, std::string* strMagazine, std::string* strSlot, std::string* boardMasterIP, std::string* boardPassiveIP, std::string* boardMasterSlot, bool* isPassiveDeg);
	bool GetBoardIps(const std::string &shelfparent, std::string *strEthA0, std::string *strEthB0, std::string *strEthA25, std::string *strEthB25);
	bool GetScxbIps(std::string shelfparent, std::string &strEthA, std::string &strEthB, int slot);
	bool getOtherBladeFBN(std::string dn, int &fbn);
	bool getOtherBladeSlot(std::string dn, int &slot);
	bool getShelfMagazine(std::string dn, std::string &magazine);
	bool getDnMagazine (std::string magazine,std::string &dn);
	bool getOtherBladeIPs(std::string dn, std::string &ipA, std::string &ipB);
	bool getDnBoard (unsigned long uMagazine, unsigned short uSlot, std::string &dn);
        bool getOtherBladeSysNum(std::string dn, int &sysnum);
        bool getOtherBoardInCluster(unsigned long uMagazine,unsigned short uSlot,int sysnum,unsigned short &iSlot);
	bool getOtherBoardSlot(unsigned long umagazine,unsigned short uslot,unsigned short &islot);
	//SW Repository
	void printSoftwareRepository();

	//SW Upgrade
	bool setSwDataFromModel(std::string blade_dn, std::string dn, SWData &data);
	bool getOtherBladeCurrLoadModule(std::string dn, std::string &value);
	bool modifyAllInventoryAttributes(std::string dn, SWData &data);
	void defineSwInventoryObjectDN(unsigned long magazine, unsigned short slot, std::string &dn);
	bool getNeighborScxIp(std::string dn, int slot, std::string &neighborSCXIPA, std::string &neighborSCXIPB);
	bool getNeighborSmxIp(std::string dn, int slot, std::string &neighborSMXIPA, std::string &neighborSMXIPB);
	bool getReportProgressCurrSoftware(std::string dn, std::string &value);

	//IPT Upgrade
	bool modifyAllLMInventoryAttributes(std::string dn, BoardSWData &data);
	bool setBoardSwDataFromModel(std::string blade_dn, std::string dn, BoardSWData &data);

	//PFM Upgrade
	bool readUpgradeStatus (unsigned long magazine, unsigned short slot, PFMData &data);
	bool changePFMState(std::string dn, std::string attribute, int status);


	//IPMI  Upgrade
	bool getOtherBladeHardwareInfoDn(std::string dn, std::string& otherBladeHWinfoDN);
	bool getIpmiState(std::string dn, int &status);
	bool changeIpmiState(std::string dn, int status);
	bool readIpmiStatus (unsigned long magazine, unsigned short slot, int &upgradeStatus, int &upgradeResult, std::string &dn);
	bool readUpgradeStatus (unsigned long magazine, unsigned short slot, IpmiData &data);
	bool changeIpmiResult(std::string dn, int result);
	bool setIpmiUpgradeTime(std::string dn);

	//BladeSwManagement
	bool createActionResultStruct();
	bool setBladeSwmReferenceStruct();
	bool setSwMStructState(int status);
	bool setSwMStructResultInfo(int value);
	bool setSwMStructResult(int value);
	bool setSwMStructTime();
	bool getSwMStructState(int &value);

	//ApSwManagement
	bool createApSwObject(std::string package,int isDefault,int forFBN,std::string swPath, std::string product,int swVerType=DEFAULT_SW_VER_TYPE);

	//CpSwManagement
	bool createCpSwObject(std::string package,int isDefault,int forFBN,std::string swPath, std::string product);

	//CpSwManagement ApSwManagement
	bool getPathSwPackage(std::string dn, std::string &value);
	bool setDefaultPackage(std::string dn, int value);
	bool createSwManagedActionResultObject(std::string parent, std::string package, std::string &id);
	bool setSwManagedActionReferenceStruct(std::string dn, std::string value);
	bool setUsedPackage(std::string container, std::string value);
	bool getUsedPackageValue(std::string dn, std::vector<std::string> &value);
	bool setNotUsedPackage(std::string container, std::string value);
	bool setEmptyUsedPackage(std::string package);
	bool setManagedSwStructState(std::string dn, int status);
	bool setManagedSwStructResultInfo(std::string dn, int value);
	bool setManagedSwStructResult(std::string dn, int value);
	bool setManagedSwStructTime(std::string dn);
	bool getSwManagedActionReferenceStruct(std::string dn, std::string &dn_struct);
	bool updateUsedPackage(unsigned long magazine, unsigned short slot, std::string old_container, std::string new_container);
	bool setFaultyPackage(std::string dn, int value);
	bool getUpgTypeValue(std::string dn, int &value);

	//BladeSwPersistance
	bool createBladeSwPersistance(unsigned long magazine, unsigned short slot);
	bool deleteBladeSwPersistance(unsigned long magazine, unsigned short slot);
	std::string getDnBladePersistance(unsigned long magazine, unsigned short slot);

	//Report Progress
	bool createReportProgressObject(std::string parent, std::string magazine, std::string slot, std::string &id);
	int checkReportProgressObject(std::string mag, std::string slot, std::string container, CCH_Util::upgradeActionType actType);
	int removeReportProgressObject(std::string mag, std::string slot, std::string container);
	bool getReportProgressObject(std::string mag, std::string slot, std::string &dn_rp);

	//Brf
	int checkBrmPersistentDataOwnerObject();
	int createBrmPersistentDataOwnerObject();
	bool deleteBrmPersistentDataOwnerObject();

	void getImmBladeSWObjects(std::vector<std::string> p_dnList,std::vector<std::string> & immCxpObject);

};//namespace

#endif /* FIXS_CCH_IMMINTERFACE_H_ */
