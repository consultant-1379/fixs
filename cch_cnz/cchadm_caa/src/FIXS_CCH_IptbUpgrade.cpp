
// FIXS_CCH_IptbUpgrade
#include "FIXS_CCH_IptbUpgrade.h"
//## begin module%4D2DC69C010E.additionalDeclarations preserve=yes
namespace
{
		void printMachineInfo(unsigned long magazine, unsigned short slot,std::string state) {
		std::string value;
		char info[256] = {0};
		CCH_Util::ulongToStringMagazine(magazine, value);
		snprintf(info, sizeof(info) - 1, "magazine: %s, slot: %u,\nSTATE: %s", value.c_str(), slot,state.c_str());

		std::cout << "    IPT UPGRADE      " << std::endl;
		std::cout << "         _           " << std::endl;
		std::cout << "         ||___       " << std::endl;
		std::cout << "         | ||~:__    " << std::endl;
		std::cout << "  _______| |__|  |   " << std::endl;
		std::cout << " |_______________|   " << std::endl;
		std::cout << "   ((_)     ((_)     " << std::endl;
		std::cout << "STATE MACHINE FOR " << info << std::endl;

	};

	// The indexes of events used by IptbUpgrade thread
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
	//DWORD Safe_Timeout = 120000;
	//DWORD TWENTY_MINUTES = 1200;
	//int MAX_RETRY = 3;

	std::string gprDataValues [29] = {
							"00000111","00000121","00000122","00000123","00000124","00000131",
							"00000141","00000142","00000151","00000161","00000162","00000163",
							"00000164","00000165","00000166","00000171","00000172","00000173",
							"0000017d","0000017e","0000017f","00000181","00000182","00000183",
							"00000191","00000192","00000193","000001a1","0000ffff" };
}

FIXS_CCH_IptbUpgrade::IptbState::IptbState()
      : m_context(0)
{
	m_id = FIXS_CCH_IptbUpgrade::STATE_UNDEFINED;
	m_set_status = false;
}

FIXS_CCH_IptbUpgrade::IptbState::IptbState (FIXS_CCH_IptbUpgrade::IptbStateId stateId)
      : m_context(0)
{
	m_id = stateId;
	m_set_status = false;
}


FIXS_CCH_IptbUpgrade::IptbState::~IptbState()
{

}

void FIXS_CCH_IptbUpgrade::IptbState::setContext (FIXS_CCH_IptbUpgrade* context)
{
	m_context = context;
}

FIXS_CCH_IptbUpgrade::IptbStateId FIXS_CCH_IptbUpgrade::IptbState::getStateId () const
{
	return m_id;
}

int FIXS_CCH_IptbUpgrade::IptbState::activateSw ()
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

int FIXS_CCH_IptbUpgrade::IptbState::commit (bool setAsDefaultPackage)
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

int FIXS_CCH_IptbUpgrade::IptbState::prepare (std::string packageToUpgrade, std::string productToUpgrade,int upgradeType)
{
	UNUSED(packageToUpgrade);
	UNUSED(productToUpgrade);
	UNUSED(upgradeType);
     
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

int FIXS_CCH_IptbUpgrade::IptbState::cancel ()
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

FIXS_CCH_IptbUpgrade::IptbStateActivated::IptbStateActivated()
  :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_ACTIVATED)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Activated Created"<<std::endl;
}

FIXS_CCH_IptbUpgrade::IptbStateActivated::~IptbStateActivated()
{

}

int FIXS_CCH_IptbUpgrade::IptbStateActivated::commit (bool setAsDefaultPackage)
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
		return SNMP_FAILURE;
	}
	/*if (setAsDefaultPackage) ///design  base
	{
		if (FIXS_CCH_SoftwareMap::getInstance()->updateNewDefaultPackage(m_context->getExpectedCXP(), CCH_Util::IPTB) == false)
		{
			return NO_REPLY_FROM_CS;
		}
	}*/
	if (setAsDefaultPackage) //APG43L
	{
		FIXS_CCH_SoftwareMap::getInstance()->setIptDefaultPackage(m_context->getExpectedCXP());
	}


	//update current CXC and CXP

	m_context->setCurrentCXC(m_context->getExpectedCXC());
	m_context->setCurrentCXP(m_context->getExpectedCXP());

//	m_context->setExpectedCXC();//reset expected cxc
//	m_context->setExpectedCXP();//reset expected cxp

	m_context->setEvent(FIXS_CCH_IptbUpgrade::IPT_COMMIT_RECEIVED);

	return result;
}

int FIXS_CCH_IptbUpgrade::IptbStateActivated::cancel ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Cancel on Activated State" << std::endl;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Cancel on Activated State", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	if (m_context->isBoardBlocked())
	{
		return SNMP_FAILURE;
	}

	if (!m_context->resetGprRegister7())
	{
		return SNMP_FAILURE;
	}

	int result = m_context->executeBasicStepsBeforeRollingBack();
	if (result == EXECUTED)
	{
		m_context->setEvent(FIXS_CCH_IptbUpgrade::IPT_CANCEL_RECEIVED);
	}

	return result;
}

FIXS_CCH_IptbUpgrade::IptbStateActivating::IptbStateActivating()
   :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_ACTIVATING)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Activating Created"<<std::endl;
}


FIXS_CCH_IptbUpgrade::IptbStateActivating::~IptbStateActivating()
{

}

FIXS_CCH_IptbUpgrade::IptbStateFailed::IptbStateFailed()
   :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_FAILED)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Failed Created"<<std::endl;
}


FIXS_CCH_IptbUpgrade::IptbStateFailed::~IptbStateFailed()
{

}
int FIXS_CCH_IptbUpgrade::IptbStateFailed::prepare (std::string packageToUpgrade, std::string productToUpgrade,int upgradeType)
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

	if (m_context->isBoardBlocked() == false)
	{
		m_context->setExpectedCXP(packageToUpgrade);
		m_context->setExpectedCXC(productToUpgrade);

		if (upgradeType == CCH_Util::NORMAL_UPGRADE)
		{
			m_context->setUpgradeType(upgradeType);
                        if (m_context->setBootType(flashBootType) == SNMP_FAILURE)
                        {
                           return SNMP_FAILURE; 
                        }
		}
		else if (upgradeType == CCH_Util::INITIAL_INSTALLATION) 
		{
			m_context->setUpgradeType(upgradeType);

			if (m_context->setBootType(networkBootType) == SNMP_FAILURE)
			{
				m_context->setExpectedCXP("");
				m_context->setExpectedCXC("");
				return SNMP_FAILURE;
			}
		}
	}
	else
	{
		//SNMP Error or Board Blocked
		m_context->setExpectedCXP("");
		m_context->setExpectedCXC("");
		return SNMP_FAILURE;
	}

	m_context->setEvent(FIXS_CCH_IptbUpgrade::IPT_PREPARE_OK);

	return result;
}

FIXS_CCH_IptbUpgrade::IptbStateIdle::IptbStateIdle()
  :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_IDLE)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Idle Created"<<std::endl;
}


FIXS_CCH_IptbUpgrade::IptbStateIdle::~IptbStateIdle()
{

}

int FIXS_CCH_IptbUpgrade::IptbStateIdle::prepare (std::string packageToUpgrade, std::string productToUpgrade,int upgradeType )
{
	int result = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARE SOFTWARE: " << packageToUpgrade.c_str() <<std::endl;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PREPARE SOFTWARE:", __FUNCTION__, __LINE__);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_context->getCurrentCXP(): " << m_context->getCurrentCXP().c_str() <<std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_context->getCurrentCXC(): " << m_context->getCurrentCXC().c_str() <<std::endl;
	if ((packageToUpgrade == m_context->getCurrentCXP()) || (productToUpgrade == m_context->getCurrentCXC()))
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << ", preliminaryCheckOnPrepare failed." << std::endl;
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
				"[%s@%d][magazine:%s][slot:%u][state:IDLE] State Machine not initialized.", __FUNCTION__, __LINE__, magStr.c_str(), m_context->m_slot);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
			if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpChar);
			std::cout << "DBG: " << tmpChar << std::endl;
		}
		return INTERNALERR;
	}
	m_context->setExpectedCXP(packageToUpgrade);
	m_context->setExpectedCXC(productToUpgrade);
	cout << "m_expectedCxp:" <<m_context->m_expectedCxp.c_str() << endl;

	if (m_context->isBoardBlocked() == false)
	{
		std::cout << "DBG: BOARD NOT BLK"  << std::endl;
		if (upgradeType == IMM_Util::NORMAL_UPGRADE)
		{
			m_context->setUpgradeType(upgradeType);
                        if (m_context->setBootType(flashBootType) == SNMP_FAILURE)
                        {
				return SNMP_FAILURE;
                        }
		}
		else if (upgradeType == IMM_Util::INITIAL_INSTALLATION)
		{
			m_context->setUpgradeType(upgradeType);

			if (m_context->setBootType(networkBootType) == SNMP_FAILURE)
			{
				m_context->setExpectedCXP("");
				m_context->setExpectedCXC("");
				return SNMP_FAILURE;
			}
		}
	}
	else
	{
		//SNMP Error or Board Blocked
		m_context->setExpectedCXP("");
		m_context->setExpectedCXC("");
		return SNMP_FAILURE;
	}
	result = m_context->setEvent(FIXS_CCH_IptbUpgrade::IPT_PREPARE_OK);
	return result;
}

FIXS_CCH_IptbUpgrade::IptbStatePrepared::IptbStatePrepared()
  :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_PREPARED)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State Prepared Created"<<std::endl;
}

FIXS_CCH_IptbUpgrade::IptbStatePrepared::~IptbStatePrepared()
{

}

