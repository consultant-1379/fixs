/*
 * FIXS_TREFC_SnmpManager.h
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */

#ifndef FIXS_TREFC_SNMPMANAGER_H_
#define FIXS_TREFC_SNMPMANAGER_H_


#include <ace/Task.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/mib_api.h>
#include <net-snmp/library/mib.h>

#include <string.h>
#include <sstream>
#include <iostream>

//#include "FIXS_TREFC_SNMPTrapReceiver.h"
#include "ACS_TRA_trace.h"
#include "FIXS_TREFC_Util.h"
//#include "FIXS_CCH_Event.h"

class FIXS_TREFC_SNMPManager //: public ACE_Task_Base
{

public:

	static const int GETSET_SNMP_OK = 0;
	static const int GETSET_SNMP_OK_LEN_ZERO = 1;
	static const int OPEN_SESS_INIT_ERROR = 2;
	static const int OPEN_SESS_ERROR = 3;
	static const int GETSET_SNMP_PARSE_OID_ERROR = 4;
	static const int GETSET_SNMP_PDU_CREATE_ERROR = 5;
	static const int GETSET_SNMP_ADD_NULL_VAR = 6;
	static const int GETSET_SNMP_RESPONSE_ERROR = 7;
	static const int GETSET_SNMP_STATUS_ERROR = 8;
	static const int GETSET_SNMP_STATUS_TIMEOUT = 9;
	static const int GETSET_SNMP_BUFFERSMALL = 10;

	enum {
		LOADRELEASE = 1,
		STARTRELEASE = 2,
		MAKEDEFAULT = 3,
		REMOVERELEASE = 4
	};

	enum {
		NOATTRIBUTE = 0,
		CURRENTSW = 1,
		DEFAULTSW = 2,
		CURRENTDEFAULTSW = 3
	};

	enum {
		FAILSAFE = 1,
		EXCHANGEBLE = 2,
		PROGRAM = 3
	};

	enum {
		SNMPGET_STD_STRING = 0,
		SNMPGET_HEX_STRING = 1,
		SNMPGET_ASCII_STRING = 2
	};


	FIXS_TREFC_SNMPManager(std::string ipAddress);
	~FIXS_TREFC_SNMPManager();

	/* enable this methods if you want an SNMP Thread

	virtual int svc(void);
	virtual int close (u_long flags = 0);

	int initialize(void);
	bool isRunning(void);
	*/
	bool isCBAenvironment(void);
	std::string getIpAddress(void);
	void setIpAddress(std::string ip);

	int snmpGet(const char* community, const char* peername,const char* usrOID, int type, void* out, unsigned int iBufSize, int strType = 0);

	int snmpSet(const char* community, const char* peername, const char* usrOID,  char type, const char *value);

	void stopWork();



	bool isMaster ();
	int isMaster (std::string ip);





	//Check who use it !
	//bool isWaiting (); //to be remove



	bool setGenNtpEntServer (int index, string valueToSet);
	bool setGenNtpEntSvrRowStatus (unsigned int action, unsigned int index);
	bool setGenNtpEntSvrRowStatus (unsigned int action, unsigned int index, int &error);
	bool setGenNtpEntConfigComplete ();
	bool setGenNtpEntNtpdRestart ();
	bool setStsiSyncInput (int syncInput);
	bool setStsiTuRole (int tuRole);
	bool setStsiTodServer (int todServer);
	bool setStsiTodRef (int todRef);
	bool setSystemTime ();
	//	      void setServStatus (IMM_Util::TimeServiceStatus status);
	bool setStsiNtpRefIp (int index, string ntpRefIp);
	bool setStsiNtpRefRowStatus (unsigned int action, unsigned int index);
	bool setStsiNtpRefRowStatus (unsigned int action, unsigned int index, int &error);
	bool setsysMgrAliasIpAddr (int index, string aliasIp);
	bool setsysMgrAliasDefaultGateway (int index, std::string aliasDefGateway);
	bool setsysMgrAliasNetMask (int index, std::string aliasNetmask);
	bool setsysMgrAliasRowStatus (unsigned int action, unsigned int index);
	bool setsysMgrAliasRealDevName (int index, std::string aliasRealDev);
	bool setStsiIslEthIn (int value);
	bool setStsiSntpSetCpuClock (int value);
	bool setStsiSntpClientMode (int value);
	bool setStsiSntpIfIp (int index, string sntpIfIp);
	bool setStsiSntpIfRowStatus (unsigned int action, unsigned int index);
	bool setStsiSntpIfIpMask (int index, string sntpIfIpMask);
	bool setStsiSntpIfVlan (int index, unsigned int sntpIfVlan);
	bool setGenNtpEntWhiteInterface (int index, string interface);
	bool setGenNtpEntWhiteRowStatus (unsigned int action, unsigned int index);
	bool setGenNtpEntWhiteRowStatus (unsigned int action, unsigned int index, int &error);
	bool setStsiSntpPartnerIp (unsigned int index, string partnerIp);


	bool getStsiSntpAssocIp (unsigned int index, string *sntpAssocIp);
	bool getStsiSntpAssocStatus (unsigned int index, int *sntpAssocStatus);
	bool getStsiSntpSelectedRefIndex (unsigned int index, int *sntpSelectedIndex);
	bool getStsiSntpStatus (unsigned int index, int *sntpStatus);
	bool getStsiNtpRefIp (unsigned int index, string *stsiNtpRefIp);
	bool getStsiNtpRefRowStatus (unsigned int index, int *value);
	//	      bool getShelfManagerStatus (unsigned int index, int *status);

	bool getGenNtpEntStatusCurrentMode (unsigned int index, int *ntpCurrMode);
	bool getGenNTPEntStatusActiveRefSourceId (unsigned int index, string *ntpAddress);
	bool getGenNTPEntServer (unsigned int index, string *ntpAddress);
	bool getstsiGpsStatus (unsigned int index, int *gpsStatus);

	//	      bool getGenNtpEntServer (unsigned int index, string *loadVer, bool *noInstance);

	//TO BE REMOVED AFTER APBM IS READY --begin
	bool setIfAdminStatus27up ();
	bool getIfAdminStatus27 (int *adminStatus);
	//TO BE REMOVED AFTER APBM IS READY --end


//
//	  private: //## implementation
//	       std::string m_ipAddress;
//
//	       std::string m_trefAddress;
//
//	       int m_shelfMgrStatus;
//
//	       std::string m_ntpAddress;
//
//	       int m_stsiSntpStatus;
//
//	       int m_stsiSntpSelectedRef;
//
//	       std::string  m_stsiSntpAssocIp;
//
//	       int m_stsiSntpAssocStatus;
//
//	       std::string m_stsiNtpRefIp;


private:

	bool bExit;
	bool bRun;
	std::string ipAddr;
	static ACE_Recursive_Thread_Mutex m_lock;          // TR_HX84357

};

#endif /* FIXS_TREFC_SNMPMANAGER_H_ */
