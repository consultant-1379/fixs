#include "FIXS_CMXH_Manager.h"
#include "FIXS_CMXH_Util.h"
//#include <iostream>

// FIXS_CMXH_SNMPHandler
#include "FIXS_CMXH_SNMPHandler.h"
//#include "ACS_TRA_trace.h"
#include "FIXS_CMXH_Logger.h"


using namespace std;

//add other variable to handle the event
const int WAIT_FAILED = -1;
const int WAIT_TIMEOUT = -2;
const int WAIT_OBJECT_0 = 0;

FIXS_CMXH_SNMPHandler::FIXS_CMXH_SNMPHandler()
: m_shutdownEvent(0),
  m_stopEvent(0),
  m_trapEvent(0),
  m_exit(false),
  m_running(false),
  m_IP_EthA(""),
  m_IP_EthB(""),
  m_SW_Version(0)

{
	m_shutdownEvent =  FIXS_CMXH_Event::OpenNamedEvent(CMXH_Util::EVENT_NAME_SHUTDOWN);
	m_stopEvent = FIXS_CMXH_Event::CreateEvent(false,false,0);

	m_trapEvent = FIXS_CMXH_Event::CreateEvent(false,false,0);
	_trace = new ACS_TRA_trace("SNMP Handler");
	FIXS_CMXH_logging = FIXS_CMXH_Logger::getLogInstance();

}

FIXS_CMXH_SNMPHandler::~FIXS_CMXH_SNMPHandler()
{
	if(m_stopEvent)
		FIXS_CMXH_Event::CloseEvent(m_stopEvent);
	if (m_trapEvent) FIXS_CMXH_Event::CloseEvent(m_trapEvent);
	FIXS_CMXH_logging = 0;
}

/*============================================================================
ROUTINE: open
============================================================================ */
int FIXS_CMXH_SNMPHandler::open(void *args)
{
	int result;

	UNUSED(args);

	result = activate(THR_DETACHED);

	return result;
}

/*============================================================================
ROUTINE: svc
============================================================================ */
int FIXS_CMXH_SNMPHandler::svc()
{
	int result = 0;
	m_running = true;
	sleep(1);

	FIXS_CMXH_EventHandle waitHandles[3];
	waitHandles[0] = m_trapEvent;
	waitHandles[1] = m_stopEvent;
	waitHandles[2] = m_shutdownEvent;
	int shutdown_Event_Index = 2;
	int stop_Event_Index = 1;
	int trap_Event_Index = 0;

	while(!m_exit)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " SNMP Handler Thread started" << std::endl;
		int pollTime = 300000; //default 5 minutes
		int time = 300; 
		if(IMM_Interface::getSnmpPollingTime(time) == true)
		{
			pollTime = time * 1000;
		}

		int returnValue = FIXS_CMXH_Event::WaitForEvents(3, waitHandles, pollTime);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Event signaled......" << std::endl;
		if (m_exit) break;

		else if (returnValue ==  WAIT_FAILED)   // Event failed
		{

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);

			m_exit = true;
		}

		else if (returnValue ==  WAIT_TIMEOUT)  // Time out
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " 5 minutes Overs..check the CMX ports" << std::endl;
			openCMXBPorts();
			configureBackPortsAsEdgePort();
			handleRSTP();
			checkVlanSettings();
		}
		else if (returnValue ==  WAIT_OBJECT_0 + trap_Event_Index) // trap received
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Trap received..check CMX ports" << std::endl;
			char tmpStr[512] = {0};
			int sid = syscall(SYS_gettid);
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPHandler.cpp] TRAP: ColdStart or WarmStart received (threadid:%d) (ip:%s)", sid, m_IP_EthA.c_str());
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN); //TR_HX74057
			openCMXBPorts();
			configureBackPortsAsEdgePort();
			handleRSTP();
			performVlanSettings();
			FIXS_CMXH_Event::ResetEvent(m_trapEvent);
		}
		else if (returnValue ==  (WAIT_OBJECT_0 + stop_Event_Index))  // stop event
		{
			FIXS_CMXH_Event::ResetEvent(m_stopEvent);
			m_exit = true;    // Terminate the thread
		}
		else if (returnValue ==  (WAIT_OBJECT_0 + shutdown_Event_Index))
		{
			m_exit = true;    // Terminate the thread	
		}	
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
		}
	}
	m_running = false;
	return result;
}

