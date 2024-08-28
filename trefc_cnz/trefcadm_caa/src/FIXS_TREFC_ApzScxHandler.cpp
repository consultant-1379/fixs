/*
 * FIXS_TREFC_ApzScxHandler.cpp
 *
 *  Created on: Jan 9, 2012
 *      Author: elalkak
 */



#include "FIXS_TREFC_ApzScxHandler.h"


#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif




FIXS_TREFC_ApzScxHandler::FIXS_TREFC_ApzScxHandler(string ipA, string ipB)
:tRefList(),
 m_timeRefAdd(),
 m_timeRefRemove(),
 m_otherScxA(""),
 m_otherScxB(""),
 m_lock()
{


	m_running = false;
	m_exit = true;

	m_ipA = ipA;
	m_ipB = ipB;
	m_snmpManager = new FIXS_TREFC_SNMPManager(ipA);

	m_NtpRetry = false;
	m_NtpHpRetry = false;
	m_addNtpRefRetry = false;
	m_addNtpHpRefRetry = false;
	m_stopNtpRetry = false;
	m_stopNtpHpRetry = false;
	m_addExtIpRetry = false;
	m_rmExtIpRetry = false;
	m_rmNtpRefRetry = false;
	m_rmNtpHpRefRetry = false;
	m_isRetryOngoing = false;
	m_setTime = false;
	m_cleanup = false;
	ntpdRestart = false;

	m_shutdownEvent = -1;

	traceApzScxHandler = new ACS_TRA_trace("FIXS_TREFC_ApzScxHandler");


}


FIXS_TREFC_ApzScxHandler::~FIXS_TREFC_ApzScxHandler()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

 std::cout << __FUNCTION__ << "@" << __LINE__ <<" ~FIXS_TREFC_ApzScxHandler() "<< std::endl; //mychange

	if(m_snmpManager) {
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	//Delete trace object
	if (traceApzScxHandler) {
		delete (traceApzScxHandler);
		traceApzScxHandler = NULL;
	}

}

int FIXS_TREFC_ApzScxHandler::initialize() {
	m_exit = false;
	std::cout << __FILE__ << "  line: " << __LINE__ << "  func.: " << __FUNCTION__ << std::endl;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Try to activate..." << std::endl;
	int result = activate();
	std::cout << __FUNCTION__ << "@" << __LINE__ << " activate: " << result << std::endl;
	if(result == -1) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Activation failed" << std::endl;
		m_exit = true;
	}

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Returning true" << std::endl;
	return result;
}



int FIXS_TREFC_ApzScxHandler::close (){
	//Thread will stop!
	m_exit = true;

	cout << "THREAD RECEIVED CLOSE REQUEST " << m_ipA << endl;

	if (m_snmpManager) {
		m_snmpManager->stopWork();
	}

	return 0;
}

int FIXS_TREFC_ApzScxHandler::svc(void)
{


                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Entering FIXS_TREFC_ApzScxHandler::svc  "<< std::endl; //mychange
		const int Number_Of_Events=1;
		const int WAIT_FAILED=-1;
		const int WAIT_TIMEOUT= -2;
		const int WAIT_OBJECT_0=0;
		const int Shutdown_Event_Index=0;
		//const int Command_Event_Index=1;

		m_shutdownEvent=FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);

		m_exit = false;
		m_running = true;
		//int result = -1;

		//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

		FIXS_TREFC_EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent};

		//loop
		while ( !m_exit ) // Set to true when the thread shall exit
		{
			int returnValue=FIXS_TREFC_Event::WaitForEvents(Number_Of_Events, handleArr, 3000);

			switch (returnValue)
			{
			case WAIT_FAILED:   // Event failed
			{

				if (traceApzScxHandler->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects )", __FUNCTION__, __LINE__);
					traceApzScxHandler->ACS_TRA_event(1,tmpStr);
				}

				m_exit = true;    // Terminate the sw upgrade thread
				break;
			}

			case (WAIT_TIMEOUT):  // Time out
			{

				//TODO:These tracings seem useless...
//				if (! m_exit)
//				{
//					if (traceApzScxHandler->ACS_TRA_ON())
//					{
//						char tmpStr[512] = {0};
//						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Weak up the timer...", __FUNCTION__, __LINE__);
//						traceApzScxHandler->ACS_TRA_event(1,tmpStr);
//					}
//				}
//				else
//				{
//					std::cout << __FUNCTION__ << "@" << __LINE__ << "Time-out will be not signaled !" << std::endl;
//					if (traceApzScxHandler->ACS_TRA_ON())
//					{
//						char tmpStr[512] = {0};
//						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Time-out will be not signaled !", __FUNCTION__, __LINE__);
//						traceApzScxHandler->ACS_TRA_event(1,tmpStr);
//					}
//				}


				break;
			}

			case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
			{

				std::cout << __FUNCTION__ << "@" << __LINE__ <<"Shutdown_Event RECEIVED " <<std::endl; //mychange

				if (traceApzScxHandler->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Received a shutdown event", __FUNCTION__, __LINE__);
					traceApzScxHandler->ACS_TRA_event(1,tmpStr);
				}

				cout << "THREAD RECEIVED SHUTDOWN EVENT " << m_ipA << endl;

				m_exit = true;  // Terminate the thread
				m_running = false;

				//Stop SNMP operation
				if (m_snmpManager) {
					m_snmpManager->stopWork();
				}

				cout << "THREAD EXITING NOW " << m_ipA << endl;
				return 0;
			}

			default:
			{
				if (traceApzScxHandler->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
					traceApzScxHandler->ACS_TRA_event(1,tmpStr);
				}
				break;
			}
			} // End of switch

			if(m_NtpRetry && !m_exit)
			{
				std::cout << "Calling m_NtpRetry!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(reconfNtp())
				{
					std::cout << "m_NtpRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					m_NtpRetry = false;
					m_isRetryOngoing = false;
				}
			}
			if(m_NtpHpRetry && !m_exit)
			{
				std::cout << "Calling m_NtpHpRetry!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(reconfNtphp())
				{
					std::cout << "m_NtpHpRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					m_NtpHpRetry = false;
					m_isRetryOngoing = false;
				}
			}
			if(m_addNtpRefRetry && !m_exit)
			{
				std::cout << "Calling m_addNtpRefRetry!!!!!!!!!!!!!!!!!!!" << std::endl;

				ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

				int size = m_timeRefAdd.size();
				bool snmp_failure = false;

				while (size > 0 && !snmp_failure) {
					std::string m_timeRef = m_timeRefAdd.at(0);

					if(addTimeRefNtp(m_timeRef) == 0)
					{
						std::cout << "m_addNtpRefRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
						m_timeRefAdd.erase(m_timeRefAdd.begin());
					} else
						snmp_failure = true;

					size = m_timeRefAdd.size();
				}

				if (size == 0) {
					m_addNtpRefRetry = false;
					m_isRetryOngoing = false;
				}

			}
			if(m_addNtpHpRefRetry && !m_exit)
			{
				std::cout << "Calling m_addNtpHpRefRetry!!!!!!!!!!!!!!!!!!!" << std::endl;

				ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

				int size = m_timeRefAdd.size();
				bool snmp_failure = false;

				while (size > 0 && !snmp_failure) {
					std::string m_timeRef = m_timeRefAdd.at(0);

					if(addTimeRefNtphp(m_timeRef) == 0)
					{
						std::cout << "m_addNtpRefRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
						m_timeRefAdd.erase(m_timeRefAdd.begin());
					} else
						snmp_failure = true;

					size = m_timeRefAdd.size();
				}

				if (size == 0) {
					m_addNtpHpRefRetry = false;
					m_isRetryOngoing = false;
				}

			}
			if(m_addExtIpRetry && !m_exit)
			{
				std::cout << "Calling m_addExtIpRetry!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(addExtIp(extIp, defaultGateway, netmask, ntpdRestart) == 0)
				{
					std::cout << "m_addExtIpRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					m_addExtIpRetry = false;
					m_isRetryOngoing = false;
				}
			}
			if(m_rmExtIpRetry && !m_exit)
			{
				std::cout << "Calling m_rmExtIpRetry!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(rmExtIp() == 0)
				{
					std::cout << "m_rmExtIpRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					m_rmExtIpRetry = false;
					m_isRetryOngoing = false;
				}
			}
			if(m_rmNtpRefRetry && !m_exit)
			{
				std::cout << "Calling m_rmNtpRefRetry!!!!!!!!!!!!!!!!!!!" << std::endl;
				
				ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

				int size = m_timeRefRemove.size();
				bool snmp_failure = false;

				while (size > 0 && !snmp_failure) {
					std::string m_timeRef = m_timeRefRemove.at(0);

					if(rmTimeRefNtp(m_timeRef) == 0)
					{
						std::cout << "m_addNtpRefRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
						m_timeRefRemove.erase(m_timeRefRemove.begin());
					} else
						snmp_failure = true;

					size = m_timeRefRemove.size();
				}

				if(size == 0)
				{
					m_rmNtpRefRetry = false;
					m_isRetryOngoing = false;
				}
			}
			if(m_rmNtpHpRefRetry && !m_exit)
			{
				std::cout << "Calling m_rmNtpHpRefRetry!!!!!!!!!!!!!!!!!!!" << std::endl;

				ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

				int size = m_timeRefRemove.size();
				bool snmp_failure = false;

				while (size > 0 && !snmp_failure) {
					std::string m_timeRef = m_timeRefRemove.at(0);

					if(rmTimeRefNtphp(m_timeRef) == 0)
					{
						std::cout << "m_addNtpRefRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
						m_timeRefRemove.erase(m_timeRefRemove.begin());
					} else
						snmp_failure = true;

					size = m_timeRefRemove.size();
				}

				if(size == 0)
				{
					m_rmNtpHpRefRetry = false;
					m_isRetryOngoing = false;
				}
			}
			if(m_stopNtpRetry && !m_exit)
			{
				std::cout << "Calling m_stopNtpRetry!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(stopNtp() == 0)
				{
					std::cout << "m_stopNtpRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					m_stopNtpRetry = false;
					m_isRetryOngoing = false;
				}
			}
			if(m_stopNtpHpRetry && !m_exit)
			{
				std::cout << "Calling m_stopNtpHpRetry!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(stopNtphp() == 0)
				{
					std::cout << "m_stopNtpHpRetry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					m_stopNtpHpRetry = false;
					m_isRetryOngoing = false;
				}
			}
			if(m_setTime && !m_exit)
			{
				cout << "calling setsystemtime function " << endl;
				if(m_snmpManager->setSystemTime())
				{
					cout << "setsystemtime function SUCCESS" << endl;
					//m_setTime = false;
				}
				else
				{
					if (switchAddress()) {
						if(m_snmpManager->setSystemTime())
						{
							cout << "setsystemtime function SUCCESS" << endl;
							//m_setTime = false;
						}
					}
				}
				cout << "Restarting NTP after setting system time " << endl;
				if(m_snmpManager->setGenNtpEntConfigComplete())
				{
					cout << "setGenNtpEntConfigComplete function SUCCESS" << endl;
				}
				else
				{
					if (switchAddress())
					{
						if(m_snmpManager->setGenNtpEntConfigComplete())
						{
							cout << "setGenNtpEntConfigComplete function SUCCESS" << endl;
						}
					}
					//m_setTime = false;
				}

				if(m_snmpManager->setGenNtpEntNtpdRestart())
				{
					cout << "setGenNtpEntNtpdRestart function SUCCESS" << endl;
					m_setTime = false;
				}
				else
				{
					if(switchAddress())
					{
						if(m_snmpManager->setGenNtpEntNtpdRestart())
						{
							cout << "setGenNtpEntNtpdRestart function SUCCESS" << endl;
							m_setTime = false;
						}
					}
				}

			}
			if(m_cleanup && !m_exit)
			{
				//int cleanUpRes = -1;
				std::cout << __FUNCTION__ << "@" << __LINE__ <<"calling cleanup function" <<std::endl; //mychange
				cout << "calling cleanup function" << endl;
				if(cleanup())
				{
					cout << "cleanup function SUCCESS" << endl;
					if(rmExtIp() == 0)
					{
						cout << "rmExtIp() function SUCCESS" << endl;
						m_cleanup = false;
						m_isRetryOngoing = false;
					}
				}

			}

			//TODO: Can this be definitely removed?
			//TO BE REMOVED AFTER APBM IS READY --begin
