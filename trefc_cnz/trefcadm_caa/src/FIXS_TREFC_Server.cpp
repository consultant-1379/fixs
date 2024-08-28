
#include "FIXS_TREFC_Server.h"
#include "FIXS_TREFC_Logger.h"


/*============================================================================
	ROUTINE: FIXS_TREFC_Server
 ============================================================================ */
FIXS_TREFC_Server::FIXS_TREFC_Server()
{

	m_environment = TREFC_Util::UNKNOWN;

	// Create the IMM handler object
	IMM_Handler = new (std::nothrow) FIXS_TREFC_ImmHandler();

	//Create DSD Thread
	//m_DSDHandle = new (std::nothrow) FIXS_TREFC_DSDHandler();

	//Create JTP Server Thread
	_jtpServer = new (std::nothrow) FIXS_TREFC_JTPServer();

	_trace = new ACS_TRA_trace("TREFC Server");

}

/*============================================================================
	ROUTINE: ~FIXS_TREFC_Server
 ============================================================================ */
FIXS_TREFC_Server::~FIXS_TREFC_Server()
{

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Server] Received shutdown event! Shutting Down...");

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Server] Stopping IMM callback thread...", LOG_LEVEL_DEBUG);

	if(IMM_Handler != NULL) {
		for(int i = 0; ((i < 3) && IMM_Handler->getSvcState()); i++) sleep(1);
		delete IMM_Handler;
		IMM_Handler = NULL;

	}

	if (m_environment == SINGLECP_CBA || m_environment == MULTIPLECP_CBA || m_environment == TREFC_Util::SMX) {
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Server] Closing JTP Server Thread...", LOG_LEVEL_DEBUG);
		if (_jtpServer != NULL)
		{
			delete _jtpServer;
			_jtpServer = NULL;
		}

		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Server] Unsubscribing from CS...");
		//unsubscribe from CS
		FIXS_TREFC_Manager *observer = FIXS_TREFC_Manager::getInstance();
		ACS_CS_API_SubscriptionMgr *mgr = ACS_CS_API_SubscriptionMgr::getInstance();
		mgr->unsubscribeHWCTableChanges(*observer);


		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Server] Finalizing TREFC Manager...");
		//Finalize TREFC Manager
		FIXS_TREFC_Manager::getInstance()->finalize();
	}

	if (_trace != NULL) {
		delete(_trace);
		_trace = NULL;
	}

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Server] Service Terminated!");
}

bool FIXS_TREFC_Server::run()
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
bool FIXS_TREFC_Server::startWorkerThreads()
{
	bool result = true;

	// Start the IMM handler thread
	if( -1 == IMM_Handler->open())
	{
		cout<<__LINE__<<" "<<__FUNCTION__<<endl;
		result = false;
	}
	else
	{
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Server] IMM Handler started.");
			_trace->ACS_TRA_event(1, tmpStr);
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
		}

		if (m_environment == SINGLECP_CBA || m_environment == MULTIPLECP_CBA || m_environment == TREFC_Util::SMX) {
			//Start JTP Server Thread
			if (_jtpServer->open() == 0)
			{
				if (_trace->ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Server] JTPServer started.");
					_trace->ACS_TRA_event(1, tmpStr);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}
			}
			else {
				if (_trace->ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Server] error on start JTPServer");
					_trace->ACS_TRA_event(1, tmpStr);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
					result = false;
				}
			}
		}
	}

	if (_trace->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "Leaving startWorkerThreads()");
		_trace->ACS_TRA_event(1, tmpStr);

	}

	return result;
}

/*============================================================================
	ROUTINE: stopWorkerThreads
 ============================================================================ */
bool FIXS_TREFC_Server::stopWorkerThreads()
{

	bool result = true;
	/*
	if (IMM_Handler->getSvcState())
	{
		result = stopImmHandler();
	}

	if(NULL != m_CmdHandle)
		delete m_CmdHandle;
	 */
	return result;
}

/*============================================================================
	ROUTINE: stopImmHandler
 ============================================================================ */
bool  FIXS_TREFC_Server::stopImmHandler()
{
	bool result = true;

//	ACE_UINT64 stopEvent=1;
//	ssize_t numByte;
//	ACE_INT32 eventfd;
//
//
//	IMM_Handler->getStopHandle(eventfd);
//
//	// Signal to IMM thread to stop
//	numByte = write(eventfd, &stopEvent, sizeof(ACE_UINT64));
//
//	if(sizeof(ACE_UINT64) != numByte)
//	{
//		result = false;
//	}

	return result;
}


bool FIXS_TREFC_Server::fxInitialize()
{
	bool success = true;

	m_environment = TREFC_Util::getEnvironment();

	//--------------------------
	//Read the environment type
	if (m_environment == TREFC_Util::UNKNOWN)
	{
		cout << "ERROR: Unable to get environment type" << endl;
		return false;
	}

	switch (m_environment)
	{
	case TREFC_Util::SINGLECP_NOTCBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - NOTCBA               " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case TREFC_Util::SINGLECP_CBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - CBA                  " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case TREFC_Util::MULTIPLECP_NOTCBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - NOTCBA             " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case TREFC_Util::MULTIPLECP_CBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - CBA                " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case TREFC_Util::SMX:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SMX                             " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	default:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   UNKNOWN                         " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		return false;
	}

	//check the correct environment
	if ( m_environment == TREFC_Util::SINGLECP_NOTCBA ||  m_environment == TREFC_Util::MULTIPLECP_NOTCBA )
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Server] Starting in not CBA Environment. All operations are forbidden.");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

		ImmInterface::deleteSwitchTimeServiceClasses();
		ImmInterface::setServiceAdmState(IMM_Util::LOCKED);
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"before setServiceStatus" <<std::endl; //mychange
		ImmInterface::setServiceStatus(IMM_Util::DISABLED);
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"after setServiceStatus" <<std::endl; //mychange
		ImmInterface::setServiceResultState(IMM_Util::EXECUTED);

		//cout << " ERROR: WRONG ENVIRONMENT !!!" << endl;
		//return false;

	} else {
		int res = 0;
		res = FIXS_TREFC_Manager::getInstance()->initTRefManager();
		if(res == -1) return false;

		//subscribe to CS
		FIXS_TREFC_Manager *observer = FIXS_TREFC_Manager::getInstance();
		ACS_CS_API_SubscriptionMgr *mgr = ACS_CS_API_SubscriptionMgr::getInstance();
		ACS_CS_API_NS::CS_API_Result csResult;
		csResult = mgr->subscribeHWCTableChanges(*observer);

		if (csResult == ACS_CS_API_NS::Result_Success)
		{
			std::cout << "\n------------------------------------------------------------"<< std::endl;
			std::cout << "             Subscribe to the CS was successful             " << std::endl;
			std::cout << "------------------------------------------------------------\n"<< std::endl;


			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Server] Subscribe to the CS was successful.");
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

		}
		else
		{
			//error
			cout<<" Cannot subscribe to CS....\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
			cout<<" csResult: "<< csResult <<endl;
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] Cannot subscribe to CS. Error code: %d", __FUNCTION__, __LINE__, csResult);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpChar);

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Server] Cannot subscribe to CS. Error code: %d.", csResult);
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
		}
	}

	return success;
}

void FIXS_TREFC_Server::printError(std::string mess,const char *func,int line)
{
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] %s",func,line,mess.c_str());
	printf("%s",tmpStr);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
}

