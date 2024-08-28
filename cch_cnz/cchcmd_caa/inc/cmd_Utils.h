/*
 * cmd_Utils.h
 *
 *  Created on: Feb 29, 2012
 *      Author: eanform
 */

#ifndef CMD_UTILS_H_
#define CMD_UTILS_H_


#include <stdint.h>
#include <dirent.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
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

#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"
#include "ACS_CC_Types.h"
#include "ACS_APGCC_Util.H"
#include "ACS_APGCC_CommonLib.h"
#include "acs_apgcc_omhandler.h"
#define DIR_SEPARATOR   '/'

# define ERROR -1

# define OPENDIR_ERROR   -1
# define LSTAT_ERROR     -2
# define REMOVE_ERROR    -3
# define CLOSEDIRR_ERROR -4


#define TMN_REPARSE_DATA_BUFFER_HEADER_SIZE \
			FIELD_OFFSET(TMN_REPARSE_DATA_BUFFER, SubstituteNameOffset)
#define MAX_NO_OF_INTERFACE 286
#define MAX_NO_OF_COUNTERS 38

#define IMM_DN_PATH_SIZE_MAX 512
//typedef uint16_t WORD;
typedef uint64_t LONGLONG;
typedef uint32_t DWORD;
//typedef uint16_t WCHAR;
//typedef uint8_t CHAR;

namespace CMD_Util
{


	static const string NAMESPACE_CS= "AxeEquipment";


	typedef uint16_t WORD;
	typedef uint64_t LONGLONG;
	typedef uint32_t DWORD;
	typedef uint16_t WCHAR;
	typedef uint16_t CHAR;

	# define MAX_PATH_CCH 1000

	// To avoid warning about unused parameter
	#define UNUSED(expr) do { (void)(expr); } while (0)

	const std::string TEMP_FOLDER = "/tmp/temp_install";

	static const char EVENT_NAME_SHUTDOWN_SERVICE_HANDLER[]    = "FIXS_CCH_SHUTDOWN_SERVICE_HANDLER_EVENT";
	static const char EVENT_NAME_SHUTDOWN[]    = "FIXS_CCH_SHUTDOWN";
	static const int Daemon_Timeout			 = 3;	 // Timeout 3 s, used by daemon

	static const std::string KEY_STATUS = "STATUS";

	static const std::string FAN_ID_STR = "0";
	static const std::string POWER_ID_STR = "1";

	static const std::string LOWER_ID_STR = "0";
	static const std::string UPPER_ID_STR = "1";

	//PFM keys
	static const std::string KEY_MODULE = "MODULE";
	static const std::string KEY_TYPE = "TYPE";
	static const std::string KEY_PATH = "PATH";
	static const std::string KEY_UPGRADE_RESULT = "UPGRADE_RESULT";

	//Ipmi keys
	static const std::string KEY_MASTERIP = "IPMIMASTERIP";
	static const std::string KEY_MASTERSLOT = "IPMIMASTERSLOT";
	static const std::string KEY_SLOTSTARTED = "IPMISLOTSTARTED";

	enum FBN {
		SCXB			= 0,
		IPTB			= 1,
		EPB1          	= 2,
		EVOET         	= 3,
		CMXB         	= 4,
		NONE			= 5
	};

	//board type
	static const int FBN_SCXB = 340;
	static const int FBN_IPTB = 350;
	static const int FBN_EPB1 = 360;
	static const int FBN_EVOET = 370;
	static const int FBN_CMXB = 380;

	enum NodeArchitecture {

		EGEM_SCB_RP	 = 0,
		EGEM2_SCX_SA = 1,
		DMX			 = 2
	};

	enum SystemType {

		SINGLE_CP_SYSTEM = 0,
		MULTI_CP_SYSTEM  = 1
	};

	enum EnvironmentType {  //environment enum
		UNKNOWN				= 0,
		SINGLECP_NOTCBA		= 1,
		SINGLECP_CBA		= 2,
		MULTIPLECP_NOTCBA	= 3,
		MULTIPLECP_CBA		= 4,
		SINGLECP_DMX		= 5,
		MULTIPLECP_DMX		= 6,
		VIRTUALIZED		= 7,
		SINGLECP_SMX		= 8,
		MULTIPLECP_SMX		= 9
	};

	enum fileMFolderType
	{
		swPackage		 = 0,
		swPackageScxb	 = 1,
		swPackageEpb1	 = 2,
		swPackageIptb	 = 3,
		swPackageEvoEt	 = 4,
		swPackageCmxb	 = 5,
		swPackageFw		 = 6
	};

