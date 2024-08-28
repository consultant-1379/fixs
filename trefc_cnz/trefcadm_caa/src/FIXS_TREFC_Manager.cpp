/*
 * FIXS_TREFC_Manager.h
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_Manager.h"
#include "FIXS_TREFC_SNMPTrapReceiver.h"
#include "FIXS_TREFC_Logger.h"


// Class FIXS_TREFC_Manager


FIXS_TREFC_Manager* FIXS_TREFC_Manager::m_instance = 0;


FIXS_TREFC_Manager::FIXS_TREFC_Manager()
:servStatus(IMM_Util::DISABLED),
 servType(IMM_Util::NONE),
 m_trapManager(0),
 m_jtpHandlerTref(0),
 m_timeChecker(0),
 m_stopOperation(false)
{

	m_apzScxHandler[0] = NULL;
	m_apzScxHandler[1] = NULL;
	m_alarmHandler = NULL;	
	isAlarmRaised = false; //HU26732
	for(int i=0; i < MAX_NO_OF_APT_SCX ; i++)
	{
		m_aptScxHandler[i] = NULL;
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Nullifying handler " << i << std::endl;
	}

	_trace = new ACS_TRA_trace("FIXS_TREFC_Manager");

	//Create new trap receiver
	m_trapManager = new  FIXS_TREFC_SNMPTrapReceiver();

}


FIXS_TREFC_Manager::~FIXS_TREFC_Manager()
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter FIXS_TREFC_Manager::~FIXS_TREFC_Manager()" << std::endl; //mychange
	//Just in case we missed the shutdown event

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Closing APZ SCX Threads...", LOG_LEVEL_DEBUG);
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"closing APZ SCX Threads" << std::endl; //mychange

	if(m_apzScxHandler[0]) {
		if (m_apzScxHandler[0]->isRunning())
			m_apzScxHandler[0]->close();
	}

	if(m_apzScxHandler[1]) {
		if (m_apzScxHandler[1]->isRunning())
			m_apzScxHandler[1]->close();
	}

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Closing APT SCX Threads...", LOG_LEVEL_DEBUG);
	for(int i=0; i < MAX_NO_OF_APT_SCX ; i++)
	{
		if(m_aptScxHandler[i]) {
			if(m_aptScxHandler[i]->isRunning())
				m_aptScxHandler[i]->close();
		}
	}

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Unsubscribing from TRAPDS...", LOG_LEVEL_DEBUG);
	cout << "Starting FIXS_TREFC_Manager destructor" << endl;
	//Delete Trap receiver
	if (m_trapManager) {
		m_trapManager->unsubscribe();
		//sleep(1);
		delete m_trapManager;
		m_trapManager = NULL;
	}
        //HU26732 START
	if(isAlarmRaised)
	{
		int problem = 35100;
                unsigned int severity = TREFC_Util::Severity_CEASING; //change the alarm severity, cause, data and text according to POD
                std::string cause = "TIME SERVICE NOT STARTED";
                std::string data = "Time Service Configuration Fault";
                std::string text;
                bool isSMX=false;
                TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();

                if(environment == TREFC_Util::SMX)
                	isSMX=true;

                if(isSMX == true)
                	text = "TIME SERVICE IS NOT STARTED ON SWITCH BOARDS\nBUT EXTERNAL TIME REFERENCES ARE CONFIGURED";
                else
                	text = "TIME SERVICE IS NOT STARTED ON SCX BUT\nEXTERNAL TIME REFERENCES ARE CONFIGURED";
                sendAlarm (problem, severity, cause, data, text);
                {
                        char tmpStr[512] = {0};
                        if(isSMX == true)
                        	snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SWITCH BOARDS BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
                        else
                        	snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SCX BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
                        FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
                }

                isAlarmRaised = false;
        }
	//HU26732 END
	if(m_alarmHandler)
	{
		m_alarmHandler->stop();
		for(int i = 0; ((i < 5) && m_alarmHandler->isRunningThread());i++) sleep(1);
		delete m_alarmHandler;
	}

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Closing JTP Handler Thread...", LOG_LEVEL_DEBUG);
	//Delete JTP Handler
	if (m_jtpHandlerTref) {
		delete m_jtpHandlerTref;
		m_jtpHandlerTref = NULL;
	}

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Closing Time Checker Thread...", LOG_LEVEL_DEBUG);
	//Delete Time Checker
	if(m_timeChecker)
	{
		m_timeChecker->stop();
		for(int i = 0; ((i < 6) && m_timeChecker->isRunningThread()); i++) sleep(1);
		cout << "TimeChecker Thread Finished" <<endl;
		delete m_timeChecker;
		m_timeChecker = NULL;
	}

	//Delete Apz SCX Handler threads
	if(m_apzScxHandler[0]) {
		for(int i = 0; ((i < 6) && m_apzScxHandler[0]->isRunning()); i++) sleep(1);
		cout << "APZ SCX 0 Handler Thread Finished" <<endl;
		delete m_apzScxHandler[0];
		m_apzScxHandler[0] = NULL;
	}

	if(m_apzScxHandler[1]) {
		for(int i = 0; ((i < 6) && m_apzScxHandler[1]->isRunning()); i++) sleep(1);
		cout << "APZ SCX 1 Handler Thread Finished" <<endl;
		delete m_apzScxHandler[1];
		m_apzScxHandler[1] = NULL;
	}

	//Delete Apt SCX Handler threads
	for(int i=0; i < MAX_NO_OF_APT_SCX ; i++)
	{
		if(m_aptScxHandler[i]) {
			for(int j = 0; ((j < 6) && m_aptScxHandler[i]->isRunning()); j++) sleep(1);
			cout << "APT SCX " << i << " Handler Thread Finished" <<endl;
			delete m_aptScxHandler[i];
			m_aptScxHandler[i] = NULL;
		}

	}

	//Delete trace object
	if (_trace) {
		delete (_trace);
		_trace = NULL;
	}

}


FIXS_TREFC_Manager * FIXS_TREFC_Manager::getInstance ()
{

	if (m_instance == 0)
	{
		m_instance = new FIXS_TREFC_Manager();
	}
	return m_instance;

}

void FIXS_TREFC_Manager::finalize() {
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter FIXS_TREFC_Manager::finalize" << std::endl; //mychange
	if (m_instance) {
		delete(m_instance);
		m_instance = NULL;
	}
}

int FIXS_TREFC_Manager::initTRefManager ()
{
std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter FIXS_TREFC_Manager::initTRefManager" << std::endl; //mychange
	int res = 0;
	m_tRefList.clear();

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Initializing TREFC Manager...");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
	}

	bool isSMX=false;
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	if(environment == TREFC_Util::SMX)
		isSMX=true;

	//create ScxTimeService Object
	ImmInterface::createSwitchTimeServiceClasses(isSMX);

	//Fetch data from CS
	res = getTrefList(isSMX);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Time Reference Configuration loaded!");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
	}

	//Start Time Checker Thread
	m_timeChecker = new FIXS_TREFC_TimeChecker();
	m_timeChecker->open();

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] TimeChecker thread started!");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
	}

	if (!m_alarmHandler)
	{
		m_alarmHandler = new FIXS_TREFC_AlarmHandler();	
	}

	//start trap handling:
	//startTrapManager ();
	if (m_trapManager)
		m_trapManager->startSubscriber();

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] TrapSubscriber started!");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
	}
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"calling setExtIpAtTrefcStart" << std::endl; //mychange
	//restore external IP Address from IMM model
	setExtIpAtTrefcStart(isSMX);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] External IP addresses Configuration loaded!");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
	}

	//get board addresses of all scx boards and start handler threads
	int apzVecSize = 0;
	int aptVecSize = 0;

	string otherIpA[2] = {"",""};
	string otherIpB[2] = {"",""};

	bool slot0 = false;
	bool slot25 = false;

	res = GetSWITCHBoardAddress(m_switchAPTVector, m_switchAPZVector, isSMX);

	if(res == 0)
	{
		apzVecSize = (int)m_switchAPZVector.size();
		aptVecSize = (int)m_switchAPTVector.size();

		if(apzVecSize > 0)
		{
			for (int i = 0; i < apzVecSize; i++)
			{
				//start handler thread for APZ SCX
                                std::cout << __FUNCTION__ << "@" << __LINE__ <<"Creating start handler thread" << std::endl; //mychange
				m_apzScxHandler[i] = new FIXS_TREFC_ApzScxHandler(m_switchAPZVector[i].IP_EthA,m_switchAPZVector[i].IP_EthB);
				if(m_apzScxHandler[i])
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting thread for APZ SWITCH %d, IP: %s.",
								m_switchAPZVector[i].slotPos, m_switchAPZVector[i].IP_EthA.c_str());
						FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					m_apzScxHandler[i]->initialize();
				}
				else
				{
					cout << "Could not create new APZ SWITCH handler" << endl;
					printTRA("Could not create new APZ SWITCH handler");
				}
				if(m_switchAPZVector[i].slotPos == 0)
					slot0 = true;
				if(m_switchAPZVector[i].slotPos == 25)
					slot25 = true;
			}
		}

		if(aptVecSize > 0)
		{
			cout << "Size of APT SWITCH Vector = " << aptVecSize << endl;
			for (int i = 0; i < aptVecSize; i++)
			{
				//start handler thread for APZ SCX
				m_aptScxHandler[i] = new FIXS_TREFC_AptScxHandler(m_switchAPTVector[i].IP_EthA,m_switchAPTVector[i].IP_EthB,m_switchAPTVector[i].ethInPort);
				if(m_aptScxHandler[i])
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting thread for APT SWITCH %d, IP: %s",
								i+1, m_switchAPTVector[i].IP_EthA.c_str());
						FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
                                        std::cout << __FUNCTION__ << "@" << __LINE__ <<"calling initialize()" << std::endl; //mychange
					m_aptScxHandler[i]->initialize();
				}
				else
				{
					cout << "Could not create new APT SWITCH handler" << endl;
					printTRA("Could not create new APT SWITCH handler");
				}
			}
		}

	}

	std::string shelfAddress("");

	unsigned long temp;

	// Start JTP Handler for APZ Magazine
	if((ImmInterface::getAPZShelfAddress(shelfAddress))== 0) {
		stringToUlongMagazine(shelfAddress, temp);
		unsigned long hlmag = ntohl(temp);
		unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
		unsigned char mag = u_plug0 & 0x0F;

		this->m_jtpHandlerTref = new FIXS_TREFC_JTPHandler(mag, slot0,slot25);
		m_jtpHandlerTref->open();
	}

	//update SNMP setting with current trefc list and external ip configuration
	if (servStatus == IMM_Util::ENABLED)
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Service Status is ENABLED! Setting Configuration on SWITCH...");
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
		}

		//bool result[2] = {false,false};


		if(servType == IMM_Util::NTP)
		{
			//APZ
			if(m_apzScxHandler[0])
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Setting NTP Configuration on APZ SWITCH %d.", m_switchAPZVector[0].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}

				if(m_switchAPZVector[0].slotPos == 0)
				{
				        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting NtpConf" << std::endl; //mychange
					m_apzScxHandler[0]->setNtpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask);
				        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Set done NtpConf" << std::endl; //mychange
				}
				else if(m_switchAPZVector[0].slotPos == 25)
				{
		                        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting NtpConf" << std::endl; //mychange
					m_apzScxHandler[0]->setNtpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask);
                                        std::cout << __FUNCTION__ << "@" << __LINE__ <<"SEt done NtpConf" << std::endl; //mychange
				}
			}
			if(m_apzScxHandler[1])
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Setting NTP Configuration on APZ SWITCH %d.", m_switchAPZVector[1].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}

				if(m_switchAPZVector[1].slotPos == 0)
				{
                 			std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting NtpConf" << std::endl; //mychange
					m_apzScxHandler[1]->setNtpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask);
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"SEt done NtpConf" << std::endl; //mychange
				}
				else if(m_switchAPZVector[1].slotPos == 25)
				{
   					std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting NtpConf" << std::endl; //mychange
					m_apzScxHandler[1]->setNtpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask);
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"SEt done NtpConf" << std::endl; //mychange
				}
			}

		} //if NTP


		if(servType == IMM_Util::NTPHP)
		{
			if(apzVecSize == 2)
			{
				otherIpA[0] = m_switchAPZVector[1].IP_EthA;
				otherIpB[0] = m_switchAPZVector[1].IP_EthB;
				otherIpA[1] = m_switchAPZVector[0].IP_EthA;
				otherIpB[1] = m_switchAPZVector[0].IP_EthB;
			}

			//APZ
			if(m_apzScxHandler[0])
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Setting NTPHP Configuration on APZ SCX %d.",
							m_switchAPZVector[0].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}

				if(m_switchAPZVector[0].slotPos == 0)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting NtphpConf" << std::endl; //mychange
					m_apzScxHandler[0]->setNtpHpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, otherIpA[0], otherIpB[0]);
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"Set done NTPhpConf. " <<std::endl; //mychange
				}
				else if(m_switchAPZVector[0].slotPos == 25)
				{
		 			std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting NtphpConf" << std::endl; //mychange	
					m_apzScxHandler[0]->setNtpHpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, otherIpA[0], otherIpB[0]);
                                	std::cout << __FUNCTION__ << "@" << __LINE__ <<"Set done NTPhpConf. " <<std::endl; //mychange

				}
			}
			if(m_apzScxHandler[1])
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Setting NTPHP Configuration on APZ SCX %d.",
							m_switchAPZVector[1].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}

				if(m_switchAPZVector[1].slotPos == 0)
				{
  					std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting NTPhpConf. " <<std::endl; //mychange

					m_apzScxHandler[1]->setNtpHpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, otherIpA[1], otherIpB[1]);
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"Set done NTPhpConf. " <<std::endl; //mychange

				}
				else if(m_switchAPZVector[1].slotPos == 25)
				{
 					std::cout << __FUNCTION__ << "@" << __LINE__ <<"Setting NTPhpConf. " <<std::endl; //mychange

					m_apzScxHandler[1]->setNtpHpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, otherIpA[1], otherIpB[1]);
			  		std::cout << __FUNCTION__ << "@" << __LINE__ <<"Set done NTPhpConf. " <<std::endl; //mychange

				}
			}

		} // if NTPHP

		//start NTP client on APT magazine SCX boards
		if(aptVecSize > 0)
		{
			for(int i=0;i<aptVecSize;i++)
			{
				if(m_aptScxHandler[i])
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Setting NTP Client Configuration on APT SCX %d, IP: %s.",
								i+1, m_switchAPTVector[i].IP_EthA.c_str());
						FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
					}
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"m_switchAPTVector[" << i << "].ethInPort " << m_switchAPTVector[i].ethInPort << std::endl;
					m_aptScxHandler[i]->setNtpClientConf(this->m_switchAPZVector, m_switchAPTVector[i].ethInPort); //fetch port number from PHA param and send it as second argument of this function
				}
			}
		}

	} //if enabled
	else // disabled
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Service Status is DISABLED! CleanUp configuration on all SCX Boards.");
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
		}

		//Set time on all SCX boards
		if(m_timeChecker->isRunningThread())
			m_timeChecker->startTimeSet();
		else
		{
			std::cout<<"DBG: timeChecker is not running";
			res = 1;
		}
           
                 std::cout << __FUNCTION__ << "@" << __LINE__ <<"Set cleanup " <<std::endl; //mychange
		//Cleanup SNMP settings on all SCX boards
		if(m_apzScxHandler[0]) //APZ
		{
			m_apzScxHandler[0]->setCleanup();
		}
		if(m_apzScxHandler[1]) //APZ
		{
			m_apzScxHandler[1]->setCleanup();
		}
		//		APT
		if(aptVecSize > 0)
		{
			for(int i=0;i<aptVecSize;i++)
			{
				if(m_aptScxHandler[i])
				{
					m_aptScxHandler[i]->setCleanup();
				}
			}
		}
	} //end disabled condition
	if(servStatus == IMM_Util::ENABLED)
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<"Time server enabled. " <<std::endl; //mychange
		std::cout << " ================================== " << std::endl;
		std::cout << " Time server Enabled ... alarm OFF " << std::endl;
		std::cout << " ================================== " << std::endl;
		//cease alarm
		int problem = 35100;
		unsigned int severity = TREFC_Util::Severity_CEASING; //change the alarm severity, cause, data and text according to POD
		std::string cause = "TIME SERVICE NOT STARTED";
		std::string data = "Time Service Configuration Fault",text;
		bool isSMX=false;
		TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();

		if(environment == TREFC_Util::SMX)
		  isSMX=true;
		if(isSMX == true)
			text = "TIME SERVICE IS NOT STARTED ON SWITCH BOARDS\nBUT EXTERNAL TIME REFERENCES ARE CONFIGURED";
		else
			text = "TIME SERVICE IS NOT STARTED ON SCX BUT\nEXTERNAL TIME REFERENCES ARE CONFIGURED";

		sendAlarm (problem, severity, cause, data, text);
		{
			char tmpStr[512] = {0};
			if(isSMX == true)
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SWITCH BOARDS BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
			else
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SCX BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
		}

		isAlarmRaised = false;
	}
	else if(servStatus == IMM_Util::DISABLED)
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<"Time server disabled. " <<std::endl; //mychange
		//check alarm
		if(!m_tRefList.empty())
		{
			std::cout << " ================================== " << std::endl;
			std::cout << " Time server Disabled ... alarm ON " << std::endl;
			std::cout << " ================================== " << std::endl;
			//cease alarm
			int problem = 35100;
			unsigned int severity = TREFC_Util::Severity_O1; //change the alarm severity, cause, data and text according to POD
			std::string cause = "TIME SERVICE NOT STARTED";
			std::string data = "Time Service Configuration Fault";
			std::string text;
			bool isSMX = false;
			TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();

			if(environment == TREFC_Util::SMX)
				isSMX=true;
			if(isSMX == true)
				text = "TIME SERVICE IS NOT STARTED ON SWITCH BOARDS\nBUT EXTERNAL TIME REFERENCES ARE CONFIGURED";
			else
				text = "TIME SERVICE IS NOT STARTED ON SCX BUT\nEXTERNAL TIME REFERENCES ARE CONFIGURED";

			sendAlarm (problem, severity, cause, data, text);
			{
				char tmpStr[512] = {0};
				if(isSMX == true)
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SWITCH BOARDS BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
				else
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SCX BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
			}
			isAlarmRaised = true;
		}
		else if(m_tRefList.empty())
		{
			std::cout << " ================================== " << std::endl;
			std::cout << " Time server list empty ... alarm OFF " << std::endl;
			std::cout << " ================================== " << std::endl;
			//cease alarm
			int problem = 35100;
			unsigned int severity = TREFC_Util::Severity_CEASING; //change the alarm severity, cause, data and text according to POD
			std::string cause = "TIME SERVICE NOT STARTED";
			std::string data = "Time Service Configuration Fault";
			std::string text;
			bool isSMX = false;
			TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
			if(environment == TREFC_Util::SMX)
				isSMX=true;
			if(isSMX == true)
				text = "TIME SERVICE IS NOT STARTED ON SWITCH BOARDS\nBUT EXTERNAL TIME REFERENCES ARE CONFIGURED";
			else
				text = "TIME SERVICE IS NOT STARTED ON SCX BUT\nEXTERNAL TIME REFERENCES ARE CONFIGURED";

			sendAlarm (problem, severity, cause, data, text);
			{
				char tmpStr[512] = {0};
				if(isSMX == true)
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SWITCH BOARDS BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
				else
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SCX BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
			}
			isAlarmRaised = false;
		}
	}
	else
		isAlarmRaised = false;


	bool ScxExist=false;
	ACS_APGCC_ImmObject paramList;
	if(ScxExist=IMM_Util::getObject(IMM_Util::ATT_SCXTIMESERVICE_DN,&paramList)) {
	std::cout <<"********************** Do Switch Network Configuration **********************"<<endl;
		migrate_ScxTref_to_SwitchTref(isSMX);
		std::cout <<"********************** Switch Network Configuration is enabled **********************"<<endl;
	}
	std::cout<<"ScxExist=="<<ScxExist<<std::endl;
	return res;

}

int FIXS_TREFC_Manager::addTRefToList (std::string trefIP, std::string trefName)
{
        std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter addTREfToList" <<std::endl; //mychange

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	std::cout << __FUNCTION__ << "@" << __LINE__ <<
			" trefIP: " << trefIP.c_str() <<
			" - trefName: " << trefName.c_str() << std::endl;

	int res1 = 0;
	int res2 = 0;
	std::map<std::string, std::string>::iterator it;
	//
	//Update SNMP changes to SCX
	if(servStatus == IMM_Util::ENABLED)
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<"Checking servTYpe as NTP " <<std::endl; //mychange
		if (servType == IMM_Util::NTP) // serviceType = NTP
		{
			//APZ
			if(m_apzScxHandler[0] != NULL)
			{
                                std::cout << __FUNCTION__ << "@" << __LINE__ <<"adding TimeRefNtp" <<std::endl; //mychange
				res1 = m_apzScxHandler[0]->addTimeRefNtp(trefIP);
                                std::cout << __FUNCTION__ << "@" << __LINE__ <<"after adding TimeREfNtp " <<std::endl; //mychange
                        }
			if(m_apzScxHandler[1] != NULL)
                        {
                                std::cout << __FUNCTION__ << "@" << __LINE__ <<"adding TimeRefNtp" <<std::endl; //mychange
				res2 = m_apzScxHandler[1]->addTimeRefNtp(trefIP);
                                std::cout << __FUNCTION__ << "@" << __LINE__ <<"after adding TimeREfNtp " <<std::endl; //mychange
                         }

			if(res1!=0 && res2!=0) {
				printTRA("Time Ref add failed: SNMP_FAILURE");

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Failed! Time Server name: %s, IP address = %s. SNMP Failure!",trefName.c_str(),
									trefIP.c_str());
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				//return SNMP_FAILURE;
			}

			//Insert into temporary Time Ref list
			m_tRefList.insert ( pair<string,string>(trefIP, trefName) );

			if(res1!=0)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Failed! Time Server name: %s, IP address = %s, on SCX %d: Setting retry!",trefName.c_str(),
							trefIP.c_str(), m_switchAPZVector[0].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
				}

				if(m_switchAPZVector[0].slotPos == 0)
				{
					m_apzScxHandler[0]->setAddNtpRefRetry(trefIP);
				}
				else if(m_switchAPZVector[0].slotPos == 25)
				{
					m_apzScxHandler[0]->setAddNtpRefRetry(trefIP);
				}

			}

			if(res2!=0)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Failed! Time Server name: %s, IP address = %s, on SCX %d: Setting retry!",trefName.c_str(),
							trefIP.c_str(), m_switchAPZVector[1].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
				}

				if(m_switchAPZVector[1].slotPos == 0)
				{
					m_apzScxHandler[1]->setAddNtpRefRetry(trefIP);
				}
				else if(m_switchAPZVector[1].slotPos == 25)
				{
					m_apzScxHandler[1]->setAddNtpRefRetry(trefIP);
				}
			}


		}
		else if (servType == IMM_Util::NTPHP) {// serveceType = NTPHP

			//APZ
                         std::cout << __FUNCTION__ << "@" << __LINE__ <<"Serv type = NTPHP " <<std::endl; //mychange
			if(m_apzScxHandler[0] != NULL)
				res1 = m_apzScxHandler[0]->addTimeRefNtphp(trefIP);
			if(m_apzScxHandler[1] != NULL)
				res2 = m_apzScxHandler[1]->addTimeRefNtphp(trefIP);

			if(res1!=0 && res2!=0) {
				printTRA("Time Ref add failed: SNMP_FAILURE");
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Failed! Time Server name: %s, IP address = %s: SNMP Failure!",trefName.c_str(),
							trefIP.c_str());
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				//return SNMP_FAILURE;
			}

			//Insert into temporary Time Ref list
			m_tRefList.insert ( pair<string,string>(trefIP, trefName) );

			if(res1!=0)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Failed! Time Server name: %s, IP address = %s, on SCX %d: Setting retry!",trefName.c_str(),
							trefIP.c_str(), m_switchAPZVector[0].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
				}

				if(m_switchAPZVector[0].slotPos == 0)
				{
					m_apzScxHandler[0]->setAddNtpHpRefRetry(trefIP);
				}
				else if(m_switchAPZVector[0].slotPos == 25)
				{
					m_apzScxHandler[0]->setAddNtpHpRefRetry(trefIP);
				}

			}
			if(res2!=0)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Failed! Time Server name: %s, IP address = %s, on SCX %d: Setting retry!",trefName.c_str(),
							trefIP.c_str(), m_switchAPZVector[1].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
				}

				if(m_switchAPZVector[1].slotPos == 0)
				{
					m_apzScxHandler[1]->setAddNtpHpRefRetry(trefIP);
				}
				else if(m_switchAPZVector[1].slotPos == 25)
				{
					m_apzScxHandler[1]->setAddNtpHpRefRetry(trefIP);
				}
			}

		}
	}
	else {//serviceStatus = DISABLED
		//Insert into list without other settings
		m_tRefList.insert ( pair<string,string>(trefIP, trefName) );
                 std::cout << __FUNCTION__ << "@" << __LINE__ <<"Insert into list without other settings " <<std::endl; //mychange
	}


	//	/**********************************************/
	//				Only for debug
	std::cout << std::endl << __FUNCTION__ << "@" << __LINE__ << " ----------- AFTER ADD ----------- " << std::endl;
	for (it = m_tRefList.begin() ; it != m_tRefList.end() ; it++)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<
				" IP: " << it->first.c_str() <<
				" Name: " << it->second.c_str() << std::endl;
	}
	std::cout << "service type: "<<servType<<"\t status : "<<servStatus<<"\t"<<__LINE__<<std::endl;
	/**********************************************/

	//	/**********************************************/
	//	/* Connect to CS in order to store the data ! */
	//	/**********************************************/
	//	std::cout << std::endl;
	//	std::cout << __FUNCTION__ << "@" << __LINE__ << " CONNECT TO CS IN ORDER TO STORE INFORMATION " << std::endl;
	//	std::cout << std::endl;
	//
	//	ACS_CS_API_SET_NS::CS_API_Set_Result result = ACS_CS_API_TREFC::addTrefEntry(tRefStruct);
	//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " result = " << result << std::endl;
	//	if(result != ACS_CS_API_SET_NS::Result_Success) //Failed to store data in CS
	//	{
	//		std::map<std::string, std::string>::iterator it;
	//		it = m_tRefList.find(trefIP.c_str());
	//		if (it != m_tRefList.end())
	//		{
	//			//Deleting entry from cache
	//			std::cout << __FUNCTION__ << "@" << __LINE__ << "Deleting entry from TREFC cache" << std::endl;
	//			m_tRefList.erase(it);
	//		}
	//		return TREFC_Util::NO_REPLY_FROM_CS;
	//
	//	}
	//


	//	/**********************************************/
	//	/* Send JTP notification for add ! */
	//	/**********************************************/
	if ( servStatus == IMM_Util::ENABLED )
	{
                std::cout << __FUNCTION__ << "@" << __LINE__ <<"ADD ENTRY when service is enabled" <<std::endl; //mychange
		std::cout <<"********************** ADD ENTRY when service is enabled **********************"<<endl;
		unsigned long ipVal = 0 ;
		unsigned short refVal  = 0;

		if ( trefIP == "GPS DEVICE" )
		{
			std::cout <<" -----------> GPS REFERENCE..........."<<__LINE__<<"\t"<<endl;
			refVal = 2; // set 2 if Reference is GPS
		}
		else
		{
			std::cout <<" -----------> TIME REFERENCE..........."<<__LINE__<<"\t"<<endl;
			checkIPAddresses(trefIP.c_str(),ipVal);
			std::cout<< " ipVal :"<<ipVal<<std::endl;
			refVal = 1;  // set 1 if Reference is IP
		}

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Time Server Added, name: %s, IP address: %s. Sending JTP Notification.",trefName.c_str(),
					trefIP.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
		}
		
		if(m_jtpHandlerTref)
			m_jtpHandlerTref->setReferenceValues(ipVal,refVal);

		if(servType == IMM_Util::NTP)
		{
			if(m_jtpHandlerTref)
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ <<"SET JTP Notification for NTP" <<std::endl; //mychange
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_REFERENCE_ADD,1);  // 1 indicates NTP service
                                std::cout << __FUNCTION__ << "@" << __LINE__ <<"Set done Jtp Notification" <<std::endl; //mychange
			}
		}
		else if(servType == IMM_Util::NTPHP)
		{
			if(refVal == 2) //GPS
			{
				if(m_jtpHandlerTref)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"SET JTP Notification for NTPHP with GPS device" <<std::endl; //mychange
					m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_REFERENCE_ADD,3);  // 3 means NTPHP service with GPS device
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"SET done JTP Notification for NTPHP" <<std::endl; //mychange
				}
			}
			else
			{
				if(m_jtpHandlerTref)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"SET JTP Notification for NTPHP " <<std::endl; //mychange
					m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_REFERENCE_ADD,2);  // 2 means NTPHP service
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"SET JTP Notification for NTPHP " <<std::endl; //mychange
				}
			}
		}
	} else {

		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Time Server Added, name: %s, IP address: %s.",trefName.c_str(),
				trefIP.c_str());
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

	}
	//	/**********************************************/

	//Alarm handling
	checkAlarmStatus(false);
	return EXECUTED;
}

