//## begin module%1.10%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.10%.codegen_version

//## begin module%4C25AE1802BD.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%4C25AE1802BD.cm

//## begin module%4C25AE1802BD.cp preserve=no
//	*****************************************************************************
//
//	.NAME
//	    FIXS_CCH_FSMDataStorage
//	.LIBRARY 3C++
//	.PAGENAME FIXS_CCH
//	.HEADER  FIXS_CCH
//
//	.COPYRIGHT
//	 COPYRIGHT Ericsson 2010.
//	 All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2009.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2009 or in
//	accordance with the terms and conditions stipulated in the
//	agreement/contract under which the program(s) have been
//	supplied.
//
//	.DESCRIPTION
//	   See below
//
//	*****************************************************************************
//## end module%4C25AE1802BD.cp

//## Module: FIXS_CCH_FSMDataStorage%4C25AE1802BD; Pseudo Package body
//## Source file: T:\fixs\cch_cnz\cch_caa\src\FIXS_CCH_FSMDataStorage.cpp

//## begin module%4C25AE1802BD.additionalIncludes preserve=no
//## end module%4C25AE1802BD.additionalIncludes

//## begin module%4C25AE1802BD.includes preserve=yes
#include <iostream> // debugging
#include <algorithm>
#include <sstream>
#include <string.h>
#include <map>
#include <vector>
#include <iostream>
//#include <tchar.h>
#include "ace/OS.h"

#include <sys/types.h>
#include <sys/stat.h>
#include "FIXS_CCH_DiskHandler.h"
//## end module%4C25AE1802BD.includes

// FIXS_CCH_FSMDataStorage
#include "FIXS_CCH_FSMDataStorage.h"
//## begin module%4C25AE1802BD.additionalDeclarations preserve=yes

namespace {

ACE_thread_mutex_t s_cs;
const int MAX_RETRIES_ON_FSM_STOP = 12;


const char * const SOFTWARE_UPG_SECTIONNAME = "SW";
const char * const FIRMWARE_UPG_SECTIONNAME = "FW";
const char * const IPMI_UPG_SECTIONNAME = "IPMI";
const int MAX_RETRY = 5;

}

namespace {

struct Lock
{
	Lock() {

		ACE_OS::thread_mutex_lock(&s_cs);
	};

	~Lock() {
		// if( std::uncaught_exception() ) std::cout << "leave Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_unlock(&s_cs);
	};
};

}


//## end module%4C25AE1802BD.additionalDeclarations


// Class FIXS_CCH_FSMDataStorage 

//## begin FIXS_CCH_FSMDataStorage::instance%4C25C5DB0094.attr preserve=no  private: static FIXS_CCH_FSMDataStorage* {UA} 0
FIXS_CCH_FSMDataStorage* FIXS_CCH_FSMDataStorage::s_instance = 0;
//## end FIXS_CCH_FSMDataStorage::instance%4C25C5DB0094.attr

FIXS_CCH_FSMDataStorage::FIXS_CCH_FSMDataStorage()
//## begin FIXS_CCH_FSMDataStorage::FIXS_CCH_FSMDataStorage%4C25AE1802BD_const.hasinit preserve=no
//## end FIXS_CCH_FSMDataStorage::FIXS_CCH_FSMDataStorage%4C25AE1802BD_const.hasinit
//## begin FIXS_CCH_FSMDataStorage::FIXS_CCH_FSMDataStorage%4C25AE1802BD_const.initialization preserve=yes
//## end FIXS_CCH_FSMDataStorage::FIXS_CCH_FSMDataStorage%4C25AE1802BD_const.initialization
{
	//## begin FIXS_CCH_FSMDataStorage::FIXS_CCH_FSMDataStorage%4C25AE1802BD_const.body preserve=yes
	//## end FIXS_CCH_FSMDataStorage::FIXS_CCH_FSMDataStorage%4C25AE1802BD_const.body
}