void FIXS_CMXH_SNMPHandler::openCMXBPorts ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;

	char tmpStr[512] = {0};
	int sid = syscall(SYS_gettid);
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPHandler] openCMXBPorts !!! (threadid:%d) (ip:%s)", sid, m_IP_EthA.c_str()); // TR_HX74057
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057

	if (m_SW_Version == 4)
	{
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] CMX 4.0 openCMXBPorts !!!",LOG_LEVEL_DEBUG);
		/* list of ports for CMX4.0 are (1,6,11,...174).
			BP1 to BP24 port (1,6,11,21...116) are Backplane ports, ports 121 and 126 are CCIB_B3 and CCIB_B4
			E1 to E8 front ports (131,136..166)
			GE ports (GE_1,GE_2,GE_3,GE_4)
		 */
		int a[36] = {1,6,11,16,21,26,31,36,41,46,51,56,61,66,71,76,81,86,91,96,101,106,111,116,131,136,141,146,151,156,161,166};

		for( int i=0; i < 36; i++)
		{
			std::cout<<"port id === "<< a[i] <<endl;
			openport(a[i]);
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
	}
	else if (m_SW_Version == 2)
	{
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] CMX 2.0 for openCMXBPorts !!!",LOG_LEVEL_DEBUG);
		for(int i=5; i <= 36; i++)  // 5-12 front port and 13-36 back port
		{
			openport (i);
		}
	}
	else
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] faulty value. m_SW_Version = %d ", __FUNCTION__, __LINE__, m_SW_Version);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057
	}
}

void FIXS_CMXH_SNMPHandler::openport (int i)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	FIXS_CMXH_SNMPManager* manager = new FIXS_CMXH_SNMPManager(m_IP_EthA);
	int valTemp = 0, statusUp = 1;

	if(!manager->getIfMibAdminStatus(i,valTemp))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
			manager->setIpAddress(m_IP_EthB);
		else
			manager->setIpAddress(m_IP_EthA);
		if(!manager->getIfMibAdminStatus(i,valTemp))
		{
			delete manager;
			return;
		}
	}
	if( valTemp != statusUp)
	{
		if(!manager->setIfMibAdminStatus(i,statusUp))
		{
			std::string tmpIp = manager->getIpAddress();
			if(tmpIp.compare(m_IP_EthA) == 0)
				manager->setIpAddress(m_IP_EthB);
			else
				manager->setIpAddress(m_IP_EthA);

			if(!manager->setIfMibAdminStatus(i,statusUp))
			{
				// trace: snmp set failed
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed", __LINE__);
					_trace->ACS_TRA_event(1,tmpStr);

				}
				delete manager;
				return;
			}
		}
	}
	delete manager;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
}

void FIXS_CMXH_SNMPHandler::configureBackPortsAsEdgePort ()
{
	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	 std::cout<<"ver in open port ==="<< m_SW_Version <<endl;

	 char tmpStr[512] = {0};
	 int sid = syscall(SYS_gettid);
	 snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPHandler] configureBackPortsAsEdgePort !!! (threadid:%d) (ip:%s)", sid, m_IP_EthA.c_str()); // TR_HX74057
	 if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057

	 	if (m_SW_Version == 4)
	 	{
	 		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] CMX 4.0 for configureBackPortsAsEdgePort !!!",LOG_LEVEL_DEBUG);
	 		//CMX4.0 Backplane ports are BP1 to BP24 port (1,6,11,21...116)
	 		int a[24] = {1,6,11,16,21,26,31,36,41,46,51,56,61,66,71,76,81,86,91,96,101,106,111,116};
	 		for( int i=0; i < 24 ; i++)
	 		{
	 			std::cout<<"edge port === "<< a[i] <<endl;
	 			BackPortsAsEdgePort (a[i]);
	 		}

	 	}
	 	else if (m_SW_Version == 2)
	 	{
	 		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] CMX 2.0 for configureBackPortsAsEdgePort !!!",LOG_LEVEL_DEBUG);
	 		for(int i = 13; i <= 36; i++)
	 		{
	 			BackPortsAsEdgePort (i);
	 		}
	 	}
	 	else
	 	{
	 		char tmpStr[512] = {0};
	 		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] faulty value. m_SW_Version = %d ", __FUNCTION__, __LINE__, m_SW_Version);
	 		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
	 		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057

	 	}
	 	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
}

