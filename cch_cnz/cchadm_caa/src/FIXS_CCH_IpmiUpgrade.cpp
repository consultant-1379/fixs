// FIXS_CCH_IpmiUpgrade
#include "FIXS_CCH_IpmiUpgrade.h"

namespace {

void printMachineInfo(unsigned long magazine, unsigned short slot) {
	std::string value;
	char info[256] = {0};
	CCH_Util::ulongToStringMagazine(magazine, value);
	snprintf(info, sizeof(info) - 1, "magazine: %s, slot: %u", value.c_str(), slot);

	std::cout << std::endl;
	std::cout << "       IPMI UPGRADE  " << std::endl;
	std::cout << "      _    _         " << std::endl;
	std::cout << "      \\../ |o_..__  " << std::endl;
	std::cout << "    ^.,(_)______(_).>" << std::endl;
	std::cout << "  ~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	std::cout << "    STATE MACHINE FOR " << info << std::endl;
	std::cout << std::endl;

};

// The indexes of events used by IMPIUpgrade thread
enum Event_Indexes
{
	Shutdown_Event_Index = 0,
	Command_Event_Index = 1,   // Command from client
	TimeoutTrap_Event_Index = 2,
	Stop_Event_Index = 3,
	Number_Of_Events = 4
};

//DWORD Safe_Timeout = 120000;
//DWORD FIVE_MINUTES = 300000;
DWORD TWENTY_MINUTES = 1200000;
//int MAX_RETRY = 3;

#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif

}

namespace {
ACE_thread_mutex_t s_cs_ipmi;
struct Lock
{
	Lock()
	{
		//std::cout << "FIXS_CCH_IpmiUpgrade: Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_trylock(&s_cs_ipmi);
	};

	~Lock()
	{
		//std::cout << "FIXS_CCH_IpmiUpgrade: Leave Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_unlock(&s_cs_ipmi);
	};
};
}

// Class FIXS_CCH_IpmiUpgrade::IpmiState

FIXS_CCH_IpmiUpgrade::IpmiState::IpmiState()
: m_context(0),
  m_id(STATE_IDLE),
  m_set_status(false)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
}

FIXS_CCH_IpmiUpgrade::IpmiState::IpmiState (FIXS_CCH_IpmiUpgrade::IpmiStateId id)
: m_context(0),
  m_set_status(false)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_id = id;
}


FIXS_CCH_IpmiUpgrade::IpmiState::~IpmiState()
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
}

FIXS_CCH_IpmiUpgrade::IpmiStateId FIXS_CCH_IpmiUpgrade::IpmiState::getStateId () const
{
	return m_id;
}

void FIXS_CCH_IpmiUpgrade::IpmiState::setContext (FIXS_CCH_IpmiUpgrade* context)
{
	//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_context = context;
}

void FIXS_CCH_IpmiUpgrade::IpmiState::setSnmpStatus (bool status)
{
	m_set_status = status;
}

//void FIXS_CCH_IpmiUpgrade::IpmiState::setUpgradeResult(int res)
//{
//	m_upgradeResult = res;
//}


int FIXS_CCH_IpmiUpgrade::IpmiState::getUpgradeResult()
{
	return m_context->getUpgradeResult();
}
// Class FIXS_CCH_IpmiUpgrade::StateIdle
FIXS_CCH_IpmiUpgrade::StateIdle::StateIdle()
{
	//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_id = STATE_IDLE;
}

FIXS_CCH_IpmiUpgrade::StateIdle::~StateIdle()
{
	//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
}

int FIXS_CCH_IpmiUpgrade::StateIdle::ipmiUpgradeResultTrap (int result)
{
	UNUSED(result);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ipmi upgrade result trap on idle state\n", __FUNCTION__, __LINE__);
		m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	int res = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "WRONG EVENT IN THIS STATE"<< std::endl;


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] WRONG EVENT IN THIS STATE...\n", __FUNCTION__, __LINE__);
		m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}
	res = WRONG_OPTION;
	return res;

}

int FIXS_CCH_IpmiUpgrade::StateIdle::upgrade (std::string valueToSet, std::string slot, std::string ipMasterSCX)
{
	std::string running("");
	std::string product("");
	std::string revision("");
	int result = 0;
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	FIXS_CCH_IpmiUpgrade::IpmiEventId qEventInfo = DEFAULT_NO_EVENT;


	if(slot == "0" || slot == "25" || slot == "26" || slot == "28")
	{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "CP authorization request for IPMI upgarde" << std::endl;
			int tempSlot = -1;
			CCH_Util::stringToInt(slot, tempSlot);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "STRING slot = " << slot << "INT slot = " << tempSlot << std::endl;
			unsigned char slotOngoing = (unsigned char)tempSlot;
			printf("DBG: [%s@%d] - ++++++++SLOTONGOING FOR SENDSTARTREQUEST: %x \n", __FUNCTION__, __LINE__, slotOngoing);

			bool startReqStatus = m_context->sendStartRequestToCP(slotOngoing);
			if (startReqStatus == false) return NOT_ALLOWED_BY_CP;
	}

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] upgrade action on idle state.\nIP master scx: %s "
				"slot upg: %s    FWpath: %s\n", __FUNCTION__, __LINE__, ipMasterSCX.c_str(), slot.c_str(), valueToSet.c_str());
		m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	//set snmp ip address
	m_snmpManager = new FIXS_CCH_SNMPManager(ipMasterSCX);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ----------------------------------------- "<< std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ip master:        " << ipMasterSCX.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slot:         		      " << slot.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " valueToSet:    	" << valueToSet.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ----------------------------------------- "<< std::endl;

	m_context->m_tftpPath = valueToSet;

	//set snmp ipmi upgrade
	if ( m_snmpManager->setIpmiUpg(slot, valueToSet) == false)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__	<< " SET Failed ! " << std::endl;
		result = SNMP_FAILURE;
		qEventInfo = SNMP_SET_NOT_OK;
		m_context->setUpgradeResult(IMM_Util::FW_NOT_OK);
		m_context->m_slotUpgOnGoing = -1;
		m_context->m_ipMaster = "";
		m_context->m_slotMaster = "";
	}
	else
	{
		result = EXECUTED;
		qEventInfo = SNMP_SET_OK;
		m_context->setUpgradeResult(IMM_Util::FW_OK);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;
	}

	// log

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IP of MasterSCX == %s, result == %d\n", __FUNCTION__, __LINE__, ipMasterSCX.c_str(), result);
		m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_context->setEvent(qEventInfo);

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	return result;
}

