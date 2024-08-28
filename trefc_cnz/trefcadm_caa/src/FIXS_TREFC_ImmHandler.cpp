
#include "FIXS_TREFC_ImmHandler.h"
#define NO_OF_EVENT_SWITCH 6


/*============================================================================
	ROUTINE: FMS_CPF_ImmHandler
 ============================================================================ */
FIXS_TREFC_ImmHandler::FIXS_TREFC_ImmHandler():
_trace(NULL),
m_oi_TimeServer(NULL),
m_oi_SwitchTimeService(NULL),
m_oi_SwitchTimeReference(NULL),
m_oi_TimeService(NULL),
m_oi_NetworkConfiguration(NULL)
{
	// create the file descriptor to signal stop
	m_StopEvent = FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);

	// create the file descriptor to signal svc terminated
	m_svcTerminated = eventfd(0,0);

	// Initialize the svc state flag
	svc_run = false;

	m_oi_TimeServer = new FIXS_TREFC_OI_TimeServer();

	m_oi_SwitchTimeService = new FIXS_TREFC_OI_SwitchTimeService();

	m_oi_SwitchTimeReference = new FIXS_TREFC_OI_SwitchTimeReference();
	
	m_oi_TimeService = FIXS_TREFC_OI_TimeService::getInstance();

	m_oi_NetworkConfiguration = FIXS_TREFC_OI_NetworkConfiguration::getInstance();

	_trace = new ACS_TRA_trace("TREFC IMMHandler");

	m_env = TREFC_Util::UNKNOWN;
	
}

/*============================================================================
	ROUTINE: ~FMS_CPF_ImmHandler
 ============================================================================ */
