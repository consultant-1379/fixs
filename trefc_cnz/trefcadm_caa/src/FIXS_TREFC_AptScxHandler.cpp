/*
 * FIXS_TREFC_AptScxHandler.cpp
 *
 *  Created on: Jan 10, 2012
 *      Author: elalkak
 */


#include "FIXS_TREFC_AptScxHandler.h"


#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif




FIXS_TREFC_AptScxHandler::FIXS_TREFC_AptScxHandler(string ipA, string ipB, int port):m_lock()
{
	std::cout << __FUNCTION__ << "@" << __LINE__ << " in constructor of aptScxHandler" << std::endl;

	m_running = false;
	m_exit = true;

	m_ipA = ipA;
	m_ipB = ipB;
	m_snmpManager = new FIXS_TREFC_SNMPManager(ipA);

	m_Retry = false;
	m_stopRetry = false;
	m_setTime = false;
	m_Port = port;
	std::cout << "\n FIXS_TREFC_AptScxHandler() constructor m_port value = " << m_Port <<endl;
	m_cleanup = false;

	m_shutdownEvent = -1;

	traceAptScxHandler = new ACS_TRA_trace("FIXS_TREFC_AptScxHandler");

}


FIXS_TREFC_AptScxHandler::~FIXS_TREFC_AptScxHandler()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	if(m_snmpManager) {
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	//Delete trace object
	if (traceAptScxHandler) {
		delete (traceAptScxHandler);
		traceAptScxHandler = NULL;
	}

}

int FIXS_TREFC_AptScxHandler::initialize() {
	m_exit = false;
	std::cout << __FILE__ << "  line: " << __LINE__ << "  func.: " << __FUNCTION__ << std::endl;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Try to activate..." << std::endl;
	int result = activate();
	std::cout << __FUNCTION__ << "@" << __LINE__ << " activate: " << result << std::endl;
	if(result == -1) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Activation failed" << std::endl;
		m_exit = true;
	}

	return result;
}



int FIXS_TREFC_AptScxHandler::close (){
	//Thread will stop!
	m_exit = true;

	//Stop all retries
	m_Retry = false;
	m_stopRetry = false;
	m_setTime = false;
	m_cleanup = false;

	cout << "THREAD RECEIVED CLOSE REQUEST " << m_ipA << endl;

	if (m_snmpManager) {
		m_snmpManager->stopWork();
	}
	return 0;
}

