
// FIXS_CCH_IplbUpgrade
#include "FIXS_CCH_IplbUpgrade.h"
#include <string.h>
#include <stdio.h>

namespace
{
void printMachineInfo(unsigned long magazine, unsigned short slot,std::string state) {
	std::string value;
	char info[256] = {0};
	CCH_Util::ulongToStringMagazine(magazine, value);
	snprintf(info, sizeof(info) - 1, "magazine: %s, slot: %u,\nSTATE: %s", value.c_str(), slot,state.c_str());

	std::cout << "    IPLB UPGRADE      " << std::endl;
	std::cout << "         _           " << std::endl;
	std::cout << "         ||___       " << std::endl;
	std::cout << "         | ||~:__    " << std::endl;
	std::cout << " |_______________|   " << std::endl;
	std::cout << "   ((_)     ((_)     " << std::endl;
	std::cout << "STATE MACHINE FOR " << info << std::endl;

};

enum Event_Indexes
{
	Shutdown_Event_Index = 0,
	Command_Event_Index = 1,   // Command from client
	Number_Of_Events = 2
};

enum Gpr_Boot_Type
{
	flashBootType = 0,
	networkBootType = 1
};

}

FIXS_CCH_IplbUpgrade::IplbState::IplbState()
: m_context(0)
{
	m_id = FIXS_CCH_IplbUpgrade::STATE_UNDEFINED;
	m_set_status = false;
}

FIXS_CCH_IplbUpgrade::IplbState::IplbState (FIXS_CCH_IplbUpgrade::IplbStateId stateId)
: m_context(0)
{
	m_id = stateId;
	m_set_status = false;
}


FIXS_CCH_IplbUpgrade::IplbState::~IplbState()
{

}

void FIXS_CCH_IplbUpgrade::IplbState::setContext (FIXS_CCH_IplbUpgrade* context)
{
	m_context = context;
}

FIXS_CCH_IplbUpgrade::IplbStateId FIXS_CCH_IplbUpgrade::IplbState::getStateId () const
{
	return m_id;
}

int FIXS_CCH_IplbUpgrade::IplbState::activateSw()
{
	int result = 0;
	std::cout << "WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION_ACTIVATE;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

int FIXS_CCH_IplbUpgrade::IplbState::commit (bool setAsDefaultPackage)
{
	UNUSED(setAsDefaultPackage);
	int result = 0;
	std::cout << "WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION_COMMIT;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

int FIXS_CCH_IplbUpgrade::IplbState::prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation)
{
	UNUSED(packageToUpgrade);
	UNUSED(productToUpgrade);
	UNUSED(partial_installation);
	int result = 0;
	std::cout << "WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION_PREPARE;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

int FIXS_CCH_IplbUpgrade::IplbState::cancel ()
{
	int result = 0;
	std::cout << "WRONG EVENT IN THIS STATE" <<std::endl;
	result = WRONG_OPTION_CANCEL;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE (%s)", __FUNCTION__, __LINE__, m_context->getStateName().c_str());
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	return result;
}

FIXS_CCH_IplbUpgrade::IplbStateActivated::IplbStateActivated()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_ACTIVATED)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Activated Created"<<std::endl;
}

FIXS_CCH_IplbUpgrade::IplbStateActivated::~IplbStateActivated()
{

}

int FIXS_CCH_IplbUpgrade::IplbStateActivated::commit (bool setAsDefaultPackage)
{     
	int result = EXECUTED;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Commit on Activated State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	if (m_context->isBoardBlocked())
	{
		return IRONSIDE_FAILURE;
	}
	if (setAsDefaultPackage) 
	{
		FIXS_CCH_SoftwareMap::getInstance()->setIplbDefaultPackage(m_context->getExpectedCXP());
	}


	//update current CXC and CXP

	m_context->setCurrentCXC(m_context->getExpectedCXC());
	m_context->setCurrentCXP(m_context->getExpectedCXP());

	m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_COMMIT_RECEIVED);

	return result;
}

int FIXS_CCH_IplbUpgrade::IplbStateActivated::cancel ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Cancel on Activated State" << std::endl;
        {
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Cancel on Activated State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	if (!m_context->configureDHCP(true))
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR DHCP" << std::endl;
		return NO_REPLY_FROM_CS;
        }

	int result = m_context->executeRollbackSteps();

        if (m_context->m_exit) return -1;

	if (result == EXECUTED)
	{
		m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_CANCEL_RECEIVED);
	}
	return result;
}

FIXS_CCH_IplbUpgrade::IplbStateActivating::IplbStateActivating()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_ACTIVATING)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Activating Created"<<std::endl;

}
FIXS_CCH_IplbUpgrade::IplbStateActivating::~IplbStateActivating()
{


}

FIXS_CCH_IplbUpgrade::IplbStateFailed::IplbStateFailed()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_FAILED)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Failed Created"<<std::endl;
}


FIXS_CCH_IplbUpgrade::IplbStateFailed::~IplbStateFailed()
{

}
int FIXS_CCH_IplbUpgrade::IplbStateFailed::prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation)
{
	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARE SOFTWARE: " << packageToUpgrade.c_str() <<std::endl;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Cancel on Activated State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	if ((packageToUpgrade == m_context->getCurrentCXP()) || (productToUpgrade == m_context->getCurrentCXC()))
	{
		//return PACKAGE_ALREADY_INSTALLED;
		return SW_ALREADY_ON_BOARD;
	}

	if (m_context->isInitialized() == false)
	{
		//------
		//trace
		{
			std::string magStr;
			(void) CCH_Util::ulongToStringMagazine(m_context->m_magazine, magStr);
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d][magazine:%s][slot:%u][state:FAILED] State Machine not initialized.", __FUNCTION__, __LINE__, magStr.c_str(), m_context->m_slot);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpChar);
			std::cout << "DBG: " << tmpChar << std::endl;
		}
		return INTERNALERR;
	}

	m_context->setExpectedCXP(packageToUpgrade);
	m_context->setExpectedCXC(productToUpgrade);
	cout << "m_expectedCxp:" <<m_context->m_expectedCxp.c_str() << "Installation" << partial_installation << endl ;



	if (m_context->isBoardBlocked() == false)
	{
		std::cout << "DBG: BOARD NOT BLK"  << std::endl;
		if (partial_installation)
		{
			m_context->setUpgradeType(CCH_Util::NORMAL_UPGRADE);
			cout << "DBG: Upgrade " << "NORMAL" ;
			if (m_context->setBootType(flashBootType) == IRONSIDE_FAILURE)
			{
				m_context->setExpectedCXP("");
				m_context->setExpectedCXC("");
				return IRONSIDE_FAILURE;
			}

		}
		else
		{
			m_context->setUpgradeType(CCH_Util::INITIAL_INSTALLATION);
			cout << "DBG: Upgrade " << "INITIAL" ;

			if (m_context->setBootType(networkBootType) == IRONSIDE_FAILURE)
			{
				m_context->setExpectedCXP("");
				m_context->setExpectedCXC("");
				return IRONSIDE_FAILURE;
			}

		}
	}
	else
	{
		m_context->setExpectedCXP("");
		m_context->setExpectedCXC("");
		return IRONSIDE_FAILURE;
	}


	m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_PREPARE_OK);

	return result;
}

FIXS_CCH_IplbUpgrade::IplbStateIdle::IplbStateIdle()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_IDLE)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Idle Created"<<std::endl;
}


FIXS_CCH_IplbUpgrade::IplbStateIdle::~IplbStateIdle()
{

}

