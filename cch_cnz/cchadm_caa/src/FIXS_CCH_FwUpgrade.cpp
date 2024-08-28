/*
 * FIXS_CCH_FwUpgrade.cpp
 *
 *  Created on: Mar 14, 2012
 *      Author: eanform
 */

#include "FIXS_CCH_FwUpgrade.h"
#define IPMI_TYPE_COUNT 3
namespace {

void printMachineInfo(unsigned long magazine, unsigned short slot) {
	std::string value;
	char info[256] = {0};
	CCH_Util::ulongToStringMagazine(magazine, value);
	snprintf(info, sizeof(info) - 1, "magazine: %s, slot: %u", value.c_str(), slot);

	//		std::cout << std::endl;
	//		std::cout << "       FW UPGRADE  " << std::endl;
	//		std::cout << "      _    _         " << std::endl;
	//		std::cout << "      \\../ |o_..__  " << std::endl;
	//		std::cout << "    ^.,(_)______(_).>" << std::endl;
	//		std::cout << "  ~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	//		std::cout << "    STATE MACHINE FOR " << info << std::endl;
	//		std::cout << std::endl;

};

// The indexes of events used by FWUpgrade thread
enum Event_Indexes
{
	Shutdown_Event_Index = 0,
	Command_Event_Index = 1,   // Command from client
	TimeoutTrap_Event_Index = 2,
	TimeoutTrap_Event_Index_FiveMin = 3,
	Stop_Event_Index = 4,
	Number_Of_Events = 5
};

DWORD Safe_Timeout = 120000;
DWORD TEN_MINUTES = 600000;
DWORD FIFTEEN_SEC = 15000;
}

namespace {
ACE_thread_mutex_t s_cs_fw;
struct Lock
{
	Lock()
	{
		//std::cout << "FIXS_CCH_IpmiUpgrade: Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_trylock(&s_cs_fw);
	};

	~Lock()
	{
		//std::cout << "FIXS_CCH_IpmiUpgrade: Leave Lock region while exception active." << std::endl;
		ACE_OS::thread_mutex_unlock(&s_cs_fw);
	};
};
}

const std::string UBOOT_FILE ="bootloader/coreboot_smxb.rom";
const std::string FAILSAFE_FILE ="linux/smxb-fail-safe-linux.img";
const std::string BMC_FILE ="bmc/Phantom.hpm";
const std::string SENSOR_FILE ="ipmi_params/sensor_table.bin";
const std::string PMP_FILE ="ipmi_params/pmb.bin";
static int fwInfoCount = 0;

// Class FIXS_CCH_FwUpgrade::EventQueue

FIXS_CCH_FwUpgrade::EventQueue::EventQueue()
: m_queueHandle(-1)
{
	Lock lock;
	if ((m_queueHandle = eventfd(0, 0)) == -1) {
		// failed to create eventlock_
	}
}


FIXS_CCH_FwUpgrade::EventQueue::~EventQueue()
{
	Lock lock;
	while (!m_qEventInfo.empty())
	{
		m_qEventInfo.pop_front();
	}

	if (m_queueHandle)
		::close(m_queueHandle);     // close event handle

}


FIXS_CCH_FwUpgrade::FwEventId FIXS_CCH_FwUpgrade::EventQueue::getFirst ()
{
	Lock lock;
	FwEventId result = DEFAULT_NO_EVENT;
	if (! m_qEventInfo.empty())
		result = m_qEventInfo.front();

	return result;

}

void FIXS_CCH_FwUpgrade::EventQueue::popData ()
{
	Lock lock;
	if (!m_qEventInfo.empty())
	{
		m_qEventInfo.pop_front();
	}
}

void FIXS_CCH_FwUpgrade::EventQueue::pushData (FIXS_CCH_FwUpgrade::FwEventId info)
{

	Lock lock;
	{ //getting access to the critical section
		bool exist = false;
		/* Amalyze all the even queue to avoid the insertion of
		 * an already present event*/
		for (unsigned int i=0; i<m_qEventInfo.size();i++)
		{
			//check first element
			FIXS_CCH_FwUpgrade::FwEventId first;
			first = getFirst();
			if (first == info )	exist = true;
			m_qEventInfo.pop_front();
			m_qEventInfo.push_back(first);
		}

		if (!exist)
		{
			m_qEventInfo.push_back(info);
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

bool FIXS_CCH_FwUpgrade::EventQueue::queueEmpty ()
{
	Lock lock;
	return m_qEventInfo.empty();
}

size_t FIXS_CCH_FwUpgrade::EventQueue::queueSize ()
{
	Lock lock;
	return m_qEventInfo.size();
}

FIXS_CCH_FwUpgrade::FwEventId FIXS_CCH_FwUpgrade::EventQueue::getItem (int index)
{
	Lock lock;
	std::list<FwEventId>::iterator it = m_qEventInfo.begin();
	while (index-- && (it != m_qEventInfo.end())) ++it;

	if (it == m_qEventInfo.end())
		return DEFAULT_NO_EVENT;
	else
		return *it;
}

void FIXS_CCH_FwUpgrade::EventQueue::cleanQueue()
{
	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}
}

// Class FIXS_CCH_FwUpgrade::FwState

FIXS_CCH_FwUpgrade::FwState::FwState()
: m_context(0),
  m_id(STATE_UNDEFINED),
  m_set_status(false)
{
}

FIXS_CCH_FwUpgrade::FwState::FwState (FIXS_CCH_FwUpgrade::FwStateId id)
: m_context(0),
  m_set_status(false)
{
	m_id = id;
}


FIXS_CCH_FwUpgrade::FwState::~FwState()
{
}

FIXS_CCH_FwUpgrade::FwStateId FIXS_CCH_FwUpgrade::FwState::getStateId () const
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	return m_id;
}

void FIXS_CCH_FwUpgrade::FwState::setContext (FIXS_CCH_FwUpgrade* context)
{
	m_context = context;
}

void FIXS_CCH_FwUpgrade::FwState::setSnmpStatus (bool status)
{
	m_set_status = status;
}

// Additional Declarations



// Class FIXS_CCH_FwUpgrade::StateIdle

FIXS_CCH_FwUpgrade::StateIdle::StateIdle()
{
	m_id = STATE_IDLE;
}


FIXS_CCH_FwUpgrade::StateIdle::~StateIdle()
{
}

int FIXS_CCH_FwUpgrade::StateIdle::fwUpgradeResultTrap (int result)
{
	int res = 0;
	UNUSED(result);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "WRONG EVENT IN THIS STATE"<< std::endl;
	res = WRONG_OPTION;
	return res;
}

int FIXS_CCH_FwUpgrade::StateIdle::upgrade (std::string valueToSet, std::string slot, std::string tmpIPA, std::string tmpIPB)
{

	std::cout<<"DBG: "<< __FUNCTION__ <<" @ "<< __LINE__ <<" UPGRADE IDLE SLOT: " << slot.c_str() <<std::endl;
	int result = 0;
	FIXS_CCH_FwUpgrade::FwEventId qEventInfo = DEFAULT_NO_EVENT;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " perform set with slot " << slot.c_str() <<" -- valueToSet " << valueToSet << std::endl;

	std::string actState = "";

	FIXS_CCH_SNMPManager *m_snmpManager = NULL;

	m_snmpManager = new FIXS_CCH_SNMPManager(tmpIPA);
	int iSlot = atoi(slot.c_str());	

	if(m_snmpManager->getBladeRevisionState(&actState) == false)
	{
		m_snmpManager->setIpAddress(tmpIPB);
		if(m_snmpManager->getBladeRevisionState(&actState) == false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " GET Failed ! " << std::endl;
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - GET Failed ! ",LOG_LEVEL_ERROR);
			result = SNMP_FAILURE;
			qEventInfo = SNMP_SET_NOT_OK;
			m_context->m_slotUpgOnGoing = -1;
		}
	}

	CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
	int status =-1;
	bool isCmx4 = false;
	int sw_version = 0;
	if ((iSlot == 26 ) || (iSlot == 28 ) )
	{
		int _result =0;
		_result = CCH_Util::getCmxSoftwareVersion (m_context->m_magazine,static_cast<unsigned long>(iSlot), sw_version);
		if((_result == 0) && (sw_version == 4))
			isCmx4 = true;
	}
	if ( isCmx4 || (env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX))
	{
		if(result != SNMP_FAILURE)
		{
			size_t found = 0;
			found = actState.find_first_of(" ");
			if(found!=string::npos)
			{
				actState = actState.substr(0,found);
			}

			if(m_snmpManager->getFwmActionResult(status))
			{
				if(status == 1)
				{
					actState = actState + "+";
				}	
			}
			else
			{
				std::string tmpIp = m_snmpManager->getIpAddress();
				if(tmpIp.compare(tmpIPA) == 0)
					m_snmpManager->setIpAddress(tmpIPB);
				else
					m_snmpManager->setIpAddress(tmpIPA);
				if(m_snmpManager->getFwmActionResult(status))
				{
					if(status == 1)
					{
						actState = actState + "+";
					}
				}
				else
					result = SNMP_FAILURE;
			}
		}

	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	std::cout << "\n BladeRevision state before saving : " << actState.c_str() << std::endl; 

	if( result == SNMP_FAILURE)
	{
		{ // log
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of getBladeRevisionState() == %d",__FUNCTION__, __LINE__,result);
			if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1, traceChar);
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
		}
		m_context->setEvent(qEventInfo);

		return result;
	}
	//save BladeRevisionState
	FIXS_CCH_FSMDataStorage::getInstance()->saveFwRevisionState(m_context->m_percistance_dn,actState);

	//Get new fwcontainerfile info from metadata.xml file
	CCH_Util::fwRemoteData fwData;
	std::string filenameXml("");
	std::string tftpRoot = FIXS_CCH_DiskHandler::getTftpRootFolderDefault();
	bool fileFound = false;
	bool isSMX = false;
	if ((env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX))
	{
		isSMX = true;
		fileFound = CCH_Util::findJsonFile(filenameXml,tftpRoot);
	}
	else
		fileFound = CCH_Util::findXmlFile(filenameXml,tftpRoot);
	
	if(fileFound)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Successful in finding metadata file" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "The metadata file name is:" << filenameXml.c_str() <<std::endl;

		std::string xmlPathFile("");
		if(FIXS_CCH_FSMDataStorage::getInstance()->readFwXmlFileName(m_context->m_percistance_dn, xmlPathFile)== false) result = INTERNALERR;
		bool parseVal = false;
		if(isSMX)
			parseVal = m_context->parseJsonFile(xmlPathFile,fwData);
		else
			parseVal = m_context->initializeParser (xmlPathFile,fwData);
		if(parseVal)
		{
			//parsing successful
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Parsing successful" << std::endl;
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - Parsing successful ! ",LOG_LEVEL_DEBUG);

			CCH_Util::EnvironmentType m_environment = FIXS_CCH_DiskHandler::getEnvironment();
			bool isAreaA = false;
			if ((m_environment == CCH_Util::SINGLECP_SMX) || (m_environment == CCH_Util::MULTIPLECP_SMX))
			{
				isSMX = true;
			}
			if(isSMX)
			{
				result = m_context->getActiveFwArea(tmpIPA, tmpIPB, isAreaA);
				if (result != EXECUTED)
					return result;
				std::string package_productno = fwData.productno;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "package_productno: " << package_productno.c_str() << " rState: "
				<< fwData.cxp_rState.c_str() << std::endl;
				int ret_val=0;
				unsigned int upgSlot = atoi(slot.c_str());
				if (isAreaA)
				{
//					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "areaA active" << SMX_FWTYPE_A_CXP << std::endl;
					if (package_productno.compare(SMX_FWTYPE_A_CXP) != 0)
					{
						int val=1; //set area A
						ret_val = m_context->activeAreaSwitch(upgSlot,val);
						if(ret_val != 0 ) return ret_val;
					}

				}
				else
				{
//					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "areaB active" << SMX_FWTYPE_B_CXP << std::endl;
					if (package_productno.compare(SMX_FWTYPE_B_CXP) != 0)
					{
						int val=0; //set area B
						ret_val = m_context->activeAreaSwitch(upgSlot,val);
						if(ret_val != 0 ) return ret_val;
					}
				}
			}
				
			fwInfoCount =0;

			//write the new fwcontainerfile info in to persistent file
			if (!FIXS_CCH_FSMDataStorage::getInstance()->saveFwUpgradeStatusFromFile(m_context->m_percistance_dn,fwData,isSMX)){
				if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - FAILED to store info from xmlFile...",LOG_LEVEL_ERROR);
				return INTERNALERR;
			}
		}
		else
		{
			//parsing error
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Parsing failed" << std::endl;
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - Parsing failed ! ",LOG_LEVEL_ERROR);
			return XMLERROR;
		}
	}
	else
	{
		//didnot find xml file
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Failed in finding metadata.xml file" << std::endl;
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - Failed in finding metadata.xml file ! ",LOG_LEVEL_ERROR);
		return INCORRECT_PACKAGE;
	}

	m_snmpManager = new FIXS_CCH_SNMPManager(tmpIPA);

	if (m_snmpManager->setFWMActionURL(valueToSet) == false)
	{
		m_snmpManager->setIpAddress(tmpIPB);

		if ( m_snmpManager->setFWMActionURL(valueToSet) == false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " SET Failed ! " << std::endl;
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - SET Failed ! ",LOG_LEVEL_ERROR);
			result = SNMP_FAILURE;
			qEventInfo = SNMP_SET_NOT_OK;
			m_context->m_slotUpgOnGoing = -1;

			{ // log
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of setFWMActionURL(url)== %d",__FUNCTION__, __LINE__, result);
				if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			}

		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " setFwUpg executed " << std::endl;
			if(m_snmpManager->setFWMAction(1)== false)
			{
				std::string tmpIp = m_snmpManager->getIpAddress();
				if(tmpIp.compare(tmpIPA) == 0)
					m_snmpManager->setIpAddress(tmpIPB);
				else
					m_snmpManager->setIpAddress(tmpIPA);
				if(m_snmpManager->setFWMAction(1)== false)
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " SET Failed ! " << std::endl;
					if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - SET Failed ! ",LOG_LEVEL_ERROR);
					result = SNMP_FAILURE;
					qEventInfo = SNMP_SET_NOT_OK;
					m_context->m_slotUpgOnGoing = -1;
					{ // log
						char traceChar[512] = {0};
						snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of setFWMAction(1)== %d",__FUNCTION__, __LINE__, result);
						if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
					}
				}
				else
				{
					result = EXECUTED;
					qEventInfo = SNMP_SET_OK;
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
					if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - SNMP SET OK SLOT ! ",LOG_LEVEL_DEBUG);
					m_context->m_slotUpgOnGoing = atoi(slot.c_str());
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;
				}
			}
			else
			{
				result = EXECUTED;
				qEventInfo = SNMP_SET_OK;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
				if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - SNMP SET OK SLOT ! ",LOG_LEVEL_ERROR);
				m_context->m_slotUpgOnGoing = atoi(slot.c_str());
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;
			}
		}
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " setFwUpg executed " << std::endl;

		if(m_snmpManager->setFWMAction(1) == false)
		{
			std::string tmpIp = m_snmpManager->getIpAddress();
			if(tmpIp.compare(tmpIPA) == 0)
				m_snmpManager->setIpAddress(tmpIPB);
			else
				m_snmpManager->setIpAddress(tmpIPA);
			if(m_snmpManager->setFWMAction(1) == false)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " SET Failed ! " << std::endl;
				if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - SET Failed ! ",LOG_LEVEL_ERROR);
				result = SNMP_FAILURE;
				qEventInfo = SNMP_SET_NOT_OK;
				m_context->m_slotUpgOnGoing = -1;
				{ // log
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of setFWMAction(1)== %d",__FUNCTION__, __LINE__, result);
					if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				}
			}
			else
			{
				result = EXECUTED;
				qEventInfo = SNMP_SET_OK;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
				if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - SNMP SET OK SLOT ! ",LOG_LEVEL_DEBUG);
				m_context->m_slotUpgOnGoing = atoi(slot.c_str());
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;

			}
		}
		else
		{
			result = EXECUTED;
			qEventInfo = SNMP_SET_OK;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - SNMP SET OK SLOT ! ",LOG_LEVEL_DEBUG);
			m_context->m_slotUpgOnGoing = atoi(slot.c_str());
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;
		}
	}

	{ // log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of SNMP settings == %d",__FUNCTION__, __LINE__,result);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1, traceChar);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	m_context->setEvent(qEventInfo);

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	return result;

}

