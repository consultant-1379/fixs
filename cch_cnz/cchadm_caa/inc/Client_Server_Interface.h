#ifndef _CLIENT_SERVER_INTERFACE_H1_
#define _CLIENT_SERVER_INTERFACE_H1_
//#include <winsock2.h>
//#include <sys/types.h> //To be define
//#include <sys/stat.h>  //To be define


#define SERVICE_NAME "CCH";
#define SERVICE_DOMAIN "FIXS";
const int MAX_DATA = 256;
const int ITEM_DATA = 64;
const int MAX_RESPONSE_SIZE = 4096;
const int ITEM_SIZE = 128;


enum respCode
{
	//upgradepackage
	EXECUTED				= 0,	//EXECUTED
	INTERNALERR				= 1,	// Generic internal error
	INCORRECTUSAGE			= 2,	// Usage
	DIRECTORYERR			= 3,	// "Directory Error" Internal Error during filesystem managing
	PACKAGE_IN_USE			= 4,	// "SW Package in use" The package is used then it can not be removed
	PACKAGE_NOT_EXIST		= 5,	// "SW Package not exists" The package does not exist
	XMLERROR     			= 6,	// "XML Error" XML is not formatted properly
	NO_REPLY_FROM_CS		= 7,    // "No reply from CS" cannot fetch information from CS

	UPGRADE_ALREADY_ONGOING		= 8,	// "Upgrade already ongoing" : an upgrade is already ongoing on specified board or magazine
	SW_ALREADY_ON_BOARD			= 9,	// "Specified SW already present on board"
	WRONG_OPTION				= 10,	// "Wrong option in this state"	
	PACKAGE_ALREADY_INSTALLED	= 11,	// "SW package already installed"
	UPGRADE_ACTIVATE_FAILED		= 12,	// "Activate failure"
	SNMP_FAILURE				= 13,	// "SNMP Failure"
	HARDWARE_NOT_PRESENT		= 14,	// "Hardware not present"
	PACKAGE_NOT_INSTALLED		= 15,	// "Package not installed"
	INCORRECT_PACKAGE			= 16,	// "Package not correct"
	WAIT_FOR_UPGRADE_ONGOING	= 17,	// "Display command is inhibited during upgrade fw ongoing"	 
	NOT_ALLOWED_UPGRADE_ONGOING	= 18,	// "Delete option is inhibited during upgrade sw ongoing"	 
	FUNC_BOARD_NAME_ERROR		= 19,	// "Functional Board name error"
	PACKAGE_IS_DEFAULT			= 20,	// "SW Package in default" The package is setted as default then it can not be removed
	MAXIMUM_LOAD_MODULES_INSTALLED	= 21,	// "Maximum Number of Load Modules are already Installed"
	FUNC_BOARD_NOTALLOWED		= 22,	// "Not Allowed for this Functional Board Name"
	SLOT_FBN_NOT_SAME			= 23, 	//"All slots specified do not contain same type of board"
	DISK_FULL					= 24, 	//"Disk quota exceeded"
	PACKAGE_NOT_APPLICABLE		= 25, 	//"Load Module not Applicable for this Functional Board Name"
	FW_ALREADY_INSTALLED		= 26,	//"Firmware already installed"
	NEIGHBOR_SCX_NOT_PRESENT	= 27,	// "NEIGHBOR SCX not present"
	NOT_ALLOWED_BY_CP        	= 28,	// "CP rejected AP SW/FW upgrade requests "
	PFM_ALREADY_INSTALLED		= 29,    // " Pfm package already installed"
	
	//Boardreset codes
	INCORRECT_MAGAZINE			= 30, //"Incorrect magazine number"
	INCORRECT_SLOT				= 31, //"Incorrect slot number"
	INCORRECT_RESPONSE			= 32, //"Illegal value - enter Yes/No (or) Y/N" Incorrect response to confirmation question
	BOARD_BLOCKED_NO_RESET      = 33,   //Board cannot be reset as it is blocked
	BOARD_ALREADY_DEBLOCKED		= 34,	//Board in not in blocked state.So need not deblock
	BOARD_ALREADY_BLOCKED		= 35,	//Board is already blocked
	OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE     = 36,   //Action not allowed other IPLB is not active
	
	ONLY_ALLOWED_FROM_IO_APG	= 40,	//Only allowed from IO APG

	// Performance Counter codes
	NO_INTERFACE_UP				= 41,
	NOT_EXIST_INTERFACE			= 42,
	BASELINE_FILE_NOT_FOUND		= 43,
	FILE_DELETION_FAILED		= 44,

	//IPMI upgrade
	IPMIUPG_READY = 50,
	IPMIUPG_ONGOING = 51,
	IPMIUPG_MIBSTATUS_READY = 52,
	IPMIUPG_MIBSTATUS_ONGOING = 53,
	IPMIUPG_MIBSTATUS_NOTSTARTED = 54,
	IPMIUPG_MIBSTATUS_UNKNOWN = 55,
	IPMIUPG_MIBTRAP_OK = 56,
	IPMIUPG_MIBTRAP_FILE_ERR = 57,
	IPMIUPG_MIBTRAP_OTHER_ERR = 58,

	//on load module upgrade
	WRONG_OPTION_PREPARE		= 59,
	WRONG_OPTION_ACTIVATE		= 60,
	WRONG_OPTION_COMMIT			= 61,
	WRONG_OPTION_CANCEL			= 62,

