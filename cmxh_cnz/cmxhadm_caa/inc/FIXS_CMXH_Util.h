
#ifndef FIXS_CMXH_Util_h
#define FIXS_CMXH_Util_h 1

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

namespace CMXH_Util
{

typedef uint16_t WORD;
typedef uint64_t LONGLONG;
//typedef uint32_t DWORD;
typedef uint16_t WCHAR;
typedef uint16_t CHAR;

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

	static const char EVENT_NAME_SHUTDOWN_SERVICE_HANDLER[]    = "FIXS_CMXH_SHUTDOWN_SERVICE_HANDLER_EVENT";
	static const char EVENT_NAME_SHUTDOWN[]    = "FIXS_CMXH_SHUTDOWN";
//	int Daemon_Timeout			 = 3;	 // Timeout 3 s, used by daemon

	enum EnvironmentType {  //environment enum
		UNKNOWN			= 0,
		SINGLECP_NOTCBA		= 1,
		SINGLECP_CBA		= 2,
		MULTIPLECP_NOTCBA	= 3,
		MULTIPLECP_CBA		= 4,
		SINGLECP_DMX		= 5,
		MULTIPLECP_DMX		= 6,
		SMX			= 7
	};

	struct vlanParameterStruct
	{
		std::string name;
		int id;
		int priority;
		std::string portMask;
		std::string untaggedPortMask;
	};


	// Name of service
	static const char FIXS_CMXH_PROCESS_NAME[] = "FIXS_CMXH_Service";

	std::string ulongToStringIP (uint32_t  ipaddress);
	bool checkFigures (std::istringstream &istr);
	bool stringToInt (std::string value, int &ivalue);
	int createDir(std::string dirName);
	int getNode();
	int	getDRBDVlan();
	void renameCopiedContFile (std::string  contFileName, std::string ipNumber);
	uint64_t getEpochTimeInSeconds();
	
} // End of namespace

//static bool isProgress=false;

#endif
