/*
 * FIXS_CCH_PowerFanUpgrade.cpp
 *
 *  Created on: Nov 28, 2011
 *      Author: eanform
 */

#include "FIXS_CCH_PowerFanUpgrade.h"

using namespace std;
const std::string SMX_PFMTYPE_A_CXP = "CXP9024732";
const std::string SMX_PFMTYPE_B_L_CXP = "CXP9024733";
const std::string SMX_PFMTYPE_B_U_CXP = "CXP9024734";
namespace {
	void printMachineInfo(unsigned long magazine, unsigned short slot) {
		std::string value;
		char info[256] = {0};
		CCH_Util::ulongToStringMagazine(magazine, value);
		snprintf(info, sizeof(info) - 1, "magazine: %s, slot: %u", value.c_str(), slot);

		std::cout << "    PFM UPGRADE " << std::endl;
		std::cout << "      ____|~\\_" << std::endl;
		std::cout << "     [4x4_|_|-]" << std::endl;
		std::cout << "      (_)   (_)" << std::endl;
		std::cout << "STATE MACHINE FOR " << info << std::endl;

	};


	// The indexes of events used by PowerFunUpgrade thread
	enum Event_Indexes
	{
		Shutdown_Event_Index = 0,
		Command_Event_Index = 1,   // Command from client
		TimeoutTrap_Event_Index = 2,
		Stop_Event_Index = 3,
		Number_Of_Events = 4
   };

	//DWORD Safe_Timeout = 120000;
	//DWORD TWENTY_MINUTES = 1200000;
	DWORD TWENTYTWO_MINUTES = 1320000;
	//int MAX_RETRY = 3;


#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif

}

namespace {
	ACE_thread_mutex_t s_cs_pfm;
	struct Lock
	{
		Lock()
		{
			//std::cout << "FIXS_CCH_PowerFanUpgrade: Lock region while exception active." << std::endl;
			ACE_OS::thread_mutex_trylock(&s_cs_pfm);
		};

		~Lock()
		{
			//std::cout << "FIXS_CCH_PowerFanUpgrade: Leave Lock region while exception active." << std::endl;
			ACE_OS::thread_mutex_unlock(&s_cs_pfm);
		};
	};
}

// Class FIXS_CCH_PowerFanUpgrade::PFMState
FIXS_CCH_PowerFanUpgrade::PFMState::PFMState()
      : m_context(0)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_id = FIXS_CCH_PowerFanUpgrade::STATE_UNDEFINED;
	m_set_status = false;

}

FIXS_CCH_PowerFanUpgrade::PFMState::PFMState (FIXS_CCH_PowerFanUpgrade::PfmStateId stateId)
      : m_context(0)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_id = stateId;
	m_set_status = false;
}


FIXS_CCH_PowerFanUpgrade::PFMState::~PFMState()
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
}

 void FIXS_CCH_PowerFanUpgrade::PFMState::setContext (FIXS_CCH_PowerFanUpgrade* context)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_context = context;
}

FIXS_CCH_PowerFanUpgrade::PfmStateId FIXS_CCH_PowerFanUpgrade::PFMState::getStateId () const
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	return m_id;
}

 void FIXS_CCH_PowerFanUpgrade::PFMState::snmpSet (bool result)
{
	m_set_status = result ;
}

//int FIXS_CCH_PowerFanUpgrade::PFMState::getUpgradeStatus ()
// {
// 	return m_context->getUpgradeStatus();
// }
static int pfmInfoCount = 0;
// Class FIXS_CCH_PowerFanUpgrade::SaxHandler


FIXS_CCH_PowerFanUpgrade::SaxHandler::SaxHandler (const std::string &xmlFilePath, CCH_Util::pfmData &pfmSwData)
: m_pfmData(pfmSwData)
{
	UNUSED(xmlFilePath);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Entered constructor of  SaxHandler" << std::endl;
	traceSax = new ACS_TRA_trace("CCH PFM SaxHandler");
	m_cxpProdnum = false;
	m_cxpRev = false;
	m_cxcProdnum = false;
	m_cxcRev = false;
}


FIXS_CCH_PowerFanUpgrade::SaxHandler::~SaxHandler()
{
	delete(traceSax);
}

void FIXS_CCH_PowerFanUpgrade::SaxHandler::startElement (const XMLCh* const  name, AttributeList &attributes)
{
	UNUSED(attributes);
	//std::string AttributeName,AttributeType,AttributeValue = "";

	char* elem = XMLString::transcode(name);
	std::string element(elem);
	//std::cout<<"element:"<<element<<std::endl;
	
	
	if (element == "product_number")
	{	
		m_cxpProdnum = true;
	}
	if (element == "cxp_rstate")
	{		
		m_cxcRev = true;
	}
	if (element == "cxc_product_number")
	{
		m_cxcProdnum = true;
	}

	if (element == "cxc_rev")
	{		
		m_cxpRev = true;
	}
}


void FIXS_CCH_PowerFanUpgrade::SaxHandler::warning (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceSax->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "xml error: warning method got called");
		traceSax->ACS_TRA_event(1, tmpStr);
	}

}

void FIXS_CCH_PowerFanUpgrade::SaxHandler::error (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceSax->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "xml error: error method got called");
		traceSax->ACS_TRA_event(1, tmpStr);
	}
}

void FIXS_CCH_PowerFanUpgrade::SaxHandler::fatalError (const SAXParseException &exception)
{
	UNUSED(exception);
	if (traceSax->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "xml error: fatalError method got called");
		traceSax->ACS_TRA_event(1, tmpStr);
	}

}

void FIXS_CCH_PowerFanUpgrade::SaxHandler::characters (const XMLCh* const  chars, const XMLSize_t length)
{
	UNUSED(length);
	if (m_cxcProdnum)
	{
		std::string name = XMLString::transcode(chars);
		m_pfmData.pfmCxcProductNum= name;
		m_cxcProdnum=false;
	}

	if (m_cxcRev)
	{
		std::string cxc = XMLString::transcode(chars);
		//std::cout <<"cxc = " << cxc.c_str() << std::endl;
		m_pfmData.pfmCxcRevision= cxc;
		m_cxcRev = false;
	}
	if (m_cxpProdnum)
	{
		std::string name = XMLString::transcode(chars);
		//		 std::cout <<"namecxp = " << name.c_str() << std::endl;
		m_pfmData.pfmCxpProductNum= name;
		m_cxpProdnum = false;
	}

	if (m_cxpRev)
	{
		std::string cxc = XMLString::transcode(chars);
		//		 std::cout <<"cxprev = " << cxc.c_str() << std::endl;
		m_pfmData.pfmCxpRevision= cxc;
		m_cxpRev = false;
	}
}
// Additional Declarations

// Class FIXS_CCH_PowerFanUpgrade::StateIdle
FIXS_CCH_PowerFanUpgrade::StateIdle::StateIdle()
  : FIXS_CCH_PowerFanUpgrade::PFMState(STATE_IDLE)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
  // m_id = STATE_IDLE;
}


