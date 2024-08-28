/*
 * FIXS_CCH_CmxUpgrade.cpp
 *
 *  Created on: Sep 13, 2012
 *      Author: egiufus
 */



// FIXS_CCH_CmxUpgrade
#include "FIXS_CCH_CmxUpgrade.h"

extern int close (int __fd);
ACE_thread_mutex_t s_csCmx;
ACE_thread_mutex_t s_csCmx_snmp;

using namespace std;

namespace {

struct Lock
{
	Lock()
	{
		//std::cout << "FIXS_CCH_CmxUpgrade: Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_trylock(&s_csCmx);
	};

	~Lock()
	{
		//std::cout << "FIXS_CCH_CmxUpgrade: Leave Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_unlock(&s_csCmx);
	};
};

struct Lock_snmp
{
	Lock_snmp()
	{
		//std::cout << "FIXS_CCH_CmxUpgrade: SNMP Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_trylock(&s_csCmx_snmp);
	};

	~Lock_snmp()
	{
		//std::cout << "FIXS_CCH_CmxUpgrade: SNMP Leave Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_unlock(&s_csCmx_snmp);
	};
};
}

namespace {

const std::string INSTALLATION_FOLDER = "/data/apz/data/boot/CMX/";
void printMachineInfo(unsigned long magazine, unsigned short slot, std::string state) {
	std::string value;
	char info[256] = {0};
	CCH_Util::ulongToStringMagazine(magazine, value);
	snprintf(info, sizeof(info) - 1, "magazine: %s, slot: %u, \nSTATE: %s", value.c_str(), slot, state.c_str());

	std::cout << "         SOFTWARE  UPGRADE " << std::endl;
	std::cout << "            ___________" << std::endl;
	std::cout << "    -  ----// --|||-- \\\\         " << std::endl;
	std::cout << "   ---- __//____|||____\\\\____   " << std::endl;
	std::cout << "       | _|    \" | \"   --_  ||" << std::endl;
	std::cout << "   ----|/ \\______|______/ \\_|| " << std::endl;
	std::cout << "  ______\\_/_____________\\_/_______" << std::endl;
	std::cout << "STATE MACHINE FOR " << info << "\n"<< std::endl;

};


// The indexes of events used by SwUpgrade thread
enum Event_Indexes
{
	Shutdown_Event_Index = 0,
	Command_Event_Index = 1,   // Command from client
	TimeoutTrap_Event_Index = 2,
	//		Stop_Event_Index = 3,
	Number_Of_Events = 3
};

//DWORD Safe_Timeout = 120000;
DWORD SIX_MINUTES = 360000;
DWORD TEN_MINUTES = 600000;

//int MAX_RETRY = 3;

#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif

}

// Class FIXS_CCH_CmxUpgrade::CmxState
FIXS_CCH_CmxUpgrade::CmxState::CmxState()
: m_context(0)
{
	m_id = FIXS_CCH_CmxUpgrade::STATE_UNDEFINED;
	m_set_status = false;
}

FIXS_CCH_CmxUpgrade::CmxState::CmxState (FIXS_CCH_CmxUpgrade::CmxStateId stateId)
: m_context(0)
{
	m_id = stateId;
	m_set_status = false;
}


FIXS_CCH_CmxUpgrade::CmxState::~CmxState()
{

}

//## Other Operations (implementation)
void FIXS_CCH_CmxUpgrade::CmxState::setContext (FIXS_CCH_CmxUpgrade* context)
{
	m_context = context;
}

FIXS_CCH_CmxUpgrade::CmxStateId FIXS_CCH_CmxUpgrade::CmxState::getStateId () const
{
	return m_id;
}

int FIXS_CCH_CmxUpgrade::CmxState::activateSW ()
{

	int result = 0;
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << " *********************** WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION_ACTIVATE;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;

}

int FIXS_CCH_CmxUpgrade::CmxState::commit ()
{
	int result = 0;
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << " *********************** WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION_COMMIT;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

int FIXS_CCH_CmxUpgrade::CmxState::prepare (std::string packageToUpgrade, std::string productToUpgrade)
{

	UNUSED(packageToUpgrade);
	UNUSED(productToUpgrade);

	int result = 0;
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << " *********************** WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION_PREPARE;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

int FIXS_CCH_CmxUpgrade::CmxState::cancel ()
{
	int result = 0;
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << " *********************** WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION_CANCEL;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

int FIXS_CCH_CmxUpgrade::CmxState::loadReleaseCompleted (int loadResult)
{
	int result = 0;
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	UNUSED(loadResult);

	std::cout << " *********************** WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

int FIXS_CCH_CmxUpgrade::CmxState::coldStart ()
{
	int result = 0;
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << " *********************** WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

// Class FIXS_CCH_CmxUpgrade::State_Activated

FIXS_CCH_CmxUpgrade::State_Activated::State_Activated()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_ACTIVATED)
{

}


FIXS_CCH_CmxUpgrade::State_Activated::~State_Activated()
{

}

//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Activated::commit ()
{

	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Commit on Activated State "<<std::endl;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Commit on Activated State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	bool success = false;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Try to make SW as Default" << std::endl;

	for (int i = 0; (i < 5) && (!success); i++)
	{
		success = m_context->makeDefault(); //	snmp set (make default)  // <---------TO COMPLETE
		if (!success)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " snmp set (make default) failed.. sleep for 1 second and retry" << std::endl;
			sleep(1);
		}
	}

	if (success)
	{

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Commit Success !", __FUNCTION__, __LINE__);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
		}
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Commit success !!! " << std::endl;

		m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_COMMIT_RECEIVED);
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Commit FAILED !!! " << std::endl;


		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Commit Failed !", __FUNCTION__, __LINE__);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
		}

		//try once again
		m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_COMMIT_RECEIVED_FAILED);

	}
	return result;
}

int FIXS_CCH_CmxUpgrade::State_Activated::cancel ()
{
	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Cancel on Activated State" << std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Cancel on Activated State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	
	bool shutdownsignaled = false;
	bool startReqStatus = m_context->sendStartRequestToCP(shutdownsignaled);
	if(shutdownsignaled) return result;

	if (startReqStatus == false) return NOT_ALLOWED_BY_CP;
		//return SNMP_FAILURE;

	//Rollback DHCP configuration
	if (m_context->configureDHCP(true) == false)
	{
		result = NO_REPLY_FROM_CS;
	}
	else
	{
		m_context->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_START_UPGRADE);

		// Start the CMX Reset
		m_context->resetSwitch();

		m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_CANCEL_RECEIVED);
		result = EXECUTED;
	}

	return result;
}

// Class FIXS_CCH_CmxUpgrade::State_Activating
FIXS_CCH_CmxUpgrade::State_Activating::State_Activating()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_ACTIVATING)
{

}


FIXS_CCH_CmxUpgrade::State_Activating::~State_Activating()
{

}


//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Activating::coldStart ()
{
	int res = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ColdStart on Activating State" << std::endl;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ColdStart on Activating State ", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	bool readSuccess = false;
	sleep(1); //CMX needs some second to update the MIB
	for (int i = 0; (i < 5) && (!readSuccess); i++)
	{
		readSuccess = m_context->checkCurrentLoadedVersion();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " retry number " << i << std::endl;
		if (!readSuccess)
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] read failed... sleep for 5 seconds and retry ", __FUNCTION__, __LINE__);
				if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
				if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
			}
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " read failed... sleep for 5 seconds and retry" << std::endl;
			sleep(5);
		}
	}

	if (readSuccess)
	{

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] read success, restart is completed ", __FUNCTION__, __LINE__);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
		}
		
		m_context->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE);
		sleep(1);
		//activated (new value!)
		m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_WARMRESTART_NEW_COMPLETED);
		res = EXECUTED;
	}
	else
	{

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Failed: the Current Loaded SW Version on CMX has not been changed, rolling back DHCP", __FUNCTION__, __LINE__);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
		}
		//rollback DHCP
		m_context->configureDHCP(true);

		// reset progress data
		m_context->setExpectedCXC();
		m_context->setExpectedCXP();

		//in order to cold restart CMX we need to change the state machine by handling a new state logic to
		//manage the cold start and the maintenance windows //m_cont->resetSwitch(); //then wait for the cold start trap

		m_context->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE);
		sleep(1);
		//failed
		//Value is not changed (old SW)
		m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_WARMRESTART_OLD_COMPLETED);
		res = FAILED_OPERATION;
	}

//	m_context->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE);

	return res;
}

// Class FIXS_CCH_CmxUpgrade::State_Automatic_Rollback_Ended

FIXS_CCH_CmxUpgrade::State_Automatic_Rollback_Ended::State_Automatic_Rollback_Ended()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_AUTOMATIC_ROLLBACK_ENDED)
{
}


FIXS_CCH_CmxUpgrade::State_Automatic_Rollback_Ended::~State_Automatic_Rollback_Ended()
{
}

//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Automatic_Rollback_Ended::commit ()
{
	int result = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Commit on AutomaticRollbackEnd State"<<std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Commit on AutomaticRollbackEnd State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_context->setExpectedCXC();
	m_context->setExpectedCXP();
	m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_COMMIT_RECEIVED);

	return result;
}

// Class FIXS_CCH_CmxUpgrade::State_Manual_Rollback_Ended

FIXS_CCH_CmxUpgrade::State_Manual_Rollback_Ended::State_Manual_Rollback_Ended()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_MANUAL_ROLLBACK_ENDED)
{
}


FIXS_CCH_CmxUpgrade::State_Manual_Rollback_Ended::~State_Manual_Rollback_Ended()
{
}


//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Manual_Rollback_Ended::commit ()
{
	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Commit on Manual_Rollback_Ended State " << std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Commit on Manual_Rollback_Ended State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_context->setExpectedCXC();
	m_context->setExpectedCXP();
	m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_COMMIT_RECEIVED);
	result = EXECUTED;

	return result;
}
// Class FIXS_CCH_CmxUpgrade::State_Idle

FIXS_CCH_CmxUpgrade::State_Idle::State_Idle()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_IDLE)
{
}


FIXS_CCH_CmxUpgrade::State_Idle::~State_Idle()
{
}

//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Idle::prepare (std::string packageToUpgrade, std::string productToUpgrade)
{
	/*
		packageToUpgrade = Container = CXP
		productToUpgrade = sw version = CXC
	 */
	bool success = false;
	int result = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Prepare on Idle State" << std::endl;

	std::cout << __FUNCTION__ << "@" << __LINE__ <<
			" packageToUpgrade " << packageToUpgrade.c_str() <<
			" productToUpgrade " << productToUpgrade.c_str() << std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Prepare on Idle State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}


	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,
				"prepare params ---- packageToUpgrade: %s  -  productToUpgrade: %s",
				packageToUpgrade.c_str(), productToUpgrade.c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,&m_context->traceObj);
	}


	if (!m_context->checkCurrentDefaultVersion())
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << ", checkCurrentDefaultVersion failed." << std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << ", ------NEED TO MAKE DEFAULT-------" << std::endl;

		for (int i = 0; (i < 5) && (!success); i++)
		{
			success = m_context->restoreMakeDefault(); //	snmp set (make default)
			if (!success)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " snmp set (make default) failed.. sleep for 1 second and retry" << std::endl;
				sleep(1);
			}
			else
				break;
		}
	}

	if (m_context->preliminaryCheckOnPrepare(productToUpgrade, packageToUpgrade) == false)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << ", preliminaryCheckOnPrepare failed." << std::endl;
		return SW_ALREADY_ON_BOARD;
	}

	m_context->setExpectedCXC(productToUpgrade);
	m_context->setExpectedCXP(packageToUpgrade);

	std::cout << __FUNCTION__ << "@" << __LINE__ << ", ExpectedCXP -> " << m_context->getExpectedCXP().c_str() << std::endl;


	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,
				"after preliminaryCheckOnPrepare ---- packageToUpgrade: %s, productToUpgrade: %s",
				packageToUpgrade.c_str(), productToUpgrade.c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,&m_context->traceObj);
	}

	if (m_context->loadRelease() == false)
	{
		result = SNMP_FAILURE; // 13;
		m_context->setExpectedCXC();
		m_context->setExpectedCXP();

		//Operation not started, State Machine didn't change the IDLE state
	}
	else
	{
		result = m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_PREPARE_RECEIVED);
		result = EXECUTED;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	return result;
}
// Class FIXS_CCH_CmxUpgrade::State_Preparing

FIXS_CCH_CmxUpgrade::State_Preparing::State_Preparing()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_PREPARING)
{
}