int FIXS_TREFC_AptScxHandler::svc(void)
{


	const int Number_Of_Events=1;
	const int WAIT_FAILED=-1;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;
	const int Shutdown_Event_Index=0;
	//const int Command_Event_Index=1;

	m_shutdownEvent=FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);


	m_running = true;
	//int result = -1;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	FIXS_TREFC_EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent};

	//loop
	while ( !m_exit ) // Set to true when the thread shall exit
	{
		//wait for 3s
		int returnValue=FIXS_TREFC_Event::WaitForEvents(Number_Of_Events, handleArr, 3000);

		switch (returnValue)
		{
		case WAIT_FAILED:   // Event failed
		{
			if (traceAptScxHandler->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects )", __FUNCTION__, __LINE__);
				traceAptScxHandler->ACS_TRA_event(1,tmpStr);
			}

			m_exit = true;    // Terminate the sw upgrade thread
			break;
		}

		case (WAIT_TIMEOUT):  // Time out
				{
			//TODO:These tracings seem useless...
//			if (! m_exit)
//			{
//				if (traceAptScxHandler->ACS_TRA_ON())
//				{
//					char tmpStr[512] = {0};
//					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Weak up the timer...", __FUNCTION__, __LINE__);
//					traceAptScxHandler->ACS_TRA_event(1,tmpStr);
//				}
//			}
//			else
//			{
//				std::cout << __FUNCTION__ << "@" << __LINE__ << "Time-out will be not signaled !" << std::endl;
//				if (traceAptScxHandler->ACS_TRA_ON())
//				{
//					char tmpStr[512] = {0};
//					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ime-out will be not signaled !", __FUNCTION__, __LINE__);
//					traceAptScxHandler->ACS_TRA_event(1,tmpStr);
//				}
//			}

			break;
				}

		case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
						{

			if (traceAptScxHandler->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Received a shutdown event", __FUNCTION__, __LINE__);
				traceAptScxHandler->ACS_TRA_event(1,tmpStr);
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
			if (traceAptScxHandler->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				traceAptScxHandler->ACS_TRA_event(1,tmpStr);
			}
			break;
		}
		} // End of switch

		if (!m_exit) {
			if(m_Retry && !m_exit)
			{
				std::cout << "APT::Calling m_Retry!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(confAsNtpClient())
				{
					std::cout << "APT::m_Retry SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					m_Retry = false;
				}
			}

			if(m_stopRetry && !m_exit)
			{
				std::cout << "APT::Calling m_Retry!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(stopNtpClient())
				{

					cout << "APT:stopntpClient returned true, stopping retry for ip : "<< m_ipA << endl;
					m_stopRetry = false;
				}
			}
			if(m_setTime && !m_exit)
			{
				std::cout << "APT::Calling m_setTime!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(m_snmpManager->setSystemTime())
				{
					std::cout << "APT:m_setTime SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					//m_setTime = false;
				}
				else
				{
					if (switchAddress()) {
						if(m_snmpManager->setSystemTime())
						{
							std::cout << "APT:m_setTime SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
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
				std::cout << "APT:Calling m_cleanup!!!!!!!!!!!!!!!!!!!" << std::endl;
				if(cleanup())
				{
					std::cout << "APT:m_cleanup SUCCESS!!!!!!!!!!!!!!!!!!!" << std::endl;
					m_cleanup = false;
				}
			}
			//cout << "****************End of APT SCX WHILE; STARTING AGAIN*****************" << endl;
		}
	}//while

	cout << "THREAD EXITING NOW " << m_ipA << endl;
	m_running = false;
	return 0;

}


bool FIXS_TREFC_AptScxHandler::confAsNtpClient()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Starting Swtch as SNTP client:" << endl;
	cout << "ipA :" << m_ipA << endl;
	cout << "ipB :" << m_ipB << endl;
	string ip("");
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	

	unsigned int vlan = 0;
	//clear genNtpEntServerTable
        for(int i = 0; i < 8; i++)
        {
                int error = FIXS_TREFC_SNMPManager::GETSET_SNMP_OK;

                if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i, error))
                {
                        if (!switchAddress()) return 13;
                        if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i, error))
                        {
                                cout << "sabya...FAILURE to set GenNtpEntSvrRowStatus to 6 at index " << i << endl;
                                if (error == FIXS_TREFC_SNMPManager::GETSET_SNMP_STATUS_TIMEOUT)
                                        return 13;
                                else
                                        continue;
                        }
                }

        } //end for

	//STSI settings for Tu role
	//islEthIn
	if(environment != TREFC_Util::SMX) {
		if(!m_snmpManager->setStsiIslEthIn(m_Port))
		{
			if (!switchAddress()) return false;
			if(!m_snmpManager->setStsiIslEthIn(m_Port))
			{
				cout << "FAILURE to set setStsiIslEthIn to "<< m_Port << endl;
				return false;
			}
		}
	}
	int value=0;
	if(environment == TREFC_Util::SMX)
		value=1;
	else
		value=2;
	//turole
	if(!m_snmpManager->setStsiTuRole(value)) 
	{
		if (!switchAddress()) return false;
		if(!m_snmpManager->setStsiTuRole(value))
		{
			cout << "FAILURE to set StsiTuRole to "<<value<< endl;
			return false; //SNMP Failure (??)
		}
	}	
	//Fill GenNtpEntServer table with APZ SCX ips	
	int index = 0;
	int size = (int)m_ApzSwitchVector.size();
        for(int i = 0; i < size; i++)
        {
                if(!m_snmpManager->setGenNtpEntServer(index, m_ApzSwitchVector[i].IP_EthA))
                {
                        if (!switchAddress()) return 13;
                        if(!m_snmpManager->setGenNtpEntServer(index, m_ApzSwitchVector[i].IP_EthA))
                        {
                                cout << "FAILURE to set GenNtpEntServer to " << m_ApzSwitchVector[i].IP_EthA <<" at index " << index << endl;
                                return false; //SNMP Failure (??)
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
		if(!m_snmpManager->setGenNtpEntServer(index, m_ApzSwitchVector[i].IP_EthB))
                {
                        if (!switchAddress()) return 13;
                        if(!m_snmpManager->setGenNtpEntServer(index, m_ApzSwitchVector[i].IP_EthB))
                        {
                                cout << "FAILURE to set GenNtpEntServer to " << m_ApzSwitchVector[i].IP_EthB <<" at index " << index << endl;
                                return false; //SNMP Failure (??)
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

                        /*case 2:
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
                                break;*/
			} //end switch case
			if(err)
				return 13; //SNMP Failure (??)
		}//end for
	}
	
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
	return true;
}

bool FIXS_TREFC_AptScxHandler::stopNtpClient()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "Stopping SNTP Client for Switch with:" << endl;
	cout << "ipA internal :" << m_ipA << endl;
	cout << "ipB internal :" << m_ipB << endl;
	//set systemTime
	if(!m_snmpManager->setSystemTime())
	{
		if (!switchAddress()) return false;
		if(!m_snmpManager->setSystemTime())
		{
			cout << "FAILURE to set system time, cannot stop ntp Client" << endl;
			return false;
		}
	}
	m_stopRetry = false;
	setCleanup();
	return true;
}


