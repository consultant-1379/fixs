#include <stdio.h>
#include <iostream>
#include <string.h>

#include "ACS_DSD_Session.h"
#include "ACS_DSD_Client.h"

#include "Get_opt.h"
#include "Client_Server_Interface.h"

#define CMD_OPTIONS  "i"

const char * const SCXFBN = "SCXB";
const char * const IPTFBN = "IPTB";
const char * const EPB1FBN = "EPB1";


/**********************************************************************/
/***************************** PROCEDURES *****************************/

int printUsage(const int errorCode=0, const char opt=0, const char* arg=0, bool usage=true);
int printErr(int exitCode);
int printList(const char response[], unsigned int respSize);

/**********************************************************************/

using namespace std;

int main (int argc, char * argv[]) {

	int returnCode = 0;

	ACS_DSD_Session session;
	ACS_DSD_Client client;
	int call_result = 0;
	const char * service_name = "FIXS_CCH_SERVER";
	const char * service_domain = "FIXS";

	int opt_i = 0;
	int opt_b = 0;
	//char *userFBN = {""};
	std::string userFBN = "";

	/*****************************************************/
	/****************** MANAGE OPTIONS *******************/
	/*****************************************************/

	Get_opt getopt(argc, (char**)argv, CMD_OPTIONS);


	if ( argc > 2 ) {
		return printUsage(INCORRECTUSAGE);// Too arguments
	}

	switch (argc) {
	case 1:
		opt_i++;
		break;

	case 2:
		opt_b++;

		{
			std::string tmp = "";
			tmp = argv[1];
			for (int z = 0 ; z < tmp.size() ; z++)
				userFBN += toupper(tmp[z]);
		}

		//cout << "userFBN: " << userFBN << " argv[1]: " << argv[1] << endl;

		if (
			strcmp(userFBN.c_str(), SCXFBN) == 0 ||
			strcmp(userFBN.c_str(), IPTFBN) == 0 ||
			strcmp(userFBN.c_str(), EPB1FBN) == 0
			)
		{
			//cout << " OK !! "<< endl;
		}
		else
			return printUsage(INCORRECTUSAGE);

		break;

	default:
		return printUsage(INCORRECTUSAGE);
		break;

	} //switch (argc)

	/*****************************************************/
	/****************** OPTION HANDLING******************/
	/*****************************************************/
	/*
	while ( ((opt = getopt()) != -1) && (opt != -2) )  {

		switch(opt)
		{
		case 'i':
			if( opt_i )
				return printUsage(INCORRECTUSAGE);
			opt_i++;

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
		}
	}

	if (opt_i < 1) {
		return printUsage(INCORRECTUSAGE);
	}
	else {
		if ( (argc != 2) ) {
			return printUsage(INCORRECTUSAGE);
		}
	}

	/**/

	/*****************************************************/
	/****************** CONNECT TO DSD *******************/
	/*****************************************************/

	//std::cout << "Connecting to service: " << service_name << " - domain: " << service_domain << std::endl;
	call_result = client.connect(session, service_name, service_domain);

	if (call_result) {
		//std::cout << "Connecting failed - Res: " << call_result << " - " << client.last_error_text() <<std::endl;
		return printErr(UNABLECONNECT);
	}

	/********************************************************/
	/******************* MANAGE OPTIONS *********************/
	/********************************************************/

	/*
	 * Send to server an operation TAG <option>:<parameters>:<parameters>
	 *
	 * Example
	 * 		"i:" 		--> showpkg
	 * 		"b:SCXB" 	--> showpkg SCXB
	 *
	 */

	if (opt_i == 1) {

		ssize_t bytes_sent = 0;

		string dsdOpt = "i:";
		bytes_sent = session.send(dsdOpt.c_str(), sizeof(dsdOpt) + 1 );

		//std::cout << "bytes_sent: " << bytes_sent << std::endl;
		if (bytes_sent < 0) {
			/*
			std::cout << "FAILED ! ... bytes_sent: " << bytes_sent << std::endl;
			std::cout << "last error: " << session.last_error() <<
					"last error text: " << session.last_error_text() << std::endl;
			*/
			return printErr(UNABLECONNECT);
		}

		char buffer[sizeof(int) + MAX_RESPONSE_SIZE] = {0};
		ssize_t bytes_received = 0;

		bytes_received = session.recv(buffer, sizeof(int) + MAX_RESPONSE_SIZE);

		if (bytes_received < 0) {
			/*
			std::cout << "FAILED ! ... bytes_received: " << bytes_received << std::endl;
			std::cout << "last error: " << session.last_error() <<
					"last error text: " << session.last_error_text() << std::endl;
			*/
			return printErr(INTERNALERR);
		}

		returnCode = printList(buffer, bytes_received);
		std::cout << endl;

		session.close();
	}
	else if (opt_b == 1)
	{
		ssize_t bytes_sent = 0;

		string dsdOpt = "b:" + userFBN;

		//std::cout << "dsdOpt: " << dsdOpt << std::endl;

		bytes_sent = session.send(dsdOpt.c_str(), sizeof(dsdOpt) + 1 );

		//std::cout << "bytes_sent: " << bytes_sent << std::endl;
		if (bytes_sent < 0) {
			/*
			std::cout << "FAILED ! ... bytes_sent: " << bytes_sent << std::endl;
			std::cout << "last error: " << session.last_error() <<
					"last error text: " << session.last_error_text() << std::endl;
			*/
			return printErr(UNABLECONNECT);
		}

		char buffer[sizeof(int) + MAX_RESPONSE_SIZE] = {0};
		ssize_t bytes_received = 0;

		bytes_received = session.recv(buffer, sizeof(int) + MAX_RESPONSE_SIZE);

		if (bytes_received < 0) {
			/*
			std::cout << "FAILED ! ... bytes_received: " << bytes_received << std::endl;
			std::cout << "last error: " << session.last_error() <<
					"last error text: " << session.last_error_text() << std::endl;
			*/
			return printErr(INTERNALERR);
		}

		returnCode = printList(buffer, bytes_received);
		std::cout << endl;

		session.close();
	}


	return printErr(returnCode);
}