int FIXS_CCH_IplbUpgrade::IplbStateIdle::prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation)
{
	int result = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARE SOFTWARE: " << packageToUpgrade.c_str() <<std::endl;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARE SOFTWARE:", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}


	if ((packageToUpgrade == m_context->getCurrentCXP()) || (productToUpgrade == m_context->getCurrentCXC()))
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << ", preliminaryCheckOnPrepare failed." << std::endl;
		std::cout << "packageToUpgrade= " << packageToUpgrade << "m_context->getCurrentCXP ==" << m_context->getCurrentCXP();
		std::cout << "packageToUpgrade= " << productToUpgrade << "m_context->getCurrentCXC ==" << m_context->getCurrentCXC();
		//return PACKAGE_ALREADY_INSTALLED;
		return SW_ALREADY_ON_BOARD;
	}

	if (m_context->isInitialized() == false)
	{
		//------

		//------
		//trace
		{
			std::string magStr;
			(void) CCH_Util::ulongToStringMagazine(m_context->m_magazine, magStr);
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d][magazine:%s][slot:%u][state:FAILED] State Machine not initialized.", __FUNCTION__, __LINE__, magStr.c_str(), m_context->m_slot);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpChar);
			std::cout << "DBG: " << tmpChar << std::endl;
		}

		return INTERNALERR;
	}


	m_context->setExpectedCXP(packageToUpgrade);
	m_context->setExpectedCXC(productToUpgrade);
	cout << "m_expectedCxp:" <<m_context->m_expectedCxp.c_str() << "Installation" << partial_installation << endl ;

	int resval = m_context->isBoardBlocked();	
	if (resval == false)
	{
		std::cout << "DBG: BOARD NOT BLK"  << std::endl;
		if (partial_installation)
		{
			m_context->setUpgradeType(CCH_Util::NORMAL_UPGRADE);
			cout << "DBG: Upgrade " << "NORMAL" ;
			if (m_context->setBootType(flashBootType) == IRONSIDE_FAILURE)
			{
				m_context->setExpectedCXP("");
				m_context->setExpectedCXC("");
				return IRONSIDE_FAILURE;
			}



		}
		else
		{
			m_context->setUpgradeType(CCH_Util::INITIAL_INSTALLATION);
			cout << "DBG: Upgrade " << "INITIAL" ;

			if (m_context->setBootType(networkBootType) == IRONSIDE_FAILURE)
			{
				m_context->setExpectedCXP("");
				m_context->setExpectedCXC("");
				return IRONSIDE_FAILURE;
			}

		}
	}
	else
	{
		m_context->setExpectedCXP("");
		m_context->setExpectedCXC("");
		return resval;
	}


	result = m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_PREPARE_OK);
	return result;
}

FIXS_CCH_IplbUpgrade::IplbStatePrepared::IplbStatePrepared()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_PREPARED)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Prepared Created"<<std::endl;
}

FIXS_CCH_IplbUpgrade::IplbStatePrepared::~IplbStatePrepared()
{

}

int FIXS_CCH_IplbUpgrade::IplbStatePrepared::activateSw()
{
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Activate on Prepared State "<<std::endl;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Activate on Prepared State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	if (m_context->isBoardBlocked())
	{
		return IRONSIDE_FAILURE;
	}

	if (m_context->configureDHCP(false)) //set dhcp to expected values
	{
		//DHCP is configured

		//restart the board so that the new software can be downloaded from the IPLB
		// Setting GprRegister7
		if (m_context->resetBoard() && m_context->resetGprRegister7())
		{
			m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_ACTIVATE_RECEIVED);
		}
		else
		{
			//rollback DHCP
			for (int retry = 0; (retry < 10) && (!m_context->configureDHCP(true)); retry++)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] DHCP Rollback failure", __FUNCTION__, __LINE__);
					if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
					std::cout << "DBG: " << tmpStr << std::endl;
					if(m_context->FIXS_CCH_logging) m_context->FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_ERROR);
				}
				usleep(500000);
			}
			return IRONSIDE_FAILURE;
		}
	}
	else
	{
		//error from CS
		cout << "NO_REPLY_FROM_CS";
		return NO_REPLY_FROM_CS;
	}

	m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_ACTIVATE_RECEIVED);
	return EXECUTED;

}

int FIXS_CCH_IplbUpgrade::IplbStatePrepared::cancel ()
{
	int result = EXECUTED;
	if ((m_context->getUpgradeType()) == CCH_Util::INITIAL_INSTALLATION)
	{
		if (m_context->setBootType(flashBootType) == IRONSIDE_FAILURE)
		{
			return IRONSIDE_FAILURE;
		}
	}
	m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_CANCEL_RECEIVED);
	return result;
}

FIXS_CCH_IplbUpgrade::EventQueue::EventQueue()
: m_queueHandle(-1)
{
	if ((m_queueHandle = eventfd(0, 0)) == -1)
	{
		// failed to create eventlock_
	}
}


FIXS_CCH_IplbUpgrade::EventQueue::~EventQueue()
{

	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}

	if (m_queueHandle)
		::close(m_queueHandle);     // close event handle
}



//## Other Operations (implementation)
FIXS_CCH_IplbUpgrade::IplbEventId  FIXS_CCH_IplbUpgrade::EventQueue::getFirst ()
{
	IplbEventId result = IPLB_DEFAULT_NO_EVENT;
	if (!m_qEventInfo.empty())
	{
		result = m_qEventInfo.front();
	}

	return result;
}

void FIXS_CCH_IplbUpgrade::EventQueue::popData ()
{
	if (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}
}

void FIXS_CCH_IplbUpgrade::EventQueue::pushData (FIXS_CCH_IplbUpgrade::IplbEventId info)
{
	//	 FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
	//Lock lock;

	{
		bool exist = false;

		cout << __FUNCTION__ << "  "<< __LINE__<< "   m_qEventInfo.size() = "<< m_qEventInfo.size() << endl;

		for (unsigned int i=0; i<m_qEventInfo.size();i++)
		{
			//check first element
			FIXS_CCH_IplbUpgrade::IplbEventId first;
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

bool FIXS_CCH_IplbUpgrade::EventQueue::queueEmpty ()
{
	return m_qEventInfo.empty();
}

size_t FIXS_CCH_IplbUpgrade::EventQueue::queueSize ()
{
	return m_qEventInfo.size();
}
void FIXS_CCH_IplbUpgrade::EventQueue::cleanQueue()
{
	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}
}
FIXS_CCH_IplbUpgrade::IplbEventId FIXS_CCH_IplbUpgrade::EventQueue::getItem (int index)
{
	//Lock lock;

	std::list<IplbEventId>::iterator it = m_qEventInfo.begin();
	while (index-- && (it != m_qEventInfo.end())) ++it;

	if (it == m_qEventInfo.end())
		return IPLB_DEFAULT_NO_EVENT;
	else
		return *it;
}

FIXS_CCH_IplbUpgrade::IplbAutomaticRollbackEnded::IplbAutomaticRollbackEnded()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_AUTOMATIC_ROLLBACK_ENDED)
{

}

FIXS_CCH_IplbUpgrade::IplbAutomaticRollbackEnded::~IplbAutomaticRollbackEnded()
{

}

//## Other Operations (implementation)
int FIXS_CCH_IplbUpgrade::IplbAutomaticRollbackEnded::commit (bool setAsDefaultPackage)
{
	UNUSED(setAsDefaultPackage);
	int result = 0;
	m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_COMMIT_RECEIVED);

	return result;

}

FIXS_CCH_IplbUpgrade::IplbAutomaticRollbackOngoing::IplbAutomaticRollbackOngoing()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_AUTOMATIC_ROLLBACK_ONGOING)
{

}
FIXS_CCH_IplbUpgrade::IplbAutomaticRollbackOngoing::~IplbAutomaticRollbackOngoing()
{

}

FIXS_CCH_IplbUpgrade::IplbManualRollbackEnded::IplbManualRollbackEnded()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_MANUAL_ROLLBACK_ENDED)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State ManualRollbackEnd Created"<<std::endl;
}


FIXS_CCH_IplbUpgrade::IplbManualRollbackEnded::~IplbManualRollbackEnded()
{

}

int FIXS_CCH_IplbUpgrade::IplbManualRollbackEnded::commit (bool setAsDefaultPackage)
{
	UNUSED(setAsDefaultPackage);
	int result = 0;
	m_context->setEvent(FIXS_CCH_IplbUpgrade::IPLB_COMMIT_RECEIVED);

	return result;
}

FIXS_CCH_IplbUpgrade::IplbManualRollbackOngoing::IplbManualRollbackOngoing()
:FIXS_CCH_IplbUpgrade::IplbState(FIXS_CCH_IplbUpgrade::STATE_MANUAL_ROLLBACK_ONGOING)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State ManualRollbackOngoing Created"<<std::endl;
}


FIXS_CCH_IplbUpgrade::IplbManualRollbackOngoing::~IplbManualRollbackOngoing()
{

}

FIXS_CCH_IplbUpgrade::FIXS_CCH_IplbUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string /*iplbAddress_EthA*/, std::string /*iplbAddress_EthB*/,std::string dn)
:	m_magazine(magazine),
 	m_slot(slot),
 	m_ironsideManager(0),
 	m_running(false),
 	m_lastErrorCode(0),
 	m_boardId(boardId),
	m_path(""),
 	m_upgradeStatus(0),
 	m_eventQueue(),
 	m_currentCxp(installedPackage),
 	//m_fullUpgradeType(false),
 	m_expectedCxp(""),
 	m_Object_DN(dn),
 	m_previousState(0),
 	m_failureCode(0),
 	m_initialized(false),
 	m_gprRegister7resetted(false),
 	m_egem2environment(false),
	m_forcedUpgrade(false),
	m_upgradeType(false)

