// xcountls.cpp : Defines the entry point for the console application.
//
//#include <tchar.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <stdint.h>

#include <ACS_CS_API.h>
#include "fixs_cch_cmdshelfmanager.h"
#include "ACS_APGCC_Util.H"
#include "ACS_TRA_trace.h"
#include "xcountls_Util.h"
#include "FIXS_CCH_PCDataStorage.h"
#include "Get_opt.h"
#include "cmd_Utils.h"

using namespace XCOUNTLS_Util;

#define CMD_OPT_VERBOSE    0x0001  // for option -v 
#define CMD_OPT_MAG        0x0002  // for option -m 
#define CMD_OPT_SLOT       0x0004  // for option -s
#define CMD_OPT_INTERFACE  0x0008  // for option -i
#define CMD_OPT_STATUS     0x0010  // for option -S
#define CMD_OPT_BASELINE   0x0020  // for option -b


// check option is set or not
#define CHECK_OPT_VERBOSE(opt)     ((CMD_OPT_VERBOSE & opt) == CMD_OPT_VERBOSE)
#define CHECK_OPT_MAG(opt)		   ((CMD_OPT_MAG & opt) == CMD_OPT_MAG)
#define CHECK_OPT_SLOT(opt)        ((CMD_OPT_SLOT & opt) == CMD_OPT_SLOT)
#define CHECK_OPT_INTERFACE(opt)   ((CMD_OPT_INTERFACE & opt) == CMD_OPT_INTERFACE)
#define CHECK_OPT_STATUS(opt)	   ((CMD_OPT_STATUS & opt) == CMD_OPT_STATUS)
#define CHECK_OPT_BASELINE(opt)	   ((CMD_OPT_STATUS & opt) == CMD_OPT_BASELINE)


//set the specific option
#define SET_OPT_VERBOSE(mask)      (mask = mask | CMD_OPT_VERBOSE)
#define SET_OPT_MAG(mask)          (mask = mask | CMD_OPT_MAG)
#define SET_OPT_SLOT(mask)         (mask = mask | CMD_OPT_SLOT)
#define SET_OPT_INTERFACE(mask)    (mask = mask | CMD_OPT_INTERFACE)
#define SET_OPT_STATUS(mask)       (mask = mask | CMD_OPT_STATUS)
#define SET_OPT_BASELINE(mask)     (mask = mask | CMD_OPT_BASELINE)

//define number of interfaces in DMX
#define DMX_INTERFACES				35

////define return code
//#define EXECUTED					0
//#define INCORRECTUSAGE				2
//#define INCORRECT_MAGAZINE			30
//#define INCORRECT_SLOT				31
//#define INCORRECT_RESPONSE			32
//#define NO_REPLY_FROM_CS			7
//#define SNMP_FAILURE				13
//#define HARDWARE_NOT_PRESENT		14
//#define FUNC_BOARD_NOTALLOWED		22
//#define DISK_FULL					24
//#define NO_INTERFACE_UP				41
//#define NOT_EXIST_INTERFACE			42
//#define BASELINE_FILE_NOT_FOUND		43
//#define FILE_DELETION_FAILED		44
//#define ILLEGAL_COMMAND				115
//#define ONLY_ALLOWED_ON_ACTIVE_NODE	116
//#define INTERNALERR					1



namespace {
const int INVALID_INTERFACE = 0;
int interfaceNr = INVALID_INTERFACE; // interface number

ACS_CS_API_HWC * hwc = 0;
ACS_CS_API_BoardSearch * boardSearch = 0;
ACS_TRA_trace _trace = ACS_TRA_trace("XCOUNTLS");

class TraceInOut
{
public:
	TraceInOut(const char * function) : m_function(function)
	{
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] In", m_function.c_str());
			_trace.ACS_TRA_event(1, tmpStr);
		}
	};

	~TraceInOut()
	{
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Out", m_function.c_str());
			_trace.ACS_TRA_event(1, tmpStr);
		}
	}
private:
	std::string m_function;
};
}


typedef struct 
{
	unsigned long mag;
	unsigned short slot;
	std::string IP_EthA;
	std::string IP_EthB;
	std::string AliasIP_EthA;
	std::string AliasIP_EthB;

} imbInfo;

typedef struct
{
	std::string increment[MAX_NO_OF_INTERFACE][MAX_NO_OF_COUNTERS];
	std::string rate[MAX_NO_OF_INTERFACE][MAX_NO_OF_COUNTERS];
	std::string deprecated[MAX_NO_OF_INTERFACE];

} PCIncrRateData;

// Command Options
#define CMD_OPTIONS  "i:m:s:vSbfdp"	// options i, m, s take arguments,
// but v and S options do not.

static CMD_Util::EnvironmentType m_environment = CMD_Util::UNKNOWN;

static const char* DmxInterfaceNames[DMX_INTERFACES] = {"BP1", "BP2", "BP3", "BP4", "BP5", "BP6", "BP7", "BP8", "BP9", "BP10", "BP11", "BP12", "BP13", "BP14", "BP15", "BP16", "BP17", "BP18", "BP19", "BP20", "BP21", "BP22", "BP23", "BP24", "LOCALHOST", "REMOTEHOST", "GE1", "GE2", "BP26", "BP28", "E1", "E2", "E3", "E4", "GE3"};

int printUsage();
//int printDefaultResult(fixs_cch_cmdshelfmanager *snmpObj,PCIncrRateData &data,int no_of_interface,unsigned int option);
int printDefaultResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, int no_of_interface,unsigned int option, bool isTransportSwitch);
int printVerboseResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, int no_of_interface,unsigned int option, bool isTransportSwitch);

bool readSnmpVerboseInformation (fixs_cch_cmdshelfmanager *snmpObj,std::string & ipA_String,std::string & ipB_String,int &no_of_interface,int l3SwitchVal);
bool readSnmpDefaultInformation (fixs_cch_cmdshelfmanager *snmpObj,std::string & ipA_String,std::string & ipB_String,int &no_of_interface,int l3SwitchVal);
bool readIronsideVerboseInformation (fixs_cch_cmdshelfmanager *snmpObj,std::string & magStr,std::string & slotStr,int &no_of_interface);
bool readIronsideDefaultInformation (fixs_cch_cmdshelfmanager *ironsideObj,std::string & magStr,std::string & slotStr,int &no_of_interface);

int scxBoardInfo(std::vector<imbInfo> & scxVector,std::string & argMag,std::string & argSlot, unsigned int option, bool isSMX);
int cmxBoardInfo(std::vector<imbInfo> & cmxVector,std::string & argMag,std::string & argSlot,unsigned int option);
int allEGEM2L2SwitchBoardInfo(std::vector<imbInfo> & scxVector,bool isSMX);
int allCMXBoardInfo(std::vector<imbInfo> & cmxVector);
int fetchBaselineData(fixs_cch_cmdshelfmanager *snmpObj,int no_of_interface, XCOUNTLS_Util::PCData &data);
void sortVector(std::vector<imbInfo> & imbVector);
int isOperationAllowed (unsigned long umagazine,unsigned short uslot);
//int isOperationOnSlotAllowed (unsigned long uslot);
int isOperationOnMagazineAllowed (unsigned long umagazine);
//@
//int fetchSwitchInfo (unsigned long umagazine,unsigned short uslot,unsigned long &uIP_EthA,unsigned long &uIP_EthB);
int fetchSwitchInfo (unsigned long umagazine,unsigned short uslot,uint32_t &uIP_EthA,uint32_t &uIP_EthB,uint32_t &uAliasIP_EthA,uint32_t &uAliasIP_EthB,bool isSMX);
//@

int pcVerboseData(unsigned long umagazine,unsigned short uslot,unsigned int option,int fileExists);
int pcDefaultData(unsigned long umagazine,unsigned short uslot,unsigned int option,int fileExists);

//@void printDefaultInterfaceResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, std::stringstream &pcTable, int ifIndex, int ifNumber);
void printDefaultInterfaceResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, std::stringstream &pcTable, int ifIndex, int ifNumber, bool isTransportSwitch);

//void printVerboseInterfaceResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, std::stringstream &pcTable, int ifIndex, int ifNumber);
void printVerboseInterfaceResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, std::stringstream &pcTable, int ifIndex, int ifNumber, bool isTransportSwitch);
int fillPrintOutDataForBoard(std::stringstream &pcTable, XCOUNTLS_Util::PCData data, int no_of_intrface,bool isTransport);
int pcBaseline();
int printBaseline();
void initializePCIncrRateData(PCIncrRateData &pcCal);
int calculateVerboseIncrRateValues(fixs_cch_cmdshelfmanager *snmpObj,XCOUNTLS_Util::PCData &data,PCIncrRateData &pcCal,ULONGLONG time, int no_of_interface,int base_interface, bool isTransportSwitch);
//@int calculateDefaultIncrRateValues(fixs_cch_cmdshelfmanager *snmpObj,XCOUNTLS_Util::PCData &data,PCIncrRateData &pcCal,ULONGLONG time, int no_of_interface,int base_interface);
int calculateDefaultIncrRateValues(fixs_cch_cmdshelfmanager *snmpObj,XCOUNTLS_Util::PCData &data,PCIncrRateData &pcCal,ULONGLONG time, int no_of_interface,int base_interface, bool isTransportSwitch);
bool IsCba(bool &res);
bool isSMX=false;
//static int SNMPFailCount = 0, statusFailCount = 0, countNotExistInterface =0;//@ failureCount=0;//@
//int displayInterfaces(unsigned long magazine, unsigned short slot, int fileExists,fixs_cch_cmdshelfmanager *shelfManager,int no_of_interface,ULONGLONG currTimeSec,unsigned int option, bool isTransportSwitch, bool snmpflagDisp);//@
//@int displayVerboseInfo(unsigned long umagazine, unsigned short uslot, int fileExists,fixs_cch_cmdshelfmanager *shelfManager,int no_of_intrface,ULONGLONG currTimeSec,unsigned int option, bool isTransportSwitch);//@
//int displayVerboseInfo(unsigned long umagazine, unsigned short uslot, int fileExists,fixs_cch_cmdshelfmanager *shelfManager,int no_of_intrface,ULONGLONG currTimeSec,unsigned int option, bool isTransportSwitch, bool snmpflagVerbose);//@
//int saveBoardCounters(std::string &mag, std::string &slotStr, fixs_cch_cmdshelfmanager *shelfManager, int no_of_intrface);
//--------------------------------------------------
// printUsage
//	print correct usage and return proper exit code
//--------------------------------------------------
int printUsage()
{
        TraceInOut inOut(__FUNCTION__);
	cerr<< "\nIncorrect usage" << std::endl;
     if (m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX)
     {
	cerr << "Usage: xcountls [-m magazine] [-s slot] [-S]" << std::endl;     // default printout
        cerr << "Usage: xcountls -v -m magazine -s slot [-S]" << std::endl;      // verbose printout
	cerr << "Usage: xcountls -b [-f]" << std::endl;
        cerr << "Usage: xcountls -d [-f]" << std::endl;
        cerr << "Usage: xcountls -p" << std::endl;


        cerr << "COMMAND OPTIONS:" << std::endl;
        cerr << "-b                  Baseline option" << std::endl;
        cerr << "                       This option is used to create a baseline file containing" << std::endl;
        cerr << "                       values of all the counters for each switch board and each" << std::endl;
        cerr << "                       traffic interface,with timestamp of data collection time." << std::endl;
        cerr << "-d                  Baseline delete option" << std::endl;
        cerr << "                       This option is used to delete a baseline file containing" << std::endl;
        cerr << "                       values of all the counters for all switch boards." << std::endl;
        cerr << "-f                  Force option" << std::endl;
        cerr << "                       This option is used to suppress dialogue mode." << std::endl;
        cerr << "-m magazine         Magazine address" << std::endl;
        cerr << "                       This option is used to print the performance counters" << std::endl;
        cerr << "                       related to the interfaces of the switch boards configured" << std::endl;
        cerr << "                       in the specified magazine." << std::endl;
        cerr << "-p                  Printout option" << std::endl;
        cerr << "                       This option is used to print the baseline information" << std::endl;
        cerr << "                       of all the performance counters from the baseline file." << std::endl;
        cerr << "-s slot             Slot position" << std::endl;
        cerr << "                       This option is used to print the performance counters" << std::endl;
        cerr << "                       related to the interfaces of the switch board specified" << std::endl;
        cerr << "                       by given slot number." << std::endl;
        cerr << "-S                  Interface status option" << std::endl;
        cerr << "                       This option is used to print the performance counters" << std::endl;
        cerr << "                       related to the interfaces of the switch board for which" << std::endl;
        cerr << "                       interface operative status is UP." << std::endl;
	cerr << "-v                  Verbose option" << std::endl;
        cerr << "                       This option is used to print all set of performance" << std::endl;
        cerr << "                       counters for a specified switch board." << std::endl;

    }
    else
    { 
	cerr << "Usage: xcountls [-m magazine] [-s slot] [-i interface] [-S]" << std::endl;	// default printout
	cerr << "Usage: xcountls -v -m magazine -s slot [-i interface] [-S]" << std::endl;	// verbose printout
	cerr << "Usage: xcountls -b [-f]" << std::endl;
	cerr << "Usage: xcountls -d [-f]" << std::endl;
	cerr << "Usage: xcountls -p" << std::endl;

	
	cerr << "COMMAND OPTIONS:" << std::endl;
	cerr << "-b                  Baseline option" << std::endl;
	cerr << "                       This option is used to create a baseline file containing" << std::endl;
	cerr << "                       values of all the counters for each switch board and each" << std::endl;
	cerr << "                       traffic interface,with timestamp of data collection time." << std::endl;
	cerr << "-d                  Baseline delete option" << std::endl;
	cerr << "                       This option is used to delete a baseline file containing" << std::endl;
	cerr << "                       values of all the counters for all switch boards." << std::endl;
	cerr << "-f                  Force option" << std::endl;
	cerr << "                       This option is used to suppress dialogue mode." << std::endl;
	cerr << "-i interface        Interface option" << std::endl;
	cerr << "                       This option is used to print the performance counters" << std::endl;
	cerr << "                       related to specific interface of the switch board." << std::endl; 
	cerr << "-m magazine         Magazine address" << std::endl;
	cerr << "                       This option is used to print the performance counters" << std::endl;
	cerr << "                       related to the interfaces of the switch boards configured" << std::endl;
	cerr << "                       in the specified magazine." << std::endl;
	cerr << "-p                  Printout option" << std::endl;
	cerr << "                       This option is used to print the baseline information" << std::endl;
	cerr << "                       of all the performance counters from the baseline file." << std::endl;
	cerr << "-s slot             Slot position" << std::endl;
	cerr << "                       This option is used to print the performance counters" << std::endl;
	cerr << "                       related to the interfaces of the switch board specified" << std::endl; 
	cerr << "                       by given slot number." << std::endl;
	cerr << "-S                  Interface status option" << std::endl;
	cerr << "                       This option is used to print the performance counters" << std::endl;
	cerr << "                       related to the interfaces of the switch board for which" << std::endl; 
	cerr << "                       interface operative status is UP." << std::endl;
	cerr << "-v                  Verbose option" << std::endl;
	cerr << "                       This option is used to print all set of performance" << std::endl;
	cerr << "                       counters for a specified switch board." << std::endl;
    }
	return INCORRECTUSAGE;
}


bool IsCba(bool &res)
{
	int iVal;
	iVal = 0;
	bool resCommand=false;

	resCommand=XCOUNTLS_Util::getImmAttributeInt(CMD_Util::DN_APZ_OBJ,CMD_Util::ATT_APZ_CBA_ARCH,&iVal);

	if(resCommand)
	{
		if (iVal == 1)
			res=true;
		else
			res=false;
	}
	return resCommand;
}


//----------------------------------------
// printDefaultResult
//	default printout is printed here
//----------------------------------------
int printDefaultResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, int no_of_interface,unsigned int option, bool isTransportSwitch)
{
	int i,j,Common_Index=0;//,cntr=0;
	int retcode = 0;
	int countDownInterface = 0;
	bool interfaceFound = false;
	stringstream pcTable;
	pcTable << std::endl;
	int isDeprecated = false;

	int order[no_of_interface];


	for (j=0; j <= (no_of_interface - 1); j++)
	{
			int k=0;
			if(isTransportSwitch)
				k= (j+TRANSPORTX_START_IFACE_VALUE);
			else
				k=j;
			order[j] = k;

	}
	if (m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX)
	{	
		if(no_of_interface == DMX_INTERFACES) {
			for(i=0; i <= DMX_INTERFACES -1; i++)
			{
				interfaceFound = false;
				for(j=0; j<= DMX_INTERFACES - 1; j++) {
					if (strcmp(snmpObj->m_ifAlias[i].c_str(),  DmxInterfaceNames[j]) == 0) {
						order[j] = i;
						interfaceFound = true;
						break;
					}
				}

				if (interfaceFound == false)
				{
					return INTERNALERR;
				}

			}
		}
	}

	if (interfaceNr == INVALID_INTERFACE)
	{// interface number (-i) hasn't been specified

		for(int j=0; j < no_of_interface; j++)
		{
			if(CHECK_OPT_STATUS(option))
			{
				if(isTransportSwitch){
					if((snmpObj->m_ifOperStatus[j]).compare("UP") !=0)
					{
						countDownInterface++;
						continue;
					}
				}
				else{
					if((snmpObj->m_ifOperStatus[order[j]]).compare("UP") !=0)
					{
						countDownInterface++;
						continue;
					}
				}
			}

			if(isTransportSwitch){
				if(data.deprecated[j] == "deprecated")
				{
					isDeprecated = true;
				}
			}else if(!isTransportSwitch) {
				if(data.deprecated[order[j]] == "deprecated")
				{
					isDeprecated = true;
				}
			}

			if(isDeprecated == false){
				printDefaultInterfaceResult(snmpObj, data, pcTable, order[j],j,isTransportSwitch);
			}


		}
		if(countDownInterface && (countDownInterface == no_of_interface))
		{
			std::cout << "\nNo traffic interface(s) up" << std::endl;
			retcode = NO_INTERFACE_UP;
		}
		else if(isDeprecated)
		{
			std::cout << "\nDeprecated for rates/increment of the board that has been reset"<< std::endl;
		}
		else
			std::cout << pcTable.str() << std::endl;

	}
	else if ((interfaceNr <= no_of_interface) && (!isTransportSwitch))
	{
		//-i specified.....to restrict the display of interfaces from transport switches
		int ifIndex = interfaceNr - 1;
		if((snmpObj->m_ifOperStatus[order[ifIndex]]).compare("UP") == 0)
		{
			if(data.deprecated[order[ifIndex]] == "deprecated")
			{
				std::cout << "\nDeprecated for rates/increment of the board that has been reset"<< std::endl;
			}
			else
			{
				printDefaultInterfaceResult(snmpObj, data, pcTable, order[ifIndex], ifIndex,isTransportSwitch);
				std::cout << pcTable.str() << std::endl;
			}
		}
		else
		{
			std::cout << "\nNo traffic interface(s) up" << std::endl;
			retcode = NO_INTERFACE_UP;
		}
	}else if((interfaceNr <= (no_of_interface+TRANSPORTX_START_IFACE_VALUE) && (interfaceNr>TRANSPORTX_START_IFACE_VALUE))&&(isTransportSwitch)){
		//-i specified and for Transport Switch interfaces(greater than 100)
			int ifIndex = (interfaceNr - 1 - TRANSPORTX_START_IFACE_VALUE);

			if(isTransportSwitch)
				Common_Index=ifIndex;
			else
				Common_Index=order[ifIndex];

			if((snmpObj->m_ifOperStatus[Common_Index]).compare("UP") == 0)
			{
				if(data.deprecated[Common_Index] == "deprecated")
				{
					std::cout << "\nDeprecated for rates/increment of the board that has been reset"<< std::endl;
				}
				else
				{
					printDefaultInterfaceResult(snmpObj, data, pcTable, order[ifIndex], ifIndex,isTransportSwitch);
					std::cout << pcTable.str() << std::endl;
				}
			}
			else
			{
				std::cout << "\nNo traffic interface(s) up" << std::endl;
				retcode = NO_INTERFACE_UP;
			}
	}
	else
	{

		if(((interfaceNr < TRANSPORTX_START_IFACE_VALUE) && (!isTransportSwitch)) ||((interfaceNr >= TRANSPORTX_START_IFACE_VALUE) && (isTransportSwitch))||((interfaceNr >= TRANSPORTX_START_IFACE_VALUE) && (!isSMX))){
		std::cout << "\nInterface number does not exist" << std::endl;
						retcode = NOT_EXIST_INTERFACE;
		}

	}

	return retcode;
}