// Class FIXS_CCH_FwUpgrade::StateOngoing

FIXS_CCH_FwUpgrade::StateOngoing::StateOngoing()
{
	m_id = STATE_ONGOING;
}

FIXS_CCH_FwUpgrade::StateOngoing::~StateOngoing()
{
}

int FIXS_CCH_FwUpgrade::StateOngoing::fwUpgradeResultTrap (int result)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	std::string t_info; // log
	int exitRes = 0;
	FIXS_CCH_FwUpgrade::FwEventId qEventInfo = DEFAULT_NO_EVENT;

	if (result == 1)  //Result 1 OK
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " TRAP OK " << std::endl;
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - TRAP OK !!!",LOG_LEVEL_DEBUG);
		t_info = "OK";
		exitRes = FWUPG_MIBTRAP_OK;
		qEventInfo = SNMP_TRAP_OK;
	}
	else if (result == 0) ////Result 0 NOT OK
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " TRAP Error: fileError;error when accessing or reading FW file " << std::endl;
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - TRAP Error: fileError;error when accessing or reading FW file ",LOG_LEVEL_ERROR);
		t_info = "TRAP Error: fileError;error when accessing or reading FW file";
		exitRes = FWUPG_MIBTRAP_FILE_ERR;
		qEventInfo = SNMP_TRAP_NOT_OK;
	}
	else if (result == 2)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " otherError " << std::endl;
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - TRAP Error: otherError ",LOG_LEVEL_ERROR);
		t_info = "TRAP otherError";
		exitRes = FWUPG_MIBTRAP_OTHER_ERR;
		qEventInfo = SNMP_TRAP_NOT_OK;
	}
	else if (result == 3)/////Result 3, UNDEFINED STATE
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " otherError " << std::endl;
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - TRAP Error: UNDEFINED STATE ",LOG_LEVEL_ERROR);
		t_info = "Ongoing Pending error";
		exitRes = FWUPG_MIBTRAP_ONGOING_ERR;
		qEventInfo = SNMP_TRAP_PENDING_NOT_OK;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setFwUpg executed " << std::endl;
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_FwUpgrade - TRAP OK: setFwUpg executed ",LOG_LEVEL_DEBUG);
		t_info = "TRAP: setFwUpg executed";
		exitRes = FWUPG_MIBTRAP_OTHER_ERR;
		qEventInfo = SNMP_TRAP_NOT_OK;
	}


	{ // log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] result == %d, %s",__FUNCTION__, __LINE__, result, t_info.c_str());
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1, traceChar);
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}


	m_context->setEvent(qEventInfo);

	return exitRes;

}

int FIXS_CCH_FwUpgrade::StateOngoing::upgrade (std::string valueToSet, std::string slot, std::string tmpIPA, std::string tmpIPB)
{
	UNUSED(valueToSet);
	UNUSED(slot);
	UNUSED(tmpIPA);
	UNUSED(tmpIPB);
	std::cout << "WRONG EVENT IN THIS STATE"<< std::endl;
	return UPGRADE_ALREADY_ONGOING;//FWUPG_ONGOING;

}

// Additional Declarations


FIXS_CCH_FwUpgrade::StateUndefined::StateUndefined()
{
	m_id = STATE_UNDEFINED;
}


FIXS_CCH_FwUpgrade::StateUndefined::~StateUndefined()
{
}


int FIXS_CCH_FwUpgrade::StateUndefined::fwUpgradeResultTrap (int result)
{
	int res = 0;
	UNUSED(result);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "WRONG EVENT IN THIS STATE"<< std::endl;
	res = WRONG_OPTION;
	return res;

}