//			{
//				//get port 27 (1GE Front port) status
//				int adminStatus27 = 0;
//				m_snmpManager->setIpAddress(m_ipA);
//				if(!m_snmpManager->getIfAdminStatus27(&adminStatus27))
//				{
//					m_snmpManager->setIpAddress(m_ipB);
//					if(!m_snmpManager->getIfAdminStatus27(&adminStatus27))
//					{
//						cout << "Could not get ADMINSTATUS FOR 1GE FRONT PORT\n" << endl;
//					}
//				}
//				else//if down, make it up
//				{
//					if(adminStatus27 != 1)
//					{
//						if(!m_snmpManager->setIfAdminStatus27up())
//						{
//							if(m_ipA.compare(m_snmpManager->getIpAddress()) == 0)
//								m_snmpManager->setIpAddress(m_ipB);
//							else
//								m_snmpManager->setIpAddress(m_ipA);
//							if(!m_snmpManager->setIfAdminStatus27up())
//							{
//								cout << "************* COULD NOT SET IFADMINSTATUS OF 1GE FRONT PORT TO UP *************\n" << endl;
//							}
//						}
//					}
//				}
//
//			}
			//TO BE REMOVED AFTER APBM IS READY --end


		}//while

		cout << "THREAD EXITING NOW " << m_ipA << endl;
		m_running = false;
		return 0;

}


//TODO: Remove all this!!!
//void FIXS_TREFC_ApzScxHandler::handleCommandEvent ()
//{
// 	TraceInOut inOut(__FUNCTION__);
//	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "m_eventQueue.size()"<< m_eventQueue.queueSize() << std::endl;
//	if (m_eventQueue.queueEmpty()) return;
//
//	//get event to analyze
//	FIXS_TREFC_ApzScxHandler::ApzScxEventId qEventInfo = TIMEREF_DEFAULT_NO_EVENT;
//	qEventInfo = m_eventQueue.getFirst();
//
//	switch(qEventInfo)
//	{
//		case TIMEREF_ADD_NTP:
//			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Got event for timerefadd ntp" << std::endl;
//			//
//
//	}
//
//	//switch state
//	switchState(qEventInfo);
//
//	m_eventQueue.popData(); //processing ok
//
//
//
//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl <<
//				" ------------ CURRENT STATE: " << getStateName().c_str() << " ------------ "
//				<< std::endl << std::endl;
//
//	if (traceObj->ACS_TRA_ON())
//	{
//		char tmpStr[512] = {0};
//		snprintf(tmpStr, sizeof(tmpStr) - 1,
//			"[%s@%d] After switchState - CURRENT STATE: %s "
//			, __FUNCTION__, __LINE__, getStateName().c_str());
//		traceObj->ACS_TRA_event(1,tmpStr);
//	}
//
//}
//
//
//
//FIXS_TREFC_ApzScxHandler::EventQueue::~EventQueue()
//{
//	{
//	//	FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
//		Lock lock;
//		while (!m_qEventInfo.empty()) {
//			m_qEventInfo.pop_front();
//		}
//	}
////	if ((m_queueHandle != INVALID_HANDLE_VALUE) && (m_queueHandle != 0)) CloseHandle(m_queueHandle);
////	try {DeleteCriticalSection(&m_queueCriticalSection);} catch (...) {}
//
//}
//
//
//
////## Other Operations (implementation)
//FIXS_TREFC_ApzScxHandler::ApzScxEventId FIXS_TREFC_ApzScxHandler::EventQueue::getFirst ()
//{
////	FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
//	Lock lock;
//	ApzScxEventId result = SCX_DEFAULT_NO_EVENT;
//	if (!m_qEventInfo.empty())
//	{
//		result = m_qEventInfo.front();
//	}
//	return result;
//
//}
//
// void FIXS_TREFC_ApzScxHandler::EventQueue::popData ()
//{
////	 FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
//	 Lock lock;
//	if (!m_qEventInfo.empty()) {
//		m_qEventInfo.pop_front();
//		//if ((m_qEventInfo.size() == 0) && ((m_queueHandle != -1) && (m_queueHandle != 0))) //ResetEvent(m_queueHandle);	FIXS_CCH_Event::ResetEvent(m_queueHandle);
//	}
//
//}
//
// void FIXS_TREFC_ApzScxHandler::EventQueue::pushData (FIXS_TREFC_ApzScxHandler::ApzScxEventId info)
//{
////	 FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
//	 Lock lock;
//	{
//		 bool exist = false;
//		 for (unsigned int i=0; i<m_qEventInfo.size();i++)
//		 {
//			//check first element
//			 FIXS_TREFC_ApzScxHandler::ApzScxEventId first;
//			first = getFirst();
//			if (first == info )	exist = true;
//
//			m_qEventInfo.pop_front();
//			m_qEventInfo.push_back(first);
//
//		 }
//
//		 if (!exist)
//		 {
//			 m_qEventInfo.push_back(info);
//
//			//	if ((m_qEventInfo.size() == 1) && (m_queueHandle != INVALID_HANDLE_VALUE))
//			int retries = 0;
//
//			while (FIXS_TREFC_Event::SetEvent(m_queueHandle) == 0)
//			{
//				cout << "Set m_queueHandle.... retries: " << retries << endl;
//				if( ++retries > 10) break;
//				sleep(1);
//			}
//		 }
//	}
//}
//
// bool FIXS_TREFC_ApzScxHandler::EventQueue::queueEmpty ()
//{
////	 FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
//	 Lock lock;
//	return m_qEventInfo.empty();
//}
//
// size_t FIXS_TREFC_ApzScxHandler::EventQueue::queueSize ()
//{
//	// FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
//	 Lock lock;
//	return m_qEventInfo.size();
//}
//
// FIXS_TREFC_ApzScxHandler::ApzScxEventId FIXS_TREFC_ApzScxHandler::EventQueue::getItem (int index)
//{
//
//	//FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
//	Lock lock;
//	std::list<ScxEventId>::iterator it = m_qEventInfo.begin();
//	while (index-- && (it != m_qEventInfo.end())) ++it;
//
//	if (it == m_qEventInfo.end())
//		return TIMEREF_DEFAULT_NO_EVENT;
//	else
//		return *it;
//
//}




