
#ifndef FIXS_CCH_UpgradeManager_h
#define FIXS_CCH_UpgradeManager_h


#include "FIXS_CCH_ScxUpgrade.h"
#include "FIXS_CCH_CmxUpgrade.h"
#include "FIXS_CCH_IplbUpgrade.h"
#include "FIXS_CCH_SmxUpgrade.h"
#include "FIXS_CCH_FSMDataStorage.h"
//#include "Client_Server_Interface.h"
#include "FIXS_CCH_IptbUpgrade.h"
////#include "ACS_AEH_EvReport.h"
#include "FIXS_CCH_PowerFanUpgrade.h"
#include "FIXS_CCH_IpmiUpgrade.h"
#include "FIXS_CCH_FwUpgrade.h"

#include "FIXS_CCH_StateMachine.h"
#include "FIXS_CCH_DiskHandler.h"
#include "FIXS_CCH_SNMPManager.h"
#include "FIXS_CCH_SoftwareMap.h"
#include "FIXS_CCH_AlarmHandler.h"
#include "ACS_CS_API.h"
#include "ACS_CS_API_ClientObserver.h"

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"

#include "acs_apgcc_omhandler.h"
#include "acs_aeh_evreport.h"

#include <ace/Mutex.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include "ace/OS.h"
#include <list>
#include <string>

class FIXS_CCH_SoftwareMap;
class FIXS_CCH_AlarmHandler;
// File copy buffer. Needed for copyng of the file from input dir to shared area.
#define INT_FILE_CPYBUFFER        (int)4096
// Separator of the shelf digit
#define CHR_SHELF_SEPARATOR       '.'
// Character may be sobstituite of the shelf digit
#define CHR_IPMI_SHELFSEPARATOR   '-'
// Separator from filename, shelf and slot
#define CHR_IPMI_SEPARATOR        '_'
// file - extension separator
#define CHR_IPMI_FILE_SEPARATOR   '.'
// dir - file separator
#define CHR_IPMI_DIR_SEPARATOR   '/'
// Default file extension if not exist
#define STR_IPMI_DEFAULT_EXT      "bin"
// Shared area
#define STR_IPMI_FTP_AREA         "/data/apz/data/boot/"

#define CLUSTER_IP1 "192.168.169.33"
#define CLUSTER_IP2 "192.168.170.33"
#define SUB_IP1 "192.168.169"
#define SUB_IP2 "192.168.170"
#define IP1 ".1"
#define IP2 ".2"
#define IPMI_UP_EXTENSION ".mot"
#define TEMP_IPMIUPGRADE "temp_ipmiupgrade"

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)


class FIXS_CCH_UpgradeManager : public ACS_CS_API_HWCTableObserver
{
 public:

    class FsmListData 
    {
      public:

          FsmListData (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ipA, unsigned long ipB, std::string container, std::string dn, CCH_Util::FBN fbn = CCH_Util::SCXB,  FIXS_CCH_StateMachine *fsm = 0);

          //	char tempValueStr[16];
          //	unsigned long tempValue = IP_EthA;
          //	sprintf(tempValueStr, "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >>
          //	16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
          //	std::string IPA = tempValueStr;
          std::string getIPEthAString ();


          //	char tempValueStr[16];
          //	unsigned long tempValue = IP_EthB;
          //	sprintf(tempValueStr, "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >>
          //	16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
          //	std::string IPB = tempValueStr;
          std::string getIPEthBString ();


          std::string getIMPIMasterIP ();

        // Data Members for Class Attributes
         unsigned short m_boardId;

         unsigned long m_magazine;

         unsigned short m_slot;

         FIXS_CCH_StateMachine *m_swPtr;

         unsigned long m_IP_EthA;

         unsigned long m_IP_EthB;

         std::string m_container;

         FIXS_CCH_StateMachine *m_pfmPtr;

         FIXS_CCH_StateMachine *m_ipmiPtr;

         FIXS_CCH_StateMachine *m_fwPtr;

         FIXS_CCH_StateMachine *m_lmPtr;

         CCH_Util::FBN m_fbn;

         std::string m_dn;

         std::string m_ipMasterSCX;