// Class FIXS_CCH_IpmiUpgrade::StateOngoing

FIXS_CCH_IpmiUpgrade::StateOngoing::StateOngoing()
{
	//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_id = STATE_ONGOING;
}


FIXS_CCH_IpmiUpgrade::StateOngoing::~StateOngoing()
{
	//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
}

int FIXS_CCH_IpmiUpgrade::StateOngoing::ipmiUpgradeResultTrap (int result)
{

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ipmiUpgradeResultTrap on ongoing state. Trap result: %d\n", __FUNCTION__, __LINE__, result);
		m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
				if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	/*
	Result of an IPMI upgrade request.
	Reserved values are:
		0 =ok
		1 =fileError;error when accessing/reading FW file
		2 =otherError
	Further values to be defined.
	 */

	std::string t_info; // log
	int exitRes = 0;
	FIXS_CCH_IpmiUpgrade::IpmiEventId qEventInfo = DEFAULT_NO_EVENT;

	if (result == 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " TRAP OK " << std::endl;
		t_info = "OK";
		exitRes = IPMIUPG_MIBTRAP_OK;
		qEventInfo = SNMP_TRAP_OK;
		m_context->setUpgradeResult(IMM_Util::FW_OK);
	}
	else if (result == 1)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " TRAP Error: fileError;error when accessing or reading FW file " << std::endl;
		t_info = "TRAP Error: fileError;error when accessing or reading FW file";
		exitRes = IPMIUPG_MIBTRAP_FILE_ERR;
		qEventInfo = SNMP_TRAP_NOT_OK;
		m_context->setUpgradeResult(IMM_Util::FW_NOT_OK);
	}
	else if (result == 2)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " otherError " << std::endl;
		t_info = "TRAP otherError";
		exitRes = IPMIUPG_MIBTRAP_OTHER_ERR;
		qEventInfo = SNMP_TRAP_NOT_OK;
		m_context->setUpgradeResult(IMM_Util::FW_NOT_OK);
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setIpmiUpg executed " << std::endl;
		t_info = "TRAP: setIpmiUpg executed";
		exitRes = IPMIUPG_MIBTRAP_OTHER_ERR;
		qEventInfo = SNMP_TRAP_NOT_OK;
		m_context->setUpgradeResult(IMM_Util::FW_NOT_OK);
	}

	//log
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] result == %d, %s\n", __FUNCTION__, __LINE__, result, t_info.c_str());
		m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_context->setEvent(qEventInfo);

	return exitRes;
}

int FIXS_CCH_IpmiUpgrade::StateOngoing::upgrade (std::string valueToSet, std::string slot, std::string ipMasterSCX)
{
	UNUSED(ipMasterSCX);
	UNUSED(slot);
	UNUSED(valueToSet);



	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "WRONG EVENT IN THIS STATE"<< std::endl;

	//log
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "WRONG EVENT IN THIS STATE");
		m_context->FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (m_context->traceObj->ACS_TRA_ON())m_context->traceObj->ACS_TRA_event(1,tmpStr);
	}

	return IPMIUPG_ONGOING;
}

// Class FIXS_CCH_IpmiUpgrade::EventQueue
FIXS_CCH_IpmiUpgrade::EventQueue::EventQueue()
: m_queueHandle(-1)
{
	Lock lock;
	if ((m_queueHandle = eventfd(0, 0)) == -1) {
		// failed to create eventlock_
	}
}

FIXS_CCH_IpmiUpgrade::EventQueue::~EventQueue()
{
	Lock lock;
	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}

	if (m_queueHandle)
		::close(m_queueHandle);     // close event handle

}


FIXS_CCH_IpmiUpgrade::IpmiEventId FIXS_CCH_IpmiUpgrade::EventQueue::getFirst ()
{
	//	FIXS_CCH_CriticalSectionGuard _(m_queueCriticalSection);
	Lock lock;
	IpmiEventId result = DEFAULT_NO_EVENT;
	if (! m_qEventInfo.empty())
		result = m_qEventInfo.front();

	return result;
	//return ((m_qEventInfo.empty()) ? DEFAULT_NO_EVENT : m_qEventInfo.front());
}

void FIXS_CCH_IpmiUpgrade::EventQueue::popData ()
{
	//	FIXS_CCH_CriticalSectionGuard _(m_queueCriticalSection);
	Lock lock;
	if (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
		//		if ((m_qEventInfo.size() == 0) && ((m_queueHandle != INVALID_HANDLE_VALUE) && (m_queueHandle != 0))) ResetEvent(m_queueHandle);
	}
}

void FIXS_CCH_IpmiUpgrade::EventQueue::pushData (FIXS_CCH_IpmiUpgrade::IpmiEventId info)
{

	Lock lock;
	{ //getting access to the critical section
		bool exist = false;
		/* Amalyze all the even queue to avoid the insertion of
		 * an already present event*/
		for (unsigned int i=0; i<m_qEventInfo.size();i++)
		{
			//check first element
			FIXS_CCH_IpmiUpgrade::IpmiEventId first;
			first = getFirst();
			if (first == info )	exist = true;
			m_qEventInfo.pop_front();
			m_qEventInfo.push_back(first);
		}

		if (!exist)
		{
			m_qEventInfo.push_back(info);
			//	if ((m_qEventInfo.size() == 1) && (m_queueHandle != INVALID_HANDLE_VALUE))
			int retries = 0;
			while (FIXS_CCH_Event::SetEvent(m_queueHandle) == 0)
			{
				cout << "Set m_queueHandle.... retries: " << retries << endl;
				if( ++retries > 10) break;
				sleep(1);
			}//end while
		}
	}// leave critical section

}

bool FIXS_CCH_IpmiUpgrade::EventQueue::queueEmpty ()
{

	Lock lock;
	return m_qEventInfo.empty();
}

size_t FIXS_CCH_IpmiUpgrade::EventQueue::queueSize ()
{

	Lock lock;
	return m_qEventInfo.size();
}