void FIXS_CMXH_SNMPHandler::BackPortsAsEdgePort (int i)
{
	int edgePort = 1, status = -1;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	FIXS_CMXH_SNMPManager* manager = new FIXS_CMXH_SNMPManager(m_IP_EthA);

	if(!manager->getStpPortAsEdgePort(i,status))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
			manager->setIpAddress(m_IP_EthB);
		else
			manager->setIpAddress(m_IP_EthA);

		if(!manager->getStpPortAsEdgePort(i,status))
		{
			// trace: snmp get failed
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP get failed", __LINE__);
				_trace->ACS_TRA_event(1,tmpStr);
			}
			delete manager;
			return;
		}
	}
	if( status != edgePort )
	{
		if(!manager->setStpPortAsEdgePort(i,edgePort))
		{
			std::string tmpIp = manager->getIpAddress();
			if(tmpIp.compare(m_IP_EthA) == 0)
				manager->setIpAddress(m_IP_EthB);
			else
				manager->setIpAddress(m_IP_EthA);

			if(!manager->setStpPortAsEdgePort(i,edgePort))
			{
				// trace: snmp set failed
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed", __LINE__);
					_trace->ACS_TRA_event(1,tmpStr);
				}
				delete manager;
				return;
			}
		}
	}
	delete manager;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;		
}

void FIXS_CMXH_SNMPHandler::assignDefaults(vlanParameterStruct data[], ACE_UINT32 &SIZE)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	SIZE=1;
	// 180 vlan
	data[0].id = 180;
	data[0].name.assign("EVOET-INGRESS");
	data[0].priority = 4;
	if(m_SW_Version==4)
	{
		data[0].portMask.assign("8421084210842108421084210842100021084210840000000000");
		data[0].untaggedPortMask.assign("0000000000000000000000000000000000000000000000000000");
	}
	else
	{
		data[0].portMask.assign("0FFFFFFFF0000000");
		data[0].untaggedPortMask.assign("0000000000000000");
	}

	// DRBD vlan if exist	
	ACE_INT32 id = CMXH_Util::getDRBDVlan();
	if (id != -1){
		SIZE=2;
		data[1].id = id;
		data[1].name.assign("APG-DRBD-VLAN");
		data[1].priority = 4;
		if(m_SW_Version==4)
		{
			data[1].portMask.assign("8421084210842108421084210842100021084210840000000000");
			data[1].untaggedPortMask.assign("0000000000000000000000000000000000000000000000000000");
		}
		else
		{
			data[1].portMask.assign("0FFFFFFFF0000000");
			data[1].untaggedPortMask.assign("0000000000000000");
		}
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
}

void FIXS_CMXH_SNMPHandler::vlanList(vlanParameterStruct data[], ACE_UINT32 &SIZE)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	vlanParameterStruct tmpData;
	std::vector<std::string> dn_list;
	ACE_UINT32 index = 0;

	if (!IMM_Util::fetchDn(dn_list)){
		assignDefaults(data, SIZE);
	}else{
		for (std::vector<std::string>::iterator it = dn_list.begin(); it != dn_list.end(); it++ ) {

			if(!IMM_Interface::getVlanParameter(*it, tmpData,m_SW_Version)){
				assignDefaults(data, SIZE);
				index=SIZE;
				break;
			}else{
				if (tmpData.id == IMM_Util::DEFAULT_DRBD_ID)
				{
					continue;
				}

				data[index].id = tmpData.id;
				data[index].name.assign(tmpData.name);
				data[index].priority = tmpData.priority;
				data[index].portMask.assign(tmpData.portMask);
				data[index].untaggedPortMask.assign(tmpData.untaggedPortMask);
				index++;
			}
		}
		SIZE=index;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;

}