FIXS_CCH_CmxUpgrade::State_Preparing::~State_Preparing()
{
}

//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Preparing::cancel ()
{
	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Cancel on Preparing State" << std::endl;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Cancel on Preparing State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_context->setExpectedCXC();
	m_context->setExpectedCXP();
	m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_CANCEL_RECEIVED);
	result = EXECUTED;

	return result;
}

int FIXS_CCH_CmxUpgrade::State_Preparing::loadReleaseCompleted (int loadResult)
{
	int result = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " loadReleaseCompleted on Preparing State" << std::endl;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] loadReleaseCompleted on Preparing State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " loadResult " << loadResult << std::endl;

	m_context->readActionStatus();

	if (loadResult == 1)
	{
		//ok
		m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_PREPARE_OK);
		result = EXECUTED;


		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Prepare OK - loadResult: %d ", __FUNCTION__, __LINE__, loadResult);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
		}
	}
	else
	{
		//error
		bool checkSuccess = false;
		for (int i = 0; (i < 3) && (!checkSuccess); i++)
		{
			checkSuccess = m_context->checkLoadedVersion();
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " retry number " << i << std::endl;
			if (!checkSuccess)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " read failed.. sleep for 5 seconds and retry" << std::endl;

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] read failed.. sleep for 5 seconds and retry ", __FUNCTION__, __LINE__);
					if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
				}

				sleep(5);
			}
		}

		if (checkSuccess)
		{
			//ok
			m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_PREPARE_OK);
			result = EXECUTED;

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Prepare OK after retries ", __FUNCTION__, __LINE__);
				if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
		else
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Prepare Failed ", __FUNCTION__, __LINE__);
				if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
			}
			m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_FAILURE_RECEIVED);
			result = FAILED_OPERATION;
		}
	}

	return result;
}
// Class FIXS_CCH_CmxUpgrade::State_Prepared

FIXS_CCH_CmxUpgrade::State_Prepared::State_Prepared()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_PREPARED)
{
}


FIXS_CCH_CmxUpgrade::State_Prepared::~State_Prepared()
{
}

//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Prepared::cancel ()
{
	int result = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Cancel on Prepared State "<<std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Cancel on Prepared State ", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_context->setExpectedCXC();
	m_context->setExpectedCXP();
	m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_CANCEL_RECEIVED);
	result = EXECUTED;
	return result;
}

int FIXS_CCH_CmxUpgrade::State_Prepared::activateSW ()
{

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Activate on Prepared State "<<std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Activate on Prepared State ", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	bool shutdownsignaled = false;
	bool startReqStatus = m_context->sendStartRequestToCP(shutdownsignaled);
	if(shutdownsignaled) return 0;	

	if (startReqStatus == false) return NOT_ALLOWED_BY_CP;
		//return SNMP_FAILURE;

	int result = -1;

	if (m_context->configureDHCP(false) == false) // link CS API
	{
		result = NO_REPLY_FROM_CS; // 7
	}
	else if (m_context->startRelease())
	{
		// MAINTENANCE WINDOWS : SNMP NOTIFY (START_UPGRADE)
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_magazine: " << m_context->m_magazine << std::endl;
        m_context->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_START_UPGRADE);
		result = m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_ACTIVATE_RECEIVED);
	}
	else
	{
		result = SNMP_FAILURE; //13
		m_context->configureDHCP(true);
	}


	return result;
}

// Class FIXS_CCH_CmxUpgrade::State_Manual_Rollback_Ongoing

FIXS_CCH_CmxUpgrade::State_Manual_Rollback_Ongoing::State_Manual_Rollback_Ongoing()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_MANUAL_ROLLBACK_ONGOING)
{
}


FIXS_CCH_CmxUpgrade::State_Manual_Rollback_Ongoing::~State_Manual_Rollback_Ongoing()
{
}

//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Manual_Rollback_Ongoing::coldStart ()
{



	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ColdStart on Manual_Rollback_Ongoing State "<<std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ColdStart on Manual_Rollback_Ongoing State ", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	int result = 0;

	//CMX rollback ended !
    m_context->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE);


	//checkRollBackVersion true if currentSW != newSW
	if (m_context->checkRollBackVersion())
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
				" CheckRollBackVersion ok "<<std::endl;


		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CheckRollBackVersion OK ", __FUNCTION__, __LINE__);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
		}

		//rollback completely done
		m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_WARMRESTART_OLD_COMPLETED);
		result = EXECUTED;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
				" checkRollBackVersion failed "<<std::endl;


		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CheckRollBackVersion failed ", __FUNCTION__, __LINE__);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
		}

		//version didn't rollback
		m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_FAILURE_RECEIVED);
		result = FAILED_OPERATION;
	}

	return result;
}

// Class FIXS_CCH_CmxUpgrade::State_Failed

FIXS_CCH_CmxUpgrade::State_Failed::State_Failed()
:FIXS_CCH_CmxUpgrade::CmxState(FIXS_CCH_CmxUpgrade::STATE_FAILED)
{
}


FIXS_CCH_CmxUpgrade::State_Failed::~State_Failed()
{
}

//## Other Operations (implementation)
int FIXS_CCH_CmxUpgrade::State_Failed::prepare (std::string packageToUpgrade, std::string productToUpgrade)
{
	bool success = false;
	int result = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Prepare on Failed State "<<std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Prepare on Failed State ", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}


	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"prepare params ---- packageToUpgrade: %s, productToUpgrade: %s",packageToUpgrade.c_str(), productToUpgrade.c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,&m_context->traceObj);

	}

	if (!m_context->checkCurrentDefaultVersion())
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << ", checkCurrentDefaultVersion failed." << std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << ", ------NEED TO MAKE DEFAULT-------" << std::endl;

		for (int i = 0; (i < 5) && (!success); i++)
		{
			success = m_context->restoreMakeDefault(); //	snmp set (make default)
			if (!success)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " snmp set (make default) failed.. sleep for 1 second and retry" << std::endl;
				sleep(1);
			}
			else
				break;
		}
	}

	if (m_context->preliminaryCheckOnPrepare(productToUpgrade, packageToUpgrade) == false)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << ", preliminaryCheckOnPrepare failed." << std::endl;


		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PreliminaryCheckOnPrepare failed ", __FUNCTION__, __LINE__);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
		}

		return SW_ALREADY_ON_BOARD;
	}

	m_context->setExpectedCXC(productToUpgrade);
	m_context->setExpectedCXP(packageToUpgrade);


	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,
				"after preliminaryCheckOnPrepare ---- packageToUpgrade: %s, productToUpgrade: %s",
				packageToUpgrade.c_str(), productToUpgrade.c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,&m_context->traceObj);
		//FIXS_CCH_Logger::createInstance()->log(traceChar, FIXS_CCH_Logger::Info);
	}

	if (m_context->loadRelease() == false)
	{
		result = SNMP_FAILURE; //13;
		m_context->setExpectedCXC();
		m_context->setExpectedCXP();

		//Operation not started, remain to Failed state !!!
	}
	else
	{
		result = m_context->setEvent(FIXS_CCH_CmxUpgrade::CMX_PREPARE_RECEIVED);
	}

	return result;
}

// Class FIXS_CCH_CmxUpgrade::EventQueue
FIXS_CCH_CmxUpgrade::EventQueue::EventQueue()
: m_queueHandle(-1)
{
	//Lock lock;

	if ((m_queueHandle = eventfd(0, 0)) == -1)
	{
		// failed to create eventlock_
	}

}


FIXS_CCH_CmxUpgrade::EventQueue::~EventQueue()
{
	//Lock lock;

	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}

	if (m_queueHandle)
		::close(m_queueHandle);     // close event handle

}



//## Other Operations (implementation)
FIXS_CCH_CmxUpgrade::CmxEventId FIXS_CCH_CmxUpgrade::EventQueue::getFirst ()
{
	//FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
	//Lock lock;

	CmxEventId result = CMX_DEFAULT_NO_EVENT;
	if (!m_qEventInfo.empty())
	{
		result = m_qEventInfo.front();
	}


	return result;

}

void FIXS_CCH_CmxUpgrade::EventQueue::popData ()
{
	//	 FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
	//Lock lock;

	if (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
		//if ((m_qEventInfo.size() == 0) && ((m_queueHandle != -1) && (m_queueHandle != 0))) //ResetEvent(m_queueHandle);	FIXS_CCH_Event::ResetEvent(m_queueHandle);
	}

}

void FIXS_CCH_CmxUpgrade::EventQueue::pushData (FIXS_CCH_CmxUpgrade::CmxEventId info)
{
	//	 FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
	//Lock lock;

	{
		bool exist = false;

		cout << __FUNCTION__ << "  "<< __LINE__<< "   m_qEventInfo.size() = "<< m_qEventInfo.size() << endl;

		for (unsigned int i=0; i<m_qEventInfo.size();i++)
		{
			//check first element
			FIXS_CCH_CmxUpgrade::CmxEventId first;
			first = getFirst();
			if (first == info )	exist = true;

			m_qEventInfo.pop_front();
			m_qEventInfo.push_back(first);

		}

		if (!exist)
		{
			m_qEventInfo.push_back(info);

			cout << __FUNCTION__ << "  "<< __LINE__<< "   m_qEventInfo.size() = "<< m_qEventInfo.size() << endl;

			int retries = 0;

			while (FIXS_CCH_Event::SetEvent(m_queueHandle) == 0)
			{
				cout << "Set m_queueHandle.... retries: " << retries << endl;
				if( ++retries > 10) break;
				sleep(1);
			}
		}
	}

}

bool FIXS_CCH_CmxUpgrade::EventQueue::queueEmpty ()
{
	//Lock lock;

	return m_qEventInfo.empty();

}

size_t FIXS_CCH_CmxUpgrade::EventQueue::queueSize ()
{
	//Lock lock;

	return m_qEventInfo.size();

}

FIXS_CCH_CmxUpgrade::CmxEventId FIXS_CCH_CmxUpgrade::EventQueue::getItem (int index)
{

	//Lock lock;

	std::list<CmxEventId>::iterator it = m_qEventInfo.begin();
	while (index-- && (it != m_qEventInfo.end())) ++it;

	if (it == m_qEventInfo.end())
		return CMX_DEFAULT_NO_EVENT;
	else
		return *it;

}

void FIXS_CCH_CmxUpgrade::EventQueue::cleanQueue()
{
	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}
}

// Class FIXS_CCH_CmxUpgrade

FIXS_CCH_CmxUpgrade::FIXS_CCH_CmxUpgrade()
: trapTimer(NULL),
  m_exit(false),
  m_running(false),
  m_lastErrorCode(0),
  m_upgradeStatus(0),
  m_fullUpradeType(false),
  m_currentState(NULL)
{

	m_jtpHandler[0] = NULL;
	m_jtpHandler[1] = NULL;
	traceObj = NULL;
	m_lastSwUpgradeReason = CCH_Util::SW_RP_EXECUTED;
	m_lastUpgradeDate = "";
	m_lastUpgradeTime = "";
	m_lastSwUpgradeResult = CCH_Util::SW_RP_OK;
	m_timeoutTrapEvent = 0;
	m_shutdownEvent = 0;
	FIXS_CCH_logging = NULL;
	m_previousCxp = "";
	m_active_commit = 0;
	m_slot = 0;
	m_magazine = 0;
	m_boardId = 0;
	m_cmdEvent = 0;
}

FIXS_CCH_CmxUpgrade::FIXS_CCH_CmxUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string cmx_IP_EthA, std::string cmx_IP_EthB, std::string dn)
:trapTimer(NULL), m_exit(false), m_running(false),
 m_lastErrorCode(0),
 m_upgradeStatus(0),
 m_fullUpradeType(false),
 m_eventQueue(),
 m_slot(slot), m_magazine(magazine), m_boardId(boardId), m_cmx_IP_EthA(cmx_IP_EthA), m_cmx_IP_EthB(cmx_IP_EthB), m_currentCxp(installedPackage), m_expectedCxp(""),m_Object_DN(dn)
