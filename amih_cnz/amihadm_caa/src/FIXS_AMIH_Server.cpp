#include "FIXS_AMIH_Server.h"
#include "FIXS_AMIH_Event.h"
#include "acs_apgcc_omhandler.h"
#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"
#include "FIXS_AMIH_HwcSubscriber.h"


const int FIXS_AMIH_Server::AMIHSRV_NO_ERROR = 0;
const int FIXS_AMIH_Server::AMIHSRV_ERR_CREATE_STOP_EVENT = -2;
const int FIXS_AMIH_Server::AMIHSRV_ERR_NEW_SYS_CONFIGURATION = -6;
const int FIXS_AMIH_Server::AMIHSRV_ERR_SYS_CONFIGURATION = -7;
const int FIXS_AMIH_Server::AMIHSRV_ERR_CREATE_EVENT_HANDLER = -8;
const int FIXS_AMIH_Server::AMIHSRV_ERR_WAIT_FAILED = -12;
const int FIXS_AMIH_Server::AMIHSRV_ERR_NEW_MGR = -13;
const int FIXS_AMIH_Server::AMIHSRV_ERR_START_MGR = -14;
const int FIXS_AMIH_Server::AMIHSRV_ERR_NEW_STAT = -16;
const int FIXS_AMIH_Server::AMIHSRV_ERR_INIT_STAT = -17;
const int FIXS_AMIH_Server::AMIHSRV_ERR_LOAD_DEF_FILE = -18;
const int FIXS_AMIH_Server::AMIHSRV_ERR_CHECK_DATADISK = -19;
const int FIXS_AMIH_Server::AMIHSRV_START_PROTOCOL_TIMEOUT = 2000;
const int FIXS_AMIH_Server::AMIHSRV_GET_CONFIGURATION_TIMEOUT = 500;
const int FIXS_AMIH_Server::AMIHSRV_WAIT_EVENT_TIMEOUT = 1000;
const int FIXS_AMIH_Server::AMIHSRV_WAIT_FINALIZE_ALL_MGR = 20000;
const int FIXS_AMIH_Server::AMIHSRV_MAX_RETRIES_ON_ERROR = 10;


/*============================================================================
	ROUTINE: FIXS_AMIH_Server
 ============================================================================ */
FIXS_AMIH_Server::FIXS_AMIH_Server()
{
	_alive=true;
	m_exit=false;

	jtpHandler = new FIXS_AMIH_JTPHandler();

	jtpHandler->activate();

	observer = new FIXS_AMIH_HwcSubscriber(jtpHandler);

	isSubscribed = false;

	//DSDHandle = new (std::nothrow) FIXS_AMIH_DSDHandler();

	_trace = new ACS_TRA_trace("FIXS_AMIH_Server");

}

/*============================================================================
	ROUTINE: ~FIXS_AMIH_Server
 ============================================================================ */
FIXS_AMIH_Server::~FIXS_AMIH_Server()
{

	if (_trace) {
		delete(_trace);
		_trace = NULL;
	}

	if (jtpHandler) {
		delete(jtpHandler);
		jtpHandler = NULL;
	}

	if (observer) {
		delete(observer);
		observer = NULL;
	}

}



bool FIXS_AMIH_Server::run()
{

	if (fxInitialize())
	{
		sleep(1);
		startWorkerThreads();
	}

	return true;

}




/*============================================================================
	ROUTINE: startImmThread
 ============================================================================ */


void FIXS_AMIH_Server::printTRA(std::string mesg)
{
	if (_trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, mesg.c_str());
		_trace->ACS_TRA_event(1, tmpStr);

	}
}



/*============================================================================
	ROUTINE: stopWorkerThreads
 ============================================================================ */
bool FIXS_AMIH_Server::stopWorkerThreads()
{
	bool result = true;

	return result;
}



bool FIXS_AMIH_Server::fxInitialize()
{
	bool cba=false;
	bool commandOk=false;
	open();

	commandOk=IsCba(cba);

	if(!commandOk)
	{

		printf("Unable to connect to imm.\n");

		printTRA("Unable to connect to imm.");

		if (isSubscribed)
		{
			ACS_CS_API_NS::CS_API_Result result;


			result = ACS_CS_API_SubscriptionMgr::getInstance()->unsubscribeHWCTableChanges(*observer);

			if (result != ACS_CS_API_NS::Result_Success)
			{
				printf("Unsubscription to HWC Table failed !\n");
				printTRA("Unsubscription to HWC Table failed !");
			}
			else
			{
				isSubscribed = false;
			}
		}
	}
	else
	{
		if (cba)
		{

			printTRA("is CBA!");
			printf("is CBA!\n");

			if (!isSubscribed)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Observer INIT " << std::endl;

				ACS_CS_API_NS::CS_API_Result result;

				result = ACS_CS_API_SubscriptionMgr::getInstance()->subscribeHWCTableChanges(*observer);

				if (result != ACS_CS_API_NS::Result_Success)
				{
					printTRA("Subscription to HWC Table failed !");
				}
				else
				{
					isSubscribed = true;
				}

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Observer INIT " << std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " result " << result << std::endl;
			}
		}
		else
		{
			printf("No SCX Architecture !");

			printTRA("No SCX Architecture !");

			if (isSubscribed)
			{
				ACS_CS_API_NS::CS_API_Result result;

				result = ACS_CS_API_SubscriptionMgr::getInstance()->unsubscribeHWCTableChanges(*observer);

				if (result != ACS_CS_API_NS::Result_Success)
				{
					printTRA("Unsubscription to HWC Table failed !");
				}
				else
				{
					isSubscribed = false;
				}

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Observer stop " << std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " result " << result << std::endl;
			}
		}
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " deconstructor - isSubscribed " << isSubscribed << std::endl;

	return true;
}


