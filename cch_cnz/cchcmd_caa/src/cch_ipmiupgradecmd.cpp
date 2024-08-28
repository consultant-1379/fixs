/*
 * cch_ipmiupgradecmd.cpp
 *
 *  Created on: May 2, 2012
 *      Author: xlalkak
 */

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

#include "Get_opt.h"
#include "cmd_Utils.h"
#include "acs_apbm_api.h"
#include "acs_apgcc_adminoperation.h"
#include "Client_Server_Interface.h"



using namespace std;

// Command Options
#define CMD_OPTIONS  "m:s:c:i"

namespace {
	ACS_CS_API_HWC * hwc = 0;
	ACS_CS_API_BoardSearch * boardSearch = 0;
	ACS_TRA_trace traceCmd=ACS_TRA_trace("cch_ipmiupgradecmd");
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

// Global functions
int isOperationAllowed (unsigned long umagazine,unsigned short uslot);
int isOperationAllowed (unsigned long umagazine, unsigned short uslot, bool & isApboard);
int printUsage(const int errorCode=0, const char opt=0, const char* arg=0, bool usage=true);
void print_ipmiupgrade_error( int errorCode);
int sendreceiveMessage_fw(ACS_DSD_Session & ses, const CCHCommandType cmdType, const string fwpkg, const string magazinePar, const string slotPar, char response[], unsigned int respSize);
int printList(const char response[], unsigned int respSize);
int ipmiInstall(ACS_DSD_Session & ses, const string pkg, const string magazinePar,const string slotPar);
int ipmiInstall(const string pkg, const unsigned long,const unsigned short uslot);
int ipmiInstallViaCom(const string pkg, uint8_t port);
int ipmiStatus(ACS_DSD_Session & ses, const string magazinePar,const string slotPar);
int ipmiStatus(unsigned long umagazine, unsigned short uslot);
int map_to_ipmiupgrade_error( int apbm_error);
int map_snmp_error( int snmp_error);



//******************************************************************************
// Print error code and possible usage information
//******************************************************************************
int printUsage(const int errorCode, const char opt, const char* arg, bool usage)
{
	TraceInOut inOut(__FUNCTION__);
	UNUSED(opt);
	UNUSED(arg);
	cerr<<"\nIncorrect usage"<<endl;
	if ( usage )
	{
		cerr << "Usage: ipmiupgexec -m magazine -s slot filename" << endl;
		cerr << "Usage: ipmiupgexec -c comport filename" << endl;
		cerr << "Usage: ipmiupgexec -i -m magazine -s slot" << endl;


		cerr << "COMMAND OPTIONS:" << endl;
		cerr << "-c comport          This option allows the command to be executed locally using the" << endl;
		cerr << "                    specified comport." << endl;
		cerr << "                    Use one of the following values: COM2, COM3" << endl;
		cerr << "-i                  Upgrade status. " << endl;
		cerr << "                    This option displays IPMI upgrade status, possible status:" << endl;
		cerr << "                    OK, NOT OK or ONGOING" << endl;
		cerr << "-m magazine         This option specifies the magazine address where the IPMI" << endl;
		cerr << "                    module to be upgraded is located " << endl;
		cerr << "-s slot             This option specifies the slot number where the IPMI module"<< endl;
		cerr <<	"                    to be upgraded is located " << endl;
	}
	return errorCode;
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
					case ACS_CS_API_HWC_NS::FBN_EPB1:
						retValue = EXECUTED;
					break;
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

int isOperationAllowed (unsigned long umagazine,unsigned short uslot, bool & isApboard)
{
	TraceInOut inOut(__FUNCTION__);
	int retValue = EXECUTED;
	isApboard = false;
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
					case ACS_CS_API_HWC_NS::FBN_EPB1:
					case CMD_Util::FBN_EVOET:  //TODO link to new CS lib
					case CMD_Util::FBN_CMXB:
						retValue = EXECUTED;
						break;
					case ACS_CS_API_HWC_NS::FBN_SCBRP:
					case ACS_CS_API_HWC_NS::FBN_APUB:
					case ACS_CS_API_HWC_NS::FBN_GEA:
						retValue = EXECUTED;
						isApboard = true;
					break;
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

int sendreceiveMessage_fw(ACS_DSD_Session & ses, const CCHCommandType cmdType, const string fwpkg, const string magazinePar, const string slotPar, char response_fw[], unsigned int respSize_fw)
{
	TraceInOut inOut(__FUNCTION__);

	//cch_upgrade_request_pfm* request_fw = new (std::nothrow) cch_upgrade_request_pfm();
	cch_ipmiupg_request* request_fw = new (std::nothrow) cch_ipmiupg_request();

	if (!request_fw) return INTERNALERR;

	memset(request_fw, 0, sizeof(cch_ipmiupg_request));
	request_fw->cmdType = cmdType; //set CCH command
	strcpy(request_fw->valueToSet, fwpkg.c_str());
	strcpy(request_fw->magAddr, magazinePar.c_str());
	strcpy(request_fw->slot, slotPar.c_str());
	strcpy(request_fw->ipSCXMaster, "");
	strcpy(request_fw->slotMaster, "");
	unsigned int reqSize_fw = sizeof(cch_ipmiupg_request);//set size

//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " sending cmd: " << request_fw->cmdType <<" magazine: " << request_fw->magazine << " size: " << reqSize_fw <<std::endl;

	ssize_t bytes_sent = 0;
	bytes_sent = ses.send(reinterpret_cast<void*>(request_fw), reqSize_fw);

	if (bytes_sent < 0)
	{//Sending failed
		cout << "Communication error" << endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		delete request_fw;
		return UNABLECONNECT; // TODO VERIFY EXIT CODE
	}
	else
	{
		//sending ok
		ssize_t bytes_received = 0;
		bytes_received = ses.recv(response_fw, respSize_fw);

		if (bytes_received < 0)
		{//receiving failed
//			std::cout << "DBG RCV NOT OK: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
			delete request_fw;
			return UNABLECONNECT ;//TO VERIFY
		}
		else
		{
			//receiving ok
			//std::cout << "DBG RCV OK: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
			respCode code_fw = (respCode)response_fw[0];
			int cd_fw = static_cast<int>(code_fw);
//			std::cout << "DBG RCV OK: " << __FUNCTION__ << "@" << __LINE__ << "return Code: "<< cd_fw<< std::endl;
			delete request_fw;
			return cd_fw;
		}
	}
	delete request_fw;
	return EXECUTED;
}


int printList(const char response[], unsigned int respSize)
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
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] The size of the received data is wrong. nItem = %d. len = %d", __FUNCTION__, __LINE__, nItem, len);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}

		return UNABLECONNECT;
	}
	else if (len == 0)
	{
		;
	}
	else
	{
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

int ipmiInstall(ACS_DSD_Session & ses, const string pkg, const string magazinePar,const string slotPar)
{
//	//cout << "DBG - fwpkg: " << pkg.c_str() << "magazinePar: " << magazinePar.c_str() << "slotPar: " << slotPar.c_str()<< endl;

	TraceInOut inOut(__FUNCTION__);

	std::string pathParameter = CMD_Util::getNbiSwPath(CMD_Util::swPackageFw)+ "/" + pkg;

	char response[MAX_RESPONSE_SIZE] = {0};
	unsigned int respSize = sizeof(int) + MAX_RESPONSE_SIZE;;

	int code = sendreceiveMessage_fw(ses, cmdIPMIUpgrade, pathParameter, magazinePar, slotPar, response, respSize);
	if (code == UNABLECONNECT)
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] communication failed. Last Windows Error: %d",__FUNCTION__, __LINE__, ses.last_error());
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	return code;
}


int ipmiInstall( const string pkg, const unsigned long magazine, unsigned short slot)
{
	TraceInOut inOut(__FUNCTION__);

    acs_apbm_api api_sess;

    std::string ipmipkg_name = CMD_Util::getNbiSwPath(CMD_Util::swPackageFw)+ "/" + pkg;
   // char ipmipkg_name [ACS_APBM_IPMIPKG_NAME_MAX_SIZE] = {0};

    int call_result = api_sess.ipmifw_upgrade( magazine, (int32_t) slot, ipmipkg_name.c_str());

	int code = map_to_ipmiupgrade_error(call_result);

	if (code == UNABLECONNECT)
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] communication failed. \n Last apbm_api error code: %d ",__FUNCTION__, __LINE__, call_result);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}