,m_active_commit(0)
{

	m_jtpHandler[0] = NULL;
	m_jtpHandler[1] = NULL;
	m_idle.setContext(this);
	m_preparing.setContext(this);
	m_prepared.setContext(this);
	m_activating.setContext(this);
	m_activated.setContext(this);
	m_failed.setContext(this);
	m_automaticRollbackEnded.setContext(this);
	m_manualRollbackOngoing.setContext(this);
	m_manualRollbackEnded.setContext(this);
	m_currentState = &m_idle;

	m_lastSwUpgradeReason = CCH_Util::SW_RP_EXECUTED;
	m_lastUpgradeDate = "";
	m_lastUpgradeTime = "";
	m_lastSwUpgradeResult = CCH_Util::SW_RP_OK;

	m_cmx_IP = m_cmx_IP_EthA;

	traceObj = new ACS_TRA_trace("FIXS_CCH_CmxUpgrade");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	//define Report Progress DN
	IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot,m_SwInventory_Object_DN);

	dn_blade_persistance =  IMM_Interface::getDnBladePersistance(m_magazine,m_slot);

	m_currentCxc.clear();
	m_currentCxc = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(installedPackage);

	m_previousCxp = m_currentCxp;

	unsigned long hlmag = ntohl(this->m_magazine);
	unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
	unsigned char mag = u_plug0 & 0x0F;

	if (traceObj->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, " -  SW Upgrade: magazine [%x] slot [%d] cxc [%s]-  ",mag,slot,m_currentCxc.c_str() );
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		traceObj->ACS_TRA_event(1,tmpStr);
	}

	//get Hostname
	std::string apHostName = CCH_Util::GetAPHostName();

	if (apHostName.compare("SC-2-1") == 0)
	{
		m_APNode = "A";
	}
	else if (apHostName.compare("SC-2-2") == 0)
	{
		m_APNode = "B";
	}
	else
	{
		m_APNode = "";
	}

	m_cmdEvent = 0;
	m_shutdownEvent = 0;

}

FIXS_CCH_CmxUpgrade::~FIXS_CCH_CmxUpgrade()
{

	if (m_jtpHandler[0])
	{
		if (m_jtpHandler[0]->isRunningThread())
		{
			m_jtpHandler[0]->stop();
		}
	}

	if (m_jtpHandler[1])
	{
		if (m_jtpHandler[1]->isRunningThread())
		{
			m_jtpHandler[1]->stop();
		}
	}

	int retry = 0;

	if (m_jtpHandler[0])
	{
		while (m_jtpHandler[0]->isRunningThread() && retry < 5)
		{
			cout<< " m_jtpHandler[0] THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		delete m_jtpHandler[0];
		m_jtpHandler[0] = NULL;
	}

	retry = 0;
	if (m_jtpHandler[1])
	{
		while (m_jtpHandler[1]->isRunningThread() && retry < 5){
			cout<< " m_jtpHandler[1] THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		delete m_jtpHandler[1];
		m_jtpHandler[1] = NULL;
	}

	delete (traceObj);
	traceObj = NULL;

	if (m_cmdEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_cmdEvent);
	}

	if ( m_timeoutTrapEvent && (m_timeoutTrapEvent != -1) )
	{
		//CancelWaitableTimer(m_timeoutTrapEvent);// do this even if the timer has not started
		::close(m_timeoutTrapEvent);
	}

	FIXS_CCH_logging = 0;

}


int FIXS_CCH_CmxUpgrade::activateSW ()
{

	//  	std::cout << "DBG:####################### " << __FUNCTION__ << "@" << __LINE__ << " m_currentState->get() " << getStateName().c_str()<< std::endl;
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);
	int result = m_currentState->activateSW();

	return result;
}

int FIXS_CCH_CmxUpgrade::commit ()
{

	//  	std::cout << "DBG:####################### " << __FUNCTION__ << "@" << __LINE__ << " m_currentState->get() " << getStateName().c_str()<< std::endl;
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);

	int result = m_currentState->commit();
	return result;
}

int FIXS_CCH_CmxUpgrade::prepare (std::string packageToUpgrade, std::string productToUpgrade)
{

	//define Report Progress DN
	IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot,m_SwInventory_Object_DN);

	//copy value of current package
	m_previousCxp = m_currentCxp;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] packageToUpgrade: %s  -  productToUpgrade: %s ", __FUNCTION__, __LINE__,packageToUpgrade.c_str(), productToUpgrade.c_str());
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}

	int result = m_currentState->prepare(packageToUpgrade, productToUpgrade);

	return result;
}

int FIXS_CCH_CmxUpgrade::setEvent (FIXS_CCH_CmxUpgrade::CmxEventId event)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << endl;
	m_eventQueue.pushData(event);

	return 0;
}

int FIXS_CCH_CmxUpgrade::open (void *args)
{
	int result = 0;
	UNUSED(args);
	cout << "\n STATE MACHINE Thread Activate... " << endl;

	{//log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"CMX UPGRADE Starting thread on instance [%p]. \n",this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}

	//	 activate(THR_DETACHED);
	activate();
	m_exit = false;
	return result;
}

int FIXS_CCH_CmxUpgrade::svc ()
{
	DWORD result = 0;
	m_running = true;
	printMachineInfo(m_magazine, m_slot, getStateName());

//	sleep(1);

	initOnStart();
	if (m_exit)
	{
		m_running = false;
		 return result;
	}

	onRestart();
	if (m_exit)
	{
		m_running = false;
		 return result;
	}

	//main loop

	EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent, m_cmdEvent, m_timeoutTrapEvent};

	const int WAIT_FAILED = -1;
	const int WAIT_TIMEOUT = -2;
	const int WAIT_OBJECT_0 = 0;
	const int Shutdown_Event_Index = 0;
	const int Command_Event_Index = 1;
	const int TimeoutTrap_Event_Index = 2;
	//	const int Stop_Event_Index=3;

	//int returnValue = WAIT_TIMEOUT;
	while ( ! m_exit ) // Set to true when the thread shall exit
	{
		fflush(stdout);
		// Wait for shutdown and command events
		//if (! m_exit)
		int returnValue=FIXS_CCH_Event::WaitForEvents(Number_Of_Events, handleArr, 240000);

		switch (returnValue)
		{
		case WAIT_FAILED:   // Event failed
		{
			std::cout << "WAIT_FAILED" << std::endl;
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CmxUpgrade WAIT_FAILED... GetLastError() == " << GetLastError() << std::endl;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects )", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			m_exit = true;    // Terminate the sw upgrade thread
			sleep(1);
			break;
		}

		case (WAIT_TIMEOUT):  // Time out
				{
			//std::cout << __FUNCTION__ << "@" << __LINE__ << " CmxUpgrade is waiting for events..." << std::endl;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WAIT_TIMEOUT ", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			break;
				}

		case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
				{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " CmxUpgrade Shutdown_Event_Index..." << std::endl;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a shutdown event", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			m_exit = true;    // Terminate the thread
			//FIXS_CCH_Event::ResetEvent(m_shutdownEvent);
			sleep(1);
			break;
				}

		case (WAIT_OBJECT_0 + Command_Event_Index):    // Command ready to be handled
				{
			//std::cout << __FUNCTION__ << "@" << __LINE__ << " CmxUpgrade Command_Event_Index => handleCommandEvent()" << std::endl;
			printMachineInfo(m_magazine, m_slot, getStateName());

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CmxUpgrade Command_Event_Index => handleCommandEvent()", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			FIXS_CCH_Event::ResetEvent(m_cmdEvent);

			handleCommandEvent();
			//				if (m_eventQueue.queueEmpty())

			break;
				}

		case (WAIT_OBJECT_0 + (TimeoutTrap_Event_Index)):
				{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " CmxUpgrade TimeoutTrap_Event_Index => handleTrapTimeout()" << std::endl;

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CmxUpgrade TimeoutTrap_Event_Index on event:[%d]", __FUNCTION__, __LINE__,m_timeoutTrapEvent);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			FIXS_CCH_Event::ResetEvent(m_timeoutTrapEvent);
			handleTrapTimeout();
			break;
				}
		//		case (WAIT_OBJECT_0 + (Stop_Event_Index)):
		//		{
		//			std::cout << __FUNCTION__ << "@" << __LINE__ << " CmxUpgrade Stop_Event_Index" << std::endl;
		//
		//
		//			{
		//				char tmpStr[512] = {0};
		//				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CmxUpgrade Stop_Event_Index", __FUNCTION__, __LINE__);
		//				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
		//				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		//			}
		//
		////			FIXS_CCH_Event::ResetEvent(m_StopEvent);
		//			m_exit = true;
		//			break;
		//		}
		default:
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			break;
		}
		} // End of switch

	} // End of the while loop

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_CmxUpgrade [%s@%d] Exiting from while", __FUNCTION__, __LINE__);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_running = false;

	//clean
	resetEvents();

	return result;
}

void FIXS_CCH_CmxUpgrade::stop ()
{
	m_exit = true;

	{//log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"CMX UPGRADE Stopping thread on instance [%p]. \n",this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}

	if (trapTimer!=NULL)
	{
		if (trapTimer->isRunningThread()) trapTimer->stopTimer();
	}

	int retry = 0;
	if (trapTimer!=NULL)
	{
		while (trapTimer->isRunningThread() && retry < 5)
		{
			cout<< " trapTimer THREAD is still running " << endl;
			sleep(1);
			retry++;
		}

		trapTimer->closeHandle();
		delete (trapTimer);
		trapTimer = NULL;
	}

	if (m_jtpHandler[0]!=NULL)
	{
		if (m_jtpHandler[0]->isRunningThread()){

			if (!m_jtpHandler[0]->isSendingNotification()){

				m_jtpHandler[0]->stop();

				retry = 0;
				while (m_jtpHandler[0]->isRunningThread() && retry < 5){
					cout<< " m_jtpHandler[0] THREAD is still running " << endl;
					sleep(1);
					retry++;
				}

				if (!m_jtpHandler[0]->isRunningThread()){

					delete m_jtpHandler[0];
					m_jtpHandler[0] = NULL;
				}


			}
		}
	}

	if (m_jtpHandler[1]!=NULL)
	{
		if (m_jtpHandler[1]->isRunningThread()){

			if (!m_jtpHandler[1]->isSendingNotification()){

				m_jtpHandler[1]->stop();

				retry = 0;
				while (m_jtpHandler[1]->isRunningThread() && retry < 5){
					cout<< " m_jtpHandler[1] THREAD is still running " << endl;
					sleep(1);
					retry++;
				}

				if (!m_jtpHandler[1]->isRunningThread()){
					delete m_jtpHandler[1];
					m_jtpHandler[1] = NULL;
				}
			}
		}
	}
}

void FIXS_CCH_CmxUpgrade::handleCommandEvent ()
{

	//	if (m_eventQueue.queueEmpty()) return;

	while (!m_eventQueue.queueEmpty()) {

		//get event to analyze
		FIXS_CCH_CmxUpgrade::CmxEventId qEventInfo = CMX_DEFAULT_NO_EVENT;
		qEventInfo = m_eventQueue.getFirst();

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"[%s@%d] "
					, __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}

		//switch state
		switchState(qEventInfo);

		m_eventQueue.popData(); //processing ok


		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl <<
				" ------------ CURRENT STATE: " << getStateName().c_str() << " ------------ "
				<< std::endl << std::endl;

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] After switchState ", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
	}

}

void FIXS_CCH_CmxUpgrade::handleTrapTimeout ()
{

	std::string expectedSoftware("CMX_");
	expectedSoftware.append(getExpectedCXC());
	if (getStateName() == "PREPARING") // timer on loadreleasecomplete
	{
		//FIXS_CCH_CriticalSectionGuard snmpLock(m_snmpSyncPtr);
		//Lock_snmp lock;


		{
			char tmpStr[512] = {0};
			unsigned long hlmag = ntohl(this->m_magazine);
			unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
			unsigned char mag = u_plug0 & 0x0F;
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d][MAG: %x, SLOT: %d] TIMER EXPIRED IN STATE == PREPARING, timeout on loadreleasecomplete",__FUNCTION__, __LINE__, mag, m_slot);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
		}


		int countFlag = 0;

		FIXS_CCH_SNMPManager * m_snmpManager = NULL;
		m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

		do{
			if(m_snmpManager->checkLoadedVersion(expectedSoftware))
			{
				// success
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " checkLoadedVersion(" << expectedSoftware.c_str() << ") SUCCESS!!!" << std::endl;
				countFlag=0;

				loadReleaseCompleted(1);

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] checkLoadedVersion success: %s",__FUNCTION__, __LINE__, expectedSoftware.c_str());
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
				}
			}
			else if (countFlag == 0)
			{
				m_snmpManager->setIpAddress(m_cmx_IP_EthB);
				countFlag++;
			}
			else
			{
				// failed
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " checkLoadedVersion(" << expectedSoftware.c_str() << ") FAILED!!!" << std::endl;


				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] checkLoadedVersion failed: %s", __FUNCTION__, __LINE__, expectedSoftware.c_str());
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}

				countFlag++;

				loadReleaseCompleted(0);
			}
		}
		while (countFlag == 1);

		if (m_snmpManager != NULL)
		{
			delete(m_snmpManager);
			m_snmpManager = NULL;
		}


	}
	else if (getStateName() == "ACTIVATING") // timer on cold start
	{
		//FIXS_CCH_CriticalSectionGuard snmpLock(m_snmpSyncPtr);
		//Lock_snmp lock;

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			unsigned long hlmag = ntohl(this->m_magazine);
			unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
			unsigned char mag = u_plug0 & 0x0F;
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d][MAG: %x, SLOT: %d] TIMER EXPIRED IN STATE == ACTIVATING, timeout on loadreleasecomplete",__FUNCTION__, __LINE__, mag, m_slot);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
		}

		// MAINTENANCE WINDOWS : SNMP NOTIFY (END_UPGRADE)
      	if (m_jtpHandler[0])m_jtpHandler[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE);
