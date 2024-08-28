
#include "FIXS_CCH_Server.h"
#include <sys/capability.h>

#define FAILURE -1

/*============================================================================
	ROUTINE: FIXS_CCH_Server
 ============================================================================ */
FIXS_CCH_Server::FIXS_CCH_Server()
:IMM_Handler(0),_jtpServer(0),_trace(0),m_CmdHandle(0),m_trapHandler(0)
{

	m_environment = CCH_Util::UNKNOWN;

	// Create the Command handler object
	m_CmdHandle =  new (std::nothrow) FIXS_CCH_CmdHandler();

	_jtpServer = new FIXS_CCH_JTPServer();

	_trace = new ACS_TRA_trace("CCH Server");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	// Create the IMM handler object
	IMM_Handler = new (std::nothrow) FIXS_CCH_ImmHandler();

	//Create TrapHandler instance
	m_trapHandler = new FIXS_CCH_TrapHandler();

	m_apgNumber = CCH_Util::Unknown;


}

/*============================================================================
	ROUTINE: ~FIXS_CCH_Server
 ============================================================================ */
FIXS_CCH_Server::~FIXS_CCH_Server()
{
	int retry = 0;

	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "Entering ~FIXS_CCH_Server()");
		if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
	}

	if (NULL != m_trapHandler)
	{
		while(m_trapHandler->getSvcState() == true && retry < 30){
			retry++;
			usleep(100000);
		}

		delete m_trapHandler;
		m_trapHandler = NULL;
	}

	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "~FIXS_CCH_Server(), TRAP handler destroyed");
		if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
	}

	if (CCH_Util::AP1 == m_apgNumber)
	{
		//unsubscribe from CS
		FIXS_CCH_UpgradeManager *observer = FIXS_CCH_UpgradeManager::getInstance();
		ACS_CS_API_SubscriptionMgr *mgr = ACS_CS_API_SubscriptionMgr::getInstance();
		mgr->unsubscribeHWCTableChanges(*observer);

		FIXS_CCH_UpgradeManager::getInstance()->finalize();

		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "~FIXS_CCH_Server(), UpgradeManager destroyed");
			if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
		}
	}

	retry = 0;
	if(NULL != IMM_Handler)
	{
		IMM_Handler->stop();
		while(IMM_Handler->getSvcState() == true && retry < 30){
			IMM_Handler->stop();
			retry++;
			usleep(100000);
		}

		delete IMM_Handler;
		IMM_Handler = NULL;
	}

	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "~FIXS_CCH_Server(), ImmHandler destroyed");
		if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
	}

	if(NULL != m_CmdHandle){
		delete m_CmdHandle;
		m_CmdHandle = NULL;
	}

	if (_jtpServer != NULL){
		delete _jtpServer;
		_jtpServer = NULL;
	}

	if (_trace != NULL){
		delete(_trace);
		_trace = NULL;
	}

	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "Leaving ~FIXS_CCH_Server()");
		if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}

	FIXS_CCH_logging = 0;

}

bool FIXS_CCH_Server::run()
{
	if (fxInitialize())
	{
		sleep(1);
		startWorkerThreads();
	}

	return true;
}


/*============================================================================
	ROUTINE: startWorkerThreads
 ============================================================================ */
bool FIXS_CCH_Server::startWorkerThreads()
{
	bool result = true;

	// Start the IMM handler thread
	if( IMM_Handler->open() == -1 )
	{
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "startWorkerThreads, IMM Handler Failed");
			if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
		}

		result = false;
	}

	if ((m_apgNumber == CCH_Util::AP1)
		&& ((m_environment == CCH_Util::SINGLECP_CBA)
		|| (m_environment == CCH_Util::MULTIPLECP_CBA)
		|| (m_environment == CCH_Util::MULTIPLECP_NOTCBA)
		|| (m_environment == CCH_Util::SINGLECP_DMX)
		|| (m_environment == CCH_Util::MULTIPLECP_DMX)
		|| (m_environment == CCH_Util::SINGLECP_SMX)
		|| (m_environment == CCH_Util::MULTIPLECP_SMX)))
	{
		if( 0 == m_CmdHandle->open() )
		{
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "startWorkerThreads");
				_trace->ACS_TRA_event(1, tmpStr);
			}
		}
		else
		{
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "startWorkerThreads(), error on start Cmd Handler");
				_trace->ACS_TRA_event(1, tmpStr);

			}
			result = false;
		}

		if (_jtpServer->open() == 0)
		{
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "startWorkerThreads, JTPServer open");
				FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				_trace->ACS_TRA_event(1, tmpStr);
			}
		}


		//trap manager subscribe
		if (m_trapHandler->open() == 0)
		{
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "startWorkerThreads, TrapHandler open");
				if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				_trace->ACS_TRA_event(1, tmpStr);
			}
		}
	}

	return result;
}

/*============================================================================
	ROUTINE: stopWorkerThreads
 ============================================================================ */
bool FIXS_CCH_Server::stopWorkerThreads()
{

	bool result = true;

//	if (IMM_Handler->getSvcState())
//	{
//		result = stopImmHandler();
//	}

//	if(NULL != m_CmdHandle)
//		delete m_CmdHandle;

	return result;
}