int FIXS_TREFC_Manager::rmTRefFromList (std::string trefIP, std::string trefName)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter rmTRefFRomList" <<std::endl; //mychange
	//	Lock lock;
	std::cout << __FUNCTION__ << "@" << __LINE__ <<
			" trefIP: " << trefIP.c_str() <<
			" - trefName: " << trefName.c_str() << std::endl;

	int res = TREFC_Util::EXECUTED;
	int res1 = 0;
	int res2 = 0;
	std::map<std::string, std::string>::iterator it;

	//Check if the service is enabled and this is the last time reference left
	if (trefIP.size() > 0)
	{
		it = m_tRefList.find(trefIP.c_str());

		if (it != m_tRefList.end())
		{
			//IP FOUND !!
			// Update SNMP MIB in SCX
			if (servStatus == IMM_Util::ENABLED)
			{
				 // serviceType = NTP
				if (servType == IMM_Util::NTP)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"calling rmTimeREfNtp,handle[0]" <<std::endl; //mychange
					if(m_apzScxHandler[0] != NULL)
						res1 = m_apzScxHandler[0]->rmTimeRefNtp(trefIP);
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"calling rmTimeREfNtp,handle[1]" <<std::endl; //mychange
					if(m_apzScxHandler[1] != NULL)
						res2 = m_apzScxHandler[1]->rmTimeRefNtp(trefIP);

					if(res1!=0 && res2!=0) {
						if (res1 == 12 || res2 == 12) {
							printTRA("Time Ref remove failed:entry not found in table");
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s. Entry not found in the list.",
										trefName.c_str(),
										trefIP.c_str());
								FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
							}
						} else {
							printTRA("Time Ref remove failed: SNMP_FAILURE");

							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s. SNMP Failure.",
										trefName.c_str(),
										trefIP.c_str());
								FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
							}
						}
						//return SNMP_FAILURE;
					}

					//Remove Time Ref from the list
					m_tRefList.erase(it);

					if(res1!=0)
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s, on SCX %d: Setting retry!",trefName.c_str(),
									trefIP.c_str(), m_switchAPZVector[0].slotPos);
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						if(m_switchAPZVector[0].slotPos == 0)
						{
							m_apzScxHandler[0]->setRmNtpRefRetry(trefIP);
						}
						else if(m_switchAPZVector[0].slotPos == 25)
						{
							m_apzScxHandler[0]->setRmNtpRefRetry(trefIP);
						}
					}
					if(res2!=0)
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s, on SCX %d: Setting retry!",trefName.c_str(),
									trefIP.c_str(), m_switchAPZVector[1].slotPos);
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						if(m_switchAPZVector[1].slotPos == 0)
						{
							m_apzScxHandler[1]->setRmNtpRefRetry(trefIP);
						}
						else if(m_switchAPZVector[1].slotPos == 25)
						{
							m_apzScxHandler[1]->setRmNtpRefRetry(trefIP);
						}
					}
				}

				else if (servType == IMM_Util::NTPHP) {// serveceType = NTPHP
					if(m_apzScxHandler[0] != NULL)
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ <<"calling rmTimeREfNtphp,handle[0]" <<std::endl; //mychange
						res1 = m_apzScxHandler[0]->rmTimeRefNtphp(trefIP);
					}
					if(m_apzScxHandler[1] != NULL)
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ <<"calling rmTimeREfNtp,handle[1]" <<std::endl; //mychange
						res2 = m_apzScxHandler[1]->rmTimeRefNtphp(trefIP);
					}

					if(res1!=0 && res2!=0) {
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s. SNMP Failure.",
									trefName.c_str(),
									trefIP.c_str());
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
						}
						printTRA("Time Ref remove failed: SNMP_FAILURE");
						//return SNMP_FAILURE;
					}

					//Remove Time Ref from the list
					m_tRefList.erase(it);

					if(res1!=0)
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s, on SCX %d: Setting retry!",trefName.c_str(),
									trefIP.c_str(), m_switchAPZVector[0].slotPos);
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						if(m_switchAPZVector[0].slotPos == 0)
						{
							m_apzScxHandler[0]->setRmNtpHpRetry(trefIP);
						}
						else if(m_switchAPZVector[0].slotPos == 25)
						{
							m_apzScxHandler[0]->setRmNtpHpRetry(trefIP);
						}

					}
					if(res2!=0)
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s, on SCX %d: Setting retry!",trefName.c_str(),
									trefIP.c_str(), m_switchAPZVector[1].slotPos);
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						if(m_switchAPZVector[1].slotPos == 0)
						{
							m_apzScxHandler[1]->setRmNtpHpRetry(trefIP);
						}
						else if(m_switchAPZVector[1].slotPos == 25)
						{
							m_apzScxHandler[1]->setRmNtpHpRetry(trefIP);
						}
					}

				}
			}
			else //DISABLED
			{
				//Remove Time Ref from the list without other settings
				m_tRefList.erase(it);
			}
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " IP not found !" << std::endl;
			printTRA("Time Ref remove failed: IP not found!");
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s. IP not found in the list.",
						trefName.c_str(),
						trefIP.c_str());
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
			}

			return TREFC_Util::TIME_REF_NOT_EXIST;
		}
	} else
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " IP not found, list is empty!" << std::endl;
		printTRA("Time Ref remove failed: IP not found, list is empty!");
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s. IP not found, list is empty.",
					trefName.c_str(),
					trefIP.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
		}
		return TREFC_Util::TIME_REF_NOT_EXIST;
	}

	//	/**********************************************/
	//	/*					Only for debug
	std::cout << std::endl << __FUNCTION__ << "@" << __LINE__ << " ----------- AFTER RM ----------- " << std::endl;
	for (it = m_tRefList.begin() ; it != m_tRefList.end() ; it++)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<
				" IP: " << it->first.c_str() <<
				" Name: " << it->second.c_str() << std::endl;
	}
	//	/**********************************************/
	//


	//	/**********************************************/
	//	/* send JTP notification for remove ! */
	//	/**********************************************/
	if ( servStatus == IMM_Util::ENABLED )
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"REMOVE ENTRY when service is enabled" <<std::endl; //mychange
		std::cout <<"********************** REMOVE ENTRY when service is enabled **********************"<<endl;
		unsigned long ipVal = 0;
		unsigned short refVal = 0;
		if ( trefIP == "GPS DEVICE" )
		{
			std::cout <<" -----------> GPSSSSSSSSSSS..........."<<__LINE__<<"\t"<<std::endl;
			refVal = 2; // set 2 if Reference is GPS
		}
		else
		{
			std::cout <<" -----------> TIME REFERENCE..........."<<__LINE__<<"\t"<<std::endl;
			checkIPAddresses(trefIP.c_str(),ipVal);
			std::cout<< " ipstr : "<< trefIP.c_str()<<"\t ipVal :"<<ipVal<<std::endl;
			refVal = 1;  // set 1 if Reference is IP
		}

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Time Server Removed, name: %s, IP address: %s. Sending JTP Notification.",trefName.c_str(),
					trefIP.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
		}
		
		if(m_jtpHandlerTref)
			m_jtpHandlerTref->setReferenceValues(ipVal,refVal);

		if(servType == IMM_Util::NTP)
		{
			if(m_jtpHandlerTref)
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ <<"Send NOtification for Ntp to remove" <<std::endl; //mychange
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_REFERENCE_REMOVE,1);  // 1 indicates NTP service
			}
		}
		else if(servType == IMM_Util::NTPHP)
		{
			if(refVal == 2) //GPS
			{
				if(m_jtpHandlerTref)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"SEnd Notification for NTPHP with GPS devices to remove" <<std::endl; //mychange
					m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_REFERENCE_REMOVE,3);  // 3 means NTPHP service with GPS
				}
			}
			else
			{
				if(m_jtpHandlerTref)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"SEnd Notification for NTPHP" <<std::endl; //mychange
					m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_REFERENCE_REMOVE,2);  // 2 means NTPHP service
				}
			}
		}
	} else {

		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Time Server Removed, name: %s, IP address: %s.",trefName.c_str(),
				trefIP.c_str());
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

	}
	//	/**********************************************/

	//Alarm handling
	checkAlarmStatus(false);
	return res;
}