void FIXS_CMXH_SNMPHandler::checkVlanSettings()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	char tmpStr[512] = {0};
        int sid = syscall(SYS_gettid);
        snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPHandler] checkVlanSettings !!! (threadid:%d) (ip:%s)", sid, m_IP_EthA.c_str()); // TR_HX74057
        if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057

	vlanParameterStruct data[MAX_VLAN_COUNT];
	ACE_UINT32 SIZE = 0; 
	vlanList(data, SIZE);
	for (unsigned int index=0; index<SIZE; index++){
		if (!checkVlan(data[index]) && !applyVlanSettings(data[index])){
			char tmpStr[512] = {0};
            snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] failed checkVlanSettings: VlanID=%d", __LINE__ ,data[index].id);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "failed checkVlanSettings: VlanID=" << data[index].id << std::endl;
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057
		}
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
}

bool FIXS_CMXH_SNMPHandler::checkVlan(vlanParameterStruct data)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	FIXS_CMXH_SNMPManager* manager = new FIXS_CMXH_SNMPManager(m_IP_EthA);
	manager->setIpAddress(m_IP_EthA);

	// check VLAN is activated or not
	// i.e., getMibVlanStaticRowStatus is 1 (active) or not
	int rowStatus = -1;
	if(!manager->getMibVlanStaticRowStatus(data.id,rowStatus))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->getMibVlanStaticRowStatus(data.id,rowStatus))
		{
			// trace: snmp get failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP failed to get row status of VLAN", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;				
			return false;
		}
	}
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"..MibVlanStaticRowStatus :"<<rowStatus<<std::endl;

	if( rowStatus != 1 ) // 1 means activate
	{
		delete manager;	
		return false;
	}

	// check VLAN name i.e., getMibVlanStaticName
	string vlanStaticName = "";
	if(!manager->getMibVlanStaticName(data.id,vlanStaticName))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->getMibVlanStaticName(data.id,vlanStaticName))
		{
			// trace: snmp get failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP get failed to get VLAN name", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;
			return false;
		}
	}
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"..MibVlanStaticName :"<<vlanStaticName.c_str()<<std::endl;

	if( vlanStaticName != data.name )
	{
		delete manager;	
		return false;
	}

	// check VLAN EGRESS Ports
	string vlanEgressPort = "";
	if(!manager->getMibVlanStaticEgressPorts(data.id,vlanEgressPort))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->getMibVlanStaticEgressPorts(data.id,vlanEgressPort))
		{
			// trace: snmp get failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP get failed to get VLAN EGRESS PORT", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;
			return false;
		}
	}
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"..MibVlanStaticEgressPorts :"<<vlanEgressPort.c_str()<<std::endl;

	//-----------------------------------
	// format egressport value from mib i.e.,
	// converting from 0F FF FF FF F0 00 00 00 00 00 00 00 to 0FFFFFFFF000000000000000
	string tempEgressPort = "";
	for(int i=0;i<(int)vlanEgressPort.length();i++)
		if((vlanEgressPort[i] != ' ') && (vlanEgressPort[i] != '\n') && (vlanEgressPort[i] != '\t'))
		{
			tempEgressPort += vlanEgressPort[i];
		}
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"..tempEgressPort :"<<tempEgressPort.c_str()<<std::endl;
	//-----------------------------------
	if( tempEgressPort.compare(0,(data.portMask).length(),data.portMask))
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" VlanStaticEgressPorts is not proper" <<std::endl;
		delete manager;	
		return false;
	}

	// check VLAN Untagged Ports
	string vlanUntaggedPort = "";
	if(!manager->getMibVlanStaticUntaggedPorts(data.id,vlanUntaggedPort))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->getMibVlanStaticUntaggedPorts(data.id,vlanUntaggedPort))
		{
			// trace: snmp get failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP failed to get VLAN Untagged PORT", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;				
			return false;
		}
	}
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"..MibVlanStaticUntaggedPorts :"<<vlanUntaggedPort.c_str()<<std::endl;
	//-----------------------------------
	// format vlanUntaggedPort value from mib i.e.,
	// converting from 00 00 00 00 00 00 00 00 00 00 00 00 to 000000000000000000000000
	string tempUntaggedPort = "";
	for(int i=0;i<(int)vlanUntaggedPort.length();i++)
		if((vlanUntaggedPort[i] != ' ')   && (vlanUntaggedPort[i] != '\n') && (vlanUntaggedPort[i] != '\t'))
		{
			tempUntaggedPort += vlanUntaggedPort[i];
		}
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"..tempUntaggedPort :"<<tempUntaggedPort.c_str()<<std::endl;
	//-----------------------------------
	if( tempUntaggedPort.compare(0,(data.untaggedPortMask).length(),data.untaggedPortMask))		
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<" vlanUntaggedPort is not proper" <<std::endl;
		delete manager;	
		return false;
	}

	// check default user priority for CMXB ports
	int vlanUserPriority = -1;
	int portCount = 0, increment = 1;
	if (m_SW_Version == 2)
	{
		portCount = 36;
		increment = 1;
	}
	else
	{
		portCount = 174;
		increment = 5;
	}

	for(int i=1; i <= portCount; i = i+increment)
	{
		if (i!=121 && i!=126)
		{
			if(!manager->getMibPortDefaultUserPriority(i,vlanUserPriority))
			{
				std::string tmpIp = manager->getIpAddress();
				if(tmpIp.compare(m_IP_EthA) == 0)
				{
					manager->setIpAddress(m_IP_EthB);
				}
				else
				{
					manager->setIpAddress(m_IP_EthA);
				}

				if(!manager->getMibPortDefaultUserPriority(i,vlanUserPriority))
				{
					// trace: snmp get failed
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP get failed for VLAN priority = %d", __LINE__,i);
					if (_trace->ACS_TRA_ON())
						_trace->ACS_TRA_event(1,tmpStr);
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					delete manager;
					return false;
				}
			}
			if(i==171)
			{
				increment=1;
			}
		}
	}
	delete manager;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
	return true;
}

