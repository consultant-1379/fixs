
#include "FIXS_CCH_ImmHandler.h"

#define IMPLEMENTER_NUM 4

ACE_Recursive_Thread_Mutex impLock;


/*============================================================================
	ROUTINE: FIXS_CCH_BrfSubscriber
 ============================================================================ */

int FIXS_CCH_BrfSubscriber::svc()
{
	char msg_buff[256]={'\0'};
	ACS_TRA_Logging *logger = FIXS_CCH_DiskHandler::getLogInstance();
	ACS_TRA_trace* _trace = new ACS_TRA_trace("FIXS_CCH_BrfSubscriber");

	svc_run = true;

	FIXS_CCH_EventHandle m_shutdownEvent = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

	int events[2] = {0};
	events[0] = m_stopEvent;
	events[1]= m_shutdownEvent;
	bool svc_exit = false;

	do
	{
		if (IMM_Interface::checkBrmPersistentDataOwnerObject() == EXECUTED)
		{
			if ((setBrfImplementer()))
			{
				snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_BrfSubscriber::svc, Successfully registered as Brf Participant!");
				if(logger)  logger->Write(msg_buff,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);

				FIXS_CCH_Event::SetEvent(m_subscriptionEvent);
				svc_exit = true;
			}
			else
			{
				snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_BrfSubscriber::svc, registration as BrfParticipant failed. Trying again...");
				if(logger)  logger->Write(msg_buff,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
			}
		}
		else
		{
			snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_BrfSubscriber::svc, creation of BrfPersistentDataOwner object failed. Trying again...");
			if(logger)  logger->Write(msg_buff,LOG_LEVEL_ERROR);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
		}


		if (!svc_exit)
		{//Try again
			int eventIndex = FIXS_CCH_Event::WaitForEvents(2,events,5000);

			if (eventIndex == 0 || eventIndex == 1)
			{
				svc_exit = true;
			}
		}

	} while (!svc_exit);

	delete(_trace);
	svc_run = false;

	return 0;
}

bool FIXS_CCH_BrfSubscriber::setBrfImplementer()
{
	bool result = true;

	ACS_CC_ReturnType returnCode;

	ACE_Guard<ACE_Recursive_Thread_Mutex> lock(impLock);

	if (m_oi_BrmPersistentDataOwner != NULL && m_oiHandlerBrmPersistentDataOwner != NULL)
	{
		m_oi_BrmPersistentDataOwner->setObjName(IMM_Util::DN_BRMPERSISTENTDATAOWNER);
		m_oi_BrmPersistentDataOwner->setImpName(IMM_Util::IMPLEMENTER_BRMPERSISTENTDATAOWNER);
		m_oi_BrmPersistentDataOwner->setScope(ACS_APGCC_ONE);

		returnCode = m_oiHandlerBrmPersistentDataOwner->addObjectImpl(m_oi_BrmPersistentDataOwner);
		if(ACS_CC_FAILURE == returnCode)
		{
			result = false;
		}
	}
	else
	{
		//Serious fault occurred
		result = false;
	}

	return result;
}


/*============================================================================
	ROUTINE: FIXS_CCH_Implementer
 ============================================================================ */
int FIXS_CCH_Implementer::svc()
{
	char msg_buff[256]={'\0'};
	ACS_TRA_Logging *logger = FIXS_CCH_DiskHandler::getLogInstance();
	ACS_TRA_trace* _trace = new ACS_TRA_trace("FIXS_CCH_Implementer");

	svc_run = true;

	FIXS_CCH_EventHandle m_shutdownEvent = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

	int events[2] = {0};
	events[0] = m_stopEvent;
	events[1]= m_shutdownEvent;
	bool svc_exit = false;
	uint32_t implementerCount = 0;

	bool *implementerSet = new bool[m_size];
	for (uint32_t i = 0; i < m_size; i++)
	{
		implementerSet[i] = false;
	}

	do
	{
		for (uint32_t i = 0; i < m_size; i++)
		{
			if (!(implementerSet[i]))
			{

				if ((implementerSet[i] = setImplementer(i)))
				{
					snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_Implementer::svc, Successfully set implementer on class %s!", m_names[i].c_str());
					if(logger)  logger->Write(msg_buff,LOG_LEVEL_INFO);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);

					FIXS_CCH_Event::SetEvent(m_subscriptionEvent[i]);
					implementerCount++;
				}
				else
				{
					cout<<"FAILURE:: " << m_names[i] << ".setClasstImplementer"<<endl;
					snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_Implementer::svc, setting implementer on class %s failed. Trying again...", m_names[i].c_str());
					if(logger)  logger->Write(msg_buff,LOG_LEVEL_WARN);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
				}
			}
		}

		if (implementerCount < m_size)
		{//Not all implementer are set. Try again

			snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_Implementer::svc, Not all class implementers are set. Trying again in 2 seconds...");
			if(logger)  logger->Write(msg_buff,LOG_LEVEL_WARN);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);

			int eventIndex = FIXS_CCH_Event::WaitForEvents(2,events,2000);

			if (eventIndex == 0 || eventIndex == 1)
			{
				svc_exit = true;
			}
		}
		else
		{
			snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_Implementer::svc, All class implementer set.");
			if(logger)  logger->Write(msg_buff,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
		}
	} while (implementerCount < m_size && !svc_exit);

	delete[] implementerSet;
	delete(_trace);
	svc_run = false;

	return 0;
}