int FIXS_CCH_IptbUpgrade::IptbStatePrepared::activateSw ()
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
		return SNMP_FAILURE;
	}

	if (m_context->configureDHCP(false)) //set dhcp to expected values
	{
		//DHCP is configured

		//restart the board so that the new software can be downloaded from the IPTB
		//set xshmcBoardReset in xshmcBoardTable
		if (m_context->resetBoard() && m_context->resetGprRegister7())
		{
			//NTP settings not needed with latest version if IPTB software
			//updateNtpSettings(m_context->m_iptAddress_EthA, m_context->m_iptAddress_EthB);
			m_context->setEvent(FIXS_CCH_IptbUpgrade::IPT_ACTIVATE_RECEIVED);
		}
		else
		{
			//rollback DHCP
			for (int retry = 0; (retry < 10) && (!m_context->configureDHCP(true)); retry++)
			{
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] DHCP Rollback failure", __FUNCTION__, __LINE__);
					if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);

				}
				usleep(50000);
			}
			return SNMP_FAILURE;
		}
	}
	else
	{
		//error from CS
		return NO_REPLY_FROM_CS;
	}

	return EXECUTED;
}

 int FIXS_CCH_IptbUpgrade::IptbStatePrepared::cancel ()
{
	int result = EXECUTED;

	if (m_context->getUpgradeType()== CCH_Util::INITIAL_INSTALLATION)
	{
		if (m_context->setBootType(flashBootType) == SNMP_FAILURE)
		{
			return SNMP_FAILURE;
		}
	}
	m_context->setEvent(FIXS_CCH_IptbUpgrade::IPT_CANCEL_RECEIVED);
	return result;
}

FIXS_CCH_IptbUpgrade::EventQueue::EventQueue()
	: m_queueHandle(-1)
 {
	 if ((m_queueHandle = eventfd(0, 0)) == -1)
	 {
		 // failed to create eventlock_
	 }
 }


FIXS_CCH_IptbUpgrade::EventQueue::~EventQueue()
{

	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}

	if (m_queueHandle)
		::close(m_queueHandle);     // close event handle
}



//## Other Operations (implementation)
FIXS_CCH_IptbUpgrade::IptbEventId  FIXS_CCH_IptbUpgrade::EventQueue::getFirst ()
{
	IptbEventId result = IPT_DEFAULT_NO_EVENT;
	if (!m_qEventInfo.empty())
	{
		result = m_qEventInfo.front();
	}

	return result;
}

 void FIXS_CCH_IptbUpgrade::EventQueue::popData ()
{
	 if (!m_qEventInfo.empty()) {
		 m_qEventInfo.pop_front();
		 //if ((m_qEventInfo.size() == 0) && ((m_queueHandle != -1) && (m_queueHandle != 0))) //ResetEvent(m_queueHandle);	FIXS_CCH_Event::ResetEvent(m_queueHandle);
	 }
}

 void FIXS_CCH_IptbUpgrade::EventQueue::pushData (FIXS_CCH_IptbUpgrade::IptbEventId info)
{
	 //	 FIXS_CCH_CriticalSectionGuard eventLock(m_queueCriticalSection);
	 //Lock lock;

	 {
		 bool exist = false;

		 cout << __FUNCTION__ << "  "<< __LINE__<< "   m_qEventInfo.size() = "<< m_qEventInfo.size() << endl;

		 for (unsigned int i=0; i<m_qEventInfo.size();i++)
		 {
			 //check first element
			 FIXS_CCH_IptbUpgrade::IptbEventId first;
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

bool FIXS_CCH_IptbUpgrade::EventQueue::queueEmpty ()
{
	return m_qEventInfo.empty();
}

size_t FIXS_CCH_IptbUpgrade::EventQueue::queueSize ()
{
	 return m_qEventInfo.size();
}
void FIXS_CCH_IptbUpgrade::EventQueue::cleanQueue()
{
	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}
}
FIXS_CCH_IptbUpgrade::IptbEventId FIXS_CCH_IptbUpgrade::EventQueue::getItem (int index)
{
	//Lock lock;

	std::list<IptbEventId>::iterator it = m_qEventInfo.begin();
	while (index-- && (it != m_qEventInfo.end())) ++it;

	if (it == m_qEventInfo.end())
		return IPT_DEFAULT_NO_EVENT;
	else
		return *it;
}

FIXS_CCH_IptbUpgrade::IptbAutomaticRollbackEnded::IptbAutomaticRollbackEnded()
   :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_AUTOMATIC_ROLLBACK_ENDED)
{

}

FIXS_CCH_IptbUpgrade::IptbAutomaticRollbackEnded::~IptbAutomaticRollbackEnded()
{

}

//## Other Operations (implementation)
int FIXS_CCH_IptbUpgrade::IptbAutomaticRollbackEnded::commit (bool setAsDefaultPackage)
{
	UNUSED(setAsDefaultPackage);
	int result = 0;
	m_context->setEvent(FIXS_CCH_IptbUpgrade::IPT_COMMIT_RECEIVED);

	return result;

}

FIXS_CCH_IptbUpgrade::IptbAutomaticRollbackOngoing::IptbAutomaticRollbackOngoing()
    :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_AUTOMATIC_ROLLBACK_ONGOING)
{

}
FIXS_CCH_IptbUpgrade::IptbAutomaticRollbackOngoing::~IptbAutomaticRollbackOngoing()
{

}

FIXS_CCH_IptbUpgrade::IptbManualRollbackEnded::IptbManualRollbackEnded()
    :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_MANUAL_ROLLBACK_ENDED)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State ManualRollbackEnd Created"<<std::endl;
}


FIXS_CCH_IptbUpgrade::IptbManualRollbackEnded::~IptbManualRollbackEnded()
{

}

int FIXS_CCH_IptbUpgrade::IptbManualRollbackEnded::commit (bool setAsDefaultPackage)
{
	UNUSED(setAsDefaultPackage);
	int result = 0;
	m_context->setEvent(FIXS_CCH_IptbUpgrade::IPT_COMMIT_RECEIVED);

	return result;
}

FIXS_CCH_IptbUpgrade::IptbManualRollbackOngoing::IptbManualRollbackOngoing()
   :FIXS_CCH_IptbUpgrade::IptbState(FIXS_CCH_IptbUpgrade::STATE_MANUAL_ROLLBACK_ONGOING)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " State ManualRollbackOngoing Created"<<std::endl;
}


FIXS_CCH_IptbUpgrade::IptbManualRollbackOngoing::~IptbManualRollbackOngoing()
{

}

FIXS_CCH_IptbUpgrade::FIXS_CCH_IptbUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string switch0_IP_EthA, std::string switch0_IP_EthB, std::string switch25_IP_EthA, std::string switch25_IP_EthB, std::string iptAddress_EthA, std::string iptAddress_EthB,std::string dn)
      : m_magazine(magazine), 
	m_slot(slot),
	m_iptAddress_EthA(iptAddress_EthA),
	m_iptAddress_EthB(iptAddress_EthB),
	m_snmpManager(0),
	m_running(false),
	m_lastErrorCode(0),
	m_boardId(boardId),
	m_path(""),
	m_upgradeStatus(0),
	m_eventQueue(),
	m_switch0_IP_EthA(switch0_IP_EthA),
	m_switch0_IP_EthB(switch0_IP_EthB),
	m_switch25_IP_EthA(switch25_IP_EthA),
	m_switch25_IP_EthB(switch25_IP_EthB),
	m_currentCxp(installedPackage),
	//m_fullUpradeType(false),
	m_expectedCxp(""),
	m_Object_DN(dn),
        m_switch_Master_IP(""),
        m_previousState(0),
	m_failureCode(0),
	m_initialized(false),
	m_gprRegister7resetted(false),
	m_egem2environment(false),
        m_forcedUpgrade(false),
        m_upgradeType(true)	
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

	traceObj = new ACS_TRA_trace("FIXS_CCH_IptbUpgrade");

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
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
		traceObj->ACS_TRA_event(1,tmpStr);
	}
	m_switch_Master_IP = m_switch0_IP_EthA;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_switch_Master_IP);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Current State: "<< getStateName()<<std::endl;

	//printMachineInfo(m_magazine,m_slot);
	m_cmdEvent = 0;
	m_shutdownEvent = 0;
}


FIXS_CCH_IptbUpgrade::~FIXS_CCH_IptbUpgrade()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	if (traceObj){
		delete(traceObj);
		traceObj = NULL;
	}

	if (m_cmdEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_cmdEvent);
	}
//	if (m_shutdownEvent)
//	{
//		FIXS_CCH_Event::CloseEvent(m_shutdownEvent);
//	}
	if (m_snmpManager) delete m_snmpManager;

	FIXS_CCH_logging = 0;
}

int FIXS_CCH_IptbUpgrade::activateSw ()
{
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);
	int result = m_currentState->activateSw();
	return result;
}

int FIXS_CCH_IptbUpgrade::commit (bool setAsDefaultPackage)
{
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);
	int result = m_currentState->commit(setAsDefaultPackage);
	return result;
}

int FIXS_CCH_IptbUpgrade::prepare (std::string packageToUpgrade, std::string productToUpgrade,int upgradeType)
{
	//define Report Progress DN
	IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot,m_SwInventory_Object_DN);
	//copy value of current package
	//m_previousCxp = m_currentCxp;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << (upgradeType ? " NORMAL INSTALLATION" : " INITIAL INSTALLATION") << std::endl;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] NORMAL INSTALLATION: %s  : INITIAL INSTALLATION: %s ", __FUNCTION__, __LINE__,packageToUpgrade.c_str(), productToUpgrade.c_str());
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
	}
	int result = m_currentState->prepare(packageToUpgrade, productToUpgrade, upgradeType);
	return result;
	//## end FIXS_CCH_IptbUpgrade::prepare%4D2DDEB40121.body
}

