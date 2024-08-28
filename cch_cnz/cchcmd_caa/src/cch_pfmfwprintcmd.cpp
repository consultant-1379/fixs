/*
 * cch_pfmfwprintcmd.cpp
 *
 *  Created on: Apr 23, 2012
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
#include "Client_Server_Interface.h"

using namespace std;

// Command Options
#define CMD_OPTIONS  "m:t:v:r:"
static CMD_Util::EnvironmentType m_environment = CMD_Util::UNKNOWN;

namespace {
	ACS_TRA_trace traceCmd=ACS_TRA_trace("cch_pfmfwprintcmd");
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
int  printUsage(const int errorCode=0, const char opt=0, const char* arg=0, bool usage=true);
int sendreceiveMessage_fw(ACS_DSD_Session & ses, const CCHCommandType cmdType, const string fwpkg, const string magazinePar, const string pfmModulePar, const string pfmTypePar, char response[], unsigned int respSize);
int printList(const char response[], unsigned int respSize);
int fwDisplay(ACS_DSD_Session & ses, const string fwpkg, const string magazinePar,const string pfmModulePar, const string pfmTypePar);
std::string getDefaultFolder (CMD_Util::EnvironmentType environment);

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
		cerr << "Usage: pfmfwprint -m magazine [ -t PFMmodtype|-v PFMtype -r PFMmodinst]" << endl;

		cerr << "COMMAND OPTIONS:" << endl;
		cerr << "-m magazine         This function specifies the magazine address where the PFM" << endl;
		cerr << "                    module to be upgraded is located " << endl;
		cerr << "-t PFMmodtype       PFM module type"<< endl;
		cerr <<	"                    This function allows the definition of power or fan type. " << endl;
		cerr << "                    Use one of the following values: power, fan" << endl;
		cerr << "-v PFMtype          PFM SW upgrade type"<< endl;
		cerr <<	"                    This function allows the definition of power and fan"<<endl;
		cerr <<	"                    SW upgrade type. " << endl;
		cerr << "                    Use one of the following values: typeA, typeB" << endl;		
		cerr << "-r PFMmodinst       PFM module instance" << endl;
        cerr << "                    This function allows the definition of the module type. " << endl;
		cerr << "                    Use one of the following values: upper, lower" << endl;
	}
	return errorCode;
}

int sendreceiveMessage_fw(ACS_DSD_Session & ses, const CCHCommandType cmdType, const string fwpkg, const string magazinePar, const string pfmModulePar, const string pfmTypePar, char response_fw[], unsigned int respSize_fw)
{
	TraceInOut inOut(__FUNCTION__);

	cch_upgrade_request_pfm* request_fw = new (std::nothrow) cch_upgrade_request_pfm();

	if (!request_fw) return INTERNALERR;

	memset(request_fw, 0, sizeof(cch_upgrade_request_pfm));
	request_fw->cmdType = cmdType; //set CCH command
	strcpy(request_fw->data, fwpkg.c_str());
	strcpy(request_fw->magazine, magazinePar.c_str());
	strcpy(request_fw->pfmModule, pfmModulePar.c_str());
	strcpy(request_fw->pfmType, pfmTypePar.c_str());
	unsigned int reqSize_fw = sizeof(cch_upgrade_request_pfm);//set size

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

int fwDisplay(ACS_DSD_Session & ses, const string fwpkg, const string magazinePar,const string pfmModulePar, const string pfmTypePar)
{
	UNUSED(fwpkg);

	char response[sizeof(int) + MAX_RESPONSE_SIZE] = {0};
	unsigned int respSize = sizeof(int) + MAX_RESPONSE_SIZE;
//	std::cout << " fwDisplay - mag: " << magazinePar.c_str() << " pfmModulePar: "<< pfmModulePar.c_str() << " pfmTypePar: "<< pfmTypePar.c_str()<<  std::endl;
	int code = sendreceiveMessage_fw(ses, cmdFwDisplay, "", magazinePar, pfmModulePar, pfmTypePar, response, respSize);
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

	return code;
}



int main (int argc, char * argv[])
{
	TraceInOut inOut(__FUNCTION__);

	// Check parameters.  To be done.

	int opt_m = 0; // specifies MAG
	int opt_t = 0; // specifies PFMType
	int opt_v = 0; // specifies PFMupgType
	int opt_r = 0; // specifies Module


	string fwpack("");
	string dataPar("");
	string magazinePar("");
	string pfmTypePar("");
	string pfmModulePar("");
	
	////Check Architecture////
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
		case 'm':
			if( opt_m )
				return printUsage(INCORRECTUSAGE);
			opt_m++;
			magazinePar = getopt.optarg;
			break;

		case 't':
			if( opt_t )
				return printUsage(INCORRECTUSAGE);
			opt_t++;
			pfmTypePar = getopt.optarg;
			break;
			
		case 'v':
			if( opt_v )
				return printUsage(INCORRECTUSAGE);
			opt_v++;
			pfmTypePar = getopt.optarg;
			break;

		case 'r':
			if( opt_r )
				return printUsage(INCORRECTUSAGE);
			opt_r++;
			pfmModulePar = getopt.optarg;
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



		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		// Check invalid mix of arguments
		if (opt_m != 1)
		{
			return printUsage(INCORRECTUSAGE);
		}
		if((m_environment != CMD_Util::SINGLECP_SMX) && (m_environment != CMD_Util::MULTIPLECP_SMX))
		{
			if(opt_v == 1)
			{
				if((opt_r == 1) && (opt_t == 1))
					return printUsage(INCORRECTUSAGE);
				else if(opt_r == 1)
				{
					std::cout << "llegal option in this system configuration" << std::endl;
					return ILLEGAL_OPTION_SYS_CONFIG;
				}
				else
					return printUsage(INCORRECTUSAGE);
			}

			if((opt_t == 1)||(opt_r == 1))
			{
				if(opt_t + opt_r != 2)
				{
					return printUsage(INCORRECTUSAGE);
				}
			}
		}
		else
		{
			if(opt_t == 1)
			{
				if((opt_r == 1) && (opt_v == 1))
					return printUsage(INCORRECTUSAGE);
				else if(opt_r == 1)
				{
					std::cout << "llegal option in this system configuration" << std::endl;
					return ILLEGAL_OPTION_SYS_CONFIG;
				}
				else
					return printUsage(INCORRECTUSAGE);
			}

			if((opt_v == 1)||(opt_r == 1))
			{
				if(opt_v + opt_r != 2)
				{
					return printUsage(INCORRECTUSAGE);
				}
			}
		}
		
	//avoid to manage '-' first caracter


	if (magazinePar[0] == '-') return printUsage(INCORRECTUSAGE);

	if (pfmTypePar[0] == '-') return printUsage(INCORRECTUSAGE);

	if (pfmModulePar[0] == '-') return printUsage(INCORRECTUSAGE);

	if (opt == -2) return printUsage(INCORRECTUSAGE);

	if ( opt_m )
	{
		unsigned long umagazine = 0;
		if (CMD_Util::stringToUlongMagazine (magazinePar, umagazine) == false)
		{
			std::cout << "Invalid magazine number" << std::endl;	//30
			return INCORRECT_MAGAZINE;
		}
	}

	if(opt_t)
	{
		std::string tmp = pfmTypePar;
		CMD_Util::stringToUpper(tmp);
		if((tmp.compare("FAN") != 0) && (tmp.compare("POWER") != 0))
		{
			std::cout << "Invalid PFM module type" << std::endl;	//30
			return PFM_INCORRECT_TYPE; //73
		}
	}

	if(opt_v)
	{
		std::string tmp = pfmTypePar;
		CMD_Util::stringToUpper(tmp);
		if((tmp.compare("TYPEA") != 0) && (tmp.compare("TYPEB") != 0))
		{
			std::cout << "Invalid PFM SW upgrade type" << std::endl;	//30
			return PFM_INCORRECT_TYPE; //73
		}
	}

	if(opt_r)
	{
		std::string tmp = pfmModulePar;
		CMD_Util::stringToUpper(tmp);
		if((tmp.compare("UPPER") != 0) && (tmp.compare("LOWER") != 0))
		{
			std::cout << "Invalid PFM module instance" << std::endl;	//30
			return PFM_INCORRECT_MODULE; //74
		}
	}

	/////////////////////////////////////////
	////////////CONNECTION TO DSD////////////
	/////////////////////////////////////////

	ACS_DSD_Session ses;
	ACS_DSD_Client cl;
	string data("");
	int call_result = 0;
	const char * service_name = "FIXS_CCH_SERVER";
	const char * service_domain = "FIXS";

	call_result = cl.connect(ses, service_name, service_domain);

	if (call_result) {
		//std::cout << "Connecting failed - Res: " << call_result << " - " << client.last_error_text() <<std::endl;
		//return printUsage(UNABLECONNECT);
		std::cout << "Unable to connect to the server" << std::endl;
		return UNABLECONNECT;
	}

	/////////////////////////////////////////


	////////////COMMAND HANDLING////////////
	int res = -1;

	if (traceCmd.ACS_TRA_ON()) { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] display FW Image", __FUNCTION__, __LINE__);
			traceCmd.ACS_TRA_event(1, tmpStr);
		}
		res = fwDisplay(ses, fwpack, magazinePar, pfmModulePar, pfmTypePar);
		if (res == EXECUTED)
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FW display terminated successfully", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
		}
		else
		{
			if (traceCmd.ACS_TRA_ON()) { //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] display FW Image Error", __FUNCTION__, __LINE__);
				traceCmd.ACS_TRA_event(1, tmpStr);
			}
			//printUsage();
		}

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

			case ILLEGAL_COMMAND:				// 115
				cout<<"Illegal command in this system configuration"<<endl;
				break;

			case UNABLECONNECT:					// 117
				cout<<"Unable to connect to the server"<<endl;
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