FIXS_CCH_IpmiUpgrade::IpmiEventId FIXS_CCH_IpmiUpgrade::EventQueue::getItem (int index)
{

	Lock lock;
	std::list<IpmiEventId>::iterator it = m_qEventInfo.begin();
	while (index-- && (it != m_qEventInfo.end())) ++it;

	if (it == m_qEventInfo.end())
		return DEFAULT_NO_EVENT;
	else
		return *it;
}

void FIXS_CCH_IpmiUpgrade::EventQueue::cleanQueue()
{
	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}
}

// Class FIXS_CCH_IpmiUpgrade
FIXS_CCH_IpmiUpgrade::FIXS_CCH_IpmiUpgrade()
  : m_trapTimer(0), m_running(false), m_upgradeResult(IMM_Util::FW_OK),m_boardId(0), m_magazine(0), m_slot(0),m_SCX_IP_EthA(0), m_SCX_IP_EthB(0), m_cmdEvent(0),
	m_shutdownEvent(0), m_eventQueue(), m_snmpSyncPtr(0), m_timeOutTrapEvent(0)


{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_jtpHandlerIpmi[0] = NULL;
	m_jtpHandlerIpmi[1] = NULL;
	m_idle.setContext(this);
	m_ongoing.setContext(this);
	m_currentState = &m_idle;
	m_slotUpgOnGoing = -1;
	m_ipMaster = "";
	m_slotMaster = "";
	m_otherBlade_dn = "";
	m_tftpPath = "";
	m_otherBladeHwInfo_dn ="";

}
FIXS_CCH_IpmiUpgrade::FIXS_CCH_IpmiUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string SCX_IP_EthA, std::string SCX_IP_EthB, std::string dn)
  : m_trapTimer(0), m_running(false), m_upgradeResult(IMM_Util::FW_OK), m_boardId(boardId), m_magazine(magazine), m_slot(slot),m_installedPackage(installedPackage), m_SCX_IP_EthA(SCX_IP_EthA),m_SCX_IP_EthB(SCX_IP_EthB),m_otherBlade_dn(dn), m_cmdEvent(0),
	m_shutdownEvent(0), m_eventQueue(), m_snmpSyncPtr(0),m_timeOutTrapEvent(0)
	//note that:
	// 1) this state machine is related to each magazine
	// 2) the SCX IP addresses are got from ipmi_upg command
{
	m_jtpHandlerIpmi[0] = NULL;
	m_jtpHandlerIpmi[1] = NULL;
	m_idle.setContext(this);
	m_ongoing.setContext(this);
	m_currentState = &m_idle;
	m_slotUpgOnGoing = -1;
	m_ipMaster = "";
	m_slotMaster = "";
	m_tftpPath = "";
	m_otherBladeHwInfo_dn ="";

	std::string magValue("");
	std::string slotValue("");
	CCH_Util::ulongToStringMagazine(this->m_magazine,magValue);
	CCH_Util::ushortToString(this->m_slot,slotValue);

	traceObj = new ACS_TRA_trace("FIXS_CCH_IpmiUpgrade");

    FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, " -  IPMI Upgrade: magazine [%s] slot [%s] -  ",magValue.c_str(),slotValue.c_str() );
		FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_StopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);
}