int FIXS_TREFC_Manager::trefcStart ()
{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"Enter trefcStart" <<std::endl; //mychange
	string otherIpA[2] = {"",""};
	string otherIpB[2] = {"",""};

	int apzVecSize = 0;
	int aptVecSize = 0;
	//bool result;

	apzVecSize = (int)m_switchAPZVector.size();
	aptVecSize = (int)m_switchAPTVector.size();
	//
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Entering Time Service Start method.", LOG_LEVEL_DEBUG);

	if(servStatus == IMM_Util::ENABLED)
	{
		printTRA("Service Start rejected: TIMESERVICE_ALREADY_STARTED");
		FIXS_TREFC_Logger::getLogInstance()->Write("Service Start rejected: TIMESERVICE_ALREADY_STARTED", LOG_LEVEL_ERROR);
		return TIMESERVICE_ALREADY_STARTED;
	}

	servStatus = IMM_Util::BUSY;

	if (refType == IMM_Util::NOT_CONFIGURED)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"refType : "<<refType<<"servType : "<<servType <<std::endl; //mychange
		cout << " refType:" << refType << " servType:" << servType << endl;
		printTRA("Service Start failed: INTERNALERR");
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start failed. No Reference or Service Type Specified!", LOG_LEVEL_ERROR);
		servStatus = IMM_Util::DISABLED;

		return TIME_REF_NOT_CONFIGURED;
	}

	if (servType == IMM_Util::NONE)
	{
		cout << " refType:" << refType << " servType:" << servType << endl;
		printTRA("Service Start failed: INTERNALERR");
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start failed. No Reference or Service Type Specified!", LOG_LEVEL_ERROR);
		servStatus = IMM_Util::DISABLED;

		return INCORRECT_SERVICE;
	}

	if( (refType == IMM_Util::NETWORK_REFERENCE) && (m_tRefList.empty())) {
		printTRA("Service Start rejected: NO_REFERENCE_EXIST");
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start rejected: No network reference defined!", LOG_LEVEL_ERROR);
		servStatus = IMM_Util::DISABLED;
		return NO_REFERENCE_EXIST;
	}

	if(isStopOperation()) return -1; // if Shut down event signaled, return

	int res1 = 0, res2 = 0, res1_extIp = 0, res2_extIp = 0;

	//send JTP query to CP for starting time service. If yes, continue. If no, return with fault code.
	// send JTP notification CANSTART
	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Sending JTP CANSTART query...");

	bool CanStartQuery =false;

	if(servType == IMM_Util::NTP)
	{
		if(m_jtpHandlerTref)
		{	
			std::cout << __FUNCTION__ << "@" << __LINE__ << " JtpHandlerTref for NTP before  : " <<std::endl; //mychange
			m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_CAN_START,1);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " JtpHandlerTref for NTP  after: " <<std::endl; //mychange
		}
	}
	else
	{
		if(refType == IMM_Util::GPS_REFERENCE)
		{
			if(m_jtpHandlerTref)
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " JtpHandlerTref for GPS before : "<< std::endl; //mychange
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_CAN_START,3);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " JtpHandlerTref for GPS after  : " <<std::endl; //mychange
			}
		}
		else
		{
			if(m_jtpHandlerTref)
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " JtpHandlerTref for default  before: " << std::endl; //mychange
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_CAN_START,2);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " JtpHandlerTref for default after : " <<std::endl; //mychange
			}
		}
	}
	if(isStopOperation()) return -1; // if Shut down event signaled, return

	//Wait for Can Start query to finish
	if(m_jtpHandlerTref)
		m_jtpHandlerTref->waitCanStartQueryToFinish();
	if(isStopOperation()) return -1; // if Shut down event signaled, return

	//Check if the CP answered to Can Start query
	if(m_jtpHandlerTref)
		CanStartQuery = m_jtpHandlerTref->checkCanSendStartR();
	else
		CanStartQuery = true; 

	if (CanStartQuery != true)
	{
		printTRA("Service Start failed: NOT_ALLOWED_BY_CP");
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start failed: NOT ALLOWED BY CP", LOG_LEVEL_ERROR);
		servStatus = IMM_Util::DISABLED;
		if(isStopOperation()) return -1;
		else
		return TREFC_Util::NOT_ALLOWED_BY_CP;
	}

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] JTP CANSTART response received! Time Service Start allowed.");

	//Stop Time Checker
	m_timeChecker->stopTimeSet();


	if(servType == IMM_Util::NTP){
		if (refType == IMM_Util::GPS_REFERENCE) //GPS device found
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Command rejected as GPS is not allowed for NTP" << std::endl;
			printTRA("Service Start rejected: GPS is not allowed for NTP");
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start rejected: GPS is not allowed for NTP", LOG_LEVEL_ERROR);
			m_timeChecker->startTimeSet();
			servStatus = IMM_Util::DISABLED;
			if(isStopOperation()) return -1;
			else
			return TREFC_Util::INVALID_REF;
		}


		if(m_apzScxHandler[0] != NULL)
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Adding External IP address configuration on SCX %d.", m_switchAPZVector[0].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
			}
			if(isStopOperation()) return -1; // if Shut down event signaled, return

			if(m_switchAPZVector[0].slotPos == 0)
			{
				if(!extIpSwitch0.extIp.empty())
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1_extIp: " << res1_extIp << " Before setting extIp for slot 0: " << std::endl; //mychange
					res1_extIp = m_apzScxHandler[0]->addExtIp(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1_extIp: " << res1_extIp << " SET successfully done on extIp for slot 0: " <<std::endl; //mychange
				}
			}
			else if(m_switchAPZVector[0].slotPos == 25)
			{
				if(!extIpSwitch25.extIp.empty())
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1_extIp: " << res1_extIp << " Before setting extIp for slot 25: " << std::endl; //mychange
					res1_extIp = m_apzScxHandler[0]->addExtIp(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1_extIp: " << res1_extIp << " SET successfully done on extIp for slot 25: " <<std::endl; //mychange
				}
			}
			if(isStopOperation()) return -1; // if Shut down event signaled, return

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting NTP Service on SCX %d.", m_switchAPZVector[0].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
			}
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1: " << res1 << " Before starting NTP Service for handler[0] " << std::endl; //mychange
			res1 = m_apzScxHandler[0]->startNtp(m_tRefList);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1: " << res1 << " Start NTP Service for handler[0] " << std::endl; //mychange
		}
		if(m_apzScxHandler[1] != NULL)
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Adding External IP address configuration on SCX %d.", m_switchAPZVector[1].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
			}
			if(isStopOperation()) return -1; // if Shut down event signaled, return

			if(m_switchAPZVector[1].slotPos == 0)
			{
				if(!extIpSwitch0.extIp.empty())
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1_extIp: " << res1_extIp << " Before setting extIp for slot 0 : " << std::endl; //mychange
					res2_extIp = m_apzScxHandler[1]->addExtIp(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1_extIp: " << res1_extIp << " SET successfully done on res1_extIp for slot 0 in handler[1]: " << std::endl; //mychange
				}
			}
			else if(m_switchAPZVector[1].slotPos == 25)
			{
				if(!extIpSwitch25.extIp.empty())
				{
					 std::cout << __FUNCTION__ << "@" << __LINE__ << " Res1_extIp: " << res1_extIp << " Before setting extIp for slot 25 : " << std::endl; //mychange
					res2_extIp = m_apzScxHandler[1]->addExtIp(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SET successfully done on res1_extIp for slot 25 in handler[1]: " << std::endl; //mychange
				}
			}
			if(isStopOperation()) return -1; // if Shut down event signaled, return

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting NTP Service on SCX %d.", m_switchAPZVector[1].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
			}
			
			std::cout << __FUNCTION__ << "@" << __LINE__ <<" Before starting NTP Service: "<< std::endl; //mychange
			res2 = m_apzScxHandler[1]->startNtp(m_tRefList);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res2: " << res2 << " Starting NTP Service: "<< std::endl; //mychange
		}
		if(isStopOperation()) return -1; // if Shut down event signaled, return

		if(res1!=0 && res2!=0) {
			printTRA("Service Start failed: INTERNALERR");
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start failed: SNMP Failure.", LOG_LEVEL_ERROR);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMP Failure in starting NTP service: " << std::endl; //mychange
			//Start service failed, restart Time Checker
			m_timeChecker->startTimeSet();
			servStatus = IMM_Util::DISABLED;
			if(isStopOperation()) return -1;
			else
			return SNMP_FAILURE;
		}

		if(res1!=0)
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Start Failed on SCX %d. Setting Retries...", m_switchAPZVector[0].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_WARN);
			}
			if(isStopOperation()) return -1; // if Shut down event signaled, return

			if(m_switchAPZVector[0].slotPos == 0)
			{
				if(m_apzScxHandler[0] != NULL)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Before setting NTPConf , slot 0 in handler[0]: " << std::endl; //mychange
					m_apzScxHandler[0]->setNtpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SET successfully done for NTPConf , slot 0 in handler[0]: " << std::endl; //mychange
				}
			}
			else if(m_switchAPZVector[0].slotPos == 25)
			{
				if(m_apzScxHandler[0] != NULL)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Before setting NTPConf , slot 25 in handler[0]: " << std::endl; //mychange
					m_apzScxHandler[0]->setNtpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SET successfully done for NTPConf , slot 25 in handler[0]: " <<std::endl; //mychange
				}
			}

		}
		if(res2!=0)
		{
			if(isStopOperation()) return -1; // if Shut down event signaled, return
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Start Failed on SCX %d. Setting Retries...", m_switchAPZVector[1].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_WARN);
			}

			if(m_switchAPZVector[1].slotPos == 0)
			{
				if(m_apzScxHandler[1] != NULL)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Before setting NTPConf , slot 0 in handler[1]: " << std::endl; //mychange
					m_apzScxHandler[1]->setNtpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SET successfully done for NTPConf , slot 0 in handler[1]: " << std::endl; //mychange
				}
			}
			else if(m_switchAPZVector[1].slotPos == 25)
			{
				if(m_apzScxHandler[1] != NULL)
				{
                                        std::cout << __FUNCTION__ << "@" << __LINE__ << " Before setting NTPConf , slot 25 in handler[1]: " << std::endl; //mychange
					m_apzScxHandler[1]->setNtpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SET successfully done for NTPConf , slot 25 in handler[1]: " <<std::endl; //mychange
				}
			}
		}
	}
	else if(servType == IMM_Util::NTPHP)
	{
		if(isStopOperation()) return -1; // if Shut down event signaled, return
		TREFC_Util::EnvironmentType env = TREFC_Util::getEnvironment();
                if ( (env == TREFC_Util::SMX) && (refType == IMM_Util::NETWORK_REFERENCE)) //Network reference with NTPHP not allowed in SMX
                {
                        std::cout << __FUNCTION__ << "@" << __LINE__ << " Command rejected as network reference is not allowed for NTPHP" << std::endl;
                        printTRA("Service Start rejected: Network reference is not allowed for NTPHP");
                        FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start rejected: Network reference is not allowed for NTPHP", LOG_LEVEL_ERROR);
                        m_timeChecker->startTimeSet();
                        servStatus = IMM_Util::DISABLED;
                        if(isStopOperation()) return -1;
                        else
                        return TREFC_Util::INVALID_REF;
                }

		if(apzVecSize == 2)
		{
			otherIpA[0] = m_switchAPZVector[1].IP_EthA;
			otherIpB[0] = m_switchAPZVector[1].IP_EthB;
			otherIpA[1] = m_switchAPZVector[0].IP_EthA;
			otherIpB[1] = m_switchAPZVector[0].IP_EthB;
		}

		if (refType == IMM_Util::GPS_REFERENCE) //GPS device present
		{
			if(m_apzScxHandler[0] != NULL)
			{

				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting NTPHP Service with GPS on SCX %d.", m_switchAPZVector[0].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntphp service with GPS: " << std::endl; //mychange
				res1 = m_apzScxHandler[0]->startNtphpGps(otherIpA[0], otherIpB[0]);
				std::cout << __FUNCTION__ << "@" << __LINE__ << "Starting ntphp service with GPS: " << std::endl; //mychange
			}

			if(m_apzScxHandler[1] != NULL)
			{

				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting NTPHP Service with GPS on Switch %d.", m_switchAPZVector[1].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntphp service with GPS: " << std::endl; //mychange
				res2 = m_apzScxHandler[1]->startNtphpGps(otherIpA[1], otherIpB[1]);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Starting NTPHP Service with GPS on Switch " << std::endl; //mychange
			}
		}
		else
		{
			if(m_apzScxHandler[0] != NULL)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Adding External IP address configuration on Switch %d.", m_switchAPZVector[0].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}

				if(m_switchAPZVector[0].slotPos == 0)
				{

					if(!extIpSwitch0.extIp.empty())
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Before setting extIp, slot 0 in handle[0] " <<std::endl; //mychange
						res1_extIp = m_apzScxHandler[0]->addExtIp(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
						std::cout << __FUNCTION__ << "@" << __LINE__ << " SEt successfully extIp, slot 0 in handle[0] " <<std::endl; //mychange
					}
				}
				else if(m_switchAPZVector[0].slotPos == 25)
				{


					if(!extIpSwitch25.extIp.empty())
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Before setting extIp " <<std::endl; //mychange
						res1_extIp = m_apzScxHandler[0]->addExtIp(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
						std::cout << __FUNCTION__ << "@" << __LINE__ << " SEt successfully extIp, slot 25 in handle[0] "<< std::endl; //mychange
					}

				}
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting NTPHP Service on Switch %d.", m_switchAPZVector[0].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting Ntphp"<<std::endl; //mychange
				res1 = m_apzScxHandler[0]->startNtphp(m_tRefList, otherIpA[0], otherIpB[0]);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Start NTPHP service" << std::endl; //mychange
			}
			if(isStopOperation()) return -1; // if Shut down event signaled, return

			if(m_apzScxHandler[1] != NULL)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Adding External IP address configuration on Switch %d.", m_switchAPZVector[1].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}


				if(m_switchAPZVector[1].slotPos == 0)
				{
					if(!extIpSwitch0.extIp.empty())
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Before adding extIp, slot 0 in handle[1] " <<std::endl; //mychange
						res2_extIp = m_apzScxHandler[1]->addExtIp(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
						std::cout << __FUNCTION__ << "@" << __LINE__ << " SEt successfully extIp, slot 0 in handle[1] " <<std::endl; //mychange
					}
				}
				else if(m_switchAPZVector[1].slotPos == 25)
				{
					if(!extIpSwitch25.extIp.empty())
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Before adding extIp, slot 25 in handle[1] " <<std::endl; //mychange
						res2_extIp = m_apzScxHandler[1]->addExtIp(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
						std::cout << __FUNCTION__ << "@" << __LINE__ << " SEt successfully extIp, slot 25 in handle[1] "<<std::endl; //mychange
					}

				}
				if(isStopOperation()) return -1; // if Shut down event signaled, return
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting NTPHP Service on Switch %d.", m_switchAPZVector[1].slotPos);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
				}
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting Ntphp Service:" <<std::endl; //mychange
				res2 = m_apzScxHandler[1]->startNtphp(m_tRefList, otherIpA[1], otherIpB[1]);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " start NTPhp handler[1]: " <<std::endl; //mychange
			}
		}
		if(res1!=0 && res2!=0) {

			printTRA("Service Start failed: INTERNALERR");
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start failed: SNMP Failure.", LOG_LEVEL_ERROR);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Service Start failed:SNMP failure." <<std::endl; //mychange
			//Start service failed, restart Time Checker
			m_timeChecker->startTimeSet();
			servStatus = IMM_Util::DISABLED;
			if(isStopOperation()) return -1;
			else
			return SNMP_FAILURE;
		}


		if(res1!=0)
		{
			if(isStopOperation()) return -1; // if Shut down event signaled, return
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Start Failed on Switch %d. Setting Retries...", m_switchAPZVector[0].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_WARN);
			}

			if(m_switchAPZVector[0].slotPos == 0)
			{
				if(m_apzScxHandler[0] != NULL)
					m_apzScxHandler[0]->setNtpHpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, otherIpA[0], otherIpB[0]);
			}
			else if(m_switchAPZVector[0].slotPos == 25)
			{
				if(m_apzScxHandler[0] != NULL)
					m_apzScxHandler[0]->setNtpHpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, otherIpA[0], otherIpB[0]);
			}
		}
		if(res2!=0)
		{
			if(isStopOperation()) return -1; // if Shut down event signaled, return
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Start Failed on Switch %d. Setting Retries...", m_switchAPZVector[1].slotPos);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_WARN);
			}

			if(m_switchAPZVector[1].slotPos == 0)
			{
				if(m_apzScxHandler[1] != NULL)
					m_apzScxHandler[1]->setNtpHpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, otherIpA[1], otherIpB[1]);
			}
			else if(m_switchAPZVector[1].slotPos == 25)
			{
				if(m_apzScxHandler[1] != NULL)
					m_apzScxHandler[1]->setNtpHpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, otherIpA[1], otherIpB[1]);
			}
		}

	}
	else
	{
		printTRA("Service Start failed: INCORRECT_SERVICE");
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Service Start failed: INCORRECT_SERVICE", LOG_LEVEL_ERROR);
		//Start service failed, restart Time Checker
		m_timeChecker->startTimeSet();
		servStatus = IMM_Util::DISABLED;
		if(isStopOperation()) return -1;
		else
		return INCORRECT_SERVICE;
	}
	
	if(isStopOperation()) return -1; // if Shut down event signaled, return
	//External ip address retry (if required)
	if(res1_extIp != 0)
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Configuration of external IP address failed on SCX %d. Setting Retries...", m_switchAPZVector[0].slotPos);
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_WARN);
		}

		if(m_switchAPZVector[0].slotPos == 0)
		{
			if(m_apzScxHandler[0] != NULL)
				m_apzScxHandler[0]->setAddExtIpRetry(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, true);
		}
		else if(m_switchAPZVector[0].slotPos == 25)
		{
			if(m_apzScxHandler[0] != NULL)
				m_apzScxHandler[0]->setAddExtIpRetry(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, true);
		}

	}
	if(isStopOperation()) return -1; // if Shut down event signaled, return
	if(res2_extIp != 0)
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Configuration of external IP address failed on SCX %d. Setting Retries...", m_switchAPZVector[1].slotPos);
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
		}

		if(m_switchAPZVector[1].slotPos == 0)
		{
			if(m_apzScxHandler[1] != NULL)
				m_apzScxHandler[1]->setAddExtIpRetry(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, true);
		}
		else if(m_switchAPZVector[1].slotPos == 25)
		{
			if(m_apzScxHandler[1] != NULL)
				m_apzScxHandler[1]->setAddExtIpRetry(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, true);
		}

	}
	if(isStopOperation()) return -1; // if Shut down event signaled, return
	//APT
	aptVecSize = (int)m_switchAPTVector.size();
	for (int j=0;j<aptVecSize;j++)
	{
		if(m_aptScxHandler[j] != NULL)
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Setting NTP Client Configuration on APT SCX %d, IP: %s.",
					j+1, m_switchAPTVector[j].IP_EthA.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

			m_aptScxHandler[j]->setNtpClientConf(m_switchAPZVector,m_switchAPTVector[j].ethInPort);
		}
	}
	if(isStopOperation()) return -1; // if Shut down event signaled, return
	//Set server status to enabled
	servStatus= IMM_Util::ENABLED;

	//Modify IMM Attributes
	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Updating service status.");
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"before setServiceStatus" <<std::endl; //mychange
	ImmInterface::setServiceStatus(servStatus);
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"after setServiceStatus" <<std::endl; //mychange
	if(isStopOperation()) return -1; // if Shut down event signaled, return
	//	// send JTP notification service START
	if(servType == IMM_Util::NTP) {
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Sending JTP Notification: NTP Service Started.");
		if(m_jtpHandlerTref)
			m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_START,1);
	} else if(servType == IMM_Util::NTPHP)
	{
		if (refType == IMM_Util::GPS_REFERENCE) //GPS device
		{
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Sending JTP Notification: NTPHP Service with GPS Device Started.");
			if(m_jtpHandlerTref)
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_START,3);
		}
		else
		{
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Sending JTP Notification: NTPHP Service Started.");
			if(m_jtpHandlerTref)
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_START,2);
		}
	}
	//
	//	//Call the CSAPI method to set the type here
	//

	if(isStopOperation()) return -1; // if Shut down event signaled, return
	//Alarm handling
	checkAlarmStatus(false);

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Exiting Time Service Start method: Start action Completed Successfully!", LOG_LEVEL_DEBUG);
	return EXECUTED;
}