FIXS_CCH_PowerFanUpgrade::StateIdle::~StateIdle()
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
}

 int FIXS_CCH_PowerFanUpgrade::StateIdle::upgrade (std::string completePathPfm, std::string pfmTypeIndex, std::string pfmInstanceIndex, std::string slot)
{

	int result = 0;

	UNUSED(slot);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++++++++THE SET OF PATH IS BEING DONE ++++++++++++++" << std::endl;

	m_context->setType(pfmTypeIndex);
	m_context->setModule(pfmInstanceIndex);
	m_context->setPath(completePathPfm);

	//set pfm
	m_context->setPfmDn();
	int swType=-1;
	
	int res=0;
	CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
	
	if ((env == CCH_Util::SINGLECP_SMX)|| (env == CCH_Util::MULTIPLECP_SMX))
	{		
		swType=m_context->getCurrentRunningPFMSoftwareType();
		if((swType!=CCH_Util::BL)&&(swType!=CCH_Util::UPG))
		{
			return SNMP_FAILURE;
		}
		//
		//Get new fwcontainerfile info from metadata.xml file
		CCH_Util::pfmData pfmSwData;
		std::string filenameXml("");
		std::string curRev="";
		std::string currProdNum="";
		int curSwUpgStatus=-1;
		std::string tftpRoot = FIXS_CCH_DiskHandler::getTftpRootFolderDefault();

		if(CCH_Util::findXmlFile(filenameXml,tftpRoot))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Successful in finding metadata.xml file" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "The metadata file name is:" << filenameXml.c_str() <<std::endl;

			std::string xmlPathFile("");
			xmlPathFile=tftpRoot+"/"+filenameXml;
			std::cout<<"xmlPathFile"<<xmlPathFile<<endl;		

			if(m_context->initializeParser (xmlPathFile,pfmSwData))
			{
			//parsing successful
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Parsing successful" << std::endl;
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_PFMUpgrade - Parsing successful ! ",LOG_LEVEL_DEBUG);			

			}
			else
			{
			//parsing error
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Parsing failed" << std::endl;
			if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_PFMUpgrade - Parsing failed ! ",LOG_LEVEL_ERROR);
			return XMLERROR;
			}
		}
		else
		{
		//didnot find xml file
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << "Failed in finding metadata.xml file" << std::endl;
		if(m_context->FIXS_CCH_logging)  m_context->FIXS_CCH_logging->Write("FIXS_CCH_PFMUpgrade - Failed in finding metadata.xml file ! ",LOG_LEVEL_ERROR);
		return INCORRECT_PACKAGE;
		}

//check of already Package Existing
//When the SwType=0 you can not upgrade the type A SW. Only ugrade of typeB is allowed
//When the SwType=1, you can either upgrade the type A SW or the type B SW
		//BL package
		std::string spc=" ";
		CCH_Util::findAndRemove(pfmSwData.pfmCxpProductNum, spc);
		//std::cout<<"pfmSwData.pfmCxpProductNum.c_str()"<<pfmSwData.pfmCxpProductNum.c_str()<<endl;
		if((pfmSwData.pfmCxpProductNum == SMX_PFMTYPE_A_CXP) && (pfmTypeIndex == "0")){	
			if(swType==CCH_Util::UPG){
				currProdNum=m_context->getCurrentRunningPFMCxcSoftwareProduct(CCH_Util::typeA);//CXC1380050
				if (strcmp(currProdNum.c_str(),pfmSwData.pfmCxcProductNum.c_str())==0)
				{
					curRev=m_context->getCurrentRunningPFMCxcSoftwareRev(CCH_Util::typeA);	
					if(strcmp(curRev.c_str(),pfmSwData.pfmCxcRevision.c_str())==0)
					{
						res=0;
						result=PFM_ALREADY_INSTALLED;
					}	
					else				
					{					
						res=1;
					}
				}
				else
				{
					result= INCORRECT_PACKAGE;
					res=0;
				}
			}
			else
			{
				res=0;
				result = INCORRECT_PACKAGE;
			}
		}
		//Type B Upper
		else if(( pfmSwData.pfmCxpProductNum == SMX_PFMTYPE_B_U_CXP)&& (pfmTypeIndex == "1")){		
			currProdNum=m_context->getCurrentRunningPFMCxcSoftwareProduct(CCH_Util::typeB);//CXC 138 0052 
			if (strcmp(currProdNum.c_str(),pfmSwData.pfmCxcProductNum.c_str())==0)
			{
				curRev=m_context->getCurrentRunningPFMCxcSoftwareRev(CCH_Util::typeB);				
				if(strcmp(curRev.c_str(),pfmSwData.pfmCxcRevision.c_str())==0)
				{
					result= PFM_ALREADY_INSTALLED;
					res=0;
				}	
				else				
				{
					res=1;
				}
			}
			else
			{
				result= INCORRECT_PACKAGE;
				res=0;
			}
		}
		//Type B lower
		else if((pfmSwData.pfmCxpProductNum == SMX_PFMTYPE_B_L_CXP) && (pfmTypeIndex == "1")){
			currProdNum=m_context->getCurrentRunningPFMCxcSoftwareProduct(CCH_Util::typeB);//CXC 138 0051 
			if (strcmp(currProdNum.c_str(),pfmSwData.pfmCxcProductNum.c_str())==0)
			{
				curRev=m_context->getCurrentRunningPFMCxcSoftwareRev(CCH_Util::typeB);				
				if(strcmp(curRev.c_str(),pfmSwData.pfmCxcRevision.c_str())==0)
				{
					result= PFM_ALREADY_INSTALLED;
					res=0;
				}	
				else				
				{
					res=1;
				}
			}
			else
			{
				res = 0;
				result = INCORRECT_PACKAGE;
			}
		}	
		else
		{
			res=0;
			result = INCORRECT_PACKAGE;
			
		}
		if(res)	
			res = m_context->setSnmpSwUpg(completePathPfm, pfmTypeIndex, pfmInstanceIndex);			
	}
	else
	{
		res = m_context->setSnmp(completePathPfm, pfmTypeIndex, pfmInstanceIndex);		
	}
	if (res)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++++++++SET OK+++++++++++++++ "<< std::endl;

		std::cout << "------------------------------------------------------------"<< std::endl;
		std::cout << "    PFM UPGRADE :   TYPE (FAN = 0, POWER = 1) - MODULE (LOWER = 0, UPPER = 1) "<< std::endl;
		std::cout << "    OBJECT :  "<< m_context->m_pfm_dn.c_str() 			<< std::endl;
		std::cout << "    Type = "<< pfmTypeIndex <<" Module = "<< pfmInstanceIndex  << std::endl;
		std::cout << "------------------------------------------------------------"<< std::endl;

		m_context->setEvent(SNMP_SET_OK);
		if (m_context->m_typeId == CCH_Util::FAN) m_context->setUpgradeFanResult(CCH_Util::PFM_RES_OK);
		else if (m_context->m_typeId == CCH_Util::POWER) m_context->setUpgradePowerResult(CCH_Util::PFM_RES_OK);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"set snmp ok"<< std::endl;
	}
	else
	{
		//if error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++++++++SET NOT OK+++++++++++++++ "<< std::endl;

		m_context->setEvent(SNMP_SET_NOT_OK);

		if (m_context->m_typeId == CCH_Util::FAN) m_context->setUpgradeFanResult(CCH_Util::PFM_RES_NOT_OK);
		else if (m_context->m_typeId == CCH_Util::POWER) m_context->setUpgradePowerResult(CCH_Util::PFM_RES_NOT_OK);

		if(result==0)
			result=SNMP_FAILURE;
		
	}

	return result;

}

 int FIXS_CCH_PowerFanUpgrade::StateIdle::pfmFwUpgradeResultTrap (int trapResult)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++++++++TRAP INCOMING ON A WRONG STATE THE RESULT OF THE TRAP IS: "<< trapResult <<"+++++++++++++++"<< std::endl;

    //PFM local state machine status variable "m_upgradeFwStatus" is set to STATUS_NOT_OK = 2
	//if a trap with "trapresult" field has a value not 0 (1 => file error; 2 => other error)
//	if (trapResult > 0)
//	{
//		m_context->setUpgradeStatus(CCH_Util::PFM_FW_STATUS_NOT_OK);
//	}
//	else
//	{
//		m_context->setUpgradeStatus(CCH_Util::PFM_FW_STATUS_OK);
//	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "WRONG EVENT IN THIS STATE, FW STATUS NOT OK:" << m_context->getUpgradeStatus() << std::endl;
	int result = WRONG_OPTION;
	return result;

}
// Additional Declarations

// Class FIXS_CCH_PowerFanUpgrade::StateOngoing
FIXS_CCH_PowerFanUpgrade::StateOngoing::StateOngoing()
  : FIXS_CCH_PowerFanUpgrade::PFMState(STATE_ONGOING)
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
  //m_id = STATE_ONGOING;
}


FIXS_CCH_PowerFanUpgrade::StateOngoing::~StateOngoing()
{
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
}

 int FIXS_CCH_PowerFanUpgrade::StateOngoing::upgrade (std::string completePathPfm, std::string pfmTypeIndex, std::string pfmInstanceIndex, std::string slot)
{

	UNUSED(completePathPfm);
	UNUSED(pfmTypeIndex);
	UNUSED(pfmInstanceIndex);
	UNUSED(slot);

	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "WRONG EVENT IN THIS STATE"<< std::endl;
	result = WRONG_OPTION;
	return result;
}

 int FIXS_CCH_PowerFanUpgrade::StateOngoing::pfmFwUpgradeResultTrap (int trapResult)
{

	std::string t_info; // log

	int result = 0;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	if(trapResult == 0)
	{
		//if ok
		t_info = "OK";
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<": trap ok"<< std::endl;
//		m_context->setUpgradeStatus(CCH_Util::PFM_FW_STATUS_OK);

		if (m_context->m_typeId == CCH_Util::FAN) m_context->setUpgradeFanResult(CCH_Util::PFM_RES_OK);
		else if (m_context->m_typeId == CCH_Util::POWER) m_context->setUpgradePowerResult(CCH_Util::PFM_RES_OK);

		m_context->setEvent(SNMP_TRAP_OK);
	}
	else
	{
		//if error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" trap NOT ok: ";
		switch(trapResult)
		{
			case 1:
				t_info = "file error; error when accessing/reading FW file";
				std::cout << " file error; error when accessing/reading FW file " << std::endl;
				break;
			case 2:
				t_info = "other error";
				std::cout << " other error" << std::endl;
				break;
			default:
				t_info = "unknown error";
				std::cout << " unknown error" << std::endl;
		}

		{ // log
			char traceChar[512] = {0};

			snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] PFM_FW_UpgradeResultTrap == %d, %s",__FUNCTION__, __LINE__, trapResult, t_info.c_str());
//			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar, LOG_LEVEL_ERROR);
			//if (ACS_TRA_ON(traceObj)) ACS_TRA_event(&traceObj, traceChar);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
		}

		m_context->setUpgradeStatus(CCH_Util::PFM_FW_STATUS_NOT_OK);
		if (m_context->m_typeId == CCH_Util::FAN) m_context->setUpgradeFanResult(CCH_Util::PFM_RES_NOT_OK);
		else if (m_context->m_typeId == CCH_Util::POWER) m_context->setUpgradePowerResult(CCH_Util::PFM_RES_NOT_OK);

		m_context->setEvent(SNMP_TRAP_NOT_OK);
		result = UPGRADE_ACTIVATE_FAILED;
	}
	return result;

}

// Additional Declarations



// Class FIXS_CCH_PowerFanUpgrade::EventQueue
FIXS_CCH_PowerFanUpgrade::EventQueue::EventQueue()
 : m_queueHandle(-1)
{
	 Lock lock;
	 if ((m_queueHandle = eventfd(0, 0)) == -1)
	 {
		 // failed to create eventlock_
	 }
}


 FIXS_CCH_PowerFanUpgrade::EventQueue::~EventQueue()
 {

	 Lock lock;
	 while (!m_qEventInfo.empty()) {
		 m_qEventInfo.pop_front();
	 }

	 if (m_queueHandle)
	 		::close(m_queueHandle);     // close event handle

 }