void FIXS_CMXH_SNMPHandler::performVlanSettings ()
{
	char tmpStr[512] = {0};
        int sid = syscall(SYS_gettid);
        snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPHandler] performVlanSettings !!! (threadid:%d) (ip:%s)", sid, m_IP_EthA.c_str()); // TR_HX74057
        if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	vlanParameterStruct data[MAX_VLAN_COUNT];
	ACE_UINT32 SIZE = 0; // default vlan count
	vlanList(data, SIZE);

	for (unsigned int index=0; index<SIZE; index++){

		if (!applyVlanSettings(data[index])){
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] failed applyVlanSettings: VlanID=%d", __LINE__ ,data[index].id);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "failed applyVlanSettings: VlanID=" << data[index].id << std::endl;
		}	
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
}


bool FIXS_CMXH_SNMPHandler::applyVlanSettings(vlanParameterStruct data)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	//Doing the SNMP setting of VLAN to CMXB
	FIXS_CMXH_SNMPManager* manager = new FIXS_CMXH_SNMPManager(m_IP_EthA);

	manager->setIpAddress(m_IP_EthA);
	//destroy the existing VLAn snmp settings
	//set snmp to destry VLAN for VLAN ID
	if(!manager->setMibVlanStaticRowStatus(data.id,6))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->setMibVlanStaticRowStatus(data.id,6))
		{
			// trace: snmp set failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed to destroy VLAN", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
		}
	}
	//set snmp to create VLAN for VLAN ID
	if(!manager->setMibVlanStaticRowStatus(data.id,5))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->setMibVlanStaticRowStatus(data.id,5))
		{
			// trace: snmp set failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed to create VLAN", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;
			return false;
		}
	}
	//set snmp to activate VLAN for VLAN ID
	if(!manager->setMibVlanStaticRowStatus(data.id,1))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->setMibVlanStaticRowStatus(data.id,1))
		{
			// trace: snmp set failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed to activate VLAN", __LINE__);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;
			return false;
		}
	}

	// set snmp for EGRESS Ports
	if(!manager->setMibVlanStaticEgressPorts(data.id,data.portMask))
	{

		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->setMibVlanStaticEgressPorts(data.id,data.portMask))
		{
			// trace: snmp set failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed to set VLAN EGRESS PORT", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;
			return false;
		}
	}

	// set snmp for Untagged Ports
	if(!manager->setMibVlanStaticUntaggedPorts(data.id,data.untaggedPortMask))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->setMibVlanStaticUntaggedPorts(data.id,data.untaggedPortMask))
		{
			// trace: snmp set failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed to set VLAN Untagged PORT", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;
			return false;
		}
	}

	// set snmp for default user priority for CMXB ports
	int portCount = 0, increment = 1;
	if (m_SW_Version == 2)
	{
		portCount = 36;
		increment = 1;
	}
	else
	{
		portCount = 174;
		increment = 5;
	}
	for(int i=1;i<=portCount; i=i+increment)
	{
		if ( i!=121 && i!=126)
		{
			if(!manager->setMibPortDefaultUserPriority(i,data.priority))
			{
				std::string tmpIp = manager->getIpAddress();
				if(tmpIp.compare(m_IP_EthA) == 0)
				{
					manager->setIpAddress(m_IP_EthB);
				}
				else
				{
					manager->setIpAddress(m_IP_EthA);
				}

				if(!manager->setMibPortDefaultUserPriority(i,data.priority))


				{
					// trace: snmp set failed
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed to set VLAN priority for PORT = %d", __LINE__,i);
					if (_trace->ACS_TRA_ON())
						_trace->ACS_TRA_event(1,tmpStr);
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					delete manager;
					return false;
				}
			}
		}
		if(i == 171)
		{
			increment = 1;
		}
	}

	// set snmp for VLAN name
	if(!manager->setMibVlanStaticName(data.id,data.name))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
		{
			manager->setIpAddress(m_IP_EthB);
		}
		else
		{
			manager->setIpAddress(m_IP_EthA);
		}

		if(!manager->setMibVlanStaticName(data.id,data.name))
		{
			// trace: snmp set failed
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed to set VLAN name", __LINE__);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1,tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			delete manager;
			return false;
		}
	}
	delete manager;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
	return true;


}
void FIXS_CMXH_SNMPHandler::setCMXBAddress (std::string &IP_EthA, std::string &IP_EthB)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] setCMXBAddress !!!",LOG_LEVEL_DEBUG);
	m_IP_EthA = IP_EthA;
	m_IP_EthB = IP_EthB;
}