{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_idle.setContext(this);
	m_prepared.setContext(this);
	m_activating.setContext(this);
	m_activated.setContext(this);
	m_failed.setContext(this);
	m_automaticRollbackOngoing.setContext(this);
	m_automaticRollbackEnded.setContext(this);
	m_manualRollbackOngoing.setContext(this);
	m_manualRollbackEnded.setContext(this);
	m_currentState = &m_idle;

	m_lastSwUpgradeReason = CCH_Util::SW_RP_EXECUTED;
	m_lastUpgradeDate = "";
	m_lastUpgradeTime = "";
	m_lastSwUpgradeResult = CCH_Util::SW_RP_OK;

	traceObj = new ACS_TRA_trace("FIXS_CCH_IplbUpgrade");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	//define Report Progress DN
	IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot,m_SwInventory_Object_DN);

	dn_blade_persistance =  IMM_Interface::getDnBladePersistance(m_magazine,m_slot);

	/*//m_neighborSCXIP = "";
	if (!IMM_Interface::getNeighborScxIp(m_Object_DN,m_slot,m_neighborSCXIPA, m_neighborSCXIPB)) {m_neighborSCXIPA = "";m_neighborSCXIPB = "";}*/

	m_currentCxc.clear();
	m_currentCxc = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(installedPackage);
	m_previousCxp = m_currentCxp;
	//unsigned char cSlot = (unsigned char)this->m_slot;
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

	uint32_t dmxc_add1,dmxc_add2;
        FIXS_CCH_UpgradeManager::getInstance()->getDmxAddress(dmxc_add1, dmxc_add2);

	m_ironsideManager = new FIXS_CCH_IronsideManager(dmxc_add1, dmxc_add2);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Current State: "<< getStateName()<<std::endl;

	//printMachineInfo(m_magazine,m_slot);
	m_cmdEvent = 0;
	m_shutdownEvent = 0;
}


FIXS_CCH_IplbUpgrade::~FIXS_CCH_IplbUpgrade()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	if (m_cmdEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_cmdEvent);
	}

	if (m_ironsideManager) delete m_ironsideManager;


	FIXS_CCH_logging = 0;
}

int FIXS_CCH_IplbUpgrade::activateSw()
{
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);
	int result = m_currentState->activateSw();
	return result;
}

int FIXS_CCH_IplbUpgrade::commit (bool setAsDefaultPackage)
{
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);
	int result = m_currentState->commit(setAsDefaultPackage);
	return result;
}

int FIXS_CCH_IplbUpgrade::prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation)
{
	//define Report Progress DN
	IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot,m_SwInventory_Object_DN);
	//copy value of current package
	m_previousCxp = m_currentCxp;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << (partial_installation ? " NORMAL UPGRADE" : " INITIAL INSTALLATION") << std::endl;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] NORMAL UPGRADE: %s  :  INITIAL INSTALLATION: %s ", __FUNCTION__, __LINE__,packageToUpgrade.c_str(), productToUpgrade.c_str());
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
	}
	int result = m_currentState->prepare(packageToUpgrade, productToUpgrade, partial_installation);
	return result;
}

int FIXS_CCH_IplbUpgrade::setEvent (FIXS_CCH_IplbUpgrade::IplbEventId event)
{
	std::cout<< __FUNCTION__ << "@" << __LINE__ << std::endl;
	//send event info to the state machine thread
	//EventInfo* qEventInfo = new EventInfo(event, container);
	m_eventQueue.pushData(event);
	return 0;
}
int FIXS_CCH_IplbUpgrade::open (void *args)
{
	int result = 0;
	UNUSED(args);
	cout << "\n STATE MACHINE Thread Activate... " << endl;

	{//log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"IPLB UPGRADE Starting thread on instance [%p]. \n",this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}

	//	 activate(THR_DETACHED);
	m_exit = false;
	activate();

	return result;
}
int FIXS_CCH_IplbUpgrade::svc ()
{
	int result = 0;
	m_running = true;
	printMachineInfo(m_magazine, m_slot, getStateName());
	//m_exit = false;
	//------
	//trace
	{
		std::string magStr;
		(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d][magazine:%s][slot:%u][state:%s]STATE MACHINE STARTED", __FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}

	initOnStart();
	if (m_exit)
	{
	m_running =false;
	return result;
	}
	cout << "its here";

	onRestart();
	if (m_exit)
	{
	m_running =false;
	return result;
	}
	//main loop
	EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent, m_cmdEvent};

	const int WAIT_FAILED=-1;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;
	const int Shutdown_Event_Index=0;
	const int Command_Event_Index=1;

	while ( ! m_exit ) // Set to true when the thread shall exit
	{
		fflush(stdout);

		// Wait for shutdown and command events
		int returnValue=FIXS_CCH_Event::WaitForEvents(Number_Of_Events, handleArr, 600000);

		switch (returnValue)
		{
		case WAIT_FAILED:   // Event failed
		{
			std::cout << "WAIT_FAILED" << std::endl;
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
		case WAIT_TIMEOUT:	// Time out
		{
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
			std::cout << __FUNCTION__ << "@" << __LINE__ << " IplbUpgrade Shutdown_Event_Index..." << std::endl;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Received a shutdown event", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}
			m_exit = true;    // Terminate the thread
			sleep(1);
			break;
				}

		case (WAIT_OBJECT_0 + Command_Event_Index):    // Command ready to be handled
				{
			printMachineInfo(m_magazine, m_slot, getStateName());

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IplbUpgrade Command_Event_Index => handleCommandEvent()", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}
			FIXS_CCH_Event::ResetEvent(m_cmdEvent);

			handleCommandEvent();
			break;
				}

		default:
		{

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}

			break;
		}
		} // End of switch

	} // End of the while loop

	//trace
	{
		std::string magStr;
		(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d][magazine:%s][slot:%u][state:%s]STATE MACHINE STOPPED", __FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_running = false;

	//clean
	resetEvents();

	return result;
}

void FIXS_CCH_IplbUpgrade::stop ()
{

	m_exit = true;
	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"IPLB UPGRADE Stopping thread on instance [%p]. \n",this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_WARN);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}
}

void FIXS_CCH_IplbUpgrade::handleCommandEvent ()
{
	//if (m_eventQueue.queueEmpty()) return;


	std::cout << "DBG: " << __FUNCTION__ << "in handleCommandEvent" << __LINE__ << std::endl;

	while (!m_eventQueue.queueEmpty()) {

		//get event to analyze
		FIXS_CCH_IplbUpgrade::IplbEventId qEventInfo = IPLB_DEFAULT_NO_EVENT;
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
		//If the new state is one of PREPARING, ACTIVATING, ROLLBACK_ONGOING
		//it needs to start the appropriate operations
		switch (m_currentState->getStateId())
		{
		case STATE_ACTIVATING:
			onActivating();//currentState->stateHandling();
			break;

		case STATE_AUTOMATIC_ROLLBACK_ONGOING:
			onAutomaticRollback();//currentState->stateHandling();
			break;

		case STATE_MANUAL_ROLLBACK_ONGOING:
			onManualRollback();//currentState->stateHandling();
			break;

		default: break;

		}
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] After switchState ", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
	}
}

