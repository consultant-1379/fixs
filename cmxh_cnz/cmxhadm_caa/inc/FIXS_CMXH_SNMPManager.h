
#ifndef SNMPMANAGER_H_
#define SNMPMANAGER_H_ 1

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

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include "FIXS_CMXH_Util.h"

class FIXS_CMXH_SNMPManager //: public ACE_Task_Base
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
		SNMPGET_STD_STRING = 0,
		SNMPGET_HEX_STRING = 1,
		SNMPGET_ASCII_STRING = 2
	};


	FIXS_CMXH_SNMPManager(std::string ipAddress);
	~FIXS_CMXH_SNMPManager();
	ACS_TRA_Logging *FIXS_CMXH_logging;

	bool isCBAenvironment(void);
	std::string getIpAddress(void);
	void setIpAddress(std::string ip);

	int snmpGet(const char* community, const char* peername,const char* usrOID, int type, void* out, unsigned int iBufSize, int strType = 0);

	int snmpSet(const char* community, const char* peername, const char* usrOID,  char type, const char *value);

	bool setIfMibAdminStatus (int index, int valueToSet);
	bool getIfMibAdminStatus (unsigned int index, int & value);

	bool setStpPortAsEdgePort (int index, int valueToSet);
	bool getStpPortAsEdgePort (unsigned int index, int & value);

	bool setBridgeMibStpPortEnable (int index, int valueToSet);
	bool getBridgeMibStpPortEnable (unsigned int index, int & value);

	bool setMibVlanStaticRowStatus (int vlanID, int valueToSet);
	bool getMibVlanStaticRowStatus (unsigned int vlanID, int &value);

	bool setMibVlanStaticName (int vlanID, std::string valueToSet);
	bool getMibVlanStaticName (unsigned int vlanID, std::string &value);

	bool setMibVlanStaticEgressPorts (int vlanID, std::string valueToSet);
	bool getMibVlanStaticEgressPorts (unsigned int vlanID, std::string &value);

	bool setMibVlanStaticUntaggedPorts (int vlanID, std::string valueToSet);
	bool getMibVlanStaticUntaggedPorts (unsigned int vlanID, std::string &value);

	bool setMibPortDefaultUserPriority (int portNum, int portPriority);
	bool getMibPortDefaultUserPriority (unsigned int portNum, int &value);

	bool getContFileName (unsigned int index, std::string & fileName);
	bool setContFileToDelete (std::string fileName);

	bool setContTransferSrvIP(std::string & srvIP);
	bool setContTransferSrvPath(std::string & snmplogPath);
	bool setContNotificationsCtrl (int & Value);
	bool setContAutomaticTransfer (int & Value);
	bool setContFileEncryption (int & Value);
	bool setCreateContFile (int & Value);








	//Check who use it !
	bool isWaiting (); //to be remove


	/************************************************************************************/

private:

	bool bExit;
	bool bRun;
	std::string ipAddr;

	CMXH_Util::EnvironmentType m_cba_environment;

        static ACE_Recursive_Thread_Mutex m_lock;  //TR_HX98360

};
#endif /* SNMPMANAGER_H_ */