FIXS_CCH_IpmiUpgrade::~FIXS_CCH_IpmiUpgrade()
{
	if (m_cmdEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_cmdEvent);
	}
	if ( m_timeOutTrapEvent && (m_timeOutTrapEvent != -1 /*INVALID_HANDLE_VALUE*/) )
	{
		//CancelWaitableTimer(m_timeOutTrapEvent);// do this even if the timer has not started
		::close(m_timeOutTrapEvent);
	}
	if (m_StopEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_StopEvent);
	}

	if (m_jtpHandlerIpmi[0])
	{
		if (m_jtpHandlerIpmi[0]->isRunningThread()) m_jtpHandlerIpmi[0]->stop();
	}

	if (m_jtpHandlerIpmi[1])
	{
		if (m_jtpHandlerIpmi[1]->isRunningThread()) m_jtpHandlerIpmi[1]->stop();
	}

	int retry = 0;
	if (m_jtpHandlerIpmi[0])
	{
		while (m_jtpHandlerIpmi[0]->isRunningThread() && retry < 5){
			cout<< " m_jtpHandlerIpmi[0] THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		delete m_jtpHandlerIpmi[0];
		m_jtpHandlerIpmi[0] = NULL;
	}

	retry = 0;
	if (m_jtpHandlerIpmi[1])
	{
		while (m_jtpHandlerIpmi[1]->isRunningThread() && retry < 5){
			cout<< " m_jtpHandlerIpmi[1] THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		delete m_jtpHandlerIpmi[1];
		m_jtpHandlerIpmi[1] = NULL;
	}

	delete (traceObj);

	FIXS_CCH_logging = 0;

}


int FIXS_CCH_IpmiUpgrade::upgrade (std::string valueToSet, std::string slot, std::string ipMasterSCX)
{
	//
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	int result = m_currentState->upgrade(valueToSet, slot, ipMasterSCX);
	return result;
	//	return (0);
}

int FIXS_CCH_IpmiUpgrade::ipmiUpgradeResultTrap (int result)
{

	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	int code = m_currentState->ipmiUpgradeResultTrap(result);
	return code;
}

std::string FIXS_CCH_IpmiUpgrade::getOtherBladeDn(){
	return m_otherBlade_dn; //it maight be for example otherBladeId=25,shelfId=1,hardwareConfigurationCategoryId=1,configurationInfoId=1
}

std::string FIXS_CCH_IpmiUpgrade::getOtherBladeHardwareInfoDn()
{
	return m_otherBladeHwInfo_dn;
}

void FIXS_CCH_IpmiUpgrade::setOtherBladeHardwareInfoDn(std::string dn)
{
	m_otherBladeHwInfo_dn = dn;
}

void FIXS_CCH_IpmiUpgrade::setSwBladePersistanceDn(std::string dn)
{
	m_swBladePersistance_dn = dn;
}

std::string FIXS_CCH_IpmiUpgrade::getSwBladePersistanceDn()
{
	return m_swBladePersistance_dn;
}

std::string FIXS_CCH_IpmiUpgrade::getStateName ()
{

	//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::string name("");
	switch(m_currentState->getStateId())
	{
	case STATE_ONGOING:
		name = "ONGOING";
		break;

	case STATE_IDLE:
	default:
		name = "IDLE";
		break;
	}
	return name;

}

void FIXS_CCH_IpmiUpgrade::switchState (FIXS_CCH_IpmiUpgrade::IpmiEventId eventId)
{

	//temporay variables used for logging
	std::string t_currState("");
	std::string t_nextState("");
	std::string t_event("");
	std::string t_type("");

	{ // log
		switch(m_currentState->getStateId())
		{
		case STATE_ONGOING:
			t_currState = "ONGOING";
			break;
		case STATE_IDLE:
			t_currState = "IDLE";
			break;
		default:
			t_currState = "UNKNOWN_STATE";
		}

		switch(eventId)
		{
		case SNMP_TRAP_OK:
			t_event = "SNMP_TRAP_OK";
			break;
		case SNMP_TRAP_NOT_OK:
			t_event = "SNMP_TRAP_NOT_OK";
			break;
		case SNMP_SET_NOT_OK:
			t_event = "SNMP_SET_NOT_OK";
			break;
		case SNMP_SET_OK:
			t_event = "SNMP_SET_OK";
			break;
		default:
			t_event = "UNKNOWN_EVENT";
		}
	}

	switch(m_currentState->getStateId())
	{
	//STARTING STATE ONGOING
	case STATE_ONGOING:
		switch(eventId)
		{
		case SNMP_TRAP_OK:
		case SNMP_TRAP_NOT_OK:
		case SNMP_SET_NOT_OK:
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
			" STATE_ONGOING change in STATE_IDLE" << std::endl;
	//		m_currentState = &m_idle;

			//Stop JTP Maintenance windows on CP
			if ((m_slotUpgOnGoing == 0) || (m_slotUpgOnGoing == 25) || (m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
						" Stop JTP Maintenance windows - slot " << m_slotUpgOnGoing << std::endl;
				if (m_jtpHandlerIpmi[0]) m_jtpHandlerIpmi[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IPMI_END_UPGRADE);
				if (m_jtpHandlerIpmi[1]) m_jtpHandlerIpmi[1]->maintenanceWindow(FIXS_CCH_JTPHandler::IPMI_END_UPGRADE);
			}
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
						" JTP Maintenance windows not enabled for slot " << m_slotUpgOnGoing << std::endl;
			}

			removeTftpFileName();

			//stop jtp threads
			stopJtpThread();

			m_currentState = &m_idle;

			//stop thread
			stop();

			//m_slotUpgOnGoing = -1;
			//m_ipMaster = "";
			//m_slotMaster = "";

			break;

		default: break;
		}
		break;

		//STARTING STATE IDLE
		case STATE_IDLE:
		default:
			switch(eventId)
			{
			case SNMP_SET_OK: // IDLE, SNMP_SET_OK == > ONGOING
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
				"STATE_IDLE change in STATE_ONGOING" << std::endl;
				m_currentState = &m_ongoing;

				//Start JTP Maintenance windows on CP
				if ((m_slotUpgOnGoing == 0) || (m_slotUpgOnGoing == 25) || (m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
							" Start JTP Maintenance windows - slot " << m_slotUpgOnGoing << std::endl;
					unsigned char slotOngoing = (unsigned char)m_slotUpgOnGoing;
					printf("DBG: [%s@%d] - ++++++++SLOTONGOING FOR MAINTENANCEWINDOWS: %x \n", __FUNCTION__, __LINE__, slotOngoing);
					if (m_jtpHandlerIpmi[0]) m_jtpHandlerIpmi[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IPMI_START_UPGRADE,slotOngoing);
					if (m_jtpHandlerIpmi[1]) m_jtpHandlerIpmi[1]->maintenanceWindow(FIXS_CCH_JTPHandler::IPMI_START_UPGRADE,slotOngoing);
				}
				else
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
							" JTP Maintenance windows not enabled for slot " << m_slotUpgOnGoing << std::endl;
				}
				break;

			case SNMP_TRAP_NOT_OK:
			case SNMP_SET_NOT_OK:
				removeTftpFileName();

				//stop jtp threads
				stopJtpThread();

				//stop thread
				stop();
				break;

			default:
				break;
			}
			break;
	}


	{ // log
		switch(m_currentState->getStateId())
		{
		case STATE_ONGOING:
			t_nextState = "ONGOING";
			break;
		case STATE_IDLE:
			t_nextState = "IDLE";
			break;
		default:
			t_nextState = "UNKNOWN_STATE";
		}


		{
			char traceChar[512] = {0};
			std::string magStr;
			CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
			snprintf(traceChar, sizeof(traceChar) - 1,
					"[IPMI UPGRADE on MAG:%s, SLOT:%u][Master: %s] CURRENT STATE: %s, EVENT RECEIVED: %s, NEXT STATE: %s",
					magStr.c_str(), m_slot,	m_ipMaster.c_str(), t_currState.c_str(), t_event.c_str(), t_nextState.c_str());
			FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

	}

	cout<< __FUNCTION__ << "@" << __LINE__ << "****************************** Next State = "<< ((m_currentState->getStateId() == STATE_IDLE) ? "IDLE" : "ONGOING") << endl;
	saveStatus();

}

void FIXS_CCH_IpmiUpgrade::setSnmpStatus (bool status)
{

	m_currentState->setSnmpStatus(status);
}


int FIXS_CCH_IpmiUpgrade::open (void *args)
{
	int result = 0;

	UNUSED(args);
	m_exit = false;

	cout << __FUNCTION__ << "@" << __LINE__ << " Ipmi upgrade: try to activate..." << endl;

	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"IPMI UPGRADE Starting thread on instance [%p]. \n",this);
		FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}

	activate(THR_DETACHED);

	return result;
}