void FIXS_CCH_IplbUpgrade::initOnStart ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "M_EXIT VALUE:" << (m_exit ? "true" : "false" )<< std::endl;
	if (m_exit) return;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	//-----------------------
	// load environment type
	CCH_Util::EnvironmentType environment = FIXS_CCH_DiskHandler::getEnvironment();
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "environment:" << environment<< std::endl;
	if((environment == CCH_Util::MULTIPLECP_CBA) || (environment == CCH_Util::SINGLECP_CBA)|| (environment == CCH_Util::MULTIPLECP_DMX))
	{
		m_egem2environment = true;
	}

	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] m_egem2environment is set to: %s", __FUNCTION__, __LINE__, (m_egem2environment ? "true" : "false"));
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,tmpStr);
	}

	//get command event
	if (m_exit) return;
	m_cmdEvent =0;

	if (m_cmdEvent == 0)
		m_cmdEvent = m_eventQueue.getHandle();

	//---------------------------
	//create shutdown event
	//if (m_exit) return;

	if (m_shutdownEvent == 0)
                m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
		

        if (m_exit) return;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_shutdownEvent event created" << std::endl;

	//------------------------------------------------
	//do not execute these steps again in case thread
	//is started, stopped and started again
	if (!m_initialized)
	{
		//get command event
		if (m_cmdEvent == 0)
			m_cmdEvent = m_eventQueue.getHandle();
		if (!m_cmdEvent || m_cmdEvent == -1) std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ERROR: command event is not ok" << std::endl;

		{ //SNMP
			/*while ( !init_snmp  && (m_exit == false))
			{
				//--------
				//trace
				{
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] error initializing m_snmpManager. last error code: %u", __FUNCTION__, __LINE__,ACE_OS::last_error());
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
					if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,traceChar);
				}
				usleep(500000);
			}*/
		}
	}

	m_initialized = true;


}

void FIXS_CCH_IplbUpgrade::onRestart ()
{
	cout << "Inside onRestart" <<endl;
	switch (getStateId())
	{
	case FIXS_CCH_IplbUpgrade::STATE_PREPARED:
		onPreparedOrActivated(true);
		break;
	case FIXS_CCH_IplbUpgrade::STATE_ACTIVATING:
		onActivating();
		break;
	case FIXS_CCH_IplbUpgrade::STATE_AUTOMATIC_ROLLBACK_ONGOING:
		onAutomaticRollback();
		break;
	case FIXS_CCH_IplbUpgrade::STATE_MANUAL_ROLLBACK_ONGOING:
		onManualRollback();
		break;
	case FIXS_CCH_IplbUpgrade::STATE_ACTIVATED:
		onPreparedOrActivated(false);
		break;
	default:
		break;
	}

}
// This method is used when activate/cancel action is issued and cch service is stopped immediately before current state is moved
// from prepared/activated to activating/manual rollback ongoing. This method helps to revert back the changes done (related to usedby field
// updation/current product) after prepared/activated and before service is stopped
void FIXS_CCH_IplbUpgrade::onPreparedOrActivated(bool rollback)
{
		if(rollback)
		{
		 	if (strcmp(m_previousCxp.c_str(),m_expectedCxp.c_str()) == 0 )//proper prepared case
			{	//configure dhcp
				configureDHCP(true);
				m_previousCxp = m_currentCxp;
			}
		}
		else
		{
			if (strcmp(m_previousCxp.c_str(),m_currentCxp.c_str()) == 0 )//proper activated case
			{//configure dhcp
				configureDHCP(false);
			}
		}

}
void FIXS_CCH_IplbUpgrade::switchState (FIXS_CCH_IplbUpgrade::IplbEventId eventId)
{

	std::cout << "DBG: " << __FUNCTION__ << "in Switch state" << __LINE__ << std::endl;

	std::string t_currState("");
	std::string t_nextState("");
	std::string t_event("");
	std::string t_type("");
	int status = 0;
	//std::string lastResult;
	//std::string lastReason;

	t_currState = getStateName();

	switch(eventId)
	{
	case IPLB_PREPARE_OK:
		t_event = "IPLB_PREPARE_OK";
		break;

	case IPLB_ACTIVATE_RECEIVED:
		t_event = "IPLB_ACTIVATE_RECEIVED";
		break;

	case IPLB_COMMIT_RECEIVED:
		t_event = "IPLB_COMMIT_RECEIVED";
		break;

	case IPLB_CANCEL_RECEIVED:
		t_event = "IPLB_CANCEL_RECEIVED";
		break;

	case IPLB_ACTIVATE_OK:
		t_event = "IPLB_ACTIVATE_OK";
		break;

	case IPLB_COMMIT_OK:
		t_event = "IPLB_COMMIT_OK";
		break;

	case IPLB_FAILURE_RECEIVED:
		t_event = "IPLB_FAILURE_RECEIVED";
		break;

	case IPLB_MANUAL_ROLLBACK_OK:
		t_event = "IPLB_MANUAL_ROLLBACK_OK";
		break;

	case IPLB_DEFAULT_NO_EVENT:
		t_event = "IPLB_DEFAULT_NO_EVENT";
		break;

	case IPLB_AUTOMATIC_ROLLBACK_OK:
		t_event = "IPLB_AUTOMATIC_ROLLBACK_OK";
		break;

	default:
		t_event = "UNKNOWN_EVENT";
	}

	//this check is based on current state and eventID


	std::cout << "DBG: " << "current state" << m_currentState->getStateId() <<"line" << __LINE__ << std::endl;
	switch(m_currentState->getStateId())
	{
	case STATE_FAILED:
	case STATE_IDLE:
		switch(eventId)
		{
		case IPLB_PREPARE_OK:
			m_currentState = &m_prepared;
			status=CCH_Util::SW_ERR_OK;
			CCH_Util::getCurrentTime(m_lastUpgradeDate, m_lastUpgradeTime);
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

		case STATE_PREPARED:
			switch(eventId)
			{
			case IPLB_ACTIVATE_RECEIVED:
				m_currentState = &m_activating;
				status=CCH_Util::SW_ERR_OK;
				modifySwInventory(status);
				break;

			case IPLB_CANCEL_RECEIVED:
				m_previousState = getStateId();
				m_currentState = &m_manualRollbackEnded;
				status=CCH_Util::SW_ERR_CANCELLED;

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
				case IPLB_ACTIVATE_OK:
					m_currentState = &m_activated;
					status=CCH_Util::SW_ERR_OK;
					modifySwInventory(status);
					break;

				case IPLB_FAILURE_RECEIVED:
					m_previousState = getStateId();
					m_currentState = &m_automaticRollbackOngoing;
                                        status=CCH_Util::SW_ERR_OK;
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
					case IPLB_COMMIT_RECEIVED:
						m_currentState = &m_idle;
						status = CCH_Util::SW_ERR_OK;
						//update current sw into the map
						FIXS_CCH_UpgradeManager::getInstance()->setCurrentProduct(m_magazine,m_slot,m_currentCxp);
						m_previousCxp=getExpectedCXP();
						setExpectedCXP("");
						setExpectedCXC("");

						modifySwInventory(status);
						FIXS_CCH_UpgradeManager::getInstance()->checkIPLBswVr();
						stop();//stop thread
						break;

					case IPLB_CANCEL_RECEIVED:
						m_previousState = getStateId();
						m_currentState = &m_manualRollbackOngoing;
						status = CCH_Util::SW_ERR_OK;
						setExpectedCXP("");
						setExpectedCXC("");
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
						case IPLB_MANUAL_ROLLBACK_OK:
							m_currentState = &m_manualRollbackEnded;
							status = CCH_Util::SW_ERR_CANCELLED;
							setExpectedCXP("");
							setExpectedCXC("");
							modifySwInventory(status);
							break;

						case IPLB_FAILURE_RECEIVED:
							m_currentState = &m_failed;
							//m_currentState = &m_idle;
							status = CCH_Util::SW_ERR_MANUAL_FAILURE;
							setExpectedCXP("");
							setExpectedCXC("");
							modifySwInventory(status);
							stop();//stop thread

							break;

						default:
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " MANUAL_ROLLBACK_ONGOING: ILLEGAL EVENT == > ROLLBACK_ONGOING "<<std::endl;
							if (traceObj->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] MANUAL_ROLLBACK_ONGOING: ILLEGAL EVENT == > ROLLBACK_ONGOING ", __FUNCTION__, __LINE__);
								traceObj->ACS_TRA_event(1,tmpStr);
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
							}
							break;
						}
						break;

						case STATE_MANUAL_ROLLBACK_ENDED:
							switch(eventId)
							{
							case IPLB_COMMIT_RECEIVED:
								m_currentState = &m_idle;
								status = SW_ERR_CANCELLED;
						                FIXS_CCH_UpgradeManager::getInstance()->setCurrentProduct(m_magazine,m_slot,m_currentCxp);
								setExpectedCXP("");
								setExpectedCXC("");
								modifySwInventory(status);
								stop();//stop thread

								break;

							default:
								std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " MANUAL_ROLLBACK_END: ILLEGAL EVENT == > MANUAL_ROLLBACK_END "<<std::endl;
								if (traceObj->ACS_TRA_ON())
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] MANUAL_ROLLBACK_END: ILLEGAL EVENT == > MANUAL_ROLLBACK_END  ", __FUNCTION__, __LINE__);
									traceObj->ACS_TRA_event(1,tmpStr);
									if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
								}
								break;
							}
							break;

							case STATE_AUTOMATIC_ROLLBACK_ONGOING:
								switch(eventId)
								{
								case IPLB_AUTOMATIC_ROLLBACK_OK:
									m_currentState = &m_automaticRollbackEnded;
									status = m_lastErrorCode;
									setExpectedCXP("");
									setExpectedCXC("");
									modifySwInventory(status);

									break;

								case IPLB_FAILURE_RECEIVED:
									m_currentState = &m_failed;
									// m_currentState = &m_idle;
                                    status = CCH_Util::SW_ERR_AUTO_FAILURE;
									setExpectedCXP("");
									setExpectedCXC("");
									modifySwInventory(status);

									stop();//stop thread

									break;

								default:
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " STATE_AUTOMATIC_ROLLBACK_ONGOING: ILLEGAL EVENT == > AUTOMATIC_FALLBACK_ONGOING "<<std::endl;
									if (traceObj->ACS_TRA_ON())
									{
										char tmpStr[512] = {0};
										snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] STATE_AUTOMATIC_ROLLBACK_ONGOING: ILLEGAL EVENT == > AUTOMATIC_FALLBACK_ONGOING ", __FUNCTION__, __LINE__);
										traceObj->ACS_TRA_event(1,tmpStr);
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
									}
									break;
								}
								break;

								case STATE_AUTOMATIC_ROLLBACK_ENDED:
									switch(eventId)
									{
									case IPLB_COMMIT_RECEIVED:
										m_currentState = &m_idle;
										status = m_lastErrorCode;
						                                FIXS_CCH_UpgradeManager::getInstance()->setCurrentProduct(m_magazine,m_slot,m_currentCxp);
										setExpectedCXP("");
										setExpectedCXC("");
										modifySwInventory(status);
										stop();//stop thread

										break;

									default:
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " STATE_AUTOMATIC_ROLLBACK_ENDED: ILLEGAL EVENT == > AUTOMATIC_FALLBACK_END "<<std::endl;
										if (traceObj->ACS_TRA_ON())
										{
											char tmpStr[512] = {0};
											snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] STATE_AUTOMATIC_ROLLBACK_ENDED: ILLEGAL EVENT == > AUTOMATIC_FALLBACK_END ", __FUNCTION__, __LINE__);
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

	{ // log
		t_nextState=getStateName();

		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1,"[IPLB UPGRADE of %s on MAG:%s, SLOT:%u] CURRENT STATE: %s, EVENT RECEIVED: %s, NEXT STATE: %s",
				t_type.c_str(), magStr.c_str(), m_slot,	t_currState.c_str(), t_event.c_str(), t_nextState.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;

	}
	saveStatus();
}