FIXS_CCH_PowerFanUpgrade::PfmEventId FIXS_CCH_PowerFanUpgrade::EventQueue::getFirst ()
{
	//FIXS_CCH_CriticalSectionGuard _(m_queueCriticalSection);
	Lock lock;
	PfmEventId result = DEFAULT_NO_EVENT;
	if (!m_qEventInfo.empty())
	{
		result = m_qEventInfo.front();
	}
	return result;

}

 void FIXS_CCH_PowerFanUpgrade::EventQueue::popData ()
{
	//FIXS_CCH_CriticalSectionGuard _(m_queueCriticalSection);
	 Lock lock;

	if (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
		//if ((m_qEventInfo.size() == 0) && ((m_queueHandle != INVALID_HANDLE_VALUE) && (m_queueHandle != 0))) ResetEvent(m_queueHandle);
	}
}

 void FIXS_CCH_PowerFanUpgrade::EventQueue::pushData (FIXS_CCH_PowerFanUpgrade::PfmEventId info)
{
	 //FIXS_CCH_CriticalSectionGuard _(m_queueCriticalSection);
	 //TR_IA20789: Removed the lock for entire function.
	 //Kept the lock wherever required.

		 bool exist = false;
		 /* Analyze all the event queue to avoid the insertion of
		  * an already present event*/
		 for (unsigned int i=0; i<m_qEventInfo.size();i++)
		 {
			 //check first element
			 FIXS_CCH_PowerFanUpgrade::PfmEventId first;
			 first = getFirst();
			 if (first == info )
				 exist = true;

			 {
				 Lock lock; //TR_IA20789
				 m_qEventInfo.pop_front();
				 m_qEventInfo.push_back(first);
			 }
		 }

		 if (!exist)
		 {
			 Lock lock; //TR_IA20789
			 m_qEventInfo.push_back(info);
			 int retries = 0;
			 while (FIXS_CCH_Event::SetEvent(m_queueHandle) == 0)
			 {
				 cout << "Set m_queueHandle.... retries: " << retries << endl;
				 if( ++retries > 10) break;
				 sleep(1);
			 }//end while
		 }
}

 bool FIXS_CCH_PowerFanUpgrade::EventQueue::queueEmpty ()
{
	//FIXS_CCH_CriticalSectionGuard _(m_queueCriticalSection);
	 Lock lock;
	return m_qEventInfo.empty();
}

 size_t FIXS_CCH_PowerFanUpgrade::EventQueue::queueSize ()
{
	//FIXS_CCH_CriticalSectionGuard _(m_queueCriticalSection);
	 Lock lock;
	return m_qEventInfo.size();
}

FIXS_CCH_PowerFanUpgrade::PfmEventId FIXS_CCH_PowerFanUpgrade::EventQueue::getItem (int index)
{
	//FIXS_CCH_CriticalSectionGuard _(m_queueCriticalSection);
	Lock lock;

	std::list<PfmEventId>::iterator it = m_qEventInfo.begin();
	while (index-- && (it != m_qEventInfo.end())) ++it;

	if (it == m_qEventInfo.end())
		return DEFAULT_NO_EVENT;
	else
		return *it;
}

void FIXS_CCH_PowerFanUpgrade::EventQueue::cleanQueue()
{
	while (!m_qEventInfo.empty()) {
		m_qEventInfo.pop_front();
	}
}

// Additional Declarations



// Class FIXS_CCH_PowerFanUpgrade
FIXS_CCH_PowerFanUpgrade::FIXS_CCH_PowerFanUpgrade()
      : m_trapTimer(0), m_running(false), m_upgradeFwStatus(0),m_cmdEvent(0), m_shutdownEvent(0), m_eventQueue(), m_boardId(0), m_timeoutTrapEvent(0), m_moduleId(CCH_Util::LOWER), m_typeId(CCH_Util::FAN)
{
//	m_jtpHandlerPFM[0] = NULL;
//	m_jtpHandlerPFM[1] = NULL;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	m_idle.setContext(this);
	m_ongoing.setContext(this);
	m_currentState = &m_idle;
}

FIXS_CCH_PowerFanUpgrade::FIXS_CCH_PowerFanUpgrade (unsigned long magazine, unsigned short slot, std::string installedPackage, std::string SCX_IP_EthA, std::string SCX_IP_EthB, std::string dn_blade)
  : m_trapTimer(0), m_running(false), m_upgradeFwStatus(0), m_cmdEvent(0), m_shutdownEvent(0), m_eventQueue(), /*m_boardId(boardId),*/ m_timeoutTrapEvent(0), m_magazine(magazine),
    m_moduleId(CCH_Util::LOWER), m_typeId(CCH_Util::FAN), m_snmpSyncPtr(0), m_SCX_IP_EthA(SCX_IP_EthA), m_SCX_IP_EthB(SCX_IP_EthB), m_slot(slot), m_dn(dn_blade)
{
	UNUSED(installedPackage);
	m_jtpHandlerPFM[0] = NULL;
	m_jtpHandlerPFM[1] = NULL;
	m_idle.setContext(this);
	m_ongoing.setContext(this);
	m_currentState = &m_idle;

	m_fanResult = CCH_Util::PFM_RES_OK;
	m_powerResult = CCH_Util::PFM_RES_OK;

	traceObj = new ACS_TRA_trace("FIXS_CCH_PowerFanUpgrade");

    FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();


	std::string magValue("");
	std::string slotValue("");
	CCH_Util::ulongToStringMagazine(this->m_magazine,magValue);
	CCH_Util::ushortToString(this->m_slot,slotValue);


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, " -  PFM Upgrade: magazine [%s] slot [%s] -  ",magValue.c_str(),slotValue.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
	}

	m_StopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);

}


