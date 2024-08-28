#include "FIXS_CCH_CmdHandler.h"

using namespace std;

const char * CSERVICE_NAME = "FIXS_CCH_SERVER";
const char * CSERVICE_DOMAIN = "FIXS";

/*============================================================================
	ROUTINE: FMS_CPF_CmdHandler
 ============================================================================ */
FIXS_CCH_CmdHandler::FIXS_CCH_CmdHandler()
{
	// Initialize the svc state flag
	tExit = false;

	_trace = new ACS_TRA_trace("FIXS_CCH_CmdHandler");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
FIXS_CCH_CmdHandler::~FIXS_CCH_CmdHandler()
{
	int result = 0;
	/************ DSD DISCONNECT *************/
	//	cout << __FUNCTION__ << "@" << __LINE__ << "Try to close DSD session..." << endl;
	result = session.close();

	//	cout << __FUNCTION__ << "@" << __LINE__ << "Try to unregister from DSD..." << endl;
	result = server.unregister();

	//	cout << __FUNCTION__ << "@" << __LINE__ << "Try to close DSD server..." << endl;
	result = server.close();

	delete (_trace);

	/************ DSD DISCONNECT *************/
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_CCH_CmdHandler::open(void *args)
{
	int result;

	UNUSED(args);

	tExit = false;

	result = activate(THR_DETACHED);
	//sleep(1);

	if(result != 0)
	{
		cout << "\n ####################################################### " << endl;
		cout << " 	FIXS_CCH_CmdHandler - DSD - Activation failed 			" << endl;
		cout << " ####################################################### \n" << endl;
	}

  	return result;
}

/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_CCH_CmdHandler::svc()
{
	int result = -1;

	int dsdSteps = 1;

	FIXS_CCH_EventHandle shutdownEvent = 0;

	//add shutdown event !
	shutdownEvent = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

	//Configure DSD Handles
	int handle_count = 0;
	acs_dsd::HANDLE* handles = NULL;

	bool openSuccess = false;

	bool publishSuccess = false;

	while(!tExit)
	{
		switch (dsdSteps)
		{
		case 1: //init DSD

			if (!openSuccess) result = server.open(acs_dsd::SERVICE_MODE_INET_SOCKET);
			else result = 0; //go to publish

			if (result < 0)
			{
				cout << __FUNCTION__ << "@" << __LINE__ << "server.open failed !" << endl;


				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  server.open failed ! ");
					FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}

				//go for wait shutdown
				dsdSteps = 3;
				break;
			}
			else
			{
				//Just for tracing ;)
				if (result > 0)
				{
					cout << __FUNCTION__ << "@" << __LINE__ << "Open with a warning !!" << endl;


					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  Open with a warning !! ");
						FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}
				}
				else
				{
					cout << __FUNCTION__ << "@" << __LINE__ << "Open successfully !!" << endl;


					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  Open successfully !! ");
						FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}
					openSuccess = true;
				}

				//Try to publish
				FIXS_CCH_logging->Write("FIXS_CCH_CmdHandler - try to publish !!!",LOG_LEVEL_DEBUG);

				result = server.publish("FIXS_CCH_SERVER", "FIXS"); //Name + App Domain
				if (result < 0) {
					cout << __FUNCTION__ << "@" << __LINE__ << "server.publish failed !" << endl;

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  Server.publish failed !! ");
						FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}

					//go for wait shutdown
					dsdSteps = 3;
					break;
				}
				else
				{
					//Connection done !
					dsdSteps = 2;

					//Just for tracing ;)
					if (result > 0) {
						cout << __FUNCTION__ << "@" << __LINE__ << "Publish return a warning !!" << endl;

						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  Publish return a warning !! ");
							FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
							if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
						}
					}
					else
					{
						cout << __FUNCTION__ << "@" << __LINE__ << "Published successfully !!" << endl;

						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  Published successfully !! ");
							FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
							if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
						}

						publishSuccess = true;
					}
				}
			}//else open result
			break;

		case 2: //Retrieve DSD Handles

			cout << __FUNCTION__ << "@" << __LINE__ << "Start step 2 !" << endl;
			//retrieve handles number

			if (handle_count == 0)
			{
				result = server.get_handles(0, handle_count);
				if (result <= 0)
				{
					cout << __FUNCTION__ << "@" << __LINE__ << "Handles number failed !" << endl;

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  Handles number failed !! ");
						FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}
				}
			}

			cout << __FUNCTION__ << "@" << __LINE__ << "handle_count: " << handle_count << endl;

			handle_count++; //increase array len for the shutdown.
			handles = new acs_dsd::HANDLE[handle_count];

			result = server.get_handles(handles, handle_count);
			if (result < 0)
			{
				delete[] handles;
				handles = NULL;

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  Handles failed !! ");
					FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}
			}
			else
			{

				cout << __FUNCTION__ << "@" << __LINE__ << " handles[handle_count]: " << handles[handle_count] << endl;

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler -  Get Handles successfully, handle_count[%d] !! ",handle_count);
					FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}

			}

			dsdSteps = 3;
			break;

		case 3: //Wait for handles

			// handle events
			int waitRes = -1; //

			FIXS_CCH_EventHandle HandlesArray[33];
			int validIndex = 0;

			if (handles != NULL) {
				for (int i=0; i < handle_count; i++)
				{
					if (handles[i] != -1)
					{
						HandlesArray[validIndex++] = handles[i];
					}
				}
			}

			HandlesArray[validIndex++] = shutdownEvent;

			if (! tExit)
			{
				waitRes = FIXS_CCH_Event::WaitForEvents(validIndex, HandlesArray, 10000);
				//cout << __FUNCTION__ << "@" << __LINE__ << "waitRes: " << waitRes << endl;
			}
			else
			{
				cout << __FUNCTION__ << "@" << __LINE__ << " EXIT Signaled "<< endl;
				break;
			}

			switch (waitRes)
			{
				case FIXS_CCH_Event_RC_TIMEOUT:  // Time out, means startup supervision completed as expected

					if (!openSuccess || !publishSuccess)
					{
						dsdSteps = 1;
					}
					else if (handles == NULL)
						dsdSteps = 2;

					break;

				default:

					if (waitRes >= 0 && waitRes < validIndex-1 )
					{
						//DSD Handle
						FIXS_CCH_Event::ResetEvent(HandlesArray[waitRes]);

						//This operation lock the cursor !
						result = server.accept(session);
						cout << __FUNCTION__ << "@" << __LINE__ << " Accept done! " << waitRes << endl;

						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler - Server Accept asked !! ");
							FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
							if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
						}

						if (result < 0)
						{
							cout << __FUNCTION__ << "@" << __LINE__ << " server.accept failed !" << endl;

							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler - Server Accept failed !! ");
								FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
								if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
							}
						}
						else
						{
							cout << __FUNCTION__ << "@" << __LINE__ << " server.accept success, wait data..." << endl;

							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler - Server Accept success, wait data... ");
								FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
								if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
							}

							char buffer[MAX_RESPONSE_SIZE] = {0};
							ssize_t bytes_received = 0;
							bytes_received = session.recv(buffer, ACS_DSD_ARRAY_SIZE(buffer));

							if (bytes_received < 0)
							{
								cout << __FUNCTION__ << "@" << __LINE__ << " data failed..." << endl;

								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_CCH_CmdHandler - Server data failed... ");
									FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
									if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
								}
								result = session.close();
							}
							else
							{
								//handle request
								fxProcessRequest(&session,buffer,bytes_received);
							}
						}
					}

					if (waitRes == validIndex-1)
					{ // Shutdown event received
						tExit = true;
					}
					break;
			} //switch (waitRes)
			break;
		} //switch (dsdSteps)
	} //while(!tExit)