bool FIXS_CCH_Implementer::setImplementer(uint32_t index)
{

	bool result = false;

	ACS_CC_ReturnType returnCode;

	ACE_Guard<ACE_Recursive_Thread_Mutex> lock(impLock);

	if (m_oImplementer[index] != NULL)
	{
		returnCode = m_oiHandler[index]->addClassImpl(m_oImplementer[index], m_names[index].c_str());
		if(ACS_CC_SUCCESS == returnCode)
		{
			result = true;
		}

	}
	return result;

}

/*============================================================================
	ROUTINE: FIXS_CCH_ImmHandler
 ============================================================================ */
FIXS_CCH_ImmHandler::FIXS_CCH_ImmHandler()
:m_oi_BladeSwManagement(0),m_oi_ApManagedSw(0),m_oi_CpManagedSw(0)
{
	// create the file descriptor to signal stop
//	m_StopEvent = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
	m_StopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);

	// Initialize the svc state flag
	svc_run = false;

	_trace = new ACS_TRA_trace("CCH IMMHandler");

	m_oi_BladeSwManagement = new FIXS_CCH_BladeSwManagement();

	m_oi_ApManagedSw = new FIXS_CCH_ApManagedSw();

	m_oi_CpManagedSw = new FIXS_CCH_CpManagedSw();

	m_oi_BladePercistanceValue = new FIXS_CCH_BladePercistanceValue();

	m_oi_BrmPersistentDataOwner = new FIXS_CCH_BrmPersistentDataOwner();

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	env = CCH_Util::UNKNOWN;

	m_svcTerminated = false;

}

/*============================================================================
	ROUTINE: ~FMS_CPF_ImmHandler
 ============================================================================ */