//----------------------------------------
// printVerboseResult
//	verbose printout is printed here
//----------------------------------------
int printVerboseResult(fixs_cch_cmdshelfmanager *snmpObj,PCIncrRateData &data,int no_of_interface,unsigned int option, bool isTransportSwitch)
{
	int i,j,Common_Index=0;
	int retcode = 0;
	int countDownInterface = 0;
	bool interfaceFound = false;
	stringstream pcTable;
	pcTable << std::endl;
	int isDeprecated = false;

	int order[no_of_interface];
	for (j=0; j <= (no_of_interface - 1); j++)
	{
		int k=0;
		if(isTransportSwitch)
			k= (j+TRANSPORTX_START_IFACE_VALUE);
		else
			k=j;
		order[j] = k;
	}

	if (m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX)
	{
		if(no_of_interface == DMX_INTERFACES) {
			for(i=0; i <= DMX_INTERFACES - 1; i++)
			{

				for(j=0; j<= DMX_INTERFACES - 1; j++) {
					if (strcmp(snmpObj->m_ifAlias[i].c_str(),  DmxInterfaceNames[j]) == 0) {
						order[j] = i;
						interfaceFound = true;
						break;
					}
				}
				if (interfaceFound == false)
					return INTERNALERR;
				else
					interfaceFound = false;

			}

		}
	}

	if (interfaceNr == INVALID_INTERFACE)
	{
		// interface number (-i) hasn't been specified
		for(int j=0;j<no_of_interface;j++)
		{
			if(CHECK_OPT_STATUS(option))
			{
				if(isTransportSwitch){
					if((snmpObj->m_ifOperStatus[j]).compare("UP") !=0)
					{
						countDownInterface++;
						continue;
					}
				}
				else{
					if((snmpObj->m_ifOperStatus[order[j]]).compare("UP") !=0)

					{
						countDownInterface++;
						continue;
					}
				}

			}

			if(isTransportSwitch){
				if(data.deprecated[j] == "deprecated")
				{
					isDeprecated = true;
				}
			}else if(!isTransportSwitch) {
				if(data.deprecated[order[j]] == "deprecated")
				{
					isDeprecated = true;
				}
			}

			if(isDeprecated == false){
				printVerboseInterfaceResult(snmpObj,data, pcTable, order[j], j, isTransportSwitch);
			}
		}
		if(countDownInterface && (countDownInterface == no_of_interface))
		{
			std::cout << "\nNo traffic interface(s) up" << std::endl;
			retcode = NO_INTERFACE_UP;
		}
		else if(isDeprecated)
		{
			std::cout << "\nDeprecated for rates/increment of the board that has been reset"<< std::endl;
		}
		else
			std::cout << pcTable.str() << std::endl;

	}
	else if ((interfaceNr <= no_of_interface) && (!isTransportSwitch))
	{		// -i specified
			int ifIndex = interfaceNr - 1;
			if((snmpObj->m_ifOperStatus[order[ifIndex]]).compare("UP") ==0)
			{
				if(data.deprecated[order[ifIndex]] == "deprecated")
				{
					std::cout << "\nDeprecated for rates/increment of the board that has been reset"<< std::endl;
				}
				else
				{
					printVerboseInterfaceResult(snmpObj,data, pcTable, order[ifIndex], ifIndex, isTransportSwitch);
					std::cout << pcTable.str() << std::endl;
				}
			}
			else
			{
				std::cout << "\nNo traffic interface(s) up" << std::endl;
				retcode = NO_INTERFACE_UP;
			}
		//}
	}else if((interfaceNr <= (no_of_interface+TRANSPORTX_START_IFACE_VALUE) && (interfaceNr>TRANSPORTX_START_IFACE_VALUE))&&(isTransportSwitch)){
		//-i specified and for Transport Switch interfaces(greater than 100)

			int ifIndex = (interfaceNr - 1 - TRANSPORTX_START_IFACE_VALUE);

			if(isTransportSwitch)
				Common_Index=ifIndex;
			else
				Common_Index=order[ifIndex];

			if((snmpObj->m_ifOperStatus[Common_Index]).compare("UP") == 0)
			{
				if(data.deprecated[Common_Index] == "deprecated")
				{
					std::cout << "\nDeprecated for rates/increment of the board that has been reset"<< std::endl;
				}
				else
				{
					printVerboseInterfaceResult(snmpObj,data, pcTable, order[ifIndex], ifIndex, isTransportSwitch);
					std::cout << pcTable.str() << std::endl;
				}
			}
			else
			{
				std::cout << "\nNo traffic interface(s) up" << std::endl;
				retcode = NO_INTERFACE_UP;
			}
	}
	else
	{
		if(((interfaceNr < TRANSPORTX_START_IFACE_VALUE) && (!isTransportSwitch)) ||((interfaceNr >= TRANSPORTX_START_IFACE_VALUE) && (isTransportSwitch))||((interfaceNr >= TRANSPORTX_START_IFACE_VALUE) && (!isSMX))){
			std::cout << "\nInterface number does not exist" << std::endl;
			retcode = NOT_EXIST_INTERFACE;
		}
	}
	return retcode;

}

bool readIronsideVerboseInformation (fixs_cch_cmdshelfmanager *ironsideObj,std::string & magStr,std::string & slotStr,int &no_of_interface)
{
	bool verbose = true;
	int slot = atoi(slotStr.c_str());
	//TODO: Some info are missing in DMX environment so Verbose mode doesn't print more info than Default one.
	if (!ironsideObj->getIfDataUDP(magStr, slot, no_of_interface, verbose))
		return false;

	ironsideObj->populateIfHCInPkts(no_of_interface);
	ironsideObj->populateIfHCOutPkts (no_of_interface);

	return true;
}