//	for(int i = 0 ; i < handle_count - 1 ; i++)
//		FIXS_CCH_Event::CloseEvent(handles[i]);

	delete[] handles;

//	cout << __FUNCTION__ << "@" << __LINE__ << "Disconnect from DSD " << endl;
	session.close();
	server.unregister();
	server.close();

	return result;
}


std::string FIXS_CCH_CmdHandler::getOpt(char *buf, int bufSize)
{
	std::string tmp;
	tmp.assign(buf,0, bufSize);

	std::cout << tmp.c_str() << std::endl;

	if (tmp.find(":") != std::string::npos)
		tmp = tmp.substr(0, tmp.find(":"));
	else
		tmp = "";

	return tmp;
}


int FIXS_CCH_CmdHandler::splitParams(char *buf, std::list<std::string> &lstParams)
{
	char * par;
	par = strtok(buf, ":");

	lstParams.clear();

	while (par != NULL) {
		lstParams.push_back(par);
		std::cout << par << std::endl;
		par = strtok(NULL, ":");
	}

	std::list<std::string>::iterator it;
	for (it = lstParams.begin() ; it != lstParams.end() ; it++)
	{
		std::cout << it->c_str() << std::endl;
	}

	return lstParams.size();
}


/**
* processRequest
* process an incoming request
*
*/
void FIXS_CCH_CmdHandler::fxProcessRequest(ACS_DSD_Session* session, char* request, int requestSize)
{
	cch_upgrade_request_pfm* cmdRequest = 0;
	cmdRequest = (cch_upgrade_request_pfm*)request;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] In - request cmd:[%d]", __FUNCTION__, __LINE__,cmdRequest->cmdType);
		FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
	}

			switch(cmdRequest->cmdType)
			{
				case cmdFwUpgrade:
					std::cout<<"FW UPGRADE requested"<<std::endl;
					FIXS_CCH_logging->Write("FIXS_CCH_CmdHandler - FW UPGRADE requested",LOG_LEVEL_DEBUG);
					fxProcessFwUpgrade(session, request, requestSize);//pfmupgexec -u FILENAME -m magazine -t PFMtype -r module
					break;

				case cmdFwDisplay:
					std::cout<<"FW DISPLAY requested"<<std::endl;
					FIXS_CCH_logging->Write("FIXS_CCH_CmdHandler - FW DISPLAY requested",LOG_LEVEL_DEBUG);
					fxProcessFwDisplay(session, request, requestSize);//pfmfwprint -m magazine 
					break;

				case cmdFwStatus:
					std::cout<<"FW STATUS requested"<<std::endl;
					FIXS_CCH_logging->Write("FIXS_CCH_CmdHandler - FW STATUS requested",LOG_LEVEL_DEBUG);
					fxProcessFwStatus(session, request, requestSize);//pfmupgexec -m magazine -i
					break;

				case cmdRemoteFwStatus:
					std::cout<<"REMOTE FW STATUS requested"<<std::endl;
					fxProcessRemoteFwStatus(session, request, requestSize); //remotefwprint -m magazine -s
					break;

				case cmdRemoteFwUpgrade:
					std::cout<<"REMOTE FW UPGRADE requested"<<std::endl;
					fxProcessRemoteFwUpg(session, request, requestSize); //remotefwupgexec -m magazine -s filename
					break;

				case cmdIPMIUpgrade:
					std::cout<<"IPMI UPGRADE requested"<<std::endl;
					fxProcessIpmiUpg(session, request, requestSize); //ipmiupgexec -m magazine -s filename
					break;

				case cmdIPMIDisplay:
					std::cout<<"IPMI DISPLAY requested"<<std::endl;
					fxProcessIpmiDisplay(session, request, requestSize); //ipmifwprint -m magazine -s
					break;

				case cmdIPMIStatus:
					std::cout<<"IPMI STATUS requested"<<std::endl;
					fxProcessIpmiStatus(session, request, requestSize); //ipmiupgexec -i -m magazine -s
					break;


//				case cmdIPMI_UPG:
//					std::cout<<"IPMI_UPG requested"<<std::endl;
//					fxProcessIpmi_Upg(session, request, requestSize);
//					break;
//
//				default:
//					std::cout << "it is not a valid command..." << std::endl;
//					FIXS_CCH_logging->Write("FIXS_CCH_CmdHandler - it is not a valid command.. ",LOG_LEVEL_DEBUG);
//					fxProcessUnknown(session, request, requestSize);
			}