	return code;
}

int ipmiInstallViaCom(const string pkg, uint8_t port)
{
	TraceInOut inOut(__FUNCTION__);

    acs_apbm_api api_sess;
    uint32_t magazine = 0;
    int32_t slot = 0;
    std::string ipmipkg_name = CMD_Util::getNbiSwPath(CMD_Util::swPackageFw)+ "/" + pkg;
   // char ipmipkg_name [ACS_APBM_IPMIPKG_NAME_MAX_SIZE] = {0};

    int call_result = api_sess.ipmifw_upgrade( magazine, (int32_t) slot, ipmipkg_name.c_str(), port);

	int code = map_to_ipmiupgrade_error(call_result);

	if (code == UNABLECONNECT)
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] communication failed. \n Last apbm_api error code: %d ",__FUNCTION__, __LINE__, call_result);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}

	return code;
}

int ipmiStatus(ACS_DSD_Session & ses, const string magazinePar, const string slotPar)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	char response[sizeof(int) + MAX_RESPONSE_SIZE] = {0};
	unsigned int respSize = sizeof(int) + MAX_RESPONSE_SIZE;
	int code = sendreceiveMessage_fw(ses, cmdIPMIStatus, "", magazinePar, slotPar, response, respSize);
	if (code == UNABLECONNECT)
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] communication failed. \n Last error code: %d \n Last error text: %s",__FUNCTION__, __LINE__,ses.last_error(),ses.last_error_text());
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	else if (code == EXECUTED)
	{
		//std::cout << "DBG OK: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		printList(response, respSize);
	}

	return code;
}


