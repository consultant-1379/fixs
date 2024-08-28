/*
 * ACS_TRAPDS_Util.h
 *
 *  Created on: Jan 23, 2012
 *      Author: eanform
 */

#ifndef FIXS_AMIH_UTIL_H_
#define FIXS_AMIH_UTIL_H_

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
#include "ACS_TRA_trace.h"

# define ERROR -1

# define OPENDIR_ERROR   -1
# define LSTAT_ERROR     -2
# define REMOVE_ERROR    -3
# define CLOSEDIRR_ERROR -4

#define TMN_REPARSE_DATA_BUFFER_HEADER_SIZE \
			FIELD_OFFSET(TMN_REPARSE_DATA_BUFFER, SubstituteNameOffset)
#define MAX_NO_OF_INTERFACE 34
#define MAX_NO_OF_COUNTERS 38

//typedef uint16_t WORD;
typedef uint64_t LONGLONG;
typedef uint32_t DWORD;
//typedef uint16_t WCHAR;
//typedef uint8_t CHAR;

namespace AMIH_Util
{

using namespace std;

typedef uint16_t WORD;
typedef uint64_t LONGLONG;
typedef uint32_t DWORD;
typedef uint16_t WCHAR;
typedef uint16_t CHAR;

# define MAX_PATH_CCH 1000

	// To avoid warning about unused parameter
	#define UNUSED(expr) do { (void)(expr); } while (0)

	const std::string TEMP_FOLDER = "/tmp/temp_install";

	static const char EVENT_NAME_SHUTDOWN_SERVICE_HANDLER[]="FIXS_AMIH_SHUTDOWN_SERVICE_HANDLER_EVENT";
	static const char EVENT_NAME_SHUTDOWN[]="FIXS_AMIH_SHUTDOWN";
	static const int Daemon_Timeout=3;	 // Timeout 3 s, used by daemon

	static const char DN_APZ[] = "axeFunctionsId=1";
	static const char ATT_APZ_RDN[] = "axeFunctionsId";
	static const char ATT_APZ_CBA[] = "apgShelfArchitecture";


	// Name of service
	static const char ACS_TRAPDS_PROCESS_NAME[] = "ACS_TRAPDS_Service";

	enum Severity
	{
	Severity_A1			=	0,
	Severity_A2			=	1,
	Severity_CEASING	=	2,
	Severity_O1			=	3
	};

	enum EnvironmentType {  //environment enum
		UNKNOWN				= 0,
		SINGLECP_NOTCBA		= 1,
		SINGLECP_CBA		= 2,
		MULTIPLECP_NOTCBA	= 3,
		MULTIPLECP_CBA		= 4
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

	unsigned short numOfChar (std::istringstream &istr, char searchChar);
	void reverseDottedDecStr (std::istringstream &istr);
	bool dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4);
	bool checkFigures (std::istringstream &istr);
	bool stringToUpper (std::string& stringToConvert);

	bool intToUshort (int value, unsigned short &ivalue);

	bool createDir (const char* dirpath, DWORD& err);
	bool gprStringToHex (std::string value, int &hexvalue);
	void getCurrentTime(std::string& dateStr, std::string& timeStr);
	std::string getCurrentTimeInSeconds();

	bool CopyFileToDataDisk(std::string & source, std::string & destination);
	std::string replaceSubstr( const std::string& content, const std::string& instance, const std::string& replacement );
	std::string getEnvironmentType(EnvironmentType env);
	std::string GetAPHostName();
	EnvironmentType isRightIptEnvironment(void);

	// Create a symbolic link
	int createSymbolicLink(
		const std::string& link,			// Symbolic link
		const std::string& dir				// Destination directory
		);
	void findAndReplace( string &source, const string find, string replace);
	void trim(string& str);
	void findAndRemove( string &source, const string find);
	bool isFlashValue (std::string value);
	bool isNetworkValue (std::string value);
	bool stringTolonglong (std::string value, LONGLONG &ivalue);
	bool doubleToString (double dvalue, std::string &value);
	LONGLONG getEpochTimeInSeconds();
	int calculateDigit(double x);
	bool gprScxStringToHex (std::string value, int &hexvalue);
	//char **findFile(char * dir,char *type_file,int *lun);
	int findFile(char* path, char* type, std::vector<std::string> &result);
	int removeFolder(const char* _fullpath);
	bool CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists);
	bool CheckExtention(const char* file,const char* type);
	void stringIpToUlong(const char *ip, unsigned long &uvalue);
	void printTRA(ACS_TRA_trace* _trace,std::string mesg);

} // End of namespace


#endif /* FIXS_AMIH_UTIL_H_ */
