/*
 * cch_ipmifwprintcmd.cpp
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
#include "Client_Server_Interface.h"

using namespace std;

// Command Options
#define CMD_OPTIONS  "m:s:"

namespace {
	ACS_CS_API_HWC * hwc = 0;
	ACS_CS_API_BoardSearch * boardSearch = 0;
	ACS_TRA_trace traceCmd=ACS_TRA_trace("cch_ipmifwprintcmd");
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
int isOperationAllowed (unsigned long umagazine, unsigned short uslot, bool & isApboard);
int printUsage(const int errorCode=0, const char opt=0, const char* arg=0, bool usage=true);
int sendreceiveMessage_fw(ACS_DSD_Session & ses, const CCHCommandType cmdType, const string magazinePar, const string slotPar, char response[], unsigned int respSize);
int printList(const char response[], unsigned int respSize);
int ipmiDisplay(ACS_DSD_Session & ses, const string magazinePar,const string slotPar);
int ipmiDisplay(uint32_t umagazine, int32_t uslot);
int mapIpmiDataError( int apbm_error);
int isLocalMagazine(unsigned long umagazine, ACS_CS_API_CommonBasedArchitecture::ApgNumber apgNumber);

//******************************************************************************
// Print error code and possible usage information
//******************************************************************************
int printUsage(const int errorCode, const char opt, const char* arg, bool usage)
{
	UNUSED(opt);
	UNUSED(arg);

	TraceInOut inOut(__FUNCTION__);
	cerr<<"\nIncorrect usage"<<endl;
	if ( usage )
	{
		cerr << "Usage: ipmifwprint -m magazine -s slot" << endl;


		cerr << "COMMAND OPTIONS:" << endl;
		cerr << "-m magazine         This option specifies the magazine address where the IPMI" << endl;
		cerr << "                    module to be upgraded is located " << endl;
		cerr << "-s slot             This option specifies the slot number where the IPMI module"<< endl;
		cerr <<	"                    to be upgraded is located " << endl;
	}
	return errorCode;
}

int isOperationAllowed (unsigned long umagazine,unsigned short uslot, bool & isApboard)
{
	TraceInOut inOut(__FUNCTION__);
	int retValue = EXECUTED;

	//get APG number
	ACS_CS_API_CommonBasedArchitecture::ApgNumber apgNumber;
	ACS_CS_API_NetworkElement::getApgNumber(apgNumber);

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
				case ACS_CS_API_HWC_NS::FBN_CMXB:
				case ACS_CS_API_HWC_NS::FBN_EvoET:
					retValue = EXECUTED;
					break;
				case ACS_CS_API_HWC_NS::FBN_APUB:
				case ACS_CS_API_HWC_NS::FBN_GEA:
				{
					isApboard = true;

					SysID sysNo = 0;
					returnValue = hwc->getSysNo(sysNo,boardId);
					if (returnValue == ACS_CS_API_NS::Result_Success)
					{
						if (sysNo == apgNumber)
						{
							retValue = EXECUTED;
						}
						else
						{
							if (traceCmd.ACS_TRA_ON()) { //trace
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Mismatch between local APG number and board system number.", __FUNCTION__, __LINE__);
								traceCmd.ACS_TRA_event(1, tmpStr);
							}
							retValue = ONLY_ALLOWED_FROM_LOCAL_MAGAZINE;
						}
					}
					else
					{
						if (traceCmd.ACS_TRA_ON()) { //trace
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Cannot retrieve SysNo from CS", __FUNCTION__, __LINE__);
							traceCmd.ACS_TRA_event(1, tmpStr);
						}
						retValue = NO_REPLY_FROM_CS;
					}
				}
				break;

				case ACS_CS_API_HWC_NS::FBN_SCBRP:
					isApboard = true;	//it means that the command will be handled by APBM
					retValue = isLocalMagazine(umagazine,apgNumber);
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

int sendreceiveMessage_fw(ACS_DSD_Session & ses, const CCHCommandType cmdType, const string magazinePar, const string slotPar, char response_fw[], unsigned int respSize_fw)
{
	TraceInOut inOut(__FUNCTION__);

	//cch_upgrade_request_pfm* request_fw = new (std::nothrow) cch_upgrade_request_pfm();
	cch_ipmiupg_request* request_fw = new (std::nothrow) cch_ipmiupg_request();

	if (!request_fw) return INTERNALERR;

	memset(request_fw, 0, sizeof(cch_ipmiupg_request));
	request_fw->cmdType = cmdType; //set CCH command
	//strcpy(request_fw->data, fwpkg.c_str());
	strcpy(request_fw->magAddr, magazinePar.c_str());
	strcpy(request_fw->slot, slotPar.c_str());
	strcpy(request_fw->ipSCXMaster, "");
	strcpy(request_fw->slotMaster, "");
	strcpy(request_fw->valueToSet, "");
	//strcpy(request_fw->pfmType, pfmTypePar.c_str());
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

int ipmiDisplay(ACS_DSD_Session & ses, const string magazinePar,const string slotPar)
{
	//std::cout << " ipmiDisplay - In" << std::endl;
	char response[sizeof(int) + MAX_RESPONSE_SIZE] = {0};
	unsigned int respSize = sizeof(int) + MAX_RESPONSE_SIZE;
//	std::cout << " fwDisplay - mag: " << magazinePar.c_str() << " pfmModulePar: "<< pfmModulePar.c_str() << " pfmTypePar: "<< pfmTypePar.c_str()<<  std::endl;
	int code = sendreceiveMessage_fw(ses, cmdIPMIDisplay, magazinePar, slotPar, response, respSize);
	if (code == UNABLECONNECT)
	{
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] communication failed. \n Last error code: %d \n Last error text: %s",__FUNCTION__, __LINE__,ses.last_error(),ses.last_error_text());
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	else if (code == EXECUTED)
	{
		printList(response, respSize);
	}

//	std::cout << " fwDisplay - Out" << std::endl;

	return code;
}

int ipmiDisplay(uint32_t umagazine, int32_t uslot){

	TraceInOut inOut(__FUNCTION__);

	acs_apbm_api api_sess;
	uint8_t ipmifw_type;
	char product_number [25] = {0};
	char revision [8] = {0};

	int call_result = api_sess.get_ipmifw_data(umagazine, uslot, ipmifw_type, product_number, revision);


	int code = mapIpmiDataError(call_result);
	if (code == UNABLECONNECT)
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] communication failed. \n Last apbm_api error code: %d ",__FUNCTION__, __LINE__, call_result);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
	}
	else if (code == SNMP_FAILURE){
		printf("SNMP Failure\n");
	}
	else if (code == EXECUTED)
	{

		printf("\nIPMI FW INFORMATION\n\n");
		//printf("Result: \n" "magazine: 0x%08X\n" "slot: %d\n" "firmware type: %d\n"	 "product number: %s\n""revision: %s\n",umagazine, uslot, ipmifw_type, product_number, revision);
		printf("RUNNING IPMI FW TYPE: %s\n",(ipmifw_type) ? "FB" : "UPG");
		printf("RUNNING IPMI FW VERSION: ");
//		for(size_t i=0;i < 25 && product_number[i]!= ' '; i++)
//				printf("%c", product_number[i]);
//
//		printf(" %s\n\n", revision);

		//ROBUSTNESS CHANGE FOR COM CLI
		std::string total = (std::string)product_number + " " + (std::string) revision;
		printf("%s\n\n",total.c_str());
	}
	return code;
}

int main (int argc, char * argv[])
{
	TraceInOut inOut(__FUNCTION__);

	int res = -1;

	// Check parameters.  To be done.
	int opt_m = 0; // specifies MAG
	int opt_s = 0; // specifies SLOT
	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	string dataPar("");
	string magazinePar("");
	string slotPar("");

	/////////////////////////////////////////

	////Check Architecture////
	CMD_Util::EnvironmentType m_environment;
	m_environment = CMD_Util::getEnvironment();

	//	cout << "m_environment: " << m_environment<< endl;

	if (m_environment == CMD_Util::UNKNOWN)
	{
		std::cout << "No reply from Configuration Service" << std::endl;
		return NO_REPLY_FROM_CS;
	}
	else if (m_environment == CMD_Util::VIRTUALIZED)
	{
		std::cout << "Illegal command in this system configuration" << std::endl;
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

	if ( argc > 5 ) {
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
				return printUsage(INCORRECTUSAGE);
		} // end switch
	} // end while


	if ( getopt.optind < argc )
	{
		return printUsage(INCORRECTUSAGE);// Too many arguments
	}

	if ( opt_m + opt_s != 2)
	{ // Invalid mix of arguments
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		return printUsage(INCORRECTUSAGE);
	}


	//avoid to manage '-' first character
	if (magazinePar[0] == '-') return printUsage(INCORRECTUSAGE);

	if (slotPar[0] == '-') return printUsage(INCORRECTUSAGE);

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

	hwc = ACS_CS_API::createHWCInstance();
	boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
	bool isApboard = false;
	res = isOperationAllowed(umagazine, uslot, isApboard);
	if(res == EXECUTED)
	{

		/////////////////////////////////////////
		////////////CONNECTION TO DSD////////////
		/////////////////////////////////////////

		ACS_DSD_Session ses;
		ACS_DSD_Client cl;
		string data("");
		int call_result = 0;
		if (!isApboard)
		{
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
		////////////COMMAND HANDLING////////////

		if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] install IPMI FW Image", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}

		res = (!isApboard) ? ipmiDisplay(ses, magazinePar, slotPar) : ipmiDisplay(umagazine, uslot) ;
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
	if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	if (hwc) ACS_CS_API::deleteHWCInstance(hwc);

	if(res != EXECUTED)
	{
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

			case SNMP_FAILURE:					// 13
				std::cout << "SNMP Failure" << std::endl;
				break;

			case HARDWARE_NOT_PRESENT:			// 14
				std::cout << "Hardware not present" << std::endl;
				break;

			case WAIT_FOR_UPGRADE_ONGOING:		// 17
				std::cout << "Display command inhibited when firmware upgrade ongoing" << std::endl;
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

			case NOT_ALLOWED_BY_CP: //28
				cout<<"Not allowed by CP"<<endl;
				break;

			case ONLY_ALLOWED_FROM_LOCAL_MAGAZINE: //123
				cout<<"Local magazine number expected in this system configuration"<<endl;
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

int mapIpmiDataError( int apbm_error){

	switch (apbm_error)
	{

	case acs_apbm::ERR_NO_ERRORS:
		return EXECUTED;

	case acs_apbm::ERR_IPMIUPG_INVALID_SLOT:
		return INCORRECT_SLOT;
	case acs_apbm::ERR_IPMIUPG_INVALID_MAGAZINE:
		return INCORRECT_MAGAZINE;
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
	case acs_apbm::ERR_IPMIUPG_UPGRADE_ALREADY_ONGOING:
		return WAIT_FOR_UPGRADE_ONGOING; 
	default:
		return INTERNALERR;

	}
}


int isLocalMagazine(unsigned long umagazine, ACS_CS_API_CommonBasedArchitecture::ApgNumber apgNumber)
{

	TraceInOut inOut(__FUNCTION__);
	int retValue = EXECUTED;

	ACS_CS_API_HWC* hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch* searchAP = ACS_CS_API_HWC::createBoardSearchInstance();
		if (searchAP)
		{
			ACS_CS_API_IdList boardList;
			searchAP->setMagazine(umagazine);
			searchAP->setFBN(ACS_CS_API_HWC_NS::FBN_APUB);
			searchAP->setSysNo(SysID(apgNumber));

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, searchAP);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					retValue = ONLY_ALLOWED_FROM_LOCAL_MAGAZINE;
				}
			}
			else
			{
				if (traceCmd.ACS_TRA_ON()) { //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Cannot retrieve SysNo from CS", __FUNCTION__, __LINE__);
					traceCmd.ACS_TRA_event(1, tmpStr);
				}
				retValue = NO_REPLY_FROM_CS;
			}

			ACS_CS_API_HWC::deleteBoardSearchInstance(searchAP);
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}

	return retValue;

}