void FIXS_AMIH_Server::printError(std::string mess,const char *func,int line)
{
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] %s",func,line,mess.c_str());
	printf("%s",tmpStr);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
}



bool FIXS_AMIH_Server::getImmAttributeInt(const std::string &strDn, const std::string &strAttr, int *iVal)
{
	ACS_CC_ReturnType nResult;

	ACS_CC_ImmParameter Param;

	OmHandler omHandler;

	char *name_attrPath;

	nResult = ACS_CC_FAILURE;
	name_attrPath = NULL;

	nResult = omHandler.Init();

	if (nResult != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
	}

	if(nResult == ACS_CC_SUCCESS)
	{
		name_attrPath = const_cast<char*>(strAttr.c_str());

		Param.attrName = name_attrPath;

		nResult = omHandler.getAttribute(strDn.c_str(), &Param );

		if ( nResult != ACS_CC_SUCCESS ){
			std::cout << "ERROR: Param FAILURE!!!\n" << std::endl;
		}
	}

	if(nResult == ACS_CC_SUCCESS)
	{
		std::cout << " --> Param value########: " << *(int*)Param.attrValues[0] << std::endl;

		(*iVal) = (*(int*)Param.attrValues[0]);

		nResult = omHandler.Finalize();

		if (nResult != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
		}
	}

  return (nResult == ACS_CC_SUCCESS);
}


bool FIXS_AMIH_Server::IsCba(bool &res)
{
	int iVal;
	iVal = 0;
	bool resCommand=false;

	resCommand=getImmAttributeInt(AMIH_Util::DN_APZ,AMIH_Util::ATT_APZ_CBA,&iVal);

	if(resCommand)
	{
		if (iVal == 1)
			res=true;
		else
			res=false;
	}



	return resCommand;

}


const char * FIXS_AMIH_Server::name ()
{
  //## begin FIXS_AMIH_Server::name%4BFFC16700DD.body preserve=yes
  //## end FIXS_AMIH_Server::name%4BFFC16700DD.body

  return "FIXS_AMIH_SERVER";
}

const char * FIXS_AMIH_Server::domain ()
{
  //## begin FIXS_AMIH_Server::domain%4BFFC1D200F9.body preserve=yes
  //## end FIXS_AMIH_Server::domain%4BFFC1D200F9.body

  return "FIXS";
}

/*============================================================================
	ROUTINE: startWorkerThreads
 ============================================================================ */


bool FIXS_AMIH_Server::startWorkerThreads()
{
	bool result = true;

/*	if (DSDHandle->open() == 0)
	{
		printTRA("startWorkerThreads, DSDHandle open");

	}
	else
	{
		printTRA("startWorkerThreads, error on start DSDHandle");

		result=false;
	}
*/
	return result;
}

int FIXS_AMIH_Server::svc()
{
	int validIndex=1;
	FIXS_AMIH_EventHandle validHandles[validIndex];
	validHandles[0]= FIXS_AMIH_Event::OpenNamedEvent(AMIH_Util::EVENT_NAME_SHUTDOWN);

	const int WAIT_SHUTDOWN		=  0;
	const int WAIT_FAILED   	= -1;
	const int WAIT_TIMEOUT		= -2;

	int retry=0;

	while (!m_exit) // Set to true when the thread shall exit
	{
		int status = FIXS_AMIH_Event::WaitForEvents(validIndex,validHandles,AMIHSRV_WAIT_EVENT_TIMEOUT);

		if (status == WAIT_TIMEOUT)
		{
			printTRA("FIXS_AMIH_Serve::run timeout event!\n");

		}
		else if(status==WAIT_SHUTDOWN)
		{
			printTRA("FIXS_AMIH_Serve::run shutdown event!\n");

			m_exit=true;

		}
		else if (status == WAIT_FAILED)
		{
			retry++;

			if(retry>AMIHSRV_MAX_RETRIES_ON_ERROR)
			{
				retry=0;
				m_exit=true;
			}
			else
			{
				printTRA("FIXS_AMIH_Serve::run retry on WAIT_FAILED event!\n");
			}
		}
	}

	if (isSubscribed)
	{
		ACS_CS_API_NS::CS_API_Result result;

		result = ACS_CS_API_SubscriptionMgr::getInstance()->unsubscribeHWCTableChanges(*observer);

		if (result != ACS_CS_API_NS::Result_Success)
		{
			printTRA("Run: Unsubscription to HWC Table failed !");
		}

		printTRA("Run: Unsubscription success !");
	}

	return true;
}

int FIXS_AMIH_Server::open()
{
	m_exit=false;

	activate();

	sleep(1);

	return 0;
}