//		if (m_jtpHandler[1])m_jtpHandler[1]->maintenanceWindow(FIXS_CCH_JTPHandler::CMXSW_END_UPGRADE);


		if (checkCurrentLoadedVersion())
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " checkCurrentLoadedVersion SUCCESS!!!" << std::endl;

			//SUCCESS
			coldStart();
			//setEvent(m_event_swUpEv_new_sw); //go to ACTIVATED
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] checkCurrentLoadedVersion SUCCESS!!", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
			}
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " checkLoadedVersion FAILED!!!" << std::endl;

			//rollback DHCP
			configureDHCP(true);

			// reset progress data
			setExpectedCXC();
			setExpectedCXP();

			//setEvent(m_event_swUpEv_old_sw);  //go to FAILED
			setEvent(FIXS_CCH_CmxUpgrade::CMX_WARMRESTART_OLD_COMPLETED);

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] checkLoadedVersion FAILED!!", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
		}
	}
	else if (getStateName() == "MANUAL_ROLLBACK_ONGOING") // timer on cold start
	{
		//FIXS_CCH_CriticalSectionGuard snmpLock(m_snmpSyncPtr);
		//Lock_snmp lock;

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			unsigned long hlmag = ntohl(this->m_magazine);
			unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
			unsigned char mag = u_plug0 & 0x0F;
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d][MAG: %x, SLOT: %d] TIMER EXPIRED IN STATE == MANUAL_ROLLBACK_ONGOING, timeout on loadreleasecomplete",__FUNCTION__, __LINE__, mag, m_slot);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
		}

		if (m_jtpHandler[0]) m_jtpHandler[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE);
//		if (m_jtpHandler[1]) m_jtpHandler[1]->maintenanceWindow(FIXS_CCH_JTPHandler::CMXSW_END_UPGRADE);


		//check snmp and checkRollBackVersion true if currentSW != newSW
		if ( checkRollBackVersion() )
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] rollback completely done", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
			}
			//rollback completely done
			setEvent(FIXS_CCH_CmxUpgrade::CMX_WARMRESTART_OLD_COMPLETED);
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] rollback Failed ", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			//version didn't rollback
			setEvent(FIXS_CCH_CmxUpgrade::CMX_FAILURE_RECEIVED);
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Timeout expired on state MANUAL_ROLLBACK_ONGOING" << std::endl;
	}

	cancelTrapTimer();

}

bool FIXS_CCH_CmxUpgrade::cancelTrapTimer ()
{

	bool result = false;

	if ((m_timeoutTrapEvent == -1) || (m_timeoutTrapEvent == 0))
	{
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] m_timeoutTrapEvent OFF ", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
		result = true;
	}
	else
	{
		trapTimer->stopTimer();

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] m_timeoutTrapEvent stopped ! ", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
	}

	return result;
}

bool FIXS_CCH_CmxUpgrade::startTrapTimer (int seconds)
{

	if(trapTimer==0)
		return false;

	trapTimer->setTrapTime(seconds);
	if (!trapTimer->isRunningThread()) trapTimer->open();

	if(m_timeoutTrapEvent==-1)
	{
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Synchronization ERROR: cannot set WaitableTimer.", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		}
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "  NOT OK !!! "<<std::endl;

		m_timeoutTrapEvent=trapTimer->getEvent();

		//		return false;
		return true;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "  OK !!! "<<std::endl;
		return true;
	}
}


void FIXS_CCH_CmxUpgrade::initOnStart ()
{

	if (m_exit) return;
	//	m_StopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);

	//TRAP timer
	if (trapTimer == NULL)
	{
		trapTimer = new FIXS_CCH_TrapTimer();
		m_timeoutTrapEvent=trapTimer->getEvent();
	}

	if (traceObj->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1," %s : Trap timer on event: [%d]", __FUNCTION__,m_timeoutTrapEvent);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		traceObj->ACS_TRA_event(1,tmpStr);
	}

	unsigned char byteSlot = (unsigned char)this->m_slot;
	unsigned long hlmag = ntohl(this->m_magazine);
	unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
	unsigned char mag = u_plug0 & 0x0F;


	//JTP handler
	if (m_jtpHandler[0] == NULL) m_jtpHandler[0] = new FIXS_CCH_JTPHandler(mag, byteSlot,1);

	CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
	if (env == CCH_Util::MULTIPLECP_CBA)
	{
		if (m_jtpHandler[1] == NULL) m_jtpHandler[1] = new FIXS_CCH_JTPHandler(mag, byteSlot,2);
	}


	//get command event
	if (m_exit) return;

	if (m_cmdEvent == 0)
		m_cmdEvent = m_eventQueue.getHandle();


	//create shutdown event
	if (m_exit) return;

	if (m_shutdownEvent == 0)
		m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

	if (m_exit) return;


	//	if ( (!m_timeoutTrapEvent) || (m_timeoutTrapEvent == -1) )	m_timeoutTrapEvent=trapTimer->getEvent();

	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] get Event TrapTimer[%d]. \n", __FUNCTION__, __LINE__,m_timeoutTrapEvent);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,traceChar);
	}

	//	if (m_StopEvent == 0)
	//		m_StopEvent=FIXS_CCH_Event::OpenNamedEvent(eventNameStop);

	//Start Thread JTP
	if (m_jtpHandler[0] != NULL) m_jtpHandler[0]->open();
	if (m_jtpHandler[1] != NULL) m_jtpHandler[1]->open();

}