//		}
//		else if (FIXS_CCH_SoftwareMap::getInstance()->refresh())
//		{
//			//before executing the following commands CCH double checks the software usage by SCX boards
//			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " cmd: " << cmd << std::endl;
//			switch(cmd)
//			{
//				case cmdPrepare:
//					std::cout<<"PREPARE requested"<<std::endl;
//					fxProcessPrepare(session, request, requestSize);//upgpkg -p -m magazine -s slot [-v cxc_version]
//					break;
//
//				case cmdCommit:
//					std::cout<<"COMMIT requested"<<std::endl;
//					fxProcessCommit(session, request, requestSize);//upgpkg -c -m magazine -s slot [-t]
//					break;
//
//				case cmdInstall:
//					std::cout<<"INSTALL requested"<<std::endl;
//					fxProcessSimpleInstall(session, request, requestSize); //upgpkg -n xxxxxx.tar.gz -.tar -.zip
//					break;
//
//				case cmdRemove:
//					std::cout<<"REMOVE requested"<<std::endl;
//					fxProcessRemove(session, request, requestSize); //upgpkg -d swrpack|all
//					break;
//
//				case cmdList:
//					std::cout<<"LIST requested"<<std::endl;
//					fxProcessList(session, request, requestSize); //upgpkg -i [-b fbn]
//					break;
//
//				case cmdSetDefault:
//					std::cout<<"SET DEFAULT PACKAGE requested"<<std::endl;
//					fxProcessSetDefault(session, request, requestSize); //upgpkg -o defaultPackage -b fbn
//					break;
//
//				default:
//					std::cout<<"Error: unknown request!"<<std::endl;
//					fxProcessUnknown(session, request, requestSize);
//			}
//		}
//		else
//		{
//			if (ACS_TRA_ON(traceServer))
//			{ //trace
//				char tmpChar[512] = {0};
//				_snprintf(tmpChar, sizeof(tmpChar) - 1, "Cannot refresh data from CS");
//				ACS_TRA_event(&traceServer, tmpChar);
//			}
//			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
//			result = NO_REPLY_FROM_CS;
//			sendResponse(session, result);
//		}
//	}
//	else
//	{
//		if (ACS_TRA_ON(traceServer))
//		{ //trace
//				char tmpChar[512] = {0};
//				_snprintf(tmpChar, sizeof(tmpChar) - 1, "No common based Architecture");
//				ACS_TRA_event(&traceServer, tmpChar);
//		}
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
//		result = ILLEGAL_COMMAND;
//		sendResponse(session, result);
//	}
//
//	if (ACS_TRA_ON(traceServer))
//	{ //trace
//		char tmpChar[512] = {0};
//		_snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] Out", __FUNCTION__, __LINE__);
//		ACS_TRA_event(&traceServer, tmpChar);
//	}
}