bool FIXS_CCH_Server::fxInitialize() {
	bool success = true;

	//Read the environment type
	if (m_environment == CCH_Util::UNKNOWN)
	{
		IMM_Interface::getEnvironmentType(m_environment);
		//IMM_Interface::getEnvironmentTypeDirect(m_environment);

		//set global environment
		FIXS_CCH_DiskHandler::setEnvironment(m_environment);
	}
	m_environment = FIXS_CCH_DiskHandler::getEnvironment();
	switch (m_environment)
	{
	case CCH_Util::SINGLECP_NOTCBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - NOTCBA               " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CCH_Util::SINGLECP_CBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - CBA                  " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CCH_Util::MULTIPLECP_NOTCBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - NOTCBA             " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CCH_Util::MULTIPLECP_CBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - CBA                " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CCH_Util::SINGLECP_DMX:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - DMX                  " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break; 
	case CCH_Util::MULTIPLECP_DMX:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - DMX                " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CCH_Util::VIRTUALIZED:
                std::cout << "------------------------------------------------------------"<< std::endl;
                std::cout << "                 SYSTEM :   VIRTUALIZED                " << std::endl;
                std::cout << "------------------------------------------------------------"<< std::endl;
		return false; 
	case CCH_Util::SINGLECP_SMX:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - SMX				  " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break; 
	case CCH_Util::MULTIPLECP_SMX:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - SMX                " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;		
	default:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   UNKNOWN                         " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		return false;
	}

	//check if running on AP1
	m_apgNumber = CCH_Util::getAPGtype();
	if (CCH_Util::AP1 != m_apgNumber)
	{
		//has to return true in order to set the implementers in any case
		return true;
	}

	// Clear CAP_SYS_RESOURCE bit thus root user cannot override disk quota limits
	cap_t cap = cap_get_proc();
	if (NULL != cap)
	{
		cap_value_t cap_list[1];
		cap_list[0] = CAP_SYS_RESOURCE;

		// Clear capability CAP_SYS_RESOURCE
		if (cap_set_flag(cap, CAP_EFFECTIVE, 1, cap_list, CAP_CLEAR) == FAILURE)
		{
			{	
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] cap_set_flag() failed,error = %s", __FUNCTION__, __LINE__, strerror(errno));
				if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
		}
		else
		{

			if (cap_set_proc(cap) == FAILURE)
			{
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] cap_set_proc() failed,error = %s", __FUNCTION__, __LINE__, strerror(errno));
				if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;


			}      
		}

		// free the cap
		if(cap_free(cap) == FAILURE)
		{
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] cap_free() failed,error = %s", __FUNCTION__, __LINE__, strerror(errno));
			if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);
			std::cout << "DBG: " << tmpChar << std::endl;


		}
	}
	else
	{
		char tmpChar[512] = {0};
		snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] cap_get_proc() failed,error = %s", __FUNCTION__, __LINE__, strerror(errno));
		if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);
		std::cout << "DBG: " << tmpChar << std::endl;
	}     

	//----------------------------
	//Create all default folders
	FIXS_CCH_DiskHandler::createDefaultFolder(m_environment);

	{ //log
		char tmpChar[512] = {0};
		snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] createDefaultFolder in environment: %d", __FUNCTION__, __LINE__, m_environment);
		if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);
		std::cout << "DBG: " << tmpChar << std::endl;

	}

	//----------------------------
	//Check between a start-up due to a restart and a start-up due to a restore
	FIXS_CCH_DiskHandler::checkStartAfterRestore();

	//------------------------------
	//Initialise the software map
	int swMapResult = FIXS_CCH_SoftwareMap::getInstance()->initialize();
	if (swMapResult != 0)
	{
		{ //error
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] FIXS_CCH_SoftwareMap initialization failed. Error code: %d", __FUNCTION__, __LINE__, swMapResult);
			if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);
			std::cout << "DBG: " << tmpChar << std::endl;

		}
		success = false;
	}
	else
	{
		{ //log
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] FIXS_CCH_SoftwareMap initialized.", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);
			if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);

		}

		//---------------------------------
		//create and start state machines

		int result = FIXS_CCH_UpgradeManager::getInstance()->initialize();
		if (result != 0)
		{
			std::string mess("Upgrade manager initialization failed. Error code: ");
			std::stringstream ss;
			ss << result;
			mess+= ss.str();
			printError(mess,__FUNCTION__,__LINE__);

			success = false;
		}
		else
		{
			//subscribe to CS
			FIXS_CCH_UpgradeManager *observer = FIXS_CCH_UpgradeManager::getInstance();
			ACS_CS_API_SubscriptionMgr *mgr = ACS_CS_API_SubscriptionMgr::getInstance();
			ACS_CS_API_NS::CS_API_Result csResult;
			csResult = mgr->subscribeHWCTableChanges(*observer);

			if (csResult == ACS_CS_API_NS::Result_Success)
			{
				std::cout << "\n------------------------------------------------------------"<< std::endl;
				std::cout << "             Subscribe to the CS was successful             " << std::endl;
				std::cout << "------------------------------------------------------------\n"<< std::endl;
			}
			else
			{
				//error
				cout<<" Cannot subscribe to CS....\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
				cout<<" csResult: "<< csResult <<endl;
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] Cannot subscribe to CS. Error code: %d", __FUNCTION__, __LINE__, csResult);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);
			}
		}


		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " FIXS_CCH_UpgradeManager initialization ok! result:" << result << std::endl;

		{ //log
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] FIXS_CCH_UpgradeManager initialization ok !!!", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);

		}
	}

	return success;
}

void FIXS_CCH_Server::printError(std::string mess,const char *func,int line)
{
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] %s",func,line,mess.c_str());
	printf("%s",tmpStr);
	if(FIXS_CCH_logging) FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
}
