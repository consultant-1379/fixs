
#ifndef FIXS_TREFC_Util_h
#define FIXS_TREFC_Util_h 1

#include <string>
#include <iostream> // debugging
#include <algorithm>
#include <sstream>
#include <string.h>
#include <map>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <tchar.h>
//#include <WinIoCtl.h>
//#include <process.h>
//#include <windows.h>
//#include <clusapi.h>
#include <sstream>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ACS_TRA_trace.h"
#include "FIXS_TREFC_IMM_Util.h"
#include "acs_aeh_evreport.h"
//#include "ACS_CS_API.h"
//#include "ACS_CS_API_Set.h"
//#include "ACS_CS_API_TREFC.h"
//#include "ACS_CS_API_DLL_Import.h"


#define CXC_NAME	"FIXS/CXC1371373"
#define SERVICE_NAME "FIXS_TREFC_SERV";
#define SERVICE_DOMAIN "FIXS";
//#define ETHIN_APT_PORT 31
using namespace std;

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

//***************************************************************************
//Parameters in parameter file
//***************************************************************************
#define SERVICE_PORT		"FIXS_TREFCBIN_Port"

//***************************************************************************
//Parameters related to Error handling
//***************************************************************************
#define ERROR						0
#define SUCCESS						1
#define READ_PARAM_ERROR			"Error reading parameter from Parameter Database"
#define INVALID_PARAM_ERROR			"Invalid parameter value - "

namespace TREFC_Util
{

	typedef uint16_t WORD;
	typedef uint64_t LONGLONG;
	typedef uint32_t DWORD;
	typedef uint16_t WCHAR;
	typedef uint16_t CHAR;

	const int MAX_DATA = 256;
	const int ITEM_SIZE = 128;
	const unsigned int MAXTREFNUMBER = 8;

	static const char EVENT_NAME_SHUTDOWN_SERVICE_HANDLER[]    = "FIXS_TREFC_SHUTDOWN_SERVICE_HANDLER_EVENT";
	static const char EVENT_NAME_SHUTDOWN[]    = "FIXS_TREFC_SHUTDOWN";
	static const int Daemon_Timeout			 = 3;	 // Timeout 3 s, used by daemon

//	static bool isAlarmRaised = false;

	enum respCode
	{
		EXECUTED					= 0,	//EXECUTED
		INTERNALERR					= 1,
		INCORRECTUSAGE				= 2,
		TIMESERVICE_ALREADY_STARTED = 3,
		TIMESERVICE_ALREADY_STOPPED = 4,
		TOO_MANY_REF				= 5,
		GPS_NOT_ALLOWED				= 6,
		NO_REPLY_FROM_CS			= 7,
		INCORRECT_SERVICE			= 8,
		CP_COMM_ERROR				= 9,
		ONLY_ALLOWED_ON_IOAPG		= 10,
		TIME_REF_NOT_EXIST			= 11,
		WRONG_IP_ADDRESS			= 12,
		SNMP_FAILURE				= 13,
		NOT_ALLOWED					= 14,
		INVALID_REF					= 15,
		NO_REFERENCE_EXIST			= 16,
		NOT_ALLOWED_BY_CP			= 17,
		TIME_REF_NOT_CONFIGURED		= 18,
		MODIFICATION_NOT_ALLOWED        = 19,
		INCORRECT_RESPONSE			= 32,

		//timerefscxconf specific error codes
		INVALID_NETMASK				= 33,
		INVALID_GATEWAY				= 34,
		EXT_IP_ALREADY_CONFIGURED	= 35,
		NO_EXT_IP_EXIST				= 36,
		INCORRECT_SLOT				= 38,
		FILE_ERROR					= 39,
		NO_SCX_IN_SLOT				= 40, //"No SCX board present in the specified slot"
		DISK_FULL					= 41,
		IP_EXIST					= 42,
		SERVICE_TYPE_MISMATCH       = 43,

		ILLEGAL_COMMAND				= 115,
		UNABLECONNECT				= 117,

		//It's needed to verify the EXIT CODE !!
		TIME_REF_IP_ALREADY_EXIST				= 120,
		GPS_ALREADY_EXIST						= 121,
		UNABLECONNECT_PASSIVE					= 122,
		TIME_REF_NAME_ALREADY_EXIST				= 123,
		SERVICE_TYPE_CHANGE_NOT_ALLOWED			= 124,
		REFERENCE_TYPE_CHANGE_NOT_ALLOWED		= 125,
		EXT_IP_REMOVE_NOT_ALLOWED				= 126
	};

	enum TRefErrors {
		TREF_ERR_OK								= 0,
		TREF_ERR_IPALREADYPRESENT				= 1

	};