//----------------------------------------------------
// fxProcessFwUpgrade
// pfmupgexec -m magazine -t PFMmodtype -r module
// pfmupgexec -m magazine -v PFMtype -r module
//----------------------------------------------------
void FIXS_CCH_CmdHandler::fxProcessFwUpgrade(ACS_DSD_Session* session, const char* request, unsigned int requestSize)
{
	UNUSED(requestSize);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	respCode result = EXECUTED;
	cch_upgrade_request_pfm* fwInstall = 0;
	fwInstall = (cch_upgrade_request_pfm*)request;
	std::string path = fwInstall->data;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "[FWINSTALL] path = " << path.c_str() << std::endl;

	/////////////////////////////
	char completePathFwPar[MAX_PATH] = {0};
	char pfmMagazinePar[ITEM_DATA] = {0};
	char pfmTypeIndexPar[ITEM_DATA] = {0};
	char pfmInstanceIndexPar[ITEM_DATA] = {0};
    char pfmSlotPar[ITEM_DATA] = {0};
//	char pfmtype[ITEM_DATA] = {0};
	std::string app("");

	snprintf(completePathFwPar, sizeof(completePathFwPar) - 1, "%s", (fwInstall->data)) ; //like 0
	snprintf(pfmMagazinePar, sizeof(pfmMagazinePar) - 1, "%s", (fwInstall->magazine)) ; //like c:\path\file.F
    snprintf(pfmTypeIndexPar, sizeof(pfmTypeIndexPar) - 1, "%s", (fwInstall->pfmType)) ; //like 0
	snprintf(pfmInstanceIndexPar, sizeof(pfmInstanceIndexPar) - 1, "%s", (fwInstall->pfmModule)) ; //like 0

	std::string pfmTypeIndexStr = pfmTypeIndexPar;
	std::string pfmInstanceIndexStr = pfmInstanceIndexPar;

	ACS_APGCC::toUpper(pfmTypeIndexStr);
	ACS_APGCC::toUpper(pfmInstanceIndexStr);
			
	if (pfmTypeIndexStr != "FAN" && pfmTypeIndexStr != "POWER" && pfmTypeIndexStr != "TYPEA" && pfmTypeIndexStr != "TYPEB") 
	{
		result = INCORRECTUSAGE;
		sendResponse(session, result);
		return;
	}
	else if ((pfmTypeIndexStr == "FAN")|| (pfmTypeIndexStr == "TYPEA"))
	{
		snprintf(pfmTypeIndexPar, sizeof(pfmTypeIndexPar) - 1, "0");

	}
	else if ((pfmTypeIndexStr == "POWER")||(pfmTypeIndexStr == "TYPEB"))
	{
		snprintf(pfmTypeIndexPar, sizeof(pfmTypeIndexPar) - 1, "1");

	}	


	if (pfmInstanceIndexStr.compare("UPPER") == 0)
	{
		snprintf(pfmSlotPar, sizeof(pfmSlotPar) - 1, "25");
		snprintf(pfmInstanceIndexPar, sizeof(pfmInstanceIndexPar) - 1, "1");
	}
	else if (pfmInstanceIndexStr.compare("LOWER") == 0)
	{
		snprintf(pfmInstanceIndexPar, sizeof(pfmInstanceIndexPar) - 1, "0");
		snprintf(pfmSlotPar, sizeof(pfmSlotPar) - 1, "0");
	}
	else
	{
		result = INCORRECTUSAGE;
		sendResponse(session, result);
		return;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "[FWINSTALL] data = " << path << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "[FWINSTALL] magazine = " << pfmMagazinePar << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "[FWINSTALL] pfmTypePar = " << pfmTypeIndexPar << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "[FWINSTALL] pfmModulePar = " << pfmInstanceIndexPar << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "[FWINSTALL] pfmslotPar = " << pfmSlotPar << std::endl;

	if (CCH_Util::PathFileExists(path.c_str()))
	{
			int error = FIXS_CCH_UpgradeManager::getInstance()->pfmUpgrade(path,pfmMagazinePar, pfmTypeIndexPar, pfmInstanceIndexPar, pfmSlotPar);
			result = static_cast<respCode> (error);
			sendResponse(session, result);
	}
	else
	{
		//sendResponse(session, INCORRECT_PACKAGE);
		sendResponse(session, DIRECTORYERR);
	}
}

//----------------------------------------------------
// fxProcessFwStatus
// upgpkg -m magazine -s
//----------------------------------------------------

void FIXS_CCH_CmdHandler::fxProcessFwStatus(ACS_DSD_Session* session, const char* request, unsigned int requestSize)
{
	UNUSED(requestSize);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] In ", __FUNCTION__, __LINE__);
		FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
	}

	respCode result = EXECUTED;
	cch_upgrade_request_pfm* status = 0;
	status = (cch_upgrade_request_pfm*)request;

	int error = FIXS_CCH_UpgradeManager::getInstance()->pfmStatus(status->magazine, status->pfmType, status->pfmModule);
	result = static_cast<respCode> (error);

	if (result)
	{
		//error
		FIXS_CCH_logging->Write("FIXS_CCH_CmdHandler - fxProcessFwStatus error...",LOG_LEVEL_ERROR);
		sendResponse(session, result);
	}
	else
	{
		int bufSize = sizeof(int) + MAX_RESPONSE_SIZE;
		char buf[sizeof(int) + MAX_RESPONSE_SIZE] = {0};

		int respSize = FIXS_CCH_UpgradeManager::getInstance()->writePfmStatus(buf,bufSize);
		if (respSize <= 0)
		{
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Internal error during the status process.");
				FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);

			}
			sendResponse(session, INTERNALERR);
		}
		else
		{
			FIXS_CCH_logging->Write("FIXS_CCH_CmdHandler - fxProcessFwStatus success...",LOG_LEVEL_DEBUG);
			sendMessage(session,EXECUTED,respSize,buf);

		}
	}
}