//## Other Operations (implementation)
FIXS_CCH_FSMDataStorage * FIXS_CCH_FSMDataStorage::getInstance ()
{
	//## begin FIXS_CCH_FSMDataStorage::getInstance%4C25C63B038F.body preserve=yes
	Lock lock;
	if (s_instance == 0)
	{
		s_instance = new (std::nothrow) FIXS_CCH_FSMDataStorage();
		//s_instance->createFolder(FIXS_CCH_DiskHandler::getConfigurationFolder());
	}
	return s_instance;
	//## end FIXS_CCH_FSMDataStorage::getInstance%4C25C63B038F.body
}

bool FIXS_CCH_FSMDataStorage::createFolder (std::string folder)
{
	UNUSED(folder);
	return true;
}

bool FIXS_CCH_FSMDataStorage::write (std::string section, std::string key, std::string value)
{
	UNUSED(section);
	UNUSED(key);
	UNUSED(value);
	return true;
}

bool FIXS_CCH_FSMDataStorage::read (std::string section, std::string key, std::string &value)
{
	UNUSED(section);
	UNUSED(key);
	UNUSED(value);
	return true;
}

bool FIXS_CCH_FSMDataStorage::prepareForWriting ()
{
	return false;
}

bool FIXS_CCH_FSMDataStorage::commitChanges ()
{
	return false;

}

bool FIXS_CCH_FSMDataStorage::saveUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::SWData data)
{
	UNUSED(magazine);
	UNUSED(slot);
	UNUSED(data);
	return true;
}

bool FIXS_CCH_FSMDataStorage::saveUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::PFMData data)
{
	UNUSED(magazine);
	UNUSED(slot);
	UNUSED(data);
	return true;
}

bool FIXS_CCH_FSMDataStorage::saveUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::IpmiData data)
{
	UNUSED(magazine);
	UNUSED(slot);
	UNUSED(data);
	return true;

	//## end FIXS_CCH_FSMDataStorage::saveUpgradeStatus%4C285D9F022C.body
}

bool FIXS_CCH_FSMDataStorage::readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::SWData &data)
{
	UNUSED(magazine);
	UNUSED(slot);
	UNUSED(data);
	return true;

	//## end FIXS_CCH_FSMDataStorage::readUpgradeStatus%4C28C39C0343.body
}

bool FIXS_CCH_FSMDataStorage::readUpgradeStatus (std::string dn, unsigned short slot, CCH_Util::PFMData &data)
{

	Lock lock;
	bool result = false;

	//get shelf
	std::string dn_shelf("");
	IMM_Util::getDnParent(dn,dn_shelf);
	char* dn_pfm= (char*)malloc(200);
	memset(dn_pfm,0,200);

	//dn pfm UPPER
	if (slot == 0)
	{
		sprintf(dn_pfm,"%s,%s",IMM_Util::RDN_LOWER_POWERFANMODULE,dn_shelf.c_str());
		data.module = CCH_Util::LOWER;
		data.dn_pfm = dn_pfm;
	}
	else if (slot == 25)
	{
		sprintf(dn_pfm,"%s,%s",IMM_Util::RDN_UPPER_POWERFANMODULE,dn_shelf.c_str());
		data.module = CCH_Util::UPPER;
		data.dn_pfm = dn_pfm;
	}


	int status = -1;
	int state = -1;

	//get fan status
	if (IMM_Util::getImmAttributeInt(dn_pfm,IMM_Util::ATT_FANMODULE_STATUS,status))
	{
		state = -1;
		if (status == IMM_Util::FW_IDLE) state = 0;
		else if (status == IMM_Util::FW_ONGOING) state = 1;

		data.upgradeFanStatus = static_cast<CCH_Util::PFMStatus>(state); //set upgradeStatus
		result = true;
	}

	//get power status
	if (IMM_Util::getImmAttributeInt(dn_pfm,IMM_Util::ATT_POWERMODULE_STATUS,status))
	{
		state = -1;
		if (status == IMM_Util::FW_IDLE) state = 0;
		else if (status == IMM_Util::FW_ONGOING) state = 1;

		data.upgradePowerStatus = static_cast<CCH_Util::PFMStatus>(state); //set upgradeStatus
		result = true;
	}

	free(dn_pfm);
	return result;

}