	enum IpmiUpgradeStatusConstants {
		IPMI_UPGRADE_STATUS_UNDEFINED = -1,
		IPMI_UPGRADE_STATUS_READY = 0,
		IPMI_UPGRADE_STATUS_ONGOING = 1,
		IPMI_UPGRADE_STATUS_NO_SUCH_BOARD = 2,
		IPMI_UPGRADE_STATUS_OPEN_FILE_ERROR = 3,
		IPMI_UPGRADE_STATUS_FAIL = 4,
		IPMI_UPGRADE_STATUS_TFTP_ERROR = 5,
		IPMI_UPGRADE_STATUS_RESET_FAIL_IN_FB = 6,
		IPMI_UPGRADE_STATUS_RESET_FAIL_IN_UPG = 7,
		IPMI_UPGRADE_STATUS_TFTP_INTERNAL_ERROR = 8,
		IPMI_UPGRADE_STATUS_CHECKSUM_ERROR = 9,
		IPMI_UPGRADE_STATUS_NOT_STARTED = 12,
		IPMI_UPGRADE_STATUS_NOT_SUPPORTED = 13
	};

	static const char ClassNameApz[] = "AxeFunctions";
	static const char classShelf[] = "AxeEquipmentShelf";
	static const char DN_APZ_OBJ[] = "axeFunctionsId=1";
	static const char ATT_APZ_RDN_OBJ[] = "axeFunctionsId";
	static const char ATT_APZ_SYSTEM_TYPE[] = "systemType";
	static const char ATT_APZ_CBA_ARCH[] = "apgShelfArchitecture";

	//Shelf CLASS
	static const char ATT_SHELF_RDN[] = "shelfId";
	static const char ATT_SHELF_ADDRESS[] = "address";


	//OtherBlade CLASS
	static const char ATT_OTHERBLADE_RDN[] = "otherBladeId";
	static const char ATT_OTHERBLADE_CURRSW[] = "currentLoadModuleVersion";
	static const char ATT_OTHERBLADE_ROJ[] = "bladeProductNumber";
	static const char ATT_OTHERBLADE_SLOT[] = "slotNumber";
	static const char ATT_OTHERBLADE_FBN[] = "functionalBoardName";
	static const char ATT_OTHERBLADE_IPA[] = "ipAddressEthA";
	static const char ATT_OTHERBLADE_IPB[] = "ipAddressEthB";


	//FileM
	static const char DN_FILEM_OBJ[] = "northBoundFoldersId=1";
	static const char ATT_FILEM_SWPACKAGE_PATH[] = "swPackage";
	static const char ATT_FILEM_SWPACKAGE_PATH_EPB1[] = "swPackageEPB1";
	static const char ATT_FILEM_SWPACKAGE_PATH_SCXB[] = "swPackageSCXB";
	static const char ATT_FILEM_SWPACKAGE_PATH_IPTB[] = "swPackageIPTB";
	static const char ATT_FILEM_SWPACKAGE_PATH_CMXB[] = "swPackageCMXB";
	static const char ATT_FILEM_SWPACKAGE_PATH_EVOET[] = "swPackageEvoET";
	static const char ATT_FILEM_SWPACKAGE_PATH_FW[] = "swPackageFW";

	const std::string SMX_FWTYPE_A_CXP = "CXP9030857/2";
	const std::string SMX_FWTYPE_B_CXP = "CXP9030857/3";
	const std::string UBOOT_FILE ="bootloader/coreboot_smxb.rom";
	const std::string FAILSAFE_FILE ="linux/smxb-fail-safe-linux.img";
	const std::string BMC_FILE ="bmc/Phantom.hpm";
	const std::string SENSOR_FILE ="ipmi_params/sensor_table.bin";
	const std::string PMP_FILE ="ipmi_params/pmb.bin";

	bool CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists, long int size);
	bool PathFileExists(const char* file);
	bool CopyFileToDataDisk(std::string & source, std::string & destination);
	std::string replaceSubstr( const std::string& content, const std::string& instance, const std::string& replacement );
	EnvironmentType getEnvironment(void);
	std::string GetAPHostName();
	int findFile(char* path, char* type, std::vector<std::string> &result);
	int removeFolder(const char* _fullpath);
	bool CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists);
	bool CheckExtention(const char* file,const char* type);
		void findAndReplace( string &source, const string find, string replace);
	void trim(string& str);
	void findAndRemove( string &source, const string find);
	bool getImmAttributeString (std::string object, std::string attribute, std::string &value);
	bool getImmAttributeInt(std::string object, std::string attribute, int &value);

	bool getChildrenObject(std::string dn, std::vector<std::string> & list );
	bool getClassObjectsList(std::string className, std::vector<std::string> & list );
	bool getRdnAttribute (std::string object_dn, std::string &rdn);

	unsigned short numOfChar (std::istringstream &istr, char searchChar);
	void reverseDottedDecStr (std::istringstream &istr);
	bool dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4);
	bool checkFigures (std::istringstream &istr);
	bool stringToInt (std::string value, int &ivalue);
	std::string intToString (int ivalue);
	std::string getNbiSwPath(fileMFolderType folderType);


	bool stringToUlongMagazine (std::string value, unsigned long &uvalue);
	bool stringToUshort (std::string value, unsigned short &uvalue);
	bool stringToUpper (std::string& stringToConvert);
	std::string ulongToStringIP (unsigned long ipaddress);
	int getApubBoard (std::string magazine, unsigned short& slot);
} // End of namespace


#endif /* CMD_UTILS_H_ */