int FIXS_CCH_CmdHandler::printList(const char response[], unsigned int respSize)
{

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
		std::string aLine("");
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

//----------------------------------------------------
// fxProcessFwDisplay
// pfmfwprint -m magazine 
//----------------------------------------------------
void FIXS_CCH_CmdHandler::fxProcessFwDisplay(ACS_DSD_Session* session, const char* request, unsigned int requestSize)
{
	UNUSED(requestSize);
	respCode result = EXECUTED;
	cch_upgrade_request_pfm* fwDisplay = 0;
	fwDisplay = (cch_upgrade_request_pfm*)request;

	std::cout << "Type : " << fwDisplay->pfmType << "  Module : " << fwDisplay->pfmModule << std::endl;
	int error = FIXS_CCH_UpgradeManager::getInstance()->pfmDisplay(fwDisplay->magazine, fwDisplay->pfmType, fwDisplay->pfmModule);
	result = static_cast<respCode> (error);
	if (result)
	{
		//error
		sendResponse(session, result);
	}
	else
	{

		int bufSize = sizeof(int) + MAX_RESPONSE_SIZE;
		char buf[sizeof(int) + MAX_RESPONSE_SIZE] = {0};
		int env=-1;
		int respSize = 0;
		env = FIXS_CCH_DiskHandler::getEnvironment();
		if((env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX))
		{
			respSize = FIXS_CCH_UpgradeManager::getInstance()->writePfmSwDisplay(buf,bufSize);
		}
		else
		{
		 respSize = FIXS_CCH_UpgradeManager::getInstance()->writePfmDisplay(buf,bufSize);
		}
		 
		if (respSize <= 0)
		{
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Internal error in fxProcessFwDisplay");
				FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
			sendResponse(session, INTERNALERR);
		}
		else
		{
			sendMessage(session,EXECUTED,respSize,buf);
		}
	}
}


//---------------------------------------------------
// sendResponse
// send a response through the dsd session
//---------------------------------------------------
bool FIXS_CCH_CmdHandler::sendResponse(ACS_DSD_Session* session, respCode code)
{
	bool retCode = true;
	void* response = 0;
	unsigned int respSize = sizeof(respCode);

	response = malloc(respSize);
	memcpy(response, &code, respSize);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " response " << response << std::endl;

	ssize_t bytes_sent = 0;
	bytes_sent = session->send(response, respSize);
	if (bytes_sent < 0)
	{
		//failed
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_CmdHandler - Impossible to send the Response vs the command client error code:[%d]",session->last_error());
		FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);

		retCode = false;

	}
	else
	{
		//send OK
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_CmdHandler - send the Response success !!");
		FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
	}

	free(response);
	session->close();

	return retCode;
}