int FIXS_CCH_FwUpgrade::StateUndefined::upgrade (std::string valueToSet, std::string slot, std::string tmpIPA, std::string tmpIPB)
{
	std::cout<<"DBG: "<< __FUNCTION__ <<" @ "<< __LINE__ <<" UPGRADE UNDEFINED SLOT: " << slot.c_str() <<std::endl;

	int result = 0;
	FIXS_CCH_FwUpgrade::FwEventId qEventInfo = DEFAULT_NO_EVENT;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " perform set with slot " << slot <<" -- valueToSet " << valueToSet << std::endl;
	std::cout<<"DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT: " << slot.c_str() <<std::endl;

	std::string actState = "";
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(tmpIPA);

	if(m_snmpManager->getBladeRevisionState(&actState)== false)
	{
		m_snmpManager->setIpAddress(tmpIPB);
		if(m_snmpManager->getBladeRevisionState(&actState)== false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " SET Failed ! " << std::endl;
			result = SNMP_FAILURE;
			qEventInfo = SNMP_SET_NOT_OK;
			m_context->m_slotUpgOnGoing = -1;
		}
	}

	if( result == SNMP_FAILURE)
	{
		{ // log
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of getBladeRevisionState() == %d",__FUNCTION__, __LINE__,result);
			if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1, traceChar);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
		}
		m_context->setEvent(qEventInfo);

		if (m_snmpManager != NULL)
		{
			delete m_snmpManager;
			m_snmpManager = NULL;
		}

		return result;
	}
	//save BladeRevisionState
	FIXS_CCH_FSMDataStorage::getInstance()->saveFwRevisionState(m_context->m_percistance_dn,actState);

	std::string url;
	std::string sub = "169";
	std::string apHostName = CCH_Util::GetAPHostName();
	std::string apNode="";
	if (apHostName.length() > 0)
	{
		apNode = apHostName.substr(apHostName.length() -1, 1);
	}

	m_snmpManager->setIpAddress(tmpIPA);
	if (tmpIPA.length() > 10)
	{
		std::cout<< "tmpIPA.length: "<< tmpIPA.length()<<std::endl;
		sub = tmpIPA.substr(8,3);
	}
	if (apNode.length() <= 0)
	{
		//Old method
		url = "tftp://192.168."+ sub + ".33/" + valueToSet;
	}
	else
	{
		if (apNode == "A")
		{
			url = "tftp://192.168."+ sub + ".1/" + valueToSet;
		}
		else if (apNode == "B")
		{
			url = "tftp://192.168."+ sub + ".2/" + valueToSet;
		}
		else
		{
			url = "tftp://192.168."+ sub + ".33/" + valueToSet;
		}
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Url : " << url.c_str() << std::endl;

	if ( m_snmpManager->setFWMActionURL(url) == false)
	{
		m_snmpManager->setIpAddress(tmpIPB);
		if (tmpIPB.length() > 10)
		{
			std::cout<< "tmpIPB.length: "<< tmpIPB.length()<<std::endl;
			sub = tmpIPB.substr(8,3);
		}
		if (apNode.length() <= 0)
		{
			//Old method
			url = "tftp://192.168."+ sub + ".33/" + valueToSet;
		}
		else
		{
			if (apNode == "A")
			{
				url = "tftp://192.168."+ sub + ".1/" + valueToSet;
			}
			else if (apNode == "B")
			{
				url = "tftp://192.168."+ sub + ".2/" + valueToSet;
			}
			else
			{
				url = "tftp://192.168."+ sub + ".33/" + valueToSet;
			}
		}
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Url : " << url.c_str() << std::endl;
		if ( m_snmpManager->setFWMActionURL(url) == false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " SET Failed ! " << std::endl;
			result = SNMP_FAILURE;
			qEventInfo = SNMP_SET_NOT_OK;
			m_context->m_slotUpgOnGoing = -1;

			{ // log
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of setFWMActionURL(url)== %d",__FUNCTION__, __LINE__,result);
			}

		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " setFwUpg executed " << std::endl;
			if(m_snmpManager->setFWMAction(1)== false)
			{
				std::string tmpIp = m_snmpManager->getIpAddress();
				if(tmpIp.compare(tmpIPA) == 0)
					m_snmpManager->setIpAddress(tmpIPB);
				else
					m_snmpManager->setIpAddress(tmpIPA);
				if(m_snmpManager->setFWMAction(1)== false)
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " SET Failed ! " << std::endl;
					result = SNMP_FAILURE;
					qEventInfo = SNMP_SET_NOT_OK;
					m_context->m_slotUpgOnGoing = -1;
					{ // log
						char traceChar[512] = {0};
						snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of setFWMAction(1)== %d",__FUNCTION__, __LINE__,result);
					}
				}
				else
				{
					result = EXECUTED;
					qEventInfo = SNMP_SET_OK;
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
					m_context->m_slotUpgOnGoing = atoi(slot.c_str());
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;

				}
			}
			else
			{
				result = EXECUTED;
				qEventInfo = SNMP_SET_OK;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
				m_context->m_slotUpgOnGoing = atoi(slot.c_str());
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;
			}
		}
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " setFwUpg executed " << std::endl;

		if(m_snmpManager->setFWMAction(1)== false)
		{
			std::string tmpIp = m_snmpManager->getIpAddress();
			if(tmpIp.compare(tmpIPA) == 0)
				m_snmpManager->setIpAddress(tmpIPB);
			else
				m_snmpManager->setIpAddress(tmpIPA);
			if(m_snmpManager->setFWMAction(1)== false)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
						<< " SET Failed ! " << std::endl;
				result = SNMP_FAILURE;
				qEventInfo = SNMP_SET_NOT_OK;
				m_context->m_slotUpgOnGoing = -1;
				{ // log
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of setFWMAction(1) == %d",__FUNCTION__, __LINE__,result);
				}
			}
			else
			{
				result = EXECUTED;
				qEventInfo = SNMP_SET_OK;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
				m_context->m_slotUpgOnGoing = atoi(slot.c_str());
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;

			}
		}
		else
		{
			result = EXECUTED;
			qEventInfo = SNMP_SET_OK;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP SET OK SLOT:" << slot.c_str() << std::endl;
			m_context->m_slotUpgOnGoing = atoi(slot.c_str());
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SLOT ONGOING:" << m_context->m_slotUpgOnGoing << std::endl;
		}

	}

	{ // log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]Result of SNMP settings == %d",__FUNCTION__, __LINE__,result);
		if (m_context->traceObj->ACS_TRA_ON()) m_context->traceObj->ACS_TRA_event(1, traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	m_context->setEvent(qEventInfo);

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	return result;

}

// Additional Declarations



// Class FIXS_CCH_FwUpgrade::SaxHandler

FIXS_CCH_FwUpgrade::SaxHandler::SaxHandler (const std::string &xmlFilePath, CCH_Util::fwRemoteData &fwData)
: m_fwData(fwData)
{
	UNUSED(xmlFilePath);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Entered constructor of  SaxHandler" << std::endl;
	traceSax = new ACS_TRA_trace("CCH FW SaxHandler");
	m_name = false;
	m_cxc = false;
	m_rState = false;

}


FIXS_CCH_FwUpgrade::SaxHandler::~SaxHandler()
{
	delete(traceSax);
}


void FIXS_CCH_FwUpgrade::SaxHandler::startElement (const XMLCh* const  name, AttributeList &attributes)
{
	UNUSED(attributes);

	char* elem = XMLString::transcode(name);
	std::string element(elem);

	if (element == "name")
	{
		m_name = true;
	}

	if (element == "cxc")
	{
		m_cxc = true;
	}

	if (element == "cxc_rstate")
	{
		m_rState = true;
	}

	if (element == "product_number")
	{
		m_cxp = true;
	}

	if (element == "cxp_rstate")
	{
		m_cxp_rState = true;
	}
}


void FIXS_CCH_FwUpgrade::SaxHandler::warning (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceSax->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "xml error: warning method got called");
		traceSax->ACS_TRA_event(1, tmpStr);
	}

}

void FIXS_CCH_FwUpgrade::SaxHandler::error (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceSax->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "xml error: error method got called");
		traceSax->ACS_TRA_event(1, tmpStr);
	}
}

void FIXS_CCH_FwUpgrade::SaxHandler::fatalError (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceSax->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "xml error: fatalError method got called");
		traceSax->ACS_TRA_event(1, tmpStr);
	}

}

void FIXS_CCH_FwUpgrade::SaxHandler::characters (const XMLCh* const  chars, const XMLSize_t length)
{
	UNUSED(length);
	if (m_name)
	{
		std::string name = XMLString::transcode(chars);

		ACS_APGCC::toUpper(name);
		m_fwData.fwType[fwInfoCount]= name;
		m_name = false;
	}

	if (m_cxc)
	{
		std::string cxc = XMLString::transcode(chars);
		ACS_APGCC::toUpper(cxc);
		m_fwData.fwVersion[fwInfoCount]= cxc;
		m_cxc = false;
	}

	if (m_rState)
	{
		std::string rState = XMLString::transcode(chars);
		ACS_APGCC::toUpper(rState);
		m_fwData.fwVersion[fwInfoCount]+= "_" + rState;
		fwInfoCount++;
		m_rState = false;
	}

	if (m_cxp)
	{
		m_fwData.productno = XMLString::transcode(chars);
		ACS_APGCC::toUpper(m_fwData.productno);
		m_cxp = false;
	}

	if (m_cxp_rState)
	{
		m_fwData.cxp_rState = XMLString::transcode(chars);
		ACS_APGCC::toUpper(m_fwData.cxp_rState);
		m_cxp_rState = false;
	}
}
// Additional Declarations



// Class FIXS_CCH_FwUpgrade

FIXS_CCH_FwUpgrade::FIXS_CCH_FwUpgrade()
: m_trapTimer(0), m_trapTimer_FiveM(0),m_running(0),m_boardId(0), m_magazine(0), m_cmdEvent(0),
  m_shutdownEvent(0), m_eventQueue(), m_snmpSyncPtr(0), m_IP_EthA(0), m_IP_EthB(0),
  m_timeOutTrapEvent(0), m_timeOutTrapEventFiveMin(0), m_slot(0)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	m_idle.setContext(this);
	m_ongoing.setContext(this);
	m_undefined.setContext(this);
	m_currentState = &m_idle;
	m_slotUpgOnGoing = -1;
	m_ipMaster = "";
	m_slotMaster = "";
	m_lastUpgradeDate = "-";
	m_lastUpgradeTime = "-";
	m_lastFWUpgradeResult = "-";
	m_lastFWUpgradeReason = "-";
}

FIXS_CCH_FwUpgrade::FIXS_CCH_FwUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string IP_EthA, std::string IP_EthB, std::string dn)
:  m_trapTimer(0),m_trapTimer_FiveM(0),m_running(0), m_boardId(boardId), m_magazine(magazine), m_cmdEvent(0),
   m_shutdownEvent(0), m_eventQueue(), m_snmpSyncPtr(0), m_IP_EthA(IP_EthA), m_IP_EthB(IP_EthB),
   m_timeOutTrapEvent(0), m_timeOutTrapEventFiveMin(0), m_slot(slot), m_otherBlade_dn(dn), m_percistance_dn("")
{
	//note that:
	// 1) this state machine is related to each magazine
	// 2) the Board IP addresses are got from fw_upg command

	UNUSED(installedPackage);

	m_jtpHandlerFw[0] = NULL;
	m_jtpHandlerFw[1] = NULL;
	m_idle.setContext(this);
	m_ongoing.setContext(this);
	m_undefined.setContext(this);
	m_currentState = &m_idle;
	m_slotUpgOnGoing = -1;
	m_ipMaster = "";
	m_slotMaster = "";
	m_lastUpgradeDate = "-";
	m_lastUpgradeTime = "-";
	m_lastFWUpgradeResult = "-";
	m_lastFWUpgradeReason = "-";

	m_percistance_dn = IMM_Interface::getDnBladePersistance(magazine,slot);

	traceObj = new ACS_TRA_trace("FIXS_CCH_FwUpgrade");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	std::string sMag("");
	CCH_Util::ulongToStringMagazine(magazine,sMag);

	{//log
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, " -  FW Upgrade: magazine [%s] slot [%d] on instance [%p]-  ",sMag.c_str(),slot,this );
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
	}
	m_StopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);

}

FIXS_CCH_FwUpgrade::~FIXS_CCH_FwUpgrade()
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	if (m_cmdEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_cmdEvent);
	}

	if (m_StopEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_StopEvent);
	}

	if ( m_timeOutTrapEvent && (m_timeOutTrapEvent != -1 /*INVALID_HANDLE_VALUE*/) )
	{
		::close(m_timeOutTrapEvent);
	}

	if ( m_timeOutTrapEventFiveMin && (m_timeOutTrapEventFiveMin != -1 /*INVALID_HANDLE_VALUE*/) )
	{
		::close(m_timeOutTrapEventFiveMin);
	}

	if (m_jtpHandlerFw[0])
	{
		if (m_jtpHandlerFw[0]->isRunningThread()) m_jtpHandlerFw[0]->stop();
	}

	if (m_jtpHandlerFw[1])
	{
		if (m_jtpHandlerFw[1]->isRunningThread()) m_jtpHandlerFw[1]->stop();
	}

	int retry = 0;
	if (m_jtpHandlerFw[0])
	{
		while (m_jtpHandlerFw[0]->isRunningThread() && retry < 5)
		{
			cout<< " m_jtpHandlerFw[0] THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		delete m_jtpHandlerFw[0];
		m_jtpHandlerFw[0] = NULL;
	}

	retry = 0;
	if (m_jtpHandlerFw[1])
	{
		while (m_jtpHandlerFw[1]->isRunningThread() && retry < 5)
		{
			cout<< " m_jtpHandlerFw[1] THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		delete m_jtpHandlerFw[1];
		m_jtpHandlerFw[1] = NULL;
	}

	delete (traceObj);

	FIXS_CCH_logging = 0;
}

int FIXS_CCH_FwUpgrade::upgrade (std::string valueToSet, std::string slot, std::string tmpIPA, std::string tmpIPB)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	int result = m_currentState->upgrade(valueToSet, slot, tmpIPA, tmpIPB);
	return result;
}

int FIXS_CCH_FwUpgrade::fwUpgradeResultTrap (int result)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	int code = m_currentState->fwUpgradeResultTrap(result);
	return code;
}

std::string FIXS_CCH_FwUpgrade::getStateName ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
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