	//remote fw
	FWUPG_READY = 63,
	FWUPG_ONGOING = 64,
	FWUPG_MIBSTATUS_READY = 65,
	FWUPG_MIBSTATUS_ONGOING = 66,
	FWUPG_MIBSTATUS_NOTSTARTED = 67,
	FWUPG_MIBSTATUS_UNKNOWN = 68,
	FWUPG_MIBTRAP_OK = 69,
	FWUPG_MIBTRAP_FILE_ERR = 70,
	FWUPG_MIBTRAP_OTHER_ERR = 71,
	FWUPG_MIBTRAP_ONGOING_ERR = 72,

	//PFM
	PFM_INCORRECT_TYPE = 73,
	PFM_INCORRECT_MODULE = 74,

	//Backup
	NOT_ALLOWED_BACKUP_ONGOING = 75, //Not allowed, backup ongoing

	ABORTED_BY_USER=76,

	//generic codes
	FAILED_OPERATION = 114,
	ILLEGAL_COMMAND	= 115,	// Illegal command in this system configuration
	UNABLECONNECT	= 117,	// Unable to connect to server
	UNABLECONNECT_PASSIVE = 122, //Illegal command on passive node
	ONLY_ALLOWED_FROM_LOCAL_MAGAZINE = 123, //Local magazine number expected in this system configuration
	ILLEGAL_OPTION_SYS_CONFIG = 124, //Illegal option in this system configuration

	//IMM exit Code
	DELETE_FAULTY_PACKAGE		 	 		 = 1000,
	DELETE_DEFAULT_PACKAGE					 = 1001,
	DELETE_USED_PACKAGE			 		 	 = 1002,
	DELETE_UPGRADE_ONGOING					 = 1003,
	DELETE_INVALID_FBN						 = 1004,
	CREATE_INVALID_FBN						 = 1005,
	FAILED_REMOVE_FOLDER					 = 1006,
	FAULTY_PACKAGE							 = 1007,
	FAILED_HARDWARE_NOT_PRESENT				 = 1008,
	FAILED_INVALID_PARAMETERS				 = 1009,
	FAILED_FBN_NOT_ALLOWED					 = 1010,
	FAILED_IMPORT_ACTION_ALREADY_RUNNING	 = 1011,
	FAILED_INVALID_CONFIGURATION			 = 1012,
	FAILED_PATH_NOT_EXIST					 = 1013,
	FAILED_WRONG_EXTENSION					 = 1014,
	FAILED_INVALID_FILENAME					 = 1015,
	FAILED_INVALID_FBN						 = 1016,
    FAILED_SNMP_FAILURE                      = 1017,
    BOARD_BLOCKED_NO_DEFAULT_RESET           = 1018,
    IRONSIDE_FAILURE							 = 1019,
    MODIFY_UPGRADE_ONGOING                   = 1020,
        BOARD_BLOCKED                        =1021,
	BOARD_DEBLOCKED                      =1022,
        FAILURE_OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE     =1023,
	NOTALLOWED_AP2                       =1024 
	

};

enum CCHCommandType
{
	cmdFwUpgrade = 8,  //upgradepfm -u "filename" -m "magazine" -t "PFMtype" -r "module"
	cmdFwDisplay = 9,  //upgradepfm -f -m "magazine"
	cmdFwStatus = 10,  //upgradepfm -s -m "magazine"
	cmdRemoteFwStatus = 12, //upgradeRemoteFw Status
	cmdRemoteFwUpgrade = 13, //upgradeRemoteFw exec
	cmdIPMIUpgrade = 100,  //ipmiupgexec using slot 0 or 25
	cmdIPMIDisplay = 101,  //ipmifwprint
	cmdIPMIStatus = 102  //ipmiupgexec -i
};

typedef struct
{
	CCHCommandType		cmdType;
	char                data[MAX_DATA]; //command parameter
	char				boardName[ITEM_DATA]; //functional board name
} cch_upgrade_request;

typedef struct
{
	CCHCommandType		cmdType;
	char                extendType[ITEM_DATA]; //t||v parameter
	char                data[MAX_DATA]; //command parameter
	char				magazine[ITEM_DATA]; //magazine parameter
	char                slot[ITEM_DATA]; //slot parameter
	int                 fbn; //FBN parameter
	bool                partialInstallation; //true means partial (normal) upgrade 

} cch_upgrade_request_sw_fw;

typedef struct
{
	CCHCommandType		cmdType;
	char                data[MAX_DATA]; //command parameter
	char				magazine[ITEM_DATA]; //magazine parameter
	char                pfmType[ITEM_DATA]; //pfmType
	char                pfmModule[ITEM_DATA]; //module

} cch_upgrade_request_pfm;

typedef struct
{
	respCode response;
	int numItem;
	char data[MAX_RESPONSE_SIZE];
} cch_list_response;

typedef struct
{
	CCHCommandType		cmdType;
	char                valueToSet[MAX_DATA]; //command parameter
	char                ipSCXMaster[MAX_DATA]; //command parameter
	char                magAddr[MAX_DATA]; //command parameter
	char                slotMaster[ITEM_DATA]; //slot master
	char                slot[ITEM_DATA]; //slot parameter
	
} cch_ipmiupg_request;

typedef struct
{
	CCHCommandType		cmdType;
	char                data[MAX_DATA]; //command parameter
	char				magazine[ITEM_DATA]; //magazine parameter
	char                slot[ITEM_DATA]; //slot parameter
} cch_upgrade_request_fw;

#endif