//---------------------------------------------------
// sendResponse
// send a response through the dsd session
//---------------------------------------------------
bool FIXS_CCH_CmdHandler::sendMessage(ACS_DSD_Session* ses,respCode code,unsigned int len, const char* data)
{
	char buf[MAX_RESPONSE_SIZE] = {0};
	unsigned int msgSize = sizeof(respCode) + len;

	//copy data
	memcpy(buf, &code, sizeof(respCode));
	memcpy( (buf+sizeof(respCode)), data, len);

	ssize_t bytes_sent = 0;
	bytes_sent = ses->send(buf, msgSize);
	if (bytes_sent < 0)
	{
		//send OK
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "Impossible to send the Message vs the command client");
		FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);

		ses->close();
		return false;
	}

	ses->close();
	return true;
}


//----------------------------------------------------
// fxProcessFwStatus
// fwupgrade -i -m magazine -s slot
//----------------------------------------------------

void FIXS_CCH_CmdHandler::fxProcessRemoteFwStatus(ACS_DSD_Session* session, const char* request, unsigned int requestSize)
{
	UNUSED(requestSize);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	respCode result = EXECUTED;
	cch_upgrade_request_fw* fwData = 0;
	fwData = (cch_upgrade_request_fw*)request;

	//get all params from command
	std::string valueToSet = fwData->data;
	std::string magAddr = fwData->magazine;
	std::string slot = fwData->slot;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " magAddr = " << magAddr.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slot = " << slot.c_str() << std::endl;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__	<< " requested status of upgrade from persistent file and present FW in SCX" << std::endl;

	CCH_Util::FwData oldFWData;
	CCH_Util::fwRemoteData newFWData;

	int error = FIXS_CCH_UpgradeManager::getInstance()->fwRemoteStatus(magAddr,slot,oldFWData,newFWData);

	result = static_cast<respCode> (error);

	if (result)
	{
		sendResponse(session, result);
		//trace error
	}
	else
	{
		int bufSize = sizeof(int) + MAX_RESPONSE_SIZE;
		char buf[sizeof(int) + MAX_RESPONSE_SIZE] = {0};

		int respSize = FIXS_CCH_UpgradeManager::getInstance()->printFwRemoteStatus(buf,bufSize,oldFWData,newFWData);
		if (respSize < 0)
		{

			{ //trace
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1, "Internal error during the printFwRemoteStatus function call");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpChar);
			}
			sendResponse(session, INTERNALERR);
		}
		else
		{
			sendMessage(session,EXECUTED,respSize,buf);
		}
	}
}