void FIXS_CCH_CmxUpgrade::switchState (FIXS_CCH_CmxUpgrade::CmxEventId eventId)
{
	cout << " DBG: "<< __FUNCTION__ << "  "<< __LINE__  << endl;

	std::string t_currState("");
	std::string t_nextState("");
	std::string t_event("");
	std::string t_type("");

	int status = 0;
	//	int lastResult = 0;
	//	int lastReason = 0;

	t_currState=getStateName();

	switch(eventId)
	{
	case CMX_PREPARE_RECEIVED:
		t_event = "CMX_PREPARE_RECEIVED";
		break;

	case CMX_ACTIVATE_RECEIVED:
		t_event = "CMX_ACTIVATE_RECEIVED";
		break;

	case CMX_COMMIT_RECEIVED:
		t_event = "CMX_COMMIT_RECEIVED";
		break;

	case CMX_CANCEL_RECEIVED:
		t_event = "CMX_CANCEL_RECEIVED";
		break;

	case CMX_PREPARE_OK:
		t_event = "CMX_PREPARE_OK";
		break;

	case CMX_WARMRESTART_NEW_COMPLETED:
		t_event = "CMX_WARMRESTART_NEW_COMPLETED";
		break;

	case CMX_WARMRESTART_OLD_COMPLETED:
		t_event = "CMX_WARMRESTART_OLD_COMPLETED";
		break;

	case CMX_COMMIT_OK:
		t_event = "CMX_COMMIT_OK";
		break;

	case CMX_FAILURE_RECEIVED:
		t_event = "CMX_FAILURE_RECEIVED";
		break;

	case CMX_ROLLBACK_OK:
		t_event = "CMX_ROLLBACK_OK";
		break;

	case CMX_DEFAULT_NO_EVENT:
		t_event = "CMX_DEFAULT_NO_EVENT";
		break;

	default:
		t_event = "UNKNOWN_EVENT";
	}

	//this check is based on currentstate AND eventID
	switch(m_currentState->getStateId())
	{

	case STATE_IDLE:
		switch(eventId)
		{
		case CMX_PREPARE_RECEIVED: // IDLE, CMX_PREPARE_RECEIVED == > PREPARING
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IDLE: CMX_PREPARE_RECEIVED == > PREPARING "<<std::endl;

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IDLE: CMX_PREPARE_RECEIVED == > PREPARING ", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			//start - timer
			//TODO
			startTrapTimer(SIX_MINUTES);

			m_currentState = &m_preparing;
			status = CCH_Util::SW_ERR_OK;
			modifySwInventory(status);

			break;
		default:
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IDLE: ILLEGAL EVENT == > IDLE "<<std::endl;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IDLE: ILLEGAL EVENT == > IDLE ", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}
			break;
		}
		break;

		case STATE_PREPARING:
			switch(eventId)
			{
			case CMX_PREPARE_OK: // PREPARING: CMX_PREPARE_OK == > PREPARED
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARING: CMX_PREPARE_OK == > PREPARED"<<std::endl;

				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARING: CMX_PREPARE_OK == > PREPARED ", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				}

				cancelTrapTimer();
				m_currentState = &m_prepared;
				status = CCH_Util::SW_ERR_OK;
				modifySwInventory(status);

				break;

			case CMX_CANCEL_RECEIVED: // PREPARING: CMX_CANCEL_RECEIVED == > MANUAL_ROLLBACK_ENDED
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARING: CMX_CANCEL_RECEIVED == > MANUAL_ROLLBACK_ENDED "<<std::endl;
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARING: CMX_CANCEL_RECEIVED == > MANUAL_ROLLBACK_ENDED ", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				}

				cancelTrapTimer();
				m_currentState = &m_manualRollbackEnded;
				status = CCH_Util::SW_ERR_CANCELLED;

				m_expectedCxc = "";
				m_expectedCxp = "";
				modifySwInventory(status);


				break;

			case CMX_FAILURE_RECEIVED: // PREPARING: CMX_FAILURE_RECEIVED == > AUTOMATIC_ROLLBACK_ENDED

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARING: CMX_FAILURE_RECEIVED == > AUTOMATIC_ROLLBACK_ENDED "<<std::endl;
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARING: CMX_FAILURE_RECEIVED == > AUTOMATIC_ROLLBACK_ENDED ", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				cancelTrapTimer();
				m_currentState = &m_automaticRollbackEnded;

				m_expectedCxc = "";
				m_expectedCxp = "";
				readResultStatus();



				break;

			default:
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARING: ILLEGAL EVENT == > PREPARING "<<std::endl;
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARING: ILLEGAL EVENT == > PREPARING ", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				}
				break;
			}
			break;

			case STATE_PREPARED:
				switch(eventId)
				{
				case CMX_ACTIVATE_RECEIVED: // PREPARED: CMX_ACTIVATE_RECEIVED == > ACTIVATING
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARED: CMX_ACTIVATE_RECEIVED == > ACTIVATING"<<std::endl;
					if (traceObj->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARED: CMX_ACTIVATE_RECEIVED == > ACTIVATING ", __FUNCTION__, __LINE__);
						traceObj->ACS_TRA_event(1,tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					startTrapTimer(TEN_MINUTES);
					m_currentState = &m_activating;
					status = CCH_Util::SW_ERR_OK;

					modifySwInventory(status);

					break;

				case CMX_CANCEL_RECEIVED: // PREPARED: CMX_CANCEL_RECEIVED == > MANUAL ROLL BACK ENDED
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARED: CMX_CANCEL_RECEIVED == > MANUAL ROLL BACK ENDED"<<std::endl;
					if (traceObj->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARED: CMX_CANCEL_RECEIVED == > MANUAL ROLL BACK ENDED ", __FUNCTION__, __LINE__);
						traceObj->ACS_TRA_event(1,tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					m_currentState = &m_manualRollbackEnded;
					status = CCH_Util::SW_ERR_CANCELLED;

					m_expectedCxc = "";
					m_expectedCxp = "";
					modifySwInventory(status);


					break;

				default:
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARED: ILLEGAL EVENT == > PREPARED "<<std::endl;
					if (traceObj->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARED: ILLEGAL EVENT == > PREPARED ", __FUNCTION__, __LINE__);
						traceObj->ACS_TRA_event(1,tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
					break;
				}
				break;

				case STATE_ACTIVATING:
					switch(eventId)
					{
					case CMX_WARMRESTART_NEW_COMPLETED: // ACTIVATING: CMX_WARMRESTART_NEW_COMPLETED == > ACTIVATED
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ACTIVATING: CMX_WARMRESTART_NEW_COMPLETED == > ACTIVATED"<<std::endl;
						if (traceObj->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ACTIVATING: CMX_WARMRESTART_NEW_COMPLETED == > ACTIVATED ", __FUNCTION__, __LINE__);
							traceObj->ACS_TRA_event(1,tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}

						cancelTrapTimer();
						// wait for JTPretires to finish
						WaitJtpNotificationRetriesToFinish();
						m_currentState = &m_activated;
						status = CCH_Util::SW_ERR_LOAD_RELEASE_IN_SUCCESS;

						modifySwInventory(status);

						break;

					case CMX_WARMRESTART_OLD_COMPLETED: // ACTIVATING: CMX_WARMRESTART_NEW_COMPLETED == > AUTOMATIC_ROLLBACK_ONGOING
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ACTIVATING: CMX_WARMRESTART_NEW_COMPLETED == > AUTOMATIC_ROLLBACK_Ended "<<std::endl;
						if (traceObj->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ACTIVATING: CMX_WARMRESTART_NEW_COMPLETED == > AUTOMATIC_ROLLBACK_Ended ", __FUNCTION__, __LINE__);
							traceObj->ACS_TRA_event(1,tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}

						cancelTrapTimer();
						// wait for JTPretires to finish
						WaitJtpNotificationRetriesToFinish();
						m_currentState = &m_automaticRollbackEnded;
						status = CCH_Util::SW_ERR_AUTO_FAILURE;

						modifySwInventory(status);

						break;

					default:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ACTIVATING: ILLEGAL EVENT == > ACTIVATING "<<std::endl;
						if (traceObj->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ACTIVATING: ILLEGAL EVENT == > ACTIVATING ", __FUNCTION__, __LINE__);
							traceObj->ACS_TRA_event(1,tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}
						break;
					}
					break;

					case STATE_ACTIVATED:
						switch(eventId)
						{
						case CMX_COMMIT_RECEIVED: // ACTIVATED: CMX_COMMIT_RECEIVED == > IDLE
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ACTIVATED: CMX_COMMIT_RECEIVED == > IDLE"<<std::endl;
							if (traceObj->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ACTIVATED: CMX_COMMIT_RECEIVED == > IDLE ", __FUNCTION__, __LINE__);
								traceObj->ACS_TRA_event(1,tmpStr);
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
							}

							m_currentState = &m_idle;
							//					status = CCH_Util::SW_ERR_OK;
							status = m_lastErrorCode;

							//update usedBy reference
						//	IMM_Interface::updateUsedPackage(m_magazine,m_slot,m_previousCxp,getExpectedCXP());

							//update current sw into the map
							FIXS_CCH_UpgradeManager::getInstance()->setCurrentProduct(m_magazine,m_slot,m_currentCxp);

							setExpectedCXP("");
							setExpectedCXC("");

							modifySwInventory(status);

							//TODO
							FIXS_CCH_UpgradeManager::getInstance()->checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);

							//stop thread
							stop();

							break;

						case CMX_COMMIT_RECEIVED_FAILED: // ACTIVATED: CMX_COMMIT_RECEIVED_FAILED == > SW_ERR_COMMIT_ERROR
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ACTIVATED: CMX_COMMIT_RECEIVED_FAILED == > SW_ERR_COMMIT_ERROR"<<std::endl;
							if (traceObj->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ACTIVATED: CMX_COMMIT_RECEIVED_FAILED == > SW_ERR_COMMIT_ERROR ", __FUNCTION__, __LINE__);
								traceObj->ACS_TRA_event(1,tmpStr);
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
							}

							status = CCH_Util::SW_ERR_COMMIT_ERROR;

							modifySwInventory(status);

							//							FIXS_CCH_UpgradeManager::getInstance()->checkAllSCXSwVer(CCH_Util::SCXB);
							//stop thread
							stop();

							break;

						case CMX_CANCEL_RECEIVED: // ACTIVATED: CMX_CANCEL_RECEIVED == > MANUAL ROLL BACK ONGOING
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ACTIVATED: CMX_CANCEL_RECEIVED == > MANUAL ROLL BACK ONGOING"<<std::endl;
							if (traceObj->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ACTIVATED: CMX_CANCEL_RECEIVED == > MANUAL ROLL BACK ONGOING ", __FUNCTION__, __LINE__);
								traceObj->ACS_TRA_event(1,tmpStr);
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
							}

							startTrapTimer(TEN_MINUTES);
							m_currentState = &m_manualRollbackOngoing;
							status = CCH_Util::SW_ERR_CANCELLED;

							modifySwInventory(status);

							break;

						default:
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ACTIVATED: ILLEGAL EVENT == > ACTIVATED "<<std::endl;
							if (traceObj->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ACTIVATED: ILLEGAL EVENT == > ACTIVATED ", __FUNCTION__, __LINE__);
								traceObj->ACS_TRA_event(1,tmpStr);
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
							}
							break;
						}
						break;

						case STATE_MANUAL_ROLLBACK_ONGOING:
							switch(eventId)
							{
							case CMX_WARMRESTART_OLD_COMPLETED: // MANUAL_ROLLBAK_ONGOING: CMX_ROLLBACK_OK == > MANUAL_ROLLBAK_ENDED
								std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " MANUAL_ROLLBAK_ONGOING: CMX_ROLLBACK_OK == > MANUAL_ROLLBAK_ENDED"<<std::endl;
								if (traceObj->ACS_TRA_ON())
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] MANUAL_ROLLBAK_ONGOING: CMX_ROLLBACK_OK == > MANUAL_ROLLBAK_ENDED ", __FUNCTION__, __LINE__);
									traceObj->ACS_TRA_event(1,tmpStr);
									if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
								}

								cancelTrapTimer();
								// wait for JTPretires to finish
								WaitJtpNotificationRetriesToFinish();
								m_currentState = &m_manualRollbackEnded;
								status = CCH_Util::SW_ERR_CANCELLED;

								m_expectedCxc = "";
								m_expectedCxp = "";
								modifySwInventory(status);


								break;

							case CMX_FAILURE_RECEIVED: // MANUAL_ROLLBAK_ONGOING: CMX_FAILURE_RECEIVED == > FAILED
								std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " MANUAL_ROLLBAK_ONGOING: CMX_FAILURE_RECEIVED == > FAILED"<<std::endl;
								if (traceObj->ACS_TRA_ON())
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] MANUAL_ROLLBAK_ONGOING: CMX_FAILURE_RECEIVED == > FAILED ", __FUNCTION__, __LINE__);
									traceObj->ACS_TRA_event(1,tmpStr);
									if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
								}

								cancelTrapTimer();
								//					m_currentState = &m_failed;
								// wait for JTPretires to finish
								WaitJtpNotificationRetriesToFinish();
								m_currentState = &m_idle;
								status = CCH_Util::SW_ERR_MANUAL_FAILURE;

								m_expectedCxc = "";
								m_expectedCxp = "";
								modifySwInventory(status);

								//stop thread
								stop();

								break;

							default:
								std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " MANUAL_ROLLBAK_ONGOING: ILLEGAL EVENT == > ROLLBAK_ONGOING "<<std::endl;
								if (traceObj->ACS_TRA_ON())
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] MANUAL_ROLLBAK_ONGOING: ILLEGAL EVENT == > ROLLBAK_ONGOING ", __FUNCTION__, __LINE__);
									traceObj->ACS_TRA_event(1,tmpStr);
									if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
								}
								break;
							}
							break;

							case STATE_MANUAL_ROLLBACK_ENDED:
								switch(eventId)
								{
								case CMX_COMMIT_RECEIVED: // MANUAL_ROLLBAK_END: CMX_COMMIT_RECEIVED == > IDLE
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " MANUAL_ROLLBACK_END: CMX_COMMIT_RECEIVED == > IDLE"<<std::endl;
									if (traceObj->ACS_TRA_ON())
									{
										char tmpStr[512] = {0};
										snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] MANUAL_ROLLBACK_END: CMX_COMMIT_RECEIVED == > IDLE ", __FUNCTION__, __LINE__);
										traceObj->ACS_TRA_event(1,tmpStr);
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
									}
									m_currentState = &m_idle;

									//					status = CCH_Util::SW_ERR_OK;
									status = m_lastErrorCode;
									setExpectedCXP("");
									setExpectedCXC("");
									modifySwInventory(status);

									//stop thread
									stop();

									break;

								default:
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " MANUAL_ROLLBACK_ENDED: ILLEGAL EVENT == > MANUAL_ROLLBACK_ENDED "<<std::endl;
									if (traceObj->ACS_TRA_ON())
									{
										char tmpStr[512] = {0};
										snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] MANUAL_ROLLBACK_ENDED: ILLEGAL EVENT == > MANUAL_ROLLBAK_END ", __FUNCTION__, __LINE__);
										traceObj->ACS_TRA_event(1,tmpStr);
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
									}
									break;
								}
								break;

								case STATE_AUTOMATIC_ROLLBACK_ENDED:
									switch(eventId)
									{
									case CMX_COMMIT_RECEIVED: // AUTOMATIC_ROLLBAK_END: CMX_COMMIT_RECEIVED == > IDLE
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " AUTOMATIC_ROLLBAK_END: CMX_COMMIT_RECEIVED == > IDLE"<<std::endl;
										if (traceObj->ACS_TRA_ON())
										{
											char tmpStr[512] = {0};
											snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] AUTOMATIC_ROLLBAK_END: CMX_COMMIT_RECEIVED == > IDLE ", __FUNCTION__, __LINE__);
											traceObj->ACS_TRA_event(1,tmpStr);
											if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
										}
										m_currentState = &m_idle;

										//					status = CCH_Util::SW_ERR_OK;
										status = m_lastErrorCode;
										setExpectedCXP("");
										setExpectedCXC("");
										modifySwInventory(status);

										//stop thread
										stop();

										break;

									default:
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " AUTOMATIC_ROLLBAK_ENDED: ILLEGAL EVENT == > AUTOMATIC_ROLLBAK_END "<<std::endl;
										if (traceObj->ACS_TRA_ON())
										{
											char tmpStr[512] = {0};
											snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] AUTOMATIC_ROLLBAK_ENDED: ILLEGAL EVENT == > AUTOMATIC_ROLLBAK_END ", __FUNCTION__, __LINE__);
											traceObj->ACS_TRA_event(1,tmpStr);
											if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
										}
										break;
									}
									break;

									case STATE_FAILED:
										switch(eventId)
										{
										case CMX_PREPARE_RECEIVED: // FAILED, CMX_PREPARE_RECEIVED == > PREPARING
											std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " FAILED: CMX_PREPARE_RECEIVED == > PREPARING "<<std::endl;

											if (traceObj->ACS_TRA_ON())
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FAILED: CMX_PREPARE_RECEIVED == > PREPARING ", __FUNCTION__, __LINE__);
												traceObj->ACS_TRA_event(1,tmpStr);
												if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
											}

											m_currentState = &m_preparing;
											CCH_Util::getCurrentTime(m_lastUpgradeDate, m_lastUpgradeTime);

											status = CCH_Util::SW_ERR_OK;

											modifySwInventory(status);
											break;
										default:
											std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " FAILED: ILLEGAL EVENT == > FAILED "<<std::endl;
											if (traceObj->ACS_TRA_ON())
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FAILED: ILLEGAL EVENT == > FAILED ", __FUNCTION__, __LINE__);
												traceObj->ACS_TRA_event(1,tmpStr);
												if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
											}
											break;
										}
										break;

										case STATE_UNDEFINED:

											break;

										default:
											break;

	}

	saveStatus();
}

int FIXS_CCH_CmxUpgrade::cancel ()
{
	//
	//	 std::cout << "DBG:####################### " << __FUNCTION__ << "@" << __LINE__ << " m_currentState->get() " << getStateName().c_str()<< std::endl;
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);
	int result = m_currentState->cancel();
	return result;
}