bool readIronsideDefaultInformation (fixs_cch_cmdshelfmanager *ironsideObj,std::string & magStr,std::string & slotStr,int &no_of_interface)
{
	int slot = atoi(slotStr.c_str());
	if (!ironsideObj->getIfDataUDP(magStr, slot, no_of_interface))
		return false;
	ironsideObj->populateIfHCInPkts(no_of_interface);
	ironsideObj->populateIfHCOutPkts (no_of_interface);

	return true;
}
//-------------------------------------------------------
// readSnmpDefaultInformation 
//	read default data through specified SCX addresses
//-------------------------------------------------------
bool readSnmpDefaultInformation (fixs_cch_cmdshelfmanager *snmpObj,std::string & ipA_String,std::string & ipB_String,int &no_of_interface, int l3SwitchVal)
{
	snmpObj->setIpAddress(ipA_String);
	if(!snmpObj->getIfNumber(no_of_interface))
	{
		snmpObj->setIpAddress(ipB_String);
		if(!snmpObj->getIfNumber(no_of_interface))
		{
			// trace: snmp query failed
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getIfNumber query failed", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}

			return false;
		}
	}

	// for CMX there are 36 ethernet interface and one VLAN interface.Counters to be fetched only for ethernet interfaces.
	if(l3SwitchVal)
	{
		if(!snmpObj->getBulkIfType(no_of_interface))
		{
			std::string tmpIp = snmpObj->getIpAddress();
			if(tmpIp.compare(ipA_String) == 0)
				snmpObj->setIpAddress(ipB_String);
			else
				snmpObj->setIpAddress(ipA_String);
			if(!snmpObj->getBulkIfType(no_of_interface))
			{
				printf("\n ..Error in GetBULK of getBulkIfType SNMP Connection\n");
				// trace: snmp query failed
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfType query failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return false;
			}

		}

		// find number of ethernet interfaces
		int count =0;
		for(int i=0; i < no_of_interface; i++)
		{

			if(snmpObj->m_ifType[i] == 6) //6 means ethernet interface
			{
				count++;
			}
			else if ((l3SwitchVal == 2) && (snmpObj->m_ifType[i] == 131))  //aggregator interfaces for SMX transport part
			{
				count++;

			}
		}
		no_of_interface = count; 

	}

	if((m_environment == CMD_Util::SINGLECP_SMX) || (m_environment == CMD_Util::MULTIPLECP_SMX))
	{  // In case of SMX populate interface name with If_MIB::ifName 
		if(!snmpObj->getBulkIfName(no_of_interface))
		{
			// debugging : printf("\n ..Error in GetBULK of getBulkIfName SNMP Connection\n");
			// trace: snmp query failed
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfName query failed", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);

			}
			//if snmp error occurs, we continue with next counter query
		}
	}
	else
	{
		if(!snmpObj->getBulkIfAlias(no_of_interface))
		{
			// debugging : printf("\n ..Error in GetBULK of getBulkIfAlias SNMP Connection\n");
			// trace: snmp query failed
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfAlias query failed", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);

			}
			//if snmp error occurs, we continue with next counter query
		}
	}

	if(!snmpObj->getBulkIfMTU(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfMTU SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfMTU query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfLastChange(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfLastChange SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfLastChange query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	} 

	if(!snmpObj->getBulkIfphysaddress(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfphysaddress SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfphysaddress query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHighSpeed(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHighSpeed SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHighSpeed query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	//as IfCounterDiscontinuityTime is not supported for CMX (evo) and SMX,it will keep "-"
	if((m_environment != CMD_Util::SINGLECP_SMX) && (m_environment != CMD_Util::MULTIPLECP_SMX) && (!(((m_environment == CMD_Util::SINGLECP_CBA) || (m_environment == CMD_Util::MULTIPLECP_CBA)) && (l3SwitchVal == true))))
	{
		if(!snmpObj->getBulkIfCounterDiscontinuityTime(no_of_interface))
		{
			// debugging : printf("\n ..Error in GetBULK of getBulkIfCounterDiscontinuityTime SNMP Connection\n");
			// trace: snmp query failed
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfCounterDiscontinuityTime query failed", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			//if snmp error occurs, we continue with next counter query
		}
	}

	if(!snmpObj->getBulkIfOperStatus(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfOperStatus SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfOperStatus query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsDuplexStatus(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsDuplexStatus SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsDuplexStatus query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCInOctets(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCInOctets SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCInOctets query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	} 

	if(!snmpObj->getBulkIfHCInUcastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCInUcastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCInUcastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCInMulticastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCInMulticastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCInMulticastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCInBroadcastPkts(no_of_interface))

	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCInBroadcastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCInBroadcastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	//Now populate m_ifHCInPkts field
	snmpObj->populateIfHCInPkts(no_of_interface);

	if(!snmpObj->getBulkIfHCOutOctets(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCOutOctets SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCOutOctets query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCOutUcastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCOutUcastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCOutUcastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	} 

	if(!snmpObj->getBulkIfHCOutMulticastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCOutMulticastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCOutMulticastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCOutBroadcastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCOutBroadcastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCOutBroadcastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	//Now populate m_ifHCOutPkts field
	snmpObj->populateIfHCOutPkts(no_of_interface);

	if(!snmpObj->getBulkIfInErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfInErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfInErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfOutErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfOutErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfOutErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfOutDiscards(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfOutDiscards SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfOutDiscards query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with default printout
	}

	return true;
}

//-------------------------------------------------------
// readSnmpVerboseInformation 
//	read verbose data through specified SCX addresses
//-------------------------------------------------------
bool readSnmpVerboseInformation (fixs_cch_cmdshelfmanager *snmpObj,std::string & ipA_String,std::string & ipB_String,int &no_of_interface, int l3SwitchVal)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IP A Address " << ipA_String << std::endl;
	snmpObj->setIpAddress(ipA_String);
	if(!snmpObj->getIfNumber(no_of_interface))
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IP B Address " << ipB_String << std::endl;
		snmpObj->setIpAddress(ipB_String);
		if(!snmpObj->getIfNumber(no_of_interface))
		{
			// trace: snmp query failed
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getIfNumber query failed", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}

			return false;
		}

	}

	// for CMX there are 36 ethernet interface and one VLAN interface.Counters to be fetched only for ethernet interfaces.
	if(l3SwitchVal)
	{
		if(!snmpObj->getBulkIfType(no_of_interface))
		{
			std::string tmpIp = snmpObj->getIpAddress();
			if(tmpIp.compare(ipA_String) == 0)
				snmpObj->setIpAddress(ipB_String);
			else
				snmpObj->setIpAddress(ipA_String);
			if(!snmpObj->getBulkIfType(no_of_interface))
			{
				// debugging : printf("\n ..Error in GetBULK of getBulkIfType SNMP Connection\n");
				// trace: snmp query failed
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfType query failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return false;
			}
		}

		// find number of ethernet interfaces
		int count =0;
		for(int i=0; i < no_of_interface; i++)
		{
			
			if(snmpObj->m_ifType[i] == 6) //6 means ethernet interface
			{
				count++;
			}
			else if ((l3SwitchVal == 2) && (snmpObj->m_ifType[i] == 131))  //aggregator interfaces for SMX transport part
			{
				count++;

			}
		}
		no_of_interface = count; 
	}


	if((m_environment == CMD_Util::SINGLECP_SMX) || (m_environment == CMD_Util::MULTIPLECP_SMX))
	{  // In case of SMX populate interface name with If_MIB::ifName 
		if(!snmpObj->getBulkIfName(no_of_interface))
		{
			// debugging : printf("\n ..Error in GetBULK of getBulkIfName SNMP Connection\n");
			// trace: snmp query failed
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfName query failed", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);

			}
			//if snmp error occurs, we continue with next counter query
		}
	}
	else
	{

		if(!snmpObj->getBulkIfAlias(no_of_interface))
		{
			// debugging : printf("\n ..Error in GetBULK of getBulkIfAlias SNMP Connection\n");
			// trace: snmp query failed
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfAlias query failed", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			//if snmp error occurs, we continue with next counter query
		}
	}

	if(!snmpObj->getBulkIfMTU(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfMTU SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfMTU query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfLastChange(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfLastChange SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfLastChange query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	} 

	if(!snmpObj->getBulkIfphysaddress(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfphysaddress SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfphysaddress query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHighSpeed(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHighSpeed SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHighSpeed query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}


	if((m_environment != CMD_Util::SINGLECP_SMX) && (m_environment != CMD_Util::MULTIPLECP_SMX) && (!(((m_environment == CMD_Util::SINGLECP_CBA) || (m_environment == CMD_Util::MULTIPLECP_CBA)) && (l3SwitchVal == true))))
	{
		if(!snmpObj->getBulkIfCounterDiscontinuityTime(no_of_interface))
		{
			// debugging : printf("\n ..Error in GetBULK of getBulkIfCounterDiscontinuityTime SNMP Connection\n");
			// trace: snmp query failed
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfCounterDiscontinuityTime query failed", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			//if snmp error occurs, we continue with next counter query
		}
	}

	if(!snmpObj->getBulkIfOperStatus(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfOperStatus SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfOperStatus query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsDuplexStatus(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsDuplexStatus SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsDuplexStatus query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCInOctets(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCInOctets SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCInOctets query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	} 

	if(!snmpObj->getBulkIfHCInUcastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCInUcastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCInUcastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCInMulticastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCInMulticastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCInMulticastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCInBroadcastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCInBroadcastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCInBroadcastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	//Now populate m_ifHCInPkts field
	snmpObj->populateIfHCInPkts(no_of_interface);

	if(!snmpObj->getBulkIfHCOutOctets(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCOutOctets SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCOutOctets query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCOutUcastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCOutUcastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCOutUcastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	} 

	if(!snmpObj->getBulkIfHCOutMulticastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCOutMulticastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCOutMulticastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfHCOutBroadcastPkts(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfHCOutBroadcastPkts SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfHCOutBroadcastPkts query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	//Now populate m_ifHCOutPkts field
	snmpObj->populateIfHCOutPkts(no_of_interface);

	if(!snmpObj->getBulkIfInErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfInErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfInErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfOutErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfOutErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfOutErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs, we continue with next counter query
	}

	if(!snmpObj->getBulkIfOutDiscards(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfOutDiscards SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfOutDiscards query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with default printout
	}


	if(!snmpObj->getBulkIfInDiscards(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfInDiscards SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfInDiscards query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkIfInUnknownProtos(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkIfInUnknownProtos SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkIfInUnknownProtos query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCStatsAlignmentErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCStatsAlignmentErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCStatsAlignmentErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCStatsFCSErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCStatsFCSErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCStatsFCSErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsSingleCollisionFrames(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsSingleCollisionFrames SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsSingleCollisionFrames query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsMultipleCollisionFrames(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsMultipleCollisionFrames SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsMultipleCollisionFrames query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsSQETestErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsSQETestErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsSQETestErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsDeferredTransmissions(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsDeferredTransmissions SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsDeferredTransmissions query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsLateCollisions(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsLateCollisions SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsLateCollisions query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsExcessiveCollisions(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsExcessiveCollisions SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsExcessiveCollisions query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCStatsInternalMacTransmitErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCStatsInternalMacTransmitErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCStatsInternalMacTransmitErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3StatsCarrierSenseErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3StatsCarrierSenseErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3StatsCarrierSenseErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCStatsFrameTooLongs(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCStatsFrameTooLongs SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCStatsFrameTooLongs query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCStatsInternalMacReceiveErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCStatsInternalMacReceiveErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCStatsInternalMacReceiveErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCStatsSymbolErrors(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCStatsSymbolErrors SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCStatsSymbolErrors query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCControlInUnknownOpcodes(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCControlInUnknownOpcodes SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCControlInUnknownOpcodes query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCInPauseFrames(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCInPauseFrames SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCInPauseFrames query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue with next counter query
	}

	if(!snmpObj->getBulkDot3HCOutPauseFrames(no_of_interface))
	{
		// debugging : printf("\n ..Error in GetBULK of getBulkDot3HCOutPauseFrames SNMP Connection\n");
		// trace: snmp query failed
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP getBulkDot3HCOutPauseFrames query failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		//if snmp error occurs here, we continue
	}

	return true;
}

//-----------------------------------------------
// scxBoardInfo
//	fetch SCX data according to the options
//-----------------------------------------------
int scxBoardInfo(std::vector<imbInfo> & scxVector,std::string & argMag,std::string & argSlot,unsigned int option, bool isSMX)
{
	int retcode = EXECUTED;
	unsigned long magazineSearch = 0;
	unsigned short slotSearch = 0;
	boardSearch->reset();
	ACS_CS_API_IdList boardList;

	if(isSMX){
		boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SMXB);
	}else{
	boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SCXB);
	}

	//filter magazines/slots
	if(CHECK_OPT_MAG(option) & CHECK_OPT_SLOT(option)) //-m <mag> and -s <slot> set
	{
		if (XCOUNTLS_Util::stringToUlong(argMag, magazineSearch) == false)
			return INCORRECT_MAGAZINE;
		if (XCOUNTLS_Util::stringToUshort(argSlot, slotSearch) == false)
			return INCORRECT_SLOT;
		boardSearch->setMagazine(magazineSearch);
		boardSearch->setSlot(slotSearch);
	}
	else if(CHECK_OPT_MAG(option)) //-m <mag> set
	{
		if (XCOUNTLS_Util::stringToUlong(argMag, magazineSearch) == false)
			return INCORRECT_MAGAZINE;
		boardSearch->setMagazine(magazineSearch);
	}
	else if(CHECK_OPT_SLOT(option)) //-s <slot> setACS_CS_API_HWC_NS:
	{
		if (XCOUNTLS_Util::stringToUshort(argSlot, slotSearch) == false)
			return INCORRECT_SLOT;
		boardSearch->setSlot(slotSearch);
	}

	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
	if (returnValue == ACS_CS_API_NS::Result_Success)
	{
		if (boardList.size() == 0)
		{
			// trace: no SCX board found. Any SCX hasn't been configured yet.
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No SCX board found", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = HARDWARE_NOT_PRESENT;
		}
		else
		{
			// got some SCX board
			for (unsigned int i = 0; i < boardList.size(); i++)
			{
				BoardID boardId = boardList[i];
				uint32_t magazine = 0;
				returnValue = hwc->getMagazine (magazine, boardId);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					// magazine read
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0x%X", __LINE__, magazine);
						_trace.ACS_TRA_event(1, tmpStr);
					}

					unsigned short slot = 0;
					returnValue = hwc->getSlot (slot, boardId);
					if (returnValue == ACS_CS_API_NS::Result_Success)
					{
						// slot read
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
							_trace.ACS_TRA_event(1, tmpStr);
						}

						uint32_t IP_EthA = 0;
						returnValue = hwc->getIPEthA (IP_EthA, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							uint32_t IP_EthB = 0;
							returnValue = hwc->getIPEthB (IP_EthB, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								imbInfo xswitch;
								std::string ipA_String = XCOUNTLS_Util::ulongToStringIP(IP_EthA);
								std::string ipB_String = XCOUNTLS_Util::ulongToStringIP(IP_EthB);
								if(isSMX)
								{
									xswitch.AliasIP_EthA ="";
									xswitch.AliasIP_EthB ="";
									uint32_t AliasIP_EthA = 0;
									returnValue = hwc->getAliasEthA (AliasIP_EthA, boardId);
									if (returnValue == ACS_CS_API_NS::Result_Success)
									{
										uint32_t AliasIP_EthB = 0;
										returnValue = hwc->getAliasEthB (AliasIP_EthB, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
										{

											std::string AliasIpA_String = XCOUNTLS_Util::ulongToStringIP(AliasIP_EthA);
											std::string AliasIpB_String = XCOUNTLS_Util::ulongToStringIP(AliasIP_EthB);

											xswitch.AliasIP_EthA = AliasIpA_String;
											xswitch.AliasIP_EthB = AliasIpB_String;

										}
									}
								}
								///This information is common for all the environments
									xswitch.mag = magazine;
									xswitch.slot = slot;
									xswitch.IP_EthA = ipA_String;
									xswitch.IP_EthB = ipB_String;

									scxVector.push_back(xswitch);
									//debugging : printf("\n...SCX Board IP fetch Successful: ipA =%ld, IpB=%ld\n",IP_EthA,IP_EthB);

							}
							else
							{
								//serious fault in CS: No IP_EthB found for the SCXB board
								if (_trace.ACS_TRA_ON())
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the SCXB board", __LINE__);
									_trace.ACS_TRA_event(1, tmpStr);
								}
								retcode = NO_REPLY_FROM_CS;
								break;
							}
						}
						else
						{
							//serious fault in CS: No IP_EthA found for the SCXB board
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the SCXB board", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							retcode = NO_REPLY_FROM_CS;
							break;
						}
					}
					else
					{
						//serious fault in CS: No slot found for the SCXB board
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the SCXB board", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						retcode = NO_REPLY_FROM_CS;
						break;
					}
				}
				else
				{
					//serious fault in CS: No magazine found for the SCXB board

					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the SCXB board", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					retcode = NO_REPLY_FROM_CS;
					break;
				}

			}

		}
	}
	else
	{
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		retcode = NO_REPLY_FROM_CS;
	}

	return retcode;
}

//-----------------------------------------------
// cmxBoardInfo
//	fetch CMX data according to the options
//-----------------------------------------------
int cmxBoardInfo(std::vector<imbInfo> & cmxVector,std::string & argMag,std::string & argSlot,unsigned int option)
{
	int retcode = EXECUTED;
	unsigned long magazineSearch = 0;
	unsigned short slotSearch = 0;
	boardSearch->reset();
	ACS_CS_API_IdList boardList;

	boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_CMXB);

	//filter magazines/slots
	if(CHECK_OPT_MAG(option) & CHECK_OPT_SLOT(option)) //-m <mag> and -s <slot> set
	{
		if (XCOUNTLS_Util::stringToUlong(argMag, magazineSearch) == false)
			return INCORRECT_MAGAZINE;
		if (XCOUNTLS_Util::stringToUshort(argSlot, slotSearch) == false)
			return INCORRECT_SLOT;
		boardSearch->setMagazine(magazineSearch);
		boardSearch->setSlot(slotSearch);
	}
	else if(CHECK_OPT_MAG(option)) //-m <mag> set
	{
		if (XCOUNTLS_Util::stringToUlong(argMag, magazineSearch) == false)
			return INCORRECT_MAGAZINE;
		boardSearch->setMagazine(magazineSearch);
	}
	else if(CHECK_OPT_SLOT(option)) //-s <slot> setACS_CS_API_HWC_NS:
	{
		if (XCOUNTLS_Util::stringToUshort(argSlot, slotSearch) == false)
			return INCORRECT_SLOT;
		boardSearch->setSlot(slotSearch);
	}

	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
	if (returnValue == ACS_CS_API_NS::Result_Success)
	{
		if (boardList.size() == 0)
		{
			// trace: no CMX board found. Any CMX hasn't been configured yet.
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No CMX board found", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = HARDWARE_NOT_PRESENT;
		}
		else
		{
			// got some CMX board
			for (unsigned int i = 0; i < boardList.size(); i++)
			{
				BoardID boardId = boardList[i];
				uint32_t magazine = 0;
				returnValue = hwc->getMagazine (magazine, boardId);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					// magazine read
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0x%X", __LINE__, magazine);
						_trace.ACS_TRA_event(1, tmpStr);
					}

					unsigned short slot = 0;
					returnValue = hwc->getSlot (slot, boardId);
					if (returnValue == ACS_CS_API_NS::Result_Success)
					{
						// slot read
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
							_trace.ACS_TRA_event(1, tmpStr);
						}

						uint32_t IP_EthA = 0;
						returnValue = hwc->getIPEthA (IP_EthA, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							uint32_t IP_EthB = 0;
							returnValue = hwc->getIPEthB (IP_EthB, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								imbInfo cmx;
								std::string ipA_String = XCOUNTLS_Util::ulongToStringIP(IP_EthA);
								std::string ipB_String = XCOUNTLS_Util::ulongToStringIP(IP_EthB);

								cmx.mag = magazine;
								cmx.slot = slot;
								cmx.IP_EthA = ipA_String;
								cmx.IP_EthB = ipB_String;
								cmxVector.push_back(cmx);
								//debugging : printf("\n...SCX Board IP fetch Successful: ipA =%ld, IpB=%ld\n",IP_EthA,IP_EthB);

							}
							else
							{
								//serious fault in CS: No IP_EthB found for the CMXB board
								if (_trace.ACS_TRA_ON())
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the CMXB board", __LINE__);
									_trace.ACS_TRA_event(1, tmpStr);
								}
								retcode = NO_REPLY_FROM_CS;
								break;
							}
						}
						else
						{
							//serious fault in CS: No IP_EthA found for the CMXB board
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the CMXB board", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							retcode = NO_REPLY_FROM_CS;
							break;
						}
					}
					else
					{
						//serious fault in CS: No slot found for the CMXB board
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the CMXB board", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						retcode = NO_REPLY_FROM_CS;
						break;
					}
				}
				else
				{
					//serious fault in CS: No magazine found for the CMXB board

					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the CMXB board", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					retcode = NO_REPLY_FROM_CS;
					break;
				}

			}

		}
	}
	else
	{
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		retcode = NO_REPLY_FROM_CS;
	}

	return retcode;
}


int allEGEM2L2SwitchBoardInfo(std::vector<imbInfo> & scxVector,bool isSMX)
{
	int retcode = EXECUTED;
	boardSearch->reset();
	ACS_CS_API_IdList boardList;

	if(isSMX){
		boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SMXB);
	}else{
		boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SCXB);
	}


	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);

	if (returnValue == ACS_CS_API_NS::Result_Success)
	{
		if (boardList.size() == 0)
		{
			// trace: no SCX board found. Any SCX hasn't been configured yet.
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No SCX board found", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}

			retcode = HARDWARE_NOT_PRESENT;			
		}
		else
		{
			// got some SCX board
			for (unsigned int i = 0; i < boardList.size(); i++)
			{
				BoardID boardId = boardList[i];
				uint32_t magazine = 0;
				returnValue = hwc->getMagazine (magazine, boardId);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					// magazine read
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0x%X", __LINE__, magazine);
						_trace.ACS_TRA_event(1, tmpStr);
					}

					unsigned short slot = 0;
					returnValue = hwc->getSlot (slot, boardId);

					if (returnValue == ACS_CS_API_NS::Result_Success)
					{
						// slot read
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
							_trace.ACS_TRA_event(1, tmpStr);
						}

						uint32_t IP_EthA = 0;
						returnValue = hwc->getIPEthA (IP_EthA, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							uint32_t IP_EthB = 0;
							returnValue = hwc->getIPEthB (IP_EthB, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								imbInfo xswitch;
								std::string magStr;
								XCOUNTLS_Util::ulongToStringMagazine(magazine, magStr);
								std::string ipA_String = XCOUNTLS_Util::ulongToStringIP(IP_EthA);
								std::string ipB_String = XCOUNTLS_Util::ulongToStringIP(IP_EthB);

								if(isSMX)
								{
									xswitch.AliasIP_EthA="";
									xswitch.AliasIP_EthB="";
									uint32_t AliasIP_EthA = 0;
									returnValue = hwc->getAliasEthA (AliasIP_EthA, boardId);
									if (returnValue == ACS_CS_API_NS::Result_Success)
									{
										uint32_t AliasIP_EthB = 0;
										returnValue = hwc->getAliasEthB (AliasIP_EthB, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
										{

											std::string AliasIpA_String = XCOUNTLS_Util::ulongToStringIP(AliasIP_EthA);
											std::string AliasIpB_String = XCOUNTLS_Util::ulongToStringIP(AliasIP_EthB);

											xswitch.AliasIP_EthA = AliasIpA_String;
											xswitch.AliasIP_EthB = AliasIpB_String;
										}
									}
								}
								xswitch.mag = magazine;
								xswitch.slot = slot;
								xswitch.IP_EthA = ipA_String;
								xswitch.IP_EthB = ipB_String;
								scxVector.push_back(xswitch);
							}
							else
							{
								//serious fault in CS: No IP_EthB found for the SCXB board
								if (_trace.ACS_TRA_ON())
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the SCXB board", __LINE__);
									_trace.ACS_TRA_event(1, tmpStr);
								}
								retcode = NO_REPLY_FROM_CS;
								break;
							}
						}
						else
						{
							//serious fault in CS: No IP_EthA found for the SCXB board
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the SCXB board", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							retcode = NO_REPLY_FROM_CS;
							break;
						}
					}
					else
					{
						//serious fault in CS: No slot found for the SCXB board
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the SCXB board", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						retcode = NO_REPLY_FROM_CS;
						break;
					}
				}
				else
				{
					//serious fault in CS: No magazine found for the SCXB board

					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the SCXB board", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}

					retcode = NO_REPLY_FROM_CS;
					break;
				}

			}

		}
	}
	else
	{
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		retcode = NO_REPLY_FROM_CS;
	}
	return retcode;	
}

int allCMXBoardInfo(std::vector<imbInfo> & cmxVector)
{
	int retcode = EXECUTED;
	boardSearch->reset();
	ACS_CS_API_IdList boardList;

	boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_CMXB);


	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);

	if (returnValue == ACS_CS_API_NS::Result_Success)
	{
		if (boardList.size() == 0)
		{
			// trace: no CMX board found. Any CMX hasn't been configured yet.
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No CMX board found", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}

			retcode = HARDWARE_NOT_PRESENT;			
		}
		else
		{
			// got some CMX board
			for (unsigned int i = 0; i < boardList.size(); i++)
			{
				BoardID boardId = boardList[i];
				uint32_t magazine = 0;
				returnValue = hwc->getMagazine (magazine, boardId);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					// magazine read
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0x%X", __LINE__, magazine);
						_trace.ACS_TRA_event(1, tmpStr);
					}

					unsigned short slot = 0;
					returnValue = hwc->getSlot (slot, boardId);

					if (returnValue == ACS_CS_API_NS::Result_Success)
					{
						// slot read
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
							_trace.ACS_TRA_event(1, tmpStr);
						}

						uint32_t IP_EthA = 0;
						returnValue = hwc->getIPEthA (IP_EthA, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							uint32_t IP_EthB = 0;
							returnValue = hwc->getIPEthB (IP_EthB, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
										imbInfo cmx;
										std::string magStr;
										XCOUNTLS_Util::ulongToStringMagazine(magazine, magStr);
										std::string ipA_String = XCOUNTLS_Util::ulongToStringIP(IP_EthA);
										std::string ipB_String = XCOUNTLS_Util::ulongToStringIP(IP_EthB);

										cmx.mag = magazine;
										cmx.slot = slot;
										cmx.IP_EthA = ipA_String;
										cmx.IP_EthB = ipB_String;

										cmxVector.push_back(cmx);

							}
							else
							{
								//serious fault in CS: No IP_EthB found for the CMXB board
								if (_trace.ACS_TRA_ON())
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the CMXB board", __LINE__);
									_trace.ACS_TRA_event(1, tmpStr);
								}
								retcode = NO_REPLY_FROM_CS;
								break;
							}
						}
						else
						{
							//serious fault in CS: No IP_EthA found for the CMXB board
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the CMXB board", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							retcode = NO_REPLY_FROM_CS;
							break;
						}
					}
					else
					{
						//serious fault in CS: No slot found for the CMXB board
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the CMXB board", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						retcode = NO_REPLY_FROM_CS;
						break;
					}
				}
				else
				{
					//serious fault in CS: No magazine found for the CMXB board

					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the CMXB board", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}

					retcode = NO_REPLY_FROM_CS;
					break;
				}

			}

		}
	}
	else
	{
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		retcode = NO_REPLY_FROM_CS;
	}

	return retcode;	
}

//-----------------------------------------------------------
// fetchBaselineData
//    Fetches the required performance counters and fills the data structure
//-----------------------------------------------------------
int fetchBaselineData(fixs_cch_cmdshelfmanager *snmpObj,int no_of_intrface, XCOUNTLS_Util::PCData &data)
{
	TraceInOut inOut(__FUNCTION__);
	ULONGLONG tempCurrTime = XCOUNTLS_Util::getEpochTimeInSeconds();

	if (!XCOUNTLS_Util::ulonglongToString(tempCurrTime, data.timeValue)) //longlong
	{
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		return INTERNALERR;
	}
	int i=0;
	for (i=0;i<no_of_intrface;i++)
	{
		if((snmpObj->m_ifAlias[i] == " " )||(snmpObj->m_ifAlias[i] == ""))
			data.interfaceName[i] = "-";
		else
			data.interfaceName[i]= snmpObj->m_ifAlias[i];  //string

		if (snmpObj->m_ifMTU[i] == -1)
			data.counterValue[i][0] = "-";
		else
		{
			if (!XCOUNTLS_Util::longToString(snmpObj->m_ifMTU[i], data.counterValue[i][0])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		data.counterValue[i][1]= snmpObj->m_ifLastChange[i]; //string

		data.counterValue[i][2]= snmpObj->m_ifphysaddress[i]; //string

		if (snmpObj->m_ifHighSpeed[i] == ULONG_MAX)
			data.counterValue[i][3] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_ifHighSpeed[i], data.counterValue[i][3])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		data.counterValue[i][4]= snmpObj->m_ifCounterDiscontinuityTime[i]; //string

		data.counterValue[i][5]= snmpObj->m_ifOperStatus[i]; //string

		data.counterValue[i][6]= snmpObj->m_dot3StatsDuplexStatus[i]; //string

		if (snmpObj->m_ifHCInOctets[i] == ULLONG_MAX)
			data.counterValue[i][7] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCInOctets[i], data.counterValue[i][7])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCInUcastPkts[i] == ULLONG_MAX)
			data.counterValue[i][8] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCInUcastPkts[i], data.counterValue[i][8])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCInMulticastPkts[i] == ULLONG_MAX)
			data.counterValue[i][9] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCInMulticastPkts[i], data.counterValue[i][9])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCInBroadcastPkts[i] == ULLONG_MAX)
			data.counterValue[i][10] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCInBroadcastPkts[i], data.counterValue[i][10])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCInPkts[i] == ULLONG_MAX)
			data.counterValue[i][11] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCInPkts[i], data.counterValue[i][11])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCOutOctets[i] == ULLONG_MAX)
			data.counterValue[i][12] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCOutOctets[i], data.counterValue[i][12])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCOutUcastPkts[i] == ULLONG_MAX)
			data.counterValue[i][13] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCOutUcastPkts[i], data.counterValue[i][13])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCOutMulticastPkts[i] == ULLONG_MAX)
			data.counterValue[i][14] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCOutMulticastPkts[i], data.counterValue[i][14])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCOutBroadcastPkts[i] == ULLONG_MAX)
			data.counterValue[i][15] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCOutBroadcastPkts[i], data.counterValue[i][15])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifHCOutPkts[i] == ULLONG_MAX)
			data.counterValue[i][16] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_ifHCOutPkts[i], data.counterValue[i][16])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifInErrors[i] == ULONG_MAX)
			data.counterValue[i][17] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_ifInErrors[i], data.counterValue[i][17])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifOutErrors[i] == ULONG_MAX)
			data.counterValue[i][18] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_ifOutErrors[i], data.counterValue[i][18])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifOutDiscards[i] == ULONG_MAX)
			data.counterValue[i][19] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_ifOutDiscards[i], data.counterValue[i][19])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifInDiscards[i] == ULONG_MAX)
			data.counterValue[i][20] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_ifInDiscards[i], data.counterValue[i][20])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_ifInUnknownProtos[i] == ULONG_MAX)
			data.counterValue[i][21] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_ifInUnknownProtos[i], data.counterValue[i][21])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCStatsAlignmentErrors[i] == ULLONG_MAX)
			data.counterValue[i][22] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCStatsAlignmentErrors[i], data.counterValue[i][22])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCStatsFCSErrors[i] == ULLONG_MAX)
			data.counterValue[i][23] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCStatsFCSErrors[i], data.counterValue[i][23])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3StatsSingleCollisionFrames[i] == ULONG_MAX)
			data.counterValue[i][24] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_dot3StatsSingleCollisionFrames[i], data.counterValue[i][24])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3StatsMultipleCollisionFrames[i] == ULONG_MAX)
			data.counterValue[i][25] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_dot3StatsMultipleCollisionFrames[i], data.counterValue[i][25])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3StatsSQETestErrors[i] == ULONG_MAX)
			data.counterValue[i][26] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_dot3StatsSQETestErrors[i], data.counterValue[i][26])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3StatsDeferredTransmissions[i] == ULONG_MAX)
			data.counterValue[i][27] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_dot3StatsDeferredTransmissions[i], data.counterValue[i][27])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3StatsLateCollisions[i] == ULONG_MAX)
			data.counterValue[i][28] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_dot3StatsLateCollisions[i], data.counterValue[i][28])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3StatsExcessiveCollisions[i] == ULONG_MAX)
			data.counterValue[i][29] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_dot3StatsExcessiveCollisions[i], data.counterValue[i][29])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCStatsInternalMacTransmitErrors[i] == ULLONG_MAX)
			data.counterValue[i][30] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCStatsInternalMacTransmitErrors[i], data.counterValue[i][30])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3StatsCarrierSenseErrors[i] == ULONG_MAX)
			data.counterValue[i][31] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulongToString(snmpObj->m_dot3StatsCarrierSenseErrors[i], data.counterValue[i][31])) //long
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCStatsFrameTooLongs[i] == ULLONG_MAX)
			data.counterValue[i][32] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCStatsFrameTooLongs[i], data.counterValue[i][32])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCStatsInternalMacReceiveErrors[i] == ULLONG_MAX)
			data.counterValue[i][33] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCStatsInternalMacReceiveErrors[i], data.counterValue[i][33])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCStatsSymbolErrors[i] == ULLONG_MAX)
			data.counterValue[i][34] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCStatsSymbolErrors[i], data.counterValue[i][34])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCControlInUnknownOpcodes[i] == ULLONG_MAX)
			data.counterValue[i][35] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCControlInUnknownOpcodes[i], data.counterValue[i][35])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCInPauseFrames[i] == ULLONG_MAX)
			data.counterValue[i][36] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCInPauseFrames[i], data.counterValue[i][36])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}

		if (snmpObj->m_dot3HCOutPauseFrames[i] == ULLONG_MAX)
			data.counterValue[i][37] = "-";
		else
		{
			if (!XCOUNTLS_Util::ulonglongToString(snmpObj->m_dot3HCOutPauseFrames[i], data.counterValue[i][37])) //longlong
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
		}
	}


	return EXECUTED;

}