FIXS_CCH_ImmHandler::~FIXS_CCH_ImmHandler()
{
	char msg_buff[256]={'\0'};
	snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_ImmHandler::destructor, start") ;
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(msg_buff,LOG_LEVEL_DEBUG);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
	if (m_oi_CpManagedSw != NULL)
			m_oiHandlerCpManagedSw.removeClassImpl(m_oi_CpManagedSw,m_oi_CpManagedSw->getIMMClassName());

	if (m_oi_ApManagedSw != NULL)
			m_oiHandlerApManagedSw.removeClassImpl(m_oi_ApManagedSw,m_oi_ApManagedSw->getIMMClassName());

	if (m_oi_BladeSwManagement != NULL)
		m_oiHandlerBladeSwManagement.removeClassImpl(m_oi_BladeSwManagement,m_oi_BladeSwManagement->getIMMClassName());

	if (m_oi_BladePercistanceValue != NULL)
			m_oiHandlerBladePercistance.removeClassImpl(m_oi_BladePercistanceValue,m_oi_BladePercistanceValue->getIMMClassName());

	if(m_oi_BrmPersistentDataOwner != NULL)
		m_oiHandlerBrmPersistentDataOwner.removeObjectImpl(m_oi_BrmPersistentDataOwner);

	if (m_oi_CpManagedSw){
		delete(m_oi_CpManagedSw);
		m_oi_CpManagedSw = NULL;
	}

	if (m_oi_ApManagedSw){
		delete(m_oi_ApManagedSw);
		m_oi_ApManagedSw = NULL;
	}

	if (m_oi_BladeSwManagement){
		delete(m_oi_BladeSwManagement);
		m_oi_BladeSwManagement = NULL;
	}

	if (m_oi_BladePercistanceValue){
		delete(m_oi_BladePercistanceValue);
		m_oi_BladePercistanceValue = NULL;
	}

	if (m_oi_BrmPersistentDataOwner){
		delete(m_oi_BrmPersistentDataOwner);
		m_oi_BrmPersistentDataOwner = NULL;
	}

	if (m_StopEvent) FIXS_CCH_Event::CloseEvent(m_StopEvent);


//	delete(_trace);

//	FIXS_CCH_logging = 0;
	snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_ImmHandler::destructor, end" );
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(msg_buff,LOG_LEVEL_DEBUG);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
	
	delete(_trace);		

	FIXS_CCH_logging = 0;
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_CCH_ImmHandler::open(void *args)
{
	UNUSED(args);

	//int retries = 0;

	activate(THR_DETACHED);
	cout<<"\n............Wait for events.........\n"<<endl;

	return 0;

}


/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_CCH_ImmHandler::svc()
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

	bool brf_participant = false;
	int handles = IMPLEMENTER_NUM + 2;

	FIXS_CCH_EventHandle subscriptionEvent = FIXS_CCH_Event::CreateEvent(false,false,0);
	FIXS_CCH_EventHandle implementerEvent[IMPLEMENTER_NUM] = {0};

	int events[IMPLEMENTER_NUM + 2] = {0};
	events[0]=m_StopEvent;

	// Events from 1 to 4 used to handle IMM implementers
	for (uint32_t i = 0; i < IMPLEMENTER_NUM; i++)
	{
		implementerEvent[i] = FIXS_CCH_Event::CreateEvent(false,false,0);
		events[i+1]=implementerEvent[i];
	}

	// Brf Subscription
	events[IMPLEMENTER_NUM + 1]=subscriptionEvent;

	CCH_Util::APGtype m_APGnum = CCH_Util::getAPGtype();
	FIXS_CCH_BrfSubscriber subscriber(&m_oiHandlerBrmPersistentDataOwner, m_oi_BrmPersistentDataOwner, subscriptionEvent);
	
	if(m_APGnum != CCH_Util::AP2)
	{
		subscriber.open();
	}

	//Prepare OI handler arrays
	acs_apgcc_oihandler_V3* handlers[IMPLEMENTER_NUM] = {0};
	acs_apgcc_objectimplementerinterface_V3* implementers[IMPLEMENTER_NUM] = {0};
	std::string names[IMPLEMENTER_NUM];

	handlers[0] = &m_oiHandlerBladeSwManagement;
	implementers[0] = m_oi_BladeSwManagement;
	names[0] = m_oi_BladeSwManagement->getIMMClassName();

	handlers[1] = &m_oiHandlerBladePercistance;
	implementers[1] = m_oi_BladePercistanceValue;
	names[1] = m_oi_BladePercistanceValue->getIMMClassName();

	handlers[2] = &m_oiHandlerApManagedSw;
	implementers[2] = m_oi_ApManagedSw;
	names[2] = m_oi_ApManagedSw->getIMMClassName();

	handlers[3] = &m_oiHandlerCpManagedSw;
	implementers[3] = m_oi_CpManagedSw;
	names[3] = m_oi_CpManagedSw->getIMMClassName();

	//Start thread to set implementers
	FIXS_CCH_Implementer implementerThread(handlers, implementers, implementerEvent, names, 4);
	implementerThread.open();

	//Flags to recognize implementer set
	bool implementerSet[IMPLEMENTER_NUM] = {false};

	bool loop = true;
	while(loop)
	{
		int eventIndex=FIXS_CCH_Event::WaitForEvents(handles,events,5000);

		if (eventIndex == -2)
		{
			continue;
		}

		else if (eventIndex == -1)
		{
			snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_ImmHandler::svc, exit after error=%s", strerror(errno) );
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(msg_buff,LOG_LEVEL_ERROR);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
			break;
		}

		else if (eventIndex == 0)   // stop event
		{
			{
				snprintf(msg_buff, sizeof(msg_buff) - 1, "[%s@%d] Recieved a stop event", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(msg_buff,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,msg_buff);
			}
			FIXS_CCH_Event::ResetEvent(m_StopEvent);
			loop = false;
			break;
		}
		else if (1 <= eventIndex && eventIndex <= IMPLEMENTER_NUM)
		{// Implementer events
			int j = eventIndex - 1;

			if (implementerSet[j])
			{
				// Received a IMM request
				result = implementers[j]->dispatch(ACS_APGCC_DISPATCH_ONE);

				if(ACS_CC_SUCCESS != result)
				{
					snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_ImmHandler::svc, error on %s dispatch event", implementers[j]->getImpName().c_str() );
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(msg_buff,LOG_LEVEL_ERROR);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
					FIXS_CCH_Event::ResetEvent(events[eventIndex]);
				}
				continue;
			}
			else
			{
				//Received implementer set event
				snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_ImmHandler::svc, %s implementer set, waiting for callbacks.",  implementers[j]->getImpName().c_str() );
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(msg_buff,LOG_LEVEL_WARN);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);

				events[eventIndex] = implementers[j]->getSelObj();

				implementerSet[j] = true;
			}
		}

		else if (eventIndex == IMPLEMENTER_NUM + 1)   // IMM callback or subscription event
		{

			if (brf_participant)
			{//Brf callback
				// Received a IMM request
				result = m_oi_BrmPersistentDataOwner->dispatch(ACS_APGCC_DISPATCH_ONE);

				if(ACS_CC_SUCCESS != result)
				{
					snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_ImmHandler::svc, error on BrmPersistentDataOwner dispatch event" );
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(msg_buff,LOG_LEVEL_ERROR);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);
					FIXS_CCH_Event::ResetEvent(events[4]);
				}
			}
			else
			{//Subscribed to Brf
				snprintf(msg_buff,sizeof(msg_buff)-1,"FIXS_CCH_ImmHandler::svc, Received Brf Subscription event, waiting for callbacks." );
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(msg_buff,LOG_LEVEL_WARN);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, msg_buff);

				events[5] = m_oi_BrmPersistentDataOwner->getSelObj();
				brf_participant = true;
			}
			continue;
		}

	}

	if (m_APGnum != CCH_Util::AP2)
		subscriber.stop();
	FIXS_CCH_Event::CloseEvent(subscriptionEvent);

	implementerThread.stop();
	for (uint32_t i = 0; i < IMPLEMENTER_NUM; i++)
	{
		FIXS_CCH_Event::CloseEvent(implementerEvent[i]);
	}

	subscriber.wait();
	implementerThread.wait();

	// Set the svc thread state to off
	svc_run = false;

	return 0;



}

/*============================================================================
	ROUTINE: signalSvcTermination
 ============================================================================ */
void FIXS_CCH_ImmHandler::signalSvcTermination()
{
	ACE_UINT64 stopEvent=1;
	ssize_t numByte;

	// Signal to IMM thread to stop
	numByte = write(m_svcTerminated, &stopEvent, sizeof(ACE_UINT64));

	if(sizeof(ACE_UINT64) != numByte)
	{
	//	cout << "FIXS_CCH_ImmHandler " <<__LINE__<< " call dispach " << __FUNCTION__<< endl;
	}

}


void FIXS_CCH_ImmHandler::stop ()
{
        std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Inside STOP.... "<< std::endl;
        if (m_StopEvent)
        {
                int retries=0;
                while (FIXS_CCH_Event::SetEvent(m_StopEvent) == 0)
                {
                        if( ++retries > 10) break;
                        sleep(1);
                }

                if (retries>10)
                {
                        if (_trace->ACS_TRA_ON())
                        {
                                char tmpStr[512] = {0};
                                snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal Stop event.", __FUNCTION__, __LINE__);
                                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
                                _trace->ACS_TRA_event(1,tmpStr);
                        }
                }
        }
}