void FIXS_CCH_FwUpgrade::switchState (FIXS_CCH_FwUpgrade::FwEventId eventId)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::string t_currState("");
	std::string t_nextState("");
	std::string t_event("");
	std::string t_type(""); //temporay variables used for logging
	const int Number_Of_Events = 2;	
	const int WAIT_TIMEOUT = -2;
	m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
	FIXS_CCH_EventHandle handlArr[Number_Of_Events] = {m_shutdownEvent, m_StopEvent};
	
	//Checking if it is PENDING_NOT_OK
	if(eventId == SNMP_TRAP_NOT_OK)
	{
		int status = -1;
		stringstream sSlot;
		sSlot << m_slotUpgOnGoing;

		CCH_Util::EnvironmentType m_environment = FIXS_CCH_DiskHandler::getEnvironment();

		int env_switch = -1;
		int sw_version = 0;
		if ((m_environment == CCH_Util::SINGLECP_SMX) || (m_environment == CCH_Util::MULTIPLECP_SMX))
			env_switch = 2;
		else
		{
			if((m_slotUpgOnGoing == 0) || (m_slotUpgOnGoing == 25))
				env_switch = 0;
			else if ((m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
			{
				int _result =0;
				_result = CCH_Util::getCmxSoftwareVersion (m_magazine,static_cast<unsigned long>(m_slotUpgOnGoing), sw_version);
				if((_result == 0) && (sw_version == 4))
					env_switch = 1;
				else
					env_switch = 0;
			}
		}
		status = checkFWStatus(m_IP_EthA, m_IP_EthB, sSlot.str(),env_switch);


		if(status == 2)
		{
			eventId = SNMP_TRAP_PENDING_NOT_OK;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"Setting the event to SNMP_TRAP_PENDING_NOT_OK"<< std::endl;
		}
	}

	{ // log
		switch(m_currentState->getStateId())
		{
		case STATE_ONGOING:
			t_currState = "ONGOING";
			break;
		case STATE_IDLE:
			t_currState = "IDLE";
			break;
		case STATE_UNDEFINED:
			t_currState = "UNDEFINED";
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
		case SNMP_TRAP_PENDING_NOT_OK:
			t_event = "SNMP_TRAP_PENDING_NOT_OK";
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
		case SNMP_TRAP_PENDING_NOT_OK:
		{

			//Stop JTP Maintenance windows on CP
			if ((m_slotUpgOnGoing == 0) || (m_slotUpgOnGoing == 25) || (m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" Stop JTP Maintenance windows - slotOngoing " << m_slotUpgOnGoing << std::endl;
				if (m_jtpHandlerFw[0]) m_jtpHandlerFw[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE);
			}
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" JTP Maintenance windows not enabled for slot " << m_slotUpgOnGoing << std::endl;
			}
			sleep(1);
			stopJtpThread();
			
			if(eventId == SNMP_TRAP_PENDING_NOT_OK)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" STATE_ONGOING change in STATE_UNDEFINED" << std::endl;
				m_currentState = &m_undefined;

				{ // log
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]State changing from ONGOING to UNDEFINED",__FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				}
			}
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" STATE_ONGOING change in STATE_IDLE" << std::endl;
				m_currentState = &m_idle;

				{ // log
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]State changing from ONGOING to IDLE",__FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				}

			}


			{
				string revState("");
				string storeRev("");
				bool readRev = false;
				bool readReason = false;
				bool stopRetry = false;
				bool isSmxOrCmx4= false;
				bool retVal =true;
				int actionStatus = -1;
				bool isCmx4 = false;

				FIXS_CCH_SNMPManager *m_snmpManager = NULL;
				m_snmpManager = new FIXS_CCH_SNMPManager(m_IP_EthA);
				CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();

				if((m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
				{
					int _result =0;
					int sw_version = 0;
					_result = CCH_Util::getCmxSoftwareVersion (m_magazine,static_cast<unsigned long>(m_slotUpgOnGoing), sw_version);
					if((_result == 0) && (sw_version == 4))
						isCmx4 = true;	

				}
				
				if ((env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX) || isCmx4)
				{
					retVal = waitTillFwmActionResultFetched(m_snmpManager,stopRetry,isCmx4,actionStatus);
					isSmxOrCmx4 = true;

				}

				if((isSmxOrCmx4 == false ) || (retVal == true)) //in case not SMx or readActionResult success in SMX env.
				{	
					if(m_snmpManager->getBladeRevisionState(&revState))
					{
						readRev = true;
					}
					else
					{
						m_snmpManager->setIpAddress(m_IP_EthB);
						if(m_snmpManager->getBladeRevisionState(&revState))
							readRev = true;
					}
					int countRetry = 0;
					if(eventId != SNMP_TRAP_PENDING_NOT_OK) // means not UNDEFINED state
					{					
						while ((readRev == false ) && (countRetry < (isCmx4? 50:25)) && (stopRetry == false))
						{					
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "wait for 3 second and then retry getBladeRevisionState.....\n";
							int eventIndex=FIXS_CCH_Event::WaitForEvents(Number_Of_Events,handlArr,(isCmx4 ? 4000:3000));
							switch(eventIndex)
							{												
								case (WAIT_TIMEOUT):  // Time out
								{
									m_snmpManager->setIpAddress(m_IP_EthA);
									if(m_snmpManager->getBladeRevisionState(&revState))
									{
										readRev = true;
									}
									else
									{
										m_snmpManager->setIpAddress(m_IP_EthB);
										if(m_snmpManager->getBladeRevisionState(&revState))
										readRev = true;
									}
									countRetry++;
									break;
								}
								default:
								{
									char traceChar[512] = {0};
									snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]default ",__FUNCTION__, __LINE__); 							
									stopRetry = true;
									break;
								}
							}//End of switch						
						}
					}
				}
					
				if(((isSmxOrCmx4 == false) && (readRev == true)) || ((isSmxOrCmx4 == true) && (retVal == true) && (readRev == true)))
				{
					
					size_t found = string::npos;
					found = revState.find_first_of(" ");
					if(found!=string::npos)
					{
						revState = revState.substr(0,found);
					}
					found = revState.find("+");
					if(isSmxOrCmx4 == true)
					{
						if(actionStatus == 1)
							found = 2; // to represent still upgrade ongoing
						else
							found = string::npos;
					} 
					if(found == string::npos) //+ sign not found
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade not ongoing" << std::endl;
						//read from persistent file
						FIXS_CCH_FSMDataStorage::getInstance()->readFwRevisionState(m_percistance_dn,storeRev);
						found = storeRev.find_first_of("+"); //Just to eliminate a plus sign in stored revision
						if(found!=string::npos)
						{
							storeRev = storeRev.substr(0,found);
						}
					
						// for SMX product revision will not change after RFU upgrade
						if(isSmxOrCmx4)
						{
							if(eventId == SNMP_TRAP_OK)
								m_lastFWUpgradeResult = "OK";
							else
								m_lastFWUpgradeResult = "NOT OK";
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "m_lastFWUpgradeResult: " << m_lastFWUpgradeResult.c_str() << std::endl; 
						}
						else
						{
							if(strcmp(revState.c_str(),storeRev.c_str()) == 0 )
							{
								m_lastFWUpgradeResult = "NOT OK";
								std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
													" m_lastFWUpgradeResult_1 = " << m_lastFWUpgradeResult << std::endl;
							}
							else
							{
								m_lastFWUpgradeResult = "OK";
							}
						}
					}
					else{ //+ sign is found and that means still on going
						m_lastFWUpgradeResult = "NOT OK";
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
										" m_lastFWUpgradeResult_2 = " << m_lastFWUpgradeResult << std::endl;
					}
				}
				else{
					m_lastFWUpgradeResult = "NOT OK";
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
							" m_lastFWUpgradeResult_3 = " << m_lastFWUpgradeResult << std::endl;
				}
				
				string actState;
				if(stopRetry==false)
				{
					m_snmpManager->setIpAddress(m_IP_EthA);
					if(m_snmpManager->getFwmActionStatus(&actState))
					{
						m_lastFWUpgradeReason = actState;
						readReason = true;
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
													" m_lastFWUpgradeReason_1 = " << m_lastFWUpgradeReason << std::endl;
					}
					else
					{
						m_snmpManager->setIpAddress(m_IP_EthB);
						if(m_snmpManager->getFwmActionStatus(&actState))
						{
							m_lastFWUpgradeReason = actState;
							readReason = true;
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
														" m_lastFWUpgradeReason_2 = " << m_lastFWUpgradeReason << std::endl;
						}
					}
				}

				int countRead = 0;
				if(eventId != SNMP_TRAP_PENDING_NOT_OK) // means not UNDEFINED state
				{
					while((readReason == false) && (countRead < (isCmx4 ? 50:25)) && (stopRetry == false))
					{					
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "wait for 3 sec and try to fetch FwmActionStatus again...\n";
						int eventIndex=FIXS_CCH_Event::WaitForEvents(Number_Of_Events,handlArr,(isCmx4 ? 4000:3000));
						switch(eventIndex)
						{
						case (WAIT_TIMEOUT):  // Time out
						{
							m_snmpManager->setIpAddress(m_IP_EthA);
							if(m_snmpManager->getFwmActionStatus(&actState))
							{
								readReason = true;
								m_lastFWUpgradeReason = actState;
								std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
																	" m_lastFWUpgradeReason_3 = " << m_lastFWUpgradeReason << std::endl;
							}
							else
							{
								m_snmpManager->setIpAddress(m_IP_EthB);
								if(m_snmpManager->getFwmActionStatus(&actState))
								{
									readReason = true;
									m_lastFWUpgradeReason = actState;
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
																		" m_lastFWUpgradeReason_4 = " << m_lastFWUpgradeReason << std::endl;
								}
							}
							countRead++;
							break;
						}
						default:
						{
							char traceChar[512] = {0};
							snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] default case",__FUNCTION__, __LINE__);
							stopRetry=true;
							break;
						}
						}						
					}						
				}

				CCH_Util::EnvironmentType m_environment = FIXS_CCH_DiskHandler::getEnvironment();
				if ((m_environment == CCH_Util::SINGLECP_SMX) || (m_environment == CCH_Util::MULTIPLECP_SMX))
				{
					m_snmpManager->resetSwitch(m_slotUpgOnGoing);
				}

				if (m_snmpManager != NULL)
				{
					delete m_snmpManager;
					m_snmpManager = NULL;
				}

				if (!readReason) m_lastFWUpgradeReason = "SNMP Failure";

				std::cout << __FUNCTION__ << "@" << __LINE__ << " m_lastFWUpgradeResult : " <<m_lastFWUpgradeResult.c_str()<< std::endl;
				std::cout << __FUNCTION__ << "@" << __LINE__ << " m_lastFWUpgradeReason : " <<m_lastFWUpgradeReason.c_str()<< std::endl;

				//load xml and sh file
				std::string shPathFile("");
				std::string xmlPathName("");
				FIXS_CCH_FSMDataStorage::getInstance()->readFwShFileName(m_percistance_dn,shPathFile);
				FIXS_CCH_FSMDataStorage::getInstance()->readFwXmlFileName(m_percistance_dn,xmlPathName);

				//remove files
				FIXS_CCH_DiskHandler::deleteFile(shPathFile.c_str());
				FIXS_CCH_DiskHandler::deleteFile(xmlPathName.c_str());

				//cleanup xml and sh file
				shPathFile.clear();
				xmlPathName.clear();
				FIXS_CCH_FSMDataStorage::getInstance()->saveFwShFileName(m_percistance_dn,shPathFile);
				FIXS_CCH_FSMDataStorage::getInstance()->saveFwXmlFileName(m_percistance_dn,xmlPathName);

				m_slotUpgOnGoing = -1;
				m_ipMaster = "";
				m_slotMaster = "";
			}

			//stop thread
			stop();
		}

		break;

		default: break;
		}
		break;

		//STARTING STATE IDLE
		case STATE_IDLE:
		case STATE_UNDEFINED:
		default:
			switch(eventId)
			{
			case SNMP_SET_OK: // IDLE, SNMP_SET_OK == > ONGOING
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"STATE_IDLE/STATE_UNDEFINED change to STATE_ONGOING" << std::endl;
				m_currentState = &m_ongoing;
				{ // log
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] FW UPGRADE State changing from IDLE/UNDEFINED to ONGOING",__FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				}
				//Start JTP Maintenance windows on CP
				if ((m_slotUpgOnGoing == 0) || (m_slotUpgOnGoing == 25) || (m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" Start JTP Maintenance windows - slotOngoing " << m_slotUpgOnGoing << std::endl;
					unsigned char slotOngoing = (unsigned char)m_slotUpgOnGoing;
					printf("DBG: [%s@%d] - +++++SLOTONGOING FOR MAINTENANCEWINDOWS: %x \n", __FUNCTION__, __LINE__, slotOngoing);
					if (m_jtpHandlerFw[0]) m_jtpHandlerFw[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IMBSW_START_UPGRADE,slotOngoing);
					//if (m_jtpHandlerFw[1]) m_jtpHandlerFw[1]->maintenanceWindow(FIXS_CCH_JTPHandler::SCXSW_START_UPGRADE,slotOngoing);
				}
				else
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
							" JTP Maintenance windows not enabled for slot " << m_slotUpgOnGoing << std::endl;
				}
				break;

			case SNMP_SET_NOT_OK: // IDLE, SNMP_SET_OK == > ONGOING
			{
				string revState("");
				string storeRev("");
				bool readRev = false;
				bool readReason = false;
				FIXS_CCH_SNMPManager *m_snmpManager = NULL;
				m_snmpManager = new FIXS_CCH_SNMPManager(m_IP_EthA);

				if(m_snmpManager->getBladeRevisionState(&revState))
				{
					readRev = true;
				}
				else
				{
					m_snmpManager->setIpAddress(m_IP_EthB);
					if(m_snmpManager->getBladeRevisionState(&revState))
						readRev = true;
				}

				if(readRev == true)
				{
					size_t found = 0;
					found = revState.find_first_of(" ");
					if(found!=string::npos)
					{
						revState = revState.substr(0,found);
					}
					found = revState.find("+");
					if(found == string::npos) //+ sign not found
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade ongoing" << std::endl;
						//read from persistent file
						FIXS_CCH_FSMDataStorage::getInstance()->readFwRevisionState(m_percistance_dn,storeRev);
						found = storeRev.find_first_of("+"); //Just to eliminate a plus sign in stored revision
						if(found!=string::npos)
						{
							storeRev = storeRev.substr(0,found);
						}
						if( strcmp(revState.c_str(),storeRev.c_str()) == 0 )
						{
							m_lastFWUpgradeResult = "NOT OK";
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
									" m_lastFWUpgradeResult_4 = " << m_lastFWUpgradeResult << std::endl;
						}
						else
						{
							m_lastFWUpgradeResult = "OK";
						}
					}
					else{ //+ sign is found and that means still on going
						m_lastFWUpgradeResult = "NOT OK";
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
								" m_lastFWUpgradeResult_5 = " << m_lastFWUpgradeResult << std::endl;
					}
				}
				else{
					m_lastFWUpgradeResult = "NOT OK";
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
							" m_lastFWUpgradeResult_6 = " << m_lastFWUpgradeResult << std::endl;
				}

				string actState;
				m_snmpManager->setIpAddress(m_IP_EthA);
				if(m_snmpManager->getFwmActionStatus(&actState))
				{
					m_lastFWUpgradeReason = actState;
					readReason = true;
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
														" m_lastFWUpgradeReason_11 = " << m_lastFWUpgradeReason << std::endl;
				}
				else
				{
					m_snmpManager->setIpAddress(m_IP_EthB);
					if(m_snmpManager->getFwmActionStatus(&actState))
					{
						m_lastFWUpgradeReason = actState;
						readReason = true;
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
															" m_lastFWUpgradeReason_22 = " << m_lastFWUpgradeReason << std::endl;
					}
				}

				if (m_snmpManager != NULL)
				{
					delete m_snmpManager;
					m_snmpManager = NULL;
				}

				if (!readReason) m_lastFWUpgradeReason = "SNMP Failure";

				std::cout << __FUNCTION__ << "@" << __LINE__ << " m_lastFWUpgradeResult : " <<m_lastFWUpgradeResult.c_str()<< std::endl;
				std::cout << __FUNCTION__ << "@" << __LINE__ << " m_lastFWUpgradeReason : " <<m_lastFWUpgradeReason.c_str()<< std::endl;

				//load xml and sh file
				std::string shPathFile("");
				std::string xmlPathName("");
				FIXS_CCH_FSMDataStorage::getInstance()->readFwShFileName(m_percistance_dn,shPathFile);
				FIXS_CCH_FSMDataStorage::getInstance()->readFwXmlFileName(m_percistance_dn,xmlPathName);

				//remove files
				FIXS_CCH_DiskHandler::deleteFile(shPathFile.c_str());
				FIXS_CCH_DiskHandler::deleteFile(xmlPathName.c_str());

				//cleanup xml and sh file
				shPathFile.clear();
				xmlPathName.clear();
				FIXS_CCH_FSMDataStorage::getInstance()->saveFwShFileName(m_percistance_dn,shPathFile);
				FIXS_CCH_FSMDataStorage::getInstance()->saveFwXmlFileName(m_percistance_dn,xmlPathName);

				m_slotUpgOnGoing = -1;
				m_ipMaster = "";
				m_slotMaster = "";


				//stop thread
				stop();

			}

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
		case STATE_UNDEFINED:
			t_nextState = "UNDEFINED";
			break;
		default:
			t_nextState = "UNKNOWN_STATE";
		}
		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1,"[FW UPGRADE on MAG:%s, SLOT:%u] CURRENT STATE: %s, EVENT RECEIVED: %s, NEXT STATE: %s",magStr.c_str(), m_slot, t_currState.c_str(), t_event.c_str(), t_nextState.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1, traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	cout<< __FUNCTION__ << "@" << __LINE__ << "****************************** Next State = "<< (((m_currentState->getStateId() == STATE_IDLE) ? "IDLE" : ((m_currentState->getStateId() == STATE_UNDEFINED) ? "UNDEFINED" : "ONGOING"))) << endl;
	saveStatus();

}