//----------------------------------------------------
// fxProcessFw_Upg
// fwupgrade -m magazine -s slot container
//----------------------------------------------------
void FIXS_CCH_CmdHandler::fxProcessRemoteFwUpg(ACS_DSD_Session* session, const char* request, unsigned int requestSize)
{
	UNUSED(requestSize);
	//FW_UPG command
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	respCode result = EXECUTED;

	cch_upgrade_request_fw* fwData = 0;
	fwData = (cch_upgrade_request_fw*)request;

	//get all params from command
	std::string valueToSet = fwData->data;
	std::string magAddr = fwData->magazine;
	std::string slot = fwData->slot;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " valueToSet = " << valueToSet.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " magAddr = " << magAddr.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slot = " << slot.c_str() << std::endl;

	/*
	valueToSet is used to understand if the request
	is about an "update" or a "read status"
	*/
		int resCode = FIXS_CCH_UpgradeManager::getInstance()->fwUpgradeStart(magAddr, slot, valueToSet);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
			<< " resCode " << resCode << std::endl;

		result = static_cast<respCode> (resCode);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
			<< " resCode " << resCode << std::endl;

	sendResponse(session, result);
}

void FIXS_CCH_CmdHandler::fxProcessIpmiUpg(ACS_DSD_Session* session, const char* request, unsigned int requestSize)
{
	UNUSED(requestSize);
	//IPMI_UPG command
	//TraceInOut inOut(__FUNCTION__);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	respCode result = EXECUTED;

	cch_ipmiupg_request* ipmiData = 0;
	ipmiData = (cch_ipmiupg_request*)request;

	//get all params from command
	std::string valueToSet = ipmiData->valueToSet;
	std::string ipSCXMaster = ipmiData->ipSCXMaster;
	std::string magAddr = ipmiData->magAddr;
	std::string slotMaster = ipmiData->slotMaster;
	std::string slot = ipmiData->slot;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " valueToSet = " << valueToSet.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ipSCXMaster = " << ipSCXMaster.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " magAddr = " << magAddr.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slotMaster = " << slotMaster.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slot = " << slot.c_str() << std::endl;

	/*
	valueToSet is used to understand if the request
	is about an "update" or a "read status"
	*/
	if (valueToSet == "")
	{
		/*
		when valueToSet is empty the command report
		the status of IPMI UPG process
		like: impi_upg -s -B 1.2.0.4:0

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
			<< " requested status of upgrade " << std::endl;

		int resCode = FIXS_CCH_UpgradeManager::getInstance()->ipmiUpgradeStatus(magAddr, slot, slotMaster, ipSCXMaster);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
			<< " resCode " << resCode << std::endl;

		result = static_cast<respCode> (resCode);
		*/
	}
	else
	{
		/*
		when valueToSet is full the command execute
		an upgrade ... the var. will contain a value
		to use with SNMP SET command.

		like: impi_upg -B 1.2.0.4:0 -U ipmifwfile.mot
		*/
		int resCode = FIXS_CCH_UpgradeManager::getInstance()->ipmiUpgradeStart(magAddr, slot, slotMaster, ipSCXMaster, "", valueToSet);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
			<< " resCode " << resCode << std::endl;

		result = static_cast<respCode> (resCode);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
			<< " resCode " << resCode << std::endl;

	}

	sendResponse(session, result);
}