std::string FIXS_CCH_CmxUpgrade::getStateName ()
{
	std::string t_currState="";

	switch(getStateId())
	{
	case STATE_UNDEFINED:
		t_currState = "UNDEFINED";
		break;
	case STATE_IDLE:
		t_currState = "IDLE";
		break;

	case STATE_PREPARING:
		t_currState = "PREPARING";
		break;

	case STATE_PREPARED:
		t_currState = "PREPARED";
		break;

	case STATE_ACTIVATING:
		t_currState = "ACTIVATING";
		break;

	case STATE_ACTIVATED:
		t_currState = "ACTIVATED";
		break;

	case STATE_FAILED:
		t_currState = "FAILED";
		break;

	case STATE_MANUAL_ROLLBACK_ONGOING:
		t_currState = "MANUAL_ROLLBACK_ONGOING";
		break;

	case STATE_MANUAL_ROLLBACK_ENDED:
		t_currState = "MANUAL_ROLLBACK_ENDED";
		break;

	case STATE_AUTOMATIC_ROLLBACK_ENDED:
		t_currState = "AUTOMATIC_ROLLBACK_ENDED";
		break;
	default:
		t_currState = "UNKNOWN_STATE";
	}

	return t_currState;
}

bool FIXS_CCH_CmxUpgrade::configureDHCP (bool rollback)
{


	bool retValue = false;
	std::string package("");
	std::string product("");

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
			" m_currentCxp == " << m_currentCxp.c_str() <<
			" getExpectedCXP() == " << getExpectedCXP().c_str() << std::endl;

	if (rollback)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_currentCxp == " << m_currentCxp.c_str() << std::endl;
		package = m_currentCxp;
		product = m_currentCxc;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " getExpectedCXP() == " << getExpectedCXP().c_str() << std::endl;
		package = getExpectedCXP();
		product = getExpectedCXC();
	}

	if (traceObj->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,
				"[%s@%d] Configuring DHCP with package Id = %s.This operation %s a rollback",
				__FUNCTION__, __LINE__, package.c_str(), ( rollback ? "is" : "isn't") );

		traceObj->ACS_TRA_event(1,tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
	}

	std::string magazine("");
	ACS_CS_API_SET_NS::CS_API_Set_Result result;

	CCH_Util::ulongToStringMagazine(m_magazine,magazine);
	//	CCH_Util::findAndReplace(magazine,".","_");

	cout << "---------------------- API BEGIN---------------------------------------- " << endl;
	cout << "magazine :" << magazine.c_str() << endl;
	cout << "slot :" << m_slot << endl;
	cout << "package :" << package.c_str() << endl;
	cout << "product :" << product.c_str() << endl;
	cout << "---------------------- API END---------------------------------------- " << endl;

	result = ACS_CS_API_Set::setSoftwareVersion(package, magazine, m_slot);
	std::cout << "\n......m_previousCxp = " << m_previousCxp.c_str() << std::endl;
	if(rollback)
	{
		IMM_Interface::updateUsedPackage(m_magazine,m_slot,getExpectedCXP(),m_currentCxp);
	}
	else
	{
		IMM_Interface::updateUsedPackage(m_magazine,m_slot,m_previousCxp,getExpectedCXP());
	}

	switch (result)
	{
	case ACS_CS_API_SET_NS::Result_Success:
		retValue = true;
		break;
	default:
		char tmpStr[512] = {0};
                snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSoftwareVersion(%s) "
                                            , __FUNCTION__, __LINE__, package.c_str());
                if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		retValue = false;
		break;
	}

	return retValue;

	//return true;
}

bool FIXS_CCH_CmxUpgrade::preliminaryCheckOnPrepare (std::string softwareVersion, std::string containerId)
{

	bool result = true;
	std::string currCXC("");
	currCXC = ACS_APGCC::after(this->m_currentCxc,"CMX_");

	if (currCXC.empty()) currCXC = this->m_currentCxc;

	if (strcmp(currCXC.c_str(),softwareVersion.c_str()) == 0)
	{
		result = false;
	}

	{
		char tmpStr[512] = {0};
		if (!result) snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] preliminaryCheckOnPrepare failed! package got from user: %s, installed package got from CS: %s", __FUNCTION__, __LINE__, containerId.c_str(), m_currentCxp.c_str());
		else snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] preliminaryCheckOnPrepare is ok! package got from user: %s, installed package got from CS: %s", __FUNCTION__, __LINE__, containerId.c_str(), m_currentCxp.c_str());
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}
	return result;
}

FIXS_CCH_CmxUpgrade::CmxStateId FIXS_CCH_CmxUpgrade::getStateId () const
{
	return m_currentState->getStateId();
}

bool FIXS_CCH_CmxUpgrade::loadRelease ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	bool result = false;

	int countFlag=0;
	string ipadd("");

	std::string url("");
	std::string ip("");
	std::string sub = "169";
	std::string cmxfilename("");

	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do  // if the first snmp statement fails, trying second one
	{
		//check IPN
		ip = m_snmpManager->getIpAddress();

		if (ip.length() > 10)
		{
			std::cout<< "ip.length: "<< ip.length()<<std::endl;
			sub = ip.substr(8,3);
		}

		//url = "tftp://192.168."+ sub + ".33/CMX/"+ getExpectedCXP();

		std::string folder = INSTALLATION_FOLDER + getExpectedCXP();

		//todo
		cmxfilename = getCmxFileName(m_expectedCxp);

		//		cmxfilename = "CMX3.tar";

		//url = "tftp://192.168."+ sub + ".33/CMX/"+ getExpectedCXP() + "/" + cmxfilename;

		if (m_APNode.length() <= 0)
		{
			//Old method
			url = "tftp://192.168."+ sub + ".33/boot/CMX/"+ getExpectedCXP() + "/" + cmxfilename;
		}
		else
		{
			if (m_APNode == "A")
			{
				url = "tftp://192.168."+ sub + ".1/boot/CMX/"+ getExpectedCXP() + "/" + cmxfilename;
			}
			else if (m_APNode == "B")
			{
				url = "tftp://192.168."+ sub + ".2/boot/CMX/"+ getExpectedCXP() + "/" + cmxfilename;
			}
			else
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] "
							"Unknown char: %s ... use old method",
							__FUNCTION__, __LINE__, m_APNode.c_str() );
					traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
				}
				//Old method
				url = "tftp://192.168."+ sub + ".33/boot/CMX/"+ getExpectedCXP() + "/" + cmxfilename;
			}
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Url : " << url.c_str() << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Substr ip: " << sub << std::endl;

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] "
					"URL: %s ", __FUNCTION__, __LINE__, url.c_str() );
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}

		if(m_snmpManager->setSWMActionURL(url))
		{
			result = true;
			countFlag=0;
		}
		else if (countFlag == 0)
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMActionURL(URL)- switch IP Address "
						, __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}

			m_snmpManager->setIpAddress(m_cmx_IP_EthB);
			countFlag++;
			result = false;
		}
		else
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMActionURL(URL) "
						, __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	if (result)
	{
		countFlag=0;
		result = false;

		m_snmpManager->setIpAddress(m_cmx_IP_EthA);

		do  // if the first snmp statement fails, trying second one
		{
			if (m_snmpManager->setSWMAction(FIXS_CCH_SNMPManager::LOADRELEASE) == 1)
			{
				result = true;
				countFlag=0;
			}
			else if (countFlag == 0)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMAction(FIXS_CCH_SNMPManager::LOADRELEASE)-switch IP Address "
							, __FUNCTION__, __LINE__);
					if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}

				m_snmpManager->setIpAddress(m_cmx_IP_EthB);
				countFlag++;
				result = false;
			}
			else
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMAction(FIXS_CCH_SNMPManager::LOADRELEASE) "
							, __FUNCTION__, __LINE__);
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				countFlag++;
				result = false;
			}
		}
		while(countFlag == 1);
	}

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	return result;
}


int FIXS_CCH_CmxUpgrade::loadReleaseCompleted (int loadResult)
{
	int res=0;

	{ //log

		if (traceObj->ACS_TRA_ON())
		{
			char traceChar[512] = {0};
			unsigned long hlmag = ntohl(this->m_magazine);
			unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
			unsigned char mag = u_plug0 & 0x0F;
			snprintf(traceChar, sizeof(traceChar) - 1,"[SOFTWARE UPGRADE on MAG:%x, SLOT:%u] loadReleaseCompleted Trap: %d",mag, m_slot, loadResult);
			traceObj->ACS_TRA_event(1,traceChar);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		}
	}


	res = m_currentState->loadReleaseCompleted(loadResult);

	return res;
}

bool FIXS_CCH_CmxUpgrade::checkLoadedVersion ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	bool result = false;
	std::string expectedSoftware("CMX_");
	expectedSoftware.append(getExpectedCXC());

	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	if (m_snmpManager->checkLoadedVersion(expectedSoftware))
	{
		result = true;
	}
	else
	{
		m_snmpManager->setIpAddress(m_cmx_IP_EthB);
		if (m_snmpManager->checkLoadedVersion(expectedSoftware))
		{
			result = true;
		}
	}

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	return result;
}

bool FIXS_CCH_CmxUpgrade::checkCurrentLoadedVersion ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	std::string currLoadVer("");

	bool result = false;
	int countFlag=0;
	string ipadd("");
	std::string productExpected("CMX_");
	productExpected.append(m_expectedCxc);

	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do
	{
		currLoadVer.clear();
		if (m_snmpManager->getCurrentLoadedVersion(currLoadVer) && (currLoadVer.empty() == false))
		{
			//snmp success
			cout<< __FUNCTION__ << "@" << __LINE__ << ", SNMP value == \"" << currLoadVer.c_str() << "\", internal value == \"" << productExpected.c_str() << "\"" << endl;
			cout<< __FUNCTION__ << "@" << __LINE__ << ", m_expectedCxc == "<< m_expectedCxc.c_str() << endl;
			cout<< __FUNCTION__ << "@" << __LINE__ << ", m_expectedCxp == "<< m_expectedCxp.c_str() << endl;

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"[%s@%d] SNMP value: %s  -  internal value: %s  \n  m_expectedCxc: %s  -  m_expectedCxp: %s "
						, __FUNCTION__, __LINE__,
						currLoadVer.c_str(), productExpected.c_str(),
						m_expectedCxc.c_str(), m_expectedCxp.c_str());
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			if (currLoadVer == productExpected)
			{
				countFlag++;
				result = true;
			}
			else
			{
				cout<< __FUNCTION__ << "@" << __LINE__ << ", ERROR!! ";
				cout<< "SNMP value(\"" << currLoadVer.c_str() << "\") != internal value(\"" << productExpected.c_str() << "\")" << endl;
				result=false;
			}

			//countFlag=0;

		}
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_cmx_IP_EthB);
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in getCurrentLoadedVersion(&currLoadVer)-switching IP Address(%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}



			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in getCurrentLoadedVersion(&currLoadVer) "
						, __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	return result;
}

int FIXS_CCH_CmxUpgrade::coldStart ()
{
	int res=0;
	//	std::cout << "DBG:####################### " << __FUNCTION__ << "@" << __LINE__ << " m_currentState->get() " << getStateName().c_str()<< std::endl;
	{ //log

		if (traceObj->ACS_TRA_ON())
		{
			char traceChar[512] = {0};
			unsigned long hlmag = ntohl(this->m_magazine);
			unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
			unsigned char mag = u_plug0 & 0x0F;
			snprintf(traceChar, sizeof(traceChar) - 1,"[SOFTWARE UPGRADE on MAG:%x, SLOT:%u] Cold Start Trap received",mag, m_slot);
			traceObj->ACS_TRA_event(1,traceChar);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		}

	}
	//res=this->m_CurrState->coldStart();
	res = m_currentState->coldStart();

	return res;
}

void FIXS_CCH_CmxUpgrade::maintenanceWindow (unsigned char msgType)
{

	if (m_jtpHandler[0] != NULL)
	{
		if (!m_jtpHandler[0]->isRunningThread()) m_jtpHandler[0]->open();
	}

	if (m_jtpHandler[1] != NULL)
	{
		if (!m_jtpHandler[1]->isRunningThread()) m_jtpHandler[1]->open();
	}

	if (m_jtpHandler[0]) m_jtpHandler[0]->maintenanceWindow(msgType);
	if (m_jtpHandler[1]) m_jtpHandler[1]->maintenanceWindow(msgType);
}