//-----------------------------------------------------------
// sortVector
//    sort the vector list based on magazine and slot number
//-----------------------------------------------------------
void sortVector(std::vector<imbInfo> & imbVector)
{
	std::vector<imbInfo> tempPrintOutVector;     //temp vector holding the imbInfo structs
	std::vector<imbInfo>::iterator tempIterator;

	int leastVal = 0;
	int loop = (int) imbVector.size() - 1;

	if (loop <= 0)    //vector only contains 1 board, i.e. list sorted
		return;

	for (int i = 0; i < loop; i++)
	{
		for (int j = 0; j < (int)imbVector.size() - 1; j++)
		{
			if (ntohl(imbVector[leastVal].mag) > ntohl(imbVector[j+1].mag))
				leastVal = j + 1;

			else if (ntohl(imbVector[leastVal].mag) == ntohl(imbVector[j+1].mag))
			{
				if (imbVector[leastVal].slot > imbVector[j+1].slot)
					leastVal = j + 1;
			}
		}

		tempPrintOutVector.push_back(imbVector[leastVal]);
		tempIterator = imbVector.begin();
		tempIterator = tempIterator + leastVal;
		(void)imbVector.erase(tempIterator); //erases the board with the least mag and slot addr from the vector
		leastVal = 0;  //clear the leastVal in order to start with first vector element in next loop

		if (imbVector.size() == 1)        //if there's only one board left in the vector
		{
			tempPrintOutVector.push_back(imbVector[0]);         //add the board in the tempvector
			tempIterator = imbVector.begin();
			(void)imbVector.erase(tempIterator);          //erases the board from the vector  
		}
	}
	imbVector.assign(tempPrintOutVector.begin(), tempPrintOutVector.end());   

}

//-------------------------------------------------------------
// isOperationAllowed
//	check if the board specified by magazine&address is SCXB
//-------------------------------------------------------------
int isOperationAllowed (unsigned long umagazine,unsigned short uslot)
{
	TraceInOut inOut(__FUNCTION__);
	// std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slot " << uslot << std::endl;
	int retValue = EXECUTED;
	boardSearch->reset();
	boardSearch->setMagazine(umagazine);
	boardSearch->setSlot(uslot);
	ACS_CS_API_IdList boardList;
	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
	if (returnValue == ACS_CS_API_NS::Result_Success)
	{
		if (boardList.size() == 1)
		{
			BoardID boardId = boardList[0];
			unsigned short fbn = 0;
			returnValue = hwc->getFBN(fbn, boardId);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if ((fbn == ACS_CS_API_HWC_NS::FBN_SCXB) || (fbn == ACS_CS_API_HWC_NS::FBN_CMXB) || (fbn == ACS_CS_API_HWC_NS::FBN_SMXB))
				{
					retValue = EXECUTED;
				}
				else 
				{
					retValue = FUNC_BOARD_NOTALLOWED;
				}
			}
			else
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,
							"[%s@%d] Cannot retrieve FBN from CS", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retValue = NO_REPLY_FROM_CS;
			}
		}
		else
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Board List is empty", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retValue = HARDWARE_NOT_PRESENT;
		}
	}
	else
	{
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		retValue = NO_REPLY_FROM_CS;
	}

	return retValue;
}

int isOperationOnSlotAllowed (unsigned short uslot)
{
	TraceInOut inOut(__FUNCTION__);
	// std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slot " << uslot << std::endl;
	int retValue = EXECUTED;
	boardSearch->reset();
	boardSearch->setSlot(uslot);
	ACS_CS_API_IdList boardList;
	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
	if (returnValue == ACS_CS_API_NS::Result_Success)
	{
		if (boardList.size() == 0)
		{
			// trace: no board found.
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No SCX or CMX board found", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retValue = HARDWARE_NOT_PRESENT;
		}
		else
		{
			for (unsigned int i = 0; i < boardList.size(); i++)
			{
				BoardID boardId = boardList[i];
				unsigned short fbn = 0;
				returnValue = hwc->getFBN(fbn, boardId);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					if ((fbn == ACS_CS_API_HWC_NS::FBN_SCXB) || (fbn == ACS_CS_API_HWC_NS::FBN_CMXB)|| (fbn == ACS_CS_API_HWC_NS::FBN_SMXB))
					{
						return EXECUTED;
					}
				}
				else
				{
					if (_trace.ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1,
								"[%s@%d] Cannot retrieve FBN from CS", __FUNCTION__, __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return NO_REPLY_FROM_CS;
				}
			}
			retValue = FUNC_BOARD_NOTALLOWED;
		}
	}
	else
	{
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		retValue = NO_REPLY_FROM_CS;
	}

	return retValue;
}

int isOperationOnMagazineAllowed (unsigned long umagazine)
{
	TraceInOut inOut(__FUNCTION__);
	// std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " magazine " << umagazine << std::endl;
	int retValue = EXECUTED;
	boardSearch->reset();
	boardSearch->setMagazine(umagazine);
	ACS_CS_API_IdList boardList;
	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
	if (returnValue == ACS_CS_API_NS::Result_Success)
	{
		if (boardList.size() == 0)
		{
			// trace: no board found.
			if (_trace.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No SCX board found", __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retValue = HARDWARE_NOT_PRESENT;
		}
		else
		{
			for (unsigned int i = 0; i < boardList.size(); i++)
			{
				BoardID boardId = boardList[i];
				unsigned short fbn = 0;
				returnValue = hwc->getFBN(fbn, boardId);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					if ((fbn == ACS_CS_API_HWC_NS::FBN_SCXB) || (fbn == ACS_CS_API_HWC_NS::FBN_CMXB) || (fbn == ACS_CS_API_HWC_NS::FBN_SMXB))
					{
						return EXECUTED;
					}
				}
				else
				{
					if (_trace.ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1,
								"[%s@%d] Cannot retrieve FBN from CS", __FUNCTION__, __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return NO_REPLY_FROM_CS;
				}
			}
			retValue = FUNC_BOARD_NOTALLOWED;
		}
	}
	else
	{
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		retValue = NO_REPLY_FROM_CS;
	}

	return retValue;
}

//---------------------------------
// fetchSwitchInfo
//		get IP addresses of the board
//---------------------------------
int fetchSwitchInfo (unsigned long umagazine,unsigned short uslot,uint32_t &uIP_EthA,uint32_t &uIP_EthB,uint32_t &uAliasIP_EthA,uint32_t &uAliasIP_EthB,bool isSMX)
{
	TraceInOut inOut(__FUNCTION__);
	int retValue = EXECUTED;
	ACS_CS_API_IdList boardList;

	boardSearch->reset();
	boardSearch->setMagazine(umagazine);
	boardSearch->setSlot(uslot);

	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
	if (returnValue == ACS_CS_API_NS::Result_Success)
	{
		if (boardList.size() == 1)
		{
			BoardID boardId = boardList[0];
			uint32_t IP_EthA = 0;
			returnValue = hwc->getIPEthA (IP_EthA, boardId);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				uint32_t IP_EthB = 0;
				returnValue = hwc->getIPEthB (IP_EthB, boardId);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					if(isSMX)
					{
						uint32_t AliasIP_EthA = 0;
						returnValue = hwc->getAliasEthA (AliasIP_EthA, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							uint32_t AliasIP_EthB = 0;
							returnValue = hwc->getAliasEthB (AliasIP_EthB, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{

								uAliasIP_EthA = AliasIP_EthA;
								uAliasIP_EthB = AliasIP_EthB;
								retValue = EXECUTED;

							}
						}
					}
					uIP_EthA = IP_EthA;
					uIP_EthB = IP_EthB;
				}
				else
				{
					if (_trace.ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] getIPEthB failed, error from CS", __FUNCTION__, __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					retValue = NO_REPLY_FROM_CS;
				}
			}
			else
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] getIPEthA failed, error from CS", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retValue = NO_REPLY_FROM_CS;
			}

		}
		else
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Board List is empty", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retValue = HARDWARE_NOT_PRESENT;
		}
	}
	else
	{
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
		retValue = NO_REPLY_FROM_CS;
	}
	return retValue;
}