bool FIXS_TREFC_AptScxHandler::cleanup()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	cout << "******************Starting generic APT cleanup for ip : " << m_ipA << "**********************" << endl;
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	int value=0;
	if(environment == TREFC_Util::SMX)
		value=1;
	else
		value=2;
	//stsiTuRole
	if(!m_snmpManager->setStsiTuRole(value)) //set to slave(2)
	{
		if (!switchAddress()) return false;

		if(!m_snmpManager->setStsiTuRole(value))
		{
			cout << "BACKGROUND CLEANUP Failed to set StsiTuRole to 2" << endl;
			//return 13; //SNMP Failure (??) //Not an essential setting
			return false;
		}
	}

	//m_Port = ETHIN_APT_PORT;

	if(environment != TREFC_Util::SMX) {	
		std::cout << "\n cleanup() m_port value = " << m_Port <<endl;

		//islEthIn
		if(!m_snmpManager->setStsiIslEthIn(m_Port))
		{
			if (!switchAddress()) return false;
			if(!m_snmpManager->setStsiIslEthIn(m_Port))
			{
				cout << "FAILURE to set setStsiIslEthIn to "<< m_Port << endl;
				return false;
			}
		}
	}
	//clear genNtpEntServerTable
        for(int i = 0; i < 8; i++)
        {
                if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i))
                {
                        if (!switchAddress()) return false;
                        if(!m_snmpManager->setGenNtpEntSvrRowStatus(6, i))
                        {
                                cout << "BACKGROUND CLEANUP failure to set GenNtpEntSvrRowStatus to 6 at index " << i << endl;
//                              continue;
//                              return 13; //SNMP Failure (??)
                              //  return false;
                        }
                }

        } //end for
	if(environment != TREFC_Util::SMX) {
		for(int i = 0; i < 8; i++)
		{
			if(!m_snmpManager->setGenNtpEntWhiteRowStatus(6, i))
	                {
        	                if (!switchAddress()) return 13;
                	        if(!m_snmpManager->setGenNtpEntWhiteRowStatus(6, i))
				{
					cout << "FAILURE setting GenNtpEntWhiteRowStatus to 6 for index " << i << endl;
				}
			}
		}
	}
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

	cout << "******************Returning success from generic APT cleanup for ip : " << m_ipA << "**********************" << endl;
	return true;
}

void FIXS_TREFC_AptScxHandler::setNtpClientConf(vector<SWITCHAPZIPAddress> apzSwitchVector, int port)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	m_ApzSwitchVector.clear();
	
	std::cout << " setNtpClientConf() m_Port.... = " << m_Port << endl; 
//	m_Port = 31;

	m_ApzSwitchVector = apzSwitchVector;
	m_Port = port;
	
	std::cout << " setNtpClientConf() m_Port = " << m_Port << endl; 

	//terminate all stop retry because we will start client now
	m_stopRetry = false;
	m_setTime = false;
	m_cleanup = false;

	//if(!confAsntpClient())
	{
		cout << "setting retry for SNTP client config " << endl;
		m_Retry = true;
	}
}



void FIXS_TREFC_AptScxHandler::setStopNtpClient()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	//terminate all other retries
	m_Retry = false;
	m_setTime = false;
	m_cleanup = false;

	//if(!stopntpClient())
	cout << "Setting retry for stopping SNTP client config" << endl;
	m_stopRetry = true;

}


void FIXS_TREFC_AptScxHandler::setTime()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);
	m_setTime = true;
}

void FIXS_TREFC_AptScxHandler::setCleanup()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	//terminate start retries
	m_Retry = false;


	cout << "Setting trigger for generic cleanup" << std::endl;
	m_cleanup = true;
}

bool FIXS_TREFC_AptScxHandler::switchAddress() {
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