bool FIXS_CCH_FSMDataStorage::readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::IpmiData &data)
{
	UNUSED(magazine);
	UNUSED(slot);
	UNUSED(data);
	return true;
}



bool FIXS_CCH_FSMDataStorage::saveUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::BoardSWData data)
{
	UNUSED(magazine);
	UNUSED(slot);
	UNUSED(data);
	return true;

}

bool FIXS_CCH_FSMDataStorage::readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::BoardSWData &data)
{
	UNUSED(magazine);
	UNUSED(slot);
	UNUSED(data);
	return true;
}

void FIXS_CCH_FSMDataStorage::getSections (std::vector<std::string> &names)
{
	UNUSED(names);
}


bool FIXS_CCH_FSMDataStorage::readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::FwData &data)
{
	bool result = true;

	bool foundInstances = false;

	std::string dn = IMM_Interface::getDnBladePersistance(magazine,slot);

	std::vector<std::string> fwStorage_list;
	if (!IMM_Util::getChildrenObject(dn,fwStorage_list))
	{
		if (createFwStorageEnvironment(dn,data)) result = true;
		else result = false;
	}
	else
	{
		std::string fwStorage_dn("");
		std::string rdn_fwStorage("");

		//loop of instances
		for (unsigned i=0; i<fwStorage_list.size(); i++)
		{
			fwStorage_dn.clear();
			fwStorage_dn = fwStorage_list[i].c_str();

			rdn_fwStorage.clear();
			//check if is an object of FWStorege Class
			IMM_Util::getRdnAttribute(fwStorage_dn,rdn_fwStorage);
			if ( strcmp(rdn_fwStorage.c_str(),IMM_Util::RDN_OTHERBLADEFWSTORAGE) == 0 )
			{
				foundInstances=true;

				//get values
				int state = 0;
				IMM_Util::getImmAttributeInt(fwStorage_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_STATUS,state);
				data.upgradeStatus = static_cast<CCH_Util::FwStatus>(state);

				std::string result("OK");
				IMM_Util::getImmAttributeString(fwStorage_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_RESULT,result);
				data.result = result;

				std::string reason("-");
				IMM_Util::getImmAttributeString(fwStorage_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_REASON,reason);
				data.lastUpgradeReason = reason;

				std::string lastTime("-");
				IMM_Util::getImmAttributeString(fwStorage_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_TIME,lastTime);
				data.lastUpgradeTime = lastTime;

				std::string lastDate("-");
				IMM_Util::getImmAttributeString(fwStorage_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_DATE,lastDate);
				data.lastUpgradeDate = lastDate;

				data.slotUpgOnGoing = slot;

				std::vector<std::string> fwStorageInfo_list;
				if (IMM_Util::getChildrenObject(fwStorage_dn,fwStorageInfo_list))
				{
					std::string fwStorageInfo_dn("");
					std::string rdn_fwStorageInfo("");

					//loop of instances
					for (unsigned j=0; j<fwStorageInfo_list.size(); j++)
					{
						fwStorageInfo_dn.clear();
						fwStorageInfo_dn = fwStorageInfo_list[j].c_str();

						rdn_fwStorageInfo.clear();
						//check if is an object of FWStorege Class
						IMM_Util::getRdnAttribute(fwStorageInfo_dn,rdn_fwStorageInfo);
						if ( strcmp(rdn_fwStorageInfo.c_str(),IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_OLD) == 0 )
						{

							std::string fwType("");
							IMM_Util::getImmAttributeString(fwStorageInfo_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_TYPE,fwType);
							data.fwInfo.fwType[j] = fwType;

							std::string fwVers("");
							IMM_Util::getImmAttributeString(fwStorageInfo_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_VERSION,fwVers);
							data.fwInfo.fwVersion[j] = fwVers;
						}
					}
				}
				break;
			}
		}

		if (!foundInstances)
		{
			if (createFwStorageEnvironment(dn,data)) result = true;
			else result = false;
		}
	}

	return result;
}