bool FIXS_CCH_CmxUpgrade::readActionStatus ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	int status = 0;
	bool result = false;
	int countFlag = 0;
	std::string ipadd("");

	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do {
		if ( m_snmpManager->getSwmActionStatus(status))
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"[%s@%d] getSwmActionStatus OK "
						, __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

			setLastErrorCode(status);//m_lastErrorCode = reason;
			result = true;
			countFlag=0;
		}
		else if (countFlag == 0)
		{

			m_snmpManager->setIpAddress(m_cmx_IP_EthB);
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error - switching IP Address(%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}



			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error "
						, __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
			result = false;
		}
	} while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}
	return result;
}

bool FIXS_CCH_CmxUpgrade::startRelease ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	bool result = false;
	int countFlag=0;
	string ipadd("");

	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do  // if the first snmp statement fails, trying second one
	{
		std::string version = "CMX_" + getExpectedCXC();
		if ( m_snmpManager->setSWMActionVersion(version) )
		{
			result = true;
			countFlag=0;
		}
		else if(countFlag==0)
		{
			m_snmpManager->setIpAddress(m_cmx_IP_EthB);
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMActionVersion-switching IP Address(%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}



			countFlag++;
			result = false;
		}
		else
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMActionVersion(%s) "
						, __FUNCTION__, __LINE__, getExpectedCXC().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	if (result)
	{
		countFlag = 0;
		do  // if the first snmp statement fails, trying second one
		{
			if ( m_snmpManager->setSWMAction(FIXS_CCH_SNMPManager::STARTRELEASE) ==1 )
			{
				result = true;
				countFlag=0;
			}
			else if(countFlag==0)
			{
				m_snmpManager->setIpAddress(m_cmx_IP_EthB);
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMAction-switching IP Address(%s) "
							, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}



				countFlag++;
				result = false;
			}
			else
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMAction(%s) "
							, __FUNCTION__, __LINE__, getExpectedCXC().c_str());
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				countFlag++;
				result = false;
			}
		}
		while(countFlag == 1);
	}

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	return result;
}

bool FIXS_CCH_CmxUpgrade::makeDefault ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	bool result = false;
	int countFlag=0;
	string ipadd("");

	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do  // if the first snmp statement fails, trying second one
	{
		if (m_snmpManager->setSWMAction(FIXS_CCH_SNMPManager::MAKEDEFAULT) == 1)
		{
			char tmpStr[512] = {0};
                        snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Executed setSWMAction(FIXS_CCH_SNMPManager::MAKEDEFAULT) "
                                                , __FUNCTION__, __LINE__);
                        if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr);

			setCurrentCXC(getExpectedCXC()); //m_currentCxc = m_expectedCxc;
			result = true;
			countFlag=0;
		}
		else if(countFlag==0)
		{
			m_snmpManager->setIpAddress(m_cmx_IP_EthB);

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMAction(FIXS_CCH_SNMPManager::MAKEDEFAULT)-switching IP Address(%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}

     		countFlag++;
			result = false;
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMAction(FIXS_CCH_SNMPManager::MAKEDEFAULT) "
						, __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	if (result)
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_currentCxp == " << m_currentCxp.c_str() << std::endl;
		m_currentCxp = m_expectedCxp;
	}

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	return result;
}

void FIXS_CCH_CmxUpgrade::resetSwitch ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	// reset switch can only be issued on the CMX Master because
	// the reset OID belongs to the IPMI MIB

	int countFlag = 0;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do  // if the first snmp statement fails, trying second one
	{
		if (m_snmpManager->resetSwitch(m_slot))
		{
			countFlag=0;
		}
		else if(countFlag==0)
		{
			m_snmpManager->setIpAddress(m_cmx_IP_EthB);

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in resetSwitch()-switching IP Address(%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}

			countFlag++;
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in resetSwitch() "
						, __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
		}
	}
	while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}
}

void FIXS_CCH_CmxUpgrade::saveStatus ()
{

	CCH_Util::SWData mydata;
	getOverallStatus(mydata);

	//NOT NEEDED ????
	//FIXS_CCH_FSMDataStorage::getInstance()->saveUpgradeStatus(m_magazine, m_slot, mydata);
}

void FIXS_CCH_CmxUpgrade::onRestart ()
{

	if (getStateName() == "PREPARING" )
	{
		std::string expectedSoftware("CMX_");
		expectedSoftware.append(getExpectedCXC());
		bool checkResult = false;

		{

			FIXS_CCH_SNMPManager * m_snmpManager = NULL;
			m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

			checkResult = m_snmpManager->checkLoadedVersion(expectedSoftware);
			if (!checkResult)
			{
				m_snmpManager->setIpAddress(m_cmx_IP_EthB);
				checkResult = m_snmpManager->checkLoadedVersion(expectedSoftware);
			}

			if (m_snmpManager != NULL)
			{
				delete(m_snmpManager);
				m_snmpManager = NULL;
			}
		}

		if (checkResult)
		{
			// success
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " checkLoadedVersion(" << expectedSoftware.c_str() << ") SUCCESS!!!" << std::endl;
			char tmpStr[512] = {0};
                        snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CHECK ON RESTART: CMX downloaded the new software(%s) "
                                                , __FUNCTION__, __LINE__,expectedSoftware.c_str() );
                        if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
			setEvent(FIXS_CCH_CmxUpgrade::CMX_PREPARE_OK);
		}
		else
		{
			// failed
			if (readActionStatus())
			{

   			        char tmpStr[512] = {0};
				switch (m_lastErrorCode)
				{
				case 1:// - load release in progress
		                        snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CHECK ON RESTART: Load release in progress, started SIX_MINUTES timer  "                                   , __FUNCTION__, __LINE__ );
                		        if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
		                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);

					startTrapTimer(SIX_MINUTES);
					break;
				case 2:// - load release success
                                        snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CHECK ON RESTART: Load release success,CMX downloaded%s \n " , __FUNCTION__, __LINE__,expectedSoftware.c_str() );
                                        if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);


					setEvent(FIXS_CCH_CmxUpgrade::CMX_PREPARE_OK);
					break;
				case 3:// - load release host not found
				case 4:// - load release login incorrect
				case 5://- load release no such directory
				case 6://- load release no such file
				case 7:// - load release erroneous url
				case 8://- load release error
				case 9:// removeReleaseIn Progress
				case 10:// - remove release success
				case 11:// - remove release error
				case 12:// - status uninitiated
				default:
                                        snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CHECK ON RESTART: CMX did not download %s \n " , __FUNCTION__, __LINE__,expectedSoftware.c_str() );
                                        if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);


					configureDHCP(true);//rollback dhcp //DHCP CONFIGURATION MOVED TO Prepare@activate
					setEvent(FIXS_CCH_CmxUpgrade::CMX_FAILURE_RECEIVED);
					break;
				}
			}
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
				   char tmpStr[512] = {0};
                                   snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CHECK ON RESTART:cannot read Status, starting SIX_MINUTES timer   \n) "
                                                , __FUNCTION__, __LINE__ );
                                   if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
                                   if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);

				startTrapTimer(SIX_MINUTES);
			}
		}

	}
	else if (getStateName() == "ACTIVATING")
	{

		//		//Start Thread JTP
		//		if (m_jtpHandler[0]) m_jtpHandler[0]->open();
		//		if (m_jtpHandler[1]) m_jtpHandler[1]->open();

		if (checkCurrentLoadedVersion())
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " checkCurrentLoadedVersion SUCCESS!!!" << std::endl;

			if (m_jtpHandler[0])m_jtpHandler[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE);
//			if (m_jtpHandler[1])m_jtpHandler[1]->maintenanceWindow(FIXS_CCH_JTPHandler::CMXSW_END_UPGRADE);

			setEvent(FIXS_CCH_CmxUpgrade::CMX_WARMRESTART_NEW_COMPLETED);
		}
		else
		{
			startTrapTimer(TEN_MINUTES);
		}
	}
	else if (getStateName() == "MANUAL_ROLLBACK_ONGOING")
	{
		//		//Start Thread JTP
		//		if (m_jtpHandler[0]) m_jtpHandler[0]->open();
		//		if (m_jtpHandler[1]) m_jtpHandler[1]->open();

		startTrapTimer(TEN_MINUTES);
	}

}

void FIXS_CCH_CmxUpgrade::getOverallStatus (CCH_Util::SWData &swdata)
{


	swdata.expectedCxp = m_expectedCxp;
	swdata.expectedCxc = m_expectedCxc;
	swdata.currentCxc = m_currentCxc;
	swdata.currentCxp = m_currentCxp;
	swdata.lastUpgradeDate = m_lastUpgradeDate;
	swdata.lastUpgradeTime = m_lastUpgradeTime;
	swdata.lastUpgradeResult = m_lastSwUpgradeResult;
	swdata.lastUpgradeReason = m_lastSwUpgradeReason;
	swdata.lastErrorCode = m_lastErrorCode;

	switch(getStateId())
	{
	case STATE_IDLE:
		swdata.upgradeStatus = CCH_Util::SW_IDLE;
		break;

	case STATE_PREPARING:
		swdata.upgradeStatus=CCH_Util::SW_PREPARING;
		break;

	case STATE_PREPARED:
		swdata.upgradeStatus=CCH_Util::SW_PREPARED;
		break;

	case STATE_ACTIVATING:
		swdata.upgradeStatus=CCH_Util::SW_ACTIVATING;
		break;

	case STATE_ACTIVATED:
		swdata.upgradeStatus=CCH_Util::SW_ACTIVATED;
		break;

	case STATE_FAILED:
		swdata.upgradeStatus=CCH_Util::SW_FAILED;
		break;

	case STATE_AUTOMATIC_ROLLBACK_ENDED:
		swdata.upgradeStatus=CCH_Util::SW_AUTOMATIC_ROLLBACK_ENDED;
		break;

	case STATE_MANUAL_ROLLBACK_ONGOING:
		swdata.upgradeStatus=CCH_Util::SW_MANUAL_ROLLBACK_ONGOING;
		break;

	case STATE_MANUAL_ROLLBACK_ENDED:
		swdata.upgradeStatus=CCH_Util::SW_MANUAL_ROLLBACK_ENDED;
		break;

	case STATE_UNDEFINED:
	case STATE_AUTOMATIC_ROLLBACK_ONGOING:
		break;
	}

}

void FIXS_CCH_CmxUpgrade::restoreStatus (CCH_Util::SWData &swdata)
{


	m_expectedCxp = swdata.expectedCxp;
	m_expectedCxc = swdata.expectedCxc;
	m_currentCxc = swdata.currentCxc;
	m_currentCxp = swdata.currentCxp;
	m_lastUpgradeDate = swdata.lastUpgradeDate;
	m_lastUpgradeTime = swdata.lastUpgradeTime;
	m_lastSwUpgradeResult = swdata.lastUpgradeResult;
	m_lastSwUpgradeReason = swdata.lastUpgradeReason;
	m_lastErrorCode = swdata.lastErrorCode;

	switch ( swdata.upgradeStatus ) {
	case CCH_Util::SW_PREPARING:
		m_currentState = &m_preparing;
		break;

	case CCH_Util::SW_PREPARED:
		m_currentState = &m_prepared;
		break;

	case CCH_Util::SW_ACTIVATING:
		m_currentState = &m_activating;
		break;

	case CCH_Util::SW_ACTIVATED:
		m_currentState = &m_activated;
		break;

	case CCH_Util::SW_MANUAL_ROLLBACK_ONGOING:
		m_currentState = &m_manualRollbackOngoing;
		break;

	case CCH_Util::SW_MANUAL_ROLLBACK_ENDED:
		m_currentState = &m_manualRollbackEnded;
		break;

	case CCH_Util::SW_AUTOMATIC_ROLLBACK_ENDED:
		m_currentState = &m_automaticRollbackEnded;
		break;

	case CCH_Util::SW_FAILED:
		m_currentState = &m_failed;
		break;

	case CCH_Util::SW_IDLE:
	default:
		m_currentState = &m_idle;
		break;
	}
	//restore into IMM ????????????????????is needed
	//int status = 0;
	//modifySwInventory(status);
}

std::string FIXS_CCH_CmxUpgrade::getCurrentRunningSoftware ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	std::string currentLoadedVer("");
	bool result = false;
	int countFlag = 0;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do
	{
		if ( m_snmpManager->getCurrentLoadedVersion(currentLoadedVer) && (currentLoadedVer.empty() == false))
		{
			result = true;
			countFlag=0;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"[%s@%d] getCurrentLoadedVersion success - currentLoadedVer: %s "
						, __FUNCTION__, __LINE__, currentLoadedVer.c_str());
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}
		}
		else if (countFlag == 0)
		{

			m_snmpManager->setIpAddress(m_cmx_IP_EthB);
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in getCurrentRunningSoftware(&currLoadVer)-switching IP Address(%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}



			countFlag++;
			result = false;
		}
		else
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in getCurrentRunningSoftware(&currLoadVer) "
						, __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
			result = false;
		}
	} while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" currentLoadedVer:"<<currentLoadedVer<<std::endl;
	return currentLoadedVer;
}