FIXS_TREFC_ImmHandler::~FIXS_TREFC_ImmHandler()
{

	//FIXS_TREFC_Event::CloseEvent(m_StopEvent);

	FIXS_TREFC_Event::CloseEvent(m_svcTerminated);

	if (m_oi_TimeServer != NULL) {
		m_oiHandler_TimeServer.removeClassImpl(m_oi_TimeServer,m_oi_TimeServer->getIMMClassName());
		delete m_oi_TimeServer;
		m_oi_TimeServer = NULL;
	}

	if(m_env == TREFC_Util::SMX)
	{
		if (m_oi_SwitchTimeService != NULL) {
			m_oiHandler_SwitchTimeService.removeClassImpl(m_oi_SwitchTimeService,m_oi_SwitchTimeService->getIMMClassName());
			delete m_oi_SwitchTimeService;
			m_oi_SwitchTimeService = NULL;
		}

		if (m_oi_SwitchTimeReference != NULL) {
			m_oiHandler_SwitchTimeReference.removeClassImpl(m_oi_SwitchTimeReference,m_oi_SwitchTimeReference->getIMMClassName());
			delete m_oi_SwitchTimeReference;
			m_oi_SwitchTimeReference = NULL;
		}

		
		if (m_oi_NetworkConfiguration != NULL) {
			m_oiHandler_NetworkConfiguration.removeClassImpl(m_oi_NetworkConfiguration,m_oi_NetworkConfiguration->getIMMClassName());
			m_oi_NetworkConfiguration->finalize();
			m_oi_NetworkConfiguration = NULL;
		}
	}
	else
	{
		if (m_oi_SwitchTimeService != NULL) {
			std::vector<std::string> classNames;
			std::string scxtimeSrv(IMM_Util::classScxTimeService);
			std::string switchtimeSrv(IMM_Util::classSwitchTimeService);
			classNames.push_back(scxtimeSrv);
			classNames.push_back(switchtimeSrv);
			m_oiHandler_SwitchTimeService.removeMultipleClassImpl(m_oi_SwitchTimeService,classNames);
			delete m_oi_SwitchTimeService;
			m_oi_SwitchTimeService = NULL;
			classNames.clear();
		}
		
		if (m_oi_SwitchTimeReference != NULL) {
			std::vector<std::string> classNames;
			std::string scxtimeRef(IMM_Util::classScxTimeReference);
			std::string switchtimeRef(IMM_Util::classSwitchTimeReference);
			classNames.push_back(scxtimeRef);
			classNames.push_back(switchtimeRef);
			m_oiHandler_SwitchTimeReference.removeMultipleClassImpl(m_oi_SwitchTimeReference,classNames);
			delete m_oi_SwitchTimeReference;
			m_oi_SwitchTimeReference = NULL;
			classNames.clear();
		}

		if (m_oi_NetworkConfiguration != NULL) {
			std::vector<std::string> classNames;
			std::string netconfig(IMM_Util::classNetworkConfiguration);
			std::string switchnetconfig(IMM_Util::classNetworkConfigurationSwitch);
			classNames.push_back(netconfig);
			classNames.push_back(switchnetconfig);
			m_oiHandler_NetworkConfiguration.removeMultipleClassImpl(m_oi_NetworkConfiguration,classNames);
			m_oi_NetworkConfiguration->finalize();
			m_oi_NetworkConfiguration = NULL;
			classNames.clear();
		}
			
	}

	if (m_oi_TimeService != NULL) {
		m_oiHandler_TimeService.removeClassImpl(m_oi_TimeService,m_oi_TimeService->getIMMClassName());
		m_oi_TimeService->finalize();
		m_oi_TimeService = NULL;
	}

	if (_trace) {
		delete(_trace);
		_trace=NULL;
	}
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_TREFC_ImmHandler::open(void *args)
{
	char msg_buff[256]={'\0'};
	//ACS_CC_ReturnType returnCode = ACS_CC_SUCCESS;

	UNUSED(args);

	bool isSMX = false;
	m_env = TREFC_Util::getEnvironment();
	if(m_env == TREFC_Util::SMX)
		isSMX=true;

	if (setImplementer(isSMX) == false) {
		cout << "ERROR: Unable to start IMMHandler thread" << std::endl;
		snprintf(msg_buff,sizeof(msg_buff)-1,"ERROR: Unable to start FIXS_TREFC_IMMHandler thread");
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
		return -1;
	}

//	returnCode = m_oiHandler_TimeServer.addClassImpl(m_oi_TimeServer, m_oi_TimeServer->getIMMClassName());
//	if(ACS_CC_FAILURE == returnCode)
//	{
//		cout<<"FAILURE::m_oiHandler_TimeServer.setClasstImplemented"<<endl;
//	}
//
//	returnCode = m_oiHandler_GpsDevice.addClassImpl(m_oi_GpsDevice, m_oi_GpsDevice->getIMMClassName());
//	if(ACS_CC_FAILURE == returnCode)
//	{
//		cout<<"FAILURE::m_oiHandler_GpsDevice.setClasstImplemented"<<endl;
//	}
//
//	returnCode = m_oiHandler_TimeService.addClassImpl(m_oi_TimeService, m_oi_TimeService->getIMMClassName());
//	if(ACS_CC_FAILURE == returnCode)
//	{
//		cout<<"FAILURE::m_oiHandler_TimeService.setClasstImplemented"<<endl;
//	}
//
//	returnCode = m_oiHandler_NetworkConfiguration.addClassImpl(m_oi_NetworkConfiguration, m_oi_NetworkConfiguration->getIMMClassName());
//	if(ACS_CC_FAILURE == returnCode)
//	{
//		cout<<"FAILURE::m_oiHandler_NetworkConfiguration.setClasstImplemented"<<endl;
//	}

//	if (returnCode != ACS_CC_SUCCESS)
//	{
//		m_oiHandler_TimeServer.removeClassImpl(m_oi_TimeServer,m_oi_TimeServer->getIMMClassName());
//		m_oiHandler_GpsDevice.removeClassImpl(m_oi_GpsDevice,m_oi_GpsDevice->getIMMClassName());
//		m_oiHandler_TimeService.removeClassImpl(m_oi_TimeService,m_oi_TimeService->getIMMClassName());
//		m_oiHandler_NetworkConfiguration.removeClassImpl(m_oi_NetworkConfiguration,m_oi_NetworkConfiguration->getIMMClassName());
//
//		return 1;
//	}

	activate();
	sleep(1);
	cout<<"\n............Wait for events.........\n"<<endl;

	return 0;

}


/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_TREFC_ImmHandler::svc()
{

	if (_trace->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "Thread Activated !!!");
		_trace->ACS_TRA_event(1, tmpStr);
	}

	char msg_buff[256]={'\0'};
	ACS_CC_ReturnType result;

	// Set the svc thread state to on
	svc_run = true;

	// waiting for IMM requests or stop

	int events[NO_OF_EVENT_SWITCH]={0};
	events[0]=m_StopEvent;
	events[1]=m_oi_TimeServer->getSelObj();
	events[2]=m_oi_SwitchTimeService->getSelObj();
	events[3]=m_oi_SwitchTimeReference->getSelObj();
	events[4]=m_oi_TimeService->getSelObj();
	events[5]=m_oi_NetworkConfiguration->getSelObj();

	while(svc_run)
	{
		int eventIndex=FIXS_TREFC_Event::WaitForEvents(NO_OF_EVENT_SWITCH,events,0);

		if (eventIndex == -1)
		{
			snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_TREFC_ImmHandler::svc, exit after error=%s", strerror(errno) );
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
			svc_run=false;
			break;
		}

	        else if (eventIndex == 0)   // Shutdown event
        	{
        		cout << "FIXS_TREFC_ImmHandler Shutdown" <<__LINE__<< " " << __FUNCTION__<< endl;
        		svc_run=false;
                	break;
        	}
		else if (eventIndex == 1)   // IMM callback
		{
			// Received a IMM request on a TimeServer
			result = m_oi_TimeServer->dispatch(ACS_APGCC_DISPATCH_ONE);

			if(ACS_CC_SUCCESS != result)
			{
				FIXS_TREFC_Event::ResetEvent(events[eventIndex]);
				cout << "FIXS_TREFC_ImmHandler " <<__LINE__<< " " << __FUNCTION__<< endl;
				snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_TREFC_ImmHandler::svc, error on TimeServer dispatch event" );
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
			}
			continue;
		}
		else if(eventIndex == 2)
		{
			// Received a IMM request on a GpsDevice
			result = m_oi_SwitchTimeService->dispatch(ACS_APGCC_DISPATCH_ONE);

			if(ACS_CC_SUCCESS != result)
			{
				FIXS_TREFC_Event::ResetEvent(events[eventIndex]);
				cout << "FIXS_TREFC_ImmHandler " <<__LINE__<< " " << __FUNCTION__<< endl;
				snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_TREFC_ImmHandler::svc, error on ScxTimeService dispatch event" );
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
			}
			continue;
		}
		else if(eventIndex == 3)
		{
        		// Received a IMM request on a GpsDevice
			result = m_oi_SwitchTimeReference->dispatch(ACS_APGCC_DISPATCH_ONE);

			if(ACS_CC_SUCCESS != result)
			{
        			FIXS_TREFC_Event::ResetEvent(events[eventIndex]);
        			cout << "FIXS_TREFC_ImmHandler " <<__LINE__<< " " << __FUNCTION__<< endl;
        			snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_TREFC_ImmHandler::svc, error on ScxTimeReference dispatch event" );
        			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
        		}	
        		continue;
        	}
        	else if(eventIndex == 4)
        	{
			// Received a IMM request on TimeService
			result = m_oi_TimeService->dispatch(ACS_APGCC_DISPATCH_ONE);

			if(ACS_CC_SUCCESS != result)
			{
				FIXS_TREFC_Event::ResetEvent(events[eventIndex]);
				cout << "FIXS_TREFC_ImmHandler " <<__LINE__<< " " << __FUNCTION__<< endl;
				snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_TREFC_ImmHandler::svc, error on TimeService dispatch event" );
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
			}
			continue;
		}
        	else if(eventIndex == 5)
		{
			// Received a IMM request on a m_oi_PowerFanModule

			result = m_oi_NetworkConfiguration->dispatch(ACS_APGCC_DISPATCH_ONE);

			if(ACS_CC_SUCCESS != result)
			{
				FIXS_TREFC_Event::ResetEvent(events[eventIndex]);
				cout << "FIXS_TREFC_ImmHandler " <<__LINE__<< " " << __FUNCTION__<< endl;
				snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_TREFC_ImmHandler::svc, error on NetworkConfiguration dispatch event" );
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
			}
			continue;
		}
	}
	// Signal svc termination
	signalSvcTermination();

	// Set the svc thread state to off
	svc_run = false;

	return 0;

}

