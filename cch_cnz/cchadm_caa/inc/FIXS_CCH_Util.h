
#ifndef FIXS_CCH_Util_h
#define FIXS_CCH_Util_h 1

#include <stdint.h>
#include <dirent.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string.h>
#include <map>
#include <vector>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

#include "ace/OS.h"

#include "FIXS_CCH_IMM_Util.h"
#include "Client_Server_Interface.h"


#include "ACS_APGCC_Util.H"
#include "ACS_APGCC_CommonLib.h"

#define DIR_SEPARATOR   '/'

# define ERROR -1

# define OPENDIR_ERROR   -1
# define LSTAT_ERROR     -2
# define REMOVE_ERROR    -3
# define CLOSEDIRR_ERROR -4

//#include "ACS_CS_API.h"

//#include "ACS_APGCC_Common_Define.h"
//#include "CDR_Base.h"


//ACS_APGCC_COMMON_DEFINE_H_

#define TMN_REPARSE_DATA_BUFFER_HEADER_SIZE \
			FIELD_OFFSET(TMN_REPARSE_DATA_BUFFER, SubstituteNameOffset)
#define MAX_NO_OF_INTERFACE 34
#define MAX_NO_OF_COUNTERS 38
#define MAX_NO_OF_VERSION_INDEX 20
#define DEFAULT_SW_VER_TYPE 2
#define MAX_ITER_TO_GET_CURRENTSW 2

//typedef uint16_t WORD;
typedef uint64_t LONGLONG;
typedef uint32_t DWORD;
//typedef uint16_t WCHAR;
//typedef uint8_t CHAR;

namespace CCH_Util
{

	typedef uint16_t WORD;
	typedef uint64_t LONGLONG;
	//typedef uint32_t DWORD;
	typedef uint16_t WCHAR;
	typedef uint16_t CHAR;

	# define MAX_PATH_CCH 1000

	// To avoid warning about unused parameter
	#define UNUSED(expr) do { (void)(expr); } while (0)