//-------------------------------------
// pcVerboseData
//	Performance Counters Verbose Data
//-------------------------------------
int pcVerboseData(unsigned long umagazine,unsigned short uslot,unsigned int option,int fileExists)
{
	int retcode = EXECUTED;
	uint32_t uIP_EthA = 0;
	uint32_t uIP_EthB = 0;
	uint32_t uAliasIP_EthA = 0;
	uint32_t uAliasIP_EthB = 0;
	fixs_cch_cmdshelfmanager ironsideManager("");
	
	std::string magStr;
	//IN DMX Environment SCX is not defined in HWC Table. Skip this check!
	if (m_environment != CMD_Util::SINGLECP_DMX && m_environment != CMD_Util::MULTIPLECP_DMX)
	{
		hwc = ACS_CS_API::createHWCInstance();
		if (hwc)
		{
			boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
			if (boardSearch)
			{
				retcode = isOperationAllowed(umagazine,uslot);
				if (retcode == EXECUTED)
				{
					retcode = fetchSwitchInfo(umagazine,uslot,uIP_EthA,uIP_EthB,uAliasIP_EthA,uAliasIP_EthB,isSMX);
				}
			}
			else
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance() failed", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retcode = INTERNALERR;//Memory error
			}
		}
		else
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createHWCInstance() failed", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = INTERNALERR;//Memory error
		}

		if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
	   vector<std::string> portVec;
	   hwc = ACS_CS_API::createHWCInstance();
	   if (hwc)
	   {
		boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
		//IN DMX Environment SCX and CMX are not defined in HWC Table. 
		//So we check directly the uslot value instead of isOperationAllowed
			if (uslot == 0 || uslot == 25 || uslot == 26 || uslot == 28 )
			{
				XCOUNTLS_Util::ulongToStringMagazine(umagazine, magStr);

				if (ironsideManager.checkShelfFromAddressUDP(magStr,retcode) != true)
				{
				//		retcode = HARDWARE_NOT_PRESENT;
				}

			}
			else
			{
				retcode = isOperationAllowed(umagazine,uslot); 
			}
		}
		else
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance() failed", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = INTERNALERR;//Memory error
		}
	   }
	   else
	   {
		if (_trace.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createHWCInstance() failed", __FUNCTION__, __LINE__);
			_trace.ACS_TRA_event(1, tmpStr);
		}
			retcode = INTERNALERR;//Memory error
	   }

	   if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	   if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
	}   
	if(retcode == EXECUTED)
	{
		//-----------------------------------------------
		// COMMON PRINTOUT 
		//-----------------------------------------------
		std::cout << std::endl << "BOARD COUNTERS" << std::endl;
		std::string currentDate="-";
		std::string currentTime="-";
		std::string baselineTime="-";
		ULONGLONG currTimeSec =0;
		XCOUNTLS_Util::getCurrentTime(currentDate, currentTime);
		std::cout << "TIME " << currentDate << setw(1) << "" << currentTime << std::endl;
		if(fileExists == true)
		{
			currTimeSec = XCOUNTLS_Util::getEpochTimeInSeconds();

			if (FIXS_CCH_PCDataStorage::getInstance()->loadBaseline()  == 0) {
				if(!FIXS_CCH_PCDataStorage::getInstance()->readBaselineHeading(baselineTime))
				{
					if (_trace.ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] readBaselineHeading() failed", __FUNCTION__, __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					retcode = INTERNALERR;
				}
			}
			else {
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] loadBaseline() failed", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retcode = INTERNALERR;
			}

			if(retcode == INTERNALERR) //in case of internal error while reading baseline
				return retcode;

		}

		std::cout << "BASELINE TIME: " << baselineTime << std::endl;

		fixs_cch_cmdshelfmanager snmpManager_C("");
		fixs_cch_cmdshelfmanager* shelfManager_C = &snmpManager_C;
		fixs_cch_cmdshelfmanager snmpManager_T("");
		fixs_cch_cmdshelfmanager* shelfManager_T = &snmpManager_T;
		int no_of_intrface = -1;
		int intrfc_cnt[2]={-1};
		bool snmpflag = true;
		bool isTransportSwitch=false;
		std::string magStr;
		std::string slotStr;
		XCOUNTLS_Util::ulongToStringMagazine(umagazine, magStr);
		XCOUNTLS_Util::ushortToString(uslot,slotStr);
		std::string IP_EthA,IP_EthB,AliasIP_EthA,AliasIP_EthB;
		IP_EthA = XCOUNTLS_Util::ulongToStringIP (uIP_EthA);
		IP_EthB = XCOUNTLS_Util::ulongToStringIP (uIP_EthB);
		AliasIP_EthA = XCOUNTLS_Util::ulongToStringIP (uAliasIP_EthA);
		AliasIP_EthB = XCOUNTLS_Util::ulongToStringIP (uAliasIP_EthB);
		int l3SwitchVal =0;
		if((uslot == 26) || (uslot == 28))
		{
			l3SwitchVal = 1;
		}

		if ((m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX) )
		{
			snmpflag = readIronsideVerboseInformation(&ironsideManager,magStr,slotStr,no_of_intrface);
			shelfManager_C = &ironsideManager;
			intrfc_cnt[0]=no_of_intrface;
		}
		else
		{
			isTransportSwitch=false;
			snmpflag = readSnmpVerboseInformation(&snmpManager_C, IP_EthA, IP_EthB, no_of_intrface, l3SwitchVal);
			intrfc_cnt[0]=no_of_intrface;

			if(isSMX){
			isTransportSwitch=true;
			if (isSMX) l3SwitchVal = 2;  // SMX transport switch
			snmpflag = readSnmpVerboseInformation(&snmpManager_T, AliasIP_EthA, AliasIP_EthB, no_of_intrface, l3SwitchVal);
			intrfc_cnt[1]=no_of_intrface;
			}
		}


		std::cout << std::endl << "MAG   " << magStr << "   SLOT " << uslot;

		if(snmpflag == false)
		{
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__;
			if ((m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX) )
				std::cout<< "\nCOMMUNICATION ERROR:NO AVAILABLE DATA" << std::endl;
			else
				std::cout<< "\nSNMP ERROR:NO AVAILABLE DATA" << std::endl;
			retcode = SNMP_FAILURE;
		}
		else
		{
			XCOUNTLS_Util::PCData data;
			PCIncrRateData pcCal;
			initializePCIncrRateData(pcCal);
			int  base_interface[2]={-1};
			if(fileExists == true)
			{
				if(FIXS_CCH_PCDataStorage::getInstance()->readBoardCounters(umagazine,uslot,data,base_interface,isSMX))
				{
					retcode = EXECUTED;
				}
				else
				{
					if (_trace.ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] readBoardCounters() failed", __FUNCTION__, __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					std::cout << "\nBaseline not available for all rates/increment of that board" << std::endl;
					return EXECUTED;
				}

				isTransportSwitch=false;
				retcode = calculateVerboseIncrRateValues(shelfManager_C,data,pcCal,currTimeSec,intrfc_cnt[0],base_interface[0],isTransportSwitch);
				if(retcode == INTERNALERR)
				{
					//std::cout << "\nError when executing" << std::endl;
					return retcode;
				}
				else{
					retcode = printVerboseResult(shelfManager_C,pcCal, intrfc_cnt[0],option,isTransportSwitch);
					if(retcode == INTERNALERR)
					{
						//std::cout << "\nError when executing" << std::endl;
						return retcode;
					}
				}

				if(isSMX)
				{
					isTransportSwitch=true;
					retcode = calculateVerboseIncrRateValues(shelfManager_T,data,pcCal,currTimeSec,intrfc_cnt[1],base_interface[1],isTransportSwitch);
					if(retcode == INTERNALERR)
					{
						//std::cout << "\nError when executing" << std::endl;
						return retcode;
					}else{
						retcode = printVerboseResult(shelfManager_T,pcCal, intrfc_cnt[1],option,isTransportSwitch);
						if(retcode == INTERNALERR)
						{
							//std::cout << "\nError when executing" << std::endl;
							return retcode;
						}
					}

				}

			}else{
				isTransportSwitch=false;
				retcode = printVerboseResult(shelfManager_C,pcCal, intrfc_cnt[0],option,isTransportSwitch);
				if(retcode == INTERNALERR)
				{
					//std::cout << "\nError when executing" << std::endl;
					return retcode;
				}
				if(isSMX)
				{
					isTransportSwitch=true;
					retcode = printVerboseResult(shelfManager_T,pcCal, intrfc_cnt[1],option,isTransportSwitch);
					if(retcode == INTERNALERR)
					{
						//std::cout << "\nError when executing" << std::endl;
						return retcode;
					}
				}

			} //end of else  (file doesnt exists)



		}

	}
	return retcode;
}
//-------------------------------------
// pcDefaultData
//	Performance Counters Default Data
//-------------------------------------
int pcDefaultData(unsigned long umagazine,unsigned short uslot,unsigned int option,int fileExists)
{
	int retcode = EXECUTED;
	std::vector<imbInfo> imbVector;
	fixs_cch_cmdshelfmanager ironsideManager("");
//	unsigned long uIP_EthA = 0;
//	unsigned long uIP_EthB = 0;
	
		std::string magStr;
	if (m_environment != CMD_Util::SINGLECP_DMX && m_environment != CMD_Util::MULTIPLECP_DMX)
	{
		hwc = ACS_CS_API::createHWCInstance();

		if (hwc)
		{
			boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
			if (boardSearch)
			{
				if(CHECK_OPT_MAG(option) & CHECK_OPT_SLOT(option))
				{
					retcode = isOperationAllowed(umagazine,uslot);
				}
				else if(CHECK_OPT_SLOT(option))
				{
					retcode = isOperationOnSlotAllowed(uslot);
				}
				else if(CHECK_OPT_MAG(option))
				{
					retcode = isOperationOnMagazineAllowed(umagazine);
				}
				if(retcode == EXECUTED)
				{
					std::string argMag = "",argSlot="";
					XCOUNTLS_Util::ulongToStringMagazine(umagazine, argMag);
					XCOUNTLS_Util::ushortToString(uslot,argSlot);
					retcode = scxBoardInfo(imbVector,argMag,argSlot,option,isSMX);
					if ((retcode == EXECUTED) || (retcode == HARDWARE_NOT_PRESENT))
					{
						int retcodeTemp = cmxBoardInfo(imbVector,argMag,argSlot,option);
						if(retcodeTemp != HARDWARE_NOT_PRESENT)
							retcode = retcodeTemp;
					}
				}
			}
			else
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance() failed", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retcode = INTERNALERR;//Memory error
			}
		}
		else
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createHWCInstance() failed", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = INTERNALERR;//Memory error
		}

		if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
	}
	else ///DMX ENVIRONMENT
	{
	
		//TODO: Check that the HW actually exists in DMX and get the right Shelf Id. Then fill imbVector
		//Temporary for debug ////////////////////////////////////////////////////////////////////////////
		hwc = ACS_CS_API::createHWCInstance();
		if (hwc)
		{
		
			boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
			if (boardSearch)
			{
				if(CHECK_OPT_MAG(option) & CHECK_OPT_SLOT(option))
				{
				//IN DMX Environment SCX and CMX are not defined in HWC Table. 
				//So we check directly the uslot value instead of isOperationAllowed
					if (uslot == 0 || uslot == 25 || uslot ==26 ||uslot ==28)
					{
						XCOUNTLS_Util::ulongToStringMagazine(umagazine, magStr);

						if (ironsideManager.checkShelfFromAddressUDP(magStr,retcode) == true)
						{
							imbInfo scx;
							scx.IP_EthA = "";
							scx.IP_EthB = "";
							scx.mag = umagazine;
							scx.slot = uslot;

							imbVector.push_back(scx);
						}
					}
					else
					{
						retcode = isOperationAllowed(umagazine, uslot); 
					}
				}
				else if(CHECK_OPT_SLOT(option))
				{

					if (uslot == 0 || uslot == 25  || uslot ==26 ||uslot ==28)
					{
		//				std::cout << "@@@DBG: Getting Shelf List from DMX" << std::endl;
						std::vector<std::string> magAddrs;
						retcode = ironsideManager.getShelfListUDP(magAddrs);
						if(retcode == XCOUNTLS_Util::EXECUTED)
						{
							for (std::vector<std::string>::iterator it = magAddrs.begin(); it != magAddrs.end(); it++)
							{
								imbInfo scx;

								scx.IP_EthA = "";
								scx.IP_EthB = "";
								XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), scx.mag);
								scx.slot = uslot;

								imbVector.push_back(scx);
							}
						}

					}
					else
					{
						retcode = isOperationOnSlotAllowed(uslot);
					}
				}
				else if(CHECK_OPT_MAG(option))
				{
					XCOUNTLS_Util::ulongToStringMagazine(umagazine, magStr);

					if (ironsideManager.checkShelfFromAddressUDP(magStr,retcode) == true)
					{
						imbInfo scx_0;
						scx_0.IP_EthA = "";
						scx_0.IP_EthB = "";
						scx_0.mag = umagazine;
						scx_0.slot = 0;
						imbVector.push_back(scx_0);
						imbInfo scx_25;
						scx_25.IP_EthA = "";
						scx_25.IP_EthB = "";
						scx_25.mag = umagazine;
						scx_25.slot = 25;
						imbVector.push_back(scx_25);
						imbInfo cmx_26;
						cmx_26.IP_EthA = "";
						cmx_26.IP_EthB = "";
						cmx_26.mag = umagazine;
						cmx_26.slot = 26;
						imbVector.push_back(cmx_26);
						imbInfo cmx_28;
						cmx_28.IP_EthA = "";
						cmx_28.IP_EthB = "";
						cmx_28.mag = umagazine;
						cmx_28.slot = 28;
						imbVector.push_back(cmx_28);

					}
					
				}
				else
				{
//					std::cout << "@@@DBG: Getting Shelf List from DMX" << std::endl;
					std::vector<std::string> magAddrs;
					retcode = ironsideManager.getShelfListUDP(magAddrs);
					if(retcode == XCOUNTLS_Util::EXECUTED)
					{
						for (std::vector<std::string>::iterator it = magAddrs.begin(); it != magAddrs.end(); it++)
						{

							imbInfo scx_0;
							scx_0.IP_EthA = "";
							scx_0.IP_EthB = "";
							XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), scx_0.mag);
							scx_0.slot = 0;
							imbVector.push_back(scx_0);
							imbInfo scx_25;
							scx_25.IP_EthA = "";
							scx_25.IP_EthB = "";
							XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), scx_25.mag);
							scx_25.slot = 25;
							imbVector.push_back(scx_25);
							imbInfo cmx_26;
							cmx_26.IP_EthA = "";
							cmx_26.IP_EthB = "";
							XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), cmx_26.mag);
							cmx_26.slot = 26;
							imbVector.push_back(cmx_26);
							imbInfo cmx_28;
							cmx_28.IP_EthA = "";
							cmx_28.IP_EthB = "";
							XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), cmx_28.mag);
							cmx_28.slot = 28;
							imbVector.push_back(cmx_28);

						}
					}
				}
	   		}
			else
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance() failed", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retcode = INTERNALERR;//Memory error
			}
		}
		else
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createHWCInstance() failed", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = INTERNALERR;//Memory error
		}
		if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
		//Temporary for debug ////////////////////////////////////////////////////////////////////////////

	}

	if(retcode == INTERNALERR)
	{
		std::cout << "Error when executing" << std::endl;
		return retcode;
	}

	if(retcode == EXECUTED)
	{
		int size = (int)imbVector.size();
		int SNMPFailCount = 0;
		int statusFailCount = 0;
		int countNotExistInterface =0;

		if(size == 0)
		{
			retcode = HARDWARE_NOT_PRESENT;
			return retcode;
		}
		sortVector(imbVector); // sort the list of SCX and CMX boards

		//-----------------------------------------------
		// COMMON PRINTOUT FOR ALL OPTIONS
		//-----------------------------------------------

		std::cout << std::endl << "BOARD COUNTERS" << std::endl;
		std::string currentDate="-";
		std::string currentTime="-";
		std::string baselineTime="-";
		ULONGLONG currTimeSec =0;
		XCOUNTLS_Util::getCurrentTime(currentDate, currentTime);
		std::cout << "TIME " << currentDate << setw(1) << "" << currentTime << std::endl;
		if(fileExists == true)
		{
			currTimeSec = XCOUNTLS_Util::getEpochTimeInSeconds();

			if (FIXS_CCH_PCDataStorage::getInstance()->loadBaseline()==0) {
				if(	!FIXS_CCH_PCDataStorage::getInstance()->readBaselineHeading(baselineTime))
				{
					if (_trace.ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] readBaselineHeading() failed", __FUNCTION__, __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					//in case of internal error while reading baseline headerSection
					std::cout << "\nError when executing" << std::endl;
					retcode = INTERNALERR;
					return retcode;
				}
			} else {
				//in case of internal error while reading baseline headerSection
				std::cout << "\nError when executing" << std::endl;
				retcode = INTERNALERR;
				return retcode;
			}
		}
		std::cout << "BASELINE TIME: " << baselineTime << std::endl <<std::endl;


		for(int i=0; i <= (size-1); i++)
		{
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " - size = " << size << std::endl;
			fixs_cch_cmdshelfmanager snmpManager_C("");
			fixs_cch_cmdshelfmanager* shelfManager_C = &snmpManager_C;
			fixs_cch_cmdshelfmanager snmpManager_T("");
			fixs_cch_cmdshelfmanager* shelfManager_T = &snmpManager_T;
			int l3SwitchVal = 0;
			int no_of_interface = -1;
			int intrfc_cnt[2] = {-1};
			bool isTransportSwitch=false;//@
			//bool snmpflagDisp = true;
			bool snmpflag = true;
			std::string magStr;
			std::string slotStr;
			(void) XCOUNTLS_Util::ulongToStringMagazine(imbVector[i].mag, magStr);
			(void) XCOUNTLS_Util::ushortToString(imbVector[i].slot,slotStr);
			if((imbVector[i].slot == 26 ) || (imbVector[i].slot == 28 ))
			{
				l3SwitchVal = 1;
			}

			if ((m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX) )
			{
				snmpflag = readIronsideDefaultInformation(&ironsideManager, magStr, slotStr, no_of_interface);
				shelfManager_C = &ironsideManager;
				intrfc_cnt[0]=no_of_interface;
			}
			else
			{
				snmpflag = readSnmpDefaultInformation(&snmpManager_C, imbVector[i].IP_EthA, imbVector[i].IP_EthB, no_of_interface,l3SwitchVal);
				intrfc_cnt[0]=no_of_interface;

				if(isSMX){
					isTransportSwitch=true;
					l3SwitchVal=2;
					snmpflag = readSnmpDefaultInformation(&snmpManager_T, imbVector[i].AliasIP_EthA, imbVector[i].AliasIP_EthB, no_of_interface,l3SwitchVal);
					intrfc_cnt[1]=no_of_interface;
				}
			}

			std::cout <<  "MAG   " << magStr << "   SLOT " << imbVector[i].slot;

			if(snmpflag == false)
			{
				if ((m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX) )
                                	std::cout<< "\nCOMMUNICATION ERROR:NO AVAILABLE DATA" << std::endl;
                        	else
					std::cout << "\nSNMP ERROR:NO AVAILABLE DATA" << std::endl;
				SNMPFailCount++; 
				continue;
			}
			else
			{
				XCOUNTLS_Util::PCData data;
				PCIncrRateData pcCal;
				initializePCIncrRateData(pcCal);
				int  base_interface[2]={-1};
				if(fileExists == true)
				{
					if(	FIXS_CCH_PCDataStorage::getInstance()->readBoardCounters(imbVector[i].mag,imbVector[i].slot,data,base_interface,isSMX))
					{
						retcode = EXECUTED;
					}
					else
					{
						if (_trace.ACS_TRA_ON())
						{ //trace
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] readBoardCounters() failed", __FUNCTION__, __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						std::cout << "\nBaseline not available for all rates/increment of that board" << std::endl;
						continue;

					}
					if(retcode == EXECUTED)
					{
						isTransportSwitch=false;
						retcode = calculateDefaultIncrRateValues(shelfManager_C,data,pcCal,currTimeSec,intrfc_cnt[0],base_interface[0],isTransportSwitch);//,base_interface);

						if(retcode == INTERNALERR)
						{
							//std::cout << "\nError when executing" << std::endl;
							return retcode;
						} else {
							retcode = printDefaultResult(shelfManager_C, pcCal, intrfc_cnt[0],option,isTransportSwitch);

							if(retcode == INTERNALERR)
							{
								//std::cout << "\nError when executing" << std::endl;
								return retcode;
							}
						}
						if(isSMX){
							isTransportSwitch=true;
							retcode = calculateDefaultIncrRateValues(shelfManager_T,data,pcCal,currTimeSec,intrfc_cnt[1],base_interface[1],isTransportSwitch);//base_interface);
							if(retcode == INTERNALERR)
							{
								//std::cout << "\nError when executing" << std::endl;
								return retcode;
							} else {
								retcode = printDefaultResult(shelfManager_T, pcCal, intrfc_cnt[1],option,isTransportSwitch);

								if(retcode == INTERNALERR)
								{
									//std::cout << "\nError when executing" << std::endl;
									return retcode;
								}
							}
						}
					}

				}else{
					isTransportSwitch=false;
					retcode = printDefaultResult(shelfManager_C, pcCal, intrfc_cnt[0],option,isTransportSwitch);
					if(retcode == INTERNALERR)
					{
						//std::cout << "\nError when executing" << std::endl;
						return retcode;
					}

					if(isSMX){
						isTransportSwitch=true;
						retcode = printDefaultResult(shelfManager_T, pcCal, intrfc_cnt[1],option,isTransportSwitch);
						if(retcode == INTERNALERR)
						{
							//std::cout << "\nError when executing" << std::endl;
							return retcode;
						}
					}

				} //end of else (if baseline file doesnt exists)


				if(retcode == NO_INTERFACE_UP)
					statusFailCount++;
				if(retcode == NOT_EXIST_INTERFACE)
					countNotExistInterface++;
			}
		}


		if(SNMPFailCount == size)
			retcode = SNMP_FAILURE;    //if SNMP Failure for all SCXB Board,then return SNMP_FAILURE
		else if(statusFailCount == size)
			retcode = NO_INTERFACE_UP; //all scxb board interfaces are down
		else if(countNotExistInterface == size)
			retcode = NOT_EXIST_INTERFACE;
		else if((SNMPFailCount + countNotExistInterface) == size)
			retcode = NOT_EXIST_INTERFACE;
		else if((SNMPFailCount + statusFailCount) == size)  
			retcode = SNMP_FAILURE; //if all board fails due to either snmp error or interface not UP
		else
			retcode = EXECUTED;  

	}
	return retcode;
}
//---------------------------------------------
// printDefaultInterfaceResult
//		default printout for a specific index
//		note: interfaceNr == ifIndex + 1
//---------------------------------------------
void printDefaultInterfaceResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, std::stringstream &pcTable, int ifIndex, int ifNumber,bool isTransportSwitch)
{
	int Common_var=0;
 	if (m_environment != CMD_Util::SINGLECP_DMX && m_environment != CMD_Util::MULTIPLECP_DMX)
	{
		if(((snmpObj->m_ifAlias[ifIndex]).compare("") == 0) || ((snmpObj->m_ifAlias[ifIndex]).compare(" ") == 0))
		{
			if(isTransportSwitch)
			{
				Common_var=ifNumber;
				pcTable << setiosflags(ios::left) << "INTERFACE_" << ((ifNumber+TRANSPORTX_START_IFACE_VALUE)+1) << " : " << "-" << std::endl;
			}
			else{
				Common_var=ifIndex;
				pcTable << setiosflags(ios::left) << "INTERFACE_" << (ifNumber+1) << " : " << "-" << std::endl;
			}
		}
		else
		{
			if(isTransportSwitch)
			{
				Common_var=ifNumber;
				pcTable << setiosflags(ios::left) << "INTERFACE_" << ((ifNumber+TRANSPORTX_START_IFACE_VALUE)+1) << " : " << snmpObj->m_ifAlias[Common_var] << std::endl;
			}
			else{
				Common_var=ifIndex;
				pcTable << setiosflags(ios::left) << "INTERFACE_" << (ifNumber+1) << " : " << snmpObj->m_ifAlias[Common_var] << std::endl;
			}
		}
		pcTable << setiosflags(ios::left) << setw(2) << "" << "INTERFACE DATA" << std::endl;
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "IF-DATA" << setw(1) << "" << setw(20) << "IF-VALUE" << std::endl;
		if(snmpObj->m_ifMTU[Common_var] == -1)
			pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "Mtu" << setw(1) << "" << setw(20) << "-" << std::endl;
		else
			pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "Mtu" << setw(1) << "" << setw(20) << snmpObj->m_ifMTU[Common_var]<< std::endl;
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "PhyAdd" << setw(1) << "" << setw(20) << snmpObj->m_ifphysaddress[Common_var] << std::endl;
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "LastChange" << setw(1) << "" << setw(20) << snmpObj->m_ifLastChange[Common_var] << std::endl;
		if(snmpObj->m_ifHighSpeed[Common_var] == ULONG_MAX)
			pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HSpeed" << setw(1) << "" << setw(20) << "-" << std::endl;
		else
			pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HSpeed" << setw(1) << "" << setw(20) << snmpObj->m_ifHighSpeed[Common_var] << std::endl;
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "CntDiscTime" << setw(1) << "" << setw(20) << snmpObj->m_ifCounterDiscontinuityTime[Common_var] << std::endl;
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OperStat" << setw(1) << "" << setw(20) << snmpObj->m_ifOperStatus[Common_var] << std::endl;
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "DuplexStat" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsDuplexStatus[Common_var]<< std::endl;
		pcTable << std::endl;
	}
	else//DMX ENVIRONMENT
	{
		 if(((snmpObj->m_ifAlias[ifIndex]).compare("") == 0) || ((snmpObj->m_ifAlias[ifIndex]).compare(" ") == 0))
                        pcTable << setiosflags(ios::left) << "BRIDGE_PORT"  << " : " << "-" << std::endl;
                else
                        pcTable << setiosflags(ios::left) << "BRIDGE_PORT"  << " : " << snmpObj->m_ifAlias[ifIndex] << std::endl;
                pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OperStat" << setw(1) << "" << setw(20) << snmpObj->m_ifOperStatus[ifIndex] << std::endl;
                pcTable << std::endl;
	
	}
	pcTable << setiosflags(ios::left) << setw(2) << "" << "COUNTERS DATA" << std::endl;
	pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "COUNTER" << setw(1) << "" << setw(20) << "CURRENT VAL" << setw(1) << "" << setw(17) << "INCREASE" << setw(1) << "" << setw(17) << "RATE" << std::endl;
	if(snmpObj->m_ifHCInOctets[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInOctets" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInOctets" << setw(1) << "" << setw(20) << snmpObj->m_ifHCInOctets[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][0] << setw(1) << "" << setw(17) << data.rate[Common_var][0] << std::endl;
	if(snmpObj->m_ifHCInPkts[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInPkts" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInPkts" << setw(1) << "" << setw(20) << snmpObj->m_ifHCInPkts[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][2] << setw(1) << "" << setw(17) << data.rate[Common_var][2] << std::endl;
	if(snmpObj->m_ifHCOutOctets[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutOct" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutOct" << setw(1) << "" << setw(20) << snmpObj->m_ifHCOutOctets[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][3] << setw(1) << "" << setw(17) << data.rate[Common_var][3] << std::endl;
	if(snmpObj->m_ifHCOutPkts[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutPkts" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutPkts" << setw(1) << "" << setw(20) << snmpObj->m_ifHCOutPkts[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][1] << setw(1) << "" << setw(17) << data.rate[Common_var][1] << std::endl;
	if(snmpObj->m_ifInErrors[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "InErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "InErr" << setw(1) << "" << setw(20) << snmpObj->m_ifInErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][4] << setw(1) << "" << setw(17) << data.rate[Common_var][4] << std::endl;
	if(snmpObj->m_ifOutErrors[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OutErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OutErr" << setw(1) << "" << setw(20) << snmpObj->m_ifOutErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][5] << setw(1) << "" << setw(17) << data.rate[Common_var][5] << std::endl;
	if(snmpObj->m_ifOutDiscards[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OutDiscard" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OutDiscard" << setw(1) << "" << setw(20) << snmpObj->m_ifOutDiscards[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][6] << setw(1) << "" << setw(17) << data.rate[Common_var][6] << std::endl;
	pcTable << std::endl;	
}

//---------------------------------------------
// printVerboseInterfaceResult
//		default printout for a specific index
//		note: interfaceNr == ifIndex + 1
//---------------------------------------------
void printVerboseInterfaceResult(fixs_cch_cmdshelfmanager *snmpObj, PCIncrRateData &data, std::stringstream &pcTable, int ifIndex, int ifNumber, bool isTransportSwitch)
{
	int Common_var=0;
	if (m_environment != CMD_Util::SINGLECP_DMX && m_environment != CMD_Util::MULTIPLECP_DMX)
      	{ 
	if(((snmpObj->m_ifAlias[ifIndex]).compare("") == 0) || ((snmpObj->m_ifAlias[ifIndex]).compare(" ") == 0))
	{
		if(isTransportSwitch)
		{
			Common_var=ifNumber;
			pcTable << setiosflags(ios::left) << "INTERFACE_" << ((ifNumber+TRANSPORTX_START_IFACE_VALUE)+1) << " : " << "-" << std::endl;
		}else {
			Common_var=ifIndex;
			pcTable << setiosflags(ios::left) << "INTERFACE_" << (ifNumber+1) << " : " << "-" << std::endl;
		}
	}
	else
	{
		if(isTransportSwitch)
		{
			Common_var=ifNumber;
			pcTable << setiosflags(ios::left) << "INTERFACE_" << ((ifNumber+TRANSPORTX_START_IFACE_VALUE)+1) << " : " << snmpObj->m_ifAlias[Common_var] << std::endl;
		}else {
			Common_var=ifIndex;
		pcTable << setiosflags(ios::left) << "INTERFACE_" << (ifNumber+1) << " : " << snmpObj->m_ifAlias[Common_var] << std::endl;
		}
	}
	pcTable << setiosflags(ios::left) << setw(2) << "" << "INTERFACE DATA" << std::endl;
	pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "IF-DATA" << setw(1) << "" << setw(20) << "IF-VALUE" << std::endl;
	if(snmpObj->m_ifMTU[Common_var] == -1)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "Mtu" << setw(1) << "" << setw(20) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "Mtu" << setw(1) << "" << setw(20) << snmpObj->m_ifMTU[Common_var]<< std::endl;
	pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "PhyAdd" << setw(1) << "" << setw(20) << snmpObj->m_ifphysaddress[Common_var] << std::endl;
	pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "LastChange" << setw(1) << "" << setw(20) << snmpObj->m_ifLastChange[Common_var] << std::endl;
	if(snmpObj->m_ifHighSpeed[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HSpeed" << setw(1) << "" << setw(20) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HSpeed" << setw(1) << "" << setw(20) << snmpObj->m_ifHighSpeed[Common_var] << std::endl;
	pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "CntDiscTime" << setw(1) << "" << setw(20) << snmpObj->m_ifCounterDiscontinuityTime[Common_var] << std::endl;
	pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OperStat" << setw(1) << "" << setw(20) << snmpObj->m_ifOperStatus[Common_var] << std::endl;
	pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "DuplexStat" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsDuplexStatus[Common_var]<< std::endl;
	pcTable << std::endl;
	}
	else
	{
	 if(((snmpObj->m_ifAlias[Common_var]).compare("") == 0) || ((snmpObj->m_ifAlias[ifIndex]).compare(" ") == 0))
                pcTable << setiosflags(ios::left) << "BRIDGE_PORT"  << " : " << "-" << std::endl;
         else
                pcTable << setiosflags(ios::left) << "BRIDGE_PORT"  << " : " << snmpObj->m_ifAlias[ifIndex] << std::endl;
	
	 pcTable << setiosflags(ios::left) << "" << setw(11) << "OperStat" << setw(1) << "" << setw(20) << snmpObj->m_ifOperStatus[ifIndex] << std::endl;
	pcTable << std::endl;
	}
	pcTable << setiosflags(ios::left) << setw(2) << "" << "COUNTERS DATA" << std::endl;
	pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "COUNTER" << setw(1) << "" << setw(20) << "CURRENT VAL" << setw(1) << "" << setw(17) << "INCREASE" << setw(1) << "" << setw(17) << "RATE" << std::endl;
	if(snmpObj->m_ifHCInOctets[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInOctets" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInOctets" << setw(1) << "" << setw(20) << snmpObj->m_ifHCInOctets[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][0] << setw(1) << "" << setw(17) << data.rate[Common_var][0] << std::endl;
	if(snmpObj->m_ifHCInUcastPkts[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInUcstPkt" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInUcstPkt" << setw(1) << "" << setw(20) << snmpObj->m_ifHCInUcastPkts[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][1] << setw(1) << "" << setw(17) << data.rate[Common_var][1] << std::endl;
	if(snmpObj->m_ifInDiscards[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "InDiscards" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "InDiscards" << setw(1) << "" << setw(20) << snmpObj->m_ifInDiscards[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][11] << setw(1) << "" << setw(17) << data.rate[Common_var][11] << std::endl;
	if(snmpObj->m_ifInErrors[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "InErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "InErr" << setw(1) << "" << setw(20) << snmpObj->m_ifInErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][8] << setw(1) << "" << setw(17) << data.rate[Common_var][8] << std::endl;

	if (m_environment != CMD_Util::SINGLECP_DMX && m_environment != CMD_Util::MULTIPLECP_DMX)
        {
	if(snmpObj->m_ifInUnknownProtos[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "InUnProtos" << setw(1) << "" << setw(20)<< "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "InUnProtos" << setw(1) << "" << setw(20) << snmpObj->m_ifInUnknownProtos[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][12] << setw(1) << "" << setw(17) << data.rate[Common_var][12] << std::endl;
	}
	if(snmpObj->m_ifHCInMulticastPkts[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInMulPkts" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInMulPkts" << setw(1) << "" << setw(20) << snmpObj->m_ifHCInMulticastPkts[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][2] << setw(1) << "" << setw(17) << data.rate[Common_var][2] << std::endl;
	if(snmpObj->m_ifHCInBroadcastPkts[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInBrdPkts" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInBrdPkts" << setw(1) << "" << setw(20) << snmpObj->m_ifHCInBroadcastPkts[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][3] << setw(1) << "" << setw(17) << data.rate[Common_var][3] << std::endl;
	if(snmpObj->m_ifHCOutOctets[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutOct" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutOct" << setw(1) << "" << setw(20) << snmpObj->m_ifHCOutOctets[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][4] << setw(1) << "" << setw(17) << data.rate[Common_var][4] << std::endl;
	if(snmpObj->m_ifHCOutUcastPkts[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOtUcstPkt" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOtUcstPkt" << setw(1) << "" << setw(20) << snmpObj->m_ifHCOutUcastPkts[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][5] << setw(1) << "" << setw(17) << data.rate[Common_var][5] << std::endl;
	if(snmpObj->m_ifOutDiscards[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OutDiscard" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OutDiscard" << setw(1) << "" << setw(20) << snmpObj->m_ifOutDiscards[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][10] << setw(1) << "" << setw(17) << data.rate[Common_var][10] << std::endl;
	if(snmpObj->m_ifOutErrors[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OutErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "OutErr" << setw(1) << "" << setw(20) << snmpObj->m_ifOutErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][9] << setw(1) << "" << setw(17) << data.rate[Common_var][9] << std::endl;
	if(snmpObj->m_ifHCOutMulticastPkts[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutMulPkt" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutMulPkt" << setw(1) << "" << setw(20) << snmpObj->m_ifHCOutMulticastPkts[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][6] << setw(1) << "" << setw(17) << data.rate[Common_var][6] << std::endl;
	if(snmpObj->m_ifHCOutBroadcastPkts[Common_var] == ULLONG_MAX)
		pcTable  << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutBrdPkt" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutBrdPkt" << setw(1) << "" << setw(20) << snmpObj->m_ifHCOutBroadcastPkts[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][7] << setw(1) << "" << setw(17) << data.rate[Common_var][7] << std::endl;
	if (m_environment != CMD_Util::SINGLECP_DMX && m_environment != CMD_Util::MULTIPLECP_DMX)
        {
	if(snmpObj->m_dot3HCStatsAlignmentErrors[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCAlignErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCAlignErr" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCStatsAlignmentErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][13] << setw(1) << "" << setw(17) << data.rate[Common_var][13] << std::endl;
	if(snmpObj->m_dot3HCStatsFCSErrors[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCFCSErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCFCSErr" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCStatsFCSErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][14] << setw(1) << "" << setw(17) << data.rate[Common_var][14] << std::endl;
	if(snmpObj->m_dot3StatsSingleCollisionFrames[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "SingCollFrm" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "SingCollFrm" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsSingleCollisionFrames[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][15] << setw(1) << "" << setw(17) << data.rate[Common_var][15] << std::endl;
	if(snmpObj->m_dot3StatsMultipleCollisionFrames[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "MultCollFrm" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "MultCollFrm" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsMultipleCollisionFrames[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][16] << setw(1) << "" << setw(17) << data.rate[Common_var][16] <<std::endl;
	if(snmpObj->m_dot3StatsSQETestErrors[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "SQETestErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "SQETestErr" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsSQETestErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][17] << setw(1) << "" << setw(17) << data.rate[Common_var][17] << std::endl;
	if(snmpObj->m_dot3StatsDeferredTransmissions[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "DeferrTrans" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "DeferrTrans" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsDeferredTransmissions[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][18] << setw(1) << "" << setw(17) << data.rate[Common_var][18] << std::endl;
	if(snmpObj->m_dot3StatsLateCollisions[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "LateColl" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "LateColl" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsLateCollisions[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][19] << setw(1) << "" << setw(17) << data.rate[Common_var][19] << std::endl;
	if(snmpObj->m_dot3StatsExcessiveCollisions[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "ExcessColl" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "ExcessColl" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsExcessiveCollisions[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][20] << setw(1) << "" << setw(17) << data.rate[Common_var][20] << std::endl;
	if(snmpObj->m_dot3HCStatsInternalMacTransmitErrors[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "IrMACTrmErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "IrMACTrmErr" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCStatsInternalMacTransmitErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][21] << setw(1) << "" << setw(17) << data.rate[Common_var][21] << std::endl;
	if(snmpObj->m_dot3StatsCarrierSenseErrors[Common_var] == ULONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "CarSenseErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "CarSenseErr" << setw(1) << "" << setw(20) << snmpObj->m_dot3StatsCarrierSenseErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][22] << setw(1) << "" << setw(17) << data.rate[Common_var][22] << std::endl;
	if(snmpObj->m_dot3HCStatsFrameTooLongs[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCFrmTooLng" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCFrmTooLng" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCStatsFrameTooLongs[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][23] << setw(1) << "" << setw(17) << data.rate[Common_var][23] << std::endl;
	if(snmpObj->m_dot3HCStatsInternalMacReceiveErrors[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCIrMACRcEr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCIrMACRcEr" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCStatsInternalMacReceiveErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][24] << setw(1) << "" << setw(17) << data.rate[Common_var][24] << std::endl;
	if(snmpObj->m_dot3HCStatsSymbolErrors[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCSymbErr" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11)<< "HCSymbErr" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCStatsSymbolErrors[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][25] << setw(1) << "" << setw(17) << data.rate[Common_var][25] << std::endl;
	if(snmpObj->m_dot3HCControlInUnknownOpcodes[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCCrInUnOpc" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCCrInUnOpc" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCControlInUnknownOpcodes[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][26] << setw(1) << "" << setw(17) << data.rate[Common_var][26] << std::endl;
	if(snmpObj->m_dot3HCInPauseFrames[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInPseFrm" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCInPseFrm" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCInPauseFrames[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][27] << setw(1) << "" << setw(17) << data.rate[Common_var][27] << std::endl;
	if(snmpObj->m_dot3HCOutPauseFrames[Common_var] == ULLONG_MAX)
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutPseFrm" << setw(1) << "" << setw(20) << "-" << setw(1) << "" << setw(17) << "-" << setw(1) << "" << setw(17) << "-" << std::endl;
	else
		pcTable << setiosflags(ios::left) << setw(4) << "" << setw(11) << "HCOutPseFrm" << setw(1) << "" << setw(20) << snmpObj->m_dot3HCOutPauseFrames[Common_var] << setw(1) << "" << setw(17) << data.increment[Common_var][28] << setw(1) << "" << setw(17) << data.rate[Common_var][28] << std::endl;
	}
	pcTable << std::endl;
}
//---------------------------------------------
// fillPrintOutDataForBoard
//		fill printout data for each SCXB and CMXB board in to output stream
//---------------------------------------------
int fillPrintOutDataForBoard(std::stringstream &pcTable, XCOUNTLS_Util::PCData data, int no_of_intrface,bool isTransport)
{
	int i,j;
	bool interfaceFound = false;

	if(!isTransport) //To restrict printing of KEY_TIME twice.
		pcTable << setiosflags(ios::left) << "KEY_TIME" << " : " << data.timeValue<<std::endl<<std::endl;

	int order[no_of_intrface] ;
	for (j=0; j <=(no_of_intrface - 1); j++)
	{
		if(isTransport)
		{
			int k= (j + TRANSPORTX_START_IFACE_VALUE);
			order[j] = k;
		}
		else
			order[j] = j;
	}

	if (m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX)
	{
		if(no_of_intrface == DMX_INTERFACES)
		{
			for(i=0; i<= DMX_INTERFACES - 1; i++)
			{
				for(j=0; j<= DMX_INTERFACES - 1; j++) {
					if(strcmp(data.interfaceName[i].c_str(), DmxInterfaceNames[j]) == 0) {
						order[j] = i;
						interfaceFound = true;
						break;
					}
				}
				if (interfaceFound == false)
					return INTERNALERR;
				else
					interfaceFound = false;
			}
		}
	}

	for(i=0;i< no_of_intrface;i++)
	{
		if (m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX)
        	{
		   pcTable << setiosflags(ios::left) << "BRIDGE_PORT" <<  " : " << data.interfaceName[order[i]]<< std::endl<<std::endl;
		   pcTable << setiosflags(ios::left) << "OperStat" << " : " << data.counterValue[order[i]][5]<<std::endl;
		}
		else
		{

			int temp_iface =(isTransport ? (order[i]+1) : (i+1));

			pcTable << setiosflags(ios::left) << "INTERFACE_" << temp_iface << " : " << data.interfaceName[order[i]]<< std::endl<<std::endl;
			pcTable << setiosflags(ios::left) << "Mtu" << " : " << data.counterValue[order[i]][0]<<std::endl;
			pcTable << setiosflags(ios::left) << "LastChange" << " : " << data.counterValue[order[i]][1]<<std::endl;
			pcTable << setiosflags(ios::left) << "PhyAdd" << " : " << data.counterValue[order[i]][2]<<std::endl;
			pcTable << setiosflags(ios::left) << "HSpeed" << " : " << data.counterValue[order[i]][3]<<std::endl;
			pcTable << setiosflags(ios::left) << "CntDiscTime" << " : " << data.counterValue[order[i]][4]<<std::endl;
			pcTable << setiosflags(ios::left) << "OperStat" << " : " << data.counterValue[order[i]][5]<<std::endl;
			pcTable << setiosflags(ios::left) << "DuplexStat" << " : " << data.counterValue[order[i]][6]<<std::endl;
		}
		

		pcTable << setiosflags(ios::left) << "HCInOctets" << " : " << data.counterValue[order[i]][7]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCInUcstPkt" << " : " << data.counterValue[order[i]][8]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCInMulPkts" << " : " << data.counterValue[order[i]][9]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCInBrdPkts" << " : " << data.counterValue[order[i]][10]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCInPkts" << " : " << data.counterValue[order[i]][11]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCOutOct" << " : " << data.counterValue[order[i]][12]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCOutUcstPkt" << " : " << data.counterValue[order[i]][13]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCOutMulPkt" << " : " << data.counterValue[order[i]][14]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCOutBrdPkt" << " : " << data.counterValue[order[i]][15]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCOutPkts" << " : " << data.counterValue[order[i]][16]<<std::endl;
		pcTable << setiosflags(ios::left) << "InErr" << " : " << data.counterValue[order[i]][17]<<std::endl;
		pcTable << setiosflags(ios::left) << "OutErr" << " : " << data.counterValue[order[i]][18]<<std::endl;
		pcTable << setiosflags(ios::left) << "OutDiscard" << " : " << data.counterValue[order[i]][19]<<std::endl;
		pcTable << setiosflags(ios::left) << "InDiscards" << " : " << data.counterValue[order[i]][20]<<std::endl;


		if (m_environment != CMD_Util::SINGLECP_DMX &&  m_environment != CMD_Util::MULTIPLECP_DMX)
                {
		pcTable << setiosflags(ios::left) << "InUnProtos" << " : " << data.counterValue[order[i]][21]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCAlignErr" << " : " << data.counterValue[order[i]][22]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCFCSErr" << " : " << data.counterValue[order[i]][23]<<std::endl;
		pcTable << setiosflags(ios::left) << "SingCollFrm" << " : " << data.counterValue[order[i]][24]<<std::endl;
		pcTable << setiosflags(ios::left) << "MultCollFrm" << " : " << data.counterValue[order[i]][25]<<std::endl;
		pcTable << setiosflags(ios::left) << "SQETestErr" << " : " << data.counterValue[order[i]][26]<<std::endl;
		pcTable << setiosflags(ios::left) << "DeferrTrans" << " : " << data.counterValue[order[i]][27]<<std::endl;
		pcTable << setiosflags(ios::left) << "LateColl" << " : " << data.counterValue[order[i]][28]<<std::endl;
		pcTable << setiosflags(ios::left) << "ExcessColl" << " : " << data.counterValue[order[i]][29]<<std::endl;
		pcTable << setiosflags(ios::left) << "IrMACTrmErr" << " : " << data.counterValue[order[i]][30]<<std::endl;
		pcTable << setiosflags(ios::left) << "CarSenseErr" << " : " << data.counterValue[order[i]][31]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCFrmTooLng" << " : " << data.counterValue[order[i]][32]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCIrMACRcEr" << " : " << data.counterValue[order[i]][33]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCSymbErr" << " : " << data.counterValue[order[i]][34]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCCrInUnOpc" << " : " << data.counterValue[order[i]][35]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCInPseFrm" << " : " << data.counterValue[order[i]][36]<<std::endl;
		pcTable << setiosflags(ios::left) << "HCOutPseFrm" << " : " << data.counterValue[order[i]][37]<<std::endl;
		}

		pcTable <<std::endl;
	}	
	return EXECUTED;
}
//-------------------------------------
// pcBaseline
//	Performance Counters Baseline Data
//-------------------------------------
int pcBaseline()
{
	TraceInOut inOut(__FUNCTION__);
	std::vector<imbInfo> imbVector;
	int retcode = EXECUTED;
	int retcodeCMX = EXECUTED;
	fixs_cch_cmdshelfmanager ironsideManager("");
	
	//IN DMX Environment SCX is not defined in HWC Table. Skip this check!
	if (m_environment != CMD_Util::SINGLECP_DMX && m_environment != CMD_Util::MULTIPLECP_DMX)
	{
		hwc = ACS_CS_API::createHWCInstance();
		if (hwc)
		{
			boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
			if (boardSearch)
			{
				retcode = allEGEM2L2SwitchBoardInfo(imbVector,isSMX);
				if(retcode == NO_REPLY_FROM_CS)
				{
					if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
					if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
					return retcode;
				}

				retcodeCMX = allCMXBoardInfo(imbVector);
				if(retcodeCMX == NO_REPLY_FROM_CS)
				{
					if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
					if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
					return retcodeCMX;
				}

			}
			else
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance() failed", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retcode = INTERNALERR;//Memory error
				if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
				return retcode;
			}
		}
		else
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createHWCInstance() failed", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = INTERNALERR;//Memory error
			return retcode;
		}

		if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		if (hwc) ACS_CS_API::deleteHWCInstance(hwc);

	}
	else //DMX ENVIRONMENT
	{
		std::vector<std::string> magAddrs;
		retcode = ironsideManager.getShelfListUDP(magAddrs);
		if(retcode != XCOUNTLS_Util::EXECUTED)
			return retcode;

		for (std::vector<std::string>::iterator it = magAddrs.begin(); it != magAddrs.end(); it++)
		{
			
			imbInfo scx_0;
			scx_0.IP_EthA = "";
			scx_0.IP_EthB = "";
			XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), scx_0.mag);
			scx_0.slot = 0;
			imbVector.push_back(scx_0);
			imbInfo scx_25;
			scx_25.IP_EthA = "";
			scx_25.IP_EthB = "";
			XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), scx_25.mag);
			scx_25.slot = 25;
			imbVector.push_back(scx_25);
			imbInfo cmx_26;
			cmx_26.IP_EthA = "";
			cmx_26.IP_EthB = "";
			XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), cmx_26.mag);
			cmx_26.slot = 26;
			imbVector.push_back(cmx_26);
			imbInfo cmx_28;
			cmx_28.IP_EthA = "";
			cmx_28.IP_EthB = "";
			XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), cmx_28.mag);
			cmx_28.slot = 28;
			imbVector.push_back(cmx_28);

		}
	}

	if((retcode == EXECUTED) || (retcodeCMX == EXECUTED))
	{
		int size = (int)imbVector.size();
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Size of imbVector is :" << size<< std::endl;
		if(size !=0)
		{
			sortVector(imbVector); //sort the list of SCXB and CMXB boards
		}

		if((retcode = FIXS_CCH_PCDataStorage::getInstance()->saveBaselineHeading()) != EXECUTED)
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] saveBaselineHeading() failed", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Failed in saveBaselineHeading()"<< std::endl;
			return retcode;
		}

		//FIXS_CCH_Command_SNMPManager *snmpManager = 0;
		if(size !=0)
		{
			int failureCount = 0;
			for(int i=0; i <= (size-1); i++)
			{
				fixs_cch_cmdshelfmanager snmpManager("");
				fixs_cch_cmdshelfmanager* shelfManager = &snmpManager;
				int no_of_intrface = -1;
				bool snmpflag = true;
				std::string magStr,slotStr;
				XCOUNTLS_Util::ulongToStringMagazine(imbVector[i].mag, magStr);
				XCOUNTLS_Util::ushortToString(imbVector[i].slot,slotStr);
				int l3SwitchVal = 0;
				if((imbVector[i].slot == 26 ) || (imbVector[i].slot == 28 ))
				{
					l3SwitchVal = 1; 
				}

				if ((m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX) )
				{
					snmpflag = readIronsideVerboseInformation(&ironsideManager,magStr,slotStr, no_of_intrface);
					shelfManager = &ironsideManager;
				}
				else
				{
					snmpflag = readSnmpVerboseInformation(&snmpManager, imbVector[i].IP_EthA, imbVector[i].IP_EthB, no_of_intrface,l3SwitchVal);
				}

				//std::cout << std::endl << "MAG   " << magStr << "   SLOT " << imbVector[i].slot;
				if(snmpflag == false)
				{
					if (_trace.ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] readVerboseInformation() failed", __FUNCTION__, __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					//std::cout<<"\n\tSNMP ERROR:NO AVAILABLE DATA"<<endl;
					failureCount++; 
					continue;
				}
				else
				{
					XCOUNTLS_Util::PCData data;

					retcode = fetchBaselineData(shelfManager,no_of_intrface,data);

					if( retcode == EXECUTED)
					{
						bool isSMXFlag=false;
						FIXS_CCH_PCDataStorage::getInstance()->saveBoardCounters(imbVector[i].mag,imbVector[i].slot,data,no_of_intrface,isSMXFlag);
						//Returned value always true

						//					if((retcode = FIXS_CCH_PCDataStorage::getInstance()->saveBoardCounters(scxVector[i].mag,scxVector[i].slot,data)) != EXECUTED)
						//					{
						//
						//						if (_trace.ACS_TRA_ON())
						//						{ //trace
						//							char tmpStr[512] = {0};
						//							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] saveBoardCounters() failed", __FUNCTION__, __LINE__);
						//							_trace.ACS_TRA_event(1, tmpStr);
						//						}
						//						return retcode;
						//					}
					}
					else
					{
						if (_trace.ACS_TRA_ON())
						{ //trace
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] fetchBaselineData() failed", __FUNCTION__, __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return retcode;
					}
				}

				//@
				if(isSMX)
				{
					//bool isSMXFlag=true;
					l3SwitchVal = 2;
					snmpflag = readSnmpVerboseInformation(&snmpManager, imbVector[i].AliasIP_EthA, imbVector[i].AliasIP_EthB, no_of_intrface,l3SwitchVal);
					if(snmpflag){
						XCOUNTLS_Util::PCData data;

						retcode = fetchBaselineData(shelfManager,no_of_intrface,data);

						if( retcode == EXECUTED)
						{
							bool isSMXFlag=true;
							FIXS_CCH_PCDataStorage::getInstance()->saveBoardCounters(imbVector[i].mag,imbVector[i].slot,data,no_of_intrface,isSMXFlag);
						}
					}
				}

			} // end for

			if(failureCount == size)
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] SNMP failure for SCX boards", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				if ((m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX) )
                               		std::cout<< "\nCOMMUNICATION ERROR:NO AVAILABLE DATA" << std::endl;
                        	else
					std::cout<<"SNMP ERROR:NO AVAILABLE DATA"<<endl;
				retcode = SNMP_FAILURE; //if SNMP Failure for all SCXB Board,then return SNMP_FAILURE
			}
			else
			{
				FIXS_CCH_PCDataStorage::getInstance()->saveBaseline();
				FIXS_CCH_PCDataStorage::getInstance()->commitChanges();
				retcode = EXECUTED;  
			}


		}
		return retcode;
	}			
	else
	{
		return HARDWARE_NOT_PRESENT;
	}
}

void initializePCIncrRateData(PCIncrRateData &pcCal)
{
	for(int i=0;i<MAX_NO_OF_INTERFACE;i++)
	{
		for(int j=0;j<MAX_NO_OF_COUNTERS;j++)
		{
			pcCal.increment[i][j] = "-";
			pcCal.rate[i][j] = "-";

		}
		pcCal.deprecated[i] = "-";
	}
}


int calculateVerboseIncrRateValues(fixs_cch_cmdshelfmanager *snmpObj,XCOUNTLS_Util::PCData &data,PCIncrRateData &pcCal,ULONGLONG time, int no_of_interface, int base_interface,bool isTransportSwitch)
{
	int retcode = EXECUTED;
	int k;
	//for (int i=1;i<=base_interface;i++)
	for (int i=0;i< base_interface;i++)
	{
		if(isTransportSwitch)
			k=(i+TRANSPORTX_START_IFACE_VALUE);
		else
			k=i;
		ULONGLONG tempIfHCInOctets =0;
		ULONGLONG tempIncr = 0;
		ULONGLONG tempIncr1 = 0;
		double tempRate =0.0, tempRate1 =0.0;
		ULONGLONG tempBaseTime =0;
		int j;
		bool found = false;
		for( j=0;j<no_of_interface;j++)
                {
                        if (strcmp(snmpObj->m_ifAlias[j].c_str(),  data.interfaceName[k].c_str()) == 0) {
	                                found = true;
                                break;
                        }

                }
                if(!found)
                        continue;	
		if (data.counterValue[k][7] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][7],tempIfHCInOctets))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}

			if (!XCOUNTLS_Util::stringToulonglong(data.timeValue,tempBaseTime))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempIfHCInOctets > snmpObj->m_ifHCInOctets[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr = (snmpObj->m_ifHCInOctets[i] - tempIfHCInOctets)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][0])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if(!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][0])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempIfHCInUcastPkts = 0;

		if (data.counterValue[k][8] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][8],tempIfHCInUcastPkts))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempIfHCInUcastPkts > snmpObj->m_ifHCInUcastPkts[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr = (snmpObj->m_ifHCInUcastPkts[i] - tempIfHCInUcastPkts)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][1])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][1])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempIfHCInMulticastPkts = 0;

		if (data.counterValue[k][9] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][9],tempIfHCInMulticastPkts))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
	
			if(tempIfHCInMulticastPkts > snmpObj->m_ifHCInMulticastPkts[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr = (snmpObj->m_ifHCInMulticastPkts[i] - tempIfHCInMulticastPkts)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][2])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][2])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempIfHCInBroadcastPkts = 0;

		if (data.counterValue[k][10] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][10],tempIfHCInBroadcastPkts))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempIfHCInBroadcastPkts > snmpObj->m_ifHCInBroadcastPkts[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr = (snmpObj->m_ifHCInBroadcastPkts[i] - tempIfHCInBroadcastPkts)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][3])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][3])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempIfHCOutOctets = 0;

		if (data.counterValue[k][12] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][12],tempIfHCOutOctets))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}

			if(tempIfHCOutOctets > snmpObj->m_ifHCOutOctets[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{	
		
				tempIncr = (snmpObj->m_ifHCOutOctets[i] - tempIfHCOutOctets)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][4])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][4])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToStringRate() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempIfHCOutUcastPkts = 0;

		if (data.counterValue[k][13] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][13],tempIfHCOutUcastPkts))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}

			if(tempIfHCOutUcastPkts > snmpObj->m_ifHCOutUcastPkts[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr = (snmpObj->m_ifHCOutUcastPkts[i] - tempIfHCOutUcastPkts)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][5])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][5])) // double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempIfHCOutMulticastPkts = 0;

		if (data.counterValue[k][14] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][14],tempIfHCOutMulticastPkts))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}

			if(tempIfHCOutMulticastPkts > snmpObj->m_ifHCOutMulticastPkts[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr = (snmpObj->m_ifHCOutMulticastPkts[i] - tempIfHCOutMulticastPkts)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][6])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][6])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempIfHCOutBroadcastPkts = 0;

		if (data.counterValue[k][15] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][15],tempIfHCOutBroadcastPkts))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempIfHCOutBroadcastPkts > snmpObj->m_ifHCOutBroadcastPkts[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr = (snmpObj->m_ifHCOutBroadcastPkts[i] - tempIfHCOutBroadcastPkts)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][7])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][7])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 =0;
		unsigned long tempIfInErrors = 0;

		if (data.counterValue[k][17] != "-")
		{
			tempIfInErrors = strtoul((data.counterValue[k][17]).c_str(),NULL,10);
			if(tempIfInErrors > snmpObj->m_ifInErrors[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_ifInErrors[i] - tempIfInErrors)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][8])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][8])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempIfOutErrors = 0;

		if (data.counterValue[k][18] != "-")
		{
			tempIfOutErrors = strtoul((data.counterValue[k][18]).c_str(),NULL,10);
			if(tempIfOutErrors > snmpObj->m_ifOutErrors[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_ifOutErrors[i] - tempIfOutErrors)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][9])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][9])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempIfOutDiscards = 0;

		if (data.counterValue[k][19] != "-")
		{
			tempIfOutDiscards = strtoul((data.counterValue[k][19]).c_str(),NULL,10);
			if(tempIfOutDiscards > snmpObj->m_ifOutDiscards[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_ifOutDiscards[i] - tempIfOutDiscards)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][10])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][10])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempIfInDiscards = 0;

		if (data.counterValue[k][20] != "-")
		{
			tempIfInDiscards = strtoul((data.counterValue[k][20]).c_str(),NULL,10);
			if(tempIfInDiscards > snmpObj->m_ifInDiscards[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_ifInDiscards[i] - tempIfInDiscards)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][11])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][11])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempIfInUnknownProtos = 0;

		if (data.counterValue[k][21] != "-")
		{
			tempIfInUnknownProtos = strtoul((data.counterValue[k][21]).c_str(),NULL,10);
			if(tempIfInUnknownProtos > snmpObj->m_ifInUnknownProtos[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_ifInUnknownProtos[i] - tempIfInUnknownProtos)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][12])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][12])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCStatsAlignmentErrors = 0;

		if (data.counterValue[k][22] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][22],tempDot3HCStatsAlignmentErrors))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringToulonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempDot3HCStatsAlignmentErrors > snmpObj->m_dot3HCStatsAlignmentErrors[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr = (snmpObj->m_dot3HCStatsAlignmentErrors[i] - tempDot3HCStatsAlignmentErrors)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][13])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][13])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCStatsFCSErrors = 0;

		if (data.counterValue[k][23] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][23],tempDot3HCStatsFCSErrors))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringToulonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}

			if(tempDot3HCStatsFCSErrors > snmpObj->m_dot3HCStatsFCSErrors[j])
			{
				pcCal.deprecated[j] = "deprecated";
			}
			else
			{
				tempIncr = (snmpObj->m_dot3HCStatsFCSErrors[i] - tempDot3HCStatsFCSErrors)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][14])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][14])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempDot3StatsSingleCollisionFrames = 0;

		if (data.counterValue[k][24] != "-")
		{
			tempDot3StatsSingleCollisionFrames = strtoul((data.counterValue[k][24]).c_str(),NULL,10);
			if(tempDot3StatsSingleCollisionFrames > snmpObj->m_dot3StatsSingleCollisionFrames[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr1 = (ULONGLONG)(snmpObj->m_dot3StatsSingleCollisionFrames[i] - tempDot3StatsSingleCollisionFrames)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][15])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][15])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempDot3StatsMultipleCollisionFrames = 0;

		if (data.counterValue[k][25] != "-")
		{
			tempDot3StatsMultipleCollisionFrames = strtoul((data.counterValue[k][25]).c_str(),NULL,10);
			if(tempDot3StatsMultipleCollisionFrames > snmpObj->m_dot3StatsMultipleCollisionFrames[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr1 = (ULONGLONG)(snmpObj->m_dot3StatsMultipleCollisionFrames[i] - tempDot3StatsMultipleCollisionFrames)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][16])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][16])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempDot3StatsSQETestErrors = 0;

		if (data.counterValue[k][26] != "-")
		{
			tempDot3StatsSQETestErrors = strtoul((data.counterValue[k][26]).c_str(),NULL,10);
			if(tempDot3StatsSQETestErrors > snmpObj->m_dot3StatsSQETestErrors[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_dot3StatsSQETestErrors[i] - tempDot3StatsSQETestErrors)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][17])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][17])) // double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempDot3StatsDeferredTransmissions = 0;

		if (data.counterValue[k][27] != "-")
		{
			tempDot3StatsDeferredTransmissions = strtoul((data.counterValue[k][27]).c_str(),NULL,10);
			if(tempDot3StatsDeferredTransmissions > snmpObj->m_dot3StatsDeferredTransmissions[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
			
				tempIncr1 = (ULONGLONG)(snmpObj->m_dot3StatsDeferredTransmissions[i] - tempDot3StatsDeferredTransmissions)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][18])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][18])) // double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempDot3StatsLateCollisions = 0;

		if (data.counterValue[k][28] != "-")
		{
			tempDot3StatsLateCollisions = strtoul((data.counterValue[k][28]).c_str(),NULL,10);
			if(tempDot3StatsLateCollisions > snmpObj->m_dot3StatsLateCollisions[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_dot3StatsLateCollisions[i] - tempDot3StatsLateCollisions)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][19])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][19])) // double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempDot3StatsExcessiveCollisions = 0;

		if (data.counterValue[k][29] != "-")
		{
			tempDot3StatsExcessiveCollisions = strtoul((data.counterValue[k][29]).c_str(),NULL,10);
			if(tempDot3StatsExcessiveCollisions > snmpObj->m_dot3StatsExcessiveCollisions[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_dot3StatsExcessiveCollisions[i] - tempDot3StatsExcessiveCollisions)/1024;
				if(time != tempBaseTime)
					tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][20])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][20])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCStatsInternalMacTransmitErrors = 0;

		if (data.counterValue[k][30] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][30],tempDot3HCStatsInternalMacTransmitErrors))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempDot3HCStatsInternalMacTransmitErrors > snmpObj->m_dot3HCStatsInternalMacTransmitErrors[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{
				tempIncr = (snmpObj->m_dot3HCStatsInternalMacTransmitErrors[i] - tempDot3HCStatsInternalMacTransmitErrors)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][21])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][21])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr1 = 0;
		tempRate1 = 0.0;
		unsigned long tempDot3StatsCarrierSenseErrors = 0;

		if (data.counterValue[k][31] != "-")
		{
			tempDot3StatsCarrierSenseErrors = strtoul((data.counterValue[k][31]).c_str(),NULL,10);
			if(tempDot3StatsCarrierSenseErrors > snmpObj->m_dot3StatsCarrierSenseErrors[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr1 = (ULONGLONG)(snmpObj->m_dot3StatsCarrierSenseErrors[i] - tempDot3StatsCarrierSenseErrors)/1024;
				if(time != tempBaseTime)
				tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][22])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][22])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCStatsFrameTooLongs = 0;

		if (data.counterValue[k][32] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][32],tempDot3HCStatsFrameTooLongs))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempDot3HCStatsFrameTooLongs > snmpObj->m_dot3HCStatsFrameTooLongs[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr = (snmpObj->m_dot3HCStatsFrameTooLongs[i] - tempDot3HCStatsFrameTooLongs)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][23])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][23])) // double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCStatsInternalMacReceiveErrors = 0;

		if (data.counterValue[k][33] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][33],tempDot3HCStatsInternalMacReceiveErrors))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempDot3HCStatsInternalMacReceiveErrors > snmpObj->m_dot3HCStatsInternalMacReceiveErrors[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr = (snmpObj->m_dot3HCStatsInternalMacReceiveErrors[i] - tempDot3HCStatsInternalMacReceiveErrors)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][24])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][24])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCStatsSymbolErrors = 0;

		if (data.counterValue[k][34] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][34],tempDot3HCStatsSymbolErrors))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			if(tempDot3HCStatsSymbolErrors > snmpObj->m_dot3HCStatsSymbolErrors[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr = (snmpObj->m_dot3HCStatsSymbolErrors[i] - tempDot3HCStatsSymbolErrors)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][25])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][25])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCControlInUnknownOpcodes = 0;

		if (data.counterValue[k][35] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][35],tempDot3HCControlInUnknownOpcodes))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringToulonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}

			if(tempDot3HCControlInUnknownOpcodes > snmpObj->m_dot3HCControlInUnknownOpcodes[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr = (snmpObj->m_dot3HCControlInUnknownOpcodes[i] - tempDot3HCControlInUnknownOpcodes)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][26])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][26])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCInPauseFrames = 0;

		if (data.counterValue[k][36] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][36],tempDot3HCInPauseFrames))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringToulonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempDot3HCInPauseFrames > snmpObj->m_dot3HCInPauseFrames[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr = (snmpObj->m_dot3HCInPauseFrames[i] - tempDot3HCInPauseFrames)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][27])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][27])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

		tempIncr = 0;
		tempRate = 0.0;
		ULONGLONG tempDot3HCOutPauseFrames = 0;

		if (data.counterValue[k][37] != "-")
		{
			if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][37],tempDot3HCOutPauseFrames))
			{
				if (_trace.ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringToulonglong() method failed", __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				return INTERNALERR;
			}
			
			if(tempDot3HCOutPauseFrames > snmpObj->m_dot3HCOutPauseFrames[i])
			{
				pcCal.deprecated[i] = "deprecated";
			}
			else
			{

				tempIncr = (snmpObj->m_dot3HCOutPauseFrames[i] - tempDot3HCOutPauseFrames)/1024;
				if(time != tempBaseTime)
					tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

				if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][28])) //longlong
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ulonglongToString() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(time != tempBaseTime)
				{
					if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][28])) //double
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] doubleToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}
				}
			}
		}

	}

	return retcode;
}