int FIXS_CCH_IpmiUpgrade::svc ()
{
	int result = 0;

	sleep(1);

	initOnStart();

	if (m_exit) return result;

	onRestart();
	if (m_exit) return result;

	m_running = true;

	EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent, m_cmdEvent, m_timeOutTrapEvent, m_StopEvent};

	const int WAIT_FAILED=-1;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;
	const int Shutdown_Event_Index=0;
	const int Command_Event_Index=1;
	const int TimeoutTrap_Event_Index=2;
	const int Stop_Event_Index=3;
	//
	printMachineInfo(m_magazine,m_slotUpgOnGoing);

	while ( !m_exit ) // Set to true when the thread shall exit
	{
		fflush(stdout);
		// Wait for shutdown and command events
		int returnValue=FIXS_CCH_Event::WaitForEvents(Number_Of_Events, handleArr, 10000);
		//DWORD returnValue = WaitForMultipleObjects(Number_Of_Events, handleArr, FALSE, Safe_Timeout);
		switch (returnValue)
		{
		case WAIT_FAILED:   // Event failed
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] Error: Cannot WaitForMultipleObjects \n", __FUNCTION__, __LINE__);
				FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			m_exit = true;    // Terminate the sw upgrade thread
			break;
		}
		case WAIT_TIMEOUT:  // Time out
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] Waiting for events...\n", __FUNCTION__, __LINE__);
//				FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			break;
		}
		case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
	 					{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] Recieved a shutdown event\n", __FUNCTION__, __LINE__);
				FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			m_exit = true;    // Terminate the thread
			break;
	 					}
		case (WAIT_OBJECT_0 + Command_Event_Index):    // Command ready to be handled
	 				  {
			printMachineInfo(m_magazine,m_slotUpgOnGoing);
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] Recieved a Command_Event_Index \n", __FUNCTION__, __LINE__);
				FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			FIXS_CCH_Event::ResetEvent(m_cmdEvent);
			handleCommandEvent();
			break;
	 			  }

		case (WAIT_OBJECT_0 + TimeoutTrap_Event_Index):
	 			  {
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "---------------------------------------------------------" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ".......................TIME OUT !!!......................" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "---------------------------------------------------------" << std::endl;

			FIXS_CCH_Event::ResetEvent(m_timeOutTrapEvent);
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] Recieved a TimeoutTrap_Event_Index \n", __FUNCTION__, __LINE__);
				FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_WARN);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			handleFiveMinutesTrapTimeout();
			break;
	 			  }

		case (WAIT_OBJECT_0 + (Stop_Event_Index)):
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " IpmiUpgrade Stop_Event_Index" << std::endl;

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] IpmiUpgrade Stop_Event_Index", __FUNCTION__, __LINE__);
					FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				}

				FIXS_CCH_Event::ResetEvent(m_StopEvent);
				m_exit = true;
				break;
			}
		default:
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,
						"[%s@%d] Error: Unexpected event signaled.\n", __FUNCTION__, __LINE__);
				FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			break;
		}
		} // End of switch

		sleep(1);
	} // End of the while loop

	//clean
	resetEvents();

	m_running = false;
	return result;
}

void FIXS_CCH_IpmiUpgrade::stop ()
{
	m_exit = true;
	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"IPMI UPGRADE Stopping thread on instance [%p] with stop event [%s]. \n",this,eventNameStop.c_str());
		FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_WARN);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}

	if (m_StopEvent)
	{
		int retries=0;
		cout << __FUNCTION__ <<" " << __LINE__ << endl;
		while (FIXS_CCH_Event::SetEvent(m_StopEvent) == 0)
		{
			cout << "Set StopEvent.... retries: " << retries << endl;
			FIXS_CCH_logging->Write("IpmiUpgrade Set StopEvent.... retries",LOG_LEVEL_WARN);
			if( ++retries > 10) break;
			sleep(1);
		}

		if (retries>10)
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal Stop event.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
	}

	int retry = 0;


	if (m_trapTimer!=NULL)
	{
		if (m_trapTimer->isRunningThread())	m_trapTimer->stopTimer();
	}

	retry = 0;
	if (m_trapTimer)
	{
		while (m_trapTimer->isRunningThread() && retry < 5){
			cout<< " trapTimer THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		m_trapTimer->closeHandle();
		delete (m_trapTimer);
		m_trapTimer = NULL;
	}

}

void FIXS_CCH_IpmiUpgrade::handleCommandEvent ()
{


	if (m_eventQueue.queueEmpty()) return;

	//get event to analyze
	FIXS_CCH_IpmiUpgrade::IpmiEventId qEventInfo = DEFAULT_NO_EVENT;
	qEventInfo = m_eventQueue.getFirst();

	if ( (this->m_currentState->getStateId() == STATE_IDLE) && (qEventInfo == this->SNMP_SET_OK) )
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "START TIMER - STATE_IDLE - SNMP_SET_OK " << std::endl;
		startTrapEventTimer(TWENTY_MINUTES); //start timer waiting for load release completed
	}
	else if (
			(this->m_currentState->getStateId() == STATE_ONGOING) &&
			((qEventInfo == this->SNMP_TRAP_NOT_OK) || (qEventInfo == this->SNMP_TRAP_OK)
					|| (qEventInfo == this->SNMP_SET_NOT_OK))
	)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "STOP TIMER - STATE_ONGOING - SNMP_TRAP = " << qEventInfo << std::endl;
		cancelTrapEventTimer();
	}

	//figure out next state
	switchState(qEventInfo);
	m_eventQueue.popData(); //processing ok

}

void FIXS_CCH_IpmiUpgrade::handleFiveMinutesTrapTimeout ()
{



	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " TIMER EXPIRED : STATE == " << this->getStateName().c_str() << std::endl;

	{  //log
		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d][MAG:%s, SLOT:%u] TWENTY MINUTES TIMEOUT EXPIRED IN STATE %s",__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());

		FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_WARN);
		if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_currentState->getStateId() == STATE_ONGOING)  //timer on load release complete
	{

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] TIMER EXPIRED IN STATE == %s ", __FUNCTION__, __LINE__, this->getStateName().c_str());
			FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " .................... TIMER EXPIRED ......................" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;

		stringstream strSlotUpgOnGoing;
		strSlotUpgOnGoing << m_slotUpgOnGoing;

		int status = checkIPMIStatus(m_SCX_IP_EthA, m_SCX_IP_EthB, strSlotUpgOnGoing.str());
		if (status == CCH_Util::IPMI_ONGOING)
		{
			//start a new timer !
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " .........UPGRADE ALREADY ONGOING - START NEW TIMER......." << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
			startTrapEventTimer(TWENTY_MINUTES);
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " .................. TRAP NOT ARRIVED ....................." << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;

			m_currentState->ipmiUpgradeResultTrap(1);
			cancelTrapEventTimer();
		}

	}
}