         std::string m_slotMasterSCX;
	

      protected:
        // Additional Protected Declarations

      private:
        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

	uint32_t dmxc_address[2];	

	typedef std::list<FIXS_CCH_UpgradeManager::FsmListData> swUpgradeList_t;

	static  FIXS_CCH_UpgradeManager* getInstance ();

	int initialize ();

	void finalize ();
        
	void getDmxAddress(uint32_t &dmxc_add1, uint32_t  &dmxc_add2);

	int writeProgressReport (char* buffer, int bufferSize, std::string magazine, std::string slot);

	int prepare (unsigned long magazine, unsigned short slot, std::string product, int upgradeType, bool force);

	int activate (unsigned long umagazine, unsigned short uslot);

	int cancel (unsigned long umagazine, unsigned short uslot);

	int commit (unsigned long umagazine, unsigned short uslot);

	int progressreport (std::string magazine, std::string slot);

	int swinfo (std::string magazine, std::string slot);

	bool getUlongMagazine (std::string magazine, unsigned long &umagazine);

	bool getUshortSlot (std::string slot,  short unsigned &uslot);

	void loadReleaseCompleted (int loadResult,std::string snmpIP);

//	bool checkImmStatusInProgress (std::string dn, int nextState, std::string attribute);

	void coldStart (std::string snmpIP);	// IP address which the TRAP comes from

	bool checkSubnetIpTrapReceiver(std::string snmpIP, CCH_Util::trapType trapId, bool reset);

	//	this metod writes the current running software on the scxb into the input
	//	char buffer. this metods supports the swinfo status comman.
	int writeSwInfo (char* buffer, int bufferSize, std::string magazine, std::string slot);

	//bool checkOtherSlotStatus (std::string magazine, std::string slot);
	bool checkOtherSlotStatus (unsigned long umagazine, unsigned short uslot);

	bool checkOtherSlotPresence(std::string strBoardMip, std::string slot);

	bool isScxSoftwareUpgradeInProgress ();

	bool isCmxSoftwareUpgradeInProgress ();
	
	bool isSmxSoftwareUpgradeInProgress ();

	int ipmiUpgradeStatus (std::string magazine, std::string slot, std::string slotMaster, std::string ipMasterSCX);

	int InitFtpPath(const std::string &strPathIn, const std::string &strShelf, const std::string &dtrSlot, std::string *strPathOut);

	int ipmiUpgradeStart (std::string magazine, std::string slot, std::string slotMaster, std::string  ipMasterSCX, std::string  ipPassiveSCX, std::string valueToSet);	// Value to be set in the SHELF-MIB. The format is <IPaddress>:<FilePath>

	void ipmiUpgradeTrap (std::string subrackNumValue, std::string slotPosValue, std::string ipmiUpgradeResult, std::string snmpIP);

	int ipmiDisplay (std::string magazine, std::string slot);

	int writeIpmiStatus (char* buffer, int bufferSize);

	int writeIpmiDisplay (char* buffer, int bufferSize);


      int pfmUpgrade (std::string completePathFwPar, std::string magazine, std::string pfmTypeIndex, 	// Fan or Power
      std::string pfmInstanceIndex, 	// Lower or Upper
      std::string slot);


      void pfmUpgradeResult (int pfmUpgradeResult_, 	// Result read from the remote MIB.
      	// 0 stands for Not OK
      	// 1 stands for OK
      std::string snmpIP	// IP address which the TRAP comes from
      );


      //bool pfmCheckOtherSlotStatus (std::string magazine, std::string slot);
      bool pfmCheckOtherSlotStatus (unsigned long umagazine, unsigned short uslot);

      int pfmStatus (std::string magazine, std::string type, std::string inst);

      int pfmDisplay (std::string magazine, std::string type, std::string inst);

      int writePfmStatus (char* buffer, int bufferSize);

      int writePfmDisplay (char* buffer, int bufferSize);
	  
      int writePfmSwDisplay (char* buffer, int bufferSize);

      //	This method is used by PFM state machines to set "Scanninf ON/OFF" on the
      //	SCX not involved in the PFM upgrade.
      void setScanning (unsigned long magazine, unsigned short slot, bool on);