FIXS_CCH_PowerFanUpgrade::~FIXS_CCH_PowerFanUpgrade()
{
	__
	if (m_cmdEvent)
	{
		//(void) CloseHandle(m_cmdEvent);
		FIXS_CCH_Event::CloseEvent(m_cmdEvent);
	}

	if ( m_timeoutTrapEvent && (m_timeoutTrapEvent != -1) )
	{
		//CancelWaitableTimer(m_timeoutTrapEvent);// do this even if the timer has not started
		//(void) CloseHandle(m_timeoutTrapEvent);
		::close(m_timeoutTrapEvent);
	}

	if (m_StopEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_StopEvent);
	}

	if (m_jtpHandlerPFM[0])
	{
		if (m_jtpHandlerPFM[0]->isRunningThread())	m_jtpHandlerPFM[0]->stop();
	}

	if (m_jtpHandlerPFM[1])
	{
		if (m_jtpHandlerPFM[1]->isRunningThread())	m_jtpHandlerPFM[1]->stop();
	}

	int retry = 0;
	if (m_jtpHandlerPFM[0])
	{
		while (m_jtpHandlerPFM[0]->isRunningThread() && retry < 5){
			cout<< " m_jtpHandlerPFM[0] THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		delete m_jtpHandlerPFM[0];
		m_jtpHandlerPFM[0] = NULL;
	}

	retry = 0;
	if (m_jtpHandlerPFM[1])
	{
		while (m_jtpHandlerPFM[1]->isRunningThread() && retry < 5){
			cout<< " m_jtpHandlerPFM[1] THREAD is still running " << endl;
			sleep(1);
			retry++;
		}
		delete m_jtpHandlerPFM[1];
		m_jtpHandlerPFM[1] = NULL;
	}

	delete (traceObj);

	FIXS_CCH_logging = 0;

}


 int FIXS_CCH_PowerFanUpgrade::upgrade (std::string completePathPfm, std::string pfmTypeIndex, std::string pfmInstanceIndex, std::string slot)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	int result = m_currentState->upgrade(completePathPfm, pfmTypeIndex, pfmInstanceIndex, slot);
	return result;
}

 int FIXS_CCH_PowerFanUpgrade::pfmFwUpgradeResultTrap (int trapResult)
{


	{
		char traceChar[512] = {0};
		std::string magStr, t_currState;
		t_currState = (m_currentState->getStateId() == STATE_ONGOING) ? "ONGOING" : "IDLE";
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d][PFM UPGRADE on MAG:%s, SLOT:%u] CURRENT STATE: %s, FwUpgradeResultTrap : %d",__FUNCTION__, __LINE__, magStr.c_str(), m_slot,	t_currState.c_str(), trapResult);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << traceChar << std::endl;
	}
	int result = m_currentState->pfmFwUpgradeResultTrap(trapResult);
	return result;
}

 void FIXS_CCH_PowerFanUpgrade::switchState (FIXS_CCH_PowerFanUpgrade::PfmEventId eventId)
{
	std::string t_currState, t_nextState, t_event, t_type; //temporay variables used for logging

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

		if (m_typeId == CCH_Util::FAN) t_type = "FAN";
		else t_type = "POWER";
	}

	//this check is basad on currentstate AND eventID
	switch(m_currentState->getStateId())
	{
		//STARTING STATE ONGOING
		case STATE_ONGOING:
			switch(eventId)
			{
				case SNMP_TRAP_OK:
				case SNMP_TRAP_NOT_OK:
					m_currentState = &m_idle;
					m_type = "";

					removeTftpFileName();
					//Stop JTP Maintenance windows on CP
					if (m_typeId == CCH_Util::FAN)
					{
						if (m_jtpHandlerPFM[0])m_jtpHandlerPFM[0]->maintenanceWindow(FIXS_CCH_JTPHandler::FAN_END_UPGRADE);
						if (m_jtpHandlerPFM[1])m_jtpHandlerPFM[1]->maintenanceWindow(FIXS_CCH_JTPHandler::FAN_END_UPGRADE);
					}
					else //POWER
					{
						if (m_jtpHandlerPFM[0])m_jtpHandlerPFM[0]->maintenanceWindow(FIXS_CCH_JTPHandler::POWER_END_UPGRADE);
						if (m_jtpHandlerPFM[1])m_jtpHandlerPFM[1]->maintenanceWindow(FIXS_CCH_JTPHandler::POWER_END_UPGRADE);
					}
					__
					//stop thread
					stop();

					break;

				default:
					break;
			}
			break;

		//STARTING STATE IDLE
		case STATE_IDLE:
		default:
			switch(eventId)
			{
				case SNMP_SET_OK: // IDLE, SNMP_SET_OK == > ONGOING
					m_currentState = &m_ongoing;

					//Start JTP Maintenance windows on CP
					if (m_typeId == CCH_Util::FAN)
					{
						if (m_jtpHandlerPFM[0])m_jtpHandlerPFM[0]->maintenanceWindow(FIXS_CCH_JTPHandler::FAN_START_UPGRADE);
						if (m_jtpHandlerPFM[1])m_jtpHandlerPFM[1]->maintenanceWindow(FIXS_CCH_JTPHandler::FAN_START_UPGRADE);
					}
					else //POWER
					{
						if (m_jtpHandlerPFM[0])m_jtpHandlerPFM[0]->maintenanceWindow(FIXS_CCH_JTPHandler::POWER_START_UPGRADE);
						if (m_jtpHandlerPFM[1])m_jtpHandlerPFM[1]->maintenanceWindow(FIXS_CCH_JTPHandler::POWER_START_UPGRADE);
					}
					break;

				case SNMP_TRAP_NOT_OK:
				case SNMP_SET_NOT_OK:
					m_currentState = &m_idle;
					m_type = "";
					removeTftpFileName();
					__
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

		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1,"[PFM UPGRADE of %s on MAG:%s, SLOT:%u] CURRENT STATE: %s, EVENT RECEIVED: %s, NEXT STATE: %s",
				t_type.c_str(), magStr.c_str(), m_slot,t_currState.c_str(), t_event.c_str(), t_nextState.c_str());

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	cout<< __FUNCTION__ << "@" << __LINE__ << "****************************** Next State = "<< ((m_currentState->getStateId() == STATE_IDLE) ? "IDLE" : "ONGOING") << endl;
	saveStatus();

	// SNMP_SET_OK, SNMP_SET_NOT_OK, SNMP_TRAP_OK, SNMP_TRAP_NOT_OK

}

 std::string FIXS_CCH_PowerFanUpgrade::getStateName ()
{
	std::string name;
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

 bool FIXS_CCH_PowerFanUpgrade::setSnmp (std::string completePathFw, std::string pfmTypeIndex, std::string pfmInstanceIndex)
{

	//FIXS_CCH_CriticalSectionGuard _(*m_snmpSyncPtr);
	bool result = false;

	int countFlag=0;
	string ipadd("");
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;

	std::string url("");
	std::string ip("");
	std::string sub = "169";


	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);

	do  // if the first snmp statement fails, trying second one
	{
		//check IPN
		ip = m_snmpManager->getIpAddress();

		if (ip.length() > 10)
		{
			std::cout<< "ip.length: "<< ip.length()<<std::endl;
			sub = ip.substr(8,3);
		}

		cout << "completePathFw: " << completePathFw.c_str()<< endl;
		cout << "pfmTypeIndex: " << pfmTypeIndex.c_str()<< endl;
		cout << "pfmInstanceIndex: " << pfmInstanceIndex.c_str()<< endl;

		if(m_snmpManager->setPfmFWUpgrade(completePathFw, pfmTypeIndex, pfmInstanceIndex))
		{
			{//log
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] setPfmFWUpgrade(%s) SUCCESS !!! ", __FUNCTION__, __LINE__,completePathFw.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			}

			result = true;
			countFlag=0;

		}
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			{//log
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FAILED setPfmFWUpgrade(%s) on pfmType[%s], pfmModule[%s] - retry switching IP Address ", __FUNCTION__, __LINE__,completePathFw.c_str(), pfmTypeIndex.c_str(), pfmInstanceIndex.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			}

			countFlag++;
			result = false;
		}
		else
		{

			{//log
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FAILED setPfmFWUpgrade(%s) on pfmType[%s], pfmModule[%s] ", __FUNCTION__, __LINE__,completePathFw.c_str(), pfmTypeIndex.c_str(), pfmInstanceIndex.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			}

			countFlag++;
			result = false;
		}

	}
	while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

   return result;
	//m_currentState->snmpSet(result);

}
 bool FIXS_CCH_PowerFanUpgrade::setSnmpSwUpg (std::string completePathFw, std::string pfmTypeIndex, std::string pfmInstanceIndex)
{
	bool result = false;

	int countFlag=0;
	string ipadd("");
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;

	std::string url("");
	std::string ip("");
	std::string sub = "169";


	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);

	do  // if the first snmp statement fails, trying second one
	{
		//check IPN
		ip = m_snmpManager->getIpAddress();

		if (ip.length() > 10)
		{
			std::cout<< "ip.length: "<< ip.length()<<std::endl;
			sub = ip.substr(8,3);
		}

		cout << "completePathFw: " << completePathFw.c_str()<< endl;
		cout << "pfmTypeIndex: " << pfmTypeIndex.c_str()<< endl;
		cout << "pfmInstanceIndex: " << pfmInstanceIndex.c_str()<< endl;

		if(m_snmpManager->setPfmSWUpgrade(completePathFw, pfmTypeIndex, pfmInstanceIndex))
		{
			{//log
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] setPfmSWUpgrade(%s) SUCCESS !!! ", __FUNCTION__, __LINE__,completePathFw.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			}

			result = true;
			countFlag=0;

		}
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			{//log
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FAILED setPfmSWUpgrade(%s) on pfmType[%s], pfmModule[%s] - retry switching IP Address ", __FUNCTION__, __LINE__,completePathFw.c_str(), pfmTypeIndex.c_str(), pfmInstanceIndex.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			}

			countFlag++;
			result = false;
		}
		else
		{

			{//log
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FAILED setPfmSWUpgrade(%s) on pfmType[%s], pfmModule[%s] ", __FUNCTION__, __LINE__,completePathFw.c_str(), pfmTypeIndex.c_str(), pfmInstanceIndex.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			}

			countFlag++;
			result = false;
		}

	}
	while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

   return result;

}

 int FIXS_CCH_PowerFanUpgrade::setEvent (FIXS_CCH_PowerFanUpgrade::PfmEventId event)
{


	std::cout<< __FUNCTION__ << "@" << __LINE__ << std::endl;
	//send event info to the state machine thread
	//EventInfo* qEventInfo = new EventInfo(event, container);
	m_eventQueue.pushData(event);

	return 0;

}

 int FIXS_CCH_PowerFanUpgrade::open (void *args)
 {
	 int result = 0;

	 UNUSED(args);

	 cout << __FUNCTION__ << "@" << __LINE__ << " Pfm upgrade: try to activate..." << endl;

	 {//log
		 char traceChar[512] = {0};
		 snprintf(traceChar, sizeof(traceChar) - 1,"PFM UPGRADE Starting thread on instance [%p]. \n",this);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		 if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	 }

	 activate(THR_DETACHED);

	 m_exit = false;
	 return result;
 }


 int FIXS_CCH_PowerFanUpgrade::svc ()
 {
	 DWORD result = 0;

	 sleep(1);

	 initOnStart();
	 if (m_exit) return result;

	 CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
	if ((env == CCH_Util::SINGLECP_SMX)|| (env == CCH_Util::MULTIPLECP_SMX))
	{
		onRestartInSmx();
	}
	else
	 onRestart();
	 if (m_exit) return result;

	 //main loop
	 EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent, m_cmdEvent, m_timeoutTrapEvent, m_StopEvent};

	 const int WAIT_FAILED=-1;
	 const int WAIT_TIMEOUT= -2;
	 const int WAIT_OBJECT_0=0;
	 const int Shutdown_Event_Index=0;
	 const int Command_Event_Index=1;
	 const int TimeoutTrap_Event_Index=2;
	 const int Stop_Event_Index=3;

	 m_running = true;
	 __

	 while ( ! m_exit ) // Set to true when the thread shall exit
	 {
		 // Wait for shutdown and command events
		 int returnValue = FIXS_CCH_Event::WaitForEvents(Number_Of_Events, handleArr, 100);
		 switch (returnValue)
		 {
		 case WAIT_FAILED:   // Event failed
		 {
			 if (traceObj->ACS_TRA_ON())
			 {
				 char tmpStr[512] = {0};
				 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects", __FUNCTION__, __LINE__);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				 traceObj->ACS_TRA_event(1,tmpStr);
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
//				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				 traceObj->ACS_TRA_event(1,tmpStr);
			 }
			 break;
		 }
		 case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
				{

			 if (traceObj->ACS_TRA_ON())
			 {
				 char tmpStr[512] = {0};
				 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a shutdown event", __FUNCTION__, __LINE__);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				 traceObj->ACS_TRA_event(1,tmpStr);
			 }
			 m_exit = true;    // Terminate the thread
			 //FIXS_CCH_Event::ResetEvent(m_shutdownEvent);
			 break;
				}

		 case (WAIT_OBJECT_0 + Command_Event_Index):    // Command ready to be handled
				{
			 printMachineInfo(m_magazine, m_slot);

			 if (traceObj->ACS_TRA_ON())
			 {
				 char tmpStr[512] = {0};
				 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PfmUpgrade Command_Event_Index", __FUNCTION__, __LINE__);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				 traceObj->ACS_TRA_event(1,tmpStr);
			 }
			 FIXS_CCH_Event::ResetEvent(m_cmdEvent);
			 handleCommandEvent();
			 __
			 break;
				}

		 case (WAIT_OBJECT_0 + TimeoutTrap_Event_Index):
				{
			 __
			 handleTrapTimeout();
			 if (traceObj->ACS_TRA_ON())
			 {
				 char tmpStr[512] = {0};
				 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PfmUpgrade TimeoutTrap_Event_Index", __FUNCTION__, __LINE__);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				 traceObj->ACS_TRA_event(1,tmpStr);
			 }
			 FIXS_CCH_Event::ResetEvent(m_timeoutTrapEvent);
			 break;
				}
		 case (WAIT_OBJECT_0 + (Stop_Event_Index)):
				{
			 std::cout << __FUNCTION__ << "@" << __LINE__ << " PfmUpgrade Stop_Event_Index" << std::endl;

			 {
				 char tmpStr[512] = {0};
				 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PfmUpgrade Stop_Event_Index", __FUNCTION__, __LINE__);
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
				 snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				 traceObj->ACS_TRA_event(1,tmpStr);
			 }

			 break;
		 }
		 } // End of switch

	 } // End of the while loop


	 m_running = false;

	 //clean
	 resetEvents();

	 return result;
}

 void FIXS_CCH_PowerFanUpgrade::stop ()
 {
	 m_exit = true;
	 __

	 {//log
		 char traceChar[512] = {0};
		 snprintf(traceChar, sizeof(traceChar) - 1,"PFM UPGRADE Stopping thread on instance [%p]. \n",this);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_WARN);
		 if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	 }



	 cout << __FUNCTION__ <<" " << __LINE__ << endl;
	 if (m_trapTimer)
	 {
		 if (m_trapTimer->isRunningThread())	m_trapTimer->stopTimer();
	 }

	 int retry = 0;
	 if (m_trapTimer)
	 {
		 __
		 while (m_trapTimer->isRunningThread() && retry < 5){
			 cout<< " trapTimer THREAD is still running " << endl;
			 sleep(1);
			 retry++;
		 }
		 __
		 m_trapTimer->closeHandle();
		 delete (m_trapTimer);
		 m_trapTimer = 0;
	 }
	
	 cout << __FUNCTION__ <<" " << __LINE__ << endl;
	 if (m_StopEvent)
	 {
		 int retries=0;
		 cout << __FUNCTION__ <<" " << __LINE__ << endl;
		 while (FIXS_CCH_Event::SetEvent(m_StopEvent) == 0)
		 {
			 cout << "Set StopEvent.... retries: " << retries << endl;
			 if( ++retries > 10) break;
			 sleep(1);
		 }

		 if (retries>10)
		 {
			 if (traceObj->ACS_TRA_ON())
			 {
				 char tmpStr[512] = {0};
				 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal Stop event.", __FUNCTION__, __LINE__);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				 traceObj->ACS_TRA_event(1,tmpStr);
			 }
		 }
	 }