bool FIXS_CCH_FSMDataStorage::createFwStorageEnvironment(std::string dn,CCH_Util::FwData &data)
{
	bool retVal = true;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	ACS_APGCC_ImmObject object;
	if (!IMM_Util::getObject(dn_fw, &object))
	{
		/////////////////CREATE object old
		//The vector of attributes
		vector<ACS_CC_ValuesDefinitionType> AttrList;
		int stateFwOld = CCH_Util::FW_IDLE;

		//the attributes of PFM class
		ACS_CC_ValuesDefinitionType FWRDN = IMM_Util::defineAttributeString(IMM_Util::RDN_OTHERBLADEFWSTORAGE,ATTR_STRINGT,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,1);
		ACS_CC_ValuesDefinitionType FWstatus = IMM_Util::defineAttributeInt(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_STATUS,ATTR_INT32T,&stateFwOld,1);
		ACS_CC_ValuesDefinitionType FWresult = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_RESULT,ATTR_STRINGT,"-",1);
		ACS_CC_ValuesDefinitionType FWreason = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_REASON,ATTR_STRINGT,"-",1);
		ACS_CC_ValuesDefinitionType FWdate = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_DATE,ATTR_STRINGT,"-",1);
		ACS_CC_ValuesDefinitionType FWtime = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_TIME,ATTR_STRINGT,"-",1);

		AttrList.push_back(FWRDN);
		AttrList.push_back(FWstatus);
		AttrList.push_back(FWresult);
		AttrList.push_back(FWreason);
		AttrList.push_back(FWdate);
		AttrList.push_back(FWtime);

		if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classOtherBladeFwStorage,dn.c_str(),AttrList))
		{
			retVal = false;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "CREATION object FW Storage FAILED" << std::endl;
		}
		else
		{
			data.lastUpgradeDate="-";
			data.lastUpgradeTime="-";
			data.result="-";
			data.lastUpgradeReason="-";
			data.upgradeStatus=CCH_Util::FW_IDLE;
		}

		//free memory
		delete[] FWRDN.attrValues;
		FWRDN.attrValues = NULL;
		delete[] FWstatus.attrValues;
		FWstatus.attrValues = NULL;
		delete[] FWresult.attrValues;
		FWresult.attrValues = NULL;
		delete[] FWreason.attrValues;
		FWreason.attrValues = NULL;
		delete[] FWdate.attrValues;
		FWdate.attrValues = NULL;
		delete[] FWtime.attrValues;
		FWtime.attrValues = NULL;

	}

	free(dn_fw);
	return retVal;
}

bool FIXS_CCH_FSMDataStorage::readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::fwRemoteData &data)
{
	bool result = true;

	std::string dn = IMM_Interface::getDnBladePersistance(magazine,slot);

	std::vector<std::string> fwStorage_list;
	if (IMM_Util::getChildrenObject(dn,fwStorage_list))
	{
		std::string fwStorage_dn("");
		std::string rdn_fwStorage("");

		//loop of instances
		for (unsigned i=0; i<fwStorage_list.size(); i++)
		{
			fwStorage_dn.clear();
			fwStorage_dn = fwStorage_list[i].c_str();

			rdn_fwStorage.clear();
			//check if is an object of FWStorege Class
			IMM_Util::getRdnAttribute(fwStorage_dn,rdn_fwStorage);
			if ( strcmp(rdn_fwStorage.c_str(),IMM_Util::RDN_OTHERBLADEFWSTORAGE) == 0 )
			{
				std::vector<std::string> fwStorageInfo_list;
				if (IMM_Util::getChildrenObject(fwStorage_dn,fwStorageInfo_list))
				{
					std::string fwStorageInfo_dn("");
					std::string rdn_fwStorageInfo("");

					//loop of instances
					for (unsigned j=0; j<fwStorageInfo_list.size(); j++)
					{
						fwStorageInfo_dn.clear();
						fwStorageInfo_dn = fwStorageInfo_list[j].c_str();

						rdn_fwStorageInfo.clear();
						//check if is an object of FWStorege Class
						IMM_Util::getRdnAttribute(fwStorageInfo_dn,rdn_fwStorageInfo);
						if ( strcmp(rdn_fwStorageInfo.c_str(),IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_NEW) == 0 )
						{
							std::string fwType("");
							IMM_Util::getImmAttributeString(fwStorageInfo_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_TYPE,fwType);
							data.fwType[j] = fwType;

							std::string fwVers("");
							IMM_Util::getImmAttributeString(fwStorageInfo_dn,IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_VERSION,fwVers);
							data.fwVersion[j] = fwVers;
						}
					}
				}
			}
		}
	}

	return result;
}