int FIXS_TREFC_ApzScxHandler::addExtIp(string ip, string defaultGateway, string netmask, bool ntpRestart)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Adding external ip configuration for :" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	m_snmpManager->setIpAddress(m_ipA);

	if(!m_snmpManager->setsysMgrAliasRowStatus(6,1)) //destroy
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setsysMgrAliasRowStatus(6,1))
		{
			cout << "FAILURE setting sysMgrAliasRowStatus to 6, try setting to 5" << endl;
			if(!m_snmpManager->setsysMgrAliasRowStatus(5,1))
			{
				if (!switchAddress()) return 13;
				if(!m_snmpManager->setsysMgrAliasRowStatus(5,1))
				{
					cout << "FAILURE setting sysMgrAliasRowStatus to 5, try setting to 2" << endl;
					if(!m_snmpManager->setsysMgrAliasRowStatus(2,1))
					{
						if (!switchAddress()) return 13;
						if(!m_snmpManager->setsysMgrAliasRowStatus(2,1))
						{
							cout << "FAILURE setting sysMgrAliasRowStatus to 2" << endl;
							//return 13; //SNMP Failue(??)
						}
					}
				}
			}

		}
	}

	if(!m_snmpManager->setsysMgrAliasRowStatus(5,1)) //create and wait
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setsysMgrAliasRowStatus(5,1))
		{
			cout << "FAILURE setting sysMgrAliasRowStatus to 5" << endl;
			//return 13; //SNMP Failue(??)
		}
	}

	if(!m_snmpManager->setsysMgrAliasIpAddr(1, ip))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setsysMgrAliasIpAddr(1, ip))
		{
			cout << "FAILURE setting sysMgrAliasIpAddr to " << ip << endl;
			return 13; //SNMP_FAILURE (??)
		}
	}


	if(!m_snmpManager->setsysMgrAliasDefaultGateway(1, defaultGateway))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setsysMgrAliasDefaultGateway(1, defaultGateway))
		{
			cout << "FAILURE setting sysMgrAliasDefaultGateway to " << defaultGateway << endl;
			return 13; //SNMP_FAILURE (??)
		}
	}


	if(!m_snmpManager->setsysMgrAliasNetMask(1, netmask))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setsysMgrAliasNetMask(1, netmask))
		{
			cout << "FAILURE setting sysMgrAliasNetMask to " << netmask << endl;
			return 13; //SNMP_FAILURE (??)
		}
	}

	if(environment != TREFC_Util::SMX) {
		if(!m_snmpManager->setsysMgrAliasRealDevName(1, "eth0"))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setsysMgrAliasRealDevName(1, "eth0"))
			{
				cout << "FAILURE setting sysMgrAliasRealDevName to eth0" << endl;
				return 13; //SNMP_FAILURE (??)
			}
		}
	}

	if(!m_snmpManager->setsysMgrAliasRowStatus(1,1)) //make row active
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setsysMgrAliasRowStatus(1,1))
		{
			cout << "FAILURE setting sysMgrAliasRowStatus to 1" << endl;
			return 13; //SNMP_FAILURE (??)
		}
	}

	bool err = false;
	if(environment != TREFC_Util::SMX) {
		for(int i = 0; i < 8; i++)
		{
			int error;

			if(!m_snmpManager->setGenNtpEntWhiteRowStatus(6, i, error))
			{
				if (!switchAddress()) return 13;
				if(!m_snmpManager->setGenNtpEntWhiteRowStatus(6, i, error))
				{
					cout << "FAILURE setting GenNtpEntWhiteRowStatus to 6 for index " << i << endl;
					continue;
				}
			}
			switch(i)
			{
				case 0:
					if(!m_snmpManager->setGenNtpEntWhiteInterface(0, "eth0"))
					{
						if (!switchAddress()) return 13;
						if(!m_snmpManager->setGenNtpEntWhiteInterface(0, "eth0"))
						{
							cout << "FAILURE setting setGenNtpEntWhiteInterface to eth0 for index 0" << endl;
							err = true;
							break;
						}
					}
	
					if(!m_snmpManager->setGenNtpEntWhiteRowStatus(1, 0))
					{
						if (!switchAddress()) return 13;
						if(!m_snmpManager->setGenNtpEntWhiteRowStatus(1, 0))
						{
							cout << "FAILURE setting GenNtpEntWhiteRowStatus to 1 for index 0" << endl;
							err = true;
							break;
						}
					}
					break;
	
				case 1:
					if(!m_snmpManager->setGenNtpEntWhiteInterface(1, "eth1"))
					{
						if (!switchAddress()) return 13;
						if(!m_snmpManager->setGenNtpEntWhiteInterface(1, "eth1"))
						{
							cout << "FAILURE setting GenNtpEntWhiteInterface to eth1 for index 1" << endl;
							err = true;
						}
					}
	
					if(!m_snmpManager->setGenNtpEntWhiteRowStatus(1, 1))
					{
						if (!switchAddress()) return 13;
						if(!m_snmpManager->setGenNtpEntWhiteRowStatus(1, 1))
						{
							cout << "FAILURE setting GenNtpEntWhiteRowStatus to 1 for index 1" << endl;
							err = true;
							break;
						}
					}
					break;
	
				case 2:
					if(!m_snmpManager->setGenNtpEntWhiteInterface(2, "eth0:0"))
					{
						if (!switchAddress()) return 13;
						if(!m_snmpManager->setGenNtpEntWhiteInterface(2, "eth0:0"))
						{
							cout << "FAILURE setting GenNtpEntWhiteInterface to eth0:0 for index 2" << endl;
							err = true;
						}
					}
					if(!m_snmpManager->setGenNtpEntWhiteRowStatus(1, 2))
					{
						if (!switchAddress()) return 13;
						if(!m_snmpManager->setGenNtpEntWhiteRowStatus(1, 2))
						{
							cout << "FAILURE setting GenNtpEntWhiteRowStatus to 1 for index 2" << endl;
							err = true;
							break;
						}
					}
					break;
			} //end switch case
			if(err)
				return 13; //SNMP Failure (??)
	
		}//end for
	}


	if(ntpRestart)
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Restart ntp deamon after adding "<< std::endl; //mychange
		//restart ntp daemon after adding
		if(!m_snmpManager->setGenNtpEntConfigComplete())
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setGenNtpEntConfigComplete())
			{
				cout << "FAILURE setting GenNtpEntConfigComplete"<< endl;
				return 13; //SNMP_FAILURE (??)
			}
		}
		else
		{
			cout << "Set GenNtpEntConfigComplete"<<endl;
		}
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntNtpdRestart "<< std::endl; //mychange
		if(!m_snmpManager->setGenNtpEntNtpdRestart())
		{
                        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntNtpdRestart inside: "<< std::endl; //mychange
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setGenNtpEntNtpdRestart())
			{
				cout << "FAILURE setting GenNtpEntNtpdRestart"<< endl;
				return 13; //SNMP_FAILURE (??)
			}
		}
 
		else
		{
			cout << "Set GenNtpEntNtpdRestart"<<endl;
		}
	}


	return 0;
}