void FIXS_CCH_IplbUpgrade::saveStatus ()
{
	CCH_Util::SWData mydata;
	getOverallStatus(mydata);
}
int FIXS_CCH_IplbUpgrade::cancel ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);
	int result = m_currentState->cancel();
	return result;
}

std::string FIXS_CCH_IplbUpgrade::getStateName ()
{
	std::string t_currState="";
	switch(m_currentState->getStateId())
	{

	case STATE_IDLE:
		t_currState = "IDLE";
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

	case STATE_AUTOMATIC_ROLLBACK_ONGOING:
		t_currState = "AUTOMATIC ROLLBACK ONGOING";
		break;

	case STATE_AUTOMATIC_ROLLBACK_ENDED:
		t_currState = "AUTOMATIC ROLLBACK END";
		break;

	case STATE_MANUAL_ROLLBACK_ONGOING:
		t_currState = "MANUAL ROLLBACK ONGOING";
		break;

	case STATE_MANUAL_ROLLBACK_ENDED:
		t_currState = "MANUAL ROLLBACK END";
		break;

	case STATE_UNDEFINED:
		t_currState = "UNDEFINED";
		break;

	default:
		t_currState = "UNKNOWN_STATE";
	}

	return t_currState;
}

void FIXS_CCH_IplbUpgrade::setPath (std::string completePathIplb)
{
	m_path=completePathIplb;
}

FIXS_CCH_IplbUpgrade::IplbStateId FIXS_CCH_IplbUpgrade::getStateId () const
{
	return m_currentState->getStateId();
}

bool FIXS_CCH_IplbUpgrade::configureDHCP (bool rollback)
{

	bool retValue = false;

	//-------------------------
	//set package to configure
	std::string package;
	if (rollback)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_currentCxp == " << m_currentCxp.c_str() << std::endl;
		package = m_currentCxp;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_expectedCxp == " << m_expectedCxp.c_str() << std::endl;
		package = m_expectedCxp;
	}


	//------
	//trace
	{
		char tmpChar[512] = {0};
		if (rollback)
		{
			snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d] Rolling back DHCP Configuration with CXP package Id = %s.",
					__FUNCTION__, __LINE__, package.c_str());
		}
		else
		{
			snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d] Configuring DHCP with CXP package Id = %s",
					__FUNCTION__, __LINE__, package.c_str());
		}

		if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1, tmpChar);
		std::cout << "DBG: " << tmpChar << std::endl;
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
	}
	std::string magazine("");
	ACS_CS_API_SET_NS::CS_API_Set_Result result;
	CCH_Util::ulongToStringMagazine(m_magazine,magazine);
	//---------------
	//configure dhcp
	for (int retry = 0; ((retry < 7) && (retValue == false)); retry++)
	{

		result = ACS_CS_API_Set::setSoftwareVersion(package,magazine,m_slot);
		switch (result)
		{
		case ACS_CS_API_SET_NS::Result_Success:
			retValue = true;
			if(rollback)
				IMM_Interface::updateUsedPackage(m_magazine,m_slot,getExpectedCXP(),m_currentCxp);
			else
				IMM_Interface::updateUsedPackage(m_magazine,m_slot,m_previousCxp,getExpectedCXP());

			break;
		default:
		{
			{
				//trace
				std::string magStr;
				(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,
						"[%s@%d] ACS_CS_API_Set::setSoftwareVersion failed setting package '%s' for IPLB on magazine %s and slot %u. CS Error Code: %u",
						__FUNCTION__, __LINE__, package.c_str(), magStr.c_str(), m_slot, result);
				if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1, tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
			}
			retValue = false;
			usleep(300000);
		}
		break;
		}
	}


	return retValue;
}

void FIXS_CCH_IplbUpgrade::resetEvents ()
{
	FIXS_CCH_Event::ResetEvent(m_cmdEvent);
	m_eventQueue.cleanQueue();
}


bool FIXS_CCH_IplbUpgrade::resetBoard ()
{
	int retValue = EXECUTED;
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade:IPLB Board reset is Invoked",LOG_LEVEL_DEBUG);

	retValue = m_ironsideManager->reset_board(m_magazine,m_slot);
	if(retValue)
	{
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade : IPLB Board reset failed ",LOG_LEVEL_DEBUG);
		return false;
	}
	
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade : IPLB Board reset is Successfull ",LOG_LEVEL_DEBUG);
	return true;
}

int FIXS_CCH_IplbUpgrade::onActivating ()
{
	//bool loop = true;
	//Wait for GPR data update
	cout << "ACTIVATING inside OnActivating" << endl;
	int result = checkUpgradeStatus();
	if (m_exit) return -1;

	if (result == EXECUTED)
		setEvent(FIXS_CCH_IplbUpgrade::IPLB_ACTIVATE_OK);
	else
	{
		//------
		//trace
		{
			std::string magStr;
			(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d][magazine:%s][slot:%u][state:%s] Check of Upgrade Status failed: triggering automatic rollback",
					__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
			std::cout << "DBG: " << tmpChar << std::endl;
		}

		setEvent(FIXS_CCH_IplbUpgrade::IPLB_FAILURE_RECEIVED);
	}

	return result;
}

int FIXS_CCH_IplbUpgrade::onPreparing ()
{
	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARING STATE: start preparing..."<<std::endl;
	//sleep(5);
	setEvent(FIXS_CCH_IplbUpgrade::IPLB_PREPARE_OK);
	return result;
}