bool FIXS_CCH_IpmiUpgrade::startTrapEventTimer (int seconds)
{
	m_trapTimer->setTrapTime(seconds);

	m_trapTimer->open();

	if(m_timeOutTrapEvent == -1)
	{
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Synchronization ERROR: cannot set WaitableTimer.", __FUNCTION__, __LINE__);
			FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
		return false;
	}
	else
	{
		return true;
	}



	//
	//	__;
	//
	//	LARGE_INTEGER timer;
	//	DWORD sec = TWENTY_MINUTES;
	//	LONGLONG llSec = static_cast<LONGLONG>(sec);
	//	timer.QuadPart = llSec * (-10000000);	// Time after which the state of the timer
	//											//is to be set to signaled, in 100 nanosecond intervals.
	//
	//	if (!SetWaitableTimer(m_timeOutTrapEvent, &timer, 0, 0, 0,FALSE))
	//	{
	//		char tmpStr[512] = {0};
	//		_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Synchronization ERROR: cannot set WaitableTimer. Windows Error Code: %u", __FUNCTION__, __LINE__, GetLastError());
	//		std::cout << tmpStr << std::endl;
	//		if (ACS_TRA_ON(traceObj)) ACS_TRA_event(&traceObj, tmpStr);
	//		return false;
	//	}
	//	else
	//	{
	//		return true;
	//	}

	// Gene
}

bool FIXS_CCH_IpmiUpgrade::cancelTrapEventTimer ()
{

	bool result = false;

	if ((m_timeOutTrapEvent == -1) || (m_timeOutTrapEvent == 0))
	{

		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_timeoutTrapEvent OFF ", __FUNCTION__, __LINE__);
			FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

		result = true;
	}
	else {
		if(m_trapTimer)
			m_trapTimer->stopTimer();

		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_timeoutTrapEvent stopped !", __FUNCTION__, __LINE__);
			FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

		result = true;
	}

	//
	//	__;
	//	if ((m_timeOutTrapEvent == INVALID_HANDLE_VALUE) || (m_timeOutTrapEvent == 0)) return true;
	//	if (CancelWaitableTimer(m_timeOutTrapEvent)) return true;
	//	else return false;

	// gene
	return result;
}

void FIXS_CCH_IpmiUpgrade::setEvent (FIXS_CCH_IpmiUpgrade::IpmiEventId eventId)
{

	m_eventQueue.pushData(eventId);
}

int FIXS_CCH_IpmiUpgrade::checkIPMIStatus (std::string ip_EthA, std::string ip_EthB, std::string slot)
{

	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	int snmp_output = -1;

	m_snmpManager = new FIXS_CCH_SNMPManager(ip_EthA);
	if(m_snmpManager->getIpmiUpgStatus(slot, snmp_output) == false)
	{
		std::cout<< __FUNCTION__ << "@" << __LINE__ <<
				"Failed on IP " << ip_EthA << std::endl;
		//try with second IP
		m_snmpManager->setIpAddress(ip_EthB);
		if(m_snmpManager->getIpmiUpgStatus(slot, snmp_output) == false)
		{
			std::cout<< __FUNCTION__ << "@" << __LINE__ <<
					"Failed on IP 2 " << ip_EthB << std::endl;

			return -1;
		}
	}
	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	return snmp_output;


}

int FIXS_CCH_IpmiUpgrade::getIPMIUpgrade (std::string ip_EthA, std::string ip_EthB, std::string slot, std::string &ipmiFw)
{
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(ip_EthA);
	if(m_snmpManager->getIpmiFWUpgrade(&ipmiFw, slot) == false)
	{
		std::cout<< __FUNCTION__ << "@" << __LINE__ <<
				"Failed on IP " << ip_EthA << std::endl;
		//try with second IP
		m_snmpManager->setIpAddress(ip_EthB);
		if(m_snmpManager->getIpmiFWUpgrade(&ipmiFw, slot) == false)
		{
			std::cout<< __FUNCTION__ << "@" << __LINE__ <<
					"Failed on IP 2 " << ip_EthB << std::endl;

			return -1;
		}
	}
	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}
	return 0;
}


void FIXS_CCH_IpmiUpgrade::restoreStatus (CCH_Util::IpmiData &ipmidata)
{


	switch ( ipmidata.upgradeStatus )
	{
	case CCH_Util::IPMI_ONGOING:
		m_currentState = &m_ongoing;
		m_ipMaster = ipmidata.ipMasterSCX;
		m_ipPassive = ipmidata.ipPassiveSCX;
		m_slotMaster = ipmidata.slotMasterSCX;
		m_slotUpgOnGoing = ipmidata.slotUpgOnGoing;
		m_tftpPath = ipmidata.path;
		m_upgradeResult = ipmidata.upgradeResult;
		m_swBladePersistance_dn = ipmidata.dn_ipmi;
		break;

	case CCH_Util::IPMI_IDLE:

		m_currentState = &m_idle;
		m_ipMaster = ipmidata.ipMasterSCX;
		m_ipPassive = ipmidata.ipPassiveSCX;
		m_slotMaster = ipmidata.slotMasterSCX;
		m_upgradeResult = ipmidata.upgradeResult;
		m_swBladePersistance_dn = ipmidata.dn_ipmi;
		break;

	default:
		m_currentState = &m_idle;
		break;
	}

}

void FIXS_CCH_IpmiUpgrade::saveStatus ()
{
	//std::string otherBladeHWInfo("");
	std::string ipmiDn("");

	CCH_Util::IpmiData mydata;
	int status = IMM_Util::FW_UNKNOWN;
//	int iRunning = 0;

	mydata.ipMasterSCX = m_ipMaster;
	mydata.slotMasterSCX = m_slotMaster;
	mydata.slotUpgOnGoing = m_slotUpgOnGoing;
	mydata.ipPassiveSCX = m_ipPassive;

	int upgradeRes = m_currentState->getUpgradeResult();

	//get otherBladeHardwareInfo destinguish name
	//otherBladeHWInfo = getOtherBladeHardwareInfoDn();

	//get the distinguished name
	ipmiDn = getSwBladePersistanceDn();

	if (m_currentState->getStateId() == STATE_ONGOING)
	{
		//////////////ONGOING
		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] try to modify IDLE->ONGOING on \n object:[%s] ", __FUNCTION__, __LINE__,ipmiDn.c_str());
			FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

		mydata.upgradeStatus = CCH_Util::IPMI_ONGOING;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " IPMI_ONGOING " << std::endl;
		status = IMM_Util::FW_ONGOING;

		IMM_Interface::changeIpmiState(ipmiDn.c_str(),status);

	}
	else
	{
		/////////////IDLE
		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] try to modify ONGOING->IDLE on \n object:[%s] ", __FUNCTION__, __LINE__,ipmiDn.c_str());
			FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

		mydata.upgradeStatus = CCH_Util::IPMI_IDLE;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " IPMI_IDLE " << std::endl;
		status = IMM_Util::FW_IDLE;

