#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <string.h>

#include <stdint.h>
#include "ACS_CS_API.h"
#include "ACS_DSD_Session.h"
#include "ACS_DSD_Client.h"
#include "ACS_TRA_trace.h"
#include "ACS_APGCC_Util.H"

#include "Get_opt.h"
#include "cmd_Utils.h"
#include "Client_Server_Interface.h"

#include "fixs_cch_cmdshelfmanager.h"
#include <iomanip>
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


namespace {
	ACS_CS_API_HWC * hwc = 0;
	ACS_CS_API_BoardSearch * boardSearch = 0;
	ACS_TRA_trace traceCmd=ACS_TRA_trace("cch_remotefwupgradecmd");
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

using namespace std;

// Command Options
#define CMD_OPTIONS  "m:s:if"
// list here all the valid options
// "n:" means that n must be followed by something


// Global functions
int printUsage();
int isOperationAllowed (unsigned long umagazine,unsigned short uslot);
int sendreceiveMessage_fw(ACS_DSD_Session & ses, const CCHCommandType cmdType, const std::string data, const std::string slotPar, const std::string magazinePar, char response_sw_fw[], unsigned int respSize_sw_fw);
int printListFwInfo(const char response[], unsigned int respSize);
int fwStatus(ACS_DSD_Session & ses, const string magazinePar, const string slotPar);
int fwUpgrade(ACS_DSD_Session & ses, const string fwpkg,const string magazinePar,const string slotPar);
void strUpperCase(std::string& s);

bool isSMX=false;
#define MAX_VERSION_INDEX 20
typedef struct
{
	std::string loadedType[MAX_VERSION_INDEX];
	std::string loadedVersion[MAX_VERSION_INDEX];
	std::string productno;
	std::string cxp_rState;

} FWMUpgradeInfo;

bool findRfudataFile (std::string &filenameSh);
bool findMetadataFile (std::string &metadataFilename);
int containerFileInfo(string containerFile,unsigned long umagazine,unsigned short argSlot);
bool initializeParser (std::string &filenameXml, FWMUpgradeInfo &info);
static int fwInfoCount = 0;
const std::string TEMP_FWFOLDER1 ="/tmp/temp_fwinstall";
int fetchSmxIpFromShelf (unsigned long umagazine,std::string& slot0IpA, std::string& slot0IpB ,std::string& slot25IpA, std::string& slot25IpB);
int ChangeActiveAreaToUpgradeFW(FWMUpgradeInfo &info, unsigned long umagazine,unsigned short argSlot);
bool parseJsonFile(std::string &metadataFilename, FWMUpgradeInfo &info);
bool readFwFromJson(boost::property_tree::ptree const& pt, FWMUpgradeInfo &info);

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
	char* elem = XMLString::transcode(name);
	std::string element(elem);

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
int printUsage()
{
	TraceInOut inOut(__FUNCTION__);
	cerr<<"\nIncorrect usage"<<endl;

	cerr << "Usage: fwupgexec -m magazine -s slot [-f] containerFile" << endl;
	cerr << "Usage: fwupgexec -i -m magazine -s slot \n"<< endl;

	cerr << "COMMAND OPTIONS:" << endl;
	cerr << "-f                 Force" << endl;
	cerr << "                      This option is used to suppress dialogue mode." << endl;
	cerr << "-m magazine        Magazine address" << endl;
	cerr << "                      This option is used to specify the magazine address where the" << endl;
	cerr << "                      board is placed."<<endl;
	cerr << "-s slot            Slot position" << endl;
	cerr << "                      This option is used to specify the slot where the board is placed." << endl;
	cerr << "-i                 Display the Progress Report" << endl;
	cerr << "                      This option is used to display the progress report of an ongoing "<<endl;
	cerr << "                      FW upgrade."<< endl << endl;
	cerr << "COMMAND OPERANDS:" << endl;
	cerr << "containerFile  Container file" << endl;
	cerr << "                      This operand is used to specify the container file. "<<endl;

	return INCORRECTUSAGE;
}


int sendreceiveMessage_fw(ACS_DSD_Session & ses, const CCHCommandType cmdType, const string dataPar, const string slotPar, const string magazinePar, char response_sw_fw[], unsigned int respSize_sw_fw)
{
	TraceInOut inOut(__FUNCTION__);
	cch_upgrade_request_fw* request_fw = new (std::nothrow) cch_upgrade_request_fw();
	if (!request_fw) return INTERNALERR;

	memset(request_fw, 0, sizeof(cch_upgrade_request_fw));
	request_fw->cmdType = cmdType; //set CCH command
	strcpy(request_fw->data, dataPar.c_str());
	strcpy(request_fw->magazine, magazinePar.c_str());
	strcpy(request_fw->slot, slotPar.c_str());
	unsigned int reqSize_sw_fw = sizeof(cch_upgrade_request_fw);//set size

	ssize_t bytes_sent = 0;
	bytes_sent = ses.send(reinterpret_cast<void*>(request_fw), reqSize_sw_fw);

	if (bytes_sent < 0)
	{
		//Sending failed
		if (traceCmd.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] sendMsg Error. Error Text: %s, Error Code: %d", __FUNCTION__, __LINE__,ses.last_error_text(), ses.last_error());
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		delete request_fw;
		return UNABLECONNECT;
	}
	else
	{
		//sending ok
		ssize_t bytes_received = 0;
		bytes_received = ses.recv(response_sw_fw, respSize_sw_fw);

		if (bytes_received < 0)
		{
			//receiving failed
			if (traceCmd.ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] recvMsg Error. Error Text: %s, Error Code: %d", __FUNCTION__, __LINE__, ses.last_error_text(), ses.last_error());
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
			delete request_fw;
			return UNABLECONNECT;
		}
		else
		{
			//receiving ok
			respCode code_sw_fw = (respCode)response_sw_fw[0];
			int cd_sw_fw = static_cast<int>(code_sw_fw);
			delete request_fw;
			return cd_sw_fw;
		}
	}

	delete request_fw;
	return EXECUTED;
}

int printListFwInfo(const char response[], unsigned int respSize)
{
	TraceInOut inOut(__FUNCTION__);

	bool wrongSize = false;
	int nItem = 0;
	const int itemDim = sizeof(int);
	const int codeDim = sizeof(respCode);
	const int headerSize = itemDim + codeDim;

	const int len = respSize - headerSize;
	char bufSize[itemDim] = {0};
	if (respSize < codeDim + itemDim)
	{
		wrongSize = true;
	}
	else
	{
		memcpy(bufSize,response + codeDim,itemDim);//copy message
		nItem = (int)bufSize[0];
		if(len < (nItem * ITEM_SIZE))
		{
			wrongSize = true;
		}
	}
	if (wrongSize)
	{
		if (traceCmd.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] [fwDisplay] The size of the received data is wrong. nItem = %d", __FUNCTION__, __LINE__, nItem);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		return UNABLECONNECT;
	}
	else if (len == 0)
	{
		printf("%s\n","      ");
	}
	else
	{
		printf("%s\n","      ");
		char bufData[MAX_RESPONSE_SIZE] ={0};
		memcpy(bufData,response + headerSize,len);//copy message
		string aLine;
		char *tmpBuf = bufData;

		for (int i=0; i<nItem; i++)
		{
			aLine.assign(tmpBuf,ITEM_SIZE);
			printf ("%s \n", aLine.c_str());
			tmpBuf += ITEM_SIZE;
		}
	}

	return EXECUTED;
}


int fwUpgrade(ACS_DSD_Session & ses, const string fwpkg,const string magazinePar,const string slotPar)
{
	TraceInOut inOut(__FUNCTION__);


	if (traceCmd.ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] fwpkg: %s", __FUNCTION__, __LINE__, fwpkg.c_str());
		traceCmd.ACS_TRA_event(1, tmpStr);
	}