void FIXS_CCH_IplbUpgrade::getOverallStatus (CCH_Util::SWData &iplbdata)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ << " In" << std::endl;

	iplbdata.expectedCxp = m_expectedCxp;
	iplbdata.expectedCxc = m_expectedCxc;
	iplbdata.currentCxc = m_currentCxc;
	iplbdata.currentCxp = m_currentCxp;
	iplbdata.lastUpgradeDate = m_lastUpgradeDate;
	iplbdata.lastUpgradeTime = m_lastUpgradeTime;
	iplbdata.lastUpgradeResult = m_lastSwUpgradeResult;
	iplbdata.lastUpgradeReason = m_lastSwUpgradeReason;
iplbdata.dnBladePersistance = dn_blade_persistance;
	//TODO: used m_lastErrorCode in switchState need to confirm for the usage.
	/*iplbdata.lastErrorCode = m_lastErrorCode;
	iplbdata.lastUpgradeType = (m_fullUpgradeType ? 1 : 0); //(m_fullUpgradeType ? "IPLB_FULL_UPG" : "IPLB_NORM_UPG");
	iplbdata.previousState = m_previousState;
	iplbdata.failureCode = m_failureCode;*/
	iplbdata.forcedUpgrade = m_forcedUpgrade;
        iplbdata.upgradeType   = static_cast<CCH_Util::BoardUpgradeType>(m_upgradeType);

	switch(m_currentState->getStateId())
	{

	case STATE_IDLE:
		iplbdata.upgradeStatus = CCH_Util::SW_IDLE;
		break;

	case STATE_PREPARED:
		iplbdata.upgradeStatus = CCH_Util::SW_PREPARED;
		break;

	case STATE_ACTIVATING:
		iplbdata.upgradeStatus = CCH_Util::SW_ACTIVATING;
		break;

	case STATE_ACTIVATED:
		iplbdata.upgradeStatus = CCH_Util::SW_ACTIVATED;
		break;

	case STATE_FAILED:
		iplbdata.upgradeStatus = CCH_Util::SW_FAILED;
		break;

	case STATE_AUTOMATIC_ROLLBACK_ONGOING:
		iplbdata.upgradeStatus = CCH_Util::SW_AUTOMATIC_ROLLBACK_ONGOING;
		break;

	case STATE_AUTOMATIC_ROLLBACK_ENDED:
		iplbdata.upgradeStatus = CCH_Util::SW_AUTOMATIC_ROLLBACK_ENDED;
		break;

	case STATE_MANUAL_ROLLBACK_ONGOING:
		iplbdata.upgradeStatus = CCH_Util::SW_MANUAL_ROLLBACK_ONGOING;
		break;

	case STATE_MANUAL_ROLLBACK_ENDED:
		iplbdata.upgradeStatus = CCH_Util::SW_MANUAL_ROLLBACK_ENDED;
		break;

	default:
		iplbdata.upgradeStatus = CCH_Util::SW_FAILED;
	}


	std::cout << __FUNCTION__ << "@" << __LINE__ << " Out" << std::endl;
}