int FIXS_TREFC_ApzScxHandler::rmExtIp()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Removing external ip configuration for :" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	//int rowStatus = 0;
	m_snmpManager->setIpAddress(m_ipA);
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();

	if(!m_snmpManager->setsysMgrAliasRowStatus(6,1)) //destroy row
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setsysMgrAliasRowStatus(6,1))
		{
			cout << "FAILURE setting sysMgrAliasRowStatus to 6" << endl;
			return 13; //SNMP_FAILURE (??)

		}

	}
	if(environment != TREFC_Util::SMX) {
		for (int i = 0; i < 8; i++)
		{
			if(!m_snmpManager->setGenNtpEntWhiteRowStatus(6, i))
			{
				if (!switchAddress()) return 13;
				if(!m_snmpManager->setGenNtpEntWhiteRowStatus(6, i))
				{
					cout << "FAILURE setting GenNtpEntWhiteRowStatus to 6 for index " << i << endl;
					return 13;
				}
			}
		}
	}
	return 0;
}

int FIXS_TREFC_ApzScxHandler::addTimeRefNtp(string timeRefIp)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Adding time reference NTP for :" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	//check first free index

	int freeIndex = -1;
	string tempNtp("");

	m_snmpManager->setIpAddress(m_ipA);

	for (int i = 0; i < 8; i++)
	{
		if(!m_snmpManager->getGenNTPEntServer(i, &tempNtp))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->getGenNTPEntServer(i, &tempNtp))
			{
				freeIndex = i;
				break;
			}
		}
	}//end for

	if (freeIndex == -1) {
		cout << "FAILURE: No freeindex found" << endl;
		return 1; //Free index not found
	}

	cout << "Free index :" << freeIndex << endl;

	if(!m_snmpManager->setGenNtpEntServer(freeIndex, timeRefIp))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntServer(freeIndex, timeRefIp))
		{
			cout << "FAILURE setting GenNtpEntServer to "<< timeRefIp << endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set GenNtpEntServer to "<< timeRefIp << endl;
	}


	if(!m_snmpManager->setGenNtpEntSvrRowStatus(1, freeIndex))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntSvrRowStatus(1, freeIndex))
		{
			cout << "FAILURE setting GenNtpEntServerRowStatus to 1 "<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set GenNtpEntServerRowStatus to 1"<<endl;
	}

	//restart ntp daemon after adding
	if(!m_snmpManager->setGenNtpEntConfigComplete())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntConfigComplete())
		{
			cout << "FAILURE setting GenNtpEntConfigComplete"<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set GenNtpEntConfigComplete"<<endl;
	}

        std::cout << __FUNCTION__ << "@" << __LINE__ <<"before setGenNtpEntNtpdRestart" <<std::endl; //mychange
	if(!m_snmpManager->setGenNtpEntNtpdRestart())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntNtpdRestart())
		{
			cout << "FAILURE setting GenNtpEntNtpdRestart"<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set GenNtpEntNtpdRestart"<<endl;
	}
	return 0;
}


int FIXS_TREFC_ApzScxHandler::addTimeRefNtphp(string timeRefIp)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Adding time reference NTPHP for :" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	//check first free index

	int freeIndex = 0;
	string tempNtp("");
	int tempStatus = 0;

	m_snmpManager->setIpAddress(m_ipA);

	for (int i = 0; i < 8; i++)
	{
		if(!m_snmpManager->getStsiNtpRefIp(i, &tempNtp))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->getStsiNtpRefIp(i, &tempNtp))
			{
				cout << "FAILURE getting StsiNtpRefIp" << endl;
				return 13; //SNMP_FAILURE (??)
			}
		}


		if(!m_snmpManager->getStsiNtpRefRowStatus(i, &tempStatus))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->getStsiNtpRefRowStatus(i, &tempStatus))
			{
				cout << "FAILURE getting StsiNtpRefRowStatus" << endl;
				return 13; //SNMP_FAILURE (??)
			}
		}

		if (tempNtp.compare("0.0.0.0") == 0 || tempStatus == 2) {
			freeIndex = i;
			break;
		}

	}//end for

	if (freeIndex == -1) {
		cout << "FAILURE: No freeindex found" << endl;
		return 1; //Free index not found
	}

	cout << "Free index :" << freeIndex << endl;
	if(!m_snmpManager->setStsiNtpRefRowStatus(2, freeIndex))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiNtpRefRowStatus(2, freeIndex))
		{
			cout << "FAILURE setting StsiSntpIfRowStatus to 2 "<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set StsiSntpIfRowStatus to 2"<<endl;
	}

	if(!m_snmpManager->setStsiNtpRefIp(freeIndex, timeRefIp))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiNtpRefIp(freeIndex, timeRefIp))
		{
			cout << "FAILURE setting StsiNtpRefIp to "<< timeRefIp << endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set StsiNtpRefIp to "<< timeRefIp << endl;
	}

	if(!m_snmpManager->setStsiNtpRefRowStatus(1, freeIndex))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiNtpRefRowStatus(1, freeIndex))
		{
			cout << "FAILURE setting StsiNtpRefRowStatus to 1 "<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set StsiNtpRefRowStatus to 1"<<endl;
	}
	return 0;
}


int FIXS_TREFC_ApzScxHandler::rmTimeRefNtp(string timeRefIp)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Removing time reference NTP for :" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	//find index of entry to be removed
	m_snmpManager->setIpAddress(m_ipA);
	int freeIndex = 0;
	bool found = false;
	string tempNtp("");


	for (int i = 0; i < 8; i++)
	{
		tempNtp.clear();
		if(!m_snmpManager->getGenNTPEntServer(i, &tempNtp))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->getGenNTPEntServer(i, &tempNtp))
			{
				continue;
			}
		}
		if(timeRefIp.compare(tempNtp) == 0)
		{
			cout << "found entry in genNtpEntServerTable" << endl;
			found = true;
			freeIndex = i;
			break;
		}
	}//end for

	if(!found)
	{
		cout << "Did not find entry in table " << endl;
		return 12; //change error code later
	}

	if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, freeIndex))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, freeIndex))
		{
			cout << "FAILURE to set genNtpEntServerRowStatus to 6" << endl;
		}
	}
	else
	{
		cout << "Set genNtpEntServerRowStatus to 6" << endl;
	}

	//restart ntp daemon after removing entry
	if(!m_snmpManager->setGenNtpEntConfigComplete())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntConfigComplete())
		{
			cout << "FAILURE setting GenNtpEntConfigComplete"<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set GenNtpEntConfigComplete"<<endl;
	}
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntNtpdRestart  "<< std::endl; //mychange
	if(!m_snmpManager->setGenNtpEntNtpdRestart())
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntNtpdRestart inside: "<< std::endl; //mychange
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntNtpdRestart())
		{
			cout << "FAILURE setting GenNtpEntNtpdRestart"<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
		cout << "Set GenNtpEntNtpdRestart"<<endl;
	}
	return 0;
}

int FIXS_TREFC_ApzScxHandler::rmTimeRefNtphp(string timeRefIp)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Removing time reference NTPHP for :" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	//find index of entry to be removed
	m_snmpManager->setIpAddress(m_ipA);
	int freeIndex = 0;
	bool found = false;
	string tempNtp("");

	for (int i = 0; i < 8; i++)
	{
		tempNtp.clear();
		if(!m_snmpManager->getStsiNtpRefIp(i, &tempNtp))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->getStsiNtpRefIp(i, &tempNtp))
			{
				continue;
			}

		}

		cout << "IP ADDRESS TO FIND: " << timeRefIp << std::endl;
		cout << "IP ADDRESS FOUND: " << tempNtp << std::endl;
		if(timeRefIp.compare(tempNtp) == 0)
		{
			cout << "found entry in stsiNtpRefTable" << endl;
			found = true;
			freeIndex = i;
			break;
		}
	}//end for

	if(!found)
	{
		cout << "Did not find entry in table " << endl;
		return 12; //change error code later
	}

	if(!m_snmpManager->setStsiNtpRefRowStatus(2, freeIndex))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiNtpRefRowStatus(2, freeIndex))
		{
			cout << "FAILURE to set stsiNtpRefRowStatus to 2" << endl;
			return 13; //SNMP Failure (??)
		}

	}
	else
	{
		cout << "Set stsiNtpRefRowStatus to 2" << endl;
	}

	if(!m_snmpManager->setStsiNtpRefIp(freeIndex, "0.0.0.0"))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiNtpRefIp(freeIndex, "0.0.0.0"))
		{
			cout << "FAILURE to set StsiNtpRefIp to 0.0.0.0" << endl;
			//return 13; //SNMP Failure (??)
		}

	}
	else
	{
		cout << "Set StsiNtpRefIp to 0.0.0.0" << endl;
	}
	return 0;
}