int calculateDefaultIncrRateValues(fixs_cch_cmdshelfmanager *snmpObj,XCOUNTLS_Util::PCData &data,PCIncrRateData &pcCal,ULONGLONG time, int no_of_interface,int base_interface,bool isTransportSwitch)
{
	int retcode = EXECUTED;
	int k;

		for (int i=0;i< base_interface;i++)
		{
			if(isTransportSwitch)
				k=(i+TRANSPORTX_START_IFACE_VALUE);
			else
				k=i;

			ULONGLONG tempIfHCInOctets =0;
			ULONGLONG tempIncr = 0;
			double tempRate =0.0, tempRate1 =0.0;
			ULONGLONG tempIncr1 = 0 ;
			ULONGLONG tempBaseTime =0;
			bool found = false;
			int j;
			for(j=0;j<no_of_interface;j++)
			{
				if (strcmp(snmpObj->m_ifAlias[j].c_str(),  data.interfaceName[k].c_str()) == 0) {
					found = true;
					break;
				}
			}
			if(!found)
				continue;

			if( data.counterValue[k][7] != "-")
			{

				if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][7],tempIfHCInOctets))
				{

					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}
				if (!XCOUNTLS_Util::stringToulonglong(data.timeValue,tempBaseTime))
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(tempIfHCInOctets > snmpObj->m_ifHCInOctets[i])
				{
					pcCal.deprecated[j] = "deprecated";
				}
				else
				{
					tempIncr = (snmpObj->m_ifHCInOctets[i] - tempIfHCInOctets)/1024;
					if(time != tempBaseTime)
						tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

					if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][0])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}

					if(time != tempBaseTime)
					{
						if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][0])) //double
						{
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							return INTERNALERR;
						}
					}
				}

			}

			tempIncr = 0;
			tempRate = 0.0;
			ULONGLONG tempIfHCOutPkts = 0;

			if(data.counterValue[k][16] != "-")
			{
				if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][16],tempIfHCOutPkts))
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}
				if(tempIfHCOutPkts > snmpObj->m_ifHCOutPkts[i])
				{
					pcCal.deprecated[i] = "deprecated";
				}
				else
				{
					tempIncr = (snmpObj->m_ifHCOutPkts[i] - tempIfHCOutPkts)/1024;
					if(time != tempBaseTime)
						tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

					if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][1])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}

					if(time != tempBaseTime)
					{
						if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][1])) //double
						{
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							return INTERNALERR;
						}
					}
				}
			}

			tempIncr = 0;
			tempRate = 0.0;
			ULONGLONG tempIfHCInPkts = 0;

			if(data.counterValue[k][11] != "-")
			{
				if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][11],tempIfHCInPkts))
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(tempIfHCInPkts > snmpObj->m_ifHCInPkts[i])
				{
					pcCal.deprecated[i] = "deprecated";
				}
				else
				{
					tempIncr = (snmpObj->m_ifHCInPkts[i] - tempIfHCInPkts)/1024;
					if(time != tempBaseTime)
						tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

					if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][2])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}

					if(time != tempBaseTime)
					{
						if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][2])) //double
						{
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							return INTERNALERR;
						}
					}
				}

			}

			tempIncr = 0;
			tempRate = 0.0;
			ULONGLONG tempIfHCOutOctets = 0;

			if(data.counterValue[k][12] != "-")
			{
				if (!XCOUNTLS_Util::stringToulonglong(data.counterValue[k][12],tempIfHCOutOctets))
				{
					if (_trace.ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] stringTolonglong() method failed", __LINE__);
						_trace.ACS_TRA_event(1, tmpStr);
					}
					return INTERNALERR;
				}

				if(tempIfHCOutOctets > snmpObj->m_ifHCOutOctets[i])
				{
					pcCal.deprecated[i] = "deprecated";
				}
				else
				{
					tempIncr = (snmpObj->m_ifHCOutOctets[i] - tempIfHCOutOctets )/1024; //@
					if(time != tempBaseTime)
						tempRate = ((double)tempIncr /(double) (time - tempBaseTime));

					if (!XCOUNTLS_Util::ulonglongToString(tempIncr, pcCal.increment[i][3])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}

					if(time != tempBaseTime)
					{
						if (!XCOUNTLS_Util::doubleToStringRate(tempRate, pcCal.rate[i][3])) //double
						{
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							return INTERNALERR;
						}
					}
				}

			}

			tempIncr1 = 0;
			tempRate1 = 0.0;
			unsigned long tempIfInErrors = 0;

			if(data.counterValue[k][17] != "-")
			{
				tempIfInErrors = strtoul((data.counterValue[k][17]).c_str(),NULL,10);
				if(tempIfInErrors > snmpObj->m_ifInErrors[i])
				{
					pcCal.deprecated[i] = "deprecated";
				}
				else
				{
					tempIncr1 = (ULONGLONG)(snmpObj->m_ifInErrors[i] - tempIfInErrors)/1024;
					if(time != tempBaseTime)
						tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

					if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][4])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}

					if(time != tempBaseTime)
					{
						if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][4])) //double
						{
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							return INTERNALERR;
						}
					}
				}

			}

			tempIncr1 = 0;
			tempRate1 = 0.0;
			unsigned long tempIfOutErrors = 0;

			if(data.counterValue[k][18] != "-")
			{
				tempIfOutErrors = strtoul((data.counterValue[k][18]).c_str(),NULL,10);
				if(tempIfOutErrors > snmpObj->m_ifOutErrors[i])
				{
					pcCal.deprecated[i] = "deprecated";
				}
				else
				{

					tempIncr1 = (ULONGLONG)(snmpObj->m_ifOutErrors[i] - tempIfOutErrors)/1024;
					if(time != tempBaseTime)
						tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

					if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][5])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}

					if(time != tempBaseTime)
					{
						if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][5])) //double
						{
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							return INTERNALERR;
						}
					}
				}

			}

			tempIncr1 = 0;
			tempRate1 = 0.0;
			unsigned long tempIfOutDiscards = 0;

			if(data.counterValue[k][19] != "-")
			{
				tempIfOutDiscards = strtoul((data.counterValue[k][19]).c_str(),NULL,10);
				if(tempIfOutDiscards > snmpObj->m_ifOutDiscards[i])
				{
					pcCal.deprecated[i] = "deprecated";
				}
				else
				{

					tempIncr1 = (ULONGLONG)(snmpObj->m_ifOutDiscards[i] - tempIfOutDiscards)/1024;
					if(time != tempBaseTime)
						tempRate1 = ((double)tempIncr1 /(double) (time - tempBaseTime));

					if (!XCOUNTLS_Util::ulonglongToString(tempIncr1, pcCal.increment[i][6])) //longlong
					{
						if (_trace.ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
							_trace.ACS_TRA_event(1, tmpStr);
						}
						return INTERNALERR;
					}

					if(time != tempBaseTime)
					{
						if (!XCOUNTLS_Util::doubleToStringRate(tempRate1, pcCal.rate[i][6])) //double
						{
							if (_trace.ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] longlongToString() method failed", __LINE__);
								_trace.ACS_TRA_event(1, tmpStr);
							}
							return INTERNALERR;
						}
					}
				}

			}


		}
	//}
	return retcode;
}