bool FIXS_CCH_FSMDataStorage::saveUpgradeStatus (std::string dn, CCH_Util::FwData &data)
{
	bool res = false;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	ACS_APGCC_ImmObject object;
	if (IMM_Util::getObject(dn_fw, &object))
	{
		int state = data.upgradeStatus;
		ACS_CC_ImmParameter status = IMM_Util::defineParameterInt(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_STATUS,ATTR_INT32T,&state,1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,status);
		delete[] status.attrValues;

		res = true;
	}

	free(dn_fw);
	return res;

}

bool FIXS_CCH_FSMDataStorage::saveFwShFileName (std::string dn, std::string valueToSet)
{
	bool res = false;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	ACS_APGCC_ImmObject object;
	if (IMM_Util::getObject(dn_fw, &object))
	{
		ACS_CC_ImmParameter shFile = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_SH_FILE,ATTR_STRINGT,valueToSet.c_str(),1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,shFile);
		delete[] shFile.attrValues;

		res = true;
	}

	free(dn_fw);
	return res;

}

bool FIXS_CCH_FSMDataStorage::readFwShFileName (std::string dn, std::string &value)
{
	bool res = false;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	if (IMM_Util::getImmAttributeString(dn_fw,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_SH_FILE,value)) res = true;

	free(dn_fw);
	return res;

}

bool FIXS_CCH_FSMDataStorage::saveFwXmlFileName (std::string dn, std::string valueToSet)
{
	bool res = false;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	ACS_APGCC_ImmObject object;
	if (IMM_Util::getObject(dn_fw, &object))
	{
		ACS_CC_ImmParameter xmlFile = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_XML_FILE,ATTR_STRINGT,valueToSet.c_str(),1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,xmlFile);
		delete[] xmlFile.attrValues;

		res = true;
	}

	free(dn_fw);
	return res;

}
bool FIXS_CCH_FSMDataStorage::readFwXmlFileName (std::string dn, std::string &value)
{
	bool res = false;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	if (IMM_Util::getImmAttributeString(dn_fw,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_XML_FILE,value)) res = true;

	free(dn_fw);
	return res;

}

bool FIXS_CCH_FSMDataStorage::saveFwRevisionState (std::string dn, std::string valueToSet)
{
	bool res = false;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	ACS_APGCC_ImmObject object;
	if (IMM_Util::getObject(dn_fw, &object))
	{
		ACS_CC_ImmParameter revState = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_REVISION,ATTR_STRINGT,valueToSet.c_str(),1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,revState);
		delete[] revState.attrValues;

		res = true;
	}

	free(dn_fw);
	return res;

}

bool FIXS_CCH_FSMDataStorage::readFwRevisionState (std::string dn, std::string &value)
{
	bool res = false;
	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	if (IMM_Util::getImmAttributeString(dn_fw,IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_REVISION,value)) res = true;

	free(dn_fw);
	return res;
}