int FIXS_TREFC_ApzScxHandler::startNtp(map<string, string> tRefList)
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Enter startNtp "<< std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Starting NTP for SCX with:" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;

	if (tRefList.size() > 8) {
		cout << "Invalid Time Reference List" << endl;
		return 1;
	}


	//clear genNtpEntServerTable
	for(int i = 0; i < 8; i++)
	{
		int error = FIXS_TREFC_SNMPManager::GETSET_SNMP_OK;

		if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i, error))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i, error))
			{
				cout << "FAILURE to set GenNtpEntSvrRowStatus to 6 at index " << i << endl;
				if (error == FIXS_TREFC_SNMPManager::GETSET_SNMP_STATUS_TIMEOUT)
					return 13;
				else
					continue;
				//return 13; //SNMP Failure (??)
			}
		}

	} //end for
	
	std::cout << __FUNCTION__ << "@" << __LINE__ <<" Setting Role(1): "<< std::endl;
	if(!m_snmpManager->setStsiTuRole(1)) //set to master(1)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" Setting Role:inside "<< std::endl; //mychange
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiTuRole(1))
		{
			cout << "FAILURE to set StsiTuRole to 1" << endl;
			return 13; //SNMP Failure (??)
		}
	        std::cout << __FUNCTION__ << "@" << __LINE__ <<" SET successful done on Role: " << std::endl; //mychange
	}

	//Fill genNtpEntServerTable with IPs from list
	std::map<string, string>::iterator it;
	int index = 0;
	for(it = tRefList.begin(); it != tRefList.end(); it++)
	{
		if(!m_snmpManager->setGenNtpEntServer(index, it->first))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setGenNtpEntServer(index, it->first))
			{
				cout << "FAILURE to set GenNtpEntServer to " << it->first <<" at index " << index << endl;
				return 13; //SNMP Failure (??)
			}
		}
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntSvrRowStatus"<< std::endl; //mychange
		if(!m_snmpManager->setGenNtpEntSvrRowStatus(1, index))
		{
                        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntSvrRowStatus inside: "<< std::endl; //mychange
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setGenNtpEntSvrRowStatus(1, index))
			{
				cout << "FAILURE to set GenNtpEntSvrRowStatus to 1 at index " << index << endl;
				return 13; //SNMP Failure (??)
			}
                        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set done GenNtpEntSvrRowStatus inside: "<< std::endl; //mychange
		}
		index++;
		cout<<index<<endl;
	} //end for

	//configcomplete
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntConfigComplete "<< std::endl; //mychange
	if(!m_snmpManager->setGenNtpEntConfigComplete())
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntConfigComplete inside"<< std::endl; //mychange
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntConfigComplete())
		{
			cout << "FAILURE setting GenNtpEntConfigComplete"<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}

	//ntpdrestart
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntNtpdRestart "<< std::endl; //mychange
	if(!m_snmpManager->setGenNtpEntNtpdRestart())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntNtpdRestart())
		{
			cout << "FAILURE setting GenNtpEntNtpdRestart"<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}
	else
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntNtpdRestart "<< std::endl; //mychange
		//cout << "Set GenNtpEntNtpdRestart"<<endl;
	}

	return 0;
}


int FIXS_TREFC_ApzScxHandler::startNtphp(map<string, string> tRefList, string otherScxIpA, string otherScxIpB)
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Enter startNtphp "<< std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Starting NTP for SCX with:" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	string ip("");

	unsigned int vlan = 0;

	if (tRefList.size() > 8) {
		cout << "Invalid Time Reference List" << endl;
		return 1;
	}

	//Set system time to stop ntpd
	//if (!switchAddress()) return 13;
	if(!m_snmpManager->setSystemTime())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setSystemTime())
		{
			cout << "FAILURE to set system time, cannot stop NTP" << endl;
			return 13; //SNMP Failure (??)
		}
	}

	//clear stsiNtpRefTable
/*	for(int i = 0; i < 8; i++)
	{
		int error;
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set setStsiNtpRefRowStatus "<< std::endl; //mychange
		if(!m_snmpManager->setStsiNtpRefRowStatus(2, i, error))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setStsiNtpRefRowStatus(2, i, error))
			{
				cout << "FAILURE to set stsiNtpRefRowStatus to 2 at index " << i << endl;
				if (error == FIXS_TREFC_SNMPManager::GETSET_SNMP_STATUS_TIMEOUT)
					return 13;
				else
					continue;
				//return 13; //SNMP Failure (??)
				//return 13; //SNMP Failure (??)
			}
		}
	}*/ //end for

	//Fill stsiNtpRefTable with IPs from list
	/*std::map<string, string>::iterator it;
	int index = 0;
	for(it = tRefList.begin(); it != tRefList.end(); it++)
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set setStsiNtpRefIp "<< std::endl; //mychange
		if(!m_snmpManager->setStsiNtpRefIp(index, it->first))
		{
                        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set setStsiNtpRefIp "<< std::endl; //mychange
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setStsiNtpRefIp(index, it->first))
			{
				cout << "FAILURE to set stsiNtpRefIp to " << it->first <<" at index " << index << endl;
				return 13; //SNMP Failure (??)
			}
                        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set done setStsiNtpRefIp "<< std::endl; //mychange
		}
                std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set setStsiNtpRefRowStatus "<< std::endl; //mychange
		if(!m_snmpManager->setStsiNtpRefRowStatus(1, index))
		{
                        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set setStsiNtpRefRowStatus "<< std::endl; //mychange
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setStsiNtpRefRowStatus(1, index))
			{
				cout << "FAILURE to set stsiNtpRefRowStatus to 1 at index " << index << endl;
				return 13; //SNMP Failure (??)
			}
                        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set setStsiNtpRefRowStatus "<< std::endl; //mychange
		}
		index++;
		cout<<index<<endl;
	} //end for

	//Fill stsiSntpIfTable with own ip addresses
	for(int i = 1; i <= 2; i++)
	{
		ip.clear();
		vlan = 0;
		if(!m_snmpManager->setStsiSntpIfRowStatus(2, i))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setStsiSntpIfRowStatus(2, i))
			{
				cout << "FAILURE to set StsiSntpIfRowStatus to 2 at index " << i << endl;
				return 13; //SNMP Failure (??)
			}
		}

		if(i == 1)
		{
			ip = m_ipA;
			vlan = 4046;
		}
		else
		{
			ip = m_ipB;
			vlan = 4047;
		}

		if(!m_snmpManager->setStsiSntpIfIp(i, ip))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setStsiSntpIfIp(i, ip))
			{
				cout << "FAILURE to set StsiSntpIfIp to " << ip <<" at index " << i << endl;
				return 13; //SNMP Failure (??)
			}
		}

		if(!m_snmpManager->setStsiSntpIfIpMask(i, "255.255.255.0"))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setStsiSntpIfIpMask(i, "255.255.255.0"))
			{
				cout << "FAILURE to set StsiSntpIfIpMask to 255.255.255.0 at index " << i << endl;
				return 13; //SNMP Failure (??)
			}
		}

		if(!m_snmpManager->setStsiSntpIfVlan(i, vlan))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setStsiSntpIfVlan(i, vlan))
			{
				cout << "FAILURE to set StsiSntpIfVlan to " << vlan <<" at index " << i << endl;
				return 13; //SNMP Failure (??)
			}
		}

		if(!m_snmpManager->setStsiSntpIfRowStatus(1, i))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setStsiSntpIfRowStatus(1, i))
			{
				cout << "FAILURE to set StsiSntpIfRowStatus to 1 at index " << i << endl;
				return 13; //SNMP Failure (??)
			}
		}

	} //end for

	//Fill stsiSntpPartnerTable
	if(!m_snmpManager->setStsiSntpPartnerIp(1, otherScxIpA))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiSntpPartnerIp(1, otherScxIpA))
		{
			cout << "FAILURE to set StsiSntpPartnerIp to " << otherScxIpA << "at index 1"<< endl;
			return 13; //SNMP Failure (??)
		}
	}

	if(!m_snmpManager->setStsiSntpPartnerIp(2, otherScxIpB))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiSntpPartnerIp(2, otherScxIpB))
		{
			cout << "FAILURE to set StsiSntpPartnerIp to " << otherScxIpB << "at index 2"<< endl;
			return 13; //SNMP Failure (??)
		}
	}
	//end of stsiSntpPartnerTable setting

	//syncInput
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Setting SyncInput(6): " << std::endl; //mychange
	if(!m_snmpManager->setStsiSyncInput(6)) //set to ntp(6)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" Setting SyncInput(6): " << std::endl; //mychange		
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiSyncInput(6))
		{
			cout << "FAILURE to set StsiSyncInput to 6" << endl;
			return 13; //SNMP Failure (??)
		}
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" SET successful done on SyncInput(6): " << std::endl; //mychange
	}
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" After Setting SyncInput(6): " << std::endl; //mychange
	//turole
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Setting Role: " << std::endl; //mychange
	if(!m_snmpManager->setStsiTuRole(1)) //set to master(1)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting Role: " <<std::endl; //mychange
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiTuRole(1))
		{
			cout << "FAILURE to set StsiTuRole to 1" << endl;
			return 13; //SNMP Failure (??)
		}
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" SET sucessful done on Role: " << std::endl; //mychange
	}

	//todRef
	if(!m_snmpManager->setStsiTodRef(1)) //set to ntp(1)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" Setting StsiTodRef(1): " <<std::endl; //mychange
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiTodRef(1))
		{
			cout << "FAILURE to set StsiTodRef to 1" << endl;
			return 13; //SNMP Failure (??)
		}
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" SET successful done on StssiTodRef: " << std::endl; //mychange
	}

	//setCpuClock
	if(!m_snmpManager->setStsiSntpSetCpuClock(1)) //set to enabled(1)
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiSntpSetCpuClock(1))
		{
			cout << "FAILURE to set StsiSntpSetCpuClock to 1" << endl;
			return 13; //SNMP Failure (??)
		}
	}

	//clientMode
	if(!m_snmpManager->setStsiSntpClientMode(2)) //set to predefined(2)
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiSntpClientMode(2))
		{
			cout << "FAILURE to set StsiSntpClientMode to 2" << endl;
			return 13; //SNMP Failure (??)
		}
	}

	//todServer
	if(!m_snmpManager->setStsiTodServer(2)) //set to negotiation(2)
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiTodServer(2)) //set to negotiation(2)
		{
			cout << "FAILURE to set StsiTodServer to 2" << endl;
			return 13; //SNMP Failure (??)
		}
	}*/

	return 0;
}