int FIXS_CCH_IptbUpgrade::setEvent (FIXS_CCH_IptbUpgrade::IptbEventId event)
{
	std::cout<< __FUNCTION__ << "@" << __LINE__ << std::endl;
	//send event info to the state machine thread
	//EventInfo* qEventInfo = new EventInfo(event, container);
	m_eventQueue.pushData(event);
	return 0;
}
int FIXS_CCH_IptbUpgrade::open (void *args)
{
	int result = 0;
	UNUSED(args);
	cout << "\n STATE MACHINE Thread Activate... " << endl;

	{//log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"IPT UPGRADE Starting thread on instance [%p]. \n",this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}

	//	 activate(THR_DETACHED);
        m_exit = false; 
	activate();
	return result;
}
int FIXS_CCH_IptbUpgrade::svc ()
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
			std::cout << __FUNCTION__ << "@" << __LINE__ << " IptbUpgrade Shutdown_Event_Index..." << std::endl;
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
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IptbUpgrade Command_Event_Index => handleCommandEvent()", __FUNCTION__, __LINE__);
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

 void FIXS_CCH_IptbUpgrade::stop ()
{
	m_exit = true;
	{//log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"IPT UPGRADE Stopping thread on instance [%p]. \n",this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_WARN);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}
}

void FIXS_CCH_IptbUpgrade::handleCommandEvent ()
{
	 //if (m_eventQueue.queueEmpty()) return;

	 while (!m_eventQueue.queueEmpty()) {

		 //get event to analyze
		 FIXS_CCH_IptbUpgrade::IptbEventId qEventInfo = IPT_DEFAULT_NO_EVENT;
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

 void FIXS_CCH_IptbUpgrade::initOnStart ()
{
  //## begin FIXS_CCH_IptbUpgrade::initOnStart%4D2F225B016D.body preserve=yes
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "M_EXIT VALUE:" << (m_exit ? "true" : "false" )<< std::endl;
	if (m_exit) return;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	//-----------------------
	// load environment type
	CCH_Util::EnvironmentType environment = FIXS_CCH_DiskHandler::getEnvironment();
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "environment:" << environment<< std::endl;
	if((environment == CCH_Util::MULTIPLECP_CBA) || (environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_SMX) || (environment == CCH_Util::SINGLECP_SMX)) //TR_HW29936
	{
		m_egem2environment = true;
	}
	//m_egem2environment = ( (environment == CCH_Util::MULTIPLECP_CBA) || (environment == CCH_Util::SINGLECP_CBA) );

	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] m_egem2environment is set to: %s", __FUNCTION__, __LINE__, (m_egem2environment ? "true" : "false"));
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,tmpStr);
	}

	//get command event
		if (m_exit) return;

		if (m_cmdEvent == 0)
			m_cmdEvent = m_eventQueue.getHandle();
/*	//---------------------------
	//destroy the shutdown event
	if (m_shutdownEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_shutdownEvent);
	}*/


	//---------------------------
	//create shutdown event
	if (m_exit) return;

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

		/*	{ //SNMP
			while ( !init_snmp  && (m_exit == false))
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
			}
		}*/
	}

	m_initialized = true;

	if (getStateId() == FIXS_CCH_IptbUpgrade::STATE_ACTIVATING || getStateId() == FIXS_CCH_IptbUpgrade::STATE_AUTOMATIC_ROLLBACK_ONGOING || getStateId() == FIXS_CCH_IptbUpgrade::STATE_MANUAL_ROLLBACK_ONGOING)
	{ //Read Mastership
		while ( (checkMasterScbrp() == false) && (m_exit == false) )
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Cannot read the master SCB-RP. last error code: %u",__FUNCTION__, __LINE__, ACE_OS::last_error());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,tmpStr);
			}
			usleep(500000);
		}
	}

}

 void FIXS_CCH_IptbUpgrade::onRestart ()
{
  	switch (getStateId())
	{
		case FIXS_CCH_IptbUpgrade::STATE_ACTIVATING:
			onActivating();
			break;
		case FIXS_CCH_IptbUpgrade::STATE_AUTOMATIC_ROLLBACK_ONGOING:
			onAutomaticRollback();
			break;
		case FIXS_CCH_IptbUpgrade::STATE_MANUAL_ROLLBACK_ONGOING:
			onManualRollback();
			break;
		default:
			break;
	}

}

 void FIXS_CCH_IptbUpgrade::switchState (FIXS_CCH_IptbUpgrade::IptbEventId eventId)
 {

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
	 case IPT_PREPARE_OK:
		 t_event = "IPT_PREPARE_OK";
		 break;

	 case IPT_ACTIVATE_RECEIVED:
		 t_event = "IPT_ACTIVATE_RECEIVED";
		 break;

	 case IPT_COMMIT_RECEIVED:
		 t_event = "IPT_COMMIT_RECEIVED";
		 break;

	 case IPT_CANCEL_RECEIVED:
		 t_event = "IPT_CANCEL_RECEIVED";
		 break;

	 case IPT_ACTIVATE_OK:
		 t_event = "IPT_ACTIVATE_OK";
		 break;

	 case IPT_COMMIT_OK:
		 t_event = "IPT_COMMIT_OK";
		 break;

	 case IPT_FAILURE_RECEIVED:
		 t_event = "IPT_FAILURE_RECEIVED";
		 break;

	 case IPT_MANUAL_ROLLBACK_OK:
		 t_event = "IPT_MANUAL_ROLLBACK_OK";
		 break;

	 case IPT_DEFAULT_NO_EVENT:
		 t_event = "IPT_DEFAULT_NO_EVENT";
		 break;

	 case IPT_AUTOMATIC_ROLLBACK_OK:
		 t_event = "IPT_AUTOMATIC_ROLLBACK_OK";
		 break;

	 default:
		 t_event = "UNKNOWN_EVENT";
	 }

	 //this check is based on current state and eventID
	 switch(m_currentState->getStateId())
	 {
	 case STATE_FAILED:
	 case STATE_IDLE:
		 switch(eventId)
		 {
		 case IPT_PREPARE_OK:
			 m_currentState = &m_prepared;
			 status=CCH_Util::LM_ERR_OK;
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
			 case IPT_ACTIVATE_RECEIVED:
				 m_currentState = &m_activating;
				 status=CCH_Util::LM_ERR_OK;
				 modifySwInventory(status);
				 break;

			 case IPT_CANCEL_RECEIVED:
				 m_previousState = getStateId();
				 m_currentState = &m_manualRollbackEnded;
				 status=CCH_Util::LM_ERR_CANCEL;

				 //m_expectedCxc = "";
				 //m_expectedCxp = "";
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
				 case IPT_ACTIVATE_OK:
					 m_currentState = &m_activated;
					 status=CCH_Util::LM_ERR_OK;
					 modifySwInventory(status);
					 break;

				 case IPT_FAILURE_RECEIVED:
					 m_previousState = getStateId();
					 m_currentState = &m_automaticRollbackOngoing;
					 status=CCH_Util::LM_ERR_ACTIVATE_ERROR;
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
					 case IPT_COMMIT_RECEIVED:
						 m_currentState = &m_idle;
						 status = m_lastErrorCode;
						 //setResultAndReason(CCH_Util::LM_ERR_MANUAL_ROLLBACK_SUCCESS);
						 //update current sw into the map
						 FIXS_CCH_UpgradeManager::getInstance()->setCurrentIptProduct(m_magazine,m_slot,m_currentCxp);
						 //update usedBy reference
						 //IMM_Interface::updateUsedPackage(m_magazine,m_slot,m_previousCxp,getExpectedCXP());
						 setExpectedCXP("");
						 setExpectedCXC("");

						 modifySwInventory(status);

						 //FIXS_CCH_UpgradeManager::getInstance()->checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);

						 stop();//stop thread
						 break;

					 case IPT_CANCEL_RECEIVED:
						 m_previousState = getStateId();
						 m_currentState = &m_manualRollbackOngoing;
						 status = CCH_Util::LM_ERR_CANCEL;
						 //setExpectedCXP("");
						 //setExpectedCXC("");
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
						 case IPT_MANUAL_ROLLBACK_OK:
							 m_currentState = &m_manualRollbackEnded;
							 status = CCH_Util::LM_ERR_MANUAL_ROLLBACK_SUCCESS;
							// setExpectedCXP("");
							 //setExpectedCXC("");
							 modifySwInventory(status);
							 break;

						 case IPT_FAILURE_RECEIVED:
							 //m_currentState = &m_failed;
							 m_currentState = &m_idle;
							 status = CCH_Util::LM_ERR_MANUAL_ROLLBACK_FAILURE;
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
								 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
							 }
							 break;
						 }
						 break;

						 case STATE_MANUAL_ROLLBACK_ENDED:
							 switch(eventId)
							 {
							 case IPT_COMMIT_RECEIVED:
								 m_currentState = &m_idle;
								 status = m_lastErrorCode;
								 //setResultAndReason(CCH_Util::LM_ERR_MANUAL_ROLLBACK_SUCCESS);
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
									 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
								 }
								 break;
							 }
							 break;

							 case STATE_AUTOMATIC_ROLLBACK_ONGOING:
								 switch(eventId)
								 {
								 case IPT_AUTOMATIC_ROLLBACK_OK:
									 m_currentState = &m_automaticRollbackEnded;
									 status = CCH_Util::LM_ERR_AUTOMATIC_ROLLBACK_SUCCESS;
									 setExpectedCXP("");
									 setExpectedCXC("");
									 modifySwInventory(status);

									 break;

								 case IPT_FAILURE_RECEIVED:
									 //m_currentState = &m_failed;
									 m_currentState = &m_idle;
									 status = CCH_Util::LM_ERR_AUTOMATIC_ROLLBACK_FAILURE;
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
										 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
									 }
									 break;
								 }
								 break;

								 case STATE_AUTOMATIC_ROLLBACK_ENDED:
									 switch(eventId)
									 {
									 case IPT_COMMIT_RECEIVED:
										 m_currentState = &m_idle;
										 status = m_lastErrorCode;
										 //setResultAndReason(CCH_Util::LM_ERR_AUTOMATIC_ROLLBACK_SUCCESS);
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
											 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
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
	 snprintf(traceChar, sizeof(traceChar) - 1,"[IPT UPGRADE of %s on MAG:%s, SLOT:%u] CURRENT STATE: %s, EVENT RECEIVED: %s, NEXT STATE: %s",
			 t_type.c_str(), magStr.c_str(), m_slot,	t_currState.c_str(), t_event.c_str(), t_nextState.c_str());
	 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
	 if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;

	 }
	 saveStatus();
}

 void FIXS_CCH_IptbUpgrade::saveStatus ()
{
	CCH_Util::BoardSWData mydata;
	getOverallStatus(mydata);

	//NOT NEEDED???
	//FIXS_CCH_FSMDataStorage::getInstance()->saveUpgradeStatus(m_magazine, m_slot, mydata);
}

 int FIXS_CCH_IptbUpgrade::cancel ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	if (strcmp(m_SwInventory_Object_DN.c_str(),"") == 0 )
		 		IMM_Interface::defineSwInventoryObjectDN(m_magazine, m_slot, m_SwInventory_Object_DN);
	int result = m_currentState->cancel();
	return result;
}

 std::string FIXS_CCH_IptbUpgrade::getStateName ()
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