//******************************************************************************
// Print error code and possible usage information
//******************************************************************************
int printUsage(const int errorCode, const char opt, const char* arg, bool usage)
{
	cerr<<"\nIncorrect usage"<<endl;
	if ( usage )
	{
		cerr << "Usage: showpkg [boardName]" << endl << endl;

		cerr << "COMMAND OPTIONS:" << endl;
		cerr << "   Board Name:		" << endl;
		cerr << "       SCXB 		Display only SCXB software packages" << endl;
		cerr << "       IPTB 		Display only IPTB software packages" << endl;
		cerr << "       EPB1 		Display only EPB1 software packages" << endl;

	}
	return errorCode;
}

/*
int printUsage(const int errorCode, const char opt, const char* arg, bool usage)
{
	cerr<<"\nIncorrect usage"<<endl;
	if ( usage )
	{
		cerr << "Usage: showpkg -i " << endl;

		cerr << "COMMAND OPTIONS:" << endl;
		cerr << "-i                 Display all packages and Load Modules installed on APG File" << endl;
		cerr << "                   System"<<endl;
		cerr << "                      This option is used to display all packages and relative" << endl;
		cerr << "                      Load Modules installed on APG file system." << endl;
	}
	return errorCode;
}
*/


int printErr(int exitCode)
{
	if (exitCode == 0) return 0;

	switch (exitCode)
	{
		case INCORRECTUSAGE:				// 2
			std::cout << "Incorrect usage" << std::endl;
			printUsage(INCORRECTUSAGE);
			break;

		case DIRECTORYERR:					// 3
			std::cout << "Invalid file path or file name" << std::endl;
			break;

		case PACKAGE_IN_USE:				// 4
			std::cout << "Upgrade Package in use" << std::endl;
			break;

		case PACKAGE_NOT_EXIST:				// 5
			std::cout << "Upgrade Package does not exist" << std::endl;
			break;

		case XMLERROR:						// 6
			std::cout << "XML Error" << std::endl;
			break;

		case NO_REPLY_FROM_CS:				// 7
			std::cout << "No reply from Configuration Service" << std::endl;
			break;

		case UPGRADE_ALREADY_ONGOING:		// 8
			std::cout << "Upgrade already ongoing" << std::endl;
			break;

		case SW_ALREADY_ON_BOARD:			// 9
			std::cout << "Specified SW already present on board" << std::endl;
			break;

		case WRONG_OPTION:					// 10
			std::cout << "Wrong option in this state" << std::endl;
			break;

		case WRONG_OPTION_PREPARE:			// 59
			std::cout << "PREPARE not allowed in this state" << std::endl;
			break;

		case WRONG_OPTION_ACTIVATE:			// 60
			std::cout << "ACTIVATE not allowed in this state" << std::endl;
			break;

		case WRONG_OPTION_COMMIT:			// 61
			std::cout << "COMMIT not allowed in this state" << std::endl;
			break;

		case WRONG_OPTION_CANCEL:			// 62
			std::cout << "CANCEL not allowed in this state" << std::endl;
			break;

		case PACKAGE_ALREADY_INSTALLED:		// 11
			std::cout << "Upgrade Package already installed" << std::endl;
			break;

		case UPGRADE_ACTIVATE_FAILED:		// 12
			std::cout << "Activate failure" << std::endl;
			break;

		case SNMP_FAILURE:					// 13
			std::cout << "SNMP Failure" << std::endl;
			break;

		case HARDWARE_NOT_PRESENT:			// 14
			std::cout << "Hardware not present" << std::endl;
			break;

		case PACKAGE_NOT_INSTALLED:			// 15
			std::cout << "Upgrade Package not installed" << std::endl;
			break;

		case INCORRECT_PACKAGE:				// 16
			std::cout << "Wrong Package" << std::endl;
			break;

		case NOT_ALLOWED_UPGRADE_ONGOING:	// 18
			std::cout << "Not allowed, upgrade ongoing" << std::endl;
			break;

		case FUNC_BOARD_NAME_ERROR:			// 19
			std::cout << "Functional Board Name Error" << std::endl;
			break;

		case PACKAGE_IS_DEFAULT:			// 20
			std::cout << "Specified Upgrade Package is set as default" << std::endl;
			break;

		case MAXIMUM_LOAD_MODULES_INSTALLED: // 21
			std::cout << "Maximum Number of Upgrade Package already installed for this FBN" << std::endl;
			break;

		case FUNC_BOARD_NOTALLOWED:				   // 22
			std::cout << "Option not allowed for this Functional Board Name" << std::endl;
			break;

		case SLOT_FBN_NOT_SAME:				        // 23
			std::cout << "All slots specified do not contain same type of board" << std::endl;
			break;

		case DISK_FULL:				                // 24
			std::cout << "Disk quota exceeded" << std::endl;
			break;

		case PACKAGE_NOT_APPLICABLE:				// 25
			std::cout << "Load Module not applicable for this Functional Board Name" << std::endl;
			break;

		case INCORRECT_SLOT:				       // 31
			std::cout << "Invalid slot number" << std::endl;
			break;

		case ONLY_ALLOWED_FROM_IO_APG:	           // 39
			std::cout << "Only allowed from IO APG" << std::endl;
			break;

		case ILLEGAL_COMMAND:				      // 115
			std::cout << "Illegal command in this system configuration" << std::endl;
			break;

		case UNABLECONNECT:					      // 117
			std::cout << "Unable to connect to the server" << std::endl;
			break;

		case INTERNALERR:					      // 1
		default:
			std::cout << "Error when executing" << std::endl;
			break;
	}
	return exitCode;
}

int printList(const char response[], unsigned int respSize)
{
	//TraceInOut inOut(__FUNCTION__);
	bool wrongSize = false;
	int nItem = 0;
	const int itemDim = sizeof(int);
	const int codeDim = 0; // sizeof(respCode);
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
		return UNABLECONNECT;
	}
	else if (len == 0)
	{
		;
	}
	else
	{
		char bufData[sizeof(int) + MAX_RESPONSE_SIZE] ={0};
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