      //	HwcObserver specialized update method
      void update (const ACS_CS_API_HWCTableChange& observer);

      bool ipmiCheckOtherSlotStatus (unsigned long umagazine);

      bool ipmiCheckBoardStatus (unsigned long umagazine, unsigned short uslot, unsigned short uslotMaster);

      void swMaintenanceList (CCH_Util::RecOfMaintenance *&  listOfMaitenance, unsigned short &  boardListLen);

      int swInfoFbn (int fbn, std::vector<CCH_Util::SWINFO_FBN> &currentSwList);

      //	this metod writes the current running software on the scxb into the input
      //	char buffer. this metods supports the swinfo status comman.
      int writeSwInfoFbn (char* buffer, int bufferSize, int fbn, std::vector<CCH_Util::SWINFO_FBN> currentSwList);

      int prepareEpb1 (unsigned long umagazine, unsigned short uslot, std::string container, int typeOfPrepare = CCH_Util::SINGLE_BOARD);

      int prepareEvoEt (unsigned long umagazine, unsigned short uslot, std::string container, int typeOfPrepare = CCH_Util::SINGLE_BOARD);

      bool tokenize (const string &str, vector <unsigned short>& tokens, const string &delimiters);

      void sortlist (std::vector<CCH_Util::SWINFO_FBN> &currentSwList);

      int fetchBoardIdList (unsigned long mag, vector <unsigned short> &uSlotIntVec, vector <unsigned short> &boardIdList, int boardName, int type);

      int isEpbBoard (unsigned long magazine, unsigned short slot);

      bool isScxData (unsigned long magazine, unsigned short slot);

      bool isIptData (unsigned long magazine, unsigned short slot);

      bool isIplbData(unsigned long magazine, unsigned short slot);	

      int checkIplbSupervision(unsigned long magazine, unsigned short slot, unsigned short islot); 

      bool checkIplbUpgradeStatus (unsigned long magazine, unsigned short slot);

      bool isCmxData (unsigned long magazine, unsigned short slot);

      bool isEvoEtData (unsigned long magazine, unsigned short slot);
	  
      bool isSmxData (unsigned long magazine, unsigned short slot);

      //bool checkAllSCXSwVer (unsigned short fbn = 4);
      bool checkAllSCXandCMXSwVer (unsigned short fbn = 4, unsigned short fbn2 = CCH_Util::CMXB, int timeoutFlag = 0);

      bool checkIPLBswVr();
	  
      bool checkSMXswVr(int timeoutFlag = 0);

      int checkOccurrenceScxContainer (std::string container);

      int checkOccurrenceEpb1Container (std::string container);

      void sendAlarm (const int problem, const unsigned int severity, const std::string &cause, const std::string &data, const std::string &text);

      bool isIptLoadModuleUpgradeInProgress ();
      bool isIptLoadModuleUpgradeInProgress(std::string container);

      bool isIplbLoadModuleUpgradeInProgress();
      bool isIplbLoadModuleUpgradeInProgress(std::string container);

      bool removeScxEntry(unsigned long magazine, unsigned short slot,std::string &tempExpectedCxp);

      bool removeIptEntry(unsigned long magazine, unsigned short slot,std::string &tempExpectedCxp);

      bool removeIplbEntry(unsigned long magazine, unsigned short slot,std::string &tempExpectedCxp);

      bool removeEpb1Entry(unsigned long magazine, unsigned short slot);
	  
      bool removeSmxEntry(unsigned long magazine, unsigned short slot,std::string &tempExpectedCxp);

	  void addScxEntry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm = 0);