int ipmiStatus(unsigned long umagazine, unsigned short uslot)
{
	TraceInOut inOut(__FUNCTION__);

	uint8_t ipmifw_status;
	char lastupg_date [ACS_APBM_DATA_TIME_SIZE];
	acs_apbm_api api_sess;


	int call_result = api_sess.get_ipmi_firmware_status(umagazine, uslot, ipmifw_status, lastupg_date);
	//printf("IPMIUPGRADESTATUS: %d\n",ipmifw_status);
	int code = map_to_ipmiupgrade_error(call_result);

	if (code == UNABLECONNECT)
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] communication failed. \n Last apbm_api error code: %d ",__FUNCTION__, __LINE__, call_result);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	else if (code == EXECUTED)
	{

		printf("\n%-20s\n", "IPMI FIRMWARE UPGRADE PROGRESS REPORT");
		//std::cout << "DBG OK: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		if(strcmp(lastupg_date,"") == 0 )
			strcpy(lastupg_date, "-");
		switch (ipmifw_status)
		{
			case CMD_Util::IPMI_UPGRADE_STATUS_UNDEFINED:
			case CMD_Util::IPMI_UPGRADE_STATUS_READY:
				printf("\nLAST UPGRADE TIME: %-20s\n",lastupg_date);
				printf("\n%-20s\n\n","IPMI FW UPGRADE STATUS: OK");
				break;
			case CMD_Util::IPMI_UPGRADE_STATUS_ONGOING :
				printf("%-20s\n\n", "IPMI FW UPGRADE STATUS: ONGOING");
				break;
			default:
				printf("\nLAST UPGRADE TIME: %-20s\n", lastupg_date);
				printf("\n%-20s\n\n", "IPMI FW UPGRADE STATUS: NOT OK");
				//code = map_snmp_error(ipmifw_status);
				break;
			break;

		}
	}

	return code;
}