int FIXS_TREFC_ApzScxHandler::stopNtp()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Stopping NTP for SCX with:" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	//set systemTime
	if(!m_snmpManager->setSystemTime())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setSystemTime())
		{
			cout << "FAILURE to set system time, cannot stop NTP" << endl;
			return 13; //SNMP Failure (??)
		}
	}
//	//stsiTuRole
//	if(!m_snmpManager->setStsiTuRole(0)) //set to idle(0)
//	{
//		if(m_ipA.compare(m_snmpManager->getIpAddress()) == 0)
//			m_snmpManager->setIpAddress(m_ipB);
//		else
//			m_snmpManager->setIpAddress(m_ipA);
//		if(!m_snmpManager->setStsiTuRole(0))
//		{
//			cout << "FAILURE to set StsiTuRole to 0" << endl;
//			//return 13; //SNMP Failure (??) //Not an essential setting
//		}
//	}
//	//clear genNtpEntServerTable
//	for(int i = 0; i < 8; i++)
//	{
//		if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i))
//		{
//			if(m_ipA.compare(m_snmpManager->getIpAddress()) == 0)
//				m_snmpManager->setIpAddress(m_ipB);
//			else
//				m_snmpManager->setIpAddress(m_ipA);
//			if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i))
//			{
//				cout << "FAILURE to set GenNtpEntSvrRowStatus to 6 at index " << i << endl;
//				continue;
//				//return 13; //SNMP Failure (??)
//			}
//		}
//
//	} //end for
//	//syncInput
//	if(!m_snmpManager->setStsiSyncInput(0)) //set to noReference(0)
//	{
//		if(m_ipA.compare(m_snmpManager->getIpAddress()) == 0)
//			m_snmpManager->setIpAddress(m_ipB);
//		else
//			m_snmpManager->setIpAddress(m_ipA);
//		if(!m_snmpManager->setStsiSyncInput(0))
//		{
//			cout << "FAILURE to set StsiSyncInput to 0" << endl;
//			//return 13; //SNMP Failure (??)
//		}
//	}
	setCleanup();
	return 0;
}

int FIXS_TREFC_ApzScxHandler::stopNtphp()
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Enter stopNtphp()" << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Stopping NTPHP for SCX with:" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	//todServer
	/*if(!m_snmpManager->setStsiTodServer(0)) //set to disabled(0)
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiTodServer(0)) //set to disabled(0)
		{
			cout << "FAILURE to set StsiTodServer to 0" << endl;
			return 13; //SNMP Failure (??)
		}
	}
	//clientMode
	if(!m_snmpManager->setStsiSntpClientMode(0)) //set to disabled(0)
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiSntpClientMode(0))
		{
			cout << "FAILURE to set StsiSntpClientMode to 0" << endl;
			return 13; //SNMP Failure (??)
		}
	}
	//stsiTuRole
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Setting Role(0) to idle: " << std::endl; //mychange
	if(!m_snmpManager->setStsiTuRole(0)) //set to idle(0)
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiTuRole(0))
		{
			cout << "FAILURE to set StsiTuRole to 0" << endl;
			//return 13; //SNMP Failure (??) //Not an essential setting
		}
	}*/
	//set systemTime
	if(!m_snmpManager->setSystemTime())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setSystemTime())
		{
			cout << "FAILURE to set system time, cannot stop NTPHP" << endl;
			return 13; //SNMP Failure (??)
		}
	}
	//setCpuClock
	/*if(!m_snmpManager->setStsiSntpSetCpuClock(0)) //set to disabled(0)
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiSntpSetCpuClock(0))
		{
			cout << "FAILURE to set StsiSntpSetCpuClock to 0" << endl;
			//return 13; //SNMP Failure (??)
		}
	}
	//stsiTodRef
	if(!m_snmpManager->setStsiTodRef(0)) //set to none(0)
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiTodRef(0))
		{
			cout << "FAILURE to set StsiTodRef to 0" << endl;
			return 13; //SNMP Failure (??)
		}
	}


//	//clear genStsiNtpRefTable
//	for(int i = 0; i < 8; i++)
//	{
//		if(!m_snmpManager->setStsiNtpRefRowStatus(2, i))
//		{
//			if(m_ipA.compare(m_snmpManager->getIpAddress()) == 0)
//				m_snmpManager->setIpAddress(m_ipB);
//			else
//				m_snmpManager->setIpAddress(m_ipA);
//			if(!m_snmpManager->setStsiNtpRefRowStatus(2, i))
//			{
//				cout << "FAILURE to set stsiNtpRefRowStatus to 2 at index " << i << endl;
//				continue;
//				//return 13; //SNMP Failure (??)
//			}
//		}
//
//	} //end for */
	//syncInput
	if(!m_snmpManager->setStsiSyncInput(0)) //set to noReference(0)
	{
		if(m_ipA.compare(m_snmpManager->getIpAddress()) == 0)
			m_snmpManager->setIpAddress(m_ipB);
		else
			m_snmpManager->setIpAddress(m_ipA);
		if(!m_snmpManager->setStsiSyncInput(0))
		{
			cout << "FAILURE to set StsiSyncInput to 0" << endl;
			//return 13; //SNMP Failure (??)
		}
	}

	setCleanup();
	return 0;
}

int FIXS_TREFC_ApzScxHandler::getTimeRefStatusNtp(int *currentMode, string *activeRef)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	int mode = 0;
	string ref("");
	cout << "Received get NTP timeref status" << endl;
	if(!m_snmpManager->getGenNtpEntStatusCurrentMode(0, &mode))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->getGenNtpEntStatusCurrentMode(0, &mode))
		{
			cout << "FAILURE to get GenNtpEntStatusCurrentMode" << endl;
			return 13; //SNMP Failure (??)
		}
	}
	*currentMode = mode;
	if(!m_snmpManager->getGenNTPEntStatusActiveRefSourceId(0, &ref))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->getGenNTPEntStatusActiveRefSourceId(0, &ref))
		{
			cout << "FAILURE to get GenNTPEntStatusActiveRefSourceId" << endl;
			return 13; //SNMP Failure (??)
		}
	}
	*activeRef = ref;
	return 0;
}


int FIXS_TREFC_ApzScxHandler::getTimeRefStatusNtphp(int *sntpStatus, string *activeRef)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Received get NTPHP timeref status" << endl;
	int mode = 0, index = -1;
	string ref("");

	if(*sntpStatus == 1)//gps
	{
		if(!m_snmpManager->getstsiGpsStatus(0, &mode))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->getstsiGpsStatus(0, &mode))
			{
				cout << "FAILURE to get stsiGpsStatus" << endl;
				return 13; //SNMP Failure (??)
			}
		}
		cout<< "GPS STATUS = " << mode << endl;
		if (mode == 1) //syncRefOk
			*sntpStatus = 5; //CONNECTED
		else //other values
			*sntpStatus = 99; //NOT CONNECTED
		//*sntpStatus = mode;
		return 0;
	}
	if(!m_snmpManager->getStsiSntpStatus(0, &mode))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->getStsiSntpStatus(0, &mode))
		{
			cout << "FAILURE to get GenNtpEntStatusCurrentMode" << endl;
			return 13; //SNMP Failure (??)
		}
	}
	*sntpStatus = mode;
	if(!m_snmpManager->getStsiSntpSelectedRefIndex(0, &index))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->getStsiSntpSelectedRefIndex(0, &index))
		{
			cout << "FAILURE to get StsiSntpSelectedRefIndex" << endl;
			return 13; //SNMP Failure (??)
		}
	}
	if(index != -1)
	{
		if(!m_snmpManager->getStsiSntpAssocIp((unsigned int)index, &ref))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->getStsiSntpAssocIp((unsigned int)index, &ref))
			{
				cout << "FAILURE to get StsiSntpAssocIp" << endl;
				return 13; //SNMP Failure (??)
			}
		}
	}

	*activeRef = ref;
	return 0;
}

