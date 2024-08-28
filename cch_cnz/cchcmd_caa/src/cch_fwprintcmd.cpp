/*
 * cch_fwversioncmd.cpp
 *
 *  Created on: Mar 07, 2012
 *      Author: tcslali
 */

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <iomanip>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <stdint.h>

#include "ACS_CS_API.h"
#include "ACS_DSD_Session.h"
#include "ACS_DSD_Client.h"
#include "ACS_TRA_trace.h"
#include "fixs_cch_cmdshelfmanager.h"

#include "Get_opt.h"
#include "cmd_Utils.h"
#include "Client_Server_Interface.h"

#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/DocumentHandler.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

XERCES_CPP_NAMESPACE_USE
XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

using namespace std;

// Command Options
#define CMD_OPTIONS  "m:s:"
#define IPMI_TYPE_COUNT 3

namespace {
ACS_CS_API_HWC * hwc = 0;
ACS_CS_API_BoardSearch * boardSearch = 0;
ACS_TRA_trace traceCmd=ACS_TRA_trace("cch_fwversioncmd");
class TraceInOut
{
public:
	TraceInOut(const char * function) : m_function(function)
	{
		if (traceCmd.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] In", m_function.c_str());
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	};

	~TraceInOut()
	{
		if (traceCmd.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Out", m_function.c_str());
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
private:
	std::string m_function;
};
}

#define MAX_VERSION_INDEX 20

typedef struct
{
	std::string loadedType[MAX_VERSION_INDEX];
	std::string loadedVersion[MAX_VERSION_INDEX];
	std::string productno;
	std::string cxp_rState;

} FWMUpgradeInfo;


// Global functions
int isOperationAllowed (unsigned long umagazine,unsigned short uslot, int & env_switch);
int  printUsage(const int errorCode=0, const char opt=0, const char* arg=0, bool usage=true);
int getFirmwareVersion(unsigned long umagazine,unsigned short uslot, bool isSMX);
void printVersionInfo(FWMUpgradeInfo &info, bool isSMX);
int isUpgradeOngoing (unsigned long umagazine,unsigned short uslot,int env);
int containerFileInfo(string containerFile);
bool findRfuDataFile (std::string &filenameSh);
bool findMetadataFile (std::string &filenameXml);
bool initializeParser (std::string &metadataFilename, FWMUpgradeInfo &info);
void printContainerFileInfo(FWMUpgradeInfo &info);
int fetchSwitchInfo (unsigned long umagazine,unsigned short uslot,unsigned long &uIP_EthA,unsigned long &uIP_EthB);
bool readFwFromJson(boost::property_tree::ptree const& pt, FWMUpgradeInfo &info);
bool parseJsonFile(std::string &metadataFilename, FWMUpgradeInfo &info);
void sequenceFWType(FWMUpgradeInfo &info);
int getCmxSoftwareVersion (unsigned long umagazine,unsigned short uslot, int & sw_version);
const std::string TEMP_FWFOLDER ="/tmp/temp_fwinstall";
static int fwInfoCount = 0;
std::string m_ipA("");
std::string m_ipB("");

//===========================================
class SaxHandler : virtual public HandlerBase
{
public:
	SaxHandler (const std::string &xmlFilePath, FWMUpgradeInfo &info);
	virtual ~SaxHandler();
	void startElement (const XMLCh* const  name, AttributeList &attributes);
	void characters(const XMLCh* const chars, const XMLSize_t length);
	void warning (const SAXParseException &exception);
	void error (const SAXParseException &exception);
	void fatalError (const SAXParseException &exception);

protected:

private:
	SaxHandler(const SaxHandler &right);
	SaxHandler & operator=(const SaxHandler &right);
	FWMUpgradeInfo &m_info;
	bool m_name ;
	bool m_cxc ;
	bool m_rState ;
	bool m_cxp;
	bool m_cxp_rState;
};

SaxHandler::SaxHandler (const std::string &xmlFilePath, FWMUpgradeInfo &info)
: m_info(info)
{
	m_name = false;
	m_cxc = false;
	m_rState = false;
	UNUSED(xmlFilePath);
}

SaxHandler::~SaxHandler()
{
}

void SaxHandler::startElement (const XMLCh* const  name, AttributeList &attributes)
{
	UNUSED(attributes);
	//std::string element = XMLString::transcode(name);
	char* elem = XMLString::transcode(name);
	std::string element(elem);
	//cout << " element = " << element.c_str() << std::endl;

	if (element == "name")
	{
		m_name = true;
	}

	if (element == "cxc")
	{
		m_cxc = true;
	}

	if (element == "cxc_rstate")
	{
		m_rState = true;
	}

	if (element == "product_number")
	{
		m_cxp = true;
	}

	if (element == "cxp_rstate")
	{
		m_cxp_rState = true;
	}
}

void SaxHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
	UNUSED(length);

	if (m_name)
	{
		std::string name = XMLString::transcode(chars);
		CMD_Util::stringToUpper (name);
		m_info.loadedType[fwInfoCount]= name;
		m_name = false;
	}

	if (m_cxc)
	{
		std::string cxc = XMLString::transcode(chars);
		CMD_Util::stringToUpper (cxc);
		m_info.loadedVersion[fwInfoCount]= cxc;
		m_cxc = false;
	}

	
	if (m_cxp)
	{
		std::string cxp = XMLString::transcode(chars);
		CMD_Util::stringToUpper (cxp);
		m_info.productno= cxp;
		m_cxp = false;
	}

	if (m_cxp_rState)
	{
		std::string rState = XMLString::transcode(chars);
		CMD_Util::stringToUpper (rState);
		m_info.cxp_rState= rState;
		m_cxp_rState= false;
	}

	if (m_rState)
	{
		std::string rState = XMLString::transcode(chars);
		CMD_Util::stringToUpper (rState);
		m_info.loadedVersion[fwInfoCount]+= "_" + rState;
		fwInfoCount++;
		m_rState = false;
	}
}

void SaxHandler::warning (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceCmd.ACS_TRA_ON()) { //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] xml error: warning method got called", __FUNCTION__, __LINE__);
		traceCmd.ACS_TRA_event(1, tmpStr);
	}
}