void FIXS_CCH_FwUpgrade::setSnmpStatus (bool status)
{
	m_currentState->setSnmpStatus(status);
}

int FIXS_CCH_FwUpgrade::open (void *args)
{
	int result = 0;

	UNUSED(args);

	cout << __FUNCTION__ << "@" << __LINE__ << " fw upgrade: try to activate..." << endl;

	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"FW UPGRADE Starting thread on instance [%p]. \n",this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}
	m_exit = false;

	activate();


	return result;
}

int FIXS_CCH_FwUpgrade::svc ()
{
	int result = 0;
	int timerCnt = 0;

	sleep(1);

	initOnStart();
	if (m_exit) return result;

	if (m_currentState->getStateId() == STATE_ONGOING)
	{
		//check if this slot is already ongoing
		int status = -1;
		stringstream sSlot;
		sSlot << m_slotUpgOnGoing;
		CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
		int env_switch = -1;
		if ((env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX))
			env_switch = 2;
		else
		{
			if((m_slotUpgOnGoing == 0) || (m_slotUpgOnGoing == 25))
				env_switch = 0;
			else if ((m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
			{
				int _result =0;
				int sw_version = 0;
				_result = CCH_Util::getCmxSoftwareVersion (m_magazine,static_cast<unsigned long>(m_slotUpgOnGoing), sw_version);
				if((_result == 0) && (sw_version == 4))
					env_switch = 1;
				else
					env_switch = 0; 
			}
                }

		status = checkFWStatus(m_IP_EthA, m_IP_EthB, sSlot.str(),env_switch);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" verify board status " << sSlot.str() << " - status = " << status << std::endl;

		if (status == 1)
		{
			switchState(SNMP_TRAP_OK);
		}
		else //now is stopped !
		{
			startTrapEventTimer();
		}
	}

	m_running = true;

	//loop
	EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent, m_cmdEvent, m_timeOutTrapEvent, m_timeOutTrapEventFiveMin, m_StopEvent};

	const int WAIT_FAILED = -1;
	const int WAIT_TIMEOUT = -2;
	const int WAIT_OBJECT_0 = 0;
	const int Shutdown_Event_Index = 0;
	const int Command_Event_Index = 1;
	const int TimeoutTrap_Event_Index = 2;
	const int TimeoutTrap_Event_Index_FiveMin = 3;
	const int Stop_Event_Index = 4;

	while ( ! m_exit ) // Set to true when the thread shall exit
	{
		fflush(stdout);
		// Wait for shutdown and command events
		int returnValue=FIXS_CCH_Event::WaitForEvents(Number_Of_Events, handleArr, Safe_Timeout);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" WaitForEvents: "<< returnValue << endl;
		switch (returnValue)
		{
		case WAIT_FAILED:   // Event failed
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects", __FUNCTION__, __LINE__);
				if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1, tmpStr);
			}

			m_exit = true;    // Terminate the sw upgrade thread
			break;
		}
		case WAIT_TIMEOUT:  // Time out
		{
			//std::cout << __FUNCTION__ << "@" << __LINE__ << " SWUpgrade is waiting for events..." << std::endl;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Waiting for events...", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1, tmpStr);
			}
			break;
		}
		case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
				{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a shutdown event", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1, tmpStr);
			}
			m_exit = true;    // Terminate the thread
			break;
				}
		case (WAIT_OBJECT_0 + Command_Event_Index):    // Command ready to be handled
				  {

			printMachineInfo(m_magazine,m_slotUpgOnGoing);
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
			handleTenMinutesTrapTimeout();
			break;
				  }
		case (WAIT_OBJECT_0 + TimeoutTrap_Event_Index_FiveMin):
				  {
			FIXS_CCH_Event::ResetEvent(m_timeOutTrapEventFiveMin);

			if(timerCnt < 40)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "---------------------------------------------------------" << std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ".......................TIME OUT Every Fifteen seconds!!!......................" << std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "---------------------------------"<< timerCnt <<"------------------------" << std::endl;

				stringstream strSlotUpgOnGoing;
				strSlotUpgOnGoing << m_slotUpgOnGoing;
				int env_switch = -1;
				int sw_version = 0;
				CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
				if ((env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX))
					env_switch = 2;
				else
				{
					if((m_slotUpgOnGoing == 0) || (m_slotUpgOnGoing == 25))
						env_switch = 0;
					else if ((m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
					{
						int _result =0;
						_result = CCH_Util::getCmxSoftwareVersion (m_magazine,static_cast<unsigned long>(m_slotUpgOnGoing), sw_version);
						if((_result == 0) && (sw_version == 4))
							env_switch = 1;
						else
							env_switch = 0;
					}
				}

				int status = checkFWStatus(m_IP_EthA, m_IP_EthB, strSlotUpgOnGoing.str(),env_switch);
				if(status == 1)
				{
					cancelTrapEventTimerFiveMin();
					m_currentState->fwUpgradeResultTrap(1);
					break;
				}
				cancelTrapEventTimerFiveMin();
				startTrapEventTimerFiveMin();
				timerCnt++;
			}
			else
			{
				timerCnt = 0;
				m_currentState->fwUpgradeResultTrap(3);
				//Here fill the last upgrade result for FSM storage as NOT OK
				cancelTrapEventTimerFiveMin();
			}
			break;
				  }

		case (WAIT_OBJECT_0 + (Stop_Event_Index)):
				{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " FwUpgrade Stop_Event_Index" << std::endl;
			{//log
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FwUpgrade Stop_Event_Index", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
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
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled ", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1, tmpStr);
			}

			break;
		}
		} // End of switch

	} // End of the while loop


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_FwUpgrade [%s@%d] Exiting from while", __FUNCTION__, __LINE__);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_running = false;

	//clean
	resetEvents();

	return result;
}