bool FIXS_CCH_FSMDataStorage::saveRemoteFWUpgradeStatus(std::string dn, CCH_Util::FwData &data)
{
	bool res = false;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	ACS_APGCC_ImmObject object;
	if (IMM_Util::getObject(dn_fw, &object))
	{
		int state = data.upgradeStatus;
		ACS_CC_ImmParameter status = IMM_Util::defineParameterInt(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_STATUS,ATTR_INT32T,&state,1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,status);
		delete[] status.attrValues;

		ACS_CC_ImmParameter time = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_TIME,ATTR_STRINGT,data.lastUpgradeTime.c_str(),1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,time);
		delete[] time.attrValues;

		ACS_CC_ImmParameter date = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_DATE,ATTR_STRINGT,data.lastUpgradeDate.c_str(),1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,date);
		delete[] date.attrValues;

		ACS_CC_ImmParameter result = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_RESULT,ATTR_STRINGT,data.result.c_str(),1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,result);
		delete[] result.attrValues;

		ACS_CC_ImmParameter reason = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEFWSTORAGE_RFW_REASON,ATTR_STRINGT,data.lastUpgradeReason.c_str(),1);
		IMM_Util::modify_OM_ImmAttr(dn_fw,reason);
		delete[] reason.attrValues;

		res = true;
	}

	free(dn_fw);
	return res;

}

bool FIXS_CCH_FSMDataStorage::saveFwUpgradeStatusFromFile(std::string dn,CCH_Util::fwRemoteData &data, bool isSMX)
{
	bool res = true;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	std::string rdn_fwStorageInfo("");
	std::vector<std::string> fwStorageInfo_list;

	if (IMM_Util::getChildrenObject(dn_fw,fwStorageInfo_list))
	{
		std::string fwStorageInfo_dn("");

		//loop of instances
		for (unsigned j=0; j<fwStorageInfo_list.size(); j++)
		{
			fwStorageInfo_dn.clear();
			fwStorageInfo_dn = fwStorageInfo_list[j].c_str();

			rdn_fwStorageInfo.clear();
			//check if is an object of FWStorege Class
			IMM_Util::getRdnAttribute(fwStorageInfo_dn,rdn_fwStorageInfo);
			if ( strcmp(rdn_fwStorageInfo.c_str(),IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_NEW) == 0 )
			{
				IMM_Util::deleteImmObject(fwStorageInfo_dn);
			}
		}

	}
	int id = 0;
	for(int i=0;i< MAX_NO_OF_VERSION_INDEX; i++)
	{
		if (strcmp(data.fwType[i].c_str(),"") == 0) continue;
		//The vector of attributes
		vector<ACS_CC_ValuesDefinitionType> AttrList;

		std::string sInd = CCH_Util::intToString(id);
		std::string rdn = IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_NEW + string("=") + sInd;


		ACS_CC_ValuesDefinitionType FWINFORDN;
		ACS_CC_ValuesDefinitionType FWTYPE;
		ACS_CC_ValuesDefinitionType FWVERS;

		if (isSMX && (i == 0))
		{
			std::string cxpType;
			bool isAreaA=false;
			if((data.productno).compare(SMX_FWTYPE_A_CXP) == 0 )
			{
				isAreaA = true;
			}
			cxpType = (isAreaA) ? ("CXP_TYPE_A"): ("CXP_TYPE_B");
			std::string cxpValue = data.productno + "_" +  data.cxp_rState;

			FWINFORDN = IMM_Util::defineAttributeString(IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_NEW,ATTR_STRINGT,rdn.c_str(), 1);
			FWTYPE = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_TYPE,ATTR_STRINGT,cxpType.c_str(), 1);
			FWVERS = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_VERSION,ATTR_STRINGT, cxpValue.c_str(), 1);

			AttrList.push_back(FWINFORDN);
			AttrList.push_back(FWTYPE);
			AttrList.push_back(FWVERS);

			if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classOtherBladeFwStorageInfoNew, dn_fw, AttrList))
				res = false;

			//free memory
			delete[] FWINFORDN.attrValues;
			FWINFORDN.attrValues = NULL;
			delete[] FWTYPE.attrValues;
			FWTYPE.attrValues = NULL;
			delete[] FWVERS.attrValues;
			FWVERS.attrValues = NULL;

			if (!res)
				break;
			id++;

			sInd = CCH_Util::intToString(id);
			rdn = IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_NEW
				+ string("=") + sInd;
			AttrList.clear();
		}

		//the attributes of PFM class
		FWINFORDN = IMM_Util::defineAttributeString(IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_NEW,ATTR_STRINGT,rdn.c_str(),1);
		FWTYPE = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_TYPE,ATTR_STRINGT,data.fwType[i].c_str(),1);
		FWVERS = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_VERSION,ATTR_STRINGT,data.fwVersion[i].c_str(),1);

		AttrList.push_back(FWINFORDN);
		AttrList.push_back(FWTYPE);
		AttrList.push_back(FWVERS);

		if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classOtherBladeFwStorageInfoNew,dn_fw,AttrList)) res = false;

		//free memory
		delete[] FWINFORDN.attrValues;
		FWINFORDN.attrValues = NULL;
		delete[] FWTYPE.attrValues;
		FWTYPE.attrValues = NULL;
		delete[] FWVERS.attrValues;
		FWVERS.attrValues = NULL;

		id++;
		if (!res) break;
	}
	free(dn_fw);
	return res;
}