int main (int argc, char * argv[])
{
	TraceInOut inOut(__FUNCTION__);

	int res = -1;
	// Check parameters.  To be done.
    int opt_i = 0; // ipmi status
	int opt_m = 0; // specifies MAG
	int opt_s = 0; // specifies SLOT
	int opt_c = 0; // specifies COMPORT
	int opt_path = 0; //ipmi package name
	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	string ipmipack("");
	string dataPar("");
	string magazinePar("");
	string slotPar("");
	string comPortPar("");
	///////////Check Architecture////////////////
	                       
	CMD_Util::EnvironmentType m_environment;
	m_environment = CMD_Util::getEnvironment();
	
	if (m_environment == CMD_Util::UNKNOWN)
	{
		std::cout << "No reply from Configuration Service" << std::endl;
		return NO_REPLY_FROM_CS;
	}
	else if (m_environment == CMD_Util::VIRTUALIZED)
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Illegal command ", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		cout<<"Illegal command in this system configuration"<<endl;
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

		return ILLEGAL_COMMAND;
	}               
		
	//////////end check architecture/////////////		
	//check if is active node
	if (ACS_APGCC::is_active_node() == false){
		cout<<"Illegal command on passive node"<<endl;
		return UNABLECONNECT_PASSIVE;
	}

	int  opt;
	Get_opt getopt (argc, (char**)argv, CMD_OPTIONS);

	if ( argc < 2 ) {
		return printUsage(INCORRECTUSAGE);// Too few arguments
	}

	while ( ((opt = getopt(true)) != -1) && (opt != -2) )  {
		switch(opt)
		{
		case 'i':
			if( opt_i )
				return printUsage(INCORRECTUSAGE);
			opt_i++;
			break;

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

		case 'c':
			if( opt_c )
				return printUsage(INCORRECTUSAGE);
			opt_c++;
			comPortPar = getopt.optarg;
			break;


		default:// Illegal option
			// Check if this is an known option
			if ( getopt.optind > 1 )
			{
				if(opt_i == 0)
				{
					if(opt_path)
						return printUsage(INCORRECTUSAGE);
					if(getopt.optarg)
					{
						ipmipack = getopt.optarg;
						opt_path++;
						break;
					}
					else
						return printUsage(INCORRECTUSAGE);
				}
				else
					return printUsage(INCORRECTUSAGE);
			}
			return printUsage(INCORRECTUSAGE);
		} // end switch
	} // end while


	if ( getopt.optind < argc )
	{
		return printUsage(INCORRECTUSAGE);// Too many arguments
	}

	if ( opt_i + opt_path != 1)
	{ // Invalid mix of arguments
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		return printUsage(INCORRECTUSAGE);
	}

	if(opt_c && (opt_m || opt_s))
		return printUsage(INCORRECTUSAGE);
	if(opt_c && opt_i)
		return printUsage(INCORRECTUSAGE);
	if(!opt_c)
	{
		if((opt_m + opt_s) != 2)
			return printUsage(INCORRECTUSAGE);
	}

	//avoid to manage '-' first caracter
	if (ipmipack[0] == '-') return printUsage(INCORRECTUSAGE);

	if (magazinePar[0] == '-') return printUsage(INCORRECTUSAGE);

	if (slotPar[0] == '-') return printUsage(INCORRECTUSAGE);

	if (comPortPar[0] == '-') return printUsage(INCORRECTUSAGE);

	if (opt == -2) return printUsage(INCORRECTUSAGE);

	if ( opt_m )
	{
		if (CMD_Util::stringToUlongMagazine (magazinePar, umagazine) == false)
		{
			std::cout << "Invalid magazine number" << std::endl;	//30
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

	if (opt_c && opt_path)
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] install IPMI FW Image", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
		}

			////////////COMMAND HANDLING////////////
		uint8_t port = 0;
		if( comPortPar == "COM2")
			port = acs_apbm::COM2;
		else if (comPortPar == "COM3")
			port = acs_apbm::COM3;
		else
			return printUsage(INCORRECTUSAGE);

		res = ipmiInstallViaCom(ipmipack, port);


		if (res == EXECUTED)
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IPMI FW installation terminated successfully", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
			}

		}
		else if (res == INCORRECTUSAGE)
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] install IPMI FW Image Error, Incorrect parameter usage", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
			printUsage();
		}
		if(res != EXECUTED )
		print_ipmiupgrade_error(res);
		return res;
	}
	
        if ((m_environment == CMD_Util::SINGLECP_DMX) || (m_environment == CMD_Util::MULTIPLECP_DMX) )
        {
                 if (traceCmd.ACS_TRA_ON()) { //trace
                        char tmpStr[512] = {0};
                        snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Illegal command in non common based architecture", __FUNCTION__, __LINE__);
                        traceCmd.ACS_TRA_event(1, tmpStr);
                     }
                cout<<"Illegal command in this system configuration"<<endl;
                //std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

                return ILLEGAL_COMMAND;
        }
        else
        {
                 if (traceCmd.ACS_TRA_ON()) { //trace
                     char tmpStr[512] = {0};
                     snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] command in valid common based architecture", __FUNCTION__, __LINE__);
                     traceCmd.ACS_TRA_event(1, tmpStr);
                   }
        }


	hwc = ACS_CS_API::createHWCInstance();
	boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
	bool isApboard;
	res = isOperationAllowed(umagazine,uslot, isApboard);
	if(res != EXECUTED ) {

		if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Return value = %d", __FUNCTION__, __LINE__, res);
				traceCmd.ACS_TRA_event(1, tmpStr);
		}
		print_ipmiupgrade_error(res);
		return res;
	}

	ACS_DSD_Session ses;

	ACS_DSD_Client cl;
	string data("");
	int call_result = 0;

	if (!isApboard)
	{
		/////////////////////////////////////////
		////////////CONNECTION TO DSD////////////
		/////////////////////////////////////////
		const char * service_name = "FIXS_CCH_SERVER";
		const char * service_domain = "FIXS";

		call_result = cl.connect(ses, service_name, service_domain);

		if (call_result) {
			//std::cout << "Connecting failed - Res: " << call_result << " - " << client.last_error_text() <<std::endl;
			//return printUsage(UNABLECONNECT);
			std::cout << "Unable to connect to the server" << std::endl;
			return UNABLECONNECT;
		}
	}

	if (opt_path)
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] install IPMI FW Image", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
		}

			////////////COMMAND HANDLING////////////


		res = (!isApboard)?  ipmiInstall(ses, ipmipack, magazinePar, slotPar): ipmiInstall(ipmipack, umagazine, uslot);
		if (res == EXECUTED)
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IPMI FW installation terminated successfully", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
			}
		}
		else if (res == INCORRECTUSAGE)
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] install IPMI FW Image Error, Incorrect parameter usage", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
			printUsage();
		}
	}



	else if (opt_i)
		{
			res = (!isApboard)? ipmiStatus(ses, magazinePar, slotPar) : ipmiStatus(umagazine, uslot);

			if (res == EXECUTED) {
				if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IPMI FW upgrade status", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
				}
			}
		}

	if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	if (hwc) ACS_CS_API::deleteHWCInstance(hwc);

	if(res != EXECUTED)
	{
		print_ipmiupgrade_error(res);
	}

	if (traceCmd.ACS_TRA_ON()) { //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Return value = %d", __FUNCTION__, __LINE__, res);
		traceCmd.ACS_TRA_event(1, tmpStr);
	}

	return res;
}