void FIXS_CCH_IptbUpgrade::setPath (std::string completePathIptb)
{
	 m_path=completePathIptb;
}

FIXS_CCH_IptbUpgrade::IptbStateId FIXS_CCH_IptbUpgrade::getStateId () const
{
	 return m_currentState->getStateId();
}

bool FIXS_CCH_IptbUpgrade::configureDHCP (bool rollback)
{
  	bool retValue = false;

	//-------------------------
	//set package to configure
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
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
	}

	std::string magazine("");
	ACS_CS_API_SET_NS::CS_API_Set_Result result;
	CCH_Util::ulongToStringMagazine(m_magazine,magazine);
	//---------------
	cout << "---------------------- API BEGIN---------------------------------------- " << endl;
	cout << "magazine :" << magazine.c_str() << endl;
	cout << "slot :" << m_slot << endl;
	cout << "package :" << package.c_str() << endl;
	cout << "product :" << product.c_str() << endl;
	cout << "---------------------- API END---------------------------------------- " << endl;
	//configure dhcp
	for (int retry = 0; ((retry < 7) && (retValue == false)); retry++)
	{
		result = ACS_CS_API_Set::setSoftwareVersion(package,magazine,m_slot);
		switch (result)
		{
		case ACS_CS_API_SET_NS::Result_Success:
			retValue = true;
			//update usedBy reference
			if(rollback)
                                IMM_Interface::updateUsedPackage(m_magazine,m_slot,getExpectedCXP(),m_currentCxp);
                        else
                                IMM_Interface::updateUsedPackage(m_magazine,m_slot,m_currentCxp,getExpectedCXP());
			break;
		default:
			{
				{
					//trace
					std::string magStr;
					(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
					char tmpChar[512] = {0};
					snprintf(tmpChar, sizeof(tmpChar) - 1,
						"[%s@%d] ACS_CS_API_Set::setSoftwareVersion failed setting package '%s' for IPTB on magazine %s and slot %u. CS Error Code: %u",
						__FUNCTION__, __LINE__, package.c_str(), magStr.c_str(), m_slot, result);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
					std::cout << "DBG: " << tmpChar << std::endl;

				}
				retValue = false;
				usleep(300000);
			}
			break;
		}
	}
	return retValue;
  //## end FIXS_CCH_IptbUpgrade::configureDHCP%4D3D582501AF.body
}

 bool FIXS_CCH_IptbUpgrade::resetBoard ()
{
  	if (!checkMasterScbrp())
	{
		return false;
	}

	if(m_egem2environment)
	{
		if(!m_snmpManager->setBladeReset(1,m_slot))
		{
			//log
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: Failed Reset IPT Board in SCX enviroment at slot " << m_slot << std::endl;
			return false;
		}
		else std::cout << __FUNCTION__ << "@" << __LINE__ << " Successfull reset of IPT Board in SCX enviroment at slot " << m_slot << std::endl;
	}
	else
	{
		if (!m_snmpManager->setXshmcBoardReset((int)m_slot))
		{
			//log
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: Failed Reset IPT Board in SCB-RP enviroment at slot " << m_slot << std::endl;
			return false;
		}
		else std::cout << __FUNCTION__ << "@" << __LINE__ << " Successfull reset of IPT Board in SCB-RP enviroment at slot " << m_slot << std::endl;
	}

	return true;
}

 int FIXS_CCH_IptbUpgrade::onActivating ()
{
	//bool loop = true;
	//Wait for GPR data update
	int result = checkUpgradeStatus();
	if (m_exit) return -1;

	if (result == EXECUTED)
		setEvent(FIXS_CCH_IptbUpgrade::IPT_ACTIVATE_OK);
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
		setEvent(FIXS_CCH_IptbUpgrade::IPT_FAILURE_RECEIVED);
	}

	return result;
}

 int FIXS_CCH_IptbUpgrade::onPreparing ()
{
	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PREPARING STATE: start preparing..."<<std::endl;

	if (getUpgradeType() == CCH_Util::INITIAL_INSTALLATION)
	{
		//Set network boot
		//string input ="FF00000002000000";
		std::string GPRDataValue	=	"";
		std::string mask			=	"FF000000";
		std::string networkBoot		=	"02000000";
		std::string byte;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " INITIAL UPGRADE "<<std::endl;

		if (checkMasterScbrp())
		{
			if(m_egem2environment)
			{
				byte="FF 00 00 00 02 00 00 00" ;
				//network boot

				std::string slotString;
				CCH_Util::ushortToString((unsigned short)m_slot, slotString);
				if (!m_snmpManager->setGprDataValue(byte,slotString,"512"))
				{
					setEvent(FIXS_CCH_IptbUpgrade::IPT_FAILURE_RECEIVED);
					return SNMP_FAILURE;
				}
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Set NVRAM: "<<byte <<endl;
			}
			else
			{
				std::string slotString;

				CCH_Util::ushortToString((unsigned short)m_slot, slotString);
				//reading the GPR NVRAM register OID.slot.128 //NVRAM Register 0
				if (!m_snmpManager->getXshmcGprDataValue( &GPRDataValue, slotString, "128"))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ERROR : getXshmcGPRData"<<endl;
					setEvent(FIXS_CCH_IptbUpgrade::IPT_FAILURE_RECEIVED);
					return SNMP_FAILURE;
				}
				else
				{

					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Read NVRAM: "<<GPRDataValue.c_str() <<" size:" <<GPRDataValue.size()<<endl;

					size_t lookHere = 0;
					size_t foundHere;

					while((foundHere = mask.find('F', lookHere)) != string::npos)
					{
						lookHere++;
						GPRDataValue[(int)foundHere]=networkBoot[(int)foundHere];
					}



					//check NVRAM register for network boot OID.slot.128
					if (!m_snmpManager->setXshmcGprDataValue( GPRDataValue, slotString, "128" ))
					{
						setEvent(FIXS_CCH_IptbUpgrade::IPT_FAILURE_RECEIVED);
						return SNMP_FAILURE;
					}

					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Set NVRAM: "<<GPRDataValue.c_str() <<endl;
				}
			}
		}
		else
		{
			setEvent(FIXS_CCH_IptbUpgrade::IPT_FAILURE_RECEIVED);
			return SNMP_FAILURE;
		}


	}

	setEvent(FIXS_CCH_IptbUpgrade::IPT_PREPARE_OK);
	return result;
}

 void FIXS_CCH_IptbUpgrade::getOverallStatus (CCH_Util::BoardSWData &iptdata)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ << " In" << std::endl;

	iptdata.expectedCxp = m_expectedCxp;
	iptdata.expectedCxc = m_expectedCxc;
	iptdata.currentCxc = m_currentCxc;
	iptdata.currentCxp = m_currentCxp;
	iptdata.lastUpgradeDate = m_lastUpgradeDate;
	iptdata.lastUpgradeTime = m_lastUpgradeTime;
	iptdata.lastUpgradeResult = m_lastSwUpgradeResult;
	iptdata.lastUpgradeReason = m_lastSwUpgradeReason;
	iptdata.dnBladePersistance = dn_blade_persistance;
	//TODO: used m_lastErrorCode in switchState need to confirm for the usage.
	iptdata.lastErrorCode = m_lastErrorCode;
	//iptdata.lastUpgradeType = (m_fullUpradeType ? 1 : 0); //(m_fullUpradeType ? "IPT_FULL_UPG" : "IPT_NORM_UPG");
	iptdata.previousState = m_previousState;
	iptdata.failureCode = m_failureCode;
        iptdata.forcedUpgrade = false;
        iptdata.upgradeType  = static_cast<CCH_Util::BoardUpgradeType>(m_upgradeType);

	switch(m_currentState->getStateId())
	{

		case STATE_IDLE:
			iptdata.upgradeStatus = CCH_Util::STATE_IDLE;
			break;

		case STATE_PREPARED:
			iptdata.upgradeStatus = CCH_Util::STATE_PREPARED;
			break;

		case STATE_ACTIVATING:
			iptdata.upgradeStatus = CCH_Util::STATE_ACTIVATING;
			break;

		case STATE_ACTIVATED:
			iptdata.upgradeStatus = CCH_Util::STATE_ACTIVATED;
			break;

		case STATE_FAILED:
			iptdata.upgradeStatus = CCH_Util::STATE_FAILED;
			break;

		case STATE_AUTOMATIC_ROLLBACK_ONGOING:
			iptdata.upgradeStatus = CCH_Util::STATE_AUTOMATIC_ROLLBACK_ONGOING;
			break;

		case STATE_AUTOMATIC_ROLLBACK_ENDED:
			iptdata.upgradeStatus = CCH_Util::STATE_AUTOMATIC_ROLLBACK_ENDED;
			break;

		case STATE_MANUAL_ROLLBACK_ONGOING:
			iptdata.upgradeStatus = CCH_Util::STATE_MANUAL_ROLLBACK_ONGOING;
			break;

		case STATE_MANUAL_ROLLBACK_ENDED:
			iptdata.upgradeStatus = CCH_Util::STATE_MANUAL_ROLLBACK_ENDED;
			break;

		default:
			iptdata.upgradeStatus = CCH_Util::STATE_UNDEFINED;
	}


	std::cout << __FUNCTION__ << "@" << __LINE__ << " Out" << std::endl;
}

 void FIXS_CCH_IptbUpgrade::restoreStatus (CCH_Util::BoardSWData &boardSWData)
{

	m_expectedCxp = boardSWData.expectedCxp;
	m_expectedCxc = boardSWData.expectedCxc;
	m_currentCxc = boardSWData.currentCxc;
	m_currentCxp = boardSWData.currentCxp;
	m_lastUpgradeDate = boardSWData.lastUpgradeDate;
	m_lastUpgradeTime = boardSWData.lastUpgradeTime;
	m_lastSwUpgradeResult = boardSWData.lastUpgradeResult;
	m_lastSwUpgradeReason = boardSWData.lastUpgradeReason;
	m_previousState = boardSWData.previousState;
	m_failureCode = boardSWData.failureCode;
        m_forcedUpgrade = boardSWData.forcedUpgrade;
        m_upgradeType = boardSWData.upgradeType;
        
	/*if(boardSWData.lastUpgradeType == 1) //if(boardSWData.lastUpgradeType == "IPT_FULL_UPG")
	{
		m_fullUpradeType = true;
	}
	else
	{
		m_fullUpradeType = false;
	}*/

	switch ( boardSWData.upgradeStatus )
	{
		case CCH_Util::STATE_PREPARED:
			m_currentState = &m_prepared;
			break;

		case CCH_Util::STATE_ACTIVATING:
			m_currentState = &m_activating;
			break;

		case CCH_Util::STATE_ACTIVATED:
			m_currentState = &m_activated;
			break;
          case CCH_Util::STATE_MANUAL_ROLLBACK_ONGOING:
			m_currentState = &m_manualRollbackOngoing;
			break;

		case CCH_Util::STATE_MANUAL_ROLLBACK_ENDED:
			m_currentState = &m_manualRollbackEnded;
			break;

		case CCH_Util::STATE_AUTOMATIC_ROLLBACK_ONGOING:
			m_currentState = &m_automaticRollbackOngoing;
			break;

		case CCH_Util::STATE_AUTOMATIC_ROLLBACK_ENDED:
			m_currentState = &m_automaticRollbackEnded;
			break;

		case CCH_Util::STATE_FAILED:
			m_currentState = &m_failed;
			break;

		case CCH_Util::STATE_IDLE:
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
                        "\tforcedUpgrade:\t%d\n"
                        "\tupgradeType:\t%d\n" 
			"\t------------------------------------\n",
			__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), m_lastSwUpgradeResult,
			m_lastSwUpgradeReason, m_lastUpgradeDate.c_str(), m_lastUpgradeTime.c_str(),
			m_currentCxc.c_str(), m_currentCxp.c_str(), m_expectedCxc.c_str(), m_expectedCxp.c_str(), m_forcedUpgrade, m_upgradeType);

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
		std::cout << "DBG: " << tmpChar << std::endl;
	}
}