void SaxHandler::error (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceCmd.ACS_TRA_ON()) { //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] xml error: error method got called", __FUNCTION__, __LINE__);
		traceCmd.ACS_TRA_event(1, tmpStr);
	}
}

void SaxHandler::fatalError (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceCmd.ACS_TRA_ON()) { //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] xml error: fatalError method got called", __FUNCTION__, __LINE__);
		traceCmd.ACS_TRA_event(1, tmpStr);
	}
}

//******************************************************************************
// Print error code and possible usage information
//******************************************************************************

int printUsage(const int errorCode, const char opt, const char* arg, bool usage)
{
	UNUSED(opt);
	UNUSED(arg);

	TraceInOut inOut(__FUNCTION__);
//	cerr<<"\nIncorrect usage"<<endl;
	if ( usage )
	{
		cerr<<"\nIncorrect usage"<<std::endl;
		cerr << "Usage: fwprint -m magazine -s slot" << std::endl;
		cerr << "Usage: fwprint containerFile" << std::endl;
		cerr << std::endl;
		cerr << "COMMAND OPTIONS:" << std::endl;
		cerr << "-m magazine         Magazine address" << std::endl;
		cerr << "                       This option is used to specify the magazine address where" << std::endl;
		cerr << "                       the board is placed." << std::endl;
		cerr << "-s slot             Slot position" << std::endl;
		cerr << "                       This option is used to specify the slot where the board" << std::endl;
		cerr << "                       is placed." << std::endl;
		cerr << std::endl;
		cerr << "COMMAND OPERANDS:" << std::endl;
		cerr << "containerFile  Container file" << std::endl;
		cerr << "                       This operand is used to specify the container file " << std::endl;
	}
	return errorCode;
}

int isOperationAllowed (unsigned long umagazine,unsigned short uslot, int & env_switch)
{
	TraceInOut inOut(__FUNCTION__);
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
				switch (fbn)
				{
				// TR HW89113 BEGIN
				case ACS_CS_API_HWC_NS::FBN_SCXB:
				{
					env_switch = 0;
					retValue = EXECUTED;
					break;
				}
				case ACS_CS_API_HWC_NS::FBN_SMXB:
				{
					env_switch = 2;
					retValue = EXECUTED;
					break;
				}
				// TR HW89113 END
				case CMD_Util::FBN_CMXB:
				{
					env_switch = 1;
					retValue = EXECUTED;
					break;
				}
				default:
					retValue = FUNC_BOARD_NOTALLOWED;
				}
			}
			else
			{
				if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Cannot retrieve FBN from CS", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
				}
				retValue = NO_REPLY_FROM_CS;
			}
		}
		else
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Board List is empty", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
			retValue = HARDWARE_NOT_PRESENT;
		}
	}
	else if ((returnValue == ACS_CS_API_NS::Result_NoValue) || (returnValue == ACS_CS_API_NS::Result_NoEntry))
	{
		retValue = HARDWARE_NOT_PRESENT;
	}
	else
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS. result: %d", __FUNCTION__, __LINE__, returnValue);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		retValue = NO_REPLY_FROM_CS;
	}

	return retValue;
}

int fetchSwitchInfo (unsigned long umagazine,unsigned short uslot,unsigned long &uIP_EthA,unsigned long &uIP_EthB)
{
	TraceInOut inOut(__FUNCTION__);
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
			//			unsigned long IP_EthA = 0;
			uint32_t IP_EthA;
			returnValue = hwc->getIPEthA(IP_EthA, boardId);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				//trace{ std::string tmpstr = CCH_Util::ulongToStringIP (IP_EthA);	std::cout << "DBG: " << "The IP_EthA is:" << tmpstr <<std::endl; }

				//				unsigned long IP_EthB = 0;
				uint32_t IP_EthB;
				returnValue = hwc->getIPEthB(IP_EthB, boardId);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					//trace { std::string tmpstr = CCH_Util::ulongToStringIP (IP_EthB); std::cout << "DBG: " << "The IP_EthB is:" << tmpstr <<std::endl; }
					uIP_EthA = IP_EthA;
					uIP_EthB = IP_EthB;
					retValue = EXECUTED;
				}
				else
				{
					if (traceCmd.ACS_TRA_ON()) { //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] getIPEthB failed, error from CS", __FUNCTION__, __LINE__);
						traceCmd.ACS_TRA_event(1, tmpStr);
					}
					retValue = NO_REPLY_FROM_CS;
				}
			}
			else
			{
				if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] getIPEthA failed, error from CS", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
				}
				retValue = NO_REPLY_FROM_CS;
			}
		}
		else
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Board List is empty", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
			retValue = HARDWARE_NOT_PRESENT;
		}
	}
	else
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		retValue = NO_REPLY_FROM_CS;
	}
	return retValue;
}