void print_ipmiupgrade_error( int errorCode){

	switch (errorCode)
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
			std::cout << "Not allowed, upgrade ongoing" << std::endl;
			break;

		case ILLEGAL_COMMAND:				// 115
			cout<<"Illegal command in this system configuration"<<endl;
			break;

		case UNABLECONNECT:					// 117
			cout<<"Unable to connect to the server"<<endl;
			break;

		case INCORRECT_MAGAZINE:
			cout<<"Invalid magazine number"<<endl;
			break;

		case INCORRECT_SLOT:
			cout<<"Invalid slot number"<<endl;
			break;

		case FUNC_BOARD_NOTALLOWED: //22
			cout<<"Not allowed for this Functional Board Name"<<endl;
			break;

		case NEIGHBOR_SCX_NOT_PRESENT:
			cout<<"Not allowed, Neighbor SCX is not defined or not reachable"<<endl;
			break;

		case NOT_ALLOWED_BACKUP_ONGOING: //75
			cout << "Not allowed, backup ongoing" <<endl;
			break;

		case NOT_ALLOWED_BY_CP: //28
			cout << "Not allowed by CP" <<endl;
			break;

		case INTERNALERR:					// 1
		default:
			std::cout << "Error when executing" << std::endl;
			break;
	}
}


int map_to_ipmiupgrade_error( int apbm_error){

	switch (apbm_error)
	{

//	case acs_apbm::----
//	return SNMP_FAILURE;
//
//	case acs_apbm::
//	return INCORRECT_PACKAGE;
//
//	case acs_apbm::
//	return WAIT_FOR_UPGRADE_ONGOING;
//
//	case acs_apbm::
//	return UPGRADE_ALREADY_ONGOING;
//
//	case acs_apbm::
//	return DIRECTORYERR;

	case acs_apbm::ERR_NO_ERRORS:
		return EXECUTED;

	case acs_apbm::ERR_IPMIUPG_INVALID_SLOT:
		return INCORRECT_SLOT;
	case acs_apbm::ERR_IPMIUPG_INVALID_MAGAZINE:
		return INCORRECT_MAGAZINE;
	case acs_apbm::ERR_IPMIUPG_INVALID_FILE_NAME:
		return INCORRECT_PACKAGE;
	case acs_apbm::ERR_IPMIUPG_UPGRADE_ALREADY_ONGOING:
		return UPGRADE_ALREADY_ONGOING;
	case acs_apbm::ERR_IPMIUPG_SNMP_ERROR:
		return SNMP_FAILURE;
	case acs_apbm::ERR_IPMIUPG_FUNC_BOARD_NOT_ALLOWED:
		return FUNC_BOARD_NOTALLOWED;
	case acs_apbm::ERR_INIT_CONNECTION_FAILED:
		return UNABLECONNECT;
	case acs_apbm::ERR_CS_BOARD_NOT_FOUND:
		return NO_REPLY_FROM_CS;
	case acs_apbm::ERR_SNMP_BOARD_NOT_FOUND:
		return HARDWARE_NOT_PRESENT;
	default:
		return INTERNALERR;

	}
}