	  void addEpb1Entry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm = 0);

	  void addIptData (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *boardSm = 0);

 void addIplbData (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *boardSm = 0);


	  void addCmxEntry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm = 0);

	  void addEvoEtEntry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm = 0);
	  
	  void addSmxEntry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm = 0);

	  int getSwListSize();

	  int getEpb1ListSize();

	  int getIptbListSize();
           
          int getIplbListSize();

	  int fwRemoteStatus (std::string magazine, std::string slot, CCH_Util::FwData &tempFWData, CCH_Util::fwRemoteData &newFWData);

	  int printFwRemoteStatus (char *buffer, int bufferSize, CCH_Util::FwData &tempFWData, CCH_Util::fwRemoteData &newFWData);

	  int fwUpgradeStart (std::string magazine, std::string slot, std::string valueToSet);

	  void fwUpgradeTrap (int fwUpgradeResult, std::string snmpIP);

	  bool fwCheckOtherSlotStatus (unsigned long umagazine);

	  int unZipPackage (std::string &path, std::string &shFilename, std::string &xmlFilename, std::string mag, std::string slot, bool isRfui = false);

	  int getFirmwareVersion (unsigned long umagazine, unsigned short uslot, std::string IP_EthA_str, std::string IP_EthB_str, CCH_Util::fwRemoteData &versionInfo);

	  bool getMagAndSlotFromDn (std::string dn, unsigned long &magazine, unsigned short &slot);

	  bool isUsedPackage(std::string container);

	  std::string getCurrentProduct(unsigned long &magazine, unsigned short slot);

	  bool getBoardIps (unsigned long magazine, unsigned short slot, std::string &ethA, std::string &ethB);

	  void setBackupOngoing(bool backup);

	  bool isBackupOngoing();

	  bool isAnyUpgradeOngoing();

	  bool setCurrentProduct(unsigned long magazine, unsigned short slot, std::string container);

	  bool setCurrentIptProduct(unsigned long magazine, unsigned short slot, std::string container);

	  bool removeCmxEntry(unsigned long magazine, unsigned short slot, std::string & tempExpectedCxp);

	  bool removeEvoEtEntry(unsigned long magazine, unsigned short slot);

	  bool getMasterPassiveSCX(unsigned long magazine, std::string& masterIp, std::string& passiveIp, unsigned short& masterSlot);

	  bool getMasterPassiveSCB(unsigned long magazine, std::string& masterIpA, std::string& masterIpB, unsigned short& masterSlot);

	  bool checkCurrentVersionInBoard(std::string&, std::string&, std::string&);

	  int fetchEgem2L2SwitchIP(unsigned long magazine, unsigned short slot, std::string &ip_A, std::string &ip_B);

      // Data Members for Associations

	 //??? FIXS_CCH_SNMPManager *m_trapReceiver;
  public:
    // Additional Public Declarations

  protected:



      unsigned short numOfChar (std::istringstream &istr, char searchChar);

      void reverseDottedDecStr (std::istringstream &istr);

      bool dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4);

      bool checkFigures (std::istringstream &istr);
    // Additional Protected Declarations



  private:

      FIXS_CCH_UpgradeManager();

      virtual ~FIXS_CCH_UpgradeManager();

      void printError(std::string mess,const char *func,int line);

      FIXS_CCH_UpgradeManager(const FIXS_CCH_UpgradeManager &right);

      FIXS_CCH_UpgradeManager & operator=(const FIXS_CCH_UpgradeManager &right);

      ACS_TRA_trace* _trace;

      //	fetch magazine and slot number for each SCX board
      int fetchSCXInfo ();

      int fetchEPBInfo ();

      int fetchCMXInfo ();

      int fetchEVOETInfo ();

      int fetchBoardInfo ();

      //	fetch magazine and slot number for each NON SCX board
      int fetchIptInfo ();

      int fetchIPLBInfo ();
	  
      int fetchSMXInfo ();

      int startSWUpgradeFSMs ();

      int startPFMUpgradeFSMs ();

      int startIpmiUpgradeFSMs ();

      int startFwUpgradeFSMs ();

      int fetchBoardId (unsigned long magazine, unsigned short slot, unsigned short &boardId, /*ACS_CS_API_HWC_NS::HWC_FBN_Identifier*/ int fbn);

      int startIPTUpgradeFSMs ();

      //	fetch addresses for either SCB-RP or SCX for a given magazine
      int fetchSwitchAddresses (unsigned long magazine, unsigned long &slot0_ipA, unsigned long &slot0_ipB, unsigned long &slot25_ipA, unsigned long &slot25_ipB);

      int prepareScx (unsigned long umagazine, unsigned short uslot, std::string product);

      int prepareCmx (unsigned long umagazine, unsigned short uslot, std::string product);

      int prepareIpt (unsigned long magazine, unsigned short slot, std::string package, int upgradeType);

      int prepareIpt (unsigned long umagazine, std::string magazine, unsigned short uslot, std::string slot, std::string package, bool normalInstallationType);

      int prepareSmx (unsigned long umagazine, unsigned short uslot, std::string product);
	  
      int activateScx (unsigned long umagazine, unsigned short uslot);

      int prepareIplb (unsigned long magazine,unsigned short slot, std::string package, int upgradeType, bool force);
     
      int activateIplb (unsigned long umagazine, unsigned short uslot); 

      int activateIpt (unsigned long umagazine, unsigned short uslot);

      int activateCmx (unsigned long umagazine, unsigned short uslot);
	  
      int activateSmx (unsigned long umagazine, unsigned short uslot);

      int commitScx (unsigned long umagazine, unsigned short uslot);

      int commitCmx (unsigned long umagazine, unsigned short uslot);

      int commitIpt (unsigned long umagazine, unsigned short uslot, bool setAsDefaultPackage);
      int commitIplb (unsigned long umagazine, unsigned short uslot, bool setAsDefaultPackage);
	  
      int commitSmx (unsigned long umagazine, unsigned short uslot);

      int cancelScx (unsigned long umagazine, unsigned short uslot);

      int cancelIpt (unsigned long umagazine, unsigned short uslot);

      int cancelIplb (unsigned long umagazine, unsigned short uslot);

      int cancelCmx (unsigned long umagazine, unsigned short uslot);
	  
      int cancelSmx (unsigned long umagazine, unsigned short uslot);

      void cleanStateMachine();

      bool retrySetSoftwareVersion(std::string, std::string, short unsigned int);
	
      void removeUnusedFaultySwObjects();

      //applicable only for SMX
      void customizeFirmwareType(CCH_Util::fwRemoteData &info, int activeArea);	

      int getActiveFwAreaFromBoard (unsigned long magazine, std::string slot,int &value);
    // Data Members for Class Attributes

      static FIXS_CCH_UpgradeManager* m_instance;

      bool m_initialized;

      std::string m_currentSw;

      std::string m_currentState;

      std::string m_newSw;

      std::string m_upperFan;

      std::string m_upperPower;

      std::string m_lowerFan;

      std::string m_lowerPower;

      std::string m_upperFanFwRevision,m_upperTypeASwRevision;

      std::string m_upperPowerFwRevision,m_upperTypeBSwRevision;

      std::string m_lowerFanFwRevision,m_lowerTypeASwRevision;

      std::string m_lowerPowerFwRevision,m_lowerTypeBSwRevision;
	  
      std::string m_upperModuleType;

      std::string m_lowerModuleType;

      std::string m_upgradeDateTimeInfo;

      std::string m_upgradeResult;

      std::string m_upgradeReason;

      std::string m_ipmiResult;

      std::string m_ipmiUpgradeTime;

      std::string m_ipmiFwRevision;

      std::string m_ipmiFwType;

      static std::list<FIXS_CCH_UpgradeManager::FsmListData>* m_iptUpgradeList;

      std::string m_boardName;

      FIXS_CCH_AlarmHandler *m_alarmHandler;
      
      CCH_Util::EnvironmentType environment;

      int trapSubnet;

      bool trapLoadReleaseComplete;

      std::string trapLoadReleaseIp;

      bool trapIpmiUpgrade;

      std::string trapIpmiUpgradeIp;

      bool trapColdStartWarmStart;

      std::string trapColdStartWarmStartIp;

      bool trapPfmUpgrade;

      std::string trapPfmUpgradeIp;

      bool trapFwUpgrade;

      std::string trapFwUpgradeIp;

      ACS_TRA_Logging *FIXS_CCH_logging;

      bool m_backup;

    // Additional Private Declarations

  private:
    // Data Members for Associations

      static std::list<FIXS_CCH_UpgradeManager::FsmListData>* m_swUpgradeList;

    // Additional Implementation Declarations


};

// Class FIXS_CCH_UpgradeManager::FsmListData 

// Class FIXS_CCH_UpgradeManager 

#endif