	#define DEBUG(str) do { \
		if (CCH_Util::isDebug()) { \
			std::cout << "DBG: "<< __FILE__ << "::" << __FUNCTION__ << "@" << __LINE__<< " - " << str << std::endl; \
		} \
	} while (0)

	//Backup and Restore
	const std::string BRF_PATH = "/usr/share/pso/storage-paths/";
	const std::string BRF_CLEAR_FILENAME = "clear";
	const std::string BRF_CCH_FILENAME = "fixs_cch";

	const std::string TEMP_FOLDER = "/tmp/temp_install";
	const std::string TEMP_FWFOLDER = "/tmp/temp_fw_install";

	static const char EVENT_NAME_SHUTDOWN_SERVICE_HANDLER[]    = "FIXS_CCH_SHUTDOWN_SERVICE_HANDLER_EVENT";
	static const char EVENT_NAME_SHUTDOWN[]    = "FIXS_CCH_SHUTDOWN";
	static const int Daemon_Timeout			 = 3;	 // Timeout 3 s, used by daemon

	static const std::string KEY_STATUS = "STATUS";

	static const std::string FAN_ID_STR = "0";
	static const std::string POWER_ID_STR = "1";

	static const std::string LOWER_ID_STR = "0";
	static const std::string UPPER_ID_STR = "1";

	//Software upgrade keys
	static const std::string KEY_CURRENT_CXC = "CURRENT_CXC";
	static const std::string KEY_CURRENT_CXP = "CURRENT_CXP";
	static const std::string KEY_EXPECTED_CXC = "EXPECTED_CXC";
	static const std::string KEY_EXPECTED_CXP = "EXPECTED_CXP";
	static const std::string KEY_LAST_UPGRADE_DATE = "LAST_UPGRADE_DATE";
	static const std::string KEY_LAST_UPGRADE_TIME = "LAST_UPGRADE_TIME";
	static const std::string KEY_LAST_ERROR_CODE = "LAST_ERROR_CODE";
	static const std::string KEY_LAST_UPGRADE_RESULT = "LAST_UPGRADE_RESULT";
	static const std::string KEY_LAST_UPGRADE_REASON = "LAST_UPGRADE_REASON";
	static const std::string KEY_LAST_UPGRADE_TYPE = "LAST_UPGRADE_TYPE";
	static const std::string KEY_LAST_UPGRADE_PREVIOUS_STATE = "LAST_UPGRADE_PREVIOUS_STATE";
	static const std::string KEY_LAST_UPGRADE_FAILURE_CODE = "LAST_UPGRADE_FAILURE_CODE";

	//PFM keys
	static const std::string KEY_MODULE = "MODULE";
	static const std::string KEY_TYPE = "TYPE";
	static const std::string KEY_PATH = "PATH";
	static const std::string KEY_UPGRADE_RESULT = "UPGRADE_RESULT";

	//Ipmi keys
	static const std::string KEY_MASTERIP = "IPMIMASTERIP";
	static const std::string KEY_MASTERSLOT = "IPMIMASTERSLOT";
	static const std::string KEY_SLOTSTARTED = "IPMISLOTSTARTED";

	//Performance Counters keys
	static const std::string KEY_TIME = "TIME";
	static const std::string KEY_INTERFACE = "IF";
	static const std::string KEY_COUNTER1 = "ifMTU";
	static const std::string KEY_COUNTER2 = "ifLastChange";
	static const std::string KEY_COUNTER3 = "ifPhysicalAddress";
	static const std::string KEY_COUNTER4 = "ifHighSpeed";
	static const std::string KEY_COUNTER5 = "ifCounterDiscontinuityTime";
	static const std::string KEY_COUNTER6 = "ifOperStatus";
	static const std::string KEY_COUNTER7 = "dot3statsDuplexStatus";
	static const std::string KEY_COUNTER8 = "ifHCInOctets";
	static const std::string KEY_COUNTER9 = "ifHCInUcastPkts";
	static const std::string KEY_COUNTER10 = "ifHCInMulticastPkts";
	static const std::string KEY_COUNTER11 = "ifHCInBroadcastPkts";
	static const std::string KEY_COUNTER12 = "ifHCInPkts";
	static const std::string KEY_COUNTER13 = "ifHCOutOctets";
	static const std::string KEY_COUNTER14 = "ifHCOutUcastPkts";
	static const std::string KEY_COUNTER15 = "ifHCOutMulticastPkts";
	static const std::string KEY_COUNTER16 = "ifHCOutBroadcastPkts";
	static const std::string KEY_COUNTER17 = "ifHCOutPkts";
	static const std::string KEY_COUNTER18 = "ifInErrors";
	static const std::string KEY_COUNTER19 = "ifOutErrors";
	static const std::string KEY_COUNTER20 = "ifOutDiscards";
	static const std::string KEY_COUNTER21 = "ifInDiscards";
	static const std::string KEY_COUNTER22 = "ifInUnknownProtos";
	static const std::string KEY_COUNTER23 = "dot3HCStatsAlignmentErrors";
	static const std::string KEY_COUNTER24 = "dot3HCStatsFCSErrors";
	static const std::string KEY_COUNTER25 = "dot3StatsSingleCollisionFrames";
	static const std::string KEY_COUNTER26 = "dot3StatsMultipleCollisionFrames";
	static const std::string KEY_COUNTER27 = "dot3StatsSQETestErrors";
	static const std::string KEY_COUNTER28 = "dot3StatsDeferredTransmissions";
	static const std::string KEY_COUNTER29 = "dot3StatsLateCollisions";
	static const std::string KEY_COUNTER30 = "dot3StatsExcessiveCollisions";
	static const std::string KEY_COUNTER31 = "dot3HCStatsInternalMacTransmitErrors";
	static const std::string KEY_COUNTER32 = "dot3StatsCarrierSenseErrors";
	static const std::string KEY_COUNTER33 = "dot3HCStatsFrameTooLongs";
	static const std::string KEY_COUNTER34 = "dot3HCStatsInternalMacReceiveErrors";
	static const std::string KEY_COUNTER35 = "dot3HCStatsSymbolErrors";
	static const std::string KEY_COUNTER36 = "dot3HCControlInUnknownOpcodes";
	static const std::string KEY_COUNTER37 = "dot3HCInPauseFrames";
	static const std::string KEY_COUNTER38 = "dot3HCOutPauseFrames";

	//RFU SMX
	const std::string SMX_FWTYPE_A_CXP = "CXP9030857/2";
	const std::string SMX_FWTYPE_B_CXP = "CXP9030857/3";
	enum FBN {
		SCXB			= 0,
		IPTB			= 1,
		EPB1          	= 2,
		EVOET         	= 3,
		CMXB         	= 4,
		IPLB            = 5,
		SMXB			= 6,
		NONE			= 7
	};

	enum SwErrors {
		SW_ERR_OK								= 0,
		SW_ERR_LOAD_RELEASE_IN_PROGRESS			= 1,
		SW_ERR_LOAD_RELEASE_IN_SUCCESS			= 2,
		SW_ERR_LOAD_RELEASE_HOST_NOT_FOUND		= 3,
		SW_ERR_LOAD_RELEASE_LOGIN_INCORRECT		= 4,
		SW_ERR_LOAD_RELEASE_NO_SUCH_DIRECTORY	= 5,
		SW_ERR_LOAD_RELEASE_NO_SUCH_FILE		= 6,
		SW_ERR_LOAD_RELEASE_ERRONEOUS_URL		= 7,
		SW_ERR_LOAD_RELEASE_ERROR				= 8,
		SW_ERR_REMOVE_RELEASE_IN_PROGRESS		= 9,
		SW_ERR_REMOVE_RELEASE_SUCCESS			= 10,
		SW_ERR_REMOVE_RELEASE_ERROR				= 11,
		SW_ERR_STATUS_UNINITIATED				= 12,
		SW_ERR_END								= 13,
		SW_ERR_CANCELLED						= 14,
		SW_ERR_OLD_SOFTWARE_STILL_ACTIVE		= 15,
		SW_ERR_COMMIT_ERROR						= 16,
		SW_ERR_AUTO_FAILURE						= 17,
		SW_ERR_MANUAL_FAILURE					= 18
	};

	enum LmErrors {
		LM_ERR_OK								= 100,
		LM_ERR_AUTOMATIC_ROLLBACK_SUCCESS		= 101,
		LM_ERR_MANUAL_ROLLBACK_SUCCESS			= 102,
		LM_ERR_CANCEL    						= 103,
		LM_ERR_COMMIT_ERROR						= 104,
		LM_ERR_ACTIVATE_ERROR		    		= 105,
		LM_ERR_MANUAL_ROLLBACK_FAILURE			= 106,
		LM_ERR_AUTOMATIC_ROLLBACK_FAILURE		= 107
	};

	enum LoadModuleUpgradeStatus {
		STATE_IDLE			= 0,
		STATE_PREPARING		= 1,
		STATE_PREPARED		= 2,
		STATE_ACTIVATING	= 3,
		STATE_ACTIVATED		= 4,
		STATE_MANUAL_ROLLBACK_ONGOING	= 5,
		STATE_MANUAL_ROLLBACK_ENDED		= 6,
		STATE_AUTOMATIC_ROLLBACK_ONGOING = 7,
		STATE_AUTOMATIC_ROLLBACK_ENDED		= 8,
		STATE_UNDEFINED		= 9,
		STATE_FAILED		= 10
	};

	enum SwStatus {
		SW_IDLE						= 0,
		SW_PREPARING				= 1,
		SW_PREPARED					= 2,
		SW_ACTIVATING				= 3,
		SW_ACTIVATED				= 4,
		SW_MANUAL_ROLLBACK_ONGOING	= 5,
		SW_MANUAL_ROLLBACK_ENDED	= 6,
		SW_AUTOMATIC_ROLLBACK_ONGOING = 7,
		SW_AUTOMATIC_ROLLBACK_ENDED = 8,
		SW_UNKNOWN					= 9,
		SW_FAILED					= 10

		//SW_CANCELING				= 9

	};

    enum PFMStatus {
		PFM_IDLE		= 0,
		PFM_ONGOING		= 1
	};

    enum PFMResult {
   		PFM_RES_OK			= 0,
   		PFM_RES_NOT_OK		= 1
   	};

	enum PFMFwStatus {
		PFM_FW_STATUS_OK			= 0,
		PFM_FW_STATUS_NOT_OK		= 2
	};

	enum IpmiStatus {
		IPMI_IDLE		= 0,
		IPMI_ONGOING	= 1
	};

	enum IpmiResult {
		IPMI_OK		= 0,
		IPMI_NOT_OK	= 1
	};

	enum PFMModule {
		LOWER		= 0,
		UPPER		= 1
	};
 
	enum PFMType {
		FAN		= 0,
		POWER	= 1
	};
	
	enum PFMUpgType {
		typeA	= 0,
		typeB	= 1
	};

	enum PFMSwType {
		BL	= 0,
		UPG	= 1,
		FB =  2
	};
	enum EnvironmentType {  //environment enum
		UNKNOWN				= 0,
		SINGLECP_NOTCBA		= 1,
		SINGLECP_CBA		= 2,
		MULTIPLECP_NOTCBA	= 3,
		MULTIPLECP_CBA		= 4,
		SINGLECP_DMX		= 5,
		MULTIPLECP_DMX		= 6,
		VIRTUALIZED			= 7,
		MULTIPLECP_SMX		= 8,
		SINGLECP_SMX		= 9
	};
	enum APGtype {
		Unknown = 0,
		AP1 = 1,
		AP2 = 2
	};

	enum FwStatus {
		FW_IDLE		= 0,
		FW_ONGOING	= 1,
		FW_UNDEFINED= 2
	};

	enum RpUpgradeResult {

		SW_RP_OK		= 0,
		SW_RP_NOT_OK	= 1
	};

	enum BoardUpgradeType{
		NORMAL_UPGRADE = 0,
		INITIAL_INSTALLATION = 1
	};
	enum RpUpgradeResultInfo {

		SW_RP_EXECUTED 				= 0,
		SW_RP_PREPARE_FAILURE		= 1,
		SW_RP_ACTIVATE_FAILURE		= 2,
		SW_RP_ROLLBACK_FAILURE		= 3,
		SW_RP_CANCEL_FAILURE		= 4,
		SW_RP_MANUALLY_CANCELLED	= 5,
		SW_RP_COMMIT_FAILURE		= 6
	};
       enum DmxNetconfErrors {
                CCH_ERR_NO_ERRORS              = 0, 
                CCH_ERR_DMX_MAGAZINE_NOT_FOUND = 1,
                CCH_ERR_IP_ADDRESS_CONVERSION  = 2,
                CCH_ERR_DMX_TENANT_NOT_FOUND   = 3,
                CCH_ERR_DMX_GET_BOARD_PWR_STATUS_FAILURE = 4,
                CCH_ERR_DMX_SET_BOARD_PWR_STATUS_FAILURE = 5,
                CCH_ERR_DMX_GET_BOARD_LED_STATUS_FAILURE = 6,
                CCH_ERR_DMX_SET_BOARD_LED_STATUS_FAILURE = 7,
                CCH_ERR_DMX_GET_BOARD_STATUS_FAILURE     = 8,
                CCH_DMX_BOARD_ALREADY_BLOCKED            = 9      
   
        }; 

	enum CpPrepareUpgradeType {
		PREPARE_MAG_SLOT  = 1,
		PREPARE_SHELF     = 2,
		PREPARE_ALL       = 3,
		PREPARE_SLOT      = 4
	};
	/***********************************************************************************
		# IPMI settings/description:
		#
		# x = 0 for flash boot
		#   = 1 for network boot
		#
		# Mask       Value      Description
		# ----       -----      -----------
		# 0000ffff   00000x11   Fetching TFTP server address from APG
		# 0000ffff   00000x21   Checking access of /usr/local/bin/dhcpinfo
		# 0000ffff   00000x22   Fetching DHCP information from APG
		# 0000ffff   00000x23   Checking Bootfile Name (OPTION67)
		# 0000ffff   00000x24   Checking NTP Server (OPTION42)
		# 0000ffff   00000x31   Downloading tar-file from APG
		# 0000ffff   00000x41   Extracting of tar-file
		# 0000ffff   00000x42   Check after extracting tar-file
		# 0000ffff   00000x51   Create a snapshot of existing RPM configuration
		# 0000ffff   00000x61   Starting uninstall of RPM configuration
		# 0000ffff   00000x62   Restoring snapshot after failed uninstall
		# 0000ffff   00000x63   Starting sync after successful restoring of snapshot
		# 0000ffff   00000x64   Starting activating after successful uninstall
		# 0000ffff   00000x65   Restoring snapshot after failed activation
		# 0000ffff   00000x66   Starting sync after successful restoring of snapshot
		# 0000ffff   00000x71   Starting install of RPM configuration
		# 0000ffff   00000x72   Restoring snapshot after failed install
		# 0000ffff   00000x73   Starting sync after successful restoring of snapshot
		# 0000ffff   00000x7d   Starting download of OS RPM
		# 0000ffff   00000x7e   Restoring snapshot after failed download of OS RPM
		# 0000ffff   00000x7f   Starting sync after successful restoring of snapshot
		# 0000ffff   00000x81   Starting upgrading of OS RPM
		# 0000ffff   00000x82   Restoring snapshot after failed upgrade of OS RPM
		# 0000ffff   00000x83   Starting sync after successful restoring of snapshot
		# 0000ffff   00000x91   Starting activating after successful install
		# 0000ffff   00000x92   Restoring snapshot after failed activation
		# 0000ffff   00000x93   Starting sync after successful restoring of snapshot
		# 0000ffff   00000xa1   Initiating a reboot after OS RPM upgrade
		# 0000ffff   0000ffff   Successful upgrade of IPT
		***********************************************************************************/

	enum GPRDataMeaning
	{
		GPR_Fetching_TFTP_server_address				= 0,
		GPR_Checking_access_of_dhcpinfo					= 1,
		GPR_Fetching_DHCP_information					= 2,
		GPR_Checking_Bootfile_Name						= 3,
		GPR_Checking_NTP_Server							= 4,
		GPR_Downloading_tar_file						= 5,
		GPR_Extracting_tar_file							= 6,
		GPR_Check_after_extracting_tar_file				= 7,
		GPR_Create_snapshot_of_existing_RPM				= 8,
		GPR_Starting_uninstall_RPM						= 10,
		GPR_Restoring_snapshot_after_failed_uninstall	= 11,
		GPR_Starting_sync_after_restoring_snapshot		= 12,
		GPR_Starting_activating_after_uninstall			= 13,
		GPR_Restoring_snapshot_after_failed_activation	= 14,
		GPR_Starting_install_RPM_configuration			= 15,
		GPR_Restoring_snapshot_after_failed_install		= 16,
		GPR_Downloading_OS_RPM							= 17,
		GPR_Restoring_snapshot_after_failed_download_OS_RPM	= 18,
		GPR_Upgrading_OS_RPM							= 19,
		GPR_Restoring_snapshot_after_failed_upgrade_OS_RPM	= 20,
		GPR_Starting_activating_after_install			= 21,
		GPR_Initiating_reboot_after_OS_RPM_upgrade		= 22,
		GPR_Successful_upgrade_IPT						= 23,
		GPR_Unknown										= 24
	};


	// Name of service
	static const char FIXS_CCH_PROCESS_NAME[] = "FIXS_CCH_Service";

	enum Severity
	{
	Severity_A1			=	0,
	Severity_A2			=	1,
	Severity_CEASING	=	2,
	Severity_O1			=	3
	};
	
	enum AlarmTimeout
	{
		NO_ALARM_TIMEOUT = 0,
		SCX_ALARM_TIMEOUT = 1,
		CMX_ALARM_TIMEOUT = 2,
		SMX_ALARM_TIMEOUT = 3
	};

	enum TypeOfPrepare
	{
		MULTIPLE_BOARD = 1,
		SINGLE_BOARD = 2,
		ALL_MAGAZINE_BOARD = 3
	};

	typedef struct
	{
		unsigned char mag;
		unsigned char slot;
		unsigned char type;
	} RecOfMaintenance;

	typedef struct
	{
		unsigned long magazine;
		unsigned short slot;
		std::string currentSW;
		std::string commentSW;
	} SWINFO_FBN;
	typedef struct
	{
		SwStatus upgradeStatus;
		std::string currentCxc;
		std::string currentCxp;
		std::string expectedCxc;
		std::string expectedCxp;
		std::string lastUpgradeDate;
		std::string lastUpgradeTime;
		RpUpgradeResult lastUpgradeResult;
		RpUpgradeResultInfo lastUpgradeReason;
		std::string dnBladePersistance;
		int lastErrorCode;
		int swState;
		int previousState;
		int failureCode;
		bool forcedUpgrade;
		BoardUpgradeType upgradeType;

	} SWData;

	typedef struct
	{
		//SwStatus upgradeStatus;
		LoadModuleUpgradeStatus upgradeStatus;
		std::string currentCxc;
		std::string currentCxp;
		std::string expectedCxc;
		std::string expectedCxp;
		std::string lastUpgradeDate;
		std::string lastUpgradeTime;
		RpUpgradeResult lastUpgradeResult;
		RpUpgradeResultInfo lastUpgradeReason;
		std::string dnBladePersistance;
		int lastErrorCode;
		int lastUpgradeType; //std::string lastUpgradeType;
		int previousState;
		int failureCode;
		int lmState;
		bool forcedUpgrade;
		int upgradeType;
	} BoardSWData;

	typedef struct
	{
		PFMStatus upgradeStatus;
		PFMStatus upgradePowerStatus;
		PFMStatus upgradeFanStatus;
		PFMModule module;
		PFMType type;
		std::string path;
		std::string dn_pfm;
		PFMResult upgradePowerResult;
		PFMResult upgradeFanResult;
	} PFMData;

	typedef struct
	{
		std::string pfmCxpProductNum;
		std::string pfmCxpRevision;
		std::string pfmCxcProductNum;
		std::string pfmCxcRevision;
	} pfmData;
	
	typedef struct
	{
		IpmiStatus upgradeStatus;
		std::string ipMasterSCX;
		std::string ipPassiveSCX;
		std::string slotMasterSCX;
		std::string path;
		IpmiResult upgradeResult;
		unsigned short slotUpgOnGoing;
		std::string dn_ipmi;
	} IpmiData;

	typedef struct
	{
		std::string fwType[MAX_NO_OF_VERSION_INDEX];
		std::string fwVersion[MAX_NO_OF_VERSION_INDEX];
		std::string productno;
		std::string cxp_rState;
	} fwRemoteData;

	typedef struct
	{
		FwStatus upgradeStatus;
		std::string ipMasterSCX;
		std::string slotMasterSCX;
		unsigned short slotUpgOnGoing;
		std::string lastUpgradeDate;
		std::string lastUpgradeTime;
		std::string result;
		std::string lastUpgradeReason;
		fwRemoteData fwInfo;
	} FwData;

	typedef struct
	{
		std::string timeValue;
		std::string interfaceName[MAX_NO_OF_INTERFACE];
		std::string counterValue[MAX_NO_OF_INTERFACE][MAX_NO_OF_COUNTERS];
	} PCData;


	struct TMN_REPARSE_DATA_BUFFER
	{

		DWORD  ReparseTag;
		WORD   ReparseDataLength;
		WORD   Reserved;

		// IO_REPARSE_TAG_MOUNT_POINT specifics follow
		WORD   SubstituteNameOffset;
		WORD   SubstituteNameLength;
		WORD   PrintNameOffset;
		WORD   PrintNameLength;
		WCHAR  PathBuffer[1];
	};

	enum trapType
	{
		TrapNull 			= 0,
		LoadReleaseComplete = 1,
		IpmiUpgrade			= 2,
		ColdStartWarmStart	= 3,
		PfmUpgrade			= 4,
		FwUpgrade			= 5
	};

	enum mapBoardType
	{
		ALL_MAPS_BLADE	= 0,
		MAP_SCXB 		= 1,
		MAP_EPB1		= 2,
		MAP_IPTB		= 3,
                MAP_IPLB                = 4
	};

	enum operationCallBackType
	{
		createT = 1,
		deleteT = 2,
		modifyT = 3
	};

	enum upgradeActionType
	{
		unknownT		 = 0,
		prepareT		 = 1,
		activateT		 = 2,
		cancelT			 = 3,
		commitT			 = 4,
		prepareMultiT	 = 5,
		prepareAllT		 = 6,
		insertT			 = 7,
		defaultT		 = 8,
		prepareEpb1T	 = 9,
		prepareEvoEtT	 = 10
	};

	enum fileMFolderType
	{
		swPackage		 = 0,
		swPackageScxb	 = 1,
		swPackageEpb1	 = 2,
		swPackageIptb	 = 3,
		swPackageEvoEt	 = 4,
		swPackageCmxb	 = 5,
		swPackageIplb    = 6,
		swPackageFw		 = 7,
		swPackageCp		 = 8,
		swPackageSmxb	 = 9
	};

	bool stringToUlongMagazine (std::string value, uint_fast32_t &uvalue);
	bool ulongToStringMagazine (uint32_t uvalue, std::string &value);
	bool stringToUshort (std::string value, unsigned short &uvalue);
	bool ushortToString (unsigned short uvalue, std::string &value);
	bool longlongToString (LONGLONG uvalue, std::string &value);
	bool longToString (long uvalue, std::string &value);
	bool stringToLong (std::string value, long &uvalue);
	std::string ulongToStringIP (unsigned long ipaddress);
	bool CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists, long int size);
	bool PathFileExists(const char* file);

	std::string intToString (int ivalue);
	bool stringToInt (std::string value, int &ivalue);
	bool gprStringToHex (std::string value, int &hexvalue);

	unsigned short numOfChar (std::istringstream &istr, char searchChar);
	void reverseDottedDecStr (std::istringstream &istr);
	bool dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4);
	bool checkFigures (std::istringstream &istr);
	std::string getSwSectionName(unsigned long umagazine, unsigned short uslot);
	std::string getPFMSectionName(unsigned long umagazine, unsigned short uslot);
	std::string getIpmiSectionName(unsigned long umagazine, unsigned short uslot);
	std::string getPcSectionName(unsigned long umagazine, unsigned short uslot);
	bool stringToUpper (std::string& stringToConvert);

	bool intToUshort (int value, unsigned short &ivalue);

	bool createDir (const char* dirpath, DWORD& err);

	void getCurrentTime(std::string& dateStr, std::string& timeStr);
	std::string getCurrentTimeInSeconds();
	void getResultReason(std::string& resultValue, std::string& reasonValue, int code);

	void getResultReason(int& resultValue, int& reasonValue, int code);

	
	std::string replaceSubstr( const std::string& content, const std::string& instance, const std::string& replacement );
