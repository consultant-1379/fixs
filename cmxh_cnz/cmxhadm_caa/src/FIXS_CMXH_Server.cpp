
#include "FIXS_CMXH_Server.h"
#include "FIXS_CMXH_Logger.h"

/*============================================================================
	ROUTINE: FIXS_CMXH_Server
 ============================================================================ */
FIXS_CMXH_Server::FIXS_CMXH_Server()
: /*_jtpServer(0),*/_trace(0),m_trapHandler(0)
{

	m_environment = CMXH_Util::UNKNOWN;

	_trace = new ACS_TRA_trace("CMXH Server");

	// Create the IMM handler object
//	IMM_Handler = new (std::nothrow) FIXS_CMXH_ImmHandler();

	//Create TrapHandler instance
	m_trapHandler = new FIXS_CMXH_TrapHandler();
	FIXS_CMXH_logging = FIXS_CMXH_Logger::getLogInstance();


}

/*============================================================================
	ROUTINE: ~FIXS_CMXH_Server
 ============================================================================ */
FIXS_CMXH_Server::~FIXS_CMXH_Server()
{
//	int retry = 0;

	if (NULL != m_trapHandler)
	{
		delete m_trapHandler;
		m_trapHandler = NULL;
	}

	//not supported for DMX
	if ((m_environment != CMXH_Util::SINGLECP_DMX) && (m_environment != CMXH_Util::MULTIPLECP_DMX) && (m_environment != CMXH_Util::SMX))
	{
		//unsubscribe from CS
		FIXS_CMXH_Manager *observer = FIXS_CMXH_Manager::getInstance();
		ACS_CS_API_SubscriptionMgr *mgr = ACS_CS_API_SubscriptionMgr::getInstance();
		mgr->unsubscribeHWCTableChanges(*observer);

		FIXS_CMXH_Manager::getInstance()->finalize();
	}

/*	if(NULL != IMM_Handler)
	{
		while(IMM_Handler->getSvcState() == true && retry < 3){
			retry++;
			sleep(1);
		}

		delete IMM_Handler;
		IMM_Handler = NULL;
	} */

	if (_trace != NULL){
		delete(_trace);
		_trace = NULL;
	}
	FIXS_CMXH_logging = 0;
}

bool FIXS_CMXH_Server::run()
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
bool FIXS_CMXH_Server::startWorkerThreads()
{
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Begin" << std::endl;
	bool result = true;

	// Start the IMM handler thread
/*	if( IMM_Handler->open() == -1 )
	{
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "startWorkerThreads, IMM Handler Failed");
			if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
		}

		result = false;
	} */

	if ((m_environment == CMXH_Util::SINGLECP_CBA) || (m_environment == CMXH_Util::MULTIPLECP_CBA))
	{
		//trap manager subscribe
		if (m_trapHandler->open() == 0)
		{
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "startWorkerThreads, TrapHandler open");
				if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				_trace->ACS_TRA_event(1, tmpStr);
			}
		}
	}
//
//
//
//	if (_trace->ACS_TRA_ON())
//	{ //trace
//		char tmpStr[512] = {0};
//		snprintf(tmpStr, sizeof(tmpStr) - 1, "Leaving startWorkerThreads()");
//		FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
//		_trace->ACS_TRA_event(1, tmpStr);
//
//	}
	std::cout << __FUNCTION__ << "@" << __LINE__ << " End" << std::endl;

	return result;
}

/*============================================================================
	ROUTINE: stopWorkerThreads
 ============================================================================ */
bool FIXS_CMXH_Server::stopWorkerThreads()
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

bool FIXS_CMXH_Server::fxInitialize() {
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Begin" << std::endl;

	//--------------------------
	//Read the environment type
	if (m_environment == CMXH_Util::UNKNOWN)
	{
		IMM_Interface::getEnvironmentType(m_environment);
	}

	switch (m_environment)
	{
	case CMXH_Util::SINGLECP_NOTCBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - NOTCBA               " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CMXH_Util::SINGLECP_CBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - CBA                  " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CMXH_Util::MULTIPLECP_NOTCBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - NOTCBA             " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CMXH_Util::MULTIPLECP_CBA:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - CBA                " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		break;
	case CMXH_Util::SINGLECP_DMX:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   SINGLECP - DMX                  " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " end" << std::endl;
		return true; //in order to skip initialization
	case CMXH_Util::MULTIPLECP_DMX:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   MULTIPLECP - DMX                " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " end" << std::endl;
		return true; //in order to skip initialization
	case CMXH_Util::SMX:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM : SMX                " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " end" << std::endl;
		return true; //in order to skip initialization
	default:
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "                 SYSTEM :   UNKNOWN                         " << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " end" << std::endl;
		return false;
	}

	// add DRBD Vlan Object
	if(IMM_Interface::modifyObject() == false)
	{
		std::string mess("modifyObject() failed, continuing with the default vlan configurations");		
		printError(mess, __FUNCTION__, __LINE__);
		// we wont fail the service, lets go ahead with old vlan configuration
	}

	int res = 0;
	res = FIXS_CMXH_Manager::getInstance()->initCMXManager();
	if(res != 0)
	{
		std::string mess("Upgrade manager initialization failed. Error code: ");
		std::stringstream ss;
		ss << res;
		mess+= ss.str();
		printError(mess,__FUNCTION__,__LINE__);
		return false;
	}

	//subscribe to CS
	FIXS_CMXH_Manager *observer = FIXS_CMXH_Manager::getInstance();
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
	std::cout << __FUNCTION__ << "@" << __LINE__ << " end" << std::endl;
	return true;
}

void FIXS_CMXH_Server::printError(std::string mess,const char *func,int line)
{
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] %s",func,line,mess.c_str());
	printf("%s",tmpStr);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
}