	enum timeRefCmd
	{
		cmdAddTref		= 0,		//timerefadd
		cmdRmTref		= 1,		//timerefrm
		cmdLsTref		= 2,		//timerefls
		cmdStartTref	= 3,
		cmdStopTref		= 4,
		cmdScxAdd		= 5,
		cmdScxRm		= 6,
		cmdScxLs		= 7
	};
	typedef struct
	{
		timeRefCmd    		cmdType;
		char                data[MAX_DATA]; //command parameter
		char				name[MAX_DATA]; //name of reference
	} tref_request;

	typedef struct
	{
		timeRefCmd    		cmdType;
		char                data[MAX_DATA]; //command parameter
		char				name[MAX_DATA]; //name of reference
		bool				force;          // force option flag
	} tref_start_request;

	typedef struct
	{
		timeRefCmd    		cmdType;
		char                ipAddr[MAX_DATA];
		char				netmask[MAX_DATA];
		char				gateway[MAX_DATA];
		char				slot[MAX_DATA];
	} tref_Scx_Request;

	enum Severity
	{
		Severity_A1			=	0,
		Severity_A2			=	1,
		Severity_CEASING	=	2,
		Severity_O1			=	3,
		Severity_Event		=	4
	};

	typedef struct
	{
		int sPort;
	}ServiceParams;


	enum EnvironmentType {  //environment enum
		UNKNOWN				= 0,
		SINGLECP_NOTCBA		= 1,
		SINGLECP_CBA		= 2,
		MULTIPLECP_NOTCBA	= 3,
		MULTIPLECP_CBA		= 4,
		SINGLECP_DMX            = 5,
                MULTIPLECP_DMX          = 6,
                VIRTUALIZED             = 7,
                SMX          		= 8
	};


	struct SWITCHAPZIPAddress
	{
		std::string IP_EthA;
		std::string IP_EthB;
		unsigned short slotPos;
	};

	struct SWITCHAPTIPAddress
	{
		std::string IP_EthA;
		std::string IP_EthB;
		int ethInPort;
		int index;
	};
	struct ExtIpSwitch
	{
		std::string extIp;
		std::string defaultGateway;
		std::string netmask;
		unsigned short slot;
	};

	struct TimeServer {
		std::string ipAddress;
		std::string name;
	};

	struct ExtIpAction {
		ExtIpSwitch element;
		IMM_Util::Action action;
	};

	struct TimeServerAction {
		TimeServer element;
		IMM_Util::Action action;
	};



	// Name of service
	static const char FIXS_TREFC_PROCESS_NAME[] = "FIXS_TREFC_Service";

	unsigned short numOfChar (std::istringstream &istr, char searchChar);
	bool checkFigures (std::istringstream &istr);
	bool dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4);
	bool checkIPAddresses(const char* arg, unsigned long &ipAddress);
	bool checkReferenceName(string &s);
	bool checkIfActiveNode();
	void sendAlarm (const int problem, const unsigned int severity, const std::string &cause, const std::string &data, const std::string &text);
	std::string ulongToStringIP (unsigned long ipaddress);

	bool isSCX(int magazine, int slot, std::string ip);
	//void getHexTime(unsigned char* byte, unsigned int& size);
	void getHexTime(char* byte, unsigned int& size);
	void sendEvent (const int problem, const std::string &cause, const std::string &data, const std::string &text);
	bool stringToUpper (std::string& stringToConvert);
	bool isApzSCXMagazine(std::string snmpIP, unsigned short &slot);

	int  GetServiceParams(ServiceParams *ptrParams);
	void ErrorHandler(char *Err , int ErrCode = 0 , char *paramName = NULL );

	//EnvironmentType getEnvironment(void);
	std::string GetAPHostName();
	void getCurrentTime(std::string& dateStr, std::string& timeStr);
	std::string getCurrentTimeInSeconds();

	bool stringToUlongMagazine (std::string value, uint_fast32_t &uvalue);
	bool ulongToStringMagazine (uint32_t uvalue, std::string &value);
	bool stringToUshort (std::string value, unsigned short &uvalue);
	bool ushortToString (unsigned short uvalue, std::string &value);
	bool longlongToString (LONGLONG uvalue, std::string &value);
	bool longToString (long uvalue, std::string &value);
	bool stringToLong (std::string value, long &uvalue);
	bool doubleToString (double dvalue, std::string &value);
	std::string intToString (int ivalue);
	bool stringToInt (std::string value, int &ivalue);
	bool intToUshort (int value, unsigned short &ivalue);
	void reverseDottedDecStr (std::istringstream &istr);
	EnvironmentType getEnvironment();

	string getErrrorText(int error);
}
#endif