//		//FIXS_CCH_FSMDataStorage::getInstance()->saveUpgradeStatus(m_magazine, m_slot, mydata);
//		std::stringstream ss_slot;
//		ss_slot << m_slotUpgOnGoing;
//		//	m_snmpManager->getIpmiUpgStatus(ss_slot.str(), status);

//		std::string running("");
//		std::string product("");
//		std::string revision("");

//		m_snmpManager->setIpAddress(m_ipMaster);
//		if (! m_snmpManager->getIPMIBoardData(ss_slot.str(), running, product, revision))
//		{
//			m_snmpManager->setIpAddress(m_ipPassive);
//			m_snmpManager->getIPMIBoardData(ss_slot.str(), running, product, revision);
//		}
//
//		otherBladeHwInfo data;
//		data.fwNumber = product;
//		data.fwRevision = revision;
//
//		if(CCH_Util::stringToInt(running, iRunning))
//		{
//			data.fallBackState = iRunning;
//		}
//		else	std::cout<< "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ERROR: error during fallBackState conversion... Writing in imm model is not possible " << std::endl;

		//STATE
		IMM_Interface::changeIpmiState(ipmiDn.c_str(),status);
		//RESULT and LAST UPGRADE TIME
		IMM_Interface::changeIpmiResult(ipmiDn.c_str(),upgradeRes);
		IMM_Interface::setIpmiUpgradeTime(ipmiDn.c_str());


//		//NUMBER
//		ACS_CC_ImmParameter FWnumber = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEHARDWAREINFO_IPMI_NUMBER,ATTR_STRINGT,data.fwNumber.c_str(),1);
//		result = FIXS_CCH_OtherBladeHardwareInfo::getInstance()->modifyRuntimeObj(otherBladeHWInfo.c_str(),&FWnumber);
//		if (result != ACS_CC_SUCCESS) cout << " FIXS_CCH_OtherBladeHardwareInfo modify " << IMM_Util::ATT_OTHERBLADEHARDWAREINFO_IPMI_NUMBER<< " FAILED !!!"<< endl;
//		delete [] FWnumber.attrValues;
//
//		//REVISION
//		ACS_CC_ImmParameter FWrevision = IMM_Util::defineParameterString(IMM_Util::ATT_OTHERBLADEHARDWAREINFO_IPMI_REVISION,ATTR_STRINGT,data.fwRevision.c_str(),1);
//		result = FIXS_CCH_OtherBladeHardwareInfo::getInstance()->modifyRuntimeObj(otherBladeHWInfo.c_str(),&FWrevision);
//		if (result != ACS_CC_SUCCESS) cout << " FIXS_CCH_OtherBladeHardwareInfo modify " << IMM_Util::ATT_OTHERBLADEHARDWAREINFO_IPMI_REVISION<< " FAILED !!!"<< endl;
//		delete [] FWrevision.attrValues;

//		//FALLBACK
//		int FBstate = data.fallBackState;
//		ACS_CC_ImmParameter FallBack = IMM_Util::defineParameterInt(IMM_Util::ATT_OTTHERBLADEHARDWAREINFO_FALLBACK_STATUS,ATTR_INT32T,&FBstate,1);
//		result = FIXS_CCH_OtherBladeHardwareInfo::getInstance()->modifyRuntimeObj(otherBladeHWInfo.c_str(),&FallBack);
//		if (result != ACS_CC_SUCCESS) cout << " FIXS_CCH_OtherBladeHardwareInfo modify " << IMM_Util::ATT_OTTHERBLADEHARDWAREINFO_FALLBACK_STATUS<< " FAILED !!!"<< endl;
//		delete [] FallBack.attrValues;

		//thread stop
//		stop();

	}

}

void FIXS_CCH_IpmiUpgrade::turnOffStateMachine ()
{
	/*
	this method is used to force the TURN OFF of state machine
    it is used when a mismatch if found and it's needed turn off
	the state machine.
	 */

	switchState(SNMP_TRAP_OK);
	cancelTrapEventTimer();
}

void FIXS_CCH_IpmiUpgrade::turnOnStateMachine (int iSlot, std::string ipMasterSCX, std::string slotMaster)
{
	/*
	this method is used to force the TURN ON of state machine
    it is used when a mismatch if found and it's needed turn off
	the state machine.
	 */

	m_slotUpgOnGoing = iSlot;
	m_ipMaster = ipMasterSCX;
	m_slotMaster = slotMaster;
	switchState(SNMP_SET_OK);
	startTrapEventTimer(TWENTY_MINUTES);
}

bool FIXS_CCH_IpmiUpgrade::removeTftpFileName()
{
	bool res = false;

	std::string path("");
	std::string temp_path("");

	int iPos = m_tftpPath.find_last_of(CHR_IPMI_DIR_SEPARATOR);
	if(iPos >= 0){
		// Filename is part of input path. Extract it.
		temp_path = m_tftpPath.substr(iPos +1);
	}

	std::string tftpDir = FIXS_CCH_DiskHandler::getTftpRootFolder();
	path = tftpDir + "/" + temp_path;

	char traceChar[512] = {0};
	snprintf(traceChar, sizeof(traceChar) - 1,"IPMI UPGRADE - Tftp Path to remove : %s ",path.c_str());
	FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);

 	FIXS_CCH_DiskHandler::deleteFile(path.c_str());

	return res;
}

