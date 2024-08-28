
#ifndef xcountls_Util_h
#define xcountls_Util_h 1

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


# define ERROR -1

# define OPENDIR_ERROR   -1
# define LSTAT_ERROR     -2
# define REMOVE_ERROR    -3
# define CLOSEDIRR_ERROR -4

#define TMN_REPARSE_DATA_BUFFER_HEADER_SIZE \
			FIELD_OFFSET(TMN_REPARSE_DATA_BUFFER, SubstituteNameOffset)
#define MAX_NO_OF_INTERFACE 286
#define MAX_NO_OF_COUNTERS 38
#define TRANSPORTX_START_IFACE_VALUE  100
#define SMX_INTERFACE_COUNT 45





//typedef uint16_t WORD;
//typedef uint64_t ULONGLONG;
//typedef uint32_t DWORD;
//typedef uint16_t WCHAR;
//typedef uint8_t CHAR;

namespace XCOUNTLS_Util
{

typedef uint16_t WORD;
typedef uint64_t ULONGLONG;
typedef uint32_t DWORD;
typedef uint16_t WCHAR;
typedef uint16_t CHAR;


	enum EnvironmentType {  //environment enum
		UNKNOWN				= 0,
		SINGLECP_NOTCBA		= 1,
		SINGLECP_CBA		= 2,
		MULTIPLECP_NOTCBA	= 3,
		MULTIPLECP_CBA		= 4
	};


	//define return code
	static const int EXECUTED         				= 0;
	static const int INTERNALERR                	= 1;
	static const int INCORRECTUSAGE    				= 2;
	static const int NO_REPLY_FROM_CS      			= 7;
	static const int INCORRECT_MAGAZINE 			= 30;
	static const int INCORRECT_SLOT      			= 31;
	static const int INCORRECT_RESPONSE   			= 32;
	static const int SNMP_FAILURE           		= 13;
	static const int HARDWARE_NOT_PRESENT    		= 14;
	static const int FUNC_BOARD_NOTALLOWED    		= 22;
	static const int DISK_FULL                 		= 24;
	static const int NO_INTERFACE_UP            	= 41;
	static const int NOT_EXIST_INTERFACE        	= 42;
	static const int BASELINE_FILE_NOT_FOUND		= 43;
	static const int FILE_DELETION_FAILED 			= 44;
	static const int ILLEGAL_COMMAND            	= 115;
	static const int ONLY_ALLOWED_ON_ACTIVE_NODE	= 116;
	static const int IRONSIDE_FAILURE            	= 119;
	static const int DMX_TENANT_NOT_FOUND       = 120;

	static const std::string BASELINE = "BASELINE";

	typedef struct
	{
		std::string timeValue;
		std::string interfaceName[MAX_NO_OF_INTERFACE];
		std::string counterValue[MAX_NO_OF_INTERFACE][MAX_NO_OF_COUNTERS];
	} PCData;

	//Performance Counters keys
	static const std::string KEY_TIME = "TIME";
	static const std::string KEY_INTERFACECOUNT = "INTERFACE-COUNT";
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


	bool ulongToStringMagazine (unsigned long uvalue, std::string &value);
	bool stringToUlongMagazine (std::string value, unsigned long &uvalue);
	bool ulongToString (unsigned long uvalue, std::string &value);
	//bool ushortToString (unsigned short uvalue, std::string &value);
	std::string getPcSectionName(unsigned long umagazine, unsigned short uslot);
	//bool stringToUpper (std::string& stringToConvert);

	int uint32_to_ip_fields (uint8_t (& ip_fields) [4], uint32_t value);
	int uint32_to_ip_format (char (& value_ip) [16], uint32_t value, char separator);

	int filecopy(const std::string &source, const std::string &dest);
	//void getCurrentTime(std::string& dateStr, std::string& timeStr);
	//std::string getCurrentTimeInSeconds();
	//bool longToString (long uvalue, std::string &value);

	bool ulongToString (unsigned long uvalue, std::string &value);

	bool stringToUlong (std::string value, uint_fast32_t &uvalue);
	//bool ulongToString (uint32_t uvalue, std::string &value);
	bool stringToUshort (std::string value, unsigned short &uvalue);
	bool ushortToString (unsigned short uvalue, std::string &value);
	bool ulonglongToString (ULONGLONG uvalue, std::string &value);
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
	bool stringToUpper (std::string& stringToConvert);

	bool intToUshort (int value, unsigned short &ivalue);

	bool createDir (const char* dirpath, DWORD& err);

	void getCurrentTime(std::string& dateStr, std::string& timeStr);
	std::string getCurrentTimeInSeconds();

	bool CopyFileToDataDisk(std::string & source, std::string & destination);
	std::string replaceSubstr( const std::string& content, const std::string& instance, const std::string& replacement );
	std::string getEnvironmentType(EnvironmentType env);
	std::string GetAPHostName();

	// Convert multibyte character string to wide character string
	std::wstring mbsToWcs(			// Returns converted string
			const std::string& str	// String to convert
			);


	bool isFlashValue (std::string value);
	bool isNetworkValue (std::string value);
	bool stringToulonglong (std::string value, ULONGLONG &ivalue);
	bool doubleToString (double dvalue, std::string &value);
	bool doubleToStringRate(double dvalue, std::string &value);
	ULONGLONG getEpochTimeInSeconds();
	int calculateDigit(double x);

	int findFile(char* path, char* type, std::vector<std::string> &result);
	int removeFolder(const char* _fullpath);
	bool CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists);
	bool CheckExtention(const char* file,const char* type);
	void stringIpToUlong(const char *ip, unsigned long &uvalue);
	void findAndReplace( std::string &source, const std::string find, std::string replace);
	void trim(std::string& str);
	void findAndRemove( std::string &source, const std::string find);
	bool getImmAttributeInt(const std::string &strDn, const std::string &strAttr, int *iVal);
} // End of namespace


#endif