int isUpgradeOngoing (unsigned long umagazine,unsigned short uslot,int env)
{
	UNUSED(umagazine);
	UNUSED(uslot);

	TraceInOut inOut(__FUNCTION__);
	unsigned long ipA = 0;
	unsigned long ipB = 0;
	int result = EXECUTED;
	int sw_version = 0;
	if(env == 1)
	{
		result = getCmxSoftwareVersion (umagazine,uslot, sw_version);
		if(result != EXECUTED)
			return result;
	}

	result = fetchSwitchInfo(umagazine,uslot,ipA,ipB);
	if(result != EXECUTED)
		return result;

	m_ipA = CMD_Util::ulongToStringIP(ipA);
	m_ipB = CMD_Util::ulongToStringIP(ipB);

	fixs_cch_cmdshelfmanager *snmpManager = new fixs_cch_cmdshelfmanager("");
	snmpManager->setIpAddress(m_ipA);
	if((env == 2) || ((env == 1) && (sw_version == 4)))
	{
		int status = -1;

		if(snmpManager->getFwmActionResult(status))
		{
			if(status == 1)
				result = WAIT_FOR_UPGRADE_ONGOING;
			else
			{
				result = EXECUTED;
			}
		}
		else
		{
			snmpManager->setIpAddress(m_ipB);
			if(snmpManager->getFwmActionResult(status))
			{
				if(status == 1)
					result = WAIT_FOR_UPGRADE_ONGOING;
				else
				{
					result = EXECUTED;
				}

			}
			else
			{
				//cout << "Could not get fwmAction result" << endl;
				if(snmpManager != NULL)
				{
					delete snmpManager;
					snmpManager = NULL;
				}
				return SNMP_FAILURE;
			}
		}

	}
	else
	{

		std::string actState("");

		if(snmpManager->getBladeRevisionState(&actState))
		{
			//cout << "Blade product revision state = "<<actState<<endl;
			size_t found;
			found = actState.find("+");
			if(found != string::npos)
			{
				result = WAIT_FOR_UPGRADE_ONGOING;
			}
			else
			{
				result = EXECUTED;
			}
		}
		else
		{
			snmpManager->setIpAddress(m_ipB);
			if(snmpManager->getBladeRevisionState(&actState))
			{
				size_t found;
				found = actState.find("+");
				if(found != string::npos)
				{
					result = WAIT_FOR_UPGRADE_ONGOING;
				}
				else
				{
					result = EXECUTED;
				}
			}
			else
			{
				if(snmpManager != NULL)
				{
					delete snmpManager;
					snmpManager = NULL;
				}
				return SNMP_FAILURE;
			}
		}
	}

	if(snmpManager) delete snmpManager;

	snmpManager = NULL;
	return result;
}

void printVersionInfo(FWMUpgradeInfo &info, bool isSMX)
{
	std::stringstream fwmTable;
	fwmTable << std::endl;
	fwmTable << setiosflags(ios::left) <<"FW LOADED TABLE" << std::endl;
	fwmTable << std::endl;
	fwmTable << setiosflags(ios::left) << std::setw(30) << "FW TYPE" << std::setw(1) << "" << "FW VERSION" << std::endl;
	bool isAreaA = false;

	for(int j=0;j<MAX_VERSION_INDEX;j++)
	{
		if((info.loadedType[j] != "") || (info.loadedVersion[j] != ""))
		{
			CMD_Util::stringToUpper (info.loadedType[j]);
			CMD_Util::stringToUpper (info.loadedVersion[j]);
			if(isSMX)
			{
				if((info.loadedType[j]).compare("CXP_TYPE_A") == 0 ) 
				{
					isAreaA = true;
				}
				else if((((info.loadedType[j]).compare("IPMI_PARAMETERS_SENSOR_A") == 0 )) || ((info.loadedType[j]).compare("IPMI_PARAMETERS_SENSOR_B") == 0 ))
				{
					
					info.loadedType[j] = (isAreaA) ? ("IPMI_PARAMETERS_SENSOR_A") : ("IPMI_PARAMETERS_SENSOR_B");
				}
				else if((((info.loadedType[j]).compare("IPMI_PARAMETERS_PMB_A") == 0 )) || ((info.loadedType[j]).compare("IPMI_PARAMETERS_PMB_B") == 0 ))
				{
					info.loadedType[j] = (isAreaA) ? ("IPMI_PARAMETERS_PMB_A") : ("IPMI_PARAMETERS_PMB_B");
				}
			}
			fwmTable << setiosflags(ios::left) << std::setw(30) << info.loadedType[j] << std::setw(1) << "" << info.loadedVersion[j] ;
		}
	}
	std::cout << fwmTable.str();

}

int getFirmwareVersion(unsigned long umagazine,unsigned short uslot, bool isSMX)
{
	unsigned long ipA = 0;
	unsigned long ipB = 0;
	int result = EXECUTED;

	result = fetchSwitchInfo(umagazine,uslot,ipA,ipB);
	if(result != EXECUTED)
		return result;

	m_ipA = CMD_Util::ulongToStringIP(ipA);
	m_ipB = CMD_Util::ulongToStringIP(ipB);

	fixs_cch_cmdshelfmanager *snmpReader = new fixs_cch_cmdshelfmanager("");;
	snmpReader->setIpAddress(m_ipA);
	int attributeValue = -1;

	/********************** Fill versionInfo by making SNMP get queries *************/
	FWMUpgradeInfo versionInfo;
	bool infoFlag = false;
	int errorCode=fixs_cch_cmdshelfmanager::GETSET_SNMP_OK;

	for(int i =1; i<= MAX_VERSION_INDEX;i++)
	{
		attributeValue = -1;
		infoFlag = false;

		if(snmpReader->getFwmLoadedAttributeSCX(i,&attributeValue,errorCode))
		{
			if(attributeValue == 1) // 1 means currentSW
				infoFlag = true;
			else
				infoFlag = false;
		}
		else
		{
			std::string tmpIp = snmpReader->getIpAddress();

			if(tmpIp.compare(m_ipA) == 0)
				snmpReader->setIpAddress(m_ipB);
			else
				snmpReader->setIpAddress(m_ipA);
			if(snmpReader->getFwmLoadedAttributeSCX(i,&attributeValue,errorCode))
			{
				if(attributeValue == 1) // 1 means currentSW
					infoFlag = true;
				else
					infoFlag = false;
			}
			else if(errorCode == fixs_cch_cmdshelfmanager::GETSET_SNMP_OK_LEN_ZERO)
			{
				if(snmpReader != NULL)
				{
					delete snmpReader;
					snmpReader = NULL;
				}
				break;
			}
			else
			{
				/************SNMP FAILURE***********/
				if(snmpReader != NULL)
				{
					delete snmpReader;
					snmpReader = NULL;
				}
				return SNMP_FAILURE;
			}
		}

		if (infoFlag == true) // means currentSW
		{
			std::string fwmLoadedType = "", fwmLoadedVersion = "";

			/**************** fetching Firmware Load type ********************/
			if(snmpReader->getFwmLoadedType(i,fwmLoadedType))
			{
				versionInfo.loadedType[i-1] = fwmLoadedType;
			}
			else
			{
				std::string tmpIp = snmpReader->getIpAddress();
				if(tmpIp.compare(m_ipA) == 0)
					snmpReader->setIpAddress(m_ipB);
				else
					snmpReader->setIpAddress(m_ipA);
				if(snmpReader->getFwmLoadedType(i,fwmLoadedType))
				{
					versionInfo.loadedType[i-1] = fwmLoadedType;
				}
				else
				{
					/************SNMP FAILURE***********/
					if(snmpReader != NULL)
					{
						delete snmpReader;
						snmpReader = NULL;
					}
					return SNMP_FAILURE;
				}
			}

			/**************** fetch firmware loaded version **************/
			if(snmpReader->getFwmLoadedVersion(i, fwmLoadedVersion))
			{
				versionInfo.loadedVersion[i-1] = fwmLoadedVersion ;
			}
			else
			{
				std::string tmpIp = snmpReader->getIpAddress();
				if(tmpIp.compare(m_ipA) == 0)
					snmpReader->setIpAddress(m_ipB);
				else
					snmpReader->setIpAddress(m_ipA);
				if(snmpReader->getFwmLoadedVersion(i, fwmLoadedVersion))
				{
					versionInfo.loadedVersion[i-1] = fwmLoadedVersion ;
				}
				else
				{
					/************SNMP FAILURE***********/
					if(snmpReader)
						delete snmpReader;
					snmpReader = NULL;
					return SNMP_FAILURE;
				}
			}
			
		}
		else  // means noAttribute
		{
			versionInfo.loadedType[i-1] = "";
			versionInfo.loadedVersion[i-1] = "";
		}
	}

	/********** Print version info ****************/
	printVersionInfo(versionInfo,isSMX);

	if(snmpReader)
		delete snmpReader;
	snmpReader = NULL;
	return 0;
}