void FIXS_CCH_FwUpgrade::stop ()
{
	m_exit = true;
	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"FW UPGRADE Stopping thread on instance [%p] with stop event. \n",this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	}

	if (m_StopEvent)
	{
		int retries=0;
		cout << __FUNCTION__ <<" " << __LINE__ << endl;
		while (FIXS_CCH_Event::SetEvent(m_StopEvent) == 0)
		{
			cout << "Set StopEvent.... retries: " << retries << endl;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FwUpgrade Set StopEvent.... retries",LOG_LEVEL_DEBUG);
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

	if (m_trapTimer)
	{
		if (m_trapTimer->isRunningThread()) m_trapTimer->stopTimer();
	}

	int retry = 0;

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

	if (m_trapTimer_FiveM)
	{
		if (m_trapTimer_FiveM->isRunningThread())	m_trapTimer_FiveM->stopTimer();
	}

	retry = 0;
	if (m_trapTimer_FiveM)
	{
		while (m_trapTimer_FiveM->isRunningThread() && retry < 5){
			cout<< " m_trapTimer_FiveM THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		m_trapTimer_FiveM->closeHandle();
		delete (m_trapTimer_FiveM);
		m_trapTimer_FiveM = NULL;
	}

}

void FIXS_CCH_FwUpgrade::handleCommandEvent ()
{

	if (m_eventQueue.queueEmpty()) return;

	//get event to analyze
	FIXS_CCH_FwUpgrade::FwEventId qEventInfo = DEFAULT_NO_EVENT;
	qEventInfo = m_eventQueue.getFirst();

	if ((this->m_currentState->getStateId() == STATE_IDLE) && (qEventInfo == this->SNMP_SET_OK))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "START TIMER - STATE_IDLE - SNMP_SET_OK " << std::endl;
		startTrapEventTimer(); //start timer waiting for load release completed
	}
	else if((this->m_currentState->getStateId() == STATE_UNDEFINED) && (qEventInfo == this->SNMP_SET_OK))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "START TIMER - STATE_UNDEFINED - SNMP_SET_OK " << std::endl;
		startTrapEventTimer(); //start timer waiting for load release completed
	}
	else if (
			(this->m_currentState->getStateId() == STATE_ONGOING) &&
			((qEventInfo == this->SNMP_TRAP_NOT_OK) || (qEventInfo == this->SNMP_TRAP_OK)
					|| (qEventInfo == this->SNMP_SET_NOT_OK)|| (qEventInfo == this->SNMP_TRAP_PENDING_NOT_OK))
	)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "STOP TIMER - STATE_ONGOING - SNMP_TRAP = " << qEventInfo << std::endl;
		cancelTrapEventTimer();
	}

	//figure out next state
	switchState(qEventInfo);
	m_eventQueue.popData(); //processing ok

}

void FIXS_CCH_FwUpgrade::handleTenMinutesTrapTimeout ()
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " TIMER EXPIRED : STATE == " << this->getStateName().c_str() << std::endl;

	{ // log
		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d][MAG:%s, SLOT:%u] TEN MINUTES TIMEOUT EXPIRED IN STATE %s",__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_currentState->getStateId() == STATE_ONGOING) // timer on loadreleasecomplete
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] TIMER EXPIRED IN STATE == %s ", __FUNCTION__, __LINE__, this->getStateName().c_str());
			if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1, tmpStr);
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " .................... TIMER EXPIRED ......................" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;

		stringstream strSlotUpgOnGoing;
		strSlotUpgOnGoing << m_slotUpgOnGoing;
		CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
		int env_switch = -1;
		int sw_version = 0;
		if ((env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX))
			env_switch = 2;
		else
		{
			if((m_slotUpgOnGoing == 0) || (m_slotUpgOnGoing == 25))
				env_switch = 0;
			else if ((m_slotUpgOnGoing == 26) || (m_slotUpgOnGoing == 28))
			{
				int _result =0;
				_result = CCH_Util::getCmxSoftwareVersion (m_magazine,static_cast<unsigned long>(m_slotUpgOnGoing), sw_version);
				if((_result == 0) && (sw_version == 4))
					env_switch = 1;
				else
					env_switch = 0;
			}
		}
		int status = checkFWStatus(m_IP_EthA, m_IP_EthB, strSlotUpgOnGoing.str(),env_switch);

		if (status == 1)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " .........UPGRADE FINISHED......." << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
			m_currentState->fwUpgradeResultTrap(1);
			cancelTrapEventTimer(); //cancel the timer
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " .................. TRAP NOT ARRIVED ....................." << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------" << std::endl;
			cancelTrapEventTimer();

			startTrapEventTimerFiveMin();
		}

	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] TIMER EXPIRED IN UNEXPECTED STATE  == %s ", __FUNCTION__, __LINE__, this->getStateName().c_str());
			if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1, tmpStr);
		}
		cancelTrapEventTimer();
	}

}

bool FIXS_CCH_FwUpgrade::startTrapEventTimer ()
{
	m_trapTimer->setTrapTime(TEN_MINUTES);

	m_trapTimer->open();

	if(m_timeOutTrapEvent == -1)
	{
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Synchronization ERROR: cannot set WaitableTimer.", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
		return false;
	}
	else
	{
		return true;
	}
}

bool FIXS_CCH_FwUpgrade::cancelTrapEventTimer ()
{
	bool result = false;

	if ((m_timeOutTrapEvent == -1) || (m_timeOutTrapEvent == 0))
	{

		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_timeoutTrapEvent OFF ", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

		result = true;
	}
	else
	{
		m_trapTimer->stopTimer();

		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_timeoutTrapEvent stopped !", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

		result = true;
	}

	return result;
}

void FIXS_CCH_FwUpgrade::setEvent (FIXS_CCH_FwUpgrade::FwEventId eventId)
{
	std::cout<< __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_eventQueue.pushData(eventId);

}

int FIXS_CCH_FwUpgrade::checkFWStatus (std::string ip_EthA, std::string ip_EthB, std::string slot, int env_switch)
{
	UNUSED(slot);
	std::string actState("");
	int value=-1;
	int res = 0;
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(ip_EthA);

	if((env_switch == 2 ) || (env_switch == 1))
	{
		if(m_snmpManager->getFwmActionResult(value))
                {
			std::cout<< __FUNCTION__ << "@" << __LINE__ <<std::endl;
			if(value == 1)
			{
                                res = 2; // upgrade ongoing
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade ongoing" << std::endl;
			}
                        else
                        {
                                res = 1;
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade not ongoing" << std::endl;
                        }
                }
		else
                {
			std::cout<< __FUNCTION__ << "@" << __LINE__ <<std::endl;
                        m_snmpManager->setIpAddress(ip_EthB);
                        if(m_snmpManager->getFwmActionResult(value))
                        {
                                if(value == 1)
				{
                                        res = 2;
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade ongoing" << std::endl;
				}
                                else
                                {
                                        res = 1;
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade not ongoing" << std::endl;
                                }
                        }
			else
				res = -1;
		}

	}
	else
	{
		if(m_snmpManager->getBladeRevisionState(&actState))
		{
			std::cout<< __FUNCTION__ << "@" << __LINE__ <<std::endl;
			size_t found;
			found = actState.find("+");
			if(found != string::npos)
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade ongoing" << std::endl;
				res = 2; //upgrade ongoing
			}
			else
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade finished" << std::endl;
				res = 1; //upgrade finished
			}
		}
		else
		{
			m_snmpManager->setIpAddress(ip_EthB);
			if(m_snmpManager->getBladeRevisionState(&actState))
			{
				std::cout<< __FUNCTION__ << "@" << __LINE__ <<std::endl;
				size_t found;
				found = actState.find("+");
				if(found != string::npos)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade ongoing" << std::endl;
					res = 2; //upgrade ongoing
				}
				else
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Upgrade finished" << std::endl;
					res = 1; //upgrade finished
				}
			}
			else
				res = -1;
		}
	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	return res;
}

void FIXS_CCH_FwUpgrade::restoreStatus (CCH_Util::FwData &fwdata)
{

	std::cout << __FUNCTION__ << "@" << __LINE__ << " In" << std::endl;

	switch ( fwdata.upgradeStatus )
	{
	case CCH_Util::FW_ONGOING:
		m_currentState = &m_ongoing;
		m_slotUpgOnGoing = fwdata.slotUpgOnGoing;
		break;

	case CCH_Util::FW_UNDEFINED:
		m_currentState = &m_undefined;
		break;

	case CCH_Util::FW_IDLE:
		m_currentState = &m_idle;
		break;
	default:
		m_currentState = &m_idle;
		break;
	}
	m_lastUpgradeDate = fwdata.lastUpgradeDate;
	m_lastUpgradeTime = fwdata.lastUpgradeTime;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Out" << std::endl;

}

void FIXS_CCH_FwUpgrade::saveStatus ()
{

	std::cout << __FUNCTION__ << "@" << __LINE__ << " In" << std::endl;

	CCH_Util::FwData mydata;


	mydata.slotUpgOnGoing = m_slotUpgOnGoing;
	if (m_currentState->getStateId() == STATE_ONGOING)
	{
		mydata.upgradeStatus = CCH_Util::FW_ONGOING;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " FW_ONGOING " << std::endl;
	}
	else if(m_currentState->getStateId() == STATE_UNDEFINED)
	{
		mydata.upgradeStatus = CCH_Util::FW_UNDEFINED;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " FW_UNDEFINED " << std::endl;
	}
	else
	{
		mydata.upgradeStatus = CCH_Util::FW_IDLE;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " FW_IDLE " << std::endl;
	}

	mydata.lastUpgradeDate = m_lastUpgradeDate;
	mydata.lastUpgradeTime = m_lastUpgradeTime;
	mydata.result = m_lastFWUpgradeResult;
	mydata.lastUpgradeReason = m_lastFWUpgradeReason;

	FIXS_CCH_FSMDataStorage::getInstance()->saveRemoteFWUpgradeStatus(m_percistance_dn, mydata);

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Out" << std::endl;

}

void FIXS_CCH_FwUpgrade::turnOffStateMachine ()
{
	/*
	this method is used to force the TURN OFF of state machine
    it is used when a mismatch if found and it's needed turn off
	the state machine.
	 */
	switchState(SNMP_TRAP_OK);
	cancelTrapEventTimer();

}

void FIXS_CCH_FwUpgrade::turnOnStateMachine (int iSlot)
{


	/*
	this method is used to force the TURN ON of state machine
    it is used when a mismatch if found and it's needed turn off
	the state machine.
	 */

	m_slotUpgOnGoing = iSlot;
	switchState(SNMP_SET_OK);
	startTrapEventTimer();

}

bool FIXS_CCH_FwUpgrade::startTrapEventTimerFiveMin ()
{

	m_trapTimer_FiveM->setTrapTime(FIFTEEN_SEC);

	int retries = 0;
	while (m_trapTimer_FiveM->isRunningThread())
	{
		cout << "startTrapEventTimerFiveMin: isRunningThread.... retries: " << retries << endl;
		if( ++retries > 10) break;
		sleep(1);
	}

	m_trapTimer_FiveM->open();

	if(m_timeOutTrapEventFiveMin == -1)
	{
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Synchronization ERROR: cannot set WaitableTimer.", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
		return false;
	}
	else
	{
		return true;
	}
}

