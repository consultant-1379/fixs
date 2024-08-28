/*
 * snmpTrapReceiver.cpp
 *
 */

#include "FIXS_CMXH_SNMPTrapReceiver.h"
//#include "ACS_TRA_trace.h"
#include "FIXS_CMXH_Logger.h"
#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"


// Valorize attribute (TRAP)
const oid FIXS_CMXH_SNMPTrapReceiver::m_oidSnmpTrap[] = 						{1,3,6,1,6,3,1,1,4,1,0};
const oid FIXS_CMXH_SNMPTrapReceiver::m_oidColdStart[] =                     {1,3,6,1,6,3,1,1,5,1};
const oid FIXS_CMXH_SNMPTrapReceiver::m_oidWarmRestart[] =                   {1,3,6,1,4,1,193,177,2,2,5,0,4};
// (VALUE)
const oid FIXS_CMXH_SNMPTrapReceiver::m_oidShelfNum[] =                 		{1,3,6,1,4,1,193,177,2,2,1,1,1,0};
const oid FIXS_CMXH_SNMPTrapReceiver::m_oidSlotPos[] =                       {1,3,6,1,4,1,193,177,2,2,1,1,3,0};
const oid FIXS_CMXH_SNMPTrapReceiver::m_oidContFileValue[] =                   {1,3,6,1,4,1,193,177,2,2,4,2,1,0};
const oid FIXS_CMXH_SNMPTrapReceiver::m_oidContFileResult[] =                  {1,3,6,1,4,1,193,177,2,2,4,2,2,0};
const oid FIXS_CMXH_SNMPTrapReceiver::m_oidContFileTransferComplete[] =	       {1,3,6,1,4,1,193,177,2,2,4,1,2};

ACE_Recursive_Thread_Mutex FIXS_CMXH_SNMPTrapReceiver::m_lock;  //TR_HX98360

FIXS_CMXH_SNMPTrapReceiver::FIXS_CMXH_SNMPTrapReceiver():ACS_TRAPDS_API() {
	trapInfoVector.clear();
	FIXS_CMXH_logging = FIXS_CMXH_Logger::getLogInstance();
}

FIXS_CMXH_SNMPTrapReceiver::FIXS_CMXH_SNMPTrapReceiver(int l):ACS_TRAPDS_API(l)
{
	trapInfoVector.clear();
}

FIXS_CMXH_SNMPTrapReceiver::~FIXS_CMXH_SNMPTrapReceiver()
{
	FIXS_CMXH_logging = 0;
}


void FIXS_CMXH_SNMPTrapReceiver::handleTrap(ACS_TRAPDS_StructVariable v2)
{
	if (FIXS_CMXH_Manager::getInstance()->isCMXBVctrEmpty())  //HY36723
		return;

	int iRet;
	long lShelfNum;
	long lSlotPos;


	std::string ip_remote_agent("");
	typTrapEnm nTrap;
	int contFileResult = 0;
	std::string contFileNameValue("");
	bool CMXLogFlag = false;

	// Init
	iRet = TRAP_HANDLER_ERROR;
	lShelfNum = INT_TRAPVALUE_NONE;
	lSlotPos = INT_TRAPVALUE_NONE;

	nTrap = enmTrapNull;

	ACS_TRAPDS_varlist td;


	for (std::list<ACS_TRAPDS_varlist>::iterator it = v2.nvls.begin(); it != v2.nvls.end(); it++)
	{
		td=*it;

		if(netsnmp_oid_equals(FIXS_CMXH_SNMPTrapReceiver::m_oidSnmpTrap,sizeof(FIXS_CMXH_SNMPTrapReceiver::m_oidSnmpTrap)/sizeof(oid),td.name.oids, td.name.length) == 0)
		{
			if(netsnmp_oid_equals(FIXS_CMXH_SNMPTrapReceiver::m_oidColdStart,sizeof(FIXS_CMXH_SNMPTrapReceiver::m_oidColdStart)/sizeof(oid),td.val.objid.oids, td.val.objid.length) == 0 ||
					netsnmp_oid_equals(FIXS_CMXH_SNMPTrapReceiver::m_oidWarmRestart,sizeof(FIXS_CMXH_SNMPTrapReceiver::m_oidWarmRestart)/sizeof(oid),	td.val.objid.oids, td.val.objid.length) == 0){
				// Trap type: "ColdStart" or "WarmStart"
				nTrap = enmColdStartWarmStart;
				std::string oid_start("");
				OidToString(td.val.objid.oids,td.val.objid.length,&oid_start);
				std::cout<< "oid : "<<  oid_start << std::endl;
			}
		}

		// Check for child variables
		if(netsnmp_oid_equals(FIXS_CMXH_SNMPTrapReceiver::m_oidShelfNum,sizeof(FIXS_CMXH_SNMPTrapReceiver::m_oidShelfNum)/sizeof(oid),	td.name.oids, td.name.length) == 0){
			// Set "ShelfNum value"
			lShelfNum = td.val.integer;
		}else if(netsnmp_oid_equals(FIXS_CMXH_SNMPTrapReceiver::m_oidSlotPos,sizeof(FIXS_CMXH_SNMPTrapReceiver::m_oidSlotPos)/sizeof(oid),	td.name.oids, td.name.length) == 0){
			// Set "slotPos value"
			lSlotPos = td.val.integer;
		}

		if(netsnmp_oid_equals(FIXS_CMXH_SNMPTrapReceiver::m_oidContFileResult,sizeof(FIXS_CMXH_SNMPTrapReceiver::m_oidContFileResult)/sizeof(oid), td.name.oids, td.name.length) == 0){
			contFileResult = td.val.integer;
			std::cout << "DBG: in Handle Trap contFileresult= " << contFileResult << std::endl;
		}

		if(netsnmp_oid_equals(FIXS_CMXH_SNMPTrapReceiver::m_oidContFileValue,sizeof(FIXS_CMXH_SNMPTrapReceiver::m_oidContFileValue)/sizeof(oid), td.name.oids, td.name.length) == 0){
			contFileNameValue = td.val.string;
			std::cout << "DBG: in Handle Trap container file name= " << contFileNameValue.c_str() << std::endl;
		}

		if(netsnmp_oid_equals(FIXS_CMXH_SNMPTrapReceiver::m_oidContFileTransferComplete,sizeof(FIXS_CMXH_SNMPTrapReceiver::m_oidContFileTransferComplete)/sizeof(oid),td.val.objid.oids, td.val.objid.length) == 0)
		{
			CMXLogFlag = true;
		}


	} // end for

	if (nTrap != enmColdStartWarmStart)  //HY36723
		return;

	if(!FIXS_CMXH_Manager::getInstance()->isCMXB(lShelfNum, lSlotPos,td.ipTrap ))
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]FIXS_CMXH_SNMPManager::handleTrap ----- Not CMX board, do nothing ------", __FUNCTION__,__LINE__);
		std::cout << "DBG: " << tmpStr << std::endl;
		return;
	}
	std::cout << "DBG: Trap IP address is : " << td.ipTrap.c_str() << std::endl;
	if(CMXLogFlag && contFileResult)
	{
		CMXH_Util::renameCopiedContFile(contFileNameValue, (td.ipTrap));
	}


	ip_remote_agent = td.ipTrap;


	// Switch trap type
	switch(nTrap){
	//
	case enmColdStartWarmStart:
	{
		// Print common part
		std::cout << "*********** TRAP: ColdStart or WarmStart received " << std::endl;
		char tmpStr[512] = {0};
                snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPTrapreceiver.cpp] TRAP: ColdStart or WarmStart received for IP(%s) ", ip_remote_agent.c_str());
                if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN); //TR_HX74057
		//reject second trap from other subnet

		if(checkTrapRejectStatus(ip_remote_agent))
			return;
		iRet = TRAP_HANDLER_OK;

		FIXS_CMXH_Manager::getInstance()->retrySettingAtCMXStart(ip_remote_agent);
		FIXS_CMXH_Manager::getInstance()->deleteLogsInCMX(ip_remote_agent);
		int call_result = FIXS_CMXH_Manager::getInstance()->getLogsFromCMX(ip_remote_agent);
		if(call_result != 0)
			std::cout << "DBG: CMXB log files collection failed ! Call 'sendCmxLog()' returned " << call_result << std::endl; 
		else
			std::cout << "DBG:  CMXB log files collection successfully started !" << std::endl;

		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPTrapreceiver.cpp] Handling RSTP at ColdStartWarmStart Trap",LOG_LEVEL_WARN);  //TR_HX74057

		FIXS_CMXH_Manager::getInstance()->HandleRSTPforCMX4(ip_remote_agent);

		break;
	}

	default:
		// Trap unknown!
		//std::cout << "*********** UNKNOWN TRAP: " << std::endl;
		break;

	}
}

