/*
 * FIXS_TREFC_Manager.h
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */

#ifndef FIXS_TREFC_MANAGER_H_
#define FIXS_TREFC_MANAGER_H_

#include "FIXS_TREFC_Util.h"
#include "FIXS_TREFC_IMM_Util.h"
#include "FIXS_TREFC_TimeChecker.h"
#include "FIXS_TREFC_SNMPManager.h"
#include "FIXS_TREFC_JTPHandler.h"
#include "FIXS_TREFC_ImmInterface.h"
#include "FIXS_TREFC_AlarmHandler.h"

#include "ACS_CS_API.h"
#include "ACS_CS_API_ClientObserver.h"
#include "FIXS_TREFC_ApzScxHandler.h"
#include "FIXS_TREFC_AptScxHandler.h"
#include "FIXS_TREFC_OI_TimeService.h"
//#include "ACS_CS_API_DLL_Import.h"
#include "ACS_TRA_trace.h"
#include <string>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>

class FIXS_TREFC_TimeChecker;
class FIXS_TREFC_TrapSubscriber;

//using namespace std;
using namespace TREFC_Util;
//using namespace ACS_CS_API_TIMEREF;

#define MAX_NO_OF_APT_SCX 50

/*struct SCXIPAddress
*/
class FIXS_TREFC_SNMPTrapReceiver;
class FIXS_TREFC_SNMPManager;
class FIXS_TREFC_ApzScxHandler;
class FIXS_TREFC_AptScxHandler;
class FIXS_TREFC_AlarmHandler;

class FIXS_TREFC_Manager : public ACS_CS_API_HWCTableObserver
{



public:

	static  FIXS_TREFC_Manager * getInstance ();

	int initTRefManager ();

	int addTRefToList (std::string trefIP, std::string trefName);

	int rmTRefFromList (std::string trefIP, std::string trefName);

	//int getTRefList (char *buffer, int bufferSize, int &code);
	int getTRefListStatus (int *status_scxA, int *status_scxB, string *activeRef_scxA, string *activeRef_scxB);

	int getTRefListStatus (int *status_scxA, string *activeRef_scxA, string *activeRef_name,  int slot);

	int trefcStart ();

	int trefcStop ();

	int GetAPZSwitchMagazineSlot (unsigned char &shelf, bool &slot0, bool &slot25);

	int setTimeOnAllScx ();

	void getTrefcData (int &status, int &type, std::map<std::string,std::string> &trefList);

	bool settingSNMPToSCXatStartUP (map<string,string> &tRefList, string &IP_EthA, string &IP_EthB);

	void retrySettingAtSCXStart (int slot);

	void update (const ACS_CS_API_HWCTableChange& observer);

	int GetSWITCHBoardAddress(vector<SWITCHAPTIPAddress> &switchAPTVector,vector<SWITCHAPZIPAddress> &switchAPZVector,bool isSMX);

	int addScxExtIp (std::string extIp, std::string netMask, std::string defGateway, unsigned short slot);

	int rmScxExtIp (unsigned short slot);

	bool GetSCXAddressForAPZSlot(string &ipA, string &ipB,unsigned short slot);

	//int AddExtIpSNMPSet (unsigned short slot, string extIp, string netMask, string defGateway);

	void setExtIpAtTrefcStart (bool isSMX);

	 void coldStart (string snmpIP);
	 int trefcRestart ();
	// Additional Public Declarations
	 void finalize();

	 int validateScxExtIpAction(IMM_Util::Action action);

	 int validateTimeServerTransaction(std::vector<TimeServerAction> transaction);

	 bool getExtIpAddrConfiguration(ExtIpSwitch &extIp, unsigned int slot);

	 bool setSeviceType(IMM_Util::TimeServiceType type, int &error);

	 bool setReferenceType(IMM_Util::ExternalReferenceType type, int &error);
	
 	 void checkAlarmStatus (bool timeout);
	
	 bool isStopOperation();
	 void setStopOperation();
	 int migrate_ScxTref_to_SwitchTref(bool isSMX);
	 //int getDataFromScxTref();
	 //int setDataInSwitchDn();

protected:
	// Additional Protected Declarations

private:

	FIXS_TREFC_Manager();

	virtual ~FIXS_TREFC_Manager();

	int getTrefList (bool isSMX);

	bool isUniqueName (std::string name);

	bool isUniqueName (std::string name, std::map <string,string> temp_tRefList);

	bool isUniqueIp (std::string ip);

	bool isGPSList ();

	bool isGPSList (std::map <string,string> temp_tRefList);

	//int GetAPZShelfAddress(string &address);

	void printTRA(std::string mesg);

	int addTRefToTempList (std::string trefIP, std::string trefName, map<string,string> &temp_tRefList);

	int rmTRefFromTempList (std::string trefIP, std::string trefName, map<string,string> &temp_tRefList);

	void alignAptArray();

	void alignApzArray();

	string getNetworkReferenceName(string ipAddress);

	// Additional Private Declarations


private:
	// Data Members for Class Attributes

	static FIXS_TREFC_Manager* m_instance;

	map<string,string> m_tRefList;

	std::string serviceStatusStr;

	std::string serviceTypeStr;

	IMM_Util::TimeServiceStatus servStatus;

	IMM_Util::TimeServiceType servType;

	IMM_Util::ExternalReferenceType refType;

	//FIXS_TREFC_SNMPManager* m_snmpManager;

	FIXS_TREFC_SNMPTrapReceiver* m_trapManager;
//
	FIXS_TREFC_JTPHandler* m_jtpHandlerTref;

	FIXS_TREFC_TimeChecker* m_timeChecker;

	FIXS_TREFC_ApzScxHandler* m_apzScxHandler[2];

	FIXS_TREFC_AptScxHandler* m_aptScxHandler[MAX_NO_OF_APT_SCX];

//	FIXS_TREFC_SNMPHandler *m_snmpHandler[2];

	std::vector<SWITCHAPTIPAddress> m_switchAPTVector;

	std::vector<SWITCHAPZIPAddress> m_switchAPZVector;

	ExtIpSwitch extIpSwitch0;
	ExtIpSwitch extIpSwitch25;

	std::set<int> m_handleIdSet;

	FIXS_TREFC_AlarmHandler *m_alarmHandler;

	ACS_TRA_trace* _trace;

	bool isAlarmRaised;
	
	bool m_stopOperation;
	
	ACE_Recursive_Thread_Mutex m_lock;

	// Additional Implementation Declarations
};

// Class FIXS_TREFC_Manager

inline bool FIXS_TREFC_Manager::isStopOperation()
{
	return m_stopOperation;
}

inline void FIXS_TREFC_Manager::setStopOperation()
{
	m_stopOperation = true;
}

#endif /* FIXS_TREFC_MANAGER_H_ */