//	 while (isRunningThread() && retry < 8)
//	 {
//		 cout<< " PFM THREAD is still running " << endl;
//		 sleep(1);
//		 retry++;
//	 }


	 if (m_jtpHandlerPFM[0])
	 {
		 if (m_jtpHandlerPFM[0]->isRunningThread()){

			 if (!m_jtpHandlerPFM[0]->isSendingNotification()){

				 m_jtpHandlerPFM[0]->stop();

				 retry = 0;
				 while (m_jtpHandlerPFM[0]->isRunningThread() && retry < 5){
					 cout<< " m_jtpHandler[0] THREAD is still running " << endl;
					 sleep(1);
					 retry++;
				 }
				 delete m_jtpHandlerPFM[0];
				 m_jtpHandlerPFM[0] = NULL;


			 }
		 }
	 }
	
	 __
	 if (m_jtpHandlerPFM[1])
	 {
		 if (m_jtpHandlerPFM[1]->isRunningThread()){

			 if (!m_jtpHandlerPFM[1]->isSendingNotification()){

				 m_jtpHandlerPFM[1]->stop();

				 retry = 0;
				 while (m_jtpHandlerPFM[1]->isRunningThread() && retry < 5){
					 cout<< " m_jtpHandler[1] THREAD is still running " << endl;
					 sleep(1);
					 retry++;
				 }
				 delete m_jtpHandlerPFM[1];
				 m_jtpHandlerPFM[1] = NULL;


			 }
		 }
	 }

}

 void FIXS_CCH_PowerFanUpgrade::handleCommandEvent ()
{


	if (m_eventQueue.queueEmpty()) return;

	//get event to analyze
	FIXS_CCH_PowerFanUpgrade::PfmEventId qEventInfo = DEFAULT_NO_EVENT;
	qEventInfo = m_eventQueue.getFirst();

	if ( (this->m_currentState->getStateId() == STATE_IDLE) && (qEventInfo == SNMP_SET_OK) )
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "START TIMER - STATE_IDLE - SNMP_SET_OK " << std::endl;
		startTrapTimer(TWENTYTWO_MINUTES); //start timer waiting for load release completed
	}
	else if (
		(this->m_currentState->getStateId() == STATE_ONGOING) &&
		((qEventInfo == this->SNMP_TRAP_NOT_OK) || (qEventInfo == this->SNMP_TRAP_OK))
		)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "STOP TIMER - STATE_ONGOING - SNMP_TRAP = " << qEventInfo << std::endl;
		cancelTrapTimer();
	}

	//figure out next state

	// m_upgradeFwStatus is used for a safe check in case an upgrade just started
	// and and TRAP arrived before the switchState
	if (m_upgradeFwStatus == CCH_Util::PFM_FW_STATUS_OK)
	{
		//this normal case
		__
		switchState(qEventInfo);
	}
	else
	{
		//this is the case when a module is unplugged and you are trying to upgrade this module
		//a trap not ok received in IDLE state before the state switch
		__
		qEventInfo = this->SNMP_TRAP_NOT_OK;
		switchState(qEventInfo);
	}
	m_eventQueue.popData(); //processing ok
	//local upgrade status variable is refreshed to status OK
	m_upgradeFwStatus = CCH_Util::PFM_FW_STATUS_OK;


}

 void FIXS_CCH_PowerFanUpgrade::handleTrapTimeout ()
{


	__

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " TIMER EXPIRED IN STATE == " << this->getStateName().c_str() << std::endl;

	{ // log
		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d][MAG:%s, SLOT:%u] TWENTY MINUTES TIMEOUT EXPIRED IN STATE %s",__FUNCTION__, __LINE__, magStr.c_str(), m_slot, getStateName().c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_WARN);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_currentState->getStateId() == STATE_ONGOING) // timer on loadreleasecomplete
	{
		__
		//set "scanning off" on other
		unsigned short otherSlot = (m_slot == 0 ? 25 : 0);
		FIXS_CCH_UpgradeManager::getInstance()->setScanning(m_magazine, otherSlot, true);

		//switch state
		switchState(SNMP_TRAP_NOT_OK);

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] TIMER EXPIRED IN STATE == %s ", __FUNCTION__, __LINE__, getStateName().c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
	}
	else
	{
		__
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] TIMER EXPIRED IN UNEXPECTED STATE  == %s ", __FUNCTION__, __LINE__, this->getStateName().c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
	}

	cancelTrapTimer();
	__

}

 bool FIXS_CCH_PowerFanUpgrade::cancelTrapTimer ()
{

//	if ((m_timeoutTrapEvent == INVALID_HANDLE_VALUE) || (m_timeoutTrapEvent == 0)) return true;
//	if (CancelWaitableTimer(m_timeoutTrapEvent)) return true;
//	else return false;



	 bool result = false;

	 if ((m_timeoutTrapEvent == -1) || (m_timeoutTrapEvent == 0))
	 {
		 __
		 if (traceObj->ACS_TRA_ON())
		 {
			 char traceChar[512] = {0};
			 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_timeoutTrapEvent OFF ", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			 traceObj->ACS_TRA_event(1,traceChar);
		 }

		 result = true;
	 }
	 else {
		 if (m_trapTimer)
		 {
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
			if (m_trapTimer->isRunningThread())    m_trapTimer->stopTimer();
		 }
		 int retry = 0;
		 if (m_trapTimer)
		 {
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
			while (m_trapTimer->isRunningThread() && retry < 5){
			cout<< " trapTimer THREAD is still running " << endl;
			sleep(1);
			retry++;
		 	}
		 	m_trapTimer->closeHandle();
		 	delete (m_trapTimer);
		 	m_trapTimer = 0;
		 }
		 if (traceObj->ACS_TRA_ON())
		 {
			 char traceChar[512] = {0};
			 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_timeoutTrapEvent stopped !", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			 traceObj->ACS_TRA_event(1,traceChar);
		 }
		 
		 __
		 result = true;
	 }

	 return result;
}

 bool FIXS_CCH_PowerFanUpgrade::startTrapTimer (int seconds)
 {
	 __
	 m_trapTimer->setTrapTime(seconds);

	 m_trapTimer->open();

	 if(m_timeoutTrapEvent == -1)
	 {
		 if (traceObj->ACS_TRA_ON())
		 {
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Synchronization ERROR: cannot set WaitableTimer.", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			 traceObj->ACS_TRA_event(1,tmpStr);
		 }
		 return false;
	 }
	 else
	 {
		 return true;
	 }

}


 void FIXS_CCH_PowerFanUpgrade::setType (std::string pfmTypeIndex)
{
	m_type = pfmTypeIndex;
	
	//typeA=fan,typeB=power
	{
		if (m_type == "0") //"FAN")
			m_typeId = CCH_Util::FAN;
		else
			m_typeId = CCH_Util::POWER;
	}
}

 std::string FIXS_CCH_PowerFanUpgrade::getType ()
{
	 return m_type;
}

 bool FIXS_CCH_PowerFanUpgrade::setScanningOff ()
{

	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	bool result = false;
	int countFlag=0;

	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);

	do
	{
		if(m_snmpManager->setCurrentPfmScanningState(0)) // 0 = OFF
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++++++++SET SCANNING OFF++++++++++++"<< std::endl;
			result = true;
			countFlag = 0;
		}
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error - switch IP Address",__FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in setSWMActionURL(URL) ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	{ // log
		char traceChar[512] = {0};
		std::string t_resultStr = (result ? "SUCCESS" : "FAILED");
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d][MAG:%s, SLOT:%u] %s",	__FUNCTION__, __LINE__, magStr.c_str(), m_slot, t_resultStr.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	return result;
}

 bool FIXS_CCH_PowerFanUpgrade::setScanningOn ()
{

	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	bool result = false;
	int countFlag=0;


	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);

	do
	{
		if(m_snmpManager->setCurrentPfmScanningState(1)) // 1 = ON
		{
			result = true;
			countFlag = 0;
		}
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error - switch IP Address",__FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in setSWMActionURL(URL) ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	{ // log
		char traceChar[512] = {0};
		std::string t_resultStr = (result ? "SUCCESS" : "FAILED");
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d][MAG:%s, SLOT:%u] %s",	__FUNCTION__, __LINE__, magStr.c_str(), m_slot, t_resultStr.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	return result;
}

 void FIXS_CCH_PowerFanUpgrade::restoreStatus (CCH_Util::PFMData &pfmdata)
{
	 cout << __FUNCTION__ << " " << __LINE__<< endl;

	m_powerResult = pfmdata.upgradePowerResult;
	m_fanResult = pfmdata.upgradeFanResult;

	switch ( pfmdata.upgradeStatus )
	{
		case CCH_Util::PFM_ONGOING:
			m_currentState = &m_ongoing;
			m_moduleId = pfmdata.module;
			m_typeId = pfmdata.type;
			m_type = ((m_typeId == CCH_Util::FAN) ? CCH_Util::FAN_ID_STR : CCH_Util::POWER_ID_STR);
			m_pfm_dn = pfmdata.dn_pfm;
			cout << __FUNCTION__ << " " << __LINE__<< endl;
//			m_path = pfmdata.path;
			break;

		case CCH_Util::PFM_IDLE:
		default:
			m_currentState = &m_idle;
			break;
	}

}

 void FIXS_CCH_PowerFanUpgrade::getOverallStatus (CCH_Util::PFMData &pfmdata)
{
	pfmdata.path = m_path;
	pfmdata.module = m_moduleId;
	pfmdata.type = m_typeId;

	pfmdata.upgradeFanResult = static_cast<CCH_Util::PFMResult>(m_fanResult);
	pfmdata.upgradePowerResult = static_cast<CCH_Util::PFMResult>(m_powerResult);

	if (m_currentState->getStateId() == STATE_ONGOING)
	{
		pfmdata.upgradeStatus = CCH_Util::PFM_ONGOING;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " pfmdata.upgradeStatus = CCH_Util::PFM_ONGOING; (" << pfmdata.upgradeStatus<< ")" << std::endl;
	}
	else if (m_currentState->getStateId() == STATE_IDLE)
	{
//		int status = m_currentState->getUpgradeStatus();
//		upgradeResult = status;
		pfmdata.upgradeStatus = CCH_Util::PFM_IDLE;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " pfmdata.upgradeStatus = CCH_Util::PFM_IDLE;(" << pfmdata.upgradeStatus<< ")" << std::endl;
	}

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Out" << std::endl;
}

 void FIXS_CCH_PowerFanUpgrade::saveStatus ()
 {

	 std::cout << __FUNCTION__ << "@" << __LINE__ << " In" << std::endl;
	 int status = -1;
	 int resultToSet = -1;
	 std::string attribute("");
	 std::string attUpgStatus("");


	 CCH_Util::PFMData mydata;

	 getOverallStatus(mydata);

	 cout << "FAN Result  = "<< mydata.upgradeFanResult <<endl;
	 cout << "POWER Result  = "<< mydata.upgradePowerResult <<endl;

	 if (mydata.upgradeStatus == CCH_Util::PFM_IDLE)
	 {
		 m_path = "";
		 //		m_moduleId = CCH_Util::LOWER;
		 //		m_typeId = CCH_Util::FAN;
		 status = IMM_Util::FW_IDLE;
		 //		getOverallStatus(mydata);

	 }

	 else if(mydata.upgradeStatus == CCH_Util::PFM_ONGOING)
	 {
		 status = IMM_Util::FW_ONGOING;
	 }

	 if (m_typeId == CCH_Util::FAN)
	 {
		 if(m_slot == 0)
		 {
			 attribute = IMM_Util::ATT_BLADESWPERSISTANCE_FAN_LOWER_STATE;
			 attUpgStatus = IMM_Util::ATT_BLADESWPERSISTANCE_FAN_LOWER_RESULT;
		 }
		 else
		 {
			 attribute = IMM_Util::ATT_BLADESWPERSISTANCE_FAN_UPPER_STATE;
			 attUpgStatus = IMM_Util::ATT_BLADESWPERSISTANCE_FAN_UPPER_RESULT;
		 }

		 resultToSet = mydata.upgradeFanResult;

		 //IMM_Interface::changeFanModuleState(m_pfm_dn.c_str(),attribute,status);
	 }
	 else if (m_typeId == CCH_Util::POWER)
	 {
		 if(m_slot == 0)
		 {
			 attribute = IMM_Util::ATT_BLADESWPERSISTANCE_POWER_LOWER_STATE;
			 attUpgStatus = IMM_Util::ATT_BLADESWPERSISTANCE_POWER_LOWER_RESULT;
		 }
		 else
		 {
			 attribute = IMM_Util::ATT_BLADESWPERSISTANCE_POWER_UPPER_STATE;
			 attUpgStatus = IMM_Util::ATT_BLADESWPERSISTANCE_POWER_UPPER_RESULT;
		 }

		 resultToSet = mydata.upgradePowerResult;

		 //IMM_Interface::changePowerModuleState(m_pfm_dn.c_str(),attribute,status);
	 }
	 if(!IMM_Interface::changePFMState(m_pfm_dn.c_str(),attribute,status))
	 {
		 std::cout << "**************** ERROR ******************" << std::endl;
		 std::cout << "Could not change PFM state in IMM model" << std::endl;
		 std::cout << "*****************************************\n" << std::endl;
	 }

	 if(!IMM_Interface::changePFMState(m_pfm_dn.c_str(),attUpgStatus,resultToSet))
	 {
		 std::cout << "**************** ERROR ******************" << std::endl;
		 std::cout << "Could not change PFM result in IMM model" << std::endl;
		 std::cout << "*****************************************\n" << std::endl;
	 }

	 std::cout << "------------------------------------------------------------"<< std::endl;
	 std::cout << "    PFM UPGRADE :   TYPE (FAN = 0, POWER = 1) - MODULE (LOWER = 0, UPPER = 1) "<< std::endl;
	 std::cout << "    OBJECT :  "<< m_pfm_dn.c_str() 			<< std::endl;
	 std::cout << "    Type = "<< m_typeId <<" Module = "<< m_moduleId  << std::endl;
	 std::cout << "    Result = "<< resultToSet << std::endl;
	 std::cout << "------------------------------------------------------------"<< std::endl;

	 //FIXS_CCH_FSMDataStorage::getInstance()->saveUpgradeStatus(m_magazine, m_slot, mydata);

}

 void FIXS_CCH_PowerFanUpgrade::setModule (std::string pfmInstanceIndex)
{

	if (pfmInstanceIndex == CCH_Util::UPPER_ID_STR)
		m_moduleId = CCH_Util::UPPER;
	else
		m_moduleId = CCH_Util::LOWER;
}

 void FIXS_CCH_PowerFanUpgrade::setPath (std::string completePathPfm)
{
	m_path = completePathPfm;
}

 std::string FIXS_CCH_PowerFanUpgrade::getCurrentRunningFirmware (CCH_Util::PFMType index)
{

	//FIXS_CCH_CriticalSectionGuard _(*m_snmpSyncPtr);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	std::string runningFwRevision;
	std::string currentFwProductNumber ;
	std::string currentFwRevision;
	int typeIndex = static_cast<int>(index);
	int typeInstance = static_cast<int>(m_slot == 0 ? CCH_Util::LOWER : CCH_Util::UPPER);//lower:0 / upper:1
	bool result = false;
	int countFlag = 0;
	
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);


	do
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"typeIndex: "<<typeIndex<<" typeinstance:"<<typeInstance<<std::endl;
		if (
			m_snmpManager->getPfmFWProductNumber(&currentFwProductNumber, typeIndex, typeInstance) && (currentFwProductNumber.empty() == false)
			&&
			m_snmpManager->getPfmFWProductRevisionState(&currentFwRevision, typeIndex, typeInstance) && (currentFwRevision.empty() == false)
			)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"PfmFWProductNumber: "<<currentFwProductNumber<<"currentFwRevision: "<<currentFwRevision<<std::endl;
			result = true;
			countFlag = 0;
		}
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningSoftware(&currLoadVer)-switching IP Address ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}

			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningSoftware(&currLoadVer) ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}while(countFlag == 1);

	if (!currentFwProductNumber.empty() && !currentFwRevision.empty())
	{
		runningFwRevision = currentFwProductNumber + " " + currentFwRevision;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" runningFwRevision:"<< runningFwRevision.c_str() << std::endl;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" currentFwProductNumber:\""<< currentFwProductNumber.c_str() << "\"" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" currentFwRevision:\""<< currentFwRevision.c_str() << "\"" << std::endl;
	}

	{ // log
		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d][MAG:%s, SLOT:%u] runningFwRevision == %s",	__FUNCTION__, __LINE__, magStr.c_str(), m_slot, runningFwRevision.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

    return runningFwRevision;
}
  int FIXS_CCH_PowerFanUpgrade::getCurrentRunningPFMSoftwareType (int typeInstance)
{

	//FIXS_CCH_CriticalSectionGuard _(*m_snmpSyncPtr);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;	
	int currentSwType=-1;
	if(typeInstance==-1)
	typeInstance = static_cast<int>(m_slot == 0 ? CCH_Util::LOWER : CCH_Util::UPPER);//lower:0 / upper:1
	bool result = false;
	int countFlag = 0;
	
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);
	
	do
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" typeinstance:"<<typeInstance<<std::endl;
		currentSwType = m_snmpManager->getPfmCurSwType(typeInstance);
		if (currentSwType != -1)			
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"currentSwType: "<<currentSwType<<std::endl;
			result = true;
			countFlag = 0;
		}
		
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningPFMSoftwareType()-switching IP Address ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}

			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningPFMSoftwareType() ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}while(countFlag == 1);


	{ // log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] currentSwType == %d",	__FUNCTION__, __LINE__,  currentSwType);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

    return currentSwType;
}

 std::string FIXS_CCH_PowerFanUpgrade::getCurrentRunningPFMSoftware (CCH_Util::PFMUpgType index)
{

	//FIXS_CCH_CriticalSectionGuard _(*m_snmpSyncPtr);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	std::string runningSwRevision;
	std::string currentSwProductNumber ;
	std::string currentSwRevision;
	int typeIndex = static_cast<int>(index);
	int typeInstance = static_cast<int>(m_slot == 0 ? CCH_Util::LOWER : CCH_Util::UPPER);//lower:0 / upper:1
	bool result = false;
	int countFlag = 0;
	
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);


	do
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" typeinstance:"<<typeInstance<<std::endl;
		if (
			m_snmpManager->getPfmSwProductNumber(&currentSwProductNumber, typeIndex, typeInstance) && (currentSwProductNumber.empty() == false)
			&&
			m_snmpManager->getPfmSwProductRevisionState(&currentSwRevision, typeIndex, typeInstance) && (currentSwRevision.empty() == false)
			)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"currentSwProductNumber: "<<currentSwProductNumber<<"currentSwRevision: "<<currentSwRevision<<std::endl;
			result = true;
			countFlag = 0;
		}
		
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningPFMSoftware -switching IP Address ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}

			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningSoftware(&currLoadVer) ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}while(countFlag == 1);

	if (!currentSwProductNumber.empty() && !currentSwRevision.empty())
	{
		runningSwRevision = currentSwProductNumber + " " + currentSwRevision;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" runningSwRevision:"<< runningSwRevision.c_str() << std::endl;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" currentSwProductNumber:\""<< currentSwProductNumber.c_str() << "\"" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" currentSwRevision:\""<< currentSwRevision.c_str() << "\"" << std::endl;
	}

	{ // log
		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d][MAG:%s, SLOT:%u] runningSwRevision == %s",	__FUNCTION__, __LINE__, magStr.c_str(), m_slot, runningSwRevision.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

    return runningSwRevision;
}

 std::string FIXS_CCH_PowerFanUpgrade::getCurrentRunningPFMCxcSoftwareRev (CCH_Util::PFMUpgType index)
{

	//FIXS_CCH_CriticalSectionGuard _(*m_snmpSyncPtr);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	std::string runningSwRevision;
	int typeIndex = static_cast<int>(index);
	int typeInstance = static_cast<int>(m_slot == 0 ? CCH_Util::LOWER : CCH_Util::UPPER);//lower:0 / upper:1
	bool result = false;
	int countFlag = 0;
	std::string spc=" ";
	
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);


	do
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" typeinstance:"<<typeInstance<<std::endl;
		if (			
			m_snmpManager->getPfmSwProductRevisionState(&runningSwRevision, typeIndex, typeInstance) && (runningSwRevision.empty() == false)
			)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"runningSwRevision: "<<runningSwRevision<<std::endl;
			result = true;
			countFlag = 0;
		}
		
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningPFMSoftwareRev -switching IP Address ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}

			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningSoftwareRev ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}while(countFlag == 1);


	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	CCH_Util::findAndRemove(runningSwRevision, spc);
    return runningSwRevision;
}

 std::string FIXS_CCH_PowerFanUpgrade::getCurrentRunningPFMCxcSoftwareProduct (CCH_Util::PFMUpgType index)
{

	//FIXS_CCH_CriticalSectionGuard _(*m_snmpSyncPtr);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	std::string currentSwProductNumber ;
	int typeIndex = static_cast<int>(index);
	int typeInstance = static_cast<int>(m_slot == 0 ? CCH_Util::LOWER : CCH_Util::UPPER);//lower:0 / upper:1
	bool result = false;
	int countFlag = 0;
	std::string spc=" ";
	
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);


	do
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" typeinstance:"<<typeInstance<<std::endl;
		if (
			m_snmpManager->getPfmSwProductNumber(&currentSwProductNumber, typeIndex, typeInstance) && (currentSwProductNumber.empty() == false)
			)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"getCurrentRunningPFMCxcSoftwareProduct: "<<"currentSwProductNumber: "<<currentSwProductNumber<<std::endl;
			result = true;
			countFlag = 0;
		}
		
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningPFMCxcSoftwareProduct -switching IP Address ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}

			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentRunningPFMCxcSoftwareProduct(&currLoadVer) ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}
	
	CCH_Util::findAndRemove(currentSwProductNumber, spc);
	std::cout<<"currentSwProductNumber"<<currentSwProductNumber<<endl;
    return currentSwProductNumber;
}
  int FIXS_CCH_PowerFanUpgrade::getCurrentSwUpgradeStatus (int swType, int typeInstance)
{
	//FIXS_CCH_CriticalSectionGuard _(*m_snmpSyncPtr);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	bool result = false;
	int countFlag = 0;
	
	FIXS_CCH_SNMPManager *m_snmpManager = NULL;
	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);
	int actSwStatus = -1;
	do
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" typeinstance:"<<typeInstance<<std::endl;
		if((m_snmpManager->getPfmSWUpgradeStatus(&actSwStatus, swType, typeInstance)) && (actSwStatus!=-1))					
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"actSwStatus: "<<actSwStatus<<std::endl;
			result = true;
			countFlag = 0;
		}		
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentSwUpgradeStatus()-switching IP Address ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}

			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getCurrentSwUpgradeStatus() ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}while(countFlag == 1);


	{ // log
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] actSwStatus == %d",	__FUNCTION__, __LINE__,  actSwStatus);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_INFO);
		if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

    return actSwStatus;
}

 std::string FIXS_CCH_PowerFanUpgrade::getCurrentModuleDeviceType (bool isSmx)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;

	FIXS_CCH_SNMPManager *m_snmpManager = NULL;

	int moduleType = -1;
	std::string moduleTypeString;
	int typeInstance = static_cast<int>(m_slot == 0 ? CCH_Util::LOWER : CCH_Util::UPPER);//lower:0 / upper:1
	bool result = false;
	int countFlag = 0;

	m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);

	do
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" typeinstance: "<<typeInstance<<std::endl;
		if (m_snmpManager->getPfmModuleDeviceType(typeInstance, &moduleType))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"PfmFWModuleDeviceType: "<< moduleType <<std::endl;
			result = true;
			countFlag = 0;
		}
		else if (countFlag == 0)
		{
			m_snmpManager->setIpAddress(m_SCX_IP_EthB);

			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getModuleType(&moduleType)-switching IP Address ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}

			countFlag++;
			result = false;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char traceChar[512] = {0};
				snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error in getModuleType(&moduleType) ", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,traceChar);
			}
			countFlag++;
			result = false;
		}
	}while(countFlag == 1);


		if (moduleType == 1)
		{
			moduleTypeString = "HOD";
		}
		else if (moduleType == 0)
		{
			moduleTypeString = "LOD";
		}
		else 
		{
			if(isSmx)
			{
				if (moduleType == 2)
				{
					moduleTypeString = "LOD";
				}
				else if (moduleType == 3)
				{
					moduleTypeString = "HOD";
				}
			}
			else{
				moduleTypeString = "";
			}
		}

	{ // log
		char traceChar[512] = {0};
		std::string magStr;
		CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d][MAG:%s, SLOT:%u] moduleTypeString == %s",	__FUNCTION__, __LINE__, magStr.c_str(), m_slot, moduleTypeString.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
	}

	if (m_snmpManager != NULL)
	{
		delete m_snmpManager;
		m_snmpManager = NULL;
	}

	return moduleTypeString;	
}
bool FIXS_CCH_PowerFanUpgrade::initializeParser (std::string &filenameXml, CCH_Util::pfmData &pfmSwData)
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
	SaxHandler handler(filenameXml, pfmSwData);
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

 void FIXS_CCH_PowerFanUpgrade::initOnStart ()
 {
//	 if (m_trapTimer == NULL)
		 m_trapTimer = new FIXS_CCH_TrapTimer();


	 if ((m_slot == 0) ||(m_slot == 25))
	 {
		 unsigned char cSlot = (unsigned char)this->m_slot;
		 unsigned long hlmag = ntohl(this->m_magazine);
		 unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
		 unsigned char mag = u_plug0 & 0x0F;

		 if (m_jtpHandlerPFM[0]==NULL) m_jtpHandlerPFM[0] = new FIXS_CCH_JTPHandler(mag, cSlot,1);

		 CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
		 if ((env == CCH_Util::MULTIPLECP_CBA)|| (env == CCH_Util::MULTIPLECP_SMX))
		 {
			 if (m_jtpHandlerPFM[1]==NULL) m_jtpHandlerPFM[1] = new FIXS_CCH_JTPHandler(mag, cSlot,2);
		 }
	 }

//	 if (m_cmdEvent == 0)
		 m_cmdEvent = m_eventQueue.getHandle();

	 //shutdown event
	 if (m_shutdownEvent == 0)
	 {
		 m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
		 if(m_shutdownEvent == FIXS_CCH_Event_RC_ERROR)
		 {
			 if (traceObj->ACS_TRA_ON())
			 {
				 char traceChar[512] = {0};
				 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] error creating shutdown event. \n", __FUNCTION__, __LINE__);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				 traceObj->ACS_TRA_event(1,traceChar);
			 }
		 }
	 }

	 /*if ( (!m_timeoutTrapEvent) || (m_timeoutTrapEvent == -1) )*/	m_timeoutTrapEvent = m_trapTimer->getEvent();

	 {
		 char traceChar[512] = {0};
		 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] get Event TrapTimer[%d]. \n", __FUNCTION__, __LINE__,m_timeoutTrapEvent);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		 if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,traceChar);
	 }


	 if (m_jtpHandlerPFM[0]!=NULL) m_jtpHandlerPFM[0]->open();
	 if (m_jtpHandlerPFM[1]!=NULL) m_jtpHandlerPFM[1]->open();
}

 void FIXS_CCH_PowerFanUpgrade::onRestart ()
{

	if (m_currentState->getStateId() == STATE_ONGOING)
	{
		cout << __FUNCTION__ << " " << __LINE__<< endl;
		// check PFM upgrade status on SCX in case CCH restarted during an upgrade
		int typeIndex = 0;
		if (m_typeId == CCH_Util::FAN) typeIndex = 0;
		else typeIndex = 1;
		int typeInstance = 0;
		if (m_moduleId == CCH_Util::LOWER) typeInstance = 0;
		else typeInstance = 1;

		//set dn pfm
		setPfmDn ();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "DN PFM:" << m_pfm_dn.c_str() << std::endl;

		int actFwStatus = 0;
		bool resultGet = false;

		FIXS_CCH_SNMPManager *m_snmpManager = NULL;
		m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);

		for (int countFlag = 0; (countFlag < 3) && (!resultGet); countFlag++)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " [JUST STARTED] getPfmFWUpgradeStatus...";
			if(m_snmpManager->getPfmFWUpgradeStatus(&actFwStatus, typeIndex, typeInstance))
			{
				std::cout << " actFwStatus : " << actFwStatus << std::endl;

				m_snmpManager->getPfmFWUpgrade(&m_path,m_typeId,m_moduleId);

				std::cout << " m_path : " << m_path.c_str() << std::endl;

				resultGet = true;
				break;
			}
			else
			{
				std::cout << " ... FAILED" << std::endl;
				m_snmpManager->setIpAddress(m_SCX_IP_EthB);

				if (traceObj->ACS_TRA_ON())
				{
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error - switch IP Address ", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
					traceObj->ACS_TRA_event(1,traceChar);
				}
				resultGet = false;
				sleep(1);
			}
		}

		if (m_snmpManager != NULL)
		{
			delete m_snmpManager;
			m_snmpManager = NULL;
		}

		if (resultGet && (actFwStatus == 0))
		{
			//reset
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PFM upgrade status value is " << actFwStatus << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " RESET STATE TO *********** IDLE ***********" << std::endl;
			m_currentState = &m_idle;
			m_path = "";
			saveStatus();

			{ //log
				char traceChar[512] = {0};
				std::string magStr, t_currState;
				CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
				snprintf(traceChar, sizeof(traceChar) - 1,"[SLOT:%u] RESET STATE TO IDLE. SNMP PfmFWUpgradeStatus == 0",m_slot);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
			}

			//stop thread
			stop();
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PFM upgrade status value is " << actFwStatus << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " RESTART TIMERT" << std::endl;
			startTrapTimer(TWENTYTWO_MINUTES); //start timer waiting for load release completed
			{ //log
				char traceChar[512] = {0};
				std::string magStr, t_currState;
				CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
				snprintf(traceChar, sizeof(traceChar) - 1,"[SLOT:%u] RESTARTED ON STATE ONGOING. STARTED 22 MINUTES TIMER",m_slot);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
			}
		}
	}

}

 void FIXS_CCH_PowerFanUpgrade::onRestartInSmx ()
{

	if (m_currentState->getStateId() == STATE_ONGOING)
	{
		cout << __FUNCTION__ << " " << __LINE__<< endl;
		// check PFM upgrade status on SMX in case CCH restarted during an upgrade
		int typeIndex = 0;
		if (m_typeId == CCH_Util::FAN) typeIndex = 0;
		else typeIndex = 1;
		int typeInstance = 0;
		if (m_moduleId == CCH_Util::LOWER) typeInstance = 0;
		else typeInstance = 1;

		int swType=-1;
	
		swType=getCurrentRunningPFMSoftwareType(typeInstance);
		//set dn pfm
		setPfmDn ();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "DN PFM:" << m_pfm_dn.c_str() << std::endl;

		int actSwStatus = 0;
		bool resultGet = false;

		FIXS_CCH_SNMPManager *m_snmpManager = NULL;
		m_snmpManager = new FIXS_CCH_SNMPManager(m_SCX_IP_EthA);

		for (int countFlag = 0; (countFlag < 3) && (!resultGet); countFlag++)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " [JUST STARTED] getPfmFWUpgradeStatus...";
			if(m_snmpManager->getPfmSWUpgradeStatus(&actSwStatus, typeIndex, typeInstance))
			{
				std::cout << " actSwStatus : " << actSwStatus << std::endl;

				m_snmpManager->getPfmSWUpgrade(&m_path,typeIndex,typeInstance);

				std::cout << " m_path : " << m_path.c_str() << std::endl;

				resultGet = true;
				break;
			}
			else
			{
				std::cout << " ... FAILED" << std::endl;
				m_snmpManager->setIpAddress(m_SCX_IP_EthB);

				if (traceObj->ACS_TRA_ON())
				{
					char traceChar[512] = {0};
					snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Error - switch IP Address ", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
					traceObj->ACS_TRA_event(1,traceChar);
				}
				resultGet = false;
				sleep(1);
			}
		}

		if (m_snmpManager != NULL)
		{
			delete m_snmpManager;
			m_snmpManager = NULL;
		}

		if (resultGet && (actSwStatus == 0)|| (actSwStatus == 2))
		{
			//reset
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PFM upgrade status value is " << actSwStatus << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " RESET STATE TO *********** IDLE ***********" << std::endl;
			m_currentState = &m_idle;
			m_path = "";
			saveStatus();

			{ //log
				char traceChar[512] = {0};
				std::string magStr, t_currState;
				CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
				snprintf(traceChar, sizeof(traceChar) - 1,"[SLOT:%u] RESET STATE TO IDLE. SNMP PfmFWUpgradeStatus == 0",m_slot);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
			}

			//stop thread
			stop();
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " PFM upgrade status value is " << actSwStatus << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " RESTART TIMERT" << std::endl;
			startTrapTimer(TWENTYTWO_MINUTES); //start timer waiting for load release completed
			{ //log
				char traceChar[512] = {0};
				std::string magStr, t_currState;
				CCH_Util::ulongToStringMagazine(this->m_magazine, magStr);
				snprintf(traceChar, sizeof(traceChar) - 1,"[SLOT:%u] RESTARTED ON STATE ONGOING. STARTED 22 MINUTES TIMER",m_slot);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())	traceObj->ACS_TRA_event(1,traceChar);
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " " << traceChar << std::endl;
			}
		}
	}

}

 void FIXS_CCH_PowerFanUpgrade::setPfmDn ()
 {
	 m_pfm_dn = IMM_Interface::getDnBladePersistance(this->m_magazine,this->m_slot);
 }


 bool FIXS_CCH_PowerFanUpgrade::removeTftpFileName()
 {
 	bool res = false;

 	std::string path("");
 	std::string temp_path("");
	__
	cout<< "m_path = " << m_path.c_str() << endl;

 	int iPos = m_path.find_last_of(CHR_IPMI_DIR_SEPARATOR);
 	if(iPos >= 0){
 		// Filename is part of input path. Extract it.
 		temp_path = m_path.substr(iPos +1);
 	}

 	std::string tftpDir;
	CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
	if ((env == CCH_Util::SINGLECP_SMX)|| (env == CCH_Util::MULTIPLECP_SMX))
		tftpDir = FIXS_CCH_DiskHandler::getTftpRootFolderDefault();
	else
		tftpDir	= FIXS_CCH_DiskHandler::getTftpRootFolder();
 	path = tftpDir + "/" + temp_path;
	cout<< "path = " << path.c_str() << endl;

 	char traceChar[512] = {0};
 	snprintf(traceChar, sizeof(traceChar) - 1,"PFM UPGRADE - Tftp Path to remove : %s ",path.c_str());
 	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);

 	FIXS_CCH_DiskHandler::deleteFile(path.c_str());

 	return res;
 }

 void FIXS_CCH_PowerFanUpgrade::resetEvents ()
 {
	 FIXS_CCH_Event::ResetEvent(m_StopEvent);

	 FIXS_CCH_Event::ResetEvent(m_cmdEvent);

	 FIXS_CCH_Event::ResetEvent(m_timeoutTrapEvent);

	 m_eventQueue.cleanQueue();
 }

 std::string FIXS_CCH_PowerFanUpgrade::getPowerResultState ()
 {
 	std::string result("");
 	switch(m_powerResult)
 	{
 		case CCH_Util::PFM_RES_OK:
 			result = "OK";
 			break;

 		case CCH_Util::PFM_RES_NOT_OK:
 		default:
 			result = "NOT OK";
 			break;
 	}
 	return result;
 }

 std::string FIXS_CCH_PowerFanUpgrade::getFanResultState ()
  {
  	std::string result("");
  	switch(m_fanResult)
  	{
  		case CCH_Util::PFM_RES_OK:
  			result = "OK";
  			break;

  		case CCH_Util::PFM_RES_NOT_OK:
  		default:
  			result = "NOT OK";
  			break;
  	}
  	return result;
  }