//	EnvironmentType getEnvironment(void);
	APGtype getAPGtype(void);
	std::string getEnvironmentType(EnvironmentType env);
	std::string GetAPHostName();
	EnvironmentType isRightIptEnvironment(void);
	bool findRfuDataFile (std::string &filenametz, std::string folder);
	bool findJsonFile (std::string &filenameJson, std::string folder);
	// Create a symbolic link
	int createSymbolicLink(
		const std::string& link,			// Symbolic link
		const std::string& dir				// Destination directory
		);


	// Convert multibyte character string to wide character string
	std::wstring mbsToWcs(			// Returns converted string
			const std::string& str	// String to convert
			);

	int getMeaningGPRData (std::string value);
	std::string traslateGPRDataValue (int code);
	bool isFlashValue (std::string value);
	bool isNetworkValue (std::string value);
        bool isSymbolicLinkExists(const char *file);
	bool stringTolonglong (std::string value, LONGLONG &ivalue);
	bool doubleToString (double dvalue, std::string &value);
	LONGLONG getEpochTimeInSeconds();
	int calculateDigit(double x);
	int getMeaningScxGPRData (std::string value);
	bool gprScxStringToHex (std::string value, int &hexvalue);
	//char **findFile(char * dir,char *type_file,int *lun);
	int findFile(char* path, char* type, std::vector<std::string> &result);
	int removeFolder(const char* _fullpath);
	bool CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists);
	bool CopyFolder(const char* srcFolder,const char* dstFolder);
	bool CheckExtention(const char* file,const char* type);
	void stringIpToUlong(const char *ip, unsigned long &uvalue);
	void findAndReplace( string &source, const string find, string replace);
	void trim(string& str);
	void findAndRemove( string &source, const string find);
	bool findMetadataFile (std::string &filenameSh, std::string folder, bool isSmx);
	bool findXmlFile (std::string &filenameXml, std::string folder);
	bool deleteFile(const char *file);
	bool renameFilewithMagAndSlot(std::string &filename, std::string mag, std::string slot);
	bool removeWhiteChar(std::string &value);
	void getResultImportAction(respCode result, int &resultSwStruct, int &resultInfoSwStruct);
	void getResultUpgradeAction(int result, int &resultSwStruct, int &resultInfoSwStruct);
	bool getFolderList (std::vector<std::string> &listFolder, std::string folder_parent);
	bool findFile (std::string fileName, std::string folder);
	void readFile (std::string filePath, std::string &contentFile);
	void getUsedMagAndSlot (std::string usedValue, int &magSlot);
	void sortUsedVector (std::vector<std::string> &usedValue);
	bool createFile(std::string complete_path);
	int isBoardPresent (std::string magazine, std::string slot);
	bool isDebug();
	void setDebug(bool isDebug);
	int getCmxSoftwareVersion (unsigned long umagazine,unsigned short uslot, int & sw_version);
} // End of namespace

//static bool isProgress=false;

#endif