int containerFileInfo(string containerFile1)
{
	std::string containerFile=containerFile1;
	int result = 0;
	std::string FILE_FOLDER = CMD_Util::getNbiSwPath(CMD_Util::swPackageFw);
	if(FILE_FOLDER.empty()) // not able to fetch folder path from FileM
	{
		return INTERNALERR;
	}
	if(containerFile.find("/") != string::npos)
	{
		std::string tmp = containerFile.substr(0,(containerFile.find_last_of("/")));
		if(tmp != FILE_FOLDER)
			return INCORRECT_PACKAGE;
		else
		{
			tmp = containerFile.substr(containerFile.find_last_of("/")+1);
			containerFile = tmp;
		}
	}

	//std::string archivePath = containerFile.substr(0,(containerFile.find_last_of("/")+1));
	std::string archivePath = FILE_FOLDER;
	std::string zipFileName = containerFile;
	std::string tempFolder = TEMP_FWFOLDER + "/" + zipFileName ;
	std::string zipFileLog = TEMP_FWFOLDER + "/" + "logFile.txt" ;
	std::string containerFilePath = archivePath + "/" + containerFile;
	std::string m_cmdFolder("");
	std::string cmdUnzip("");
	if(CMD_Util::CheckExtention(containerFile.c_str(),".zip"))
	{
		cmdUnzip = "unzip \"" + containerFilePath +"\" -d \"" + TEMP_FWFOLDER + "\"&>/dev/null";
	}

	else if(CMD_Util::CheckExtention(containerFile.c_str(),".tar"))
	{
		cmdUnzip = "tar xfv \"" + containerFilePath +"\" -C \"" + TEMP_FWFOLDER + "\"&>/dev/null";
	}

	else if(CMD_Util::CheckExtention(containerFile.c_str(),".tar.gz"))
	{
		cmdUnzip = "tar zxfv \"" + containerFilePath +"\" -C \"" + TEMP_FWFOLDER + "\">/dev/null";
	}
	else return DIRECTORYERR;

	//std::string cmdUnzip = "7za -o/" + TEMP_FWFOLDER + "/ x " + tempFolder + "/" + " >> zipFileLog" ;
	m_cmdFolder = /*CMD_Util::TEMP_FOLDER + "/" + */zipFileLog;

	if(!CMD_Util::PathFileExists(TEMP_FWFOLDER.c_str()))
	{
		ACS_APGCC::create_directories(TEMP_FWFOLDER.c_str(),ACCESSPERMS);
	}
	else
	{
		CMD_Util::removeFolder(TEMP_FWFOLDER.c_str());
		ACS_APGCC::create_directories(TEMP_FWFOLDER.c_str(),ACCESSPERMS);
	}
	
	string tempDest = TEMP_FWFOLDER + "/";
	std::string shFilename = "";
	std::string metadataFilename = "";
	if ( (system(cmdUnzip.c_str())) == 0)
	{
		if(findRfuDataFile(shFilename))
		{
			if(findMetadataFile(metadataFilename))
			{
				metadataFilename = TEMP_FWFOLDER + "/" + metadataFilename;

				FWMUpgradeInfo info;

				if(initializeParser(metadataFilename,info))
				{
					printContainerFileInfo(info);
					result = EXECUTED;
				}
				else
				{
					result = XMLERROR;
				}
			}
			else
			{
				result = INCORRECT_PACKAGE;
			}
		}
		else
		{
			result = INCORRECT_PACKAGE;
		}
	}
	else
	{
		result = INCORRECT_PACKAGE;
	}

	CMD_Util::removeFolder(TEMP_FWFOLDER.c_str());

	return result;
}