void FIXS_CCH_CmdHandler::fxProcessIpmiStatus(ACS_DSD_Session* session, const char* request, unsigned int requestSize)
{
	UNUSED(requestSize);
	//IPMI_UPG command
	//	TraceInOut inOut(__FUNCTION__);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		respCode result = EXECUTED;

		cch_ipmiupg_request* ipmiData = 0;
		ipmiData = (cch_ipmiupg_request*)request;

		//get all params from command
		std::string valueToSet = ipmiData->valueToSet;
		std::string ipSCXMaster = ipmiData->ipSCXMaster;
		std::string magAddr = ipmiData->magAddr;
		std::string slotMaster = ipmiData->slotMaster;
		std::string slot = ipmiData->slot;

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " valueToSet = " << valueToSet.c_str() << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ipSCXMaster = " << ipSCXMaster.c_str() << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " magAddr = " << magAddr.c_str() << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slotMaster = " << slotMaster.c_str() << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slot = " << slot.c_str() << std::endl;


			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " requested status of upgrade " << std::endl;

			int resCode = FIXS_CCH_UpgradeManager::getInstance()->ipmiUpgradeStatus(magAddr, slot, slotMaster, ipSCXMaster);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
										<< " resCode " << resCode << std::endl;

			result = static_cast<respCode> (resCode);

				if (result)
				{
					sendResponse(session, result);
					//trace error
				}
				int bufSize = sizeof(int) + MAX_RESPONSE_SIZE;
				char buf[sizeof(int) + MAX_RESPONSE_SIZE] = {0};

				int respSize = FIXS_CCH_UpgradeManager::getInstance()->writeIpmiStatus(buf,bufSize);
				if (respSize <= 0)
				{
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "Internal error during the status process.");
						FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);

					}
					sendResponse(session, INTERNALERR);
				}
				else
				{
					FIXS_CCH_logging->Write("FIXS_CCH_CmdHandler - fxProcessFwStatus success...",LOG_LEVEL_DEBUG);
					sendMessage(session,EXECUTED,respSize,buf);

				}
}

void FIXS_CCH_CmdHandler::fxProcessIpmiDisplay(ACS_DSD_Session* session, const char* request, unsigned int requestSize)
{
//	TraceInOut inOut(__FUNCTION__);
	UNUSED(requestSize);
		respCode result = EXECUTED;
		cch_ipmiupg_request* ipmiDisplay = 0;
		ipmiDisplay = (cch_ipmiupg_request*)request;
		int error = FIXS_CCH_UpgradeManager::getInstance()->ipmiDisplay(ipmiDisplay->magAddr, ipmiDisplay->slot);
		result = static_cast<respCode> (error);
		if (result)
		{
			//error
			sendResponse(session, result);
		}
		else
		{

			int bufSize = sizeof(int) + MAX_RESPONSE_SIZE;
			char buf[sizeof(int) + MAX_RESPONSE_SIZE] = {0};

			int respSize = FIXS_CCH_UpgradeManager::getInstance()->writeIpmiDisplay(buf,bufSize);
			if (respSize <= 0)
			{
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "Internal error in fxProcessFwDisplay");
					FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}
				sendResponse(session, INTERNALERR);
			}
			else
			{
				sendMessage(session,EXECUTED,respSize,buf);
			}
		}
}