bool FIXS_CCH_CmxUpgrade::readResultStatus ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	int status = 0;
	std::string lastResult;
	std::string lastReason;
	bool result = false;
	int countFlag = 0;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do {
		if ( m_snmpManager->getSwmActionStatus(status))
		{
			std::cout << "SW ERR STATUS->Preparing Failed :" << status << std::endl;

			//modify into the IMM model
			modifySwInventory(status);
			result = true;
			countFlag=0;
		}
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_cmx_IP_EthB);
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error - switching IP Address (%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}


			countFlag++;
			result = false;
		}
		else
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error "
						, __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
			result = false;
		}
	} while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	return result;
}

bool FIXS_CCH_CmxUpgrade::checkRollBackVersion ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	bool result = false;
	std::string res;
	std::string expectedSoftware("CMX_");
	expectedSoftware.append(getExpectedCXC());

	std::string cmxVer;

	int retry = 0;
	bool exit = false;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	while (!exit)
	{
		if (m_snmpManager->getCurrentLoadedVersion(cmxVer))
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ <<
					" - checkRollBackVersion: " << cmxVer <<
					" - expectedSoftware: " << expectedSoftware << std::endl;

			exit = true;
			if (cmxVer.compare(expectedSoftware) != 0 )
			{
				//true
				result = true;
				std::cout << __FUNCTION__ << "@" << __LINE__ << " checkRollBackVersion is true " << std::endl;
			}
			else
			{
				//false
				result = false;
				std::cout << __FUNCTION__ << "@" << __LINE__ << " checkRollBackVersion is false " << std::endl;
			}
		}
		else
		{
			m_snmpManager->setIpAddress(m_cmx_IP_EthB);

			retry++;
			usleep(500000);

			//snmp failure
			result = false;
			std::cout << __FUNCTION__ << "@" << __LINE__ << " checkRollBackVersion SNMP ERROR " << std::endl;
		}

		if (retry > 3) exit = true;
	}
	if (!result)	
	{
                                char tmpStr[512] = {0};
                                snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error could not get Current Loaded Version " , __FUNCTION__, __LINE__);
                                if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
                                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
         }

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}
	return result;
}

bool FIXS_CCH_CmxUpgrade::checkCurrentDefaultVersion ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	std::string currLoadVer("");
	std::string currentSoftware("CMX_");
	currentSoftware.append(getCurrentCXC());

	bool result = false;
	int countFlag=0;
	string ipadd;

	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	//m_currentCxc = "CXC123456";

	do
	{
		if (m_snmpManager->getCurrentDefaultVersion(currLoadVer) && (currLoadVer.empty() == false))
		{
			//snmp success
			cout<< __FUNCTION__ << "@" << __LINE__ << ", SNMP value == \"" << currLoadVer.c_str() << "\", internal value == \"" << currentSoftware.c_str() << "\"" << endl;
			cout<< __FUNCTION__ << "@" << __LINE__ << ", m_expectedCxc == "<< m_expectedCxc.c_str() << endl;
			cout<< __FUNCTION__ << "@" << __LINE__ << ", m_expectedCxp == "<< m_expectedCxp.c_str() << endl;

			if (currLoadVer == currentSoftware )
			{
				result = true;
			}
			else
			{
				cout<< __FUNCTION__ << "@" << __LINE__ << ", ERROR!! ";
				cout<< "SNMP value(\"" << currLoadVer.c_str() << "\") != internal value(\"" << m_currentCxc.c_str() << "\")" << endl;
				result=false;
			}

			countFlag=0;

		}
		else if (countFlag == 0)
		{

			m_snmpManager->setIpAddress(m_cmx_IP_EthB);

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in getCurrentLoadedVersion(&currLoadVer)-switching IP Address(%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}

			countFlag++;
			result = false;
		}
		else
		{
			cout<< __FUNCTION__ << "@" << __LINE__ << " NO VALUE !!!!!!!! " << endl;

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in getCurrentLoadedVersion(&currLoadVer) "
						, __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	return result;
}

bool FIXS_CCH_CmxUpgrade::restoreMakeDefault ()
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	bool result = false;
	int countFlag=0;
	string ipadd("");

	m_snmpManager = new FIXS_CCH_SNMPManager(m_cmx_IP_EthA);

	do  // if the first snmp statement fails, trying second one
	{
		if (m_snmpManager->setSWMAction(FIXS_CCH_SNMPManager::MAKEDEFAULT) == 1)
		{
			//TODO setCurrentCXC(getCurrentRunningSoftware()); //m_currentCxc = m_expectedCxc;
			result = true;
			countFlag=0;
		}
		else if (countFlag==0)
		{

			m_snmpManager->setIpAddress(m_cmx_IP_EthB);
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMAction(FIXS_CCH_SNMPManager::MAKEDEFAULT)-switching IP Address(%s) "
						, __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
			}

			countFlag++;
			result = false;
		}
		else
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in setSWMAction(FIXS_CCH_SNMPManager::MAKEDEFAULT) "
						, __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}

	return result;
}

void FIXS_CCH_CmxUpgrade::setObjectDN(const char* dn)
{
	m_Object_DN = dn;
}

void FIXS_CCH_CmxUpgrade::setSwInvObjectDN(std::string dn)
{
	//m_SwInventory_Object_DN = dn.c_str();
	m_SwInventory_Object_DN = dn;
}

std::string FIXS_CCH_CmxUpgrade::getSwInvObjectDN()
{
	return m_SwInventory_Object_DN;
}

std::string FIXS_CCH_CmxUpgrade::getSwPercistanceDN()
{
	return dn_blade_persistance;
}

bool FIXS_CCH_CmxUpgrade::modifySwInventory(int state)
{
	int lastResult = 0;
	int lastReason = 0;
	std::string lastDate("");
	std::string currSw("");
	int currState = this->getStateId();
	CCH_Util::getCurrentTime(m_lastUpgradeDate, m_lastUpgradeTime);
	CCH_Util::trim(m_lastUpgradeTime);
	CCH_Util::getResultReason(lastResult,lastReason,state);
	setLastUpgradeResult(lastResult);
	setLastUpgradeReason(lastReason);
	setLastErrorCode(state);
	lastDate = getLastUpgradeDate();
	currSw = ACS_APGCC::after(m_currentCxc,"CMX_");

	CCH_Util::SWData data;
	data.lastUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(lastReason);
	if (!currSw.empty())  data.currentCxc = currSw;
	else data.currentCxc = m_currentCxc;
	data.expectedCxc = m_expectedCxc;
	data.lastUpgradeDate = m_lastUpgradeDate;
	data.lastErrorCode = state;
	data.lastUpgradeTime = m_lastUpgradeTime;
	data.lastUpgradeResult =  static_cast<CCH_Util::RpUpgradeResult>(lastResult);
	data.swState = currState;
	data.dnBladePersistance = dn_blade_persistance;
	//HT44454
        data.forcedUpgrade=false;
        data.upgradeType=NORMAL_UPGRADE;
        //HT44454
	//cout <<"\n m_SwInventory_Object_DN  "<<m_SwInventory_Object_DN << m_lastSwUpgradeResult <<endl;

	if(!IMM_Interface::modifyAllInventoryAttributes(m_SwInventory_Object_DN,data)) return false;
	else
	{
		std::cout <<"\n ---------------------- ATTRIBUTE CHANGED DURING UPGRADE -------------------------- "<< std::endl;
		return true;
	}
}

bool FIXS_CCH_CmxUpgrade::initSwInventory()
{
	int currState = this->getStateId();

	std::string OtherBlade_Product = "";
	IMM_Interface::getOtherBladeCurrLoadModule(m_Object_DN,OtherBlade_Product);

	CCH_Util::SWData data;
	data.lastUpgradeReason = m_lastSwUpgradeReason;
	data.currentCxc = OtherBlade_Product;
	data.expectedCxc = m_expectedCxc;
	data.lastUpgradeDate = m_lastUpgradeDate;
	data.lastErrorCode = m_lastErrorCode;
	data.lastUpgradeTime = m_lastUpgradeTime;
	data.lastUpgradeResult = m_lastSwUpgradeResult;
	data.swState = currState;

	CCH_Util::SWData swdata;
	swdata.currentCxc = OtherBlade_Product;


	//cout <<"\n m_SwInventory_Object_DN  "<<m_SwInventory_Object_DN << "\nOtherBlade_Product: "<<OtherBlade_Product <<endl;

	if(!IMM_Interface::modifyAllInventoryAttributes(m_SwInventory_Object_DN,data)) return false;
	else
	{
		std::cout <<"\n ---------------------- ATTRIBUTE CHANGED DURING UPGRADE -------------------------- "<< std::endl;
		return true;
	}
}


int FIXS_CCH_CmxUpgrade::getStateImm ()
{
	int t_currState = -1;
	switch(getStateId())
	{
	case STATE_UNDEFINED:
		t_currState = IMM_Util::UNKNOWN;
		break;
	case STATE_IDLE:
		t_currState = IMM_Util::IDLE;
		break;

	case STATE_PREPARING:
		t_currState = IMM_Util::PREPARING;
		break;

	case STATE_PREPARED:
		t_currState = IMM_Util::PREPARED;
		break;

	case STATE_ACTIVATING:
		t_currState = IMM_Util::ACTIVATING;
		break;

	case STATE_ACTIVATED:
		t_currState = IMM_Util::ACTIVATED;
		break;

	case STATE_FAILED:
		t_currState = IMM_Util::FAILED;
		break;

	case STATE_MANUAL_ROLLBACK_ONGOING:
		t_currState = IMM_Util::CANCELLING;
		break;

	case STATE_MANUAL_ROLLBACK_ENDED:
		t_currState = IMM_Util::CANCELLED;
		break;

	case STATE_AUTOMATIC_ROLLBACK_ENDED:
		t_currState = IMM_Util::ROLLBACK_COMPLETED;
		break;
	default:
		t_currState = IMM_Util::UNKNOWN;
	}

	return t_currState;
}

std::string  FIXS_CCH_CmxUpgrade::getCmxFileName( std::string package)
{
	std::string fileName("");

	//get dn package
	std::string swRep_dn("");
	FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(swRep_dn,package);

	//get path package
	std::string pathParam("");
	IMM_Util::getImmAttributeString(swRep_dn,IMM_Util::ATT_SWREPOSITORY_SWPATH,pathParam);

	int iPos = pathParam.find_last_of(CHR_IPMI_DIR_SEPARATOR);
	if(iPos >= 0)
	{
		// Filename is part of input path. Extract it.
		fileName = pathParam.substr(iPos +1);

		cout << "############# fileName : " << fileName.c_str() << endl;
	}

	return fileName;
}

void FIXS_CCH_CmxUpgrade::resetEvents ()
{

	FIXS_CCH_Event::ResetEvent(m_cmdEvent);

	FIXS_CCH_Event::ResetEvent(m_timeoutTrapEvent);

	m_eventQueue.cleanQueue();
}

bool FIXS_CCH_CmxUpgrade::sendStartRequestToCP (bool & shutdownsignaled)
{
  	bool canStartRequest = false;
	m_jtpHandler[0]->setStartReqStatus(true);
	m_jtpHandler[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IMB_UPG_START_REQ); //start request will be sent only to CP1
	m_jtpHandler[0]->waitStartReqToFinish(shutdownsignaled); // wait for start request response from CP
	canStartRequest = m_jtpHandler[0]->checkStartReqStatus();
	return canStartRequest;
}

void FIXS_CCH_CmxUpgrade::WaitJtpNotificationRetriesToFinish()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Wait for  the JTP notification retries to over "<<std::endl;
	int retry =0;
	if (m_jtpHandler[0])
	{
		if (m_jtpHandler[0]->isRunningThread()){
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "  JTP Threads 0 running "<<std::endl;
		
			while(m_jtpHandler[0]->isSendingNotification() && retry < 240)
			{
				sleep(1);
				retry++;
			}
		}
	}
	
	retry =0;
	if (m_jtpHandler[1])
	{
		if (m_jtpHandler[1]->isRunningThread()){
			
			while(m_jtpHandler[1]->isSendingNotification() && retry < 240)
			{
				sleep(1);
				retry++;
			}
		}
	}
}

