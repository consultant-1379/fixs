
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
#include "FIXS_CCH_Util.h"
#include "Client_Server_Interface.h"
#include "FIXS_CCH_DiskHandler.h"

class FIXS_CCH_SNMPManager //: public ACE_Task_Base
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


	FIXS_CCH_SNMPManager(std::string ipAddress);
	~FIXS_CCH_SNMPManager();

	bool isCBAenvironment(void);
	std::string getIpAddress(void);
	void setIpAddress(std::string ip);

	int snmpGet(const char* community, const char* peername,const char* usrOID, int type, void* out, unsigned int iBufSize, int strType = 0);

	int snmpSet(const char* community, const char* peername, const char* usrOID,  char type, const char *value);
	bool queryBoardPresencePower(int type, std::string &oidShelfpresence);
	int getBoardPower (std::string shelfMgrIp,int slot);
	int getBoardPresence (std::string shelfMgrIp,int slot);
	std::string getBladeReturnToFactoryDefaultOid();


	/************************************************************************************/
	//SCX
	int setSWMAction (unsigned int action);
	bool setSWMActionVersion (std::string version);
	bool setSWMActionURL (std::string url);
	bool getSwmActionStatus (int &actStatus);

	//Not used
	bool getSwmLoadedVersion (unsigned int index, std::string &loadVer);
	//bool getSwmLoadedType (unsigned int index, int *loadType);
	//bool getSwmLoadedAttribute (unsigned int index, int *loadAttr);
	//bool getSwmLoadedAttributeIndex (unsigned int attribute, int *kIndex);

	bool getCurrentLoadedVersion (std::string &loadVer);
	bool checkLoadedVersion (std::string loadVer);
	bool resetSwitch (unsigned int slot);
	bool getCurrentDefaultVersion (std::string &loadVer);

	/************************************************************************************/
	//IPMI
	bool getIpmiUpgStatus (std::string slot, int &input);
	bool setIpmiUpg (std::string slot, std::string valueToSet);
	bool isMaster ();
	int isMaster (std::string ip);
	int isMasterSCB (std::string ip, ushort slot);
	bool getIpmiUpgStatus (std::string slot, std::string &input);
	bool getIPMIBoardData(std::string slot, std::string &running, std::string &product, std::string &revision);
	bool isPassiveDeg ();
	bool getIpmiFWUpgrade (std::string *installedFWUpgrade, std::string slot);

	/************************************************************************************/
	//PFM
	bool setCurrentPfmScanningState (int scanningState);
	bool setPfmFWUpgrade (std::string fwUpgrade, std::string typeIndex, std::string typeInstance);
	bool getPfmFWUpgrade (std::string *installedFWUpgrade, int typeIndex, int typeInstance);
	bool getPfmFWProductNumber (std::string *installedFWProductNumber, int typeIndex, int typeInstance);
	bool getPfmFWProductRevisionState (std::string *installedFWProductRevisionState, int typeIndex, int typeInstance);
	bool getPfmFWUpgradeStatus (int *actFwStatus, int typeIndex, int typeInstance);
	bool getPfmModuleDeviceType (unsigned int typeIndex, int *ModuleType);
	bool getPfmSwProductRevisionState (std::string *installedSwProductRevisionState, int swType, int typeInstance);
	bool getPfmSwProductNumber (std::string *installedSWProductNumber, int swType, int typeInstance);
	int getPfmCurSwType (int typeInstance);
	bool setPfmSWUpgrade (std::string swUpgrade, std::string typeIndex, std::string typeInstance);
	bool getPfmSWUpgrade (std::string *installedSwUpgrade, int typeIndex, int typeInstance);
	bool getPfmSWUpgradeStatus (int  *actSwStatus, int typeIndex, int typeInstance);
	bool getPfmSwInitialProductRevision (std::string *initialSWProductRevision, int swType, int typeInstance);
	bool getPfmSwInitialProductNumber (std::string *initialSWProductNumber, int swType, int typeInstance);
	

	//Non utilizzata
	//bool getCurrentPfmScanningState (std::string *scanningState);


	//Could be delete
	//Check utilizer and modify the code !
	//bool sendSetSNMPpfm (smiVALUE &smival, smiOID &xshmcName, smiUINT32 xshmcVector[], int typeIndex, int typeInstance);
	//bool sendGetSNMPpfm (smiOID &xshmcName, smiUINT32 xshmcVector[], int typeIndex, int typeInstance);

	//Check who use it !
	bool isWaiting (); //to be remove


	/************************************************************************************/
	//IPT
	//bool getXshmcGPRData (std::string *xshmcGPRDataValue, int slot, int typeInstance);
	bool getXshmcGprDataValue(std::string *gprDataValue, std::string slot, std::string typeInstance);
	bool setXshmcGprDataValue (std::string gprDataVal, std::string slot, std::string typeInstance);
	int getXshmcBoardPowerOff (string shelfMgrIp,int slot);
	bool setXshmcBoardReset (int slot);

	bool getXshmcBoardPwrOff (unsigned int index, int* pwrOffVal);
	bool getXshmcBoardPwrOn (unsigned int index, int *pwrOnVal);
	bool setXshmcBoardPwrOn (unsigned int action, unsigned int index);
	bool setXshmcBoardPwrOff (unsigned int action, unsigned int index);
	bool getGprDataValue (std::string *gprDataVal, std::string slot, std::string typeInstance);
	bool setGprDataValue (std::string gprDataVal, std::string slot, std::string typeInstance);
	bool setBladeReset (unsigned int action, unsigned int index);
	bool isBladePower (unsigned int index, int *pwrVal);
	bool setXshmcBoardYellowLed (unsigned int action, unsigned int index);
	bool getBladePresence (unsigned int index, int *Val);
	bool getBladePwr (unsigned int index, int *Val);
	bool setBladePwr (unsigned int action, unsigned int index);

	//Not used
	//bool getXshmcBoardReset (unsigned int index, int *resetVer);
	//bool getMasterSCB (unsigned int index, int *resetVer);

	//Could be delete
	//Check utilizer and modify the code !
	//bool sendGetSNMPGPR (smiOID &xshmcName, smiUINT32 xshmcVector[], int slot, int typeInstance);
	//bool sendSetSNMPGPR (smiVALUE &smival, smiOID &xshmcName, smiUINT32 xshmcVector[], int slot, int typeInstance);


	/************************************************************************************/
	//FW
	bool getFwmLoadedAttribute (unsigned int index, int *Val);
	bool getFwmLoadedType (unsigned int index, std::string &loadVer);
	bool getFwmLoadedVersion (unsigned int index, std::string &loadVer);
	bool getBladeRevisionState (std::string *actState);
	bool setFWMAction (unsigned int action);
	bool setFWMActionURL (std::string url);
	bool getFwmActionStatus (std::string *actStatus);
	bool getFWMActionURL (std::string *installedFWUpgrade);
	bool setActiveArea (unsigned int slot, int action);
	bool getFwmActionResult(int&);
	bool getActiveArea (std::string slot,int &activeArea);

	/************************************************************************************/

private:

	bool bExit;
	bool bRun;
	std::string ipAddr;

	CCH_Util::EnvironmentType m_cba_environment;

};
#endif /* SNMPMANAGER_H_ */