int FIXS_TREFC_Manager::trefcStop ()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Entering Time Service Stop method...", LOG_LEVEL_DEBUG);

	int res1=0, res2=0;
	if(servStatus == IMM_Util::DISABLED) {
		printTRA("Stop Time Service rejected: TIMESERVICE_ALREADY_STOPPED");
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Stop Time Service rejected: TIMESERVICE_ALREADY_STOPPED.", LOG_LEVEL_ERROR);
		return TIMESERVICE_ALREADY_STOPPED;
	}

	if(servType == IMM_Util::NTP)
	{
		if(isStopOperation()) return -1; // if Shut down event signaled, return
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Stopping NTP Time Service...");
		if(m_apzScxHandler[0] != NULL)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Before stopping NTp service  : " <<std::endl; //mychange
			res1 = m_apzScxHandler[0]->stopNtp();
			std::cout << __FUNCTION__ << "@" << __LINE__ <<"res :"<<res1<< " NTP service stopped: " <<std::endl; //mychange
		}
		if(m_apzScxHandler[1] != NULL)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Before stopping NTp service  : " <<std::endl; //mychange
			res2 = m_apzScxHandler[1]->stopNtp();
			std::cout << __FUNCTION__ << "@" << __LINE__ <<"res :"<<res2<< " NTP service stopped: " <<std::endl; //mychange
		}

		if(res1!=0 && res2!=0) {
			printTRA("Stop Time Service failed: INTERNALERR");
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Stop NTP Time Service failed: SNMP FAILURE", LOG_LEVEL_ERROR);
			std::cout << __FUNCTION__ << "@" << __LINE__ <<" Stop NTP Service failed: SNMP Failure " <<std::endl; //mychange
			if(isStopOperation()) return -1;
			else
			return SNMP_FAILURE;
		}
		if(res1!=0)
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Stop NTP Time Service failed on APZ SCX %s: Setting retry!",
					m_apzScxHandler[0]->m_ipA.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
			m_apzScxHandler[0]->setStopNtpRetry();
		}
		if(isStopOperation()) return -1; // if Shut down event signaled, return
		if(res2!=0)
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Stop NTP Time Service failed on APZ SCX %s: Setting retry!",
					m_apzScxHandler[1]->m_ipA.c_str());

			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
			m_apzScxHandler[1]->setStopNtpRetry();

		}

	}
	else if(servType == IMM_Util::NTPHP)
	{
		if(isStopOperation()) return -1; // if Shut down event signaled, return
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Stopping NTPHP Time Service...");
		if(m_apzScxHandler[0] != NULL)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ <<" Before Ntphp service stop " <<std::endl; //mychange
			res1 = m_apzScxHandler[0]->stopNtphp();
			std::cout << __FUNCTION__ << "@" << __LINE__ <<"res :"<<res1<< " Ntphp service stopped: " <<std::endl; //mychange
		}
		if(m_apzScxHandler[1] != NULL)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ <<" Before Ntphp service stop " <<std::endl; //mychange
			res2 = m_apzScxHandler[1]->stopNtphp();
			std::cout << __FUNCTION__ << "@" << __LINE__ <<"res :"<<res2<< " Ntphp service stopped: " <<std::endl; //mychange
		}

		if(res1!=0 && res2!=0) {
			printTRA("Stop Time Service failed: INTERNALERR");
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Stop NTPHP Time Service failed: SNMP FAILURE", LOG_LEVEL_ERROR);
			std::cout << __FUNCTION__ << "@" << __LINE__ <<" Stop Ntphp service failed: Snmp Failure " <<std::endl; //mychange
			if(isStopOperation()) return -1;	
			else
			return SNMP_FAILURE;
		}

		if(res1!=0)
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Stop NTPHP Time Service failed on APZ SCX %s: Setting retry!",
					m_apzScxHandler[0]->m_ipA.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
			m_apzScxHandler[0]->setStopNtpHpRetry();
		}
		if(res2!=0)
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Stop NTPHP Time Service failed on APZ SCX %s: Setting retry!",
					m_apzScxHandler[1]->m_ipA.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);
			m_apzScxHandler[1]->setStopNtpHpRetry();
		}
	}
	else
	{
		printTRA("Stop Time Service rejected: INCORRECT_SERVICE");
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Stop Time Service rejected: INCORRECT_SERVICE", LOG_LEVEL_ERROR);
		if(isStopOperation()) return -1;
		else
		return INTERNALERR;
	}
	if(isStopOperation()) return -1; // if Shut down event signaled, return

	//remove external ip address
	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Removing External IP address configuration");

	if(m_apzScxHandler[0] != NULL)
	{
		if(m_apzScxHandler[0]->rmExtIp() != 0)
		{
			m_apzScxHandler[0]->setRmExtIpRetry();
		}
	}
	if(m_apzScxHandler[1] != NULL)
	{
		if(m_apzScxHandler[1]->rmExtIp() != 0)
		{
			m_apzScxHandler[1]->setRmExtIpRetry();
		}
	}
	if(isStopOperation()) return -1; // if Shut down event signaled, return

	//APT
	int aptVecSize = (int)m_switchAPTVector.size();
	for (int j=0;j<aptVecSize;j++)
	{
		if(m_aptScxHandler[j] != NULL)
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Removing NTP Client Configuration on APT SCX %d, IP: %s.",
					j+1, m_switchAPTVector[j].IP_EthA.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

			m_aptScxHandler[j]->setStopNtpClient();
		}
	}
	if(isStopOperation()) return -1; // if Shut down event signaled, return

	//Start Time Checker
	m_timeChecker->startTimeSet();

	//Set server status to disabled
	servStatus= IMM_Util::DISABLED;

	//Modify IMM Attributes
	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Updating service status.");
	if(isStopOperation()) return -1;
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"before setServiceStatus" <<std::endl; //mychange
	ImmInterface::setServiceStatus(servStatus);
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"after setServiceStatus" <<std::endl; //mychange
//	ImmInterface::setServiceType(IMM_Util::NONE);
	if(isStopOperation()) return -1; // if Shut down event signaled, return

	//Notify stop evnt via JTP
	if(servType == IMM_Util::NTP)
	{
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Sending JTP Notification: NTP Service Stopped.");
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"Sending JTP Notification: NTP Service Stopped. " <<std::endl; //mychange
		if(m_jtpHandlerTref)
			m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_STOP,1);  // 1 indicates NTP service
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"Send JTP Notification: NTP Service Stopped.  " <<std::endl; //mychange
	}
	else if(servType == IMM_Util::NTPHP)
	{
		if (refType == IMM_Util::GPS_REFERENCE) //GPS device
		{
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Sending JTP Notification: NTPHP Service with GPS Device Stopped.");
			if(m_jtpHandlerTref)
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_STOP,3);
		}
		else
		{
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Sending JTP Notification: NTPHP Service Stopped.");
			if(m_jtpHandlerTref)
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_STOP,2);  // other than 1 means NTPHP service
		}
	}
	if(isStopOperation()) return -1; // if Shut down event signaled, return
	//Alarm handling
	checkAlarmStatus(false);

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Exiting Time Service Stop method: Stop action Completed Successfully!", LOG_LEVEL_DEBUG);
	
	return EXECUTED;

}



int FIXS_TREFC_Manager::GetAPZSwitchMagazineSlot (unsigned char &shelf, bool &slot0, bool &slot25)
{
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	bool isSMX = false;
	if(environment == TREFC_Util::SMX)
		isSMX = true;
	return ImmInterface::getAPZSwitchMagazineSlot(shelf, slot0, slot25,isSMX);
}