bool FIXS_CCH_FSMDataStorage::saveOldFwUpgradeStatus(std::string dn, CCH_Util::fwRemoteData &data)
{
	bool res = true;

	char * dn_fw=(char*)malloc(200);
	memset(dn_fw,0,200);
	sprintf(dn_fw,"%s,%s",IMM_Util::ATT_OTHERBLADEFWSTORAGE_RDN,dn.c_str());

	std::string rdn_fwStorageInfo("");
	std::vector<std::string> fwStorageInfo_list;

	if (IMM_Util::getChildrenObject(dn_fw,fwStorageInfo_list))
	{
		std::string fwStorageInfo_dn("");

		//loop of instances
		for (unsigned j=0; j<fwStorageInfo_list.size(); j++)
		{
			fwStorageInfo_dn.clear();
			fwStorageInfo_dn = fwStorageInfo_list[j].c_str();

			rdn_fwStorageInfo.clear();
			//check if is an object of FWStorege Class
			IMM_Util::getRdnAttribute(fwStorageInfo_dn,rdn_fwStorageInfo);
			if ( strcmp(rdn_fwStorageInfo.c_str(),IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_OLD) == 0 )
			{
				IMM_Util::deleteImmObject(fwStorageInfo_dn);
			}
		}

	}
	int id = 0;
	for(int i=0;i< MAX_NO_OF_VERSION_INDEX; i++)
	{
		if (strcmp(data.fwType[i].c_str(),"") == 0) continue;

		//The vector of attributes
		vector<ACS_CC_ValuesDefinitionType> AttrList;

		std::string sInd = CCH_Util::intToString(id);
		std::string rdn = IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_OLD + string("=") + sInd;

		//the attributes of PFM class
		ACS_CC_ValuesDefinitionType FWINFORDN = IMM_Util::defineAttributeString(IMM_Util::RDN_OTHERBLADEFWSTORAGEINFO_OLD,ATTR_STRINGT,rdn.c_str(),1);
		ACS_CC_ValuesDefinitionType FWTYPE = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_TYPE,ATTR_STRINGT,data.fwType[i].c_str(),1);
		ACS_CC_ValuesDefinitionType FWVERS = IMM_Util::defineAttributeString(IMM_Util::ATT_OTHERBLADEFWSTORAGEINFO_FW_VERSION,ATTR_STRINGT,data.fwVersion[i].c_str(),1);

		AttrList.push_back(FWINFORDN);
		AttrList.push_back(FWTYPE);
		AttrList.push_back(FWVERS);

		if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classOtherBladeFwStorageInfoOld,dn_fw,AttrList)) res = false;

		//free memory
		delete[] FWINFORDN.attrValues;
		FWINFORDN.attrValues = NULL;
		delete[] FWTYPE.attrValues;
		FWTYPE.attrValues = NULL;
		delete[] FWVERS.attrValues;
		FWVERS.attrValues = NULL;

		id++;

		if (!res) break;
	}
	free(dn_fw);
	return res;
}