void FIXS_CMXH_SNMPHandler::setCMXSwVersion(int version)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] setCMXSwVersion !!!",LOG_LEVEL_DEBUG);
	m_SW_Version = version;
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] m_SW_Version = %d ", __FUNCTION__, __LINE__, m_SW_Version);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}
}
	void FIXS_CMXH_SNMPHandler::setTrap ()
	{
		FIXS_CMXH_Event::SetEvent(m_trapEvent);
	}

	void FIXS_CMXH_SNMPHandler::stop ()
	{
		FIXS_CMXH_Event::SetEvent(m_stopEvent);
		m_exit = true;
	}

	void FIXS_CMXH_SNMPHandler::deleteOldLogsSNMP ()
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << ": IN" << std::endl;
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] deleteOldLogsSNMP !!!",LOG_LEVEL_DEBUG);

		FIXS_CMXH_SNMPManager* manager = new FIXS_CMXH_SNMPManager(m_IP_EthA);

		int index=0;
		std::string contFileName = "";
		for (index=0; index<=10;index++)
		{
			contFileName = "";
			// Get the container file name for each index
			if(!manager->getContFileName(index,contFileName))
			{
				std::string tmpIp = manager->getIpAddress();
				if(tmpIp.compare(m_IP_EthA) == 0)
				{
					manager->setIpAddress(m_IP_EthB);
				}
				else
				{
					manager->setIpAddress(m_IP_EthA);
				}
				if(!manager->getContFileName(index,contFileName))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "FAILED to fetch file name for index ="<<index<<std::endl;
					if (_trace->ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "FAILED to fetch file name for index");
						_trace->ACS_TRA_event(1,tmpStr);
						if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						continue;
					}
				}
			}

			std::cout << __FUNCTION__ << "@" << __LINE__ << "container file name = " << contFileName.c_str() << std::endl;

			// check container name is valid
			size_t found = contFileName.find("Invalid");
			if(found != std::string::npos)
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ <<"Contailer file name invalid for index : "<< index << std::endl;
				continue;
			}

			//Set to delete each container file with name obtained above
			{
				if(!manager->setContFileToDelete(contFileName))
				{
					std::string tmpIp = manager->getIpAddress();
					if(tmpIp.compare(m_IP_EthA) == 0)
					{
						manager->setIpAddress(m_IP_EthB);
					}
					else
					{
						manager->setIpAddress(m_IP_EthA);
					}
					if(!manager->setContFileToDelete(contFileName))
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Failed to delete file with name="<<contFileName.c_str()<<std::endl;
						if (_trace->ACS_TRA_ON())
						{ //trace
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "Failed to delete file with name= %s", contFileName.c_str());
							_trace->ACS_TRA_event(1,tmpStr);
							if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}
					}
					else
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Successfully deleted file with name="<<contFileName.c_str()<<std::endl;
						if (_trace->ACS_TRA_ON())
						{ //trace
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "Successfully deleted file with name= %s",contFileName.c_str());
							_trace->ACS_TRA_event(1,tmpStr);
							if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}

					}
				}
				else
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Successfully deleted file with name="<<contFileName.c_str()<<std::endl;
					if (_trace->ACS_TRA_ON())
					{ //trace
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "Successfully deleted file with name= %s",contFileName.c_str());
						_trace->ACS_TRA_event(1,tmpStr);
						if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
				}
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "After:File name for index ="<<index<<" is: "<<contFileName.c_str()<<std::endl;
			}
		}
		delete manager;
		return;
	}