int FIXS_TREFC_Manager::getTRefListStatus (int *status_scxA, int *status_scxB, string *activeRef_scxA, string *activeRef_scxB)
{
	int scxA_status = 0, scxB_status = 0;
	string ref_scxA(""), ref_scxB("");

	if(servStatus == IMM_Util::ENABLED)
	{
		if(servType == IMM_Util::NTP)
		{
			if (m_switchAPZVector.size() > 0) {
				if(m_switchAPZVector[0].slotPos == 0)
				{
					if(m_apzScxHandler[0])
					{
						if(m_apzScxHandler[0]->getTimeRefStatusNtp(&scxA_status, &ref_scxA) != 0)
						{
							cout << "Failed to get status and active reference for SCX-A" << endl;
							printTRA("Failed to get status and active reference for SCX-A");
							FIXS_TREFC_Logger::getLogInstance()->Write("Failed to get status and active reference for SCX-A", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
					if(m_apzScxHandler[1])
					{
						if(m_apzScxHandler[1]->getTimeRefStatusNtp(&scxB_status, &ref_scxB) != 0)
						{
							cout << "Failed to get status and active reference for SCX-B" << endl;
							printTRA("Failed to get status and active reference for SCX-B");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX-B", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
				}
				else if (m_switchAPZVector[0].slotPos == 25)
				{
					if(m_apzScxHandler[0])
					{
						if(m_apzScxHandler[0]->getTimeRefStatusNtp(&scxB_status, &ref_scxB) != 0)
						{
							cout << "Failed to get status and active reference for SCX-B" << endl;
							printTRA("Failed to get status and active reference for SCX-B");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX-B", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
					if(m_apzScxHandler[1])
					{
						if(m_apzScxHandler[1]->getTimeRefStatusNtp(&scxA_status, &ref_scxA) != 0)
						{
							cout << "Failed to get status and active reference for SCX-A" << endl;
							printTRA("Failed to get status and active reference for SCX-A");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX-A", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
				}
			}

		} //ntp

		else if(servType == IMM_Util::NTPHP)
		{
			if(isGPSList())
			{
				scxA_status = 1;
				scxB_status = 1;
			}
			if (m_switchAPZVector.size() > 0) {
				if(m_switchAPZVector[0].slotPos == 0)
				{
					if(m_apzScxHandler[0])
					{
						if(m_apzScxHandler[0]->getTimeRefStatusNtphp(&scxA_status, &ref_scxA) != 0)
						{
							cout << "Failed to get status and active reference for SCX-A" << endl;
							printTRA("Failed to get status and active reference for SCX-A");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX-A", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
					if(m_apzScxHandler[1])
					{
						if(m_apzScxHandler[1]->getTimeRefStatusNtphp(&scxB_status, &ref_scxB) != 0)
						{
							cout << "Failed to get status and active reference for SCX-B" << endl;
							printTRA("Failed to get status and active reference for SCX-B");
							FIXS_TREFC_Logger::getLogInstance()->Write("Failed to get status and active reference for SCX-B", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
				}
				else if (m_switchAPZVector[0].slotPos == 25)
				{
					if(m_apzScxHandler[0])
					{
						if(m_apzScxHandler[0]->getTimeRefStatusNtphp(&scxB_status, &ref_scxB) != 0)
						{
							cout << "Failed to get status and active reference for SCX-B" << endl;
							printTRA("Failed to get status and active reference for SCX-B");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX-B", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
					if(m_apzScxHandler[1])
					{
						if(m_apzScxHandler[1]->getTimeRefStatusNtphp(&scxA_status, &ref_scxA) != 0)
						{
							cout << "Failed to get status and active reference for SCX-A" << endl;
							printTRA("Failed to get status and active reference for SCX-A");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX-A", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
				}
			}
		} //ntphp
	} //enabled

	*status_scxA = scxA_status;
	*status_scxB = scxB_status;
	*activeRef_scxA = ref_scxA;
	*activeRef_scxB = ref_scxB;
	return 0;
}

int FIXS_TREFC_Manager::getTRefListStatus (int *status_scxA, string *activeRef_scxA, string *activeRef_name, int slot)
{
	int scxA_status = 0;
	string ref_scxA("");

	if(servStatus == IMM_Util::ENABLED)
	{
		if(servType == IMM_Util::NTP)
		{
			if (m_switchAPZVector.size() > 0) {
				if(m_switchAPZVector[0].slotPos == slot)
				{
					if(m_apzScxHandler[0])
					{
						if(m_apzScxHandler[0]->getTimeRefStatusNtp(&scxA_status, &ref_scxA) != 0)
						{
							cout << "Failed to get status and active reference for SCX-A" << endl;
							printTRA("Failed to get status and active reference for SCX-A");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
				}
			}

			if (m_switchAPZVector.size() > 1) {
				if (m_switchAPZVector[1].slotPos == slot)
				{
					if(m_apzScxHandler[1])
					{
						if(m_apzScxHandler[1]->getTimeRefStatusNtp(&scxA_status, &ref_scxA) != 0)
						{
							cout << "Failed to get status and active reference for SCX-B" << endl;
							printTRA("Failed to get status and active reference for SCX-B");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
				}
			}

		} //ntp

		else if(servType == IMM_Util::NTPHP)
		{
			if(refType == IMM_Util::GPS_REFERENCE)
			{
				scxA_status = 1;
			}
			if (m_switchAPZVector.size() > 0) {
				if(m_switchAPZVector[0].slotPos == slot)
				{
					if(m_apzScxHandler[0])
					{
						if(m_apzScxHandler[0]->getTimeRefStatusNtphp(&scxA_status, &ref_scxA) != 0)
						{
							cout << "Failed to get status and active reference for SCX-A" << endl;
							printTRA("Failed to get status and active reference for SCX-A");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
				}
			}
			if (m_switchAPZVector.size() > 1) {
				if (m_switchAPZVector[1].slotPos == slot)
				{
					if(m_apzScxHandler[1])
					{
						if(m_apzScxHandler[1]->getTimeRefStatusNtphp(&scxA_status, &ref_scxA) != 0)
						{
							cout << "Failed to get status and active reference for SCX-A" << endl;
							printTRA("Failed to get status and active reference for SCX-A");
							FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Failed to get status and active reference for SCX", LOG_LEVEL_ERROR);
							return 13; //SNMP_Failure
						}
					}
				}
			}
		} //ntphp
	} //enabled

	if(ref_scxA.find(':') != string::npos)
		ref_scxA = ref_scxA.substr(ref_scxA.find(':')+2,string::npos);

	*status_scxA = scxA_status;
	*activeRef_scxA = ref_scxA;

	if (refType == IMM_Util::GPS_REFERENCE){
		*activeRef_name = "GPS";
	}
	else if (refType == IMM_Util::NETWORK_REFERENCE){
		*activeRef_name = getNetworkReferenceName(ref_scxA);
	}
	else{
		*activeRef_name = "-";
	}

	return 0;
}

int FIXS_TREFC_Manager::getTrefList (bool isSMX)
{

	int res = 0;

	std::cout << "------------------------------------------------------------"<< std::endl;
	std::cout << "                 TIME REFERENCE CONFIGURATION              " << std::endl;
	std::cout << "------------------------------------------------------------"<< std::endl;

	//get status of the TREF service
	int status = -1;

	ImmInterface::getServiceStatus(status);

	if (status == IMM_Util::ENABLED)
	{
		servStatus =IMM_Util::ENABLED;
		ImmInterface::setServiceAdmState(IMM_Util::UNLOCKED);
		cout << " Service Status: ENABLED " << endl;
	}
	else
	{
		servStatus =IMM_Util::DISABLED;
		ImmInterface::setServiceAdmState(IMM_Util::LOCKED);
		cout << " Service Status: DISABLED " << endl;
	}

	//get type of the TREF service
	int type = -1;

	ImmInterface::getServiceType(type,isSMX);

	if (type == IMM_Util::NTP)
	{
		servType = IMM_Util::NTP;
		cout << " Service Type: NTP" << endl;
	}
	else if (type == IMM_Util::NTPHP)
	{
		servType = IMM_Util::NTPHP;
		cout << " Service Type: NTPHP" << endl;
	}
	else
	{
		servType = IMM_Util::NONE;
		cout << " Service Type: NONE" << endl;
	}

	//get type of the TREF reference
	int rType = -1;
	ImmInterface::getReferenceType(rType,isSMX);

	if (rType == IMM_Util::NETWORK_REFERENCE)
	{
		refType = IMM_Util::NETWORK_REFERENCE;
		cout << " Reference Type: NETWORK_REFERENCE" << endl;
	}
	else if (rType == IMM_Util::GPS_REFERENCE)
	{
		refType = IMM_Util::GPS_REFERENCE;
		cout << " Reference Type: GPS_REFERENCE" << endl;
	}
	else
	{
		refType = IMM_Util::NOT_CONFIGURED;
		cout << " Reference Type: NOT_CONFIGURED" << endl;
	}


	std::cout << "------------------------------------------------------------"<< std::endl;

	//get Time references list
	ImmInterface::getTimeReferencesList(m_tRefList,isSMX);

	//print references map
	std::cout << "\n------------------------------------------------------------"<< std::endl;
	std::cout << "                 TIME SERVER					                " << std::endl;
	std::cout << "------------------------------------------------------------"<< std::endl;

	std::map<string, string>::iterator it;
	for(it = m_tRefList.begin(); it != m_tRefList.end(); it++)
	{
		std::cout << " IP Address: "<<it->first.c_str()<< "\t Name: "<<it->second.c_str()<<endl;
	}

	return res;

}

bool FIXS_TREFC_Manager::isUniqueName (std::string name)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	bool res = false;
	std::map<std::string, std::string>::iterator it;
	for (it = m_tRefList.begin() ; it != m_tRefList.end() ; it++)
	{
		if (it->second.compare(name) == 0)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ <<
					" FOUND !! IP: " << it->first.c_str() <<
					" Name: " << it->second.c_str() << std::endl;

			return true;
		}
	}
	return res;
}

bool FIXS_TREFC_Manager::isUniqueName (std::string name, std::map<std::string, std::string> temp_tRefList)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	bool res = false;
	std::map<std::string, std::string>::iterator it;
	for (it = temp_tRefList.begin() ; it != temp_tRefList.end() ; it++)
	{
		if (it->second.compare(name) == 0)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ <<
					" FOUND !! IP: " << it->first.c_str() <<
					" Name: " << it->second.c_str() << std::endl;

			return true;
		}
	}
	return res;
}


bool FIXS_TREFC_Manager::isUniqueIp (std::string ip)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	bool res = false;
	unsigned long ipAddr, temp;
	checkIPAddresses(ip.c_str(),ipAddr);

	std::map<std::string, std::string>::iterator it;
	for (it = m_tRefList.begin() ; it != m_tRefList.end() ; it++)
	{
		checkIPAddresses(it->first.c_str(),temp);
		if(ipAddr == temp)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ <<
					" FOUND !! IP: " << it->first.c_str() <<
					" Name: " << it->second.c_str() << std::endl;

			return true;
		}
	}
	return res;

}

bool FIXS_TREFC_Manager::isGPSList ()
{

	bool res = false;
	map<string, string>::iterator it;
	it = m_tRefList.find("GPS DEVICE");
	if (it != m_tRefList.end())
	{
		res = true;
	}

	return res;

}

bool FIXS_TREFC_Manager::isGPSList (map<string, string> temp_tRefList)
{

	bool res = false;
	map<string, string>::iterator it;
	it = temp_tRefList.find("GPS DEVICE");
	if (it != temp_tRefList.end())
	{
		res = true;
	}

	return res;

}


int FIXS_TREFC_Manager::setTimeOnAllScx ()
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	int aptVectorSize = m_switchAPTVector.size();
	//	FIXS_TREFC_SNMPManager *snmpManager;
	//	snmpManager = new FIXS_TREFC_SNMPManager("dummy");
	if(m_switchAPZVector.size() > 0)
	{
		for(unsigned int i = 0; i < m_switchAPZVector.size(); i++)
		{
			if(m_apzScxHandler[i])
				m_apzScxHandler[i]->setTime();
		}
		//		snmpManager->setIpAddress(m_switchAPZVector[0].IP_EthA);
		//		if(!snmpManager->setSystemTime())
		//		{
		//			snmpManager->setIpAddress(m_switchAPZVector[0].IP_EthB);
		//			if(!snmpManager->setSystemTime())
		//			{
		//				cout << "Set system time failed for SCX with ips " <<  m_switchAPZVector[0].IP_EthA << " and " <<  m_switchAPZVector[0].IP_EthB << std::endl;
		//				delete snmpManager;
		//				return 13; //SNMP_Failure
		//			}
		//		}
		//		if(m_switchAPZVector.size() == 2)
		//		{
		//			snmpManager->setIpAddress(m_switchAPZVector[1].IP_EthA);
		//			if(!snmpManager->setSystemTime())
		//			{
		//				snmpManager->setIpAddress(m_switchAPZVector[1].IP_EthB);
		//				if(!snmpManager->setSystemTime())
		//				{
		//					cout << "Set system time failed for SCX with ips " <<  m_switchAPZVector[1].IP_EthA << " and " <<  m_switchAPZVector[1].IP_EthB << std::endl;
		//					delete snmpManager;
		//					return 13; //SNMP_Failure
		//				}
		//			}
		//		}
	}
	else
	{
		cout << "No SCX boards in APZ magazine!!" << endl;
		//		delete snmpManager;
		//		return 0;
	}

	if(aptVectorSize != 0)
	{
		for(int i = 0; i < aptVectorSize; i++)
		{
			if(m_aptScxHandler[i])
				m_aptScxHandler[i]->setTime();
			//			snmpManager->setIpAddress(m_switchAPTVector[i].IP_EthA);
			//			if(!snmpManager->setSystemTime())
			//			{
			//				snmpManager->setIpAddress(m_switchAPTVector[i].IP_EthB);
			//				if(!snmpManager->setSystemTime())
			//				{
			//					cout << "Set system time failed for SCX with ips " <<  m_switchAPTVector[i].IP_EthA << " and " <<  m_switchAPTVector[i].IP_EthB << std::endl;
			//					delete snmpManager;
			//					return 13; //SNMP_Failure
			//				}
			//			}
		}
	}
	else
	{
		cout << "No SCX boards in APT magazine!!" << endl;
		//		delete snmpManager;
		//		return 0;
	}
	//	delete snmpManager;
	return 0;

}

void FIXS_TREFC_Manager::update (const ACS_CS_API_HWCTableChange& observer)
{

	std::cout<< "observer.dataSize = " << observer.dataSize << std::endl;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	bool isApzScx = false;

	//if (observer.dataSize > 0 )
	for (int index = 0; index < observer.dataSize; index++)
	{
		//check for FBN and magazine
		string shelfAPZ_dn(""), apBoard_dn(""),apShelfAddress("");
		string scxMag("");

		//Check if the hwc change is about a SCX board
		if ((observer.hwcData[index].fbn == ACS_CS_API_HWC_NS::FBN_SCXB) || (observer.hwcData[index].fbn == ACS_CS_API_HWC_NS::FBN_SMXB))
		{
			cout << "SCXB/SMXB \n" << endl;
			cout << "Magazine : " << observer.hwcData[index].magazine << endl;


			TREFC_Util::ulongToStringMagazine(observer.hwcData[index].magazine, scxMag);
			std::istringstream mag(scxMag);
			TREFC_Util::reverseDottedDecStr(mag);
			scxMag = mag.str();
			cout << "Magazine as string : " << scxMag << endl;

			//Check if APZ or APT magazine
			string apzMag("");

			if (ImmInterface::getAPZShelfAddress(apzMag) != 0) {
				printTRA("Update procedure aborted: Failed to fetch APZ Magazine address");
				FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Update procedure aborted: Failed to fetch APZ Magazine address", LOG_LEVEL_ERROR);
				continue;
			}
			else
			{
				if(!m_jtpHandlerTref)
				{
					unsigned long temp;
					bool slot0 = false;
					bool slot25 = false;
					stringToUlongMagazine(apzMag, temp);
					unsigned long hlmag = ntohl(temp);
					unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
					unsigned char mag = u_plug0 & 0x0F;

					m_jtpHandlerTref = new FIXS_TREFC_JTPHandler(mag, slot0,slot25);
					m_jtpHandlerTref->open();
				}

			}


//			vector<string> apBladeList,otherBladeList,shelfList;
//			IMM_Util::getClassObjectsList(IMM_Util::classApBlade,apBladeList);
//
//			bool found = false;
//			for (unsigned i=0; i<apBladeList.size(); i++)
//			{
//				apBoard_dn.clear();
//				shelfAPZ_dn.clear();
//				apBoard_dn = apBladeList[i].c_str();
//
//				//get system Number of Ap Board
//				int sysNum = 0;
//				IMM_Util::getImmAttributeInt(apBoard_dn,IMM_Util::ATT_APBLADE_SYSNUM,sysNum);
//
//				if (sysNum == 1) //AP1
//				{
//					//get DN magazine
//					IMM_Util::getDnParent(apBoard_dn,shelfAPZ_dn);
//					cout << "shelfAPZ_dn" << shelfAPZ_dn << endl;
//					found = true;
//					break;
//				}
//			}
//			if(found)
//			{
//				IMM_Util::getImmAttributeString(shelfAPZ_dn, IMM_Util::ATT_SHELF_ADDRESS,apShelfAddress);
//				cout << "AP magazine : " << apShelfAddress << endl;
//			} else {
//				printTRA("Update procedure aborted: Failed to fetch APZ Magazine address");
//				return;
//			}

			//if(0 == scxMag.compare(apShelfAddress))

			cout << "apzMag = " << apzMag << endl;

			cout << "switchMag = " << scxMag << endl;

			//if(observer.hwcData[0].magazine == apzMag)
			if(0 == scxMag.compare(apzMag))
			{
				cout << "!! APZ Magazine SCX/SMX !!" << endl;
				isApzScx = true;
			}
			else
			{
				cout << "-- APT Magazine SCX/SMX --" << endl;
				isApzScx = false;
			}
		}
		else
		{
			cout <<"NOT SCX/SMX, discard" << endl;
			continue;
		}

		string ipA = TREFC_Util::ulongToStringIP(observer.hwcData[index].ipEthA);
		string ipB = TREFC_Util::ulongToStringIP(observer.hwcData[index].ipEthB);
		unsigned short uslot = observer.hwcData[index].slot;
		cout << "ipA : " << ipA << endl;
		cout << "ipB : " << ipB << endl;
		cout << "slot : " << uslot << endl;

		switch(observer.hwcData[index].operationType)
		{

		case ACS_CS_API_TableChangeOperation::Add:

			std::cout << "DBG: ------- Add -------- \n" << std::endl;

			if(isApzScx)
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] New Switch added on slot %d on APZ Magazine.", uslot);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

				SWITCHAPZIPAddress scx;
				scx.IP_EthA = ipA;
				scx.IP_EthB = ipB;
				scx.slotPos = uslot;

				//int ind = m_switchAPZVector.size();

				int ind = -1;

				for (unsigned int i = 0; i < m_switchAPZVector.size(); i++) {
					if (m_switchAPZVector[i].slotPos == uslot) {
						ind = i;
						break;
					}
				}

				if (ind >= 0 && ind < 2) {
					if (m_apzScxHandler[ind] != NULL) {
						cout << "A Thread already exists for this Switch. Closing it before starting new one..." << endl;
						m_apzScxHandler[ind]->close();
						for(int i = 0; ((i < 6) && m_apzScxHandler[ind]->isRunning()); i++) sleep(1);
						cout << "APZ Switch Handler Thread Finished" <<endl;
						delete m_apzScxHandler[ind];
						m_apzScxHandler[ind] = NULL;
					}
				} else
					ind = m_switchAPZVector.size();

				if (ind >= 2) {
					cout << "ERROR: Trying to add more than two SCX/SMX on APZ magazine." << endl;
					FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] ERROR: Trying to add more than two SCX/SMX on APZ magazine.", LOG_LEVEL_ERROR);
					continue;
				} else {
					m_switchAPZVector.push_back(scx);
					m_apzScxHandler[ind] = new FIXS_TREFC_ApzScxHandler(ipA, ipB);
					if(m_apzScxHandler[ind] != NULL)
						m_apzScxHandler[ind]->initialize();

					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting new APZ SCX/SMX Thread, slot %d.", uslot);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
				}

				//Update JTP Handler
				if (m_jtpHandlerTref)
					m_jtpHandlerTref->updateApzScx(uslot,true);

				if(servStatus == IMM_Util::ENABLED)
				{
					//set reconf in APZ thread
					if(servType == IMM_Util::NTP)
					{
						if(m_apzScxHandler[ind] != NULL) {
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting NTP Service on new added SCX/SMX %d.", m_switchAPZVector[ind].slotPos);
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

							m_apzScxHandler[ind]->setNtpConf(m_tRefList, "", "", "");
						}
					}
					else if (servType == IMM_Util::NTPHP)
					{
						if(m_apzScxHandler[ind] != NULL) {
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting NTPHP Service on new added SCX/SMX %d.", m_switchAPZVector[ind].slotPos);
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

							m_apzScxHandler[ind]->setNtpHpConf(m_tRefList, "", "", "", m_switchAPZVector[0].IP_EthA, m_switchAPZVector[0].IP_EthB);
						}
					}
					//reconfigure APT SCX boards
					for(unsigned int i = 0; i < m_switchAPTVector.size(); i++)
					{
						if(m_aptScxHandler[i] != NULL)
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Updating NTP Client Configuration on APT SCX/SMX %d, IP: %s.",
									i+1, m_switchAPTVector[i].IP_EthA.c_str());
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

							m_aptScxHandler[i]->setNtpClientConf(m_switchAPZVector, m_switchAPTVector[i].ethInPort);
						}
					}
				}
				else //DISABLED
				{
					//set cleanup for the new thread
					m_apzScxHandler[ind]->setCleanup();
				}

 			}
			else //APT SCX
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] New SCX/SMX added on slot %d on APT Magazine: %s.", uslot, scxMag.c_str());
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);


				//get port for EthIn
				int aptEthInPort = 31;
//				string tempDn = shelfAPZ_dn.substr(shelfAPZ_dn.find_first_of(',')+1, string::npos);
//				//string shelfName = scxMag.substr(scxMag.begin(), scxMag.find_first_of('.')) + "_";
//				string shelfName = scxMag;
//				replace( shelfName.begin(), shelfName.end(), '.', '_' );
//				cout << "shelf name : " << shelfName << endl;
//				tempDn = "otherBladeId=" + TREFC_Util::intToString(uslot) + ",shelfId=" + shelfName + "," + tempDn;
//				cout << "temp dn : " << tempDn << endl;
//				IMM_Util::getImmAttributeInt(tempDn, IMM_Util::ATT_OTHERBLADE_APTETHPORT, aptEthInPort);

				ImmInterface::getAptEthInPort(scxMag, uslot,aptEthInPort);

				cout << "ETH PORT : " << aptEthInPort << endl;
				int ind = m_switchAPTVector.size();
				SWITCHAPTIPAddress aptScx;
				aptScx.IP_EthA = ipA;
				aptScx.IP_EthB = ipB;
				aptScx.ethInPort = aptEthInPort;
				aptScx.index = ind;
				m_switchAPTVector.push_back(aptScx);

				m_aptScxHandler[ind] = new FIXS_TREFC_AptScxHandler(ipA, ipB, aptEthInPort);
				if(m_aptScxHandler[ind] != NULL)
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Starting new APT SCX/SMX Thread.");
						FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					m_aptScxHandler[ind]->initialize();
					if(servStatus == IMM_Util::ENABLED)
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Setting NTP Client Configuration on APT SCX/SMX %d, IP: %s.",
								ind+1, m_switchAPTVector[ind].IP_EthA.c_str());
						FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

						m_aptScxHandler[ind]->setNtpClientConf(m_switchAPZVector, m_switchAPTVector[ind].ethInPort);
					}
					else if (servStatus == IMM_Util::DISABLED)
					{
						m_aptScxHandler[ind]->setCleanup();
					}
				}


			}

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] SCX thread list updated.");
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			break;

		case ACS_CS_API_TableChangeOperation::Delete:
			std::cout << "-------- Delete --------\n" << std::endl;
			ipA = TREFC_Util::ulongToStringIP(observer.hwcData[index].ipEthA);
			if(isApzScx)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] SCX/SMX removed on slot %d on APZ Magazine, stopping thread.", uslot);
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
				}

				for(unsigned int i = 0; i < m_switchAPZVector.size(); i++)
				{
					if(0 == ipA.compare(m_switchAPZVector[i].IP_EthA))
					{
						m_switchAPZVector.erase(m_switchAPZVector.begin()+i);

						if (m_apzScxHandler[i] != NULL) {
							m_apzScxHandler[i]->close();

							for(int j = 0; ((j < 6) && m_apzScxHandler[i]->isRunning()); j++) {
								cout << "THREAD IS STILL RUNNING " << m_apzScxHandler[i]->m_ipA << endl;
								sleep(1);
							}

							if (!m_apzScxHandler[i]->isRunning()) {
								cout << "THREAD IS FINISHED " << m_apzScxHandler[i]->m_ipA << endl;
								delete m_apzScxHandler[i];
							} else {
								cout << "UNABLE TO CLOSE THREAD " << m_apzScxHandler[i]->m_ipA << endl;
							}
							m_apzScxHandler[i] = NULL;

						}

						alignApzArray();
					}
				}

				//Update JTP Handler
				if (m_jtpHandlerTref)
					m_jtpHandlerTref->updateApzScx(uslot,false);

				if(servStatus == IMM_Util::ENABLED)
				{
					//reconfigure APT SCX boards
					for(unsigned int i = 0; i < m_switchAPTVector.size(); i++)
					{
						if(m_aptScxHandler[i] != NULL)
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Updating NTP Client Configuration on APT SCX/SMX %d, IP: %s.",
									i+1, m_switchAPTVector[i].IP_EthA.c_str());
							FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

							m_aptScxHandler[i]->setNtpClientConf(m_switchAPZVector, m_switchAPTVector[i].ethInPort);
						}
					}
				}
				for (unsigned int j = 0; j < m_switchAPZVector.size(); j++)
				{
					if (m_apzScxHandler[j]) {
						cout << "-------------------------------------" << endl;
						cout << "ip A : " << m_switchAPZVector[j].IP_EthA <<  m_apzScxHandler[j]->m_ipA << endl;
						cout << "ip B : " << m_switchAPZVector[j].IP_EthB << m_apzScxHandler[j]->m_ipB << endl;
						cout << "-------------------------------------\n" << endl;
					} else {
						cout << "ERROR: MISALLIGNMENT" << endl;
					}
				}
			}
			else
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] SCX/SMX removed on slot %d on APT Magazine %s, stopping thread.", uslot, scxMag.c_str());
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
				}


				for(unsigned int i = 0; i < m_switchAPTVector.size(); i++)
				{
					if(0 == ipA.compare(m_switchAPTVector[i].IP_EthA))
					{
						m_switchAPTVector.erase(m_switchAPTVector.begin()+i);

						if (m_aptScxHandler[i] != NULL) {
							cout << "CLOSING THREAD " << m_aptScxHandler[i]->m_ipA << endl;
							m_aptScxHandler[i]->close();
							for(int j = 0; ((j < 6) && m_aptScxHandler[i]->isRunning()); j++) {
								cout << "THREAD IS STILL RUNNING " << m_aptScxHandler[i]->m_ipA << endl;
								sleep(1);
							}

							if (!m_aptScxHandler[i]->isRunning()) {
								cout << "THREAD IS FINISHED " << m_aptScxHandler[i]->m_ipA << endl;
								delete m_aptScxHandler[i];
							} else {
								cout << "UNABLE TO CLOSE THREAD " << m_aptScxHandler[i]->m_ipA << endl;
							}

							m_aptScxHandler[i] = NULL;
						}

						alignAptArray();
					}
				}
				if (m_switchAPTVector.size() == 0)
					cout << "APT LIST EMPTY" << endl;

				for (unsigned int j = 0; j < m_switchAPTVector.size(); j++)
				{
					if (m_aptScxHandler[j]) {
						cout << "-------------------------------------" << endl;
						cout << "ip A : " << m_switchAPTVector[j].IP_EthA << " " << m_aptScxHandler[j]->m_ipA << endl;
						cout << "ip B : " << m_switchAPTVector[j].IP_EthB << " " << m_aptScxHandler[j]->m_ipB << endl;
						cout << "-------------------------------------\n" << endl;
					} else {
						cout << "ERROR: MISALLIGNMENT" << endl;
					}
				}
			}

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] SCX/SMX thread list updated.");
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			break;
		case ACS_CS_API_TableChangeOperation::Change:

                        std::cout << "DBG: ------- Change -------- \n" << std::endl;