bool FIXS_CCH_FwUpgrade::cancelTrapEventTimerFiveMin ()
{

	bool result = false;

	if ((m_timeOutTrapEventFiveMin == -1) || (m_timeOutTrapEventFiveMin == 0))
	{

		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_timeOutTrapEventFiveMin OFF ", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

		result = true;
	}
	else {
		m_trapTimer_FiveM->stopTimer();

		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_timeOutTrapEventFiveMin stopped !", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		}

		result = true;
	}
	return result;

}

void FIXS_CCH_FwUpgrade::setDateandTime ()
{

	CCH_Util::getCurrentTime(m_lastUpgradeDate, m_lastUpgradeTime);

}

bool FIXS_CCH_FwUpgrade::initializeParser (std::string &filenameXml, CCH_Util::fwRemoteData &fwData)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Entered initializeParser method" << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " XML : " << filenameXml << std::endl;
	bool errorOccurred = false;

	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& e)
	{
		// trace StrX(e.getMessage())
		if (traceObj->ACS_TRA_ON())
		{ //trace
			std::string xmlException = XMLString::transcode(e.getMessage());
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "XMLException: %s", xmlException.c_str() );
			traceObj->ACS_TRA_event(1, tmpStr);
		}
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] XML Exception. Cannot initialize Xerces parser", __FUNCTION__, __LINE__);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1, traceChar);

		return false;
	}

	SAXParser* parser = new SAXParser;

	parser->setValidationScheme(SAXParser::Val_Auto);
	parser->setDoNamespaces(false);
	parser->setDoSchema(false);
	parser->setValidationSchemaFullChecking(false);

	//  Create our SAX handler object and install it on the parser
	SaxHandler handler(filenameXml, fwData);
	parser->setDocumentHandler(&handler);
	parser->setErrorHandler(&handler);

	char  filenameXmlParse[4096];
	snprintf(filenameXmlParse, sizeof(filenameXmlParse) - 1, "%s", filenameXml.c_str());

	try
	{
		parser->parse(filenameXmlParse);
	}
	catch (const OutOfMemoryException& e)
	{
		//trace "OutOfMemoryException"
		if (traceObj->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			std::string xmlException = XMLString::transcode(e.getMessage());
			snprintf(tmpStr, sizeof(tmpStr) - 1, "OutOfMemoryException: %s", xmlException.c_str() );
			traceObj->ACS_TRA_event(1, tmpStr);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << tmpStr << std::endl;
		}
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] OutOfMemoryException. Cannot parse xml file %s.", __FUNCTION__, __LINE__, filenameXmlParse);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1, traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << traceChar << std::endl;
		errorOccurred = true;
	}
	catch (const XMLException& e)
	{
		//trace "\nError during parsing: '" << xmlFile << "'\n"<< "Exception message is:  \n"  << StrX(e.getMessage()) << "\n" << XERCES_STD_QUALIFIER endl;
		if (traceObj->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			std::string xmlException = XMLString::transcode(e.getMessage());
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error during parsing xml file: %s", xmlException.c_str() );
			traceObj->ACS_TRA_event(1, tmpStr);
		}
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] XMLException. Error parsing xml file %s", __FUNCTION__, __LINE__, filenameXmlParse);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1, traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << traceChar << std::endl;
		errorOccurred = true;
	}
	catch (...)
	{
		//trace XERCES_STD_QUALIFIER cerr << "\nUnexpected exception during parsing: '" << xmlFile << "'\n";
		if (traceObj->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Unexpected exception during parsing");
			traceObj->ACS_TRA_event(1, tmpStr);
		}
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Generic Exception got parsing xml file %s", __FUNCTION__, __LINE__, filenameXmlParse);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1, traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << traceChar << std::endl;
		errorOccurred = true;
	}

	delete parser;

	// And call the termination method
	XMLPlatformUtils::Terminate();

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "returning from initializeParser method" << std::endl;

	return (!errorOccurred);

}


void FIXS_CCH_FwUpgrade::initOnStart ()
{

	unsigned char cSlot = (unsigned char)this->m_slot;
	unsigned long hlmag = ntohl(this->m_magazine);
	unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
	unsigned char mag = u_plug0 & 0x0F;

	m_trapTimer = new FIXS_CCH_TrapTimer();
	m_trapTimer_FiveM = new FIXS_CCH_TrapTimer();

	///////////
	//	JTP  //
	///////////
	if (m_jtpHandlerFw[0]==NULL) m_jtpHandlerFw[0] = new FIXS_CCH_JTPHandler(mag, cSlot,1);
	CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
	if (env == CCH_Util::MULTIPLECP_CBA)
	{
		if (m_jtpHandlerFw[1]==NULL) m_jtpHandlerFw[1] = new FIXS_CCH_JTPHandler(mag, cSlot,2);
	}

	//get command event
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
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
			}
		}
	}


	//initialize timeout event
		m_timeOutTrapEvent = m_trapTimer->getEvent();

	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] get Event TrapTimer[%d]. \n", __FUNCTION__, __LINE__,m_timeOutTrapEvent);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,traceChar);
	}

	//initialize timeout event
		m_timeOutTrapEventFiveMin = m_trapTimer_FiveM->getEvent();

	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] get Event m_trapTimer_FiveM[%d]. \n", __FUNCTION__, __LINE__,m_timeOutTrapEventFiveMin);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,traceChar);
	}

	//initialize JTP notification client
	if (m_jtpHandlerFw[0]!=NULL) m_jtpHandlerFw[0]->open();
	if (m_jtpHandlerFw[1]!=NULL) m_jtpHandlerFw[1]->open();


}

void FIXS_CCH_FwUpgrade::resetEvents ()
{
	FIXS_CCH_Event::ResetEvent(m_StopEvent);

	FIXS_CCH_Event::ResetEvent(m_cmdEvent);

	FIXS_CCH_Event::ResetEvent(m_timeOutTrapEvent);

	FIXS_CCH_Event::ResetEvent(m_timeOutTrapEventFiveMin);

	m_eventQueue.cleanQueue();
}


bool FIXS_CCH_FwUpgrade::getCPAuthorization ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Start CP authorization request for FW Upgarde" << std::endl;
	m_jtpHandlerFw[0]->setStartReqStatus(true);

	m_jtpHandlerFw[0]->maintenanceWindow(FIXS_CCH_JTPHandler::IMB_UPG_START_REQ);

	m_jtpHandlerFw[0]->waitStartReqToFinish();

	if(!m_jtpHandlerFw[0]->checkStartReqStatus()) return false;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "CP accepted the authorization request for FW Upgrade "<<std::endl;

	return true;
}

void  FIXS_CCH_FwUpgrade::stopJtpThread()
{
	int retry =0;
	if (m_jtpHandlerFw[0])
	{
		if (m_jtpHandlerFw[0]->isRunningThread()){
		
			while(m_jtpHandlerFw[0]->isSendingNotification() && retry < 240)
			{
				sleep(1);
				retry++;
			}
			if(!m_jtpHandlerFw[0]->isSendingNotification())
			{

				m_jtpHandlerFw[0]->stop();

				retry = 0;
				while (m_jtpHandlerFw[0]->isRunningThread() && retry < 5){
					cout<< " m_jtpHandler[0] THREAD is still running " << endl;
					sleep(1);
					retry++;
				}
				if(!m_jtpHandlerFw[0]->isRunningThread())
				{
					delete m_jtpHandlerFw[0];
					m_jtpHandlerFw[0] = NULL;
				}
			}
			else
			{
				cout<< " JTP THREAD is still sending notification..... " << endl;
			}
		}
	}

	if (m_jtpHandlerFw[1])
	{
		if (m_jtpHandlerFw[1]->isRunningThread()){
			
			while(m_jtpHandlerFw[1]->isSendingNotification() && retry < 240)
			{
				sleep(1);
				retry++;
			}
			if(!m_jtpHandlerFw[1]->isSendingNotification())
			{
				m_jtpHandlerFw[1]->stop();

				retry = 0;
				while (m_jtpHandlerFw[1]->isRunningThread() && retry < 5){
					cout<< " m_jtpHandler[1] THREAD is still running " << endl;
					sleep(1);
					retry++;
				}
				if(!m_jtpHandlerFw[1]->isRunningThread())
				{
					delete m_jtpHandlerFw[1];
					m_jtpHandlerFw[1] = NULL;
				}
			}
			else
			{
				cout<< " JTP THREAD is still sending notification..... " << endl;
			}

		}
	}
}

int FIXS_CCH_FwUpgrade::getActiveFwArea (std::string IP_EthA_str, std::string IP_EthB_str, bool &isAreaA) //, CCH_Util::fwRemoteData &versionInfo)
		{

	std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;

	FIXS_CCH_SNMPManager *m_snmpManager = new FIXS_CCH_SNMPManager(IP_EthA_str);

	int attributeValue = -1;
	unsigned int currentSW_cntr = 1;
	std::string fwmLoadedType("");
	std::string fwmLoadedVersion("");
	/********************** Fill versionInfo by making SNMP get queries *************/
	bool infoFlag = false;
	//iterating the loop for two times until Current firmware is obtained//
	for(currentSW_cntr = 1; currentSW_cntr <= MAX_ITER_TO_GET_CURRENTSW; currentSW_cntr++)
	{
		attributeValue = -1;
		if (m_snmpManager->getFwmLoadedAttribute(currentSW_cntr,&attributeValue))
		{
			if (attributeValue == 1) // 1 means currentSW
					{
				infoFlag = true;
				break; //to exit when identified the currentSW
			}
			else
				infoFlag = false;
		}
		else
		{
			std::string tmpIp = m_snmpManager->getIpAddress();
			if (tmpIp.compare(IP_EthA_str) == 0)
				m_snmpManager->setIpAddress(IP_EthB_str);
			else
				m_snmpManager->setIpAddress(IP_EthA_str);
			if (m_snmpManager->getFwmLoadedAttribute(currentSW_cntr,&attributeValue))
			{
				if (attributeValue == 1) // 1 means currentSW
						{
					infoFlag = true;
					break;
				}
				else
					infoFlag = false;
			}
			else
			{
				/************SNMP FAILURE***********/
				delete m_snmpManager;
				m_snmpManager = NULL;
				return SNMP_FAILURE;
			}
		}
	}

	if (infoFlag == true) // means currentSW
			{
		/**************** fetching Firmware Load type to get the Active area ********************/
		if (m_snmpManager->getFwmLoadedType(currentSW_cntr, fwmLoadedType))
		{
			ACS_APGCC::trimEnd(fwmLoadedType); //fwmLoadedType variable now has cxp_type_a//
			ACS_APGCC::toUpper(fwmLoadedType);
			cout << "fwmLoadedType: " << fwmLoadedType.c_str() << endl;
			}
			else
			{
				std::string tmpIp = m_snmpManager->getIpAddress();
			if (tmpIp.compare(IP_EthA_str) == 0)
				m_snmpManager->setIpAddress(IP_EthB_str);
			else
				m_snmpManager->setIpAddress(IP_EthA_str);
			if (m_snmpManager->getFwmLoadedType(currentSW_cntr,fwmLoadedType))
			{
				ACS_APGCC::trimEnd(fwmLoadedType); //fwmLoadedType variable now has cxp_type_a//
				ACS_APGCC::toUpper(fwmLoadedType);
				cout << "fwmLoadedType: " << fwmLoadedType.c_str() << endl;
				}
				else
				{
				/************SNMP FAILURE***********/
				delete m_snmpManager;
				m_snmpManager = NULL;
				return SNMP_FAILURE;
			}
		}
	}

	if (m_snmpManager)
		delete m_snmpManager;
	m_snmpManager = NULL;

	//fwmLoadedType variable now has CXP_TYPE_A//
	int iPos = fwmLoadedType.find_last_of("_"); //getting the position of "_" character for last time
	fwmLoadedType = fwmLoadedType.substr(iPos + 1); //incrementing the position, so as to capture "A" of  CXP_TYPE_A
	cout << "Active firmware area is " << fwmLoadedType.c_str() << endl;

	if (fwmLoadedType.compare("A") == 0) // Check for active area as A or B
		isAreaA = true;
	else
		isAreaA = false;
	return EXECUTED;
}