/*============================================================================
	ROUTINE: signalSvcTermination
 ============================================================================ */
void FIXS_TREFC_ImmHandler::signalSvcTermination()
{
	ACE_UINT64 stopEvent=1;
	ssize_t numByte;

	// Signal to IMM thread to stop
	numByte = write(m_svcTerminated, &stopEvent, sizeof(ACE_UINT64));

	if(sizeof(ACE_UINT64) != numByte)
	{
	//	cout << "FIXS_TREFC_ImmHandler " <<__LINE__<< " call dispach " << __FUNCTION__<< endl;
	}

}

bool FIXS_TREFC_ImmHandler::setImplementer(bool isSMX) {

	ACS_CC_ReturnType returnCode = ACS_CC_SUCCESS;
	char msg_buff[256]={'\0'};

	bool TimeServerAdded = false, ScxTimeServiceAdded = false, ScxTimeReferenceAdded = false, TimeServiceAdded = false, ExtIpAddrAddedScx = false;
	bool SwitchTimeServiceAdded = false, SwitchTimeReferenceAdded = false, ExtIpAddrAddedSwitch =false;;
	bool allAdded = false;

	unsigned int retry = 0;
	unsigned int maxRetries = 3;

	while (!allAdded && retry < maxRetries) {
		if (!TimeServerAdded) {
			returnCode = m_oiHandler_TimeServer.addClassImpl(m_oi_TimeServer, m_oi_TimeServer->getIMMClassName());
			if(ACS_CC_FAILURE == returnCode)
			{
				cout<<"FAILURE::m_oiHandler_TimeServer.setClasstImplemented"<<endl;
			} else
				TimeServerAdded = true;
		}

		if (!TimeServiceAdded) {
			returnCode = m_oiHandler_TimeService.addClassImpl(m_oi_TimeService, m_oi_TimeService->getIMMClassName());
			if(ACS_CC_FAILURE == returnCode)
			{
				cout<<"FAILURE::m_oiHandler_TimeService.setClasstImplemented"<<endl;
			} else
				TimeServiceAdded = true;
		}


		if(!isSMX)
		{
			if (!ScxTimeServiceAdded) {
				std::vector<std::string> classNames;
                                std::string scxtimeSrv(IMM_Util::classScxTimeService);
                                std::string switchtimeSrv(IMM_Util::classSwitchTimeService);
                                classNames.push_back(scxtimeSrv);
                                classNames.push_back(switchtimeSrv);
				returnCode = m_oiHandler_SwitchTimeService.addMultipleClassImpl(m_oi_SwitchTimeService, classNames);
				if(ACS_CC_FAILURE == returnCode)
				{
					cout<<"FAILURE::m_oiHandler_SwitchTimeService.setClasstImplemented"<<endl;
				} else
					ScxTimeServiceAdded = true;
			}

			if (!ScxTimeReferenceAdded) {
				std::vector<std::string> classNames;
                                std::string scxtimeRef(IMM_Util::classScxTimeReference);
                                std::string switchtimeRef(IMM_Util::classSwitchTimeReference);
                                classNames.push_back(scxtimeRef);
                                classNames.push_back(switchtimeRef);
				returnCode = m_oiHandler_SwitchTimeReference.addMultipleClassImpl(m_oi_SwitchTimeReference, classNames);
				if(ACS_CC_FAILURE == returnCode)
				{
					cout<<"FAILURE::m_oiHandler_SwitchTimeReference.setClasstImplemented"<<endl;
				} else
					ScxTimeReferenceAdded = true;
			}
	
			if (!ExtIpAddrAddedScx) {
				std::vector<std::string> classNames;
				std::string netconfig(IMM_Util::classNetworkConfiguration);
				std::string switchnetconfig(IMM_Util::classNetworkConfigurationSwitch);
				classNames.push_back(netconfig);
				classNames.push_back(switchnetconfig);
				returnCode = m_oiHandler_NetworkConfiguration.addMultipleClassImpl(m_oi_NetworkConfiguration,classNames);
				if(ACS_CC_FAILURE == returnCode)
				{
					cout<<"FAILURE::m_oiHandler_NetworkConfiguration.setClasstImplemented"<<endl;
				} else
					ExtIpAddrAddedScx = true; 
                	}
		}
		else
		{
			if (!SwitchTimeServiceAdded) {
				returnCode = m_oiHandler_SwitchTimeService.addClassImpl(m_oi_SwitchTimeService, m_oi_SwitchTimeService->getIMMClassName());
				if(ACS_CC_FAILURE == returnCode)
				{
					cout<<"FAILURE::m_oiHandler_SwitchTimeService.setClasstImplemented"<<endl;
				} else
					SwitchTimeServiceAdded = true;
			}

			if (!SwitchTimeReferenceAdded) {
				returnCode = m_oiHandler_SwitchTimeReference.addClassImpl(m_oi_SwitchTimeReference, m_oi_SwitchTimeReference->getIMMClassName());
				if(ACS_CC_FAILURE == returnCode)
				{
					cout<<"FAILURE::m_oiHandler_SwitchTimeReference.setClasstImplemented"<<endl;
				} else
					SwitchTimeReferenceAdded = true;
			}

			if (!TimeServiceAdded) {
				returnCode = m_oiHandler_TimeService.addClassImpl(m_oi_TimeService, m_oi_TimeService->getIMMClassName());
				if(ACS_CC_FAILURE == returnCode)
				{
					cout<<"FAILURE::m_oiHandler_TimeService.setClasstImplemented"<<endl;
				} else
					TimeServiceAdded = true;
			}

			if (!ExtIpAddrAddedSwitch) {
				cout << " reached before m_oiHandler_NetworkConfiguration.setClasstImplement" << endl;
				returnCode = m_oiHandler_NetworkConfiguration.addClassImpl(m_oi_NetworkConfiguration, m_oi_NetworkConfiguration->getIMMClassName());
				if(ACS_CC_FAILURE == returnCode)
				{
					cout<<"FAILURE::m_oiHandler_NetworkConfiguration.setClasstImplemented"<<endl;
				} else
					ExtIpAddrAddedSwitch = true;
			}
		}

		if(isSMX)
			allAdded = TimeServerAdded && TimeServiceAdded && SwitchTimeServiceAdded && SwitchTimeReferenceAdded && ExtIpAddrAddedSwitch;
		else
			allAdded = TimeServerAdded && TimeServiceAdded && ScxTimeServiceAdded && ScxTimeReferenceAdded && ExtIpAddrAddedScx;
		retry++;

		if (!allAdded && retry == maxRetries) {
			std::cout << "Failed to set all class implementer after 3 attempts! Exiting from thread" << std::endl;
			snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_TREFC_ImmHandler::Failed to set all class implementer after 3 attempts! Exiting from thread" );
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
		}

		else if (!allAdded) {
			std::cout << "Failed to set all class implementer! Trying again in 2 seconds" << std::endl;
			snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_TREFC_ImmHandler::Failed to set all class implementer! Trying again in 2 seconds" );
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
			sleep(2);
		}

	}

	return allAdded;
}