bool findRfuDataFile (std::string &filenameSh)
{
	DIR *dp;
	bool res = false;
	struct dirent *dirp;
	if((dp  = opendir(TEMP_FWFOLDER.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << TEMP_FWFOLDER << endl;
		return false;
	}

	while ((dirp = readdir(dp)) != NULL) {
		CMD_Util::EnvironmentType m_environment;
			m_environment = CMD_Util::getEnvironment();

		if((m_environment != CMD_Util::SINGLECP_SMX) && (m_environment != CMD_Util::MULTIPLECP_SMX))
		{
			if(string(dirp->d_name).find(".sh") != string::npos)
			{
				filenameSh.assign(string(dirp->d_name));
				res = true;
				//cout << "found sh file : "<<filenameSh<<endl;
				break;
			}
		}
		else
		{

			if(string(dirp->d_name).find(".tar.gz") != string::npos)
			{
				filenameSh.assign(string(dirp->d_name));
				res = true;
				//std::cout << "found tar file : "<<filenameSh<<std::endl;
				break;
			}
		}
	}
	closedir(dp);
	return res;

}

bool findMetadataFile (std::string &metadataFilename)
{
	DIR *dp;
	bool res = false;
	struct dirent *dirp;
	if((dp  = opendir(TEMP_FWFOLDER.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << TEMP_FWFOLDER << endl;
		return false;
	}

	while ((dirp = readdir(dp)) != NULL) {
		CMD_Util::EnvironmentType m_environment =  CMD_Util::getEnvironment();
		if((m_environment == CMD_Util::SINGLECP_SMX) || (m_environment == CMD_Util::MULTIPLECP_SMX))
                {
			if(string(dirp->d_name).find(".json") != string::npos)
			{
				metadataFilename.assign(string(dirp->d_name));
				res = true;
				//cout << "found sh file : "<<filenameSh<<endl;
				break;
			}
		}
		else
		{
			if(string(dirp->d_name).find(".xml") != string::npos)
			{
				metadataFilename.assign(string(dirp->d_name));
				res = true;
				//cout << "found xml file : "<< metadataFilename.c_str()<<endl;
				break;
			}
		}
	}
	closedir(dp);
	return res;
}

bool initializeParser (std::string &metadataFilename, FWMUpgradeInfo &info)
{
  CMD_Util::EnvironmentType m_environment =  CMD_Util::getEnvironment();
  if ((m_environment != CMD_Util::SINGLECP_SMX) && (m_environment != CMD_Util::MULTIPLECP_SMX))
  {
	bool errorOccurred = false;

	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& e)
	{
		// trace StrX(e.getMessage())
		if (traceCmd.ACS_TRA_ON()) { //trace
			char* xmlExp = XMLString::transcode(e.getMessage());
			std::string xmlException(xmlExp);
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "XMLException: %s", xmlException.c_str() );
			traceCmd.ACS_TRA_event(1, tmpStr);
			XMLString::release(&xmlExp);
		}
		if (traceCmd.ACS_TRA_ON()) { //trace

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,  "[%s@%d] XML Exception. Cannot initialize Xerces parser.", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);

		}

		return false;
	}

	SAXParser* parser = new SAXParser;

	parser->setValidationScheme(SAXParser::Val_Auto);
	parser->setDoNamespaces(false);
	parser->setDoSchema(false);
	parser->setValidationSchemaFullChecking(false);

	//  Create our SAX handler object and install it on the parser
	SaxHandler handler(metadataFilename, info);
	parser->setDocumentHandler(&handler);
	parser->setErrorHandler(&handler);


	char  filenameXmlParse[MAX_PATH];
	snprintf(filenameXmlParse, sizeof(filenameXmlParse) - 1, "%s", metadataFilename.c_str());

	try
	{
		parser->parse(metadataFilename.c_str());
	}
	catch (const OutOfMemoryException& e)
	{
		//trace "OutOfMemoryException"

		if (traceCmd.ACS_TRA_ON()) { //trace
			char* xmlExp = XMLString::transcode(e.getMessage());
			std::string xmlException(xmlExp);
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "OutOfMemoryException: %s", xmlException.c_str() );
			traceCmd.ACS_TRA_event(1, tmpStr);
			XMLString::release(&xmlExp);
		}
		if (traceCmd.ACS_TRA_ON()) { //trace

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,  "[%s@%d] OutOfMemoryException. Cannot parse xml file %s.", __FUNCTION__, __LINE__, metadataFilename.c_str());
			traceCmd.ACS_TRA_event(1, tmpStr);

		}

		//		FIXS_CCH_Logger::createInstance()->log(traceChar, FIXS_CCH_Logger::Error);
		//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << traceChar << std::endl;
		errorOccurred = true;
	}
	catch (const XMLException& e)
	{
		//trace "\nError during parsing: '" << xmlFile << "'\n"<< "Exception message is:  \n"  << StrX(e.getMessage()) << "\n" << XERCES_STD_QUALIFIER endl;
		if (traceCmd.ACS_TRA_ON()) { //trace
			char* xmlExp = XMLString::transcode(e.getMessage());
			std::string xmlException(xmlExp);
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error during parsing xml file: %s", xmlException.c_str() );
			traceCmd.ACS_TRA_event(1, tmpStr);
			XMLString::release(&xmlExp);
		}
		if (traceCmd.ACS_TRA_ON()) { //trace

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,  "[%s@%d] XMLException. Error parsing xml file %s.", __FUNCTION__, __LINE__, metadataFilename.c_str());
			traceCmd.ACS_TRA_event(1, tmpStr);

		}

		errorOccurred = true;
	}
	catch (...)
	{
		//trace XERCES_STD_QUALIFIER cerr << "\nUnexpected exception during parsing: '" << xmlFile << "'\n";
		if (traceCmd.ACS_TRA_ON()) { //trace

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Unexpected exception during parsing");
			traceCmd.ACS_TRA_event(1, tmpStr);

		}
		if (traceCmd.ACS_TRA_ON()) { //trace

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,  "[%s@%d] Generic Exception got parsing xml file %s.", __FUNCTION__, __LINE__, metadataFilename.c_str());
			traceCmd.ACS_TRA_event(1, tmpStr);

		}

		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << traceChar << std::endl;
		errorOccurred = true;
	}

	delete parser;

	// And call the termination method
	XMLPlatformUtils::Terminate();

	return (!errorOccurred);
  }
  else
	return parseJsonFile(metadataFilename,info);

}