int FIXS_CCH_FwUpgrade::activeAreaSwitch (unsigned int slot,int value) // value= 0 (A),1(B)
{
        std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;

	bool result = true;
	int masterSlot = -1;
        std::string slot0IpA(""), slot25IpA(""), slot0IpB(""), slot25IpB(""),masterIp("");
        FIXS_CCH_UpgradeManager::getInstance()->fetchEgem2L2SwitchIP(m_magazine, 0, slot0IpA, slot0IpB);
        FIXS_CCH_UpgradeManager::getInstance()->fetchEgem2L2SwitchIP(m_magazine, 25, slot25IpA, slot25IpB);
        FIXS_CCH_SNMPManager* snmpReader = new (std::nothrow) FIXS_CCH_SNMPManager(slot0IpA);
        int snmpRes = snmpReader->isMaster(slot0IpA);
        if(snmpRes == 1) //master
        {
		masterSlot = 0;
        }
        else if(snmpRes == 2) //passive
        {
		masterSlot = 25;
        }
        else
        {
                snmpRes = snmpReader->isMaster(slot25IpB);
                if(snmpRes == 1) //master
                {
                        masterSlot = 25;
                }
                else if(snmpRes == 2) //passive
                {
                        masterSlot = 0;
                }
                else
                        result = false;

        }

	bool res=true;
	if(result)
	{
		if(masterSlot == 0)
		{
			snmpReader->setIpAddress(slot0IpA);
			res = snmpReader->setActiveArea(slot,value);
			if(!res)
			{
				snmpReader->setIpAddress(slot0IpB);
				res = snmpReader->setActiveArea(slot,value);
			}
		}
		else if (masterSlot == 25)
		{
			snmpReader->setIpAddress(slot25IpA);
			res = snmpReader->setActiveArea(slot,value);
			if(!res)
			{
				snmpReader->setIpAddress(slot25IpB);
				res = snmpReader->setActiveArea(slot,value);
			}
		}
	}
	else
	{
		
		if(snmpReader != NULL)
		{
			delete(snmpReader);
			snmpReader = NULL;
		}
		return SNMP_FAILURE; 
	}

        if(snmpReader != NULL)
        {
                delete(snmpReader);
                snmpReader = NULL;
        }

	if(res == true)
		return 0;
	else
		return SNMP_FAILURE; 
}

bool FIXS_CCH_FwUpgrade::waitTillFwmActionResultFetched (FIXS_CCH_SNMPManager *handler, bool &stopRetry,bool isCmx4, int &actionStatus)
{
	const int Number_Of_Events = 2;
	FIXS_CCH_EventHandle handlArr[Number_Of_Events] = {m_shutdownEvent, m_StopEvent};
	const int WAIT_TIMEOUT = -2;
	int countRetry = 0;
	bool readStatus = false;

	while ((readStatus == false ) && (countRetry < (isCmx4? 50:25)) && (stopRetry == false))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "wait for 3 second and then retry getFwmActionResult().....\n";
		int eventIndex=FIXS_CCH_Event::WaitForEvents(Number_Of_Events,handlArr,(isCmx4? 4000:3000));
		switch(eventIndex)
		{                                                                               
			case (WAIT_TIMEOUT):  // Time out
			{
				handler->setIpAddress(m_IP_EthA);
				if(handler->getFwmActionResult(actionStatus))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "FwmActionResult value: " << actionStatus << std::endl;
					readStatus = true;
				}
				else
				{
					handler->setIpAddress(m_IP_EthB);
					if(handler->getFwmActionResult(actionStatus))
					{
						readStatus = true;
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "FwmActionResult value: " << actionStatus << std::endl;
					}
				}
				countRetry++;
				break;
			}
                        default:
                        {
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d]default ",__FUNCTION__, __LINE__);
                                stopRetry = true;
                                break;
			}
		}//End of switch
	}
	return readStatus;
}

bool FIXS_CCH_FwUpgrade::readFwFromJson(boost::property_tree::ptree const& pt, CCH_Util::fwRemoteData &info)
{
    bool res = false;
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();
    std::string type; // package for Area A, Area B
    int countp =0, countf = 0;
    bool isAreaA = false;
    for (ptree::const_iterator it = pt.begin(); it != end; ++it)
    {
        //std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
	std::string str(it->first);
	if(str.compare("package") == 0)
	{
		ptree child = it->second;
		for (ptree::const_iterator itc = child.begin(); itc != child.end(); ++itc) {
			std::string str_ch(itc->first);
			if(str_ch.compare("product") == 0)
			{
				info.productno = itc->second.get_value<std::string>();
				countp++;
			}
			else if(str_ch.compare("revision") == 0)
			{
				info.cxp_rState = itc->second.get_value<std::string>();
				countp++;
			}
			else if(str_ch.compare("type") == 0)
			{
				std::string type = itc->second.get_value<std::string>();
				if(type.compare("A") == 0)
                                        isAreaA =true;
				else
					isAreaA =false;
				countp++;
			}
			if(countp >=3)
			{
				break;
			}
		}

	}
	else if(str.compare("firmware") == 0)
	{
		ptree child = it->second;
		std::string temp_cxc;
		std::string temp_file;
		std::string temp_version;
		bool isfile =false;
		for (ptree::const_iterator itc = child.begin(); itc != child.end(); ++itc)
		{


                //      std::cout << itc->first << ": " << itc->second.get_value<std::string>() << std::endl;   
			ptree childb = itc->second;
			countf=0;
			for (ptree::const_iterator itb = childb.begin(); itb != childb.end(); ++itb)
			{

                //              std::cout << itb->first << ": " << itb->second.get_value<std::string>() << std::endl;
				std::string str_ch(itb->first);
				if(str_ch.compare("name") == 0)
				{
					temp_cxc = itb->second.get_value<std::string>();
					countf++;
				}
				else if(str_ch.compare("file") == 0)
				{
					temp_file = itb->second.get_value<std::string>();
					isfile = true;
					countf++;
				}
				else if(str_ch.compare("version") == 0)
				{
					temp_version = itb->second.get_value<std::string>();
					countf++;
				}

				if(isfile && (countf == 3))
				{
					CCH_Util::stringToUpper (temp_cxc);
					CCH_Util::stringToUpper (temp_version);

					if(temp_file.compare(UBOOT_FILE) == 0)
					{
						info.fwType[fwInfoCount]= (isAreaA) ? "UBOOT_A":"UBOOT_B";
						info.fwVersion[fwInfoCount]= temp_cxc + "_" + temp_version;
						fwInfoCount++;
					}
					else if(temp_file.compare(FAILSAFE_FILE) == 0)
					{
						info.fwType[fwInfoCount]= (isAreaA) ? "FAILSAFE_A":"FAILSAFE_B";
						info.fwVersion[fwInfoCount]= temp_cxc + "_" + temp_version;
						fwInfoCount++;
					}
					else if(temp_file.compare(BMC_FILE) == 0)
					{
						info.fwType[fwInfoCount] = (isAreaA) ? "IPMI_A":"IPMI_B";
						info.fwVersion[fwInfoCount] = temp_cxc + "_" + temp_version;
						fwInfoCount++;
					}
					else if(temp_file.compare(SENSOR_FILE) == 0)
					{
						info.fwType[fwInfoCount]= (isAreaA) ? "IPMI_PARAMETERS_SENSOR_A":"IPMI_PARAMETERS_SENSOR_B";
						info.fwVersion[fwInfoCount]= temp_cxc + "_" + temp_version;
						fwInfoCount++;
					}
					else if(temp_file.compare(PMP_FILE) == 0)
					{
						info.fwType[fwInfoCount]= (isAreaA) ? "IPMI_PARAMETERS_PMB_A":"IPMI_PARAMETERS_PMB_B";
						info.fwVersion[fwInfoCount]= temp_cxc + "_" + temp_version;
						fwInfoCount++;
					}
					isfile =false;
					countf = 0;
				}

				if(fwInfoCount >=5)
					break;
			}
			temp_cxc.assign("");
			temp_version.assign("");
			temp_file.assign("");
		}
	}
	if((countp >= 3) && (fwInfoCount >= 5))
	{
		res = true;
	}
    }
    return res;
}

bool FIXS_CCH_FwUpgrade::parseJsonFile(std::string &metadataFilename, CCH_Util::fwRemoteData &info)
{
        bool res = true;
        using boost::property_tree::ptree;
        std::ifstream jsonFile(metadataFilename.c_str());
        ptree pt;
        read_json(jsonFile, pt);
        res = readFwFromJson(pt,info);
        if(res == true)
                sequenceFWType(info); 
        return res;

}

void FIXS_CCH_FwUpgrade::sequenceFWType(CCH_Util::fwRemoteData &info)
{
        std::string tempLoadedType[IPMI_TYPE_COUNT];
        std::string temploadedVersion[IPMI_TYPE_COUNT];
        int count =0;
        for(int i =0; i <= fwInfoCount; i++)
        {
                if(((info.fwType[i]).compare("IPMI_A") == 0 ) || ((info.fwType[i]).compare("IPMI_B") == 0 ))
                {
                        tempLoadedType[0] = info.fwType[i];
                        temploadedVersion[0] = info.fwVersion[i];
                        count++;
                }
                else if(((info.fwType[i]).compare("IPMI_PARAMETERS_SENSOR_A") == 0 ) || ((info.fwType[i]).compare("IPMI_PARAMETERS_SENSOR_B") == 0 ))
                {
                        tempLoadedType[1] = info.fwType[i];
                        temploadedVersion[1] = info.fwVersion[i];
                        count++;
                }
		else if(((info.fwType[i]).compare("IPMI_PARAMETERS_PMB_A") == 0 ) || ((info.fwType[i]).compare("IPMI_PARAMETERS_PMB_B") == 0 ))
                {
                        tempLoadedType[2] = info.fwType[i];
                        temploadedVersion[2] = info.fwVersion[i];
                        count++;
                }

        }

        if(count != IPMI_TYPE_COUNT)
                return;
        count = 0;
        for(int i =0; i <= fwInfoCount; i++)
        {
                if(((info.fwType[i]).compare("IPMI_A") == 0 ) || ((info.fwType[i]).compare("IPMI_B") == 0 )
                        || ((info.fwType[i]).compare("IPMI_PARAMETERS_SENSOR_A") == 0 )
                        || ((info.fwType[i]).compare("IPMI_PARAMETERS_SENSOR_B") == 0 )
                        || ((info.fwType[i]).compare("IPMI_PARAMETERS_PMB_A") == 0 )
                        || ((info.fwType[i]).compare("IPMI_PARAMETERS_PMB_B") == 0 ))
                {
                        info.fwType[i] = tempLoadedType[count];
                        info.fwVersion[i] = temploadedVersion[count];
                        count++;
                }
		if(count == IPMI_TYPE_COUNT)
                        break;
        }
}