	std::string pathParameter = CMD_Util::getNbiSwPath(CMD_Util::swPackageFw) + "/" + fwpkg;

	char response[sizeof(int) + MAX_RESPONSE_SIZE] = {0};
	unsigned int respSize = sizeof(int) + MAX_RESPONSE_SIZE;
	int code = sendreceiveMessage_fw(ses, cmdRemoteFwUpgrade, pathParameter, slotPar, magazinePar, response, respSize);
	if (code == UNABLECONNECT)
	{
		if (traceCmd.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] [fwUpgrade] communication failed. Get Last Error text: %s",__FUNCTION__, __LINE__, ses.last_error_text());
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	return code;
}

int fwStatus(ACS_DSD_Session & ses, const string magazinePar, const string slotPar)
{
	TraceInOut inOut(__FUNCTION__);

	if (traceCmd.ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] magazinePar: %s, slotPar: %s ",__FUNCTION__, __LINE__, magazinePar.c_str(), slotPar.c_str());
		traceCmd.ACS_TRA_event(1, tmpStr);
	}

	char response[sizeof(int) + MAX_RESPONSE_SIZE] = {0};
	unsigned int respSize = sizeof(int) + MAX_RESPONSE_SIZE;

	int code = sendreceiveMessage_fw(ses, cmdRemoteFwStatus, "", slotPar, magazinePar,  response, respSize);
	if (code == UNABLECONNECT)
	{
		if (traceCmd.ACS_TRA_ON()){ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] [fwStatus] communication failed. Last Error text(): %s",__FUNCTION__, __LINE__, ses.last_error_text());
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	else if (code == EXECUTED)
	{
		printListFwInfo(response, respSize);
	}

	return code;
}


int isOperationAllowed (unsigned long umagazine,unsigned short uslot)
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
				case ACS_CS_API_HWC_NS::FBN_SCXB:
				case ACS_CS_API_HWC_NS::FBN_SMXB: //Considering SMX Architecture also
					retValue = EXECUTED;
					break;
				case CMD_Util::FBN_CMXB: //TODO ACS_CS_API_HWC_NS::FBN_CMXB:
					retValue = EXECUTED;
					break;
				default:
					retValue = FUNC_BOARD_NOTALLOWED;
				}
			}
			else
			{
				if (traceCmd.ACS_TRA_ON()){ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,
							"[%s@%d] Cannot retrieve FBN from CS", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
				}
				retValue = NO_REPLY_FROM_CS;
			}
		}
		else
		{
			if (traceCmd.ACS_TRA_ON()){ //trace
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
		if (traceCmd.ACS_TRA_ON()){ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS. result: %d", __FUNCTION__, __LINE__, returnValue);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		retValue = NO_REPLY_FROM_CS;
	}

	return retValue;
}


void strUpperCase(std::string& s)
{
	TraceInOut inOut(__FUNCTION__);
	std::string::iterator i = s.begin();
	std::string::iterator end = s.end();
	while (i != end)
	{
		*i = toupper((unsigned char)*i);
		++i;
	}
}


char ** transformArgs(int *newArgc,char *argv[],int argc,int *err)
{
	UNUSED(newArgc);
	UNUSED(argv);
	UNUSED(argc);
	UNUSED(err);

    char **newArgv=0;
    return newArgv;
}

int containerFileInfo(string containerFile1, unsigned long umagazine, unsigned short uslot)
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

	std::string archivePath = FILE_FOLDER;
	std::string zipFileName = containerFile;
	std::string tempFolder = TEMP_FWFOLDER1 + "/" + zipFileName ;
	std::string zipFileLog = TEMP_FWFOLDER1 + "/" + "logFile.txt" ;
	std::string containerFilePath = archivePath + "/" + containerFile;
	std::string m_cmdFolder("");
	std::string cmdUnzip("");

	if(CMD_Util::CheckExtention(containerFile.c_str(),".zip"))
	{
		cmdUnzip = "unzip \"" + containerFilePath +"\" -d \"" + TEMP_FWFOLDER1 + "\"&>/dev/null";
	}

	else if(CMD_Util::CheckExtention(containerFile.c_str(),".tar"))
	{
		cmdUnzip = "tar xfv \"" + containerFilePath +"\" -C \"" + TEMP_FWFOLDER1 + "\"&>/dev/null";
	}

	else if(CMD_Util::CheckExtention(containerFile.c_str(),".tar.gz"))
	{
		cmdUnzip = "tar zxfv \"" + containerFilePath +"\" -C \"" + TEMP_FWFOLDER1 + "\">/dev/null";
	}
	else return DIRECTORYERR;

	m_cmdFolder = CMD_Util::TEMP_FOLDER + "/" + zipFileLog;

	if(!CMD_Util::PathFileExists(TEMP_FWFOLDER1.c_str()))
	{
		ACS_APGCC::create_directories(TEMP_FWFOLDER1.c_str(),ACCESSPERMS);
	}
	else
	{
		CMD_Util::removeFolder(TEMP_FWFOLDER1.c_str());
		ACS_APGCC::create_directories(TEMP_FWFOLDER1.c_str(),ACCESSPERMS);
	}

	string tempDest = TEMP_FWFOLDER1 + "/";
	std::string shFilename = "";
	std::string xmlFilename = "";
	if ( (system(cmdUnzip.c_str())) == 0)
	{
		if(findRfudataFile(shFilename))
		{
			if(findMetadataFile(xmlFilename))
			{
				xmlFilename = TEMP_FWFOLDER1 + "/" + xmlFilename;
				//cout << "xmlFilename: " << xmlFilename.c_str() <<endl;
				FWMUpgradeInfo info;

				if(initializeParser(xmlFilename,info))
				{
					result = ChangeActiveAreaToUpgradeFW(info, umagazine,uslot);
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

	CMD_Util::removeFolder(TEMP_FWFOLDER1.c_str());
	return result;
}

bool findRfudataFile (std::string &filenameSh)
{
	bool res = false;
	DIR *dp;

	struct dirent *dirp;
	if((dp  = opendir(TEMP_FWFOLDER1.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << TEMP_FWFOLDER1 << endl;
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
        if((dp  = opendir(TEMP_FWFOLDER1.c_str())) == NULL) {
                std::cout << "Error(" << errno << ") opening " << TEMP_FWFOLDER1 << endl;
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
	// cout << "initializing parser for : "<< filenameXml.c_str() <<endl;
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
		errorOccurred = true;
	}
	catch (const XMLException& e)
	{
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
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] XMLException. Error parsing xml file %s.", __FUNCTION__, __LINE__, metadataFilename.c_str());
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

int fetchSmxIpFromShelf (unsigned long umagazine,std::string& slot0IpA, std::string& slot0IpB ,std::string& slot25IpA, std::string& slot25IpB)
{
	TraceInOut inOut(__FUNCTION__);
	int retcode = EXECUTED;

	ACS_CS_API_IdList boardList;
	slot0IpA = "";
	slot0IpB = "";
	slot25IpA = "";
	slot25IpB = "";

	boardSearch->reset();
	boardSearch->setMagazine(umagazine);

	ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList,boardSearch);
	if (returnValue == ACS_CS_API_NS::Result_Success) {
		if (boardList.size() == 0) {
			// trace: no SMX board found. Any SMX hasn't been configured yet.
			if (traceCmd.ACS_TRA_ON()) {
				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[line:%d] No SMX board found", __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
			retcode = HARDWARE_NOT_PRESENT;
		} else {
			//if (isSMX) {
				// got some SMX board
				for (unsigned int i = 0; i < boardList.size(); i++) {
					BoardID boardId = boardList[i];
					//uint32_t magazine = 0;
					unsigned short slot = 0;
					returnValue = hwc->getSlot(slot, boardId);
					if (returnValue == ACS_CS_API_NS::Result_Success) {
						// slot read
						if (traceCmd.ACS_TRA_ON()) {
							char tmpStr[512] = { 0 };
							snprintf(tmpStr, sizeof(tmpStr) - 1,"[line:%d] slot read:%u", __LINE__, slot);
							traceCmd.ACS_TRA_event(1, tmpStr);
						}

						uint32_t IP_EthA = 0;
						returnValue = hwc->getIPEthA(IP_EthA, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success) {
							uint32_t IP_EthB = 0;
							returnValue = hwc->getIPEthB(IP_EthB, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success) {
								//imbInfo xswitch;
								std::string ipA_String = CMD_Util::ulongToStringIP(IP_EthA);
								std::string ipB_String = CMD_Util::ulongToStringIP(IP_EthB);
								if (slot == 0) {
									slot0IpA = ipA_String;
									slot0IpB = ipB_String;
								} else if (slot == 25) {
									slot25IpA = ipA_String;
									slot25IpB = ipB_String;
								}

							} else {
								//serious fault in CS: No IP_EthB found for the SMXB board
								if (traceCmd.ACS_TRA_ON()) {
									char tmpStr[512] = { 0 };
									snprintf(tmpStr, sizeof(tmpStr) - 1,"[line:%d] Serious fault in CS: No IP_EthB found for the SMXB board",__LINE__);
									traceCmd.ACS_TRA_event(1, tmpStr);
								}
								retcode = NO_REPLY_FROM_CS;
								break;
							}
						} else {
							//serious fault in CS: No IP_EthA found for the SMXB board
							if (traceCmd.ACS_TRA_ON()) {
								char tmpStr[512] = { 0 };
								snprintf(tmpStr, sizeof(tmpStr) - 1,"[line:%d] Serious fault in CS: No IP_EthA found for the SMXB board",__LINE__);
								traceCmd.ACS_TRA_event(1, tmpStr);
							}
							retcode = NO_REPLY_FROM_CS;
							break;
						}
					} else {
						//serious fault in CS: No slot found for the SMXB board
						if (traceCmd.ACS_TRA_ON()) {
							char tmpStr[512] = { 0 };
							snprintf(tmpStr, sizeof(tmpStr) - 1,"[line:%d] Serious fault in CS: No slot found for the SMXB board",__LINE__);
							traceCmd.ACS_TRA_event(1, tmpStr);
						}
						retcode = NO_REPLY_FROM_CS;
						break;
					}

				}
		//	}

		}
	} else {
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] GetBoardIds failed, error from CS", __FUNCTION__,__LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		retcode = NO_REPLY_FROM_CS;
	}
	return retcode;
}

bool ushortToString (unsigned short uvalue, std::string &value)
{
	char tmp[16] = { 0 };
	snprintf(tmp, sizeof(tmp) - 1, "%u", uvalue);
	value = tmp;
	return true;
}

int getActiveAreaFromBoard(unsigned long umagazine,unsigned short slot, int & value)
{
	int masterSlot = -1;
	std::string slot0IpA("");
	std::string slot0IpB("");
	std::string slot25IpA("");
	std::string slot25IpB("");
	bool result = true;
	std::string slotStr = "";

	fetchSmxIpFromShelf(umagazine, slot0IpA, slot0IpB, slot25IpA, slot25IpB);

	fixs_cch_cmdshelfmanager *snmpReader = new fixs_cch_cmdshelfmanager("");
	int snmpRes = snmpReader->isMaster(slot0IpA);
	if (snmpRes == 1) //master
			{
		masterSlot = 0;
	} else if (snmpRes == 2) //passive
			{
		masterSlot = 25;
	} else {
		snmpRes = snmpReader->isMaster(slot25IpB);
		if (snmpRes == 1) //master
				{
			masterSlot = 25;
		} else if (snmpRes == 2) //passive
				{
			masterSlot = 0;
		} else
			result = false;

	}

	bool res = true;
	if (result) {
		if (masterSlot == 0) {
			snmpReader->setIpAddress(slot0IpA);
			if (ushortToString(slot, slotStr) == false)
				return result;
			res = snmpReader->getActiveArea(slotStr, value);
			if (!res) {
				snmpReader->setIpAddress(slot0IpB);
				if (ushortToString(slot, slotStr) == false)
					return result;
				res = snmpReader->getActiveArea(slotStr, value);
			}
		} else if (masterSlot == 25) {
			snmpReader->setIpAddress(slot25IpA);
			if (ushortToString(slot, slotStr) == false)
				return result;
			res = snmpReader->getActiveArea(slotStr, value);
			if (!res) {
				snmpReader->setIpAddress(slot25IpB);
				if (ushortToString(slot, slotStr) == false)
					return result;
				res = snmpReader->getActiveArea(slotStr, value);
			}
		}

	} else {
		delete snmpReader;
		snmpReader = NULL;
		return SNMP_FAILURE;
	}

	if (snmpReader != NULL) {
		delete (snmpReader);
		snmpReader = NULL;
	}

	if (res == true)
		return 0;
	else
		return SNMP_FAILURE;
}


int ChangeActiveAreaToUpgradeFW(FWMUpgradeInfo &info, unsigned long umagazine,unsigned short argSlot)
{
	std::string line, choice;

	std::string prodNo=info.productno;

	int actArea = -1;
	int exitStatus = getActiveAreaFromBoard(umagazine, argSlot, actArea);
	if (exitStatus == 0) //successfully executed
	{
		if (((prodNo.compare(CMD_Util::SMX_FWTYPE_A_CXP) == 0) && (actArea == 1))|| ((prodNo.compare(CMD_Util::SMX_FWTYPE_B_CXP) == 0) && (actArea == 0)))
		{
				std::cout << endl;
				std::cout << "Do you want to change the Active area to upgrade the FW ? \nEnter y or n [default: n]\03:"<< std::flush;
				getline(cin, line);

				remove_copy(line.begin(), line.end(), back_inserter(choice), ' ');
				if ((choice == "n") || (choice == "N") || (choice == "")) {
					return ABORTED_BY_USER; //command aborted by users
				}
				if ((choice != "y") && (choice != "Y")) {
					std::cout << "Illegal value - enter y or n" << std::endl; //illegal value received
					return INCORRECT_RESPONSE;
				}
		}

	} else
		return exitStatus;

	return EXECUTED;
}

int main (int argc, char * argv[])
{
	TraceInOut inOut(__FUNCTION__);
	unsigned short uslot = 0;
	unsigned long umagazine = 0;
	string containerFile("");

	// Check parameters.  To be done.
	int opt_i = 0; // get upgrade status
	int opt_m = 0; // specifies MAG
	int opt_s = 0; // specifies SLOT
	int opt_f = 0; // Force option
	int opt_path =0;

	string fwpack("");
	string slotPar("");
	string magazinePar("");

	//-----------------------------------------------
	// C H E C K - N O D E - A R C H I T E C T U R E
	//-----------------------------------------------

	CMD_Util::EnvironmentType m_environment;
	m_environment = CMD_Util::getEnvironment();

	if (m_environment == CMD_Util::UNKNOWN)
	{
		std::cout << "No reply from Configuration Service" << std::endl;
		return NO_REPLY_FROM_CS;
	}
	else if ((m_environment != CMD_Util::MULTIPLECP_SMX) && (m_environment != CMD_Util::SINGLECP_SMX) && (m_environment != CMD_Util::SINGLECP_CBA) && (m_environment != CMD_Util::MULTIPLECP_CBA))
	{
		std::cout << "Illegal command in this system configuration" << std::endl;
		return ILLEGAL_COMMAND;
	}
	if ((m_environment == CMD_Util::SINGLECP_SMX) || (m_environment == CMD_Util::MULTIPLECP_SMX)){
		isSMX=true;
	}

	//check if is active node
	if (ACS_APGCC::is_active_node() == false){
		cout<<"Illegal command on passive node"<<endl;
		return UNABLECONNECT_PASSIVE;
	}

	string tarGz("");

	int res = -1;

		//------------------------
		//O P T I O N - C H E C K
		//------------------------

		int opt = 0;

		Get_opt getopt(argc, (char**) argv, CMD_OPTIONS);

		if (argc < 6) {
			return printUsage();	// Too few arguments
		}

		while (((opt = getopt(true)) != -1) && (opt != -2)) {
			switch (opt) {
			case 'i':
				if (opt_i)
					return printUsage();
				opt_i++;
				break;

			case 'm':
				if (opt_m)
					return printUsage();
				opt_m++;
				magazinePar = getopt.optarg;
				break;

			case 's':
				if (opt_s)
					return printUsage();
				opt_s++;
				slotPar = getopt.optarg;
				break;

			case 'f':
				if (opt_f)	//|| opt_path)
					return printUsage();
				opt_f++;
				break;

			default:	// Illegal option
				// Check if this is an known option
				if (getopt.optind > 1) {
					if ((opt_m == 1) && (opt_s == 1)) {
						if (opt_path)
							return printUsage();
						if (getopt.optarg) {
							fwpack = getopt.optarg;
							opt_path++;
							break;
						} else
							return printUsage();
					} else
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

	if(opt_i && opt_path)
	{
		return printUsage();
	}

	if((!opt_i) && (!opt_path))
	{
		return printUsage();
	}

	if( ( opt_m  != 1 ) || ( opt_s != 1) )
	{
		return printUsage();
	}

	if( opt_i )
	{
		if( opt_f )
			return printUsage();
	}

	//avoid to manage '-' first character
	if (fwpack[0] == '-') return printUsage();

	if (magazinePar[0] == '-') return printUsage();

	if (slotPar[0] == '-') return printUsage();

	if (opt == -2) return printUsage();

	if ( opt_m )
	{
		if (CMD_Util::stringToUlongMagazine (magazinePar, umagazine) == false)
		{
			std::cout << "Invalid magazine number" << std::endl;
			return INCORRECT_MAGAZINE;
		}
	}

	if ( opt_s )
	{
		if (CMD_Util::stringToUshort(slotPar, uslot) == false)
		{
			std::cout << "Invalid slot number" << std::endl;
			return INCORRECT_SLOT;
		}
	}

	
	//------------------------------------------
	// C H E C K - I F - A L L O W E D
	//------------------------------------------

	if (opt_path || opt_i)
	{
		hwc = ACS_CS_API::createHWCInstance();
		boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();

		int allowedResult = isOperationAllowed (umagazine, uslot);//return allowed in case the board is SCX or CMX or SMX
		if (allowedResult == EXECUTED)
		{
			if (opt_f == 0 && opt_path )
			{
				// Issue check printout
				std::cout << "Are you sure you want to upgrade the FW ? \nEnter y or n [default: n]\03:" << std::flush;
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
		}
		if (allowedResult == NO_REPLY_FROM_CS)
		{
			std::cout << "No reply from Configuration Service" << std::endl;
			if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
			if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
			return NO_REPLY_FROM_CS;
		}
		if (allowedResult == HARDWARE_NOT_PRESENT)
		{
			std::cout << "Hardware not present" << std::endl;
			if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
			if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
			return HARDWARE_NOT_PRESENT;
		}
		if (allowedResult == FUNC_BOARD_NOTALLOWED)
		{
			std::cout << "Not Allowed for this Functional Board Name" << std::endl;
			if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
			if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
			return FUNC_BOARD_NOTALLOWED;				//22
		}

	}

	if(isSMX){
		res = 0;

		containerFile = argv[5];
		std::string package("");

		package = containerFile.substr(containerFile.find_last_of("/") + 1,
				containerFile.length() - (containerFile.find_last_of("/") + 1));

		unsigned int dotCount = 0;

		//Check if containerFile contains any invalid characters
		for (unsigned int i = 1; i < package.length(); i++) {

			if (!((package[i] >= 'A' && package[i] <= 'Z')
					|| (package[i] >= 'a' && package[i] <= 'z')
					|| (package[i] >= '0' && package[i] <= '9')
					|| (package[i] == '-') || (package[i]) == '_'
					|| (package[i] == '.') || (package[i] == '\\')
					|| (package[i] == ':'))) {
				//std::cout<<" containerFile : "<<containerFile[i]<<std::endl;
				std::cout << "Wrong Package" << std::endl;
				return INCORRECT_PACKAGE;
			}

			if (package[i] == '.') {
				dotCount++;
			}
		}

		if (dotCount > 1) {
			std::cout << "Wrong Package" << std::endl;
			return INCORRECT_PACKAGE;
		}
		res = containerFileInfo(containerFile,umagazine,uslot);

		if (res == ABORTED_BY_USER)
		{
			return EXECUTED;
		}

	if (res == EXECUTED)
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] fwupgrade command successful", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	else{
		if (traceCmd.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Command failed", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
  }
	if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	if (hwc) ACS_CS_API::deleteHWCInstance(hwc);

	//------------------------------------------
	// C O N N E C T I O N - T O - S E R V E R
	//------------------------------------------

	ACS_DSD_Session ses;
	ACS_DSD_Client cl;
	string data("");
	int call_result = 0;
	const char * service_name = "FIXS_CCH_SERVER";
	const char * service_domain = "FIXS";

	call_result = cl.connect(ses, service_name, service_domain);

	if (call_result)
	{
		std::cout << "Unable to connect to the server" << std::endl;
		return UNABLECONNECT;
	}

	//---------------------------------
	// C O M M A N D - H A N D L I N G
	//---------------------------------
	res = -1;//dummy value

	if (opt_path)
	{
		if (traceCmd.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] upgrade FW", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		res = fwUpgrade(ses, fwpack, magazinePar, slotPar);
		if (res == EXECUTED) {
			if (traceCmd.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] upgrade FW terminated succesfully", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
		}
	}
	else if (opt_i)
	{
		if (traceCmd.ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] show FW upgrade status", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		res = fwStatus(ses, magazinePar, slotPar);
		if (res == EXECUTED) {
			if (traceCmd.ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FW status terminated successfully", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
		}
	}

	if(res != EXECUTED)
	{
	switch (res)
		{
			case INCORRECTUSAGE:					// 2
				std::cout << "Incorrect usage" << std::endl;
				printUsage();
				break;

			case DIRECTORYERR:						// 3
				std::cout << "Invalid file name" << std::endl;
				break;

			case NO_REPLY_FROM_CS:					// 7
				std::cout << "No reply from Configuration Service" << std::endl;
				break;

			case UPGRADE_ALREADY_ONGOING:			// 8
				std::cout << "Upgrade already ongoing" << std::endl;
				break;

			case SNMP_FAILURE:						// 13
				std::cout << "SNMP Failure" << std::endl;
				break;

			case INCORRECT_PACKAGE:				// 16
				std::cout << "Wrong Package" << std::endl;
				break;

			case HARDWARE_NOT_PRESENT:				// 14
				std::cout << "Hardware not present" << std::endl;
				break;

			case FUNC_BOARD_NOTALLOWED:				//22
				std::cout << "Not Allowed for this Functional Board Name" << std::endl;
				break;

			case DISK_FULL:				            // 24
				std::cout << "Disk quota exceeded" << std::endl;
				break;

			case FW_ALREADY_INSTALLED :				// 26
				std::cout << "FW already installed" << std::endl;
				break;

			case INCORRECT_SLOT:					// 31
				std::cout << "Invalid slot number" << std::endl;
				break;

			case ONLY_ALLOWED_FROM_IO_APG:			// 40
				std::cout << "Only allowed from IO APG" << std::endl;
				break;

			case ILLEGAL_COMMAND:					// 115
				std::cout << "Illegal command in this system configuration" << std::endl;
				break;

			case UNABLECONNECT:						// 117
				std::cout << "Unable to connect to the server" << std::endl;
				break;

			case NOT_ALLOWED_UPGRADE_ONGOING:
				std::cout << "Not allowed, upgrade ongoing" << std::endl;
				break;

			case NOT_ALLOWED_BACKUP_ONGOING:
				cout << "Not allowed, backup ongoing" <<endl;
				break;

			case NOT_ALLOWED_BY_CP: //28
				cout<<"Not allowed by CP"<<endl;
				break;

			case INTERNALERR:					    // 1
			default:
				std::cout << "Error when executing" << std::endl;
				break;
		}
	}
	if (traceCmd.ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Return value: %d", __FUNCTION__, __LINE__, res);
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
	return res;

}

bool readFwFromJson(boost::property_tree::ptree const& pt, FWMUpgradeInfo &info)
{
    bool res = false;
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();
    std::string type; // package for Area A, Area B
    int countp =0;
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
				countp++;
			}
			if(countp >=3)
			{
				break;
			}
		}
	}
	if(countp >= 3)
	{
		res =true;
		break;
	}
    }
    return res;
}