bool FIXS_TREFC_ApzScxHandler::reconfNtp()
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Enter reconfNtp " << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	int res = -1;
	if(!extIp.empty())
	{
		cout << "External ip entry exists, hence setting it" << endl;
		res = addExtIp(extIp, defaultGateway, netmask, false);
		if(res != 0)
		{
			cout << "Adding external ip failed" << endl;
			return false;
		}
	}
	else
	{
		cout << "No external ip setting for this SCX" << endl;
	}
	//cout << "Calling startNtp function" << endl; mychange
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Calling startNtp function " << std::endl; //mychange
	res = startNtp(tRefList);
	if(res != 0)
	{
		cout << "Starting NTP failed" << endl;
		return false;
	}
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Setting complete, returning true. " << std::endl; //mychange
	cout << "Settings complete, returning TRUEE" << endl;
	return true;
}


bool FIXS_TREFC_ApzScxHandler::reconfNtphp()
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Enter reconfNtphp: " << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	int res = -1;
	bool gps = false;
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	if(environment == TREFC_Util::SMX) {
		gps = true; //in SMX, for NTPHP, GPS is the only valid option
	}
	else
	{

		map<string, string>::iterator it;
		it = tRefList.find("GPS DEVICE");
		if (it != tRefList.end())
		{
			gps = true;
		}
	}
	
	if(!gps)
	{
		if(!extIp.empty())
		{
			cout << "External ip entry exists, hence setting it" << endl;
			res = addExtIp(extIp, defaultGateway, netmask, false);
			if(res != 0)
			{
				cout << "Adding external ip failed" << endl;
				return false;
			}
		}
		else
		{
			cout << "No external ip setting for this SCX" << endl;
		}
	}

	if(!gps)
	{
		//cout << "Calling startNtphp function" << endl; //mychange
                std::cout << __FUNCTION__ << "@" << __LINE__ <<"Calling startNtphp function" << std::endl; //mychange
		res = startNtphp(tRefList, m_otherScxA, m_otherScxB);
	}
	else
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntphp service with GPS: " << std::endl; //mychange
		cout << "Calling startNtphpGps function" << endl;
		res = startNtphpGps(m_otherScxA, m_otherScxB);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Starting ntphp service with GPS: " << std::endl; //mychange
	}

	if(res != 0)
	{
		cout << "Starting NTPHP failed" << endl;
		return false;
	}
	//cout << "Settings complete, returning TRUEE" << endl; //mychange
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting complete, returning true" << std::endl; //mychange
	return true;
}

int FIXS_TREFC_ApzScxHandler::startNtphpGps(string otherScxIpA, string otherScxIpB)
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter startNtphpGps()" << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "starting NTPHP with GPS device" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	string ip("");
	unsigned int vlan = 0;

	//Set system time to stop ntpd
	//if (!switchAddress()) return 13;
	if(!m_snmpManager->setSystemTime())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setSystemTime())
		{
			cout << "FAILURE to set system time, cannot stop NTP" << endl;
			return 13; //SNMP Failure (??)
		}
	}

	//syncInput
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"set syncINput(5)" << std::endl; //mychange
	if(!m_snmpManager->setStsiSyncInput(5)) //set to gps(5)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"set syncINput(5)" << std::endl; //mychange
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiSyncInput(5))
		{
			cout << "FAILURE to set StsiSyncInput to 5" << endl;
			return 13; //SNMP Failure (??)
		}
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"set syncINput(5) done" << std::endl; //mychange
	}

	//clear genNtpEntServerTable
	for(int i = 0; i < 8; i++)
	{
		int error = FIXS_TREFC_SNMPManager::GETSET_SNMP_OK;

		if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i, error))
		{
			if (!switchAddress()) return 13;
			if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i, error))
			{
				cout << "FAILURE to set GenNtpEntSvrRowStatus to 6 at index " << i << endl;
				if (error == FIXS_TREFC_SNMPManager::GETSET_SNMP_STATUS_TIMEOUT)
					return 13;
				else
					continue;
				//return 13; //SNMP Failure (??)
			}
		}

	} //end for

	//Fill genNtpEntServerTable with IPs from list
	std::map<string, string>::iterator it;
	int index = 0;
        
	string GpsIp="127.127.20.0";
	if(!m_snmpManager->setGenNtpEntServer(0, GpsIp))
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntServer(0, GpsIp))
		{
			cout << "FAILURE to set GenNtpEntServer to " << endl;
			return 13; //SNMP Failure (??)
		}
	}
	std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntSvrRowStatus"<< std::endl; //mychange
	if(!m_snmpManager->setGenNtpEntSvrRowStatus(1, 0))
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntSvrRowStatus inside: "<< std::endl; //mychange
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntSvrRowStatus(1, 0))
		{
			cout << "FAILURE to set GenNtpEntSvrRowStatus to 1 at index "  << endl;
			return 13; //SNMP Failure (??)
		}
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set done GenNtpEntSvrRowStatus inside: "<< std::endl; //mychange
	}
        
    // TR HW58803 BEGIN

	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	if(environment != TREFC_Util::SMX) {
		cout << "Setting setGenNtpEntWhiteInterface to eth0 for index 0" << endl;
		if(!m_snmpManager->setGenNtpEntWhiteInterface(0, "eth0"))
		{
			if(!switchAddress()) return 13;
			if(!m_snmpManager->setGenNtpEntWhiteInterface(0, "eth0"))
			{
				cout << "FAILURE setting setGenNtpEntWhiteInterface to eth0 for index 0" << endl;
				return 13;   //SNMP Failure
			}
		} 

		cout << "Setting setGenNtpEntWhiteInterface to eth1 for index 1" << endl;
		if(!m_snmpManager->setGenNtpEntWhiteInterface(1, "eth1"))
		{
			if(!switchAddress()) return 13;
			if(!m_snmpManager->setGenNtpEntWhiteInterface(1, "eth1"))
			{
				cout << "FAILURE setting GenNtpEntWhiteInterface to eth1 for index 1" << endl;
				return 13;   //SNMP Failure
			}   
		}
	}

    // HW58803 END
    
        //configcomplete
	std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntConfigComplete "<< std::endl; //mychange
	if(!m_snmpManager->setGenNtpEntConfigComplete())
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntConfigComplete inside"<< std::endl; //mychange
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntConfigComplete())
		{
			cout << "FAILURE setting GenNtpEntConfigComplete"<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}

	//ntpdrestart
	std::cout << __FUNCTION__ << "@" << __LINE__ <<" Set GenNtpEntNtpdRestart "<< std::endl; 
	if(!m_snmpManager->setGenNtpEntNtpdRestart())
	{
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setGenNtpEntNtpdRestart())
		{
			cout << "FAILURE setting GenNtpEntNtpdRestart"<< endl;
			return 13; //SNMP_FAILURE (??)
		}
	}

	//turole
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"set Role(1)" << std::endl; 
	if(!m_snmpManager->setStsiTuRole(1)) //set to master(1)
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<"set Role(1)" << std::endl; 
		if (!switchAddress()) return 13;
		if(!m_snmpManager->setStsiTuRole(1))
		{
			cout << "FAILURE to set StsiTuRole to 1" << endl;
			return 13; //SNMP Failure (??)
		}
	}
	return 0;
}