void printContainerFileInfo(FWMUpgradeInfo &info)
{
	std::stringstream fwmTable;
	fwmTable << std::endl;
	fwmTable << setiosflags(ios::left) <<"FW IN CONTAINER FILE" << std::endl;
	fwmTable << std::endl;
	fwmTable << setiosflags(ios::left) << std::setw(30) << "FW TYPE" << std::setw(1) << "" << "FW VERSION" << std::endl;
	CMD_Util::EnvironmentType m_environment =  CMD_Util::getEnvironment();
	if ((m_environment == CMD_Util::SINGLECP_SMX) || (m_environment == CMD_Util::MULTIPLECP_SMX))
	{
		std::string fwType;
		if(info.productno.compare(CMD_Util::SMX_FWTYPE_A_CXP) == 0 )
			fwType.assign("CXP_TYPE_A");
		else
			fwType.assign("CXP_TYPE_B");
		std::string cxpString = info.productno + "_" + info.cxp_rState;
		fwmTable << setiosflags(ios::left) << std::setw(30) << fwType << std::setw(1) << "" << cxpString << std::endl;
	}
	for(int j=0; j < fwInfoCount; j++)
	{
		fwmTable << setiosflags(ios::left) << std::setw(30) << info.loadedType[j]<< std::setw(1) << "" << info.loadedVersion[j] << std::endl;
	}
	std::cout << fwmTable.str();
}

int main (int argc, char * argv[])
{
	TraceInOut inOut(__FUNCTION__);

	// Check parameters.  To be done.
	int opt_m = 0; // specifies MAG
	int opt_s = 0; // specifies SLOT

	string containerFile("");

		////Check Architecture////
	CMD_Util::EnvironmentType m_environment;
	m_environment = CMD_Util::getEnvironment();

	if (m_environment == CMD_Util::UNKNOWN)
	{
		std::cout << "No reply from Configuration Service" << std::endl;
		return NO_REPLY_FROM_CS;
	}
	else if ((m_environment != CMD_Util::SINGLECP_CBA) && (m_environment != CMD_Util::MULTIPLECP_CBA) && (m_environment != CMD_Util::SINGLECP_SMX) && (m_environment != CMD_Util::MULTIPLECP_SMX))
	{
		std::cout << "Illegal command in this system configuration" << std::endl;
		return ILLEGAL_COMMAND;
	}


	//////////end check architecture/////////////

	//check if is active node
	if (ACS_APGCC::is_active_node() == false)
	{
		cout<<"Illegal command on passive node"<<endl;
		return UNABLECONNECT_PASSIVE;
	}

	string slotPar("");
	string magazinePar("");
	string tarGz("");

	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	int res = -1;

	if(argc == 2)
	{
		res = 0;
		containerFile = argv[1];
		if(containerFile[0]== '-')
		{
			return printUsage();
		}

		std::string package("");

		package = containerFile.substr(containerFile.find_last_of("/")+1,containerFile.length()-(containerFile.find_last_of("/")+1));
		unsigned int dotCount=0;

		//Check if containerFile contains any invalid characters
		for (unsigned int i = 1; i < package.length(); i++)
		{
			if(!(
					(package[i] >='A' && package[i] <= 'Z')||
					(package[i] >='a' && package[i] <= 'z')||
					(package[i] >='0' && package[i] <= '9')||
					(package[i] == '-') ||
					(package[i]) == '_' ||
					(package[i] == '.') ||
					(package[i] == '\\') ||
					(package[i] == ':')
			))
			{
			//	std::cout<<" containerFile : "<<package[i]<<std::endl;
				std::cout << "Wrong Package" << std::endl;
				return INCORRECT_PACKAGE;
			}

			if(package[i] == '.')
			{
				dotCount++;
			}
		}

		/* Routine to find .tar.gz extensions
		pos = package.find(".");
		tarGz = package.substr(pos);
		std::cout << "tarGzString: " << tarGz.c_str() << std::endl;*/

		if (dotCount > 1)
		{
			std::cout << "Wrong Package" << std::endl;
			return INCORRECT_PACKAGE;
		}
	}
	else
	{
		int  opt;
		Get_opt getopt (argc, (char**)argv, CMD_OPTIONS);

		if ( argc < 2 )
		{
			return printUsage(INCORRECTUSAGE);// Too few arguments
		}

		while ( ((opt = getopt(true)) != -1) && (opt != -2) )  {
			switch(opt)
			{
			case 'm':
				if( opt_m )
					return printUsage(INCORRECTUSAGE);
				opt_m++;
				magazinePar = getopt.optarg;
				break;

			case 's':
				if( opt_s )
					return printUsage(INCORRECTUSAGE);
				opt_s++;
				slotPar = getopt.optarg;
				break;


			default:// Illegal option
				// Check if this is an known option
				if ( getopt.optind > 1 )
				{
					char* cp = (char*)argv[getopt.optind-1];
					if ( *cp == '-' )
						cp++;
					if ( ::strchr(CMD_OPTIONS,*cp) == NULL )
						return printUsage(INCORRECTUSAGE, *cp);
				}

				return printUsage(INCORRECTUSAGE);
			} // end switch
		} // end while


		if ( getopt.optind < argc )
		{
			return printUsage(INCORRECTUSAGE);// Too many arguments
		}

		if (!(( opt_m == 1)&&(opt_s == 1))) return printUsage();

		//avoid to manage '-' first caracter
		if (magazinePar[0] == '-') return printUsage();

		if (slotPar[0] == '-') return printUsage();

		if (opt == -2) return printUsage();

		if ( opt_m )
		{
			if (CMD_Util::stringToUlongMagazine (magazinePar, umagazine) == false)
			{
				if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Wrong magazine entered", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
				}

				std::cout << "Invalid magazine number" << std::endl;	//30
				return INCORRECT_MAGAZINE;
			}
		}

		if ( opt_s )
		{
			if (CMD_Util::stringToUshort(slotPar, uslot) == false)
			{
				if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Wrong slot entered", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
				}

				std::cout << "Invalid slot number" << std::endl;
				return INCORRECT_SLOT;
			}
		}
	}
	////////////COMMAND HANDLING////////////

	if(res == -1)
	{
		hwc = ACS_CS_API::createHWCInstance();
		boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		int env_switch = -1;
		res = isOperationAllowed(umagazine,uslot, env_switch);

		if(res == EXECUTED)
		{
			bool isSMX=false;
			//fixs_cch_cmdshelfmanager snmpManager("");
			if((m_environment == CMD_Util::SINGLECP_SMX) || (m_environment == CMD_Util::MULTIPLECP_SMX))
				isSMX=true;
			res = isUpgradeOngoing(umagazine,uslot,env_switch);
			if(res == EXECUTED)
			{
				res = getFirmwareVersion(umagazine,uslot,isSMX);
			
			}
		}
		else
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]Failed to get board info", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
		}

		if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
	}

	else
	{
	//	cout << "CONTAINER FILE INFO ORDERED" << endl;
		res = containerFileInfo(containerFile);
	}
	if (res == EXECUTED)
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] fwversion command succesful", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	else
	{
		if (traceCmd.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Command failed", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}


		switch (res)
		{
		case INCORRECTUSAGE:				// 2
			cout<<"Incorrect usage"<<endl;
			break;
		case DIRECTORYERR:					// 3
			cout<<"Invalid file name"<<endl;
			break;

		case NO_REPLY_FROM_CS:				// 7
			std::cout << "No reply from Configuration Service" << std::endl;
			break;

		case UPGRADE_ALREADY_ONGOING:	// 8
			std::cout << "Upgrade already ongoing" << std::endl;
			break;

		case WRONG_OPTION:					// 10
			std::cout << "Wrong option in this state" << std::endl;
			break;

		case SNMP_FAILURE:					// 13
			std::cout << "SNMP Failure" << std::endl;
			break;

		case HARDWARE_NOT_PRESENT:			// 14
			std::cout << "Hardware not present" << std::endl;
			break;

		case INCORRECT_PACKAGE:				// 16
			std::cout << "Wrong Package" << std::endl;
			break;

		case WAIT_FOR_UPGRADE_ONGOING:		// 17
			std::cout << "Display command inhibited when firmware upgrade ongoing" << std::endl;
			break;

		case NOT_ALLOWED_UPGRADE_ONGOING: //18
			std::cout << "Not allowed, upgrade ongoing" << std::endl;
			break;

		case ILLEGAL_COMMAND:				// 115
			cout<<"Illegal command in this system configuration"<<endl;
			break;

		case UNABLECONNECT:					// 117
			cout<<"Unable to connect to the server"<<endl;
			break;

		case FUNC_BOARD_NOTALLOWED:
			cout<<"Not allowed for this Functional Board Name"<<endl;
			break;

		case INTERNALERR:					// 1
		default:
			std::cout << "Error when executing" << std::endl;
			break;
		}
	}

	if (traceCmd.ACS_TRA_ON()) { //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Return value = %d", __FUNCTION__, __LINE__, res);
		traceCmd.ACS_TRA_event(1, tmpStr);
	}

	return res;
}