bool FIXS_CCH_IptbUpgrade::checkMasterScbrp ()
{
	bool masterRead = true;
	int res = 0;
	if(m_egem2environment)
	{
		m_snmpManager->setIpAddress(m_switch0_IP_EthA);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCX address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
		m_switch_Master_IP = m_switch0_IP_EthA;
		if (!m_snmpManager->isMaster())
		{
			m_snmpManager->setIpAddress(m_switch25_IP_EthA);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCX address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
			m_switch_Master_IP = m_switch25_IP_EthA;
			if (!m_snmpManager->isMaster())
			{
				m_snmpManager->setIpAddress(m_switch0_IP_EthB);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCX address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
				m_switch_Master_IP = m_switch0_IP_EthB;
				if (!m_snmpManager->isMaster())
				{
					m_snmpManager->setIpAddress(m_switch25_IP_EthB);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCX address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
					m_switch_Master_IP = m_switch25_IP_EthB;
					if (!m_snmpManager->isMaster())
					{
						masterRead = false;
					}
				}
			}
		}
	}
	else //non-CBA
	{
		m_snmpManager->setIpAddress(m_switch0_IP_EthA);
		res = m_snmpManager->isMasterSCB(m_switch0_IP_EthA, 0);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCB-RP address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
		m_switch_Master_IP = m_switch0_IP_EthA;
		if(res != 1)
		{
			m_snmpManager->setIpAddress(m_switch25_IP_EthA);
			res = m_snmpManager->isMasterSCB(m_switch25_IP_EthA, 25);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCB-RP address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
			m_switch_Master_IP = m_switch25_IP_EthA;
			if(res != 1)
			{
				m_snmpManager->setIpAddress(m_switch0_IP_EthB);
				res = m_snmpManager->isMasterSCB(m_switch0_IP_EthB, 0);
				std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCB-RP address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
				m_switch_Master_IP = m_switch0_IP_EthB;
				if(res != 1)
				{
					m_snmpManager->setIpAddress(m_switch25_IP_EthB);
					res = m_snmpManager->isMasterSCB(m_switch25_IP_EthB, 25);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCB-RP address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
					m_switch_Master_IP = m_switch25_IP_EthB;
					if(res != 1)
					{
						masterRead = false;
					}
				}
			}
		}
	}
//	if (res == 0)
//	{
//		m_snmpManager->setIpAddress(m_switch0_IP_EthA);
//		if(m_egem2environment)
//		{
//			std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCX address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
//		}
//		else
//		{
//			std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCB-RP address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
//		}
//		m_switch_Master_IP = m_switch0_IP_EthA;
//
//		if (!m_snmpManager->isMaster())
//		{
//			m_snmpManager->setIpAddress(m_switch25_IP_EthA);
//			if(m_egem2environment)
//			{
//				std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCX address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
//			}
//			else
//			{
//				std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCB-RP address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
//			}
//			m_switch_Master_IP = m_switch25_IP_EthA;
//
//			if (!m_snmpManager->isMaster())
//			{
//				m_snmpManager->setIpAddress(m_switch0_IP_EthB);
//				if(m_egem2environment)
//				{
//					std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCX address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
//				}
//				else
//				{
//					std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCB-RP address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
//				}
//				m_switch_Master_IP = m_switch0_IP_EthB;
//
//				if (!m_snmpManager->isMaster())
//				{
//					m_snmpManager->setIpAddress(m_switch25_IP_EthB);
//					if(m_egem2environment)
//					{
//						std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCX address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
//					}
//					else
//					{
//						std::cout << __FUNCTION__ << "@" << __LINE__ << " reading mastership on SCB-RP address: " << m_snmpManager->getIpAddress().c_str() << std::endl;
//					}
//					m_switch_Master_IP = m_switch25_IP_EthB;
//
//					if (!m_snmpManager->isMaster())
//					{
//						masterRead = false;
//					}
//				}
//			}
//		}
//
//	}

	if (!masterRead)
	{
		//log
		if(m_egem2environment)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Cannot read the master SCX" << std::endl;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Cannot read the master SCB-RP" << std::endl;
		}
	}
	else
	{
		if(m_egem2environment)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " MASTER SCX is " << m_snmpManager->getIpAddress().c_str() << std::endl;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " MASTER SCB-RP is " << m_snmpManager->getIpAddress().c_str() << std::endl;
		}
	}

	return masterRead;
}

int FIXS_CCH_IptbUpgrade::onAutomaticRollback ()
{
	(void) resetGprRegister7();

	//only execute basic steps
	while ((executeBasicStepsBeforeRollingBack() != EXECUTED) && (!m_exit))
	{
		//trace
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,
							"[%s@%d] AutomaticRollback procedure failure during DHCP/SNMP session",
							__FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		}
		sleep(1);
	}
	if (m_exit) return 0;

	//wait untill the board restarts and set the GPR result value
	int result = checkUpgradeStatus();
	if ((result == -1) && (m_exit)) return 0;

	if (result == EXECUTED)
	{
		setEvent(FIXS_CCH_IptbUpgrade::IPT_AUTOMATIC_ROLLBACK_OK);
	}
	else
	{
		setEvent(FIXS_CCH_IptbUpgrade::IPT_FAILURE_RECEIVED);
	}

	return result;
}

 int FIXS_CCH_IptbUpgrade::onManualRollback ()
{
	int result = 0;

	if (checkUpgradeStatus() == EXECUTED)
	{
		result = EXECUTED;
		setEvent(FIXS_CCH_IptbUpgrade::IPT_MANUAL_ROLLBACK_OK);
	}
	else
	{
		result = INTERNALERR;
		setEvent(FIXS_CCH_IptbUpgrade::IPT_FAILURE_RECEIVED);
	}

	return result;
}