//-----------------------------------------------------------

// printBaseline
//	Prints Performance Counters Baseline Data
//-------------------------------------
int printBaseline()
{
	TraceInOut inOut(__FUNCTION__);
	std::vector<imbInfo> imbVector;
	int retcode = EXECUTED;
	int retcodeCMX = EXECUTED;
	fixs_cch_cmdshelfmanager ironsideManager("");
	//Skip the hardware check in DMX Environment
	if (m_environment != CMD_Util::SINGLECP_DMX && m_environment != CMD_Util::MULTIPLECP_DMX)
	{
		hwc = ACS_CS_API::createHWCInstance();

		if (hwc)
		{
			boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
			if (boardSearch)
			{
				retcode = allEGEM2L2SwitchBoardInfo(imbVector,isSMX);
				if(retcode == NO_REPLY_FROM_CS)
				{
					if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
					if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
					return retcode;
				}
				//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Return code of allEGEM2L2SwitchBoardInfo() is :" << retcode<< std::endl;

				retcodeCMX = allCMXBoardInfo(imbVector);
				if(retcodeCMX == NO_REPLY_FROM_CS)
				{
					if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
					if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
					return retcodeCMX;
				}
				//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Return code of allCMXBoardInfo() is :" << retcode<< std::endl;
				//std::cout << "DBG: imb vector size" << (int) imbVector.size() << std::endl;
			}
			else
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance() failed", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retcode = INTERNALERR;//Memory error
				if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
				return retcode;
			}
		}
		else
		{
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createHWCInstance() failed", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = INTERNALERR;//Memory error
			return retcode;
		}

		if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
	}
	else //DMX ENVIRONMENT
	{
		//TODO: Ask DMX for the list of Bridges defined and fill imbVector informations!!!!
		std::vector<std::string> magAddrs;
		retcode = ironsideManager.getShelfListUDP(magAddrs);
		if(retcode != XCOUNTLS_Util::EXECUTED)
			return retcode;
		for (std::vector<std::string>::iterator it = magAddrs.begin(); it != magAddrs.end(); it++)
		{

			imbInfo scx_0;
			scx_0.IP_EthA = "";
			scx_0.IP_EthB = "";
			XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), scx_0.mag);
			scx_0.slot = 0;
			imbVector.push_back(scx_0);
			imbInfo scx_25;
			scx_25.IP_EthA = "";
			scx_25.IP_EthB = "";
			XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), scx_25.mag);
			scx_25.slot = 25;
			imbVector.push_back(scx_25);
			imbInfo cmx_26;
			cmx_26.IP_EthA = "";
			cmx_26.IP_EthB = "";
			XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), cmx_26.mag);
			cmx_26.slot = 26;
			imbVector.push_back(cmx_26);
			imbInfo cmx_28;
			cmx_28.IP_EthA = "";
			cmx_28.IP_EthB = "";
			XCOUNTLS_Util::stringToUlongMagazine(it->c_str(), cmx_28.mag);
			cmx_28.slot = 28;
			imbVector.push_back(cmx_28);

		}
	}

	if((retcode == EXECUTED) || (retcodeCMX == EXECUTED))
	{
		int size = (int)imbVector.size();
		if(size == 0)
		{
			//std::cout << "Hardware not present" << std::endl;
			return HARDWARE_NOT_PRESENT;
		}

		sortVector(imbVector); // sort the list of SCXB and CMX boards
		//std::cout << " DBG: imb vector size_after: " << (int)imbVector.size() << std::endl;
		std::stringstream pcTable;
		pcTable << std::endl;


		std::string baselineHeading = "";

		if (FIXS_CCH_PCDataStorage::getInstance()->loadBaseline() == 0) {

			if(	!FIXS_CCH_PCDataStorage::getInstance()->readBaselineHeading(baselineHeading))
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] readBaselineHeading() failed", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				retcode = INTERNALERR;
				return retcode;
			}
		} else {
			if (_trace.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] loadBaseline() failed", __FUNCTION__, __LINE__);
				_trace.ACS_TRA_event(1, tmpStr);
			}
			retcode = INTERNALERR;
			return retcode;
		}

		pcTable << setiosflags(ios::left) << "BASELINE TIME" << " : " << baselineHeading << std::endl<< std::endl;

		//FIXS_CCH_Command_SNMPManager *snmpManager = 0;
		for(int i=0; i <= (size-1); i++)
		{
//			fixs_cch_cmdshelfmanager snmpManager("");
			int no_of_intrface[2]= {-1};
			std::string magStr,slotStr;
			XCOUNTLS_Util::ulongToStringMagazine(imbVector[i].mag, magStr);
			XCOUNTLS_Util::ushortToString(imbVector[i].slot,slotStr);

			pcTable << setiosflags(ios::left) << "MAG" << " : " << magStr << "   "<<"SLOT" << " : " << slotStr << std::endl<< std::endl;

			XCOUNTLS_Util::PCData data;
			bool isTransportSwitch=false;
			if(	FIXS_CCH_PCDataStorage::getInstance()->readBoardCounters(imbVector[i].mag,imbVector[i].slot,data,no_of_intrface,isSMX))
			{
				ULONGLONG timeinseconds;
				XCOUNTLS_Util::stringToulonglong(data.timeValue,timeinseconds);
				time_t     formatTime;
				struct tm  ts;
				formatTime = (time_t)timeinseconds;
				ts = *localtime(&formatTime);
				char       buf[80];
				strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", &ts);
				std::string tempstr(buf);
				data.timeValue = tempstr;

				for(int it=0;it<2;it++) {
					if(it==1)
						isTransportSwitch=true;
					retcode = fillPrintOutDataForBoard(pcTable,data,no_of_intrface[it],isTransportSwitch);
					if (retcode == INTERNALERR) {
						return retcode;
					}
				}
			}
			else
			{
				if (_trace.ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] readBoardCounters() failed", __FUNCTION__, __LINE__);
					_trace.ACS_TRA_event(1, tmpStr);
				}
				//return INTERNALERR;
				pcTable << setiosflags(ios::left) << "Baseline not available for all rates/increment of that board" << std::endl << std::endl;

			}
		}

		std::cout << pcTable.str() << std::endl;		

		retcode = EXECUTED;
	}
	else
	{
		return HARDWARE_NOT_PRESENT;

	}
	return retcode;
}
//-----------------------------------------------------------