void FIXS_CMXH_SNMPTrapReceiver::OidToString(oid *iodIn, unsigned int len, std::string *strOut)
{
	std::stringstream sStr;
	unsigned int x;
	// Init
	x = 0;
	sStr.str("");
	(*strOut) = "";
	// for all element in oid,
	for (x = 0; x < len ; ++x){
		sStr << '.';
		sStr << (int)iodIn[x];
	}
	// Set output string
	(*strOut) = sStr.str();
}

bool FIXS_CMXH_SNMPTrapReceiver::checkTrapRejectStatus(std::string & ip)
{
	//Lock lock;
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);       //TR_HX98360
        {
         char tmpStr[512] = {0};
         snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPTrapreceiver.cpp] TRAP: ColdStart or WarmStart Trap Mutex acquired !!! (ip:%s)",ip.c_str());
         if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN); //TR_HX98360
        }
       
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Begin" << std::endl;
	bool found = false;
	int i = 0;
	unsigned pos = ip.find_last_of(".");
	std::string lastpart = ip.substr(pos+1);
	for(i=0;i<(int)trapInfoVector.size();i++)
	{
		std::string tempstr = trapInfoVector[i].Ipadd;
		if(tempstr.compare(lastpart) == 0)
		{
			found = true;
			break;
		}
	}
	if(found)
	{
		uint64_t tempcurtime = getEpochTimeInSeconds();
		if((tempcurtime - trapInfoVector[i].trapTime) > (uint64_t)5)
		{
			//remove old trap info and add new trap info in vector
			trapInfoVector.erase(trapInfoVector.begin() + i);
			trapInfo cmxInfo ={"",0};
			cmxInfo.Ipadd = lastpart;
			cmxInfo.trapTime = tempcurtime;
			trapInfoVector.push_back(cmxInfo);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " remove old trap info and add new ; return false" << std::endl;
			return false;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " reject trap; return true" << std::endl;
			char tmpStr[512] = {0};
	                snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_SNMPTrapreceiver.cpp] TRAP: ColdStart or WarmStart rejected !!! (ip:%s)",ip.c_str()); // TR_HX74057
                        if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN); //TR_HX74057
			return true; //reject another trap within 5 second
		}
	}
	else
	{
		//store trap info in vector
		trapInfo cmxInfo ={"",0};
		cmxInfo.Ipadd = lastpart;
		cmxInfo.trapTime = getEpochTimeInSeconds();
		trapInfoVector.push_back(cmxInfo);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " first trap arrived; return false" << std::endl;
		return false;
	}
}