int map_snmp_error( int snmp_error){

	switch (snmp_error)
	{

	case CMD_Util::IPMI_UPGRADE_STATUS_NO_SUCH_BOARD :
		return HARDWARE_NOT_PRESENT;
	case CMD_Util::IPMI_UPGRADE_STATUS_OPEN_FILE_ERROR :
		return INCORRECT_PACKAGE;
	case CMD_Util::IPMI_UPGRADE_STATUS_FAIL :
	case CMD_Util::IPMI_UPGRADE_STATUS_TFTP_ERROR :
	case CMD_Util::IPMI_UPGRADE_STATUS_RESET_FAIL_IN_FB :
	case CMD_Util::IPMI_UPGRADE_STATUS_RESET_FAIL_IN_UPG :
	case CMD_Util::IPMI_UPGRADE_STATUS_TFTP_INTERNAL_ERROR :
	case CMD_Util::IPMI_UPGRADE_STATUS_CHECKSUM_ERROR :
	case CMD_Util::IPMI_UPGRADE_STATUS_NOT_STARTED :
		return INTERNALERR;
	case CMD_Util::IPMI_UPGRADE_STATUS_NOT_SUPPORTED :
		return FUNC_BOARD_NOTALLOWED;
	default:
		return INTERNALERR;

	}
}