void FIXS_CCH_IplbUpgrade::restoreStatus (CCH_Util::SWData &iplbData)
{

	
	m_expectedCxp = iplbData.expectedCxp;
	m_expectedCxc = iplbData.expectedCxc;
	m_currentCxc = iplbData.currentCxc;
	m_currentCxp = iplbData.currentCxp;
	m_lastUpgradeDate = iplbData.lastUpgradeDate;
	m_lastUpgradeTime = iplbData.lastUpgradeTime;
	m_lastSwUpgradeResult = iplbData.lastUpgradeResult;
	m_lastSwUpgradeReason = iplbData.lastUpgradeReason;
	//m_previousState = iplbData.previousState;
	//m_failureCode = iplbData.failureCode;

	m_forcedUpgrade = iplbData.forcedUpgrade;
        m_upgradeType = iplbData.upgradeType;

	/*if(boardSWData.lastUpgradeType == 1) //if(boardSWData.lastUpgradeType == "IPLB_FULL_UPG")
	{
		m_fullUpradeType = true;
	}
	else
	{
		m_fullUpradeType = false;
	}*/

	switch ( iplbData.upgradeStatus )
	{
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

	case CCH_Util::SW_AUTOMATIC_ROLLBACK_ONGOING:
		m_currentState = &m_automaticRollbackOngoing;
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

	//------
	//trace
	{
		std::string magStr;
		(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
		char tmpChar[512] = {0};
		snprintf(tmpChar, sizeof(tmpChar) - 1,
				"[%s@%d]STATE MACHINE RESTORED AFTER RESTART:\n"
				"\t------------------------------------\n"
				"\tmagazine:\t%s\n"
				"\tslot:\t%u\n"
				"\tstate:\t%s\n"
				"\tResult:\t%d\n"
				"\tReason:\t%d\n"
				"\tDate:\t%s\n"
				"\tTime:\t%s\n"
				"\tCurrent CXC:\t%s\n"
				"\tCurrent CXP:\t%s\n"
				"\tExpected CXC:\t%s\n"
				"\tExpected CXP:\t%s\n"
				
				"\tforcedUpgrade :\t%d\n"
				"\tUpgradeType :\t%d\n"
				"\t------------------------------------\n",
				__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), m_lastSwUpgradeResult,
				m_lastSwUpgradeReason, m_lastUpgradeDate.c_str(), m_lastUpgradeTime.c_str(),
				m_currentCxc.c_str(), m_currentCxp.c_str(), m_expectedCxc.c_str(), m_expectedCxp.c_str(),m_forcedUpgrade,m_upgradeType);

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
		std::cout << "DBG: " << tmpChar << std::endl;
	}
}




int FIXS_CCH_IplbUpgrade::onAutomaticRollback ()
{
     int result;
     int returnValue;
     int flag;   
   
     const int WAIT_FAILED=-1;
     const int WAIT_TIMEOUT= -2;
     const int WAIT_SHUTDOWN=0;

  if((!configureDHCP(true))){   

     while(!m_exit)
     {
                returnValue=FIXS_CCH_Event::WaitForEvents(1,&m_shutdownEvent, 60000);
                flag =false;

                switch (returnValue)
                {
                case WAIT_FAILED:   // Event failed
                {
                        std::cout << "WAIT_FAILED" << std::endl;
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
                case WAIT_TIMEOUT:      // Time out
                {
                    if(configureDHCP(true)) { 
                       flag = true;  
                    } 
                        break;
                }
                case WAIT_SHUTDOWN:  // Shutdown event
                {
                        std::cout << __FUNCTION__ << "@" << __LINE__ << " IplbUpgrade Shutdown_Event_Index..." << std::endl;
                        if (traceObj->ACS_TRA_ON())
                        {
                                char tmpStr[512] = {0};
                                snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Received a shutdown event", __FUNCTION__, __LINE__);
                                traceObj->ACS_TRA_event(1,tmpStr);
                                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
                        }
                        m_exit = true;    // Terminate the thread
                        sleep(1);
                        break;
                }
                default:
                {

                        if (traceObj->ACS_TRA_ON())
                        {
                                char tmpStr[512] = {0};
                                snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
                                traceObj->ACS_TRA_event(1,tmpStr);
                                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
                        }

                        break;
                }
                } // End of switch
         if (flag) break;

        } // End of the while loop
     }
   
        if (m_exit) return -1;

        if (executeRollbackSteps(true)!= EXECUTED){
            
           setEvent(FIXS_CCH_IplbUpgrade::IPLB_FAILURE_RECEIVED);
           return -1; 
        }
 
	//wait untill the board restarts and set the GPR result value
	result = checkUpgradeStatus();

        if (m_exit) return -1;

	if (result == EXECUTED)
	{
		setEvent(FIXS_CCH_IplbUpgrade::IPLB_AUTOMATIC_ROLLBACK_OK);
	}
	else
	{
		setEvent(FIXS_CCH_IplbUpgrade::IPLB_FAILURE_RECEIVED);
	}

	return result;
}

int FIXS_CCH_IplbUpgrade::onManualRollback ()
{
	int result = 0;

	result = checkUpgradeStatus();
         
        if (m_exit) return -1;
          
	if (result == EXECUTED)
	{

		setEvent(FIXS_CCH_IplbUpgrade::IPLB_MANUAL_ROLLBACK_OK);
	}
	else
	{
		result = INTERNALERR;
		setEvent(FIXS_CCH_IplbUpgrade::IPLB_FAILURE_RECEIVED);
	}

	return result;
}

int FIXS_CCH_IplbUpgrade::checkUpgradeStatus ()
{
	 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("Inside checkUpgradeStatus .......",LOG_LEVEL_INFO);
	int result = INTERNALERR;
	bool loop = true;
	std::string output = "";
	std::string GPRDataValue = "";
	const int WAIT_TIMEOUT= 0;
	ACE_Time_Value timeValue;
	timeValue.set(120); // Wait for 2 min
	//timeValue.set(60);
	struct pollfd singlePollfd[1];
	singlePollfd[0].fd = m_shutdownEvent;
	singlePollfd[0].events = POLLRDHUP | POLLHUP | POLLERR;
	singlePollfd[0].revents = 0;
	std::string success="255.255.255.255.0.0.255.255";
	cout << "Reset Register kkkkk " << endl;
	if (!m_gprRegister7resetted)
	{
		cout << "Reset Register checkkkkk" << endl;
		ACE_OS::poll (singlePollfd, 1, &timeValue);
		if (m_exit) return -1;
	}
	m_gprRegister7resetted = false;
	std::string slotString;
	CCH_Util::ushortToString((unsigned short)m_slot, slotString);
	std::string gprId = CCH_Util::intToString(512);
	std::string gprId7 = CCH_Util::intToString(263);	
	for (int retry = 0 ; (loop && (retry < 150)); retry++)
	{
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("Inside for loop .......",LOG_LEVEL_INFO);
		ACE_Time_Value singleSelectTime;
		singleSelectTime.set(10);
		//WaitForSingleObject(m_shutdownEvent, 60000);//Wait for 2 minutes
		int singlewaitResult = ACE_OS::poll (singlePollfd, 1, &singleSelectTime);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " singlewaitResult = " << singlewaitResult << std::endl;
		std::string str("512");

		switch (singlewaitResult)
		{
		case WAIT_TIMEOUT:
		{
                                 
			/*if (!m_ironsideManager->getGprData(GPRDataValue,m_magazine,m_slot,gprId))
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " getGprDataValue read of RAM register 0 is ok for slot " << m_slot << " output = " << GPRDataValue << " " << GPRDataValue[17] << " " << GPRDataValue[18] << std::endl;
				int dataValue = getIthbyteOfGprData(GPRDataValue,5);
				if (dataValue == 10)  //IPLB sets the  flash boot
				{
					cout << "Flash boot" << endl;*/
					if(m_exit)
					return -1;
					std::string output = "";
					if(!m_ironsideManager->getGprData(output,m_magazine,m_slot,gprId7))
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " getGprDataValue read of RAM register 7 is ok for slot " << m_slot << " output = " << output.c_str() << std::endl;
						//xxxxffff where x can be anything from 0-9,a-f   means success
						std::string successBytes = "";
						std::cout << __FUNCTION__ << "@" << __LINE__ << " The output.size() value " <<output.size()<< std::endl;
						if(!output.compare(success))
						{
							loop = false;
							
							//------
							//trace
							{
								std::string magStr;
								(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
								char tmpChar[512] = {0};
								snprintf(tmpChar, sizeof(tmpChar) - 1,
										"[%s@%d][magazine:%s][slot:%u][state:%s] Boot Type (OID.slot.512) is 'flash boot'and status(OID.slot.263) is success(%s)",
										__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), output.c_str());
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
								if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
								std::cout << "DBG: " << tmpChar << std::endl;
							}
							result = EXECUTED;

						}
						else
						{
							int res = getIthbyteOfGprData(output,7);
							if(res > 15 && res < 32	)
							{
								//upgrade failed
								loop = false;
								//------
								//trace
								{
									std::string magStr;
									(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
									char tmpChar[512] = {0};
									snprintf(tmpChar, sizeof(tmpChar) - 1,
											"[%s@%d][magazine:%s][slot:%u][state:%s] IPLB triggered the upgrade failure . GPR RAM register 7: %s",
											__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), output.c_str());
									if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
									if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
									std::cout << "DBG: " << tmpChar << std::endl;
								}

							}
							//------
							//trace
							{
								std::string magStr;
								(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
								char tmpChar[512] = {0};
								snprintf(tmpChar, sizeof(tmpChar) - 1,
										"[%s@%d][magazine:%s][slot:%u][state:%s] GPR RAM register 7 does not contain success value: %s",
										__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), output.c_str());
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
								if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
								std::cout << "DBG: " << tmpChar << std::endl;
							}





						}

					}
					else  //gprData reading failed
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: getGprDataValue read of RAM register 7 is not ok for slot " << m_slot << std::endl;
						//------
						//trace
						{
							std::string magStr;
							(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
							char tmpChar[512] = {0};
							snprintf(tmpChar, sizeof(tmpChar) - 1,
									"[%s@%d][magazine:%s][slot:%u][state:%s] Boot Type is set to flash boot, but cannot access to GPR RAM register 7 ",
									__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
							if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
							std::cout << "DBG: " << tmpChar << std::endl;
						}



					}

				/*}
				else
				{

					cout << "NETWORK BOOT" << endl;

					//NVRAM Register 0 (.512) is still set to Netwok boot
					//------
					//trace
					{
						std::string magStr;
						(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
						char tmpChar[512] = {0};
						snprintf(tmpChar, sizeof(tmpChar) - 1,
								"[%s@%d][magazine:%s][slot:%u][state:%s][Network Boot Phase Ongoing]\n\t OID.slot.512 which contains the Boot Type is not set to flash boot. Value: %s\n",
								__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), GPRDataValue.c_str());
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
						if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
						std::cout << "DBG: " << tmpChar << std::endl;
					}

					//check that there's no error
					output = "";
					if(!m_ironsideManager->getGprData(output,m_magazine,m_slot,gprId7))
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " getGprDataValue read of RAM register 7 is ok for slot " << m_slot << " output = " << output.c_str() << std::endl;

						int res = getIthbyteOfGprData(output,7);
						if(res > 15 && res < 32 )
						{
							//upgrade failed
							loop = false;
							//------
							//trace
							{
								std::string magStr;
								(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
								char tmpChar[512] = {0};
								snprintf(tmpChar, sizeof(tmpChar) - 1,
										"[%s@%d][magazine:%s][slot:%u][state:%s] IPLB triggered the upgrade failure . GPR RAM register 7: %s",
										__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), output.c_str());
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
								if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
								std::cout << "DBG: " << tmpChar << std::endl;
							}

						}

					}

				}


			}*/
		}

		break;
		default:
			loop = false;
			return -1;
		}
	}


	return result;
}


int FIXS_CCH_IplbUpgrade::setBootType (int bootType)
{
	int result = EXECUTED;
	int retValue;
	if(!bootType)
	{
		retValue = m_ironsideManager->setBootType(m_magazine,m_slot,IMM_Util::FLASH_BOOT);
	}
	else
	{
		retValue = m_ironsideManager->setBootType(m_magazine,m_slot,IMM_Util::NETWORK_BOOT);

	}
	if(retValue)
	{
		std::cout << "setting GprData failed";
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade : Setting GprData failed ",LOG_LEVEL_WARN);
		return IRONSIDE_FAILURE;
	}
	std::cout << "Setting bootType Success";
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade : Setting BootType Success ",LOG_LEVEL_DEBUG);
	return result;
}

/*void FIXS_CCH_IplbUpgrade::setResultAndReason (CCH_Util::LmErrors status)
{
	int lastResult=0, lastReason=0;
	CCH_Util::getResultReason(lastResult,lastReason,status);
	setLastUpgradeResult(lastResult);
	setLastUpgradeReason(lastReason);
}*/

int FIXS_CCH_IplbUpgrade::executeRollbackSteps(bool isAutoRollback)
{
	int result = EXECUTED;

	if (this->isBoardBlocked())
	{
           
	    std::cout << __FUNCTION__ << "@" << __LINE__ << " BOARD BLOCKED" << std::endl;
	    return IRONSIDE_FAILURE;
	}

	std::string slotString;
	CCH_Util::ushortToString((unsigned short)m_slot, slotString);

	if ((getUpgradeType())== CCH_Util:: INITIAL_INSTALLATION) //set  GPR VALUE to network boot (in ACTIVATED it is set to flash boot)
	{
		if (m_ironsideManager->setBootType(m_magazine,m_slot,IMM_Util::NETWORK_BOOT))
		{
			//success
			//------
			//trace
			{
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] SetBootType - NETWORK_BOOT - success", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
		}
		else
		{
			//trace
			{
				char tmpChar[512] = {0};
				 snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] SetBootType - NETWORK_BOOT - failed", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
		}
	}
	else
	{		
		//tr HV10563
		//If the NORMAL UPGRADE  fails(especially for the case IPLB installation gets stuck and timeout occurs)
		//we most likely will not have a working IPLB software on the board which then can handle a roll back with a flash boot so need to set NETWORK_BOOT
		if(isAutoRollback)
		{
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " Setting network boot in this case" << std::endl;
			m_ironsideManager->setBootType(m_magazine,m_slot,IMM_Util::NETWORK_BOOT);			
		}
		else
		{
			m_ironsideManager->setBootType(m_magazine,m_slot,IMM_Util::FLASH_BOOT);
		}
	}

	(void) resetGprRegister7();

	//-----------------
	//NORMAL UPGRADE
	//-----------------
	//Reset Board
	if (resetBoard())
	{
		return EXECUTED;
	}
	else
	{
		return IRONSIDE_FAILURE;
	}
	return result;
}

bool FIXS_CCH_IplbUpgrade::resetGprRegister7 ()
{
	m_gprRegister7resetted = false;
	int retValue = EXECUTED;

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade : Setting GprRegister7 ",LOG_LEVEL_DEBUG);
	if ((getUpgradeType())== CCH_Util::INITIAL_INSTALLATION)
	{
		retValue = m_ironsideManager->setUpgradeType(m_magazine,m_slot,IMM_Util::INITIAL_INSTALLATION);
	}
	else
	{
		retValue = m_ironsideManager->setUpgradeType(m_magazine,m_slot,IMM_Util::NORMAL_UPGRADE);
	}
	if(retValue)
	{
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade : Setting GprRegister7 failed ",LOG_LEVEL_WARN);
		return false;
	}

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade : Setting BootType Success ",LOG_LEVEL_DEBUG);
	m_gprRegister7resetted = true;
	return true;
}

int FIXS_CCH_IplbUpgrade::isBoardBlocked ()
{
	int retValue ;

	std::cout << "checking if the IPLB board is blocked";
	if(FIXS_CCH_logging) FIXS_CCH_logging->Write("FIXS_CCH_IplbUpgrade :isBoardBlocked ",LOG_LEVEL_DEBUG);
	retValue = m_ironsideManager->isBoardblocked(m_magazine,m_slot);
	if(retValue)
	{
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(" IPLB Board is blocked ",LOG_LEVEL_DEBUG);
		return retValue;
	}

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("IPLB Board is not Blocked ",LOG_LEVEL_DEBUG);
	return retValue;
}

void FIXS_CCH_IplbUpgrade::setObjectDN(const char* dn)
{
	m_Object_DN = dn;
}

std::string FIXS_CCH_IplbUpgrade::getUpgradedCXP()
{
	std::string dn = m_SwInventory_Object_DN;
	char * cstr = new char [dn.length()+1];
	std::string result[8];
	strcpy (cstr, dn.c_str());
	char *pch = strtok(cstr,",");
	int i=0;
	while (pch != NULL)
	{
		result[i] = std::string(pch);
		pch = strtok (NULL, ",");
		i++;
	}
	delete [] cstr;
	char * str = const_cast<char *>(strstr(result[1].c_str(),"="));
	std::string cxps("");

	if(str == NULL)
		return cxps;

	std::string cxp(str+1);
	cout << "EXPECTED CXP" << str+1 << " = " << cxp;
	return cxp;

}


void FIXS_CCH_IplbUpgrade::setSwInvObjectDN(std::string dn)
{
	//m_SwInventory_Object_DN = dn.c_str();
	m_SwInventory_Object_DN = dn;
}

std::string FIXS_CCH_IplbUpgrade::getSwInvObjectDN()
{
	return m_SwInventory_Object_DN;
}

std::string FIXS_CCH_IplbUpgrade::getSwPercistanceDN()
{
	return dn_blade_persistance;
}


bool FIXS_CCH_IplbUpgrade::modifySwInventory(int state)
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
	currSw = ACS_APGCC::after(m_currentCxc,"IPLB_");

	std::cout <<"\n setting state : "<< currState << std::endl;
	
	CCH_Util::SWData data;
	data.lastUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(lastReason);
	if (!currSw.empty())  data.currentCxc = currSw;
	else data.currentCxc = m_currentCxc;

	data.expectedCxc = m_expectedCxc;
	data.lastUpgradeDate = m_lastUpgradeDate;
	data.lastErrorCode = state;
	data.lastUpgradeTime = m_lastUpgradeTime;
	data.lastUpgradeResult =  static_cast<CCH_Util::RpUpgradeResult>(lastResult);
	data.swState =currState ;
	data.dnBladePersistance = dn_blade_persistance;
	//data.lastUpgradeType = (m_fullUpradeType ? 1 : 0); //(m_fullUpradeType ? "IPLB_FULL_UPG" : "IPLB_NORM_UPG");
	//data.previousState = m_previousState;
	//data.failureCode = m_failureCode;

	data.forcedUpgrade = m_forcedUpgrade;
	data.upgradeType   = static_cast<CCH_Util::BoardUpgradeType>(m_upgradeType);
	if(!IMM_Interface::modifyAllInventoryAttributes(m_SwInventory_Object_DN,data)) return false;
	else
	{
		std::cout <<"\n ---------------------- ATTRIBUTE CHANGED DURING UPGRADE -------------------------- "<< std::endl;
		return true;
	}
	return true; //remove this after
}

bool FIXS_CCH_IplbUpgrade::initSwInventory()
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
	/*data.lastUpgradeType = (m_fullUpradeType ? 1 : 0); //(m_fullUpradeType ? "IPLB_FULL_UPG" : "IPLB_NORM_UPG");
	 data.previousState = m_previousState;
	 data.failureCode = m_failureCode;*/
	 data.swState = currState;

	 data.forcedUpgrade = m_forcedUpgrade;
         data.upgradeType   = static_cast<CCH_Util::BoardUpgradeType>(m_upgradeType);

	 if(!IMM_Interface::modifyAllInventoryAttributes(m_SwInventory_Object_DN,data)) return false;
	 else
	 {
		 std::cout <<"\n ---------------------- ATTRIBUTE CHANGED DURING UPGRADE -------------------------- "<< std::endl;
		 return true;
	 }
	return true; //remove this after
}