int FIXS_CCH_IptbUpgrade::checkUpgradeStatus ()
{
	bool loop = true;
	int result = INTERNALERR;
	std::string output = "";
	std::string GPRDataValue = "";
	const int WAIT_TIMEOUT= 0;
	ACE_Time_Value timeValue;
	timeValue.set(120);
	struct pollfd singlePollfd[1];
	singlePollfd[0].fd = m_shutdownEvent;
	singlePollfd[0].events = POLLRDHUP | POLLHUP | POLLERR;
	singlePollfd[0].revents = 0;
	if (!m_gprRegister7resetted)
	{
		ACE_OS::poll (singlePollfd, 1, &timeValue);
		//WaitForSingleObject(m_shutdownEvent, 120000);//Wait for 2 minutes
		if (m_exit) return -1;
	}
	m_gprRegister7resetted = false;
	std::string slotString;
	CCH_Util::ushortToString((unsigned short)m_slot, slotString);
	for (int retry = 0 ; (loop && (retry < 15)); retry++)
	{
		ACE_Time_Value singleSelectTime;
		singleSelectTime.set(60);
		//WaitForSingleObject(m_shutdownEvent, 60000);//Wait for 2 minutes
		int singlewaitResult = ACE_OS::poll (singlePollfd, 1, &singleSelectTime);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " singlewaitResult = " << singlewaitResult << std::endl;
		switch (singlewaitResult)
		{

		case WAIT_TIMEOUT:
			{
				if(m_egem2environment)
				{
					//reading the GPR NVRAM register OID.slot.512 //NVRAM Register 0
					if (m_snmpManager->getGprDataValue(&GPRDataValue,slotString,"512"))
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " getGprDataValue read of RAM register 0 is ok for slot " << m_slot << " output = " << GPRDataValue.c_str() << std::endl;
						std::cout << __FUNCTION__ << "@" << __LINE__ << " getGprDataValue read of RAM register 0 is ok for slot " << m_slot << " GPRDataValue[12] " << GPRDataValue[12]<< " GPRDataValue[13] " << GPRDataValue[13]  << std::endl;
						if (GPRDataValue[12] == '0' && (GPRDataValue[13] == 'a' || GPRDataValue[13] == 'A') ) //IPT sets the flash boot
						{
							//check for a GPR RAM register 7 == OID.slot.263
							output = "";
							if (m_snmpManager->getGprDataValue(&output,slotString,"263"))
							{
								std::cout << __FUNCTION__ << "@" << __LINE__ << " getGprDataValue read of RAM register 7 is ok for slot " << m_slot << " output = " << output.c_str() << std::endl;
								//xxxxffff where x can be anything from 0-9,a-f   means success
								std::string successBytes = "";
								std::cout << __FUNCTION__ << "@" << __LINE__ << " The output.size() value " <<output.size()<< std::endl;
								if (output.size() == 24)
								{
									successBytes = output.substr(18,22);
									std::cout << __FUNCTION__ << "@" << __LINE__ << "The successBytes value:" <<successBytes<<"END"<<std::endl;	
								}
								if ((successBytes == "ff ff ") || (successBytes == "FF FF "))
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
								}
								else
								{
									//check the 5th byte. If it is set to 1, the upgrade is failed
									if ((output.size() == 24) && (output[18] == '1'))
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
												"[%s@%d][magazine:%s][slot:%u][state:%s] IPT triggered the upgrade failure . GPR RAM register 7: %s",
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
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
										if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
										std::cout << "DBG: " << tmpChar << std::endl;
									}
								}
							}
							else
							{
								std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: getGprDataValue read of RAM register 7 is not ok for slot " << m_slot << std::endl;
								//------
								//trace
								{
									std::string magStr;
									(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
									char tmpChar[512] = {0};
									snprintf(tmpChar, sizeof(tmpChar) - 1,
										"[%s@%d][magazine:%s][slot:%u][state:%s] Boot Type is set to flash boot, but cannot access to GPR RAM register 7 through SnmpAgent %s",
										__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), m_snmpManager->getIpAddress().c_str());
									if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
									if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
									std::cout << "DBG: " << tmpChar << std::endl;
								}
								//change the master
								(void) checkMasterScbrp();
							}

						}
						else
						{
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
							if (m_snmpManager->getGprDataValue(&output,slotString,"263"))
							{
								if ((output.size() == 23) && (output[18] == '1'))
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
											"[%s@%d][magazine:%s][slot:%u][state:%s] IPT triggered the upgrade failure during Network boot phase. GPR RAM register 7: %s",
											__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), output.c_str());
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
										if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
										std::cout << "DBG: " << tmpChar << std::endl;
								}

								}
							}
							else
							{
								//change the master
								(void) checkMasterScbrp();
							}

						}
					}
					else
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: getGprDataValue read of RAM register 0 is not ok for slot " << m_slot << std::endl;
						//------
						//trace
						{
							std::string magStr;
							(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
							char tmpChar[512] = {0};
							snprintf(tmpChar, sizeof(tmpChar) - 1,
								"[%s@%d][magazine:%s][slot:%u][state:%s] Cannot access GPR NVRAM register OID.slot.512 which contains the Boot Type",
								__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
							if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
							std::cout << "DBG: " << tmpChar << std::endl;
						}
					}
				}
				else
				{

					//reading the GPR NVRAM register OID.slot.128 //NVRAM Register 0
					if (m_snmpManager->getXshmcGprDataValue( &GPRDataValue, slotString, "128"))
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " getXshmcGPRData read of RAM register 0 is ok for slot " << m_slot << " output = " << GPRDataValue.c_str() << std::endl;
						std::cout << __FUNCTION__ << "@" << __LINE__ << " getXshmcGPRData read of RAM register 0 is ok for slot " << m_slot << " GPRDataValue[0] " << GPRDataValue[0]<< " GPRDataValue[1] " << GPRDataValue[1]  << std::endl;

						if (GPRDataValue[0] == '0' && (GPRDataValue[1] == 'a' || GPRDataValue[1] == 'A') ) //IPT sets the flash boot
						{
							//check for a GPR RAM register 7 == OID.slot.71
							output = "";
							if (m_snmpManager->getXshmcGprDataValue(&output, slotString, "71"))
							{
								std::cout << __FUNCTION__ << "@" << __LINE__ << " getXshmcGPRData read of RAM register 7 is ok for slot " << m_slot << " output = " << output.c_str() << std::endl;
								//xxxxffff where x can be anything from 0-9,a-f   means success
								std::string successBytes = "";
								if (output.size() == 8)
								{
									successBytes = output.substr(4,7);
								}

								if ((successBytes == "ffff") || (successBytes == "FFFF"))
								{
									loop = false;
									//------
									//trace
									{
										std::string magStr;
										(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
										char tmpChar[512] = {0};
										snprintf(tmpChar, sizeof(tmpChar) - 1,
											"[%s@%d][magazine:%s][slot:%u][state:%s] Boot Type (OID.slot.128) is 'flash boot'and status(OID.slot.71) is success(%s)",
											__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), output.c_str());
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
										if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
										std::cout << "DBG: " << tmpChar << std::endl;
									}
								}
								else
								{
									//check the 5th byte. If it is set to 1, the upgrade is failed
									if ((output.size() == 8) && (output[4] == '1'))
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
												"[%s@%d][magazine:%s][slot:%u][state:%s] IPT triggered the upgrade failure (5th byte is set to 1). GPR RAM register 7: %s",
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
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
										if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
										std::cout << "DBG: " << tmpChar << std::endl;
									}
								}
							}
							else
							{
								std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: getXshmcGPRData read of RAM register 7 is not ok for slot " << m_slot << std::endl;
								//------
								//trace
								{
									std::string magStr;
									(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
									char tmpChar[512] = {0};
									snprintf(tmpChar, sizeof(tmpChar) - 1,
										"[%s@%d][magazine:%s][slot:%u][state:%s] Boot Type is set to flash boot, but cannot access to GPR RAM register 7 through SnmpAgent %s",
										__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), m_snmpManager->getIpAddress().c_str());
									if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
									if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
									std::cout << "DBG: " << tmpChar << std::endl;
								}
								//change the master
								(void) checkMasterScbrp();
							}

						}
						else
						{
							//NVRAM Register 0 (.128) is still set to Netwok boot
							//------
							//trace
							{
								std::string magStr;
								(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
								char tmpChar[512] = {0};
								snprintf(tmpChar, sizeof(tmpChar) - 1,
									"[%s@%d][magazine:%s][slot:%u][state:%s][Network Boot Phase Ongoing]\n\t OID.slot.128 which contains the Boot Type is not set to flash boot. Value: %s\n",
									__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), GPRDataValue.c_str());
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
								if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
								std::cout << "DBG: " << tmpChar << std::endl;
							}

							//check that there's no error
							output = "";
							if (m_snmpManager->getXshmcGprDataValue(&output, slotString, "71"))
							{
								if ((output.size() == 8) && (output[4] == '1'))
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
											"[%s@%d][magazine:%s][slot:%u][state:%s] IPT triggered the upgrade failure during Network boot phase (5th byte is set to 1). GPR RAM register 7: %s",
											__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str(), output.c_str());
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
										if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
										std::cout << "DBG: " << tmpChar << std::endl;
									}

								}
							}
							else
							{
								//change the master
								(void) checkMasterScbrp();
							}


						}
					}
					else
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: getXshmcGPRData read of RAM register 0 is not ok for slot " << m_slot << std::endl;
						//------
						//trace
						{
							std::string magStr;
							(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
							char tmpChar[512] = {0};
							snprintf(tmpChar, sizeof(tmpChar) - 1,
								"[%s@%d][magazine:%s][slot:%u][state:%s] Cannot access GPR NVRAM register OID.slot.128 which contains the Boot Type",
								__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
							if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
							std::cout << "DBG: " << tmpChar << std::endl;
						}
					}
				}
			}
			break;
		default:
			loop = false;
			return -1;
		}

	}

	//Insert all the operation needed to move in the prepare state
	int gprCode =0;
	if(m_egem2environment)
		gprCode = CCH_Util::getMeaningScxGPRData(output);
	else
		gprCode = CCH_Util::getMeaningGPRData(output);

	//debugging
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Read GPR DATA. Value: " << output.c_str() << std::endl;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Read GPR CODE Value: " << gprCode << std::endl;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Read GPR Meaning: " << (CCH_Util::traslateGPRDataValue(gprCode)).c_str() << std::endl;
	//

	if (gprCode == CCH_Util::GPR_Successful_upgrade_IPT)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " ACTIVATE OK " << std::endl;
		result = EXECUTED;
	}
	else
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: ACTIVATE NOT OK " << std::endl;
		result = INTERNALERR;
	}

	return result;
  //## end FIXS_CCH_IptbUpgrade::checkUpgradeStatus%4D4BDACB0065.body
}

 int FIXS_CCH_IptbUpgrade::setBootType (int bootType)
{
  //## begin FIXS_CCH_IptbUpgrade::setBootType%4D6D16F30269.body preserve=yes
	int result = EXECUTED;
	std::string slotString;
	std::string byte;
	CCH_Util::ushortToString((unsigned short)m_slot, slotString);
	if(m_egem2environment)
	{
		switch (bootType)
                {
                        case flashBootType:
                        {
             	                byte="FF 00 00 00 0a 00 00 00" ;
                        }
                        break;

                        case networkBootType:
                        {	
				byte="FF 00 00 00 02 00 00 00" ; 
                        }
                        break;

                        default:
                                return INTERNALERR;
                } 

		cout <<" String for setGprDataValue is :"<<byte.c_str()<<endl;
		if (checkMasterScbrp())
		{
			if (!m_snmpManager->setGprDataValue(byte,slotString,"512"))
			{
				//error setting NVRAM register
				//------
				//trace
				{
					std::string magStr;
					(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
					char tmpChar[512] = {0};
					snprintf(tmpChar, sizeof(tmpChar) - 1,
						"[%s@%d][magazine:%s][slot:%u][state:%s]SNMP error setting NVRAM register",
						__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
					std::cout << "DBG: " << tmpChar << std::endl;
				}
				return SNMP_FAILURE;
			}
		}
		else
		{
			//error checking the mastership
			//------
			//trace
			{
				std::string magStr;
				(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d][magazine:%s][slot:%u][state:%s] SNMP error checking the mastership",
					__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
			return SNMP_FAILURE;
		}

	}
	else
	{
		std::string GPRDataValue = "";
		std::string mask = "FF000000";
		std::string bootTypeString = "";

		switch (bootType)
		{
			case flashBootType:
				bootTypeString = "0a000000";
				break;

			case networkBootType:
				bootTypeString = "02000000";
				break;

			default:
				return INTERNALERR;
		}

		if (checkMasterScbrp())
		{
			//reading the GPR NVRAM register OID.slot.128 //NVRAM Register 0
			if (m_snmpManager->getXshmcGprDataValue( &GPRDataValue, slotString, "128"))
			{
				if (GPRDataValue.substr(0,2) != bootTypeString.substr(0,2))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Read NVRAM: " << GPRDataValue.c_str() << " size: " << GPRDataValue.size() << std::endl;

					size_t lookHere = 0;
					size_t foundHere;

					while((foundHere = mask.find('F', lookHere)) != string::npos)
					{
						lookHere++;
						GPRDataValue[(int)foundHere] = bootTypeString[(int)foundHere];
					}

					std::string slotString;
					CCH_Util::ushortToString((unsigned short)m_slot, slotString);

					//check NVRAM register for network boot OID.slot.128
					if (!m_snmpManager->setXshmcGprDataValue( GPRDataValue, slotString, "128" ))
					{
						//error setting NVRAM register
						//------
						//trace
						{
							std::string magStr;
							(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
							char tmpChar[512] = {0};
							snprintf(tmpChar, sizeof(tmpChar) - 1,
								"[%s@%d][magazine:%s][slot:%u][state:%s]SNMP error setting NVRAM register",
								__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
							if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
							std::cout << "DBG: " << tmpChar << std::endl;
						}
						return SNMP_FAILURE;
					}
				}
			}
			else
			{
				//error reading NVRAM register
				//------
				//trace
				{
					std::string magStr;
					(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
					char tmpChar[512] = {0};
					snprintf(tmpChar, sizeof(tmpChar) - 1,
						"[%s@%d][magazine:%s][slot:%u][state:%s]SNMP error reading NVRAM register",
						__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());

					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
					std::cout << "DBG: " << tmpChar << std::endl;
				}
				return SNMP_FAILURE;
			}
		}
		else
		{
			//error checking the mastership
			//------
			//trace
			{
				std::string magStr;
				(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d][magazine:%s][slot:%u][state:%s] SNMP error checking the mastership",
					__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
			return SNMP_FAILURE;
		}
	}
	return result;
}

void FIXS_CCH_IptbUpgrade::setResultAndReason (CCH_Util::LmErrors status)
{
	int lastResult=0, lastReason=0;
	CCH_Util::getResultReason(lastResult,lastReason,status);
	setLastUpgradeResult(lastResult);
	setLastUpgradeReason(lastReason);
}

int FIXS_CCH_IptbUpgrade::executeBasicStepsBeforeRollingBack ()
{
	int result = EXECUTED;
	std::string byte;
	//rollback DHCP
	if (!configureDHCP(true))
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR DHCP" << std::endl;
		return NO_REPLY_FROM_CS;
	}
	std::string slotString;
	CCH_Util::ushortToString((unsigned short)m_slot, slotString);
	//Read the Master SCB-RP
	if (checkMasterScbrp())
	{
		if(m_egem2environment)
		{
			byte="FF 00 00 00 0a 00 00 00" ;

			if ((getUpgradeType() == CCH_Util::INITIAL_INSTALLATION)) //set SNMP GPR VALUE to network boot (in ACTIVATED it is set to flash boot)
			{
				//net network boot
				byte="FF 00 00 00 02 00 00 00" ;
				if (m_snmpManager->setGprDataValue(byte,slotString,"512"))
				{
					//success
						//------
						//trace
						{
							std::string magStr;
							(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
							char tmpChar[512] = {0};
							snprintf(tmpChar, sizeof(tmpChar) - 1,
								"[%s@%d][magazine:%s][slot:%u][state:%s]Successfully set NVRAM register",
								__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
							if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
							std::cout << "DBG: " << tmpChar << std::endl;
						}
				}
				else
				{
					//error setting NVRAM register
						//------
						//trace
						{
							std::string magStr;
							(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
							char tmpChar[512] = {0};
							snprintf(tmpChar, sizeof(tmpChar) - 1,
								"[%s@%d][magazine:%s][slot:%u][state:%s]SNMP error setting NVRAM register",
								__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
							if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
							std::cout << "DBG: " << tmpChar << std::endl;
						}
						return SNMP_FAILURE;
				}
			}
		}
		else
		{
			std::string GPRDataValue = "";
			std::string mask = "FF000000";
			std::string flashBoot = "0a000000";
			std::string networkBoot = "02000000";
			std::string typeBoot = flashBoot;

			//-------------
			//INITIAL UPGRADE
			//-------------
			if (getUpgradeType()== CCH_Util::INITIAL_INSTALLATION) //set SNMP GPR VALUE to network boot (in ACTIVATED it is set to flash boot)
			{
				typeBoot = networkBoot;
				if (m_snmpManager->getXshmcGprDataValue( &GPRDataValue, slotString, "128"))
				{
					//success
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Read NVRAM: "<<GPRDataValue.c_str() <<" size:" <<GPRDataValue.size()<<endl;

					size_t lookHere = 0;
					size_t foundHere;

					while((foundHere = mask.find('F', lookHere)) != string::npos)
					{
						lookHere++;
						GPRDataValue[(int)foundHere] = typeBoot[(int)foundHere]; //net or flash boot
					}

					std::string slotString;
					CCH_Util::ushortToString((unsigned short)m_slot, slotString);

					//set NVRAM register to network boot OID.slot.128
					if ( m_snmpManager->setXshmcGprDataValue( GPRDataValue, slotString, "128" ))
					{
						//success
						//------
						//trace
						{
							std::string magStr;
							(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
							char tmpChar[512] = {0};
							snprintf(tmpChar, sizeof(tmpChar) - 1,
								"[%s@%d][magazine:%s][slot:%u][state:%s]Successfully set NVRAM register",
								__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
							if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
							std::cout << "DBG: " << tmpChar << std::endl;
						}
					}
					else
					{
						//error setting NVRAM register
						//------
						//trace
						{
							std::string magStr;
							(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
							char tmpChar[512] = {0};
							snprintf(tmpChar, sizeof(tmpChar) - 1,
								"[%s@%d][magazine:%s][slot:%u][state:%s]SNMP error setting NVRAM register",
								__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
							if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
							std::cout << "DBG: " << tmpChar << std::endl;
						}
						return SNMP_FAILURE;
					}
				}
				else
				{
					//error reading NVRAM register
					//------
					//trace
					{
						std::string magStr;
						(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
						char tmpChar[512] = {0};
						snprintf(tmpChar, sizeof(tmpChar) - 1,
							"[%s@%d][magazine:%s][slot:%u][state:%s]SNMP error reading NVRAM register",
							__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
						if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
						std::cout << "DBG: " << tmpChar << std::endl;
					}
					return SNMP_FAILURE;
				}
			}
		}
		//-----------------
		//NORMAL UPGRADE
		//-----------------
		//Reset Board
		if (resetBoard())
		{
			//success
			//updateNtpSettings(m_iptAddress_EthA, m_iptAddress_EthB);
			//------
			//trace
			{
				std::string magStr;
				(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d][magazine:%s][slot:%u][state:%s] Successfully reset IPT board",
					__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
			return EXECUTED;
		}
		else
		{
			//error: cannot reset ipt board
			//------
			//trace
			{
				std::string magStr;
				(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,
					"[%s@%d][magazine:%s][slot:%u][state:%s] SNMP error: cannot reset ipt board",
					__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
			return SNMP_FAILURE;
		}

	}
	else
	{
		//error checking the mastership
		//------
		//trace
		{
			std::string magStr;
			(void) CCH_Util::ulongToStringMagazine(m_magazine, magStr);
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,
				"[%s@%d][magazine:%s][slot:%u][state:%s] SNMP error checking the mastership",
				__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpChar);
			std::cout << "DBG: " << tmpChar << std::endl;
		}
		return SNMP_FAILURE;
	
        }
	return result;
}

 bool FIXS_CCH_IptbUpgrade::resetGprRegister7 ()
{
	m_gprRegister7resetted = false;

	if(m_egem2environment)
	{
		std::string byte ;
		std::string GPRDataValue = "";
		std::string slotString;
		CCH_Util::ushortToString((unsigned short)m_slot, slotString);

		if ((getUpgradeType()) == CCH_Util::INITIAL_INSTALLATION)
                {
                        // Upgrade initiated by APG, network boot triggered on IPT
                	byte = "00 00 FF FF 00 00 01 01";
		}
                else  //Normal Upgrade 
                {
                        // Upgrade initiated by APG, flash boot triggers partial upgrade on IPT
 		        byte = "00 00 FF FF 00 00 00 01";
		}
			

		if (!m_snmpManager->setGprDataValue(byte,slotString,"263"))
		{
			//trace error
			char tmpChar [512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP error got when setting GPR register 7 to %s",
				__FUNCTION__, __LINE__, byte.c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
			std::cout << "DBG: " << tmpChar << std::endl;
			if (checkMasterScbrp())
			{
				if (!m_snmpManager->setGprDataValue(byte,slotString,"263"))
				{
					//trace error
					char tmpChar [512] = {0};
					snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP error got when setting GPR register 7 to %s",
						__FUNCTION__, __LINE__, byte.c_str()); 

					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
					std::cout << "DBG: " << tmpChar << std::endl;
				}
				else
				{
					m_gprRegister7resetted = true;
				}
			}
			else
			{
				//trace
				char tmpChar [512] = {0};
				
				snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP error got when setting GPR register 7 to %s",
                                                __FUNCTION__, __LINE__, byte.c_str()); 
				
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
		}
		else
		{
			m_gprRegister7resetted = true;
		}
	}
	else
	{
		//Improvement to speed up the checks done for the upgrade procedure
		std::string GPRDataValue = "";
		std::string GPRBootValue = "";
		std::string mask = "0000FFFF";
		std::string slotString;
		CCH_Util::ushortToString((unsigned short)m_slot, slotString);

		if ((getUpgradeType())== CCH_Util::INITIAL_INSTALLATION)
		{
			// Upgrade initiated by APG, network boot triggered on IPT
			GPRBootValue = "00000101";
		}
		else  //Normal 
		{
			// Upgrade initiated by APG, flash boot triggers partial upgrade on IPT
			GPRBootValue = "00000001";
		}

		/*{
			//trace
			char tmpChar [512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Starting %s upgrade. Setting GPR register 7 to %s",
				__FUNCTION__, __LINE__, (m_fullUpradeType ? "INITIAL" : "NORMAL"), GPRBootValue.c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
		}*/

		//read RAM register
		if (!m_snmpManager->getXshmcGprDataValue( &GPRDataValue, slotString, "71"))
		{
			//trace error
			char tmpChar [512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP error got when reading GPR register 7. OID: GPRData.%d.71 SnmpAgent on %s", __FUNCTION__, __LINE__, m_slot, m_snmpManager->getIpAddress().c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
			std::cout << "DBG: " << tmpChar << std::endl;
			if (checkMasterScbrp() == false)
			{
				//trace error
				char tmpChar [512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Cannot read SCB-RP master. SNMP error got when reading GPR register 7. OID: GPRData.%d.71", __FUNCTION__, __LINE__, m_slot);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				std::cout << "DBG: " << tmpChar << std::endl;
				return false;
			}
			else
			{
				//read on the master
				if (!m_snmpManager->getXshmcGprDataValue( &GPRDataValue, slotString, "71"))
				{
					//trace error
					char tmpChar [512] = {0};
					snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP error got when reading GPR register 7. OID: GPRData.%d.71 SnmpAgent on %s", __FUNCTION__, __LINE__, m_slot, m_snmpManager->getIpAddress().c_str());
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
					std::cout << "DBG: " << tmpChar << std::endl;
					return false;
				}
			}

		}

		//figure out new value
		size_t lookHere = 0;
		size_t foundHere;
		while((foundHere = mask.find('F', lookHere)) != string::npos)
		{
			lookHere++;
			GPRDataValue[(int)foundHere] = GPRBootValue[(int)foundHere]; //net or flash boot
		}

		//write RAM register
		if (!m_snmpManager->setXshmcGprDataValue( GPRDataValue, slotString, "71" ))
		{
			//trace error
			char tmpChar [512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP error got when setting GPR register 7 to %s",
				__FUNCTION__, __LINE__, GPRDataValue.c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
			std::cout << "DBG: " << tmpChar << std::endl;
			if (checkMasterScbrp())
			{
				if (!m_snmpManager->setXshmcGprDataValue( GPRDataValue, slotString, "71" ))
				{
					//trace error
					char tmpChar [512] = {0};
					snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP error got when setting GPR register 7 to %s",
						__FUNCTION__, __LINE__, GPRDataValue.c_str());
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
					std::cout << "DBG: " << tmpChar << std::endl;
				}
				else
				{
					m_gprRegister7resetted = true;
				}
			}
			else
			{
				//trace
				char tmpChar [512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Cannot read SCB-RP master. SNMP error got when setting GPR register 7 to %s",
					__FUNCTION__, __LINE__, GPRDataValue.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
		}
		else
		{
			m_gprRegister7resetted = true;
		}
	}

	return m_gprRegister7resetted;
}

 bool FIXS_CCH_IptbUpgrade::isBoardBlocked ()
{
	bool retValue = true;
	int pwrOffVal = 0;

	if (checkMasterScbrp())
	{
		if(m_egem2environment)
		{
			if (m_snmpManager->isBladePower(m_slot,&pwrOffVal))
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " pwrOffVal: " << pwrOffVal << std::endl;
				if ( pwrOffVal == 1) //board has been powered on
				{
					retValue = false;
				}
				else
				{
					//trace
					char tmpChar [512] = {0};
					snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Board is Blocked ", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
					std::cout << "DBG: " << tmpChar << std::endl;
				}

			}
			else
			{
				//trace
				char tmpChar [512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP Error ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
		}
	else
		{
			if (m_snmpManager->getXshmcBoardPwrOff(m_slot,&pwrOffVal))
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " pwrOffVal: " << pwrOffVal << std::endl;
				if ( pwrOffVal == 0) //board has been powered on
				{
					retValue = false;
				}
				else
				{
					//trace
					char tmpChar [512] = {0};
					snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Board is Blocked ", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_WARN);
					std::cout << "DBG: " << tmpChar << std::endl;
				}

			}
			else
			{
				//trace
				char tmpChar [512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] SNMP Error ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
				std::cout << "DBG: " << tmpChar << std::endl;
			}
		}
		return retValue;
	}
	else
	{
		//trace
		char tmpChar [512] = {0};
		snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Unable to read the Master SCB-RP", __FUNCTION__, __LINE__);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
		std::cout << "DBG: " << tmpChar << std::endl;
		return false;
	}
}

 void FIXS_CCH_IptbUpgrade::setObjectDN(const char* dn)
 {
	 m_Object_DN = dn;
 }

 void FIXS_CCH_IptbUpgrade::setSwInvObjectDN(std::string dn)
 {
	 //m_SwInventory_Object_DN = dn.c_str();
	 m_SwInventory_Object_DN = dn;
 }

 std::string FIXS_CCH_IptbUpgrade::getSwInvObjectDN()
 {
	 return m_SwInventory_Object_DN;
 }

 std::string FIXS_CCH_IptbUpgrade::getSwPercistanceDN()
   {
  	 return dn_blade_persistance;
   }

 bool FIXS_CCH_IptbUpgrade::modifySwInventory(int state)
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
	 currSw = ACS_APGCC::after(m_currentCxc,"IPT_");

	 std::cout <<"\n setting state : "<< currState << std::endl;

	 CCH_Util::BoardSWData data;
	 
	 data.lastUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(lastReason);
	 if (!currSw.empty())  data.currentCxc = currSw;
	 else data.currentCxc = m_currentCxc;
	 data.expectedCxc = m_expectedCxc;
	 data.lastUpgradeDate = m_lastUpgradeDate;
	 data.lastErrorCode = state;
	 data.lastUpgradeTime = m_lastUpgradeTime;
	 data.lastUpgradeResult =  static_cast<CCH_Util::RpUpgradeResult>(lastResult);
	 data.lmState =currState ;
	 data.dnBladePersistance = dn_blade_persistance;
	 //data.lastUpgradeType = (m_fullUpradeType ? 1 : 0); //(m_fullUpradeType ? "IPT_FULL_UPG" : "IPT_NORM_UPG");
	 data.previousState = m_previousState;
	 data.failureCode = m_failureCode;
	 data.forcedUpgrade = m_forcedUpgrade;
         data.upgradeType =static_cast<CCH_Util::BoardUpgradeType>(m_upgradeType);

 if(!IMM_Interface::modifyAllLMInventoryAttributes(m_SwInventory_Object_DN,data)) return false;
	 else
	 {
		 std::cout <<"\n ---------------------- ATTRIBUTE CHANGED DURING UPGRADE -------------------------- "<< std::endl;
		 return true;
	 }
	 return true; //remove this after
 }

 bool FIXS_CCH_IptbUpgrade::initSwInventory()
 {
	 int currState = this->getStateId();
	 std::string OtherBlade_Product = "";
	 IMM_Interface::getOtherBladeCurrLoadModule(m_Object_DN,OtherBlade_Product);
	 CCH_Util::BoardSWData data;
	 data.lastUpgradeReason = m_lastSwUpgradeReason;
	 data.currentCxc = OtherBlade_Product;
	 data.expectedCxc = m_expectedCxc;
	 data.lastUpgradeDate = m_lastUpgradeDate;
	 data.lastErrorCode = m_lastErrorCode;
	 data.lastUpgradeTime = m_lastUpgradeTime;
	 data.lastUpgradeResult = m_lastSwUpgradeResult;
	 //data.lastUpgradeType = (m_fullUpradeType ? 1 : 0); //(m_fullUpradeType ? "IPT_FULL_UPG" : "IPT_NORM_UPG");
	 data.previousState = m_previousState;
	 data.failureCode = m_failureCode;
	 data.lmState = currState;
	 data.forcedUpgrade = m_forcedUpgrade;
         data.upgradeType= static_cast<CCH_Util::BoardUpgradeType>(m_upgradeType); 
	 //cout <<"\n m_SwInventory_Object_DN  "<<m_SwInventory_Object_DN << "\nOtherBlade_Product: "<<OtherBlade_Product <<endl;
	 if(!IMM_Interface::modifyAllLMInventoryAttributes(m_SwInventory_Object_DN,data)) return false;
	 else
	 {
		 std::cout <<"\n ---------------------- ATTRIBUTE CHANGED DURING UPGRADE -------------------------- "<< std::endl;
		 return true;
	 }
	 return true; //remove this after
 }

 int FIXS_CCH_IptbUpgrade::getStateImm ()
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

 	/*	case STATE_PREPARING:
 			t_currState = IMM_Util::PREPARING;
 			break;*/

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

 std::string  FIXS_CCH_IptbUpgrade::getIptFileName( std::string package)
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

 void FIXS_CCH_IptbUpgrade::resetEvents ()
 {
 	FIXS_CCH_Event::ResetEvent(m_cmdEvent);
 	m_eventQueue.cleanQueue();
 }