void FIXS_CMXH_SNMPHandler::handleRSTP()
{
	char tmpStr[512] = {0};
        int sid = syscall(SYS_gettid);
        snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPHandler] handleRSTP !!! (threadid:%d) (ip:%s)", sid, m_IP_EthA.c_str()); // TR_HX74057
        if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057

	std::cout << __FUNCTION__ << "@" << __LINE__ <<"SW Version === "<<  m_SW_Version << std::endl;
	if (m_SW_Version == 4)
	{
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] CMX 4.0 !!!",LOG_LEVEL_DEBUG);

		/* list of ports for CMX4.0 are (1,6,11,...174).
			BP1 to BP24 port (1,6,11,21...116) are Backplane ports, ports 121 and 126 are CCIB_B3 and CCIB_B4
			E1 to E8 front ports (131,136..166)
			GE ports 171 to 174 (GE_1,GE_2,GE_3,GE_4)
		 */
		int a[36] = {1,6,11,16,21,26,31,36,41,46,51,56,61,66,71,76,81,86,91,96,101,106,111,116,131,136,141,146,151,156,161,166,171,172,173,174};

		for( int i=0; i < 36 ; i++)
		{
			std::cout<<"rstp port === "<< a[i] <<endl;
			disableRSTP(a[i]);
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
	}
	else if(m_SW_Version == 2)
	{
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] CMX 2.0 !!!",LOG_LEVEL_DEBUG);
	}
	else
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] faulty value. m_SW_Version = %d ", __FUNCTION__, __LINE__, m_SW_Version);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN);  //TR_HX74057
	}
}

void FIXS_CMXH_SNMPHandler::disableRSTP(int i)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPHandler] disableRSTP ",LOG_LEVEL_DEBUG);

	FIXS_CMXH_SNMPManager* manager = new FIXS_CMXH_SNMPManager(m_IP_EthA);
	int valTemp = 0, status = 2;
	if(!manager->getBridgeMibStpPortEnable(i,valTemp))
	{
		std::string tmpIp = manager->getIpAddress();
		if(tmpIp.compare(m_IP_EthA) == 0)
			manager->setIpAddress(m_IP_EthB);
		else
			manager->setIpAddress(m_IP_EthA);
		if(!manager->getBridgeMibStpPortEnable(i,valTemp))
		{
			delete manager;
			return;
		}
	}
	if( valTemp != status)
	{
		if(!manager->setBridgeMibStpPortEnable(i,status))
		{
			std::string tmpIp = manager->getIpAddress();
			if(tmpIp.compare(m_IP_EthA) == 0)
				manager->setIpAddress(m_IP_EthB);
			else
				manager->setIpAddress(m_IP_EthA);

			if(!manager->setBridgeMibStpPortEnable(i,status))
			{
				// trace: snmp set failed
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] SNMP set failed", __LINE__);
					_trace->ACS_TRA_event(1,tmpStr);
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

				}
				delete manager;
				return;
			}
		}
	}
	delete manager;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End" << std::endl;
}