//			ipA = TREFC_Util::ulongToStringIP(observer.hwcData[index].ipEthA);
                        if(!isApzScx)
                        {
				std::cout << "DBG: ------- apt blade object change -------- \n" << std::endl;
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] attribute changed on slot %d on APT Magazine %s.", uslot, scxMag.c_str());
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
				}

				int aptEthInPort = 31;
				ImmInterface::getAptEthInPort(scxMag, uslot,aptEthInPort);

				for(unsigned int i = 0; i < m_switchAPTVector.size(); i++)
				{
					if(0 == ipA.compare(m_switchAPTVector[i].IP_EthA))
					{
						m_switchAPTVector[i].ethInPort = aptEthInPort;
						cout << "\nPort value =  " << m_switchAPTVector[i].ethInPort << endl;
						if (m_aptScxHandler[i] ) {
							cout << "-------------------------------------" << endl;
							cout << "ip A : " << m_switchAPTVector[i].IP_EthA << " " << m_aptScxHandler[i]->m_ipA << endl;
							cout << "ip B : " << m_switchAPTVector[i].IP_EthB << " " << m_aptScxHandler[i]->m_ipB << endl;
							cout << "ethInPort : " << m_switchAPTVector[i].ethInPort << endl;
							m_aptScxHandler[i]->setPort(m_switchAPTVector[i].ethInPort);
							cout << "-------------------------------------\n" << endl;
						}
						else {
							cout << "ERROR: APT handler MISALLIGNMENT" << endl;
						}
						break;
					}
				}
			}
			break;

		default:
			//			if (ACS_TRA_ON(traceObj))
			//			{
			//				char tmpStr[512] = {0};
			//				_snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] [line:%d] HWCTable update has been discarded. Operation type: %d", __LINE__, observer.hwcData[0].operationType);
			//				ACS_TRA_event(&traceObj, tmpStr);
			//			}
			std::cout << "unknown!!" << "\n";
		}
	}



}




int FIXS_TREFC_Manager::addScxExtIp (std::string extIp, std::string netMask, std::string defGateway, unsigned short slot)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": IN" << std::endl;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	bool ntpRest= false;
	if(slot == 0)
	{
		extIpSwitch0.extIp = extIp;
		extIpSwitch0.defaultGateway = defGateway;
		extIpSwitch0.netmask = netMask;
	}
	else if (slot == 25)
	{
		extIpSwitch25.extIp = extIp;
		extIpSwitch25.defaultGateway = defGateway;
		extIpSwitch25.netmask = netMask;
	}

	if (servStatus == IMM_Util::ENABLED)
	{
		if(servType == IMM_Util::NTP)
		{
			ntpRest= true;
		}
		/***
		 * if servType is NTP, call addExpIp with last parameter true
		 * else call addExtIp with last parameter false
		 */
		if (m_apzScxHandler[0]!= NULL)
		{
			if(m_switchAPZVector[0].slotPos == slot)
			{
				m_apzScxHandler[0]->addExtIp(extIp,defGateway,netMask,ntpRest);
			}

		}

		if (m_apzScxHandler[1]!= NULL)
		{
			if(m_switchAPZVector[1].slotPos == slot)
			{
				m_apzScxHandler[1]->addExtIp(extIp,defGateway,netMask,ntpRest);
			}

		}
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": Success. OUT" << std::endl;
	return EXECUTED;
}

int FIXS_TREFC_Manager::rmScxExtIp (unsigned short slot)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": IN" << std::endl;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);


	if (m_apzScxHandler[0]!= NULL)
	{
		if(m_switchAPZVector[0].slotPos == slot)
		{
			m_apzScxHandler[0]->rmExtIp();
		}
	}
	if (m_apzScxHandler[1]!= NULL)
	{
		if(m_switchAPZVector[1].slotPos == slot)
		{
			m_apzScxHandler[1]->rmExtIp();
		}
	}

	//Remove entry from table
	if (slot == 0) {
		extIpSwitch0.extIp.clear();
		extIpSwitch0.defaultGateway.clear();
		extIpSwitch0.netmask.clear();
	}
	if (slot == 25) {
		extIpSwitch25.extIp.clear();
		extIpSwitch25.defaultGateway.clear();
		extIpSwitch25.netmask.clear();
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": Success. OUT"<<std::endl;
	return EXECUTED;

}


bool FIXS_TREFC_Manager::GetSCXAddressForAPZSlot(string &ipA, string &ipB,unsigned short slot)
{
	bool res = false;

	for (unsigned i=0; i<m_switchAPZVector.size(); i++)
	{
		if (m_switchAPZVector[i].slotPos == slot)
		{
			ipA = m_switchAPZVector[i].IP_EthA;
			ipB = m_switchAPZVector[i].IP_EthB;
			res = true;
		}
	}

	return res;
}


void FIXS_TREFC_Manager::setExtIpAtTrefcStart (bool isSMX)
{
	string extIp("");
	string netmask("");
	string gateway("");
	string slotStr("");

	//Initialize Map
	extIpSwitch0.slot = 0;
	extIpSwitch0.extIp = extIp;
	extIpSwitch0.defaultGateway = gateway;
	extIpSwitch0.netmask = netmask;

	extIpSwitch25.slot = 25;
	extIpSwitch25.extIp = extIp;
	extIpSwitch25.defaultGateway = gateway;
	extIpSwitch25.netmask = netmask;

	ImmInterface::getExternalIpAddressConfiguration(extIpSwitch0, extIpSwitch25, isSMX);

}

int FIXS_TREFC_Manager::GetSWITCHBoardAddress(vector<SWITCHAPTIPAddress> &switchAPTVector,vector<SWITCHAPZIPAddress> &switchAPZVector,bool isSMX)
{

	int res = 0;
	switchAPTVector.clear();
	switchAPZVector.clear();
	res=ImmInterface::getSWITCHBoardAddress(switchAPTVector, switchAPZVector, isSMX);

	char tmpStr[512] = {0};


	if (res == INTERNALERR) {
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] [%s@%d] Board List is empty", __FUNCTION__, __LINE__);
		printTRA(tmpStr);
	} else if (res == NO_REPLY_FROM_CS) {
		//serious fault in CS: No magazine found for the SCXB board
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] [line:%d] Serious fault in CS...", __LINE__);
		printTRA(tmpStr);
	}


	std::cout << "------------------------------------------------------------"<< std::endl;
	std::cout << "                 APZ SWITCH BOARD IP ADDRESS                  " << std::endl;
	std::cout << "------------------------------------------------------------"<< std::endl;
	for (unsigned i=0; i<switchAPZVector.size(); i++)
	{
		cout << " IP_EthA: " << switchAPZVector[i].IP_EthA.c_str() << " IP_EthB: " << switchAPZVector[i].IP_EthB << " slotPos: " << switchAPZVector[i].slotPos << endl;
	}

	std::cout << "\n------------------------------------------------------------"<< std::endl;
	std::cout << "                 APT SWITCH BOARD IP ADDRESS                  " << std::endl;
	std::cout << "------------------------------------------------------------"<< std::endl;

	for (unsigned i=0; i<switchAPTVector.size(); i++)
	{
		cout << " IP_EthA: " << switchAPTVector[i].IP_EthA.c_str() << " IP_EthB: " << switchAPTVector[i].IP_EthB << " slotPos: " << switchAPTVector[i].index << endl;
	}



	return res;

}