bool FIXS_TREFC_ApzScxHandler::cleanup()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	m_timeRefAdd.clear();
	m_timeRefRemove.clear();
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter cleanup()"<< std::endl;
	//stsiTuRole
	if(!m_snmpManager->setStsiTuRole(1)) //set to master(1)
	{
		if (!switchAddress()) return false;
		if(!m_snmpManager->setStsiTuRole(1))
		{
			cout << "BACKGROUND CLEANUP failure to set StsiTuRole to 1" << endl;
			//return 13; //SNMP Failure (??) //Not an essential setting
			return false;
		}
	}

	cout << "********Starting generic cleanup***************" << endl;

	//clear genNtpEntServerTable
	for(int i = 0; i < 8; i++)
	{
		if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i))
		{
			if (!switchAddress()) return false;
			if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i))
			{
				cout << "BACKGROUND CLEANUP failure to set GenNtpEntSvrRowStatus to 6 at index " << i << endl;
//				continue;
//				return 13; //SNMP Failure (??)
				//return false;
			}
		}

	} //end for

	//genNtpEntConfigComplete
	if(!m_snmpManager->setGenNtpEntConfigComplete())
	{
		if (!switchAddress()) return false;
		if(!m_snmpManager->setGenNtpEntConfigComplete())
		{
			cout << "BACKGROUND CLEANUP failure to set GenNtpEntConfigComplete to 1" << endl;
			//return 13; //SNMP Failure (??)
			return false;
		}
	}

	//genNtpEntNtpdRestart
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"set GenNtpEntNtpdRestart" << std::endl;  //mychange
	if(!m_snmpManager->setGenNtpEntNtpdRestart())
	{
		if (!switchAddress()) return false;
		if(!m_snmpManager->setGenNtpEntNtpdRestart())
		{
			cout << "BACKGROUND CLEANUP failure to set GenNtpEntNtpdRestart to 1" << endl;
			//return 13; //SNMP Failure (??)
			return false;
		}
                std::cout << __FUNCTION__ << "@" << __LINE__ <<"set done GenNtpEntNtpdRestart" << std::endl; //mychange
                
	}

	cout << "********Returning successfully from generic cleanup***************" << endl;
	//m_cleanup = false;
	return true;

}

void FIXS_TREFC_ApzScxHandler::setNtpConf(map<string, string> RefList, string ip, string defGateway, string nmask)
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter setNtpConf()" << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	tRefList.clear();
	extIp.clear();
	defaultGateway.clear();
	netmask.clear();

	tRefList = RefList;
	extIp = ip;
	defaultGateway = defGateway;
	netmask = nmask;

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpHpRetry = false;
		m_addNtpRefRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpRetry = false;
		m_stopNtpHpRetry = false;
		m_addExtIpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_rmNtpHpRefRetry = false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	if(!reconfNtp()) //mychange uncommeting it.
	{
		cout << "setting retry for NTP config " << endl;
		m_NtpRetry = true;
		m_isRetryOngoing = true;
	}
	cout << "*************Exit from set retry********************" << endl;
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"exit setNtpConf" << std::endl; //mychange
	return;
}


void FIXS_TREFC_ApzScxHandler::setNtpHpConf(map<string, string> RefList, string ip, string defGateway, string nmask, string otherScxIpA, string otherScxIpB)
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter setNtpHpConf()" << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	tRefList.clear();
	extIp.clear();
	defaultGateway.clear();
	netmask.clear();
	m_otherScxA.clear();
	m_otherScxB.clear();

	tRefList = RefList;
	extIp = ip;
	defaultGateway = defGateway;
	netmask = nmask;
	m_otherScxA = otherScxIpA;
	m_otherScxB = otherScxIpB;


	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_addNtpRefRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpRetry = false;
		m_stopNtpHpRetry = false;
		m_addExtIpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_rmNtpHpRefRetry = false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	//if(!reconfNtphp())
	{
		cout << "setting retry for NTPHP config " << endl;
		m_NtpHpRetry = true;
		m_isRetryOngoing = true;
	}
}


void FIXS_TREFC_ApzScxHandler::setAddNtpRefRetry(string refIp)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);
	//m_timeRef.clear();

	m_timeRefAdd.push_back(refIp);

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_NtpHpRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpRetry = false;
		m_stopNtpHpRetry = false;
		m_addExtIpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_rmNtpHpRefRetry = false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	cout << "Setting retry for adding external NTP time reference" << endl;
	m_addNtpRefRetry = true;
	m_isRetryOngoing = true;

}


void FIXS_TREFC_ApzScxHandler::setAddNtpHpRefRetry(string refIp)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);
	//m_timeRef.clear();

	m_timeRefAdd.push_back(refIp);

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_NtpHpRetry = false;
		m_addNtpRefRetry = false;
		m_stopNtpRetry = false;
		m_stopNtpHpRetry = false;
		m_addExtIpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_rmNtpHpRefRetry = false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	cout << "Setting retry for adding external NTPHP time reference" << endl;
	m_addNtpHpRefRetry = true;
	m_isRetryOngoing = true;
}

void FIXS_TREFC_ApzScxHandler::setRmNtpRefRetry(string refIp)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);
	//m_timeRef.clear();

	m_timeRefRemove.push_back(refIp);

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_NtpHpRetry = false;
		m_addNtpRefRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpRetry = false;
		m_stopNtpHpRetry = false;
		m_addExtIpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpHpRefRetry = false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	cout << "Setting retry for removing external NTP time reference" << endl;
	m_rmNtpRefRetry = true;
	m_isRetryOngoing = true;
}


void FIXS_TREFC_ApzScxHandler::setRmNtpHpRetry(string refIp)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);
	//m_timeRef.clear();

	m_timeRefRemove.push_back(refIp);

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_NtpHpRetry = false;
		m_addNtpRefRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpRetry = false;
		m_stopNtpHpRetry = false;
		m_addExtIpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	cout << "Setting retry for removing external NTPHP time reference" << endl;
	m_rmNtpHpRefRetry = true;
	m_isRetryOngoing = true;
}

void FIXS_TREFC_ApzScxHandler::setAddExtIpRetry(string ip, string defGateway, string nmask, bool restart)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	extIp.clear();
	defaultGateway.clear();
	netmask.clear();
	ntpdRestart = false;

	extIp = ip;
	defaultGateway = defGateway;
	netmask = nmask;
	ntpdRestart = restart;

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_NtpHpRetry = false;
		m_addNtpRefRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpRetry = false;
		m_stopNtpHpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_rmNtpHpRefRetry = false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	cout << "Setting retry for adding external IP configuration of SCX" << endl;
	m_addExtIpRetry = true;
	m_isRetryOngoing = true;
}

void FIXS_TREFC_ApzScxHandler::setRmExtIpRetry()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_NtpHpRetry = false;
		m_addNtpRefRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpRetry = false;
		m_stopNtpHpRetry = false;
		m_addExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_rmNtpHpRefRetry= false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;

	}
	cout << "Setting retry for removing external IP configuration of SCX" << endl;
	m_rmExtIpRetry = true;
	m_isRetryOngoing = true;
}

void FIXS_TREFC_ApzScxHandler::setStopNtpRetry()
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter setStopNtpRetry" << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_NtpHpRetry = false;
		m_addNtpRefRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpHpRetry = false;
		m_addExtIpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_rmNtpHpRefRetry= false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	cout << "Setting retry for stopping NTP" << endl;
	m_stopNtpRetry = true;
	m_isRetryOngoing = true;
}

void FIXS_TREFC_ApzScxHandler::setStopNtpHpRetry()
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter setStopNtpHpRetry" << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	if(m_isRetryOngoing)
	{
		//terminate all other retries
		m_NtpRetry = false;
		m_NtpHpRetry = false;
		m_addNtpRefRetry = false;
		m_addNtpHpRefRetry = false;
		m_stopNtpRetry = false;
		m_addExtIpRetry = false;
		m_rmExtIpRetry = false;
		m_rmNtpRefRetry = false;
		m_rmNtpHpRefRetry= false;
		m_isRetryOngoing = false;
		m_setTime = false;
		m_cleanup = false;
	}
	cout << "Setting retry for stopping NTPHP" << endl;
	m_stopNtpHpRetry = true;
	m_isRetryOngoing = true;
}

void FIXS_TREFC_ApzScxHandler::setTime()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "in set time of apz scx handler" << endl;

	m_setTime = true;

	m_NtpRetry = false;
	m_NtpHpRetry = false;

}

void FIXS_TREFC_ApzScxHandler::setCleanup()
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<" Enter setCleanup" << std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

		if(m_isRetryOngoing)
		{
			//terminate all retries
			m_NtpRetry = false;
			m_NtpHpRetry = false;
			m_addNtpRefRetry = false;
			m_addNtpHpRefRetry = false;
			m_stopNtpRetry = false;
			m_addExtIpRetry = false;
			m_rmExtIpRetry = false;
			m_rmNtpRefRetry = false;
			m_rmNtpHpRefRetry= false;
			m_isRetryOngoing = false;
			m_stopNtpHpRetry = false;
		}
		cout << "Setting trigger for generic cleanup" << std::endl;
		m_cleanup = true;
		m_isRetryOngoing = true;
}

bool FIXS_TREFC_ApzScxHandler::switchAddress() {
	if (m_exit)
		return false;

	else if (m_snmpManager) {
		if(m_ipA.compare(m_snmpManager->getIpAddress()) == 0)
			m_snmpManager->setIpAddress(m_ipB);
		else
			m_snmpManager->setIpAddress(m_ipA);

		return true;

	} else
		return false;
}