bool parseJsonFile(std::string &metadataFilename, FWMUpgradeInfo &info)
{
	bool res = true;
	using boost::property_tree::ptree;
	std::ifstream jsonFile(metadataFilename.c_str());
	ptree pt;
        read_json(jsonFile, pt);
	res = readFwFromJson(pt,info);
	if(res == true)
		sequenceFWType(info);
	return res;

}

bool readFwFromJson(boost::property_tree::ptree const& pt, FWMUpgradeInfo &info)
{
    bool res = false;
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();
    std::string type; // package for Area A, Area B
    int countp =0, countf = 0;
    bool isAreaA = false;
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) 
    {
        //std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
	std::string str(it->first);
	if(str.compare("package") == 0)
	{
		ptree child = it->second;
		for (ptree::const_iterator itc = child.begin(); itc != child.end(); ++itc) {
			std::string str_ch(itc->first);
			if(str_ch.compare("product") == 0)
			{
				info.productno = itc->second.get_value<std::string>();
				countp++;
			}
			else if(str_ch.compare("revision") == 0)
			{
				info.cxp_rState = itc->second.get_value<std::string>();
				countp++;
			}
			else if(str_ch.compare("type") == 0)
			{
				std::string type = itc->second.get_value<std::string>();
				if(type.compare("A") == 0)
					isAreaA =true;
				else
					isAreaA =false;
				countp++;
			}
			if(countp >=3)
			{
				break; 
			}
		}			

	}
	else if(str.compare("firmware") == 0)
	{
		ptree child = it->second;
		std::string temp_cxc;
		std::string temp_file;
		std::string temp_version;
		bool isfile =false;
		for (ptree::const_iterator itc = child.begin(); itc != child.end(); ++itc) 
		{

		//	std::cout << itc->first << ": " << itc->second.get_value<std::string>() << std::endl;	
			ptree childb = itc->second;
			countf=0;
			for (ptree::const_iterator itb = childb.begin(); itb != childb.end(); ++itb)
			{
			
		//		std::cout << itb->first << ": " << itb->second.get_value<std::string>() << std::endl;
				std::string str_ch(itb->first);
				if(str_ch.compare("name") == 0)
				{
					temp_cxc = itb->second.get_value<std::string>();
					countf++;
				}
				else if(str_ch.compare("file") == 0)
				{
					temp_file = itb->second.get_value<std::string>();
					isfile = true;
					countf++;
				}
				else if(str_ch.compare("version") == 0)
				{
					temp_version = itb->second.get_value<std::string>();
					countf++;
				}
		
				if(isfile && (countf == 3))
				{
					CMD_Util::stringToUpper (temp_cxc);
					CMD_Util::stringToUpper (temp_version);
					
					if(temp_file.compare(CMD_Util::UBOOT_FILE) == 0)
					{
						info.loadedType[fwInfoCount]= (isAreaA) ? "UBOOT_A":"UBOOT_B";
						info.loadedVersion[fwInfoCount]= temp_cxc + "_" + temp_version;	
						fwInfoCount++;
					}
					else if(temp_file.compare(CMD_Util::FAILSAFE_FILE) == 0)
					{
						info.loadedType[fwInfoCount]= (isAreaA) ? "FAILSAFE_A":"FAILSAFE_B";
						info.loadedVersion[fwInfoCount]= temp_cxc + "_" + temp_version;
						fwInfoCount++;	
					}
					else if(temp_file.compare(CMD_Util::BMC_FILE) == 0)
					{
						info.loadedType[fwInfoCount] = (isAreaA) ? "IPMI_A":"IPMI_B";
						info.loadedVersion[fwInfoCount] = temp_cxc + "_" + temp_version;
						fwInfoCount++;	
					}
					else if(temp_file.compare(CMD_Util::SENSOR_FILE) == 0)
					{
						info.loadedType[fwInfoCount]= (isAreaA) ? "IPMI_PARAMETERS_SENSOR_A":"IPMI_PARAMETERS_SENSOR_B";
						info.loadedVersion[fwInfoCount]= temp_cxc + "_" + temp_version;	
						fwInfoCount++;
					}
					else if(temp_file.compare(CMD_Util::PMP_FILE) == 0)
					{
						info.loadedType[fwInfoCount]= (isAreaA) ? "IPMI_PARAMETERS_PMB_A":"IPMI_PARAMETERS_PMB_B";
						info.loadedVersion[fwInfoCount]= temp_cxc + "_" + temp_version;
						fwInfoCount++;	
					}
					isfile =false;
					countf = 0;
				}
				
				if(fwInfoCount >=5)
					break;
			}
			temp_cxc.assign("");
			temp_version.assign("");
			temp_file.assign("");
		}
	}
	if((countp >= 3) && (fwInfoCount >= 5))
	{
		res = true;			
	}
    }
    return res;
}