int FIXS_CCH_IplbUpgrade::getStateImm ()
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
		break;
	}
	return t_currState;
}

std::string  FIXS_CCH_IplbUpgrade::getIplbFileName( std::string package)
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

int FIXS_CCH_IplbUpgrade::getIthbyteOfGprData(std::string gprData,int index)
{

	std::string str = gprData.substr(1,gprData.size()-1);
	if(index > 8)
		return -1;

	std::string result[8];
	char * cstr = new char [str.length()+1];
	strcpy (cstr, str.c_str());
	char *pch = strtok(cstr,".");
	int i=0;
	while (pch != NULL)
	{
		result[i] = std::string(pch);
		pch = strtok (NULL, ".");
		i++;
	}
	int res;
	CCH_Util::stringToInt(result[index-1],res);
	std::cout << "Integer Value " << res;
	delete [] cstr;
	return res;
}


void FIXS_CCH_IplbUpgrade::setResultAndReason(int result,int reason)
{

            std::string magStr;
            std::string slotStr;
            std::string dn_obj("");                                        

            (void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
            (void) CCH_Util::ulongToStringMagazine(m_slot, slotStr);  
 

            if (IMM_Interface::getReportProgressObject(magStr,slotStr,dn_obj))
            {
                std::string dateSw("");
                std::string timeSw("");
                int resultSw = result;
                int reasonSw = reason;
                CCH_Util::getCurrentTime(dateSw, timeSw);
                CCH_Util::trim(timeSw);

                //result
                ACS_CC_ImmParameter SWresult = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_RESULT,ATTR_INT32T,&resultSw,1);
                IMM_Util::modify_OM_ImmAttr(dn_obj.c_str(),SWresult);

                //reason
                ACS_CC_ImmParameter SWreason = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_REASON,ATTR_INT32T,&reasonSw,1);
                IMM_Util::modify_OM_ImmAttr(dn_obj.c_str(),SWreason);

                //time
                std::string lTime = dateSw + " " + timeSw;
                ACS_CC_ImmParameter SWtime = IMM_Util::defineParameterString(IMM_Util::ATT_SWINVENTORY_TIME,ATTR_STRINGT,lTime.c_str(),1);
                IMM_Util::modify_OM_ImmAttr(dn_obj.c_str(),SWtime);

//                delete [] SWresult.attrValues;
//                delete [] SWreason.attrValues;
//                delete [] SWtime.attrValues;
            }

}    