void FIXS_TREFC_Manager::coldStart (string snmpIP)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ << "*************cold/warm start Trap received from ip: " << snmpIP <<std::endl;
	std::cout << "length of received ip : " << snmpIP.length() << std::endl;
	bool isFromApz = false;
	bool isFromApt = false;
	int apzIndex = -1;
	int aptIndex = -1;
	//check if it from scx in any of our vectors
	int size = (int)m_switchAPZVector.size();
	if(size > 0)
	{
		for(int i=0 ; i <size;i++)
		{
			cout<< "checking ips : "<<m_switchAPZVector[i].IP_EthA << " & "<<m_switchAPZVector[i].IP_EthB<<endl;
			std::cout << "length of checked ip : " << (m_switchAPZVector[i].IP_EthA).length() << std::endl;

			if((snmpIP.compare(m_switchAPZVector[i].IP_EthA) == 0) || (snmpIP.compare(m_switchAPZVector[i].IP_EthB) == 0))
			{
				cout << "Trap is from SCX in APZ magazine" << endl;
				isFromApz = true;
				apzIndex = i;
				break;
			}
		} //for
	}
	if(!isFromApz)
	{
		int aptSize = (int)m_switchAPTVector.size();
		if(aptSize > 0)
		{
			for(int i=0 ; i <aptSize;i++)
			{
				if((snmpIP.compare(m_switchAPTVector[i].IP_EthA) == 0) || (snmpIP.compare(m_switchAPTVector[i].IP_EthB) == 0))
				{
					cout << "Trap is from SCX in APT magazine" << endl;
					isFromApt = true;
					aptIndex = i;
					break;
				}
			}
		}
	} //apt
	if(isFromApz)
	{
		//check which thread in apz deals with this scx ip and call the snmp setting function to re-configure ntp/ntphp
		if(apzIndex >= 0)
		{
			if(servStatus == IMM_Util::ENABLED)
			{
				if(servType == IMM_Util::NTP)
				{
					if(m_apzScxHandler[apzIndex])
					{
						if(m_switchAPZVector[apzIndex].slotPos == 0)
						{
							m_apzScxHandler[apzIndex]->setNtpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask);
						}
						else if(m_switchAPZVector[apzIndex].slotPos == 25)
						{
							m_apzScxHandler[apzIndex]->setNtpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask);
						}
					}
				} //ntp
				else if(servType == IMM_Util::NTPHP)
				{
					string otherSCXIpA(""), otherSCXIpB("");
					if(apzIndex == 0)
					{
						otherSCXIpA = m_switchAPZVector[1].IP_EthA;
						otherSCXIpB = m_switchAPZVector[1].IP_EthB;
					}
					else if(apzIndex == 1)
					{
						otherSCXIpA = m_switchAPZVector[0].IP_EthA;
						otherSCXIpB = m_switchAPZVector[0].IP_EthB;
					}

					if(m_apzScxHandler[apzIndex])
					{
						if(m_switchAPZVector[apzIndex].slotPos == 0)
						{
							m_apzScxHandler[apzIndex]->setNtpHpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, otherSCXIpA, otherSCXIpB);
						}
						else if(m_switchAPZVector[apzIndex].slotPos == 25)
						{
							m_apzScxHandler[apzIndex]->setNtpHpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, otherSCXIpA, otherSCXIpB);
						}
					}
				} //ntphp
			}//enabled
			else
			{
				cout << "disabled" << endl;
				if(m_apzScxHandler[apzIndex])
				{
					if(m_switchAPZVector[apzIndex].slotPos == 0)
					{
						if(!(extIpSwitch0.extIp).empty())
							m_apzScxHandler[apzIndex]->setAddExtIpRetry(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
					}
					else if (m_switchAPZVector[apzIndex].slotPos == 25)
					{
						if(!(extIpSwitch25.extIp).empty())
							m_apzScxHandler[apzIndex]->setAddExtIpRetry(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
					}
					m_apzScxHandler[apzIndex]->setTime();
					m_apzScxHandler[apzIndex]->setCleanup();

				}
				m_timeChecker->startTimeSet();

			} //disabled
		}//apzIndex is within range
	}//APZ


	else if(isFromApt)
	{
		//check which thread in apt deals with this scx ip and call the snmp setting function to re-configure sntp client setting
		if(aptIndex >= 0)
		{
			if(servStatus == IMM_Util::ENABLED)
			{
				//get port number to use for stsiIslEthIn
//				int port = 31; //Default
				if(m_aptScxHandler[aptIndex])
				{
					int port = m_switchAPTVector[aptIndex].ethInPort;
					m_aptScxHandler[aptIndex]->setNtpClientConf(m_switchAPZVector, port);
					std::cout << "m_scxAPTVector[aptIndex].ethInPort port :" << port << std::endl;
				}
			}//enabled
			else
			{
				if(m_aptScxHandler[aptIndex])
				{
					m_aptScxHandler[aptIndex]->setTime();
					m_aptScxHandler[aptIndex]->setCleanup();
				}
			} //disabled
		}//aptIndex is within range
	}
	else
	{
		std::cout << "Trap is not from SCX in our database... IGNORE" << std::endl;
	}

}

void FIXS_TREFC_Manager::getTrefcData (int &status, int &type, std::map<std::string,std::string> &trefList)
{
	//## begin FIXS_TREFC_Manager::getTrefcData%4E53876E02D6.body preserve=yes
	//*********** set the service status *****************
	if(servStatus == IMM_Util::ENABLED)
		status = 0;
	else
		status = 1;

	//*********** set the service type *******************
	if( servType == IMM_Util::NTP )
		type = 0;
	else if( servType == IMM_Util::NTPHP )
	{
		if (refType == IMM_Util::GPS_REFERENCE) //GPS device
			type = 2;
		else
			type = 1; //NTPHP with time ref
	}
	else
		type = 3;

	//*********** set the time-reference list *******************
	trefList = m_tRefList;
	//## end FIXS_TREFC_Manager::getTrefcData%4E53876E02D6.body
}


int FIXS_TREFC_Manager::trefcRestart ()
{

	if (servStatus == IMM_Util::DISABLED)
	{
		FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Restart action rejected: TIMESERVICE_ALREADY_STOPPED", LOG_LEVEL_WARN);
		printTRA("Restart action rejected: TIMESERVICE_ALREADY_STOPPED");
		return TIMESERVICE_ALREADY_STOPPED;
	}
	else if (servStatus == IMM_Util::ENABLED)
	{

		string otherIpA[2] = {"",""};
		string otherIpB[2] = {"",""};

		int apzVecSize = 0;
		int aptVecSize = 0;

		apzVecSize = (int)m_switchAPZVector.size();
		aptVecSize = (int)m_switchAPTVector.size();
		//
		ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

		if(m_tRefList.empty()) {
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Restart action rejected: NO_REFERENCE_EXIST", LOG_LEVEL_ERROR);
			printTRA("Restart action rejected: NO_REFERENCE_EXIST");
			return NO_REFERENCE_EXIST;
		}

		int res1 = 0, res2 = 0, res1_extIp = 0, res2_extIp = 0;


		//send JTP query to CP for starting time service. If yes, continue. If no, return with fault code.
		//	// send JTP notification CANSTART
		bool CanStartQuery =false;
		if(servType == IMM_Util::NTP)
		{
			if(m_jtpHandlerTref)
				m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_CAN_START,1);
		}
		else
		{
			if (refType == IMM_Util::GPS_REFERENCE) //GPS device
			{
				if(m_jtpHandlerTref)
					m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_CAN_START,3);
			}
			else
			{
				if(m_jtpHandlerTref)
					m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_CAN_START,2);
			}
		}

		if(m_jtpHandlerTref)
			m_jtpHandlerTref->waitCanStartQueryToFinish();
		if(m_jtpHandlerTref)
			CanStartQuery = m_jtpHandlerTref->checkCanSendStartR();
		else
			CanStartQuery = true;

		if (CanStartQuery != true)
		{
			//if(!force)
			//servType = IMM_Util::NONE;
			printTRA("Restart action failed: NOT_ALLOWED_BY_CP");
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Restart action failed: NOT_ALLOWED_BY_CP",LOG_LEVEL_ERROR);
			return TREFC_Util::NOT_ALLOWED_BY_CP;
		}

		//Stop Time Checker
		m_timeChecker->stopTimeSet();


		if(servType == IMM_Util::NTP){
			if (isGPSList())
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << "Command rejected as GPS is not allowed for NTP" << std::endl;
				printTRA("Restart action rejected as GPS is not allowed for NTP");
				FIXS_TREFC_Logger::getLogInstance()->Write("Restart action rejected as GPS is not allowed for NTP", LOG_LEVEL_ERROR);
				return TREFC_Util::INVALID_REF;
			}
			//servType = IMM_Util::NTP;
			if(m_apzScxHandler[0] != NULL)
			{
				if(m_switchAPZVector[0].slotPos == 0)
				{
					if(!extIpSwitch0.extIp.empty())
					{
						res1_extIp = m_apzScxHandler[0]->addExtIp(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
					}
				}
				else if(m_switchAPZVector[0].slotPos == 25)
				{
					if(!extIpSwitch25.extIp.empty())
					{
						res1_extIp = m_apzScxHandler[0]->addExtIp(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
					}
				}
                                std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntp SERVICE " << std::endl; //mychange
				res1 = m_apzScxHandler[0]->startNtp(m_tRefList);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " starting ntp service " << std::endl; //mychange
			}
			if(m_apzScxHandler[1] != NULL)
			{
				if(m_switchAPZVector[1].slotPos == 0)
				{
					if(!extIpSwitch0.extIp.empty())
					{
						res2_extIp = m_apzScxHandler[1]->addExtIp(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
					}
				}
				else if(m_switchAPZVector[1].slotPos == 25)
				{
					if(!extIpSwitch25.extIp.empty())
					{
						res2_extIp = m_apzScxHandler[1]->addExtIp(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
					}
				}
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntp service " << std::endl; //mychange
				res2 = m_apzScxHandler[1]->startNtp(m_tRefList);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " starting ntp service" << std::endl; //mychange
			}


			if(res1!=0 && res2!=0) {
				printTRA("Restart action failed: INTERNALERR");
				FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Restart action failed: INTERNALERR", LOG_LEVEL_ERROR);
				return INTERNALERR;
			}

			if(res1!=0)
			{
				if(m_switchAPZVector[0].slotPos == 0)
				{
					if(m_apzScxHandler[0] != NULL) m_apzScxHandler[0]->setNtpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask);
				}
				else if(m_switchAPZVector[0].slotPos == 25)
				{
					if(m_apzScxHandler[0] != NULL) m_apzScxHandler[0]->setNtpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask);
				}

			}
			if(res2!=0)
			{
				if(m_switchAPZVector[1].slotPos == 0)
				{
					if(m_apzScxHandler[1] != NULL) m_apzScxHandler[1]->setNtpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask);
				}
				else if(m_switchAPZVector[1].slotPos == 25)
				{
					if(m_apzScxHandler[1] != NULL) m_apzScxHandler[1]->setNtpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask);
				}
			}


		}
		else if(servType == IMM_Util::NTPHP)
		{
			servType = IMM_Util::NTPHP;
			if(apzVecSize == 2)
			{
				otherIpA[0]= m_switchAPZVector[1].IP_EthA;
				otherIpB[0]= m_switchAPZVector[1].IP_EthB;
				otherIpA[1]= m_switchAPZVector[0].IP_EthA;
				otherIpB[1]= m_switchAPZVector[0].IP_EthB;
			}


			if(isGPSList())
			{
				if(m_apzScxHandler[0] != NULL)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntphp service  " << std::endl; //mychange
					res1 = m_apzScxHandler[0]->startNtphpGps(otherIpA[0], otherIpB[0]);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " starting ntphp service  " << std::endl; //mychange
				}

				if(m_apzScxHandler[1] != NULL)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntphp service  " << std::endl; //mychange
					res2 = m_apzScxHandler[1]->startNtphpGps(otherIpA[1], otherIpB[1]);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " starting ntphp service " << std::endl; //mychange
				}
			}
			else
			{
				if(m_apzScxHandler[0] != NULL)
				{
					if(m_switchAPZVector[0].slotPos == 0)
					{
						if(!extIpSwitch0.extIp.empty())
						{
							res1_extIp = m_apzScxHandler[0]->addExtIp(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
						}
					}
					else if(m_switchAPZVector[0].slotPos == 25)
					{
						if(!extIpSwitch25.extIp.empty())
						{
							res1_extIp = m_apzScxHandler[0]->addExtIp(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
						}

					}
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntphp service  " << std::endl; //mychange
					res1 = m_apzScxHandler[0]->startNtphp(m_tRefList, otherIpA[0], otherIpB[0]);
					std::cout << __FUNCTION__ << "@" << __LINE__ << "  starting ntphp service  " << std::endl; //mychange
				}

				if(m_apzScxHandler[1] != NULL)
				{
					if(m_switchAPZVector[1].slotPos == 0)
					{
						if(!extIpSwitch0.extIp.empty())
						{
							res2_extIp = m_apzScxHandler[1]->addExtIp(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, false);
						}
					}
					else if(m_switchAPZVector[1].slotPos == 25)
					{
						if(!extIpSwitch25.extIp.empty())
						{
							res2_extIp = m_apzScxHandler[1]->addExtIp(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, false);
						}

					}
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Before starting ntphp service " << std::endl; //mychange
					res2 = m_apzScxHandler[1]->startNtphp(m_tRefList, otherIpA[1], otherIpB[1]);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " starting ntphp service : " << std::endl; //mychange
				}
			}

			if(res1!=0 && res2!=0) {
				FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Restart action failed: INTERNALERR", LOG_LEVEL_ERROR);
				printTRA("Restart action failed: INTERNALERR");
				return INTERNALERR;
			}

			if(res1!=0)
			{
				if(m_switchAPZVector[0].slotPos == 0)
				{
					if(m_apzScxHandler[0] != NULL) m_apzScxHandler[0]->setNtpHpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, otherIpA[0], otherIpB[0]);
				}
				else if(m_switchAPZVector[0].slotPos == 25)
				{
					if(m_apzScxHandler[0] != NULL) m_apzScxHandler[0]->setNtpHpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, otherIpA[0], otherIpB[0]);
				}
			}
			if(res2!=0)
			{
				if(m_switchAPZVector[1].slotPos == 0)
				{
					if(m_apzScxHandler[1] != NULL) m_apzScxHandler[1]->setNtpHpConf(m_tRefList, extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, otherIpA[1], otherIpB[1]);
				}
				else if(m_switchAPZVector[1].slotPos == 25)
				{
					if(m_apzScxHandler[1] != NULL) m_apzScxHandler[1]->setNtpHpConf(m_tRefList, extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, otherIpA[1], otherIpB[1]);
				}
			}

		}
		else
		{
			servType = IMM_Util::NONE;
			FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Restart action failed: INCORRECT_SERVICE", LOG_LEVEL_ERROR);
			printTRA("Restart action failed: INCORRECT_SERVICE");
			return INCORRECT_SERVICE;
		}

		//External ip address retry (if required)
		if(res1_extIp != 0)
		{
			if(m_switchAPZVector[0].slotPos == 0)
			{
				if(m_apzScxHandler[0] != NULL) m_apzScxHandler[0]->setAddExtIpRetry(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, true);
			}
			else if(m_switchAPZVector[0].slotPos == 25)
			{
				if(m_apzScxHandler[0] != NULL) m_apzScxHandler[0]->setAddExtIpRetry(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, true);
			}

		}
		if(res2_extIp != 0)
		{
			if(m_switchAPZVector[1].slotPos == 0)
			{
				if(m_apzScxHandler[1] != NULL) m_apzScxHandler[1]->setAddExtIpRetry(extIpSwitch0.extIp, extIpSwitch0.defaultGateway, extIpSwitch0.netmask, true);
			}
			else if(m_switchAPZVector[1].slotPos == 25)
			{
				if(m_apzScxHandler[1] != NULL) m_apzScxHandler[1]->setAddExtIpRetry(extIpSwitch25.extIp, extIpSwitch25.defaultGateway, extIpSwitch25.netmask, true);
			}

		}
		//APT
		aptVecSize = (int)m_switchAPTVector.size();
		for (int j=0;j<aptVecSize;j++)
		{
			if(m_aptScxHandler[j] != NULL)
			{
				m_aptScxHandler[j]->setNtpClientConf(m_switchAPZVector,m_switchAPTVector[j].ethInPort);
			}
		}

		servStatus= IMM_Util::ENABLED;

		//Modify serviceStatus attribute
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"before setServiceStatus" <<std::endl; //mychange
		ImmInterface::setServiceStatus(servStatus);
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"after setServiceStatus" <<std::endl; //mychange
//		int intStatus = servStatus;
//		ACS_CC_ReturnType setResult;
//		ACS_CC_ImmParameter serverStatus = IMM_Util::defineParameterInt(IMM_Util::ATT_TIMEREFERENCE_STATUS,ATTR_INT32T,&intStatus,1);
//		setResult = FIXS_TREFC_OI_TimeService::getInstance()->modifyRuntimeObj(IMM_Util::ATT_TIMEREFERENCE_OBJ,&serverStatus);
//		if (setResult == ACS_CC_FAILURE) cout << " FIXS_TREFC_TimeService modify " << IMM_Util::ATT_TIMEREFERENCE_STATUS<< " FAILED !!!"<< endl;
//		delete [] serverStatus.attrValues;

		//Modify serviceType attribute
//		ImmInterface::setServiceType(servType);


//		int intType = servType;
//		ACS_CC_ImmParameter serviceType = IMM_Util::defineParameterInt(IMM_Util::ATT_TIMEREFERENCE_TYPE,ATTR_INT32T,&intType,1);
//		setResult = FIXS_TREFC_OI_TimeService::getInstance()->modifyRuntimeObj(IMM_Util::ATT_TIMEREFERENCE_OBJ,&serviceType);
//		if (setResult == ACS_CC_FAILURE) cout << " FIXS_TREFC_TimeService modify " << IMM_Util::ATT_TIMEREFERENCE_TYPE<< " FAILED !!!"<< endl;
//		delete [] serviceType.attrValues;
	}

	//Send JTP notification