void sequenceFWType(FWMUpgradeInfo &info)
{
	std::string tempLoadedType[IPMI_TYPE_COUNT];
        std::string temploadedVersion[IPMI_TYPE_COUNT];
	int count =0;
	for(int i =0; i <= fwInfoCount; i++)
	{
		if(((info.loadedType[i]).compare("IPMI_A") == 0 ) || ((info.loadedType[i]).compare("IPMI_B") == 0 ))
		{
			tempLoadedType[0] = info.loadedType[i];
			temploadedVersion[0] = info.loadedVersion[i];
			count++;
		}
		else if(((info.loadedType[i]).compare("IPMI_PARAMETERS_SENSOR_A") == 0 ) || ((info.loadedType[i]).compare("IPMI_PARAMETERS_SENSOR_B") == 0 ))
		{
			tempLoadedType[1] = info.loadedType[i];
			temploadedVersion[1] = info.loadedVersion[i];
			count++;
		}
		else if(((info.loadedType[i]).compare("IPMI_PARAMETERS_PMB_A") == 0 ) || ((info.loadedType[i]).compare("IPMI_PARAMETERS_PMB_B") == 0 ))
		{
			tempLoadedType[2] = info.loadedType[i];
			temploadedVersion[2] = info.loadedVersion[i];
			count++;
		}

	}
	
	if(count != IPMI_TYPE_COUNT)
		return;
	count = 0;
	for(int i =0; i <= fwInfoCount; i++)
	{
		if(((info.loadedType[i]).compare("IPMI_A") == 0 ) || ((info.loadedType[i]).compare("IPMI_B") == 0 )
			|| ((info.loadedType[i]).compare("IPMI_PARAMETERS_SENSOR_A") == 0 )
			|| ((info.loadedType[i]).compare("IPMI_PARAMETERS_SENSOR_B") == 0 )
			|| ((info.loadedType[i]).compare("IPMI_PARAMETERS_PMB_A") == 0 )
			|| ((info.loadedType[i]).compare("IPMI_PARAMETERS_PMB_B") == 0 )) 
		{
			info.loadedType[i] = tempLoadedType[count];
			info.loadedVersion[i] = temploadedVersion[count];
			count++;
		}
		if(count == IPMI_TYPE_COUNT)
			break;
	}

}

int getCmxSoftwareVersion (unsigned long umagazine,unsigned short uslot, int & sw_version)
{
	TraceInOut inOut(__FUNCTION__);
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
				if(fbn == CMD_Util::FBN_CMXB)
				{
					uint16_t version = -1;
					ACS_CS_API_NS::CS_API_Result returnValue_sw = hwc->getSoftwareVersionType(version,boardId);
					if (returnValue_sw == ACS_CS_API_NS::Result_Success)
					{
						sw_version = version;
						retValue = EXECUTED;
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No software version found for cmxb", __LINE__);
						traceCmd.ACS_TRA_event(1, tmpStr);
						retValue = NO_REPLY_FROM_CS;//NO_REPLY_FROM_CS
					}
				}
				else
					retValue = FUNC_BOARD_NOTALLOWED;
			}
			else
			{
				if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Cannot retrieve FBN from CS", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
				}
				retValue = NO_REPLY_FROM_CS;
			}
		}
		else
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Board List is empty", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
			retValue = HARDWARE_NOT_PRESENT;
		}
	}
	else if ((returnValue == ACS_CS_API_NS::Result_NoValue) || (returnValue == ACS_CS_API_NS::Result_NoEntry))
	{
		retValue = HARDWARE_NOT_PRESENT;
	}
	else
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS. result: %d", __FUNCTION__, __LINE__, returnValue);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		retValue = NO_REPLY_FROM_CS;
	}

	return retValue;
}
