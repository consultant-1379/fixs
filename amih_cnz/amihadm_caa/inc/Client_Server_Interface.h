#ifndef _CLIENT_SERVER_INTERFACE_H_
#define _CLIENT_SERVER_INTERFACE_H_
//#include <winsock2.h>
//#include <sys/types.h> //To be define
//#include <sys/stat.h>  //To be define


#define SERVICE_NAME "FIXS_FIXSAP_SERVER";
#define SERVICE_DOMAIN "FIXS";
const int MAX_DATA = 256;
const int MAX_RESPONSE_SIZE = 4096;
const int ITEM_SIZE = 128;


enum respCode
{
	//upgradepackage
	EXECUTED			= 0,	//EXECUTED
	INTERNALERR         = 1,	// Generic internal error
	INCORRECTUSAGE		= 2,	// Usage
	DIRECTORYERR		= 3,	// Internal Error during filesystem managing
	PACKAGE_IN_USE		= 4,	// The package is used
	PACKAGE_NOT_EXIST	= 5,	// The package does not exist
	XMLERROR     		= 6,	// Directory is full
	NO_REPLY_FROM_CS	= 7,    // cannot fetch information from CS
	//WRONGSCXADDR		= 8,	// Magnum or slot are not set properly 
	//SCXBNOTREACH		= 9,	// No reply from SCX
	//NOTHINGTOREMOVE		= 10,	// no unused package to be removed
	

	PACKAGE_ALREADY_INSTALLED = 11,	// The package is already installed
	//COMMUNICATION_ERROR = 12,	//The server and the client cannot communicate 
	//CHECK_ARCHITECTURE_FAILED = 13,	//Cannot fetch information about Architecture

	//generic codes
	//APNOTDEFINED	= 113,	// AP is not defined
	ILLEGAL_COMMAND	= 115, // Node Parameter is 0
	//ILLEGALOPT		= 116,	// Illegal option in this system configuration
	UNABLECONNECT	= 117	// Unable to connect to server
};

enum AMIHCommandType
{
	cmdInstall = 0, //upgradepackage -n tarfile
	cmdRemove = 1,  //upgradepackage -d package|all
	cmdList = 2     //upgradepackage -i 
};

typedef struct
{
	AMIHCommandType		cmdType;
	//char                extendType; t||v parameter
	char                data[MAX_DATA];
	//char				magazine[MAX_DATA]; magazine parameter
	//char                slot[MAX_DATA]; slot parameter

} AMIH_upgrade_request;


typedef struct
{
	respCode response;
	int numItem;
	char data[MAX_RESPONSE_SIZE];
} AMIH_list_response;

#endif