int main(int argc, char* argv[])
{
	TraceInOut inOut(__FUNCTION__);
	int retcode = 0;


	//-----------------------------------------------
	// C H E C K - N O D E - A R C H I T E C T U R E
	//-----------------------------------------------
	m_environment = CMD_Util::getEnvironment();

	if (m_environment == CMD_Util::UNKNOWN)
	{
		std::cout << "No reply from Configuration Service" << std::endl;
		return NO_REPLY_FROM_CS;
	}
	else if ((m_environment != CMD_Util::SINGLECP_CBA) && (m_environment != CMD_Util::MULTIPLECP_CBA) &&
			(m_environment != CMD_Util::SINGLECP_DMX) && (m_environment != CMD_Util::MULTIPLECP_DMX) &&
			(m_environment != CMD_Util::SINGLECP_SMX) && (m_environment != CMD_Util::MULTIPLECP_SMX))
	{
		std::cout << "Illegal command in this system configuration" << std::endl;
		return ILLEGAL_COMMAND;
	}
	
	if ((m_environment == CMD_Util::SINGLECP_SMX) || (m_environment == CMD_Util::MULTIPLECP_SMX)){
		isSMX=true;
	}

	if (ACS_APGCC::is_active_node() == false)
	{
		std::cout << "Only Allowed on active node" << std::endl;
		return ONLY_ALLOWED_ON_ACTIVE_NODE;
	}
		
	int opt_v = 0; // verbose option 
	int opt_m = 0; // specifies MAG
	int opt_s = 0; // specifies SLOT
	int opt_i = 0; // specifies Interface
	int opt_S = 0; // specifies Status
	int opt_b = 0; // baseline option
	int opt_f = 0; // force option
	int opt_d = 0; // baseline delete option
	int opt_p = 0; // baseline printout option

	string slotPar("");
	string magazinePar("");
	string interfacePar("");
	//------------------------
	//O P T I O N - C H E C K
	//------------------------
	int  opt = 0;
	Get_opt getopt (argc, (char**)argv, CMD_OPTIONS);

	while ( ((opt = getopt(true)) != -1) && (opt != -2) )
	{
		switch(opt)
		{
		case 'v':
			if( opt_v )
				return printUsage();
			opt_v++;
			break;
		case 'b':
			if( opt_b )
				return printUsage();
			opt_b++;
			break;
		case 'd':
			if( opt_d )
				return printUsage();
			opt_d++;
			break;
		case 'm':
			if( opt_m )
				return printUsage();
			opt_m++;
			magazinePar = getopt.optarg;
			break;
		case 's':
			if( opt_s )
				return printUsage();
			opt_s++;
			slotPar = getopt.optarg;
			break;
		case 'i':
			if( opt_i )
				return printUsage();
			opt_i++;
			interfacePar = getopt.optarg;
			break;
		case 'S':
			if( opt_S )
				return printUsage();
			opt_S++;
			break;
		case 'f':
			if( opt_f )
				return printUsage();
			opt_f++;
			break;
		case 'p':
			if( opt_p )
				return printUsage();
			opt_p++;
			break;
		default :
			// Illegal option
			// Check if this is an known option
			if ( getopt.optind > 1 )
			{
				char* cp = (char*)argv[getopt.optind-1];
				if ( *cp == '-' )
					cp++;
				if ( ::strchr(CMD_OPTIONS,*cp) == NULL )
					return printUsage();

			}
			return printUsage();
		} // end switch
	} // end while

	//--------------------------------------------
	//C H E C K - M I X - O F - A R G U M E N T S
	//--------------------------------------------
	if ( getopt.optind < argc ) 
	{
		return printUsage();// Too many arguments
	}

	if(opt_v ==1)
	{
		if ((opt_s != 1) || (opt_m != 1))
			return printUsage();

	}

	if(opt_p ==1)
	{
		if((opt_v == 1) || (opt_m == 1) || (opt_s == 1)
				|| (opt_S == 1) || (opt_i == 1)|| (opt_b == 1)|| (opt_f == 1)  || (opt_d ==  1))
			return printUsage();
	}

	if(opt_f == 1)  // -f option is supported only when -b or -d is used
	{
		if((opt_b != 1) && (opt_d != 1))
			return printUsage();
	}

	if(opt_b ==1) //-b option is supported only with -f option
	{
		if((opt_v == 1) || (opt_m == 1) || (opt_s == 1) || (opt_d == 1)
				|| (opt_S == 1) || (opt_i == 1) || (opt_p == 1))
			return printUsage();
	}

	if(opt_d == 1) //-d option is supported only with -f option
	{
		if((opt_v == 1) || (opt_m == 1) || (opt_s == 1) || (opt_b == 1)
				|| (opt_S == 1) || (opt_i == 1) || (opt_p == 1))
			return printUsage();
	}

	if(!opt_b ==1)
	{
		if (magazinePar[0] == '-') return printUsage();
		if (slotPar[0] == '-') return printUsage();
		if (interfacePar[0] == '-') return printUsage();
	}

	if (opt == -2) return printUsage();

	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	if ( opt_m )
	{

		if (XCOUNTLS_Util::stringToUlong(magazinePar, umagazine) == false)
		{
			std::cout << "Invalid magazine number" << std::endl;
			return INCORRECT_MAGAZINE;
		}
	}

	if ( opt_s )
	{

		if (XCOUNTLS_Util::stringToUshort(slotPar, uslot) == false)
		{
			std::cout << "Invalid slot number" << std::endl;
			return INCORRECT_SLOT;
		}
	}

	if ( opt_i )
	{
			if (m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX)
                        {
				return printUsage();
                        }
                        else
			{
				if ((XCOUNTLS_Util::stringToInt(interfacePar, interfaceNr) == false) || (interfaceNr == INVALID_INTERFACE))
				{
					std::cout << "Interface number does not exist" << std::endl;
					return NOT_EXIST_INTERFACE;
				}
			}
	}	


	//-------------------------------------------------
	// CHECK COMBINATION OF OPTIONS
	//-------------------------------------------------
	unsigned int option = 0;

	if(opt_v == 1)  // when -v option is set
		SET_OPT_VERBOSE(option);

	if(opt_m == 1) // when -m option is set
		SET_OPT_MAG(option);

	if(opt_s == 1) // when -s option is set
		SET_OPT_SLOT(option);

	if(opt_i == 1) //when -i option is set
		{
		 	if (m_environment == CMD_Util::SINGLECP_DMX || m_environment == CMD_Util::MULTIPLECP_DMX)
                	{
				return printUsage();
			}
                	else
	                {
	
			SET_OPT_INTERFACE(option);
			}
		}

	if(opt_S == 1) //when -S option is set
		SET_OPT_STATUS(option);

	//std::cout << "DBG: OPTION == " << option << std::endl;

	if(opt_v == 1)
	{
		bool fileExists = false;
		FIXS_CCH_PCDataStorage PCObj ;
		fileExists =PCObj.checkFileExists();
		retcode = pcVerboseData(umagazine,uslot,option,fileExists);
	}
	else if(opt_b == 1)
	{
		bool fileExists = false;
		FIXS_CCH_PCDataStorage PCObj ;
		fileExists =PCObj.checkFileExists();

		if(opt_f == 1)
		{
			PCObj.deletePCDataStorageFile();
			retcode = pcBaseline();
		}
		else if (fileExists)
		{
			// Issue check printout    
			std::cout << "Are you sure that you want to overwrite existing baseline? \nEnter y or n [default: n]\03:" << std::flush;
			std::string line, choice;
			getline(cin, line);
			remove_copy( line.begin() , line.end() , back_inserter( choice ) , ' ');
			if ((choice == "n") || (choice == "N") || (choice == "")) 
			{
				return EXECUTED; //command aborted by users
			}
			if ((choice != "y") && (choice != "Y")) 
			{
				std::cout << "Illegal value - enter y or n" << std::endl; //illegal value received
				return INCORRECT_RESPONSE;
			}
			PCObj.deletePCDataStorageFile();
			retcode = pcBaseline();
		}
		else
		{
			retcode = pcBaseline();
		}

	}
	else if (opt_d == 1)
	{
		bool fileExists = false;

		FIXS_CCH_PCDataStorage PCObj;
		fileExists =PCObj.checkFileExists();

		if (fileExists)
		{
			if(opt_f != 1)
			{
				// Issue check printout   
				std::cout << "Are you sure that you want to delete existing baseline? \nEnter y or n [default: n]\03:" << std::flush;
 
				std::string line, choice;
				getline(cin, line);
				remove_copy( line.begin() , line.end() , back_inserter( choice ) , ' ');
				if ((choice == "n") || (choice == "N") || (choice == "")) 
				{
					return EXECUTED; //command aborted by users
				}
				if ((choice != "y") && (choice != "Y")) 
				{
					std::cout << "Illegal value - enter y or n" << std::endl; //illegal value received
					return INCORRECT_RESPONSE;
				}
			}

			bool result = PCObj.deletePCDataStorageFile();
			if(result == true)
				std::cout << "Baseline file deleted Successfully" << std::endl;
			else
			{
				retcode = FILE_DELETION_FAILED;
			}

		}
		else
		{
			retcode = BASELINE_FILE_NOT_FOUND;  
		}

	}
	else if(opt_p == 1)
	{
		FIXS_CCH_PCDataStorage PCObj ;

		if (!PCObj.checkFileExists())
		{
			std::cout << "Baseline file does not exist" << std::endl;
			return BASELINE_FILE_NOT_FOUND;
		}
		retcode = printBaseline();
	}
	else
	{
		bool fileExists = false;

		FIXS_CCH_PCDataStorage PCObj ;
		fileExists =PCObj.checkFileExists();
		retcode = pcDefaultData(umagazine,uslot,option,fileExists);
		if(retcode == INTERNALERR)
			return retcode;
	}


	if(retcode != EXECUTED)
	{
		switch (retcode)
		{
		case INCORRECTUSAGE:				// 2
			printUsage();
			break;
		

		case INCORRECT_MAGAZINE:			//30
			std::cout << "Invalid magazine number" << std::endl;
			break;

		case INCORRECT_SLOT:				//31
			std::cout << "Invalid slot number" << std::endl;
			break;

		case NO_REPLY_FROM_CS:				// 7
			std::cout << "No reply from Configuration Service" << std::endl;
			break;

		case SNMP_FAILURE:					// 13
			// std::cout << "SNMP Failure" << std::endl;
			break;

		case HARDWARE_NOT_PRESENT:			// 14
		case DMX_TENANT_NOT_FOUND:
			std::cout << "Hardware not present" << std::endl;
			break;

		case FUNC_BOARD_NOTALLOWED:			//22
			std::cout << "Not Allowed for this functional Board Name" << std::endl;
			break;

		case DISK_FULL:				//24
			cout<<"Disk quota exceeded"<<endl;
			break;

		case NO_INTERFACE_UP:               //35
			break;

		case NOT_EXIST_INTERFACE:           //36
			//std::cout << "Interface number does not exist" << std::endl;
			break;

		case BASELINE_FILE_NOT_FOUND:       //37
			std::cout << "Baseline file does not exist" << std::endl;
			break;

		case FILE_DELETION_FAILED:         //38
			std::cout << "Baseline file cancellation Failed" << std::endl;
			break;

		case ILLEGAL_COMMAND:				// 115
			std::cout<< "Illegal command in this system configuration" << std::endl;
			break;
		case IRONSIDE_FAILURE:
			  std::cout << "Communication Failure" << std::endl;
			break;
		case INTERNALERR:					// 1
		default:
			std::cout << "\nError when executing" << std::endl;
			break;
		}    
	}
	return retcode;
}