//	if(servType == IMM_Util::NTP)
//			m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_START,1);
//		else
//			m_jtpHandlerTref->setJtpNotificationEvent(FIXS_TREFC_JTPHandler::TIME_SERVICE_START,2);

	return 0;
}

void FIXS_TREFC_Manager::printTRA(std::string mesg)
{
	if (_trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, mesg.c_str());
		_trace->ACS_TRA_event(1, tmpStr);

	}
}

int FIXS_TREFC_Manager::validateScxExtIpAction(IMM_Util::Action action) {

	int res = EXECUTED;

	if (action == IMM_Util::DELETE) {
		if(servStatus == IMM_Util::ENABLED)
		{
			std::cout<<"DBG: Not allowed to remove entry when Time Service is running"<<std::endl;
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] ERROR: Not allowed to remove entry when Time Service is running");
				_trace->ACS_TRA_event(1,tmpStr);
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);

			}

			res = EXT_IP_REMOVE_NOT_ALLOWED;
		}
	}
	return res;
}


int FIXS_TREFC_Manager::validateTimeServerTransaction(std::vector<TimeServerAction> transaction) {
        std::cout <<"FIXS_TREFC_Manager::validateTimeServerTransaction"<<std::endl;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);
       
	map<string,string> temp_tRefList;
	int res = 0;
	temp_tRefList = m_tRefList;

	for (unsigned int i=0; i < transaction.size(); i++) {
		if (transaction[i].action == IMM_Util::CREATE) {
			res=addTRefToTempList(transaction[i].element.ipAddress, transaction[i].element.name, temp_tRefList);
			if (res != 0)
				return res;
		}
		else if (transaction[i].action == IMM_Util::DELETE) {
			res = rmTRefFromTempList(transaction[i].element.ipAddress, transaction[i].element.name, temp_tRefList);
			if (res != 0)
				return res;
		}
		else
			return 1; //Invalid Action
	}

	return res;

}

int FIXS_TREFC_Manager::addTRefToTempList (std::string trefIP, std::string trefName, map<string,string> &temp_tRefList)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	std::cout << __FUNCTION__ << "@" << __LINE__ <<
			" trefIP: " << trefIP.c_str() <<
			" - trefName: " << trefName.c_str() << std::endl;

	//Check if Time Ref IP already exists
	std::map<std::string, std::string>::iterator it;
	it = temp_tRefList.find(trefIP.c_str());
	if (it != temp_tRefList.end())
	{
		//IP already present
		std::cout << __FUNCTION__ << "@" << __LINE__ << " IP already present" << std::endl;
		printTRA("Time Ref add rejected: IP already present!");
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Time Server Rejected! Time Server name: %s, IP address = %s. IP already present!",
					trefName.c_str(),
					trefIP.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
		}
		return TREFC_Util::TIME_REF_IP_ALREADY_EXIST;
	}

	//Check if Time Ref Name already exists
	if (isUniqueName(trefName, temp_tRefList))
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " NAME already present" << std::endl;
		printTRA("Time Ref add rejected: NAME already present!");
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Time Server Rejected! Time Server name: %s, IP address = %s. Name already present!",
					trefName.c_str(),
					trefIP.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
		}
		return TREFC_Util::TIME_REF_NAME_ALREADY_EXIST;
	}
	else
	{
		//IP and NAME not present, add to list
		if (m_tRefList.size() < TREFC_Util::MAXTREFNUMBER)
		{
			//Update SNMP changes to SCX
			if(servStatus == IMM_Util::ENABLED)
			{
				//Insert into temporary Time Ref list
				temp_tRefList.insert ( pair<string,string>(trefIP, trefName) );

			}
			else {//serviceStatus = DISABLED
				//Insert into list without other settings
				temp_tRefList.insert ( pair<string,string>(trefIP, trefName) );
			}
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " To many references" << std::endl;
			printTRA("Time Ref add rejected: Too many references!");
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Add Time Server Rejected! Time Server name: %s, IP address = %s. Too Many References!",
						trefName.c_str(),
						trefIP.c_str());
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
			}
			return TREFC_Util::TOO_MANY_REF;
		}
	}

	return EXECUTED;
}

int FIXS_TREFC_Manager::rmTRefFromTempList (std::string trefIP, std::string trefName, map<string,string> &temp_tRefList)
{

	//	Lock lock;
	std::cout << __FUNCTION__ << "@" << __LINE__ <<
			" trefIP: " << trefIP.c_str() <<
			" - trefName: " << trefName.c_str() << std::endl;

	int res = TREFC_Util::EXECUTED;
//	int res1 = 0;
//	int res2 = 0;
	std::map<std::string, std::string>::iterator it;

	//Check if the service is enabled and this is the last time reference left
	if(temp_tRefList.size()==1 && servStatus == IMM_Util::ENABLED && refType == IMM_Util::NETWORK_REFERENCE) {
		FIXS_TREFC_Logger::getLogInstance()->Write("Time Server remove rejected: NOT ALLOWED, Service is enabled and this is the last server remaining", LOG_LEVEL_ERROR);
		printTRA("Time Ref remove rejected: NOT_ALLOWED");
		return NOT_ALLOWED;
	}
	if (trefIP.size() > 0)
	{
		it = temp_tRefList.find(trefIP.c_str());

		if (it != temp_tRefList.end())
		{
			//IP FOUND !!
			temp_tRefList.erase(it);
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " IP not found !" << std::endl;
			printTRA("Time Ref remove failed: IP not found!");
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s. IP not found in the list.",
						trefName.c_str(),
						trefIP.c_str());
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
			}
			return TREFC_Util::TIME_REF_NOT_EXIST;
		}
	} else
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " IP not found, list is empty!" << std::endl;
		printTRA("Time Ref remove failed: IP not found, list is empty!");
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Remove Failed! Time Server name: %s, IP address = %s. IP not found, list is empty.",
					trefName.c_str(),
					trefIP.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
		}
		return TREFC_Util::TIME_REF_NOT_EXIST;
	}

	return res;
}

void FIXS_TREFC_Manager::alignAptArray() {

	//align vector and array
	FIXS_TREFC_AptScxHandler* temp[MAX_NO_OF_APT_SCX];
	for(int j=0; j < MAX_NO_OF_APT_SCX ; j++)
	{
		temp[j] = NULL;
	}

	for (unsigned int j = 0; j < m_switchAPTVector.size(); j++) {
		std::string ip = m_switchAPTVector[j].IP_EthA;

		int k = 0;

		for (k = 0; k < MAX_NO_OF_APT_SCX; k++) {
			if (m_aptScxHandler[k]) {
				if (ip.compare(m_aptScxHandler[k]->m_ipA) == 0)
					break;
			}
		}
		if (k < MAX_NO_OF_APT_SCX)
			temp[j] = m_aptScxHandler[k];
		else
			cout << "ERROR: Cannot align vector and array" << endl;
	}

	for(int j=0; j < MAX_NO_OF_APT_SCX ; j++)
	{
		m_aptScxHandler[j] = temp[j];
//		if (m_aptScxHandler[j])
//			cout << "IP ADDRESS: " << m_aptScxHandler[j]->m_ipA << endl;
//		else
//			cout << "NULL" << endl;
	}
}

void FIXS_TREFC_Manager::alignApzArray() {

	//align vector and array
	FIXS_TREFC_ApzScxHandler* temp[2];
	for(int j=0; j < 2 ; j++)
	{
		temp[j] = NULL;
	}

	for (unsigned int j = 0; j < m_switchAPZVector.size(); j++) {
		std::string ip = m_switchAPZVector[j].IP_EthA;

		int k = 0;

		for (k = 0; k < 2; k++) {
			if (m_apzScxHandler[k]) {
				if (ip.compare(m_apzScxHandler[k]->m_ipA) == 0)
					break;
			}
		}
		if (k < 2)
			temp[j] = m_apzScxHandler[k];
		else
			cout << "ERROR: Cannot align vector and array" << endl;
	}

	for(int j=0; j < 2 ; j++)
	{
		m_apzScxHandler[j] = temp[j];
//		if (m_apzScxHandler[j])
//			cout << "IP ADDRESS: " << m_apzScxHandler[j]->m_ipA << endl;
//		else
//			cout << "NULL" << endl;
	}
}

bool FIXS_TREFC_Manager::getExtIpAddrConfiguration(ExtIpSwitch &extIp, unsigned int slot) {

	if (slot == 0)
		extIp = extIpSwitch0;
	else if (slot == 25)
		extIp = extIpSwitch25;
	else
		return false;

	return true;
}


bool FIXS_TREFC_Manager::setSeviceType(IMM_Util::TimeServiceType type, int &error)
{
	if (servStatus == IMM_Util::DISABLED){
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"in setSeviceType DISBALED" <<std::endl; //mychange
		error = TREFC_Util::EXECUTED;
		servType = type;
		return true;
	} else {
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"in setSeviceType else" <<std::endl; //mychange
		error = TREFC_Util::SERVICE_TYPE_CHANGE_NOT_ALLOWED;
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Set Service Type Rejected. Cannot change Service Type when Service Status is ENABLED!");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
	}

	return false;
}

bool FIXS_TREFC_Manager::setReferenceType(IMM_Util::ExternalReferenceType type, int &error)
{
	if (servStatus == IMM_Util::DISABLED){
		error = TREFC_Util::EXECUTED;
		refType = type;
		return true;
	} else {
		error = TREFC_Util::REFERENCE_TYPE_CHANGE_NOT_ALLOWED;
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Set Reference Type Rejected. Cannot change Reference Type when Service Status is ENABLED!");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
	}

	return false;
}

string FIXS_TREFC_Manager::getNetworkReferenceName(string ipAddress)
{
	map<string, string>::iterator it = m_tRefList.find(ipAddress);
	if (it != m_tRefList.end())
	{
		return (*it).second;
	}

	return "-";
}
void FIXS_TREFC_Manager::checkAlarmStatus (bool timeout)
{
	std::cout << "DBG: " << __FUNCTION__ << __LINE__ << "Entered checkAlarmStatus function.." << std::endl;
	if(servStatus == IMM_Util::ENABLED) // time service status enabled
	{
		std::cout << "DBG: " << __FUNCTION__ << __LINE__ << "time service enabled..cease alarm..." << std::endl;
		//stop timer if running
		if(m_alarmHandler->isAlarmTimerOngoing() == true)
			m_alarmHandler->cancelAlarmTimer();

		std::cout << " ================================== " << std::endl;
		std::cout << " Time server Enabled ... alarm OFF " << std::endl;
		std::cout << " ================================== " << std::endl;
		//cease alarm
		int problem = 35100;
		unsigned int severity = TREFC_Util::Severity_CEASING; //change the alarm severity, cause, data and text according to POD
		std::string cause = "TIME SERVICE NOT STARTED";
		std::string data = "Time Service Configuration Fault";
		std::string text;
		bool isSMX = false;
		TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();

		if(environment == TREFC_Util::SMX)
			isSMX=true;
		if(isSMX == true)
			text = "TIME SERVICE IS NOT STARTED ON SWITCH BOARDS\nBUT EXTERNAL TIME REFERENCES ARE CONFIGURED";
		else
			text = "TIME SERVICE IS NOT STARTED ON SCX BUT\nEXTERNAL TIME REFERENCES ARE CONFIGURED";

		sendAlarm (problem, severity, cause, data, text);
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SWITCH BOARDS BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
		}

	}
	else // time service status disabled
	{
		if(m_tRefList.empty() == true)
		{
			//stop timer if running
			if(m_alarmHandler->isAlarmTimerOngoing() == true)
				m_alarmHandler->cancelAlarmTimer();
			
			// cease alarm
			std::cout << " ================================== " << std::endl;
			std::cout << " No time reference exist ... alarm OFF " << std::endl;
			std::cout << " ================================== " << std::endl;			
			int problem = 35100;
			unsigned int severity = TREFC_Util::Severity_CEASING; //change the alarm severity, cause, data and text according to POD
			std::string cause = "TIME SERVICE NOT STARTED";
			std::string data = "Time Service Configuration Fault";
			std::string text;
			bool isSMX = false;
			TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
			if(environment == TREFC_Util::SMX)
				isSMX=true;
			if(isSMX == true)
				text = "TIME SERVICE IS NOT STARTED ON SWITCH BOARDS\nBUT EXTERNAL TIME REFERENCES ARE CONFIGURED";
			else
				text = "TIME SERVICE IS NOT STARTED ON SCX BUT\nEXTERNAL TIME REFERENCES ARE CONFIGURED";

			sendAlarm (problem, severity, cause, data, text);
			{
				char tmpStr[512] = {0};
				if(isSMX == true)
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SWITCH BOARDS BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
				else
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SCX BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
			}
		}
		else  //time reference list not empty
		{
			if(timeout)
			{
				// raise alarm
				std::cout << " ================================== " << std::endl;
				std::cout << " Time server Disabled, time reference exist..."<< std::endl;
				std::cout << " Timer expired... alarm ON " << std::endl;
				std::cout << " ================================== " << std::endl;
				int problem = 35100;
				unsigned int severity = TREFC_Util::Severity_O1; //change the alarm severity, cause, data and text according to POD
				std::string cause = "TIME SERVICE NOT STARTED";
				std::string data = "Time Service Configuration Fault";
				std::string text;
				bool isSMX = false;
				TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
				if(environment == TREFC_Util::SMX)
					isSMX=true;
				if(isSMX == true)
					text = "TIME SERVICE IS NOT STARTED ON SWITCH BOARDS\nBUT EXTERNAL TIME REFERENCES ARE CONFIGURED";
				else
					text = "TIME SERVICE IS NOT STARTED ON SCX BUT\nEXTERNAL TIME REFERENCES ARE CONFIGURED";

				sendAlarm (problem, severity, cause, data, text);
				{
					char tmpStr[512] = {0};
					if(isSMX == true)
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SWITCH BOARDS BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
					else
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_Manager] Ceasing Alarm:\nTIME SERVICE IS NOT STARTED ON SCX BUT EXTERNAL TIME REFERENCES ARE CONFIGURED.");
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
				}
			}
		
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << __LINE__ << "time service disabled, time ref exist, start timer" << std::endl;
				// start timer if not running
				if(m_alarmHandler->isAlarmTimerOngoing() == false)
					m_alarmHandler->startAlarmTimer();
			}
		}
	}
 
}

int FIXS_TREFC_Manager::migrate_ScxTref_to_SwitchTref(bool isSMX)
{
	int type = 0;
	ImmInterface::getServiceType(type,isSMX);

	int rType = 0;
	ImmInterface::getReferenceType(rType,isSMX);

	///FOR SCX A//
	bool configSwitchA= false;
	std::string scxAIP="0.0.0.0";
	if(ImmInterface::getSCXAtrefIP(scxAIP)) {
		configSwitchA =true;
	}

	std::string scxANetMask="255.255.255.0";
	if(ImmInterface::getSCXAtrefNetMask(scxANetMask)) {
		configSwitchA=true;
	}
	std::string scxAGatewayIP="0.0.0.0";
	if(ImmInterface::getSCXAtrefGatewayIP(scxAGatewayIP)) {
		configSwitchA=true;
	}	
	
	if(configSwitchA ==true) {
		int slot=0;
		ImmInterface::configureNetworkConfiguration(slot,scxAIP,scxANetMask,scxAGatewayIP);
	}
	///FOR SCX B//
	bool configSwitchB=false;
	std::string scxBIP="0.0.0.0";
	if(ImmInterface::getSCXBtrefIP(scxBIP)) {
		configSwitchB=true;
	}

	std::string scxBNetMask="255.255.255.0";
	if(ImmInterface::getSCXBtrefNetMask(scxBNetMask)) {
		configSwitchB=true;
	}

	std::string scxBGatewayIP="0.0.0.0";
	if(ImmInterface::getSCXBtrefGatewayIP(scxBGatewayIP)) {
		configSwitchB=true;
	}

	if(configSwitchB ==true) {
                std::cout<<"succesfull in fetching the info of scxB networkconfiguration object"<<std::endl;
                int slot=25;
                if(ImmInterface::configureNetworkConfiguration(slot,scxBIP,scxBNetMask,scxBGatewayIP)) {
                        std::cout << "Successfull in setting the same values to switch B: " << std::endl;
                }
        }

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Updating switch service reference type.");
	if(rType != IMM_Util::NOT_CONFIGURED){
		if(rType == IMM_Util::NETWORK_REFERENCE){
			ImmInterface::setSwitchServiceRefType(IMM_Util::NETWORK_REFERENCE);
		}else if(rType == IMM_Util::GPS_REFERENCE){
			ImmInterface::setSwitchServiceRefType(IMM_Util::GPS_REFERENCE);
		}
	}

	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_Manager] Updating switch service type.");
	if(type != IMM_Util::NOT_CONFIGURED){
		if(type == IMM_Util::NTP){
			ImmInterface::setSwitchServiceType(IMM_Util::NTP);
		}else if(type == IMM_Util::NTPHP){
			ImmInterface::setSwitchServiceType(IMM_Util::NTPHP);
		}
	}
	return 0;
}