void FIXS_CCH_IpmiUpgrade::initOnStart ()
{

	m_trapTimer = new FIXS_CCH_TrapTimer();

	unsigned char cSlot = (unsigned char)this->m_slot;
	unsigned long hlmag = ntohl(this->m_magazine);
	unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
	unsigned char mag = u_plug0 & 0x0F;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " m_magazine: " << this->m_magazine << " mag: " << (int)mag << " slot: " << this->m_slot << std::endl;

	if ((m_slot == 0) || (m_slot == 25))
	{
		if (m_jtpHandlerIpmi[0] == NULL) m_jtpHandlerIpmi[0] = new FIXS_CCH_JTPHandler(mag, cSlot,1);

		CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
		if (env == CCH_Util::MULTIPLECP_CBA)
		{
			if (m_jtpHandlerIpmi[1]==NULL) m_jtpHandlerIpmi[1] = new FIXS_CCH_JTPHandler(mag, cSlot,2);
		}
	}


		//get command event
//		if (m_cmdEvent == 0)
			m_cmdEvent = m_eventQueue.getHandle();

		//create shutdown event
		if (m_shutdownEvent == 0)
		{
			m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
			if(m_shutdownEvent == FIXS_CCH_Event_RC_ERROR)
			{
				{
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] error creating shutdown event. \n", __FUNCTION__, __LINE__);
					FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
				}
			}
		}

		//initialize timeout event
		/*if ( (!m_timeOutTrapEvent) || (m_timeOutTrapEvent == -1) )	*/m_timeOutTrapEvent = m_trapTimer->getEvent();

		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] get Event TrapTimer[%d]. \n", __FUNCTION__, __LINE__,m_timeOutTrapEvent);
			FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,traceChar);
		}

		if ((m_slot == 0) ||(m_slot == 25))
		{
			//initialize JTP notification client
			if (m_jtpHandlerIpmi[0]) m_jtpHandlerIpmi[0]->open();
			if (m_jtpHandlerIpmi[1]) m_jtpHandlerIpmi[1]->open();
		}

}

void FIXS_CCH_IpmiUpgrade::resetEvents ()
 {
	 FIXS_CCH_Event::ResetEvent(m_StopEvent);

	 FIXS_CCH_Event::ResetEvent(m_cmdEvent);

	 FIXS_CCH_Event::ResetEvent(m_timeOutTrapEvent);

	 m_eventQueue.cleanQueue();
 }

void FIXS_CCH_IpmiUpgrade::onRestart ()
{
	if (m_currentState->getStateId() == STATE_ONGOING)
	{
		//check if this slot is already ongoing
		int status = -1;
		stringstream sSlot;
		sSlot << m_slotUpgOnGoing;
		std::string ipmiUpgFwValue("");


		if ((m_SCX_IP_EthA == m_ipMaster) || (m_SCX_IP_EthA == m_ipMaster))
		{
			status = checkIPMIStatus(m_SCX_IP_EthA, m_SCX_IP_EthB, sSlot.str());

			getIPMIUpgrade(m_SCX_IP_EthA,m_SCX_IP_EthB,sSlot.str(),ipmiUpgFwValue);
		}
		else
		{
			status = checkIPMIStatus(m_ipMaster, m_ipMaster, sSlot.str());

			getIPMIUpgrade(m_ipMaster,m_ipMaster,sSlot.str(),ipmiUpgFwValue);
		}

		m_tftpPath = ipmiUpgFwValue;

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" verify board status " << sSlot.str() << " - status = " << status << std::endl;

		if (status == CCH_Util::IPMI_ONGOING) //ongoing
		{
			startTrapEventTimer(TWENTY_MINUTES);
		}
		else //now is stopped !
		{
			switchState(SNMP_TRAP_OK);
		}
	}
}

bool FIXS_CCH_IpmiUpgrade::sendStartRequestToCP (unsigned char slotOngoing)
{
  	bool canStartRequest = false;

  	if (m_jtpHandlerIpmi[0])
  	{
  		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "+++++++++++A+++++++++++" << std::endl;
  		m_jtpHandlerIpmi[0]->setStartReqStatus(true);
  		m_jtpHandlerIpmi[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IMB_UPG_START_REQ, slotOngoing); //start request will be sent only to CP1
  		m_jtpHandlerIpmi[0]->waitStartReqToFinish(); // wait for start request response from CP
  		canStartRequest = m_jtpHandlerIpmi[0]->checkStartReqStatus();
  	}
  	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "++++++++++++B++++++++++" << std::endl;
	return canStartRequest;
}

void  FIXS_CCH_IpmiUpgrade::stopJtpThread()
{
	int retry = 0;
	if (m_jtpHandlerIpmi[0])
	{
		if (m_jtpHandlerIpmi[0]->isRunningThread()){
			
			while(m_jtpHandlerIpmi[0]->isSendingNotification() && retry < 240)
			{
				sleep(1);
				retry++;
			}
			if(!m_jtpHandlerIpmi[0]->isSendingNotification())
			{

				m_jtpHandlerIpmi[0]->stop();

				retry = 0;
				while (m_jtpHandlerIpmi[0]->isRunningThread() && retry < 5){
					cout<< " m_jtpHandler[0] THREAD is still running " << endl;
					sleep(1);
					retry++;
				}
				if (!m_jtpHandlerIpmi[0]->isRunningThread())
				{
					delete m_jtpHandlerIpmi[0];
					m_jtpHandlerIpmi[0] = NULL;
				}
			}
			else
			{
					
				cout<< " JTP THREAD is still sending notification..... " << endl;
			}

		}
	}

	if (m_jtpHandlerIpmi[1])
	{
		if (m_jtpHandlerIpmi[1]->isRunningThread()){

			retry = 0;
			while(m_jtpHandlerIpmi[1]->isSendingNotification() && retry < 240)
			{
				sleep(1);
				retry++;
			}
			if(!m_jtpHandlerIpmi[1]->isSendingNotification())
			{

				m_jtpHandlerIpmi[1]->stop();	

				retry = 0;
				while (m_jtpHandlerIpmi[1]->isRunningThread() && retry < 5){
					cout<< " m_jtpHandlerIpmi[1] THREAD is still running " << endl;
					sleep(1);
					retry++;
				}
				if(!m_jtpHandlerIpmi[1]->isRunningThread())
				{
					delete m_jtpHandlerIpmi[1];
					m_jtpHandlerIpmi[1] = NULL;
				}
			}
			else
			{
				cout<< " JTP THREAD is still sending notification..... " << endl;
			}

		}
	}
}
