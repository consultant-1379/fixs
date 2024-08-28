/*
 * snmpTrapReceiver.cpp
 *
 */

#include "FIXS_CCH_SNMPTrapReceiver.h"

// Valorize attribute (TRAP)
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidSnmpTrap[] = 						{1,3,6,1,6,3,1,1,4,1,0};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidTLoadReleaseComplete[] = 			{1,3,6,1,4,1,193,177,2,2,3,0,1};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidTIpmiUpgrade[] = 				    {1,3,6,1,4,1,193,177,2,2,1,0,7};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidColdStart[] =                     {1,3,6,1,6,3,1,1,5,1};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidWarmRestart[] =                   {1,3,6,1,4,1,193,177,2,2,5,0,4};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidFwUpgrade[]	=					 {1,3,6,1,4,1,193,177,2,2,3,0,3};
// (VALUE)
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidLoadResult[] = 					{1,3,6,1,4,1,193,177,2,2,3,1,1,0};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidShelfNum[] =                 		{1,3,6,1,4,1,193,177,2,2,1,1,1,0};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidSlotPos[] =                       {1,3,6,1,4,1,193,177,2,2,1,1,3,0};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidIpmiUpgradeResultValue[] =        {1,3,6,1,4,1,193,177,2,2,1,1,13,0};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidPfmUpgradeResult[] =  		    {1,3,6,1,4,1,193,177,2,2,6,0,2};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidPfmUpgradeResultValue[] =			{1,3,6,1,4,1,193,177,2,2,6,1,8,0};
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidPfmSwUpgradeResult[] =  		    {1,3,6,1,4,1,193,177,2,2,6,0,4}; 
 const oid FIXS_CCH_SNMPTrapReceiver::m_oidFwUpgradeResultValue[]	=		{1,3,6,1,4,1,193,177,2,2,3,1,2,0};


FIXS_CCH_SNMPTrapReceiver::FIXS_CCH_SNMPTrapReceiver():ACS_TRAPDS_API() {

}

FIXS_CCH_SNMPTrapReceiver::FIXS_CCH_SNMPTrapReceiver(int l):ACS_TRAPDS_API(l)
{
}

FIXS_CCH_SNMPTrapReceiver::~FIXS_CCH_SNMPTrapReceiver() {
	// TODO Auto-generated destructor stub
}


void FIXS_CCH_SNMPTrapReceiver::handleTrap(ACS_TRAPDS_StructVariable v2)
{

	int iRet;
	long lLoadResult;
	long lShelfNum;
	long lSlotPos;
	long lIpmiUpgradeResult;
	long lPfmUpgradeResultValue;
	long lFwUpgradeResultValue;

	std::string ip_remote_agent("");
	typTrapEnm nTrap;

	// Init
	iRet = TRAP_HANDLER_ERROR;
	lLoadResult = INT_TRAPVALUE_NONE;
	lShelfNum = INT_TRAPVALUE_NONE;
	lSlotPos = INT_TRAPVALUE_NONE;
	lIpmiUpgradeResult = INT_TRAPVALUE_NONE;
	lPfmUpgradeResultValue = INT_TRAPVALUE_NONE;
	lFwUpgradeResultValue = INT_TRAPVALUE_NONE;

	nTrap = enmTrapNull;

	ACS_TRAPDS_varlist td;


	for (std::list<ACS_TRAPDS_varlist>::iterator it = v2.nvls.begin(); it != v2.nvls.end(); it++)
	{
		td=*it;

		if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidSnmpTrap,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidSnmpTrap)/sizeof(oid),td.name.oids, td.name.length) == 0)
		{
			if (netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidTLoadReleaseComplete,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidTLoadReleaseComplete)/sizeof(oid),td.val.objid.oids, td.val.objid.length ) == 0) {
				// Trap type: "LoadReleaseComplete".
				nTrap = enmLoadReleaseComplete;
			}else if (netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidTIpmiUpgrade,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidTIpmiUpgrade)/sizeof(oid),	td.val.objid.oids, td.val.objid.length) == 0){
				// Trap type: "Ipmi".
				nTrap = enmIpmiUpgrade;

			}else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidColdStart,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidColdStart)/sizeof(oid),td.val.objid.oids, td.val.objid.length) == 0 ||
					netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidWarmRestart,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidWarmRestart)/sizeof(oid),	td.val.objid.oids, td.val.objid.length) == 0){
				// Trap type: "ColdStart" or "WarmStart"
				nTrap = enmColdStartWarmStart;
				std::string oid_start("");
				OidToString(td.val.objid.oids,td.val.objid.length,&oid_start);
				std::cout<< "oid : "<<  oid_start << std::endl;
			}else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidPfmUpgradeResult,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidPfmUpgradeResult)/sizeof(oid), td.val.objid.oids, td.val.objid.length) == 0){
				nTrap = enmPfmUpgrade;
			}else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidPfmSwUpgradeResult,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidPfmUpgradeResult)/sizeof(oid), td.val.objid.oids, td.val.objid.length) == 0){
				nTrap = enmPfmUpgrade;
			}else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidFwUpgrade,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidFwUpgrade)/sizeof(oid), td.val.objid.oids, td.val.objid.length) == 0){
				nTrap = enmFwUpgrade;
			}
		}

		// Check for child variables
		if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidLoadResult,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidLoadResult)/sizeof(oid),	td.name.oids, td.name.length) == 0){
			// Set "LoadResult value"
			lLoadResult = td.val.integer;
		}else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidShelfNum,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidShelfNum)/sizeof(oid),	td.name.oids, td.name.length) == 0){
			// Set "ShelfNum value"
			lShelfNum = td.val.integer;
		}else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidSlotPos,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidSlotPos)/sizeof(oid),	td.name.oids, td.name.length) == 0){
			// Set "slotPos value"
			lSlotPos = td.val.integer;
		}else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidIpmiUpgradeResultValue,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidIpmiUpgradeResultValue)/sizeof(oid),	td.name.oids, td.name.length) == 0){
			// Set "ipmiUpgradeResult value"
			lIpmiUpgradeResult = td.val.integer;
		}
		else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidPfmUpgradeResultValue,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidPfmUpgradeResultValue)/sizeof(oid), td.name.oids, td.name.length) == 0){
			// Set "pfmUpgradeResult value"
			lPfmUpgradeResultValue = td.val.integer;
		}
		else if(netsnmp_oid_equals(FIXS_CCH_SNMPTrapReceiver::m_oidFwUpgradeResultValue,sizeof(FIXS_CCH_SNMPTrapReceiver::m_oidFwUpgradeResultValue)/sizeof(oid), td.name.oids, td.name.length) == 0){
			// Set "pfmUpgradeResult value"
			lFwUpgradeResultValue = td.val.integer;
		}

	} // end for

	ip_remote_agent = td.ipTrap;

	// Switch trap type
	switch(nTrap){
	//
	case enmLoadReleaseComplete:
	{
		// Print common part
		std::cout << "*********** TRAP: LoadReleaseComplete lLoadResult = [";
		// Check value
		if(lLoadResult != INT_TRAPVALUE_NONE){
			// Value OK
			std::cout << lLoadResult;
			// Send Ok
			iRet = TRAP_HANDLER_OK;
			FIXS_CCH_UpgradeManager::getInstance()->loadReleaseCompleted( (int) lLoadResult, ip_remote_agent);

		}else{
			// Error: Value is not evaluated!
			std::cout << "ERROR";
		}
		// Print closed part
		std::cout << "]" << std::endl;
		break;
	}
	//
	case enmIpmiUpgrade:
	{
		// Print common part
		std::cout << "*********** TRAP: IpmiUpgrade lShelfNum = [";
		// Check value
		if(lShelfNum != INT_TRAPVALUE_NONE){
			// Value OK
			std::cout << lShelfNum;
			// Send Ok
			iRet = TRAP_HANDLER_OK;
		}else{
			// Error: Value is not evaluated!
			std::cout << "ERROR";
		}
		// Print closed part
		std::cout << "]" << std::endl;
		//
		// Print common part
		std::cout << "*********** TRAP: IpmiUpgrade lSlotPos = [";
		// Check value
		if(lSlotPos != INT_TRAPVALUE_NONE){
			// Value OK
			std::cout << lSlotPos;
		}else{
			// Error: Value is not evaluated!
			std::cout << "ERROR";
			// Send error
			iRet = TRAP_HANDLER_ERROR;
		}
		// Print closed part
		std::cout << "]" << std::endl;
		//
		// Print common part
		std::cout << "*********** TRAP: IpmiUpgrade lIpmiUpgradeResult = [";
		// Check value
		if(lIpmiUpgradeResult != INT_TRAPVALUE_NONE){
			// Value OK
			std::cout << lIpmiUpgradeResult;
		}else{
			// Error: Value is not evaluated!
			std::cout << "ERROR";
			// Send error
			iRet = TRAP_HANDLER_ERROR;
		}
		// Print closed part
		std::cout << "]" << std::endl;

		std::stringstream sSubRack, sVal, sRes;
		sSubRack << lShelfNum;
		sVal << lSlotPos;
		sRes << lIpmiUpgradeResult;

		FIXS_CCH_UpgradeManager::getInstance()->ipmiUpgradeTrap(sSubRack.str(), sVal.str(), sRes.str(), ip_remote_agent);

		break;
	}

	case enmColdStartWarmStart:
	{
		// Print common part
		std::cout << "*********** TRAP: ColdStart or WarmStart received " << std::endl;
		iRet = TRAP_HANDLER_OK;

		FIXS_CCH_UpgradeManager::getInstance()->coldStart(ip_remote_agent);

		break;
	}

	case enmPfmUpgrade:
	{
		std::cout << "*********** TRAP: pfm lPfmUpgradeResultValue = [";
		// Check value
		if(lPfmUpgradeResultValue != INT_TRAPVALUE_NONE){
			// Value OK
			std::cout << lPfmUpgradeResultValue;
			// Send Ok
			iRet = TRAP_HANDLER_OK;

			FIXS_CCH_UpgradeManager::getInstance()->pfmUpgradeResult( (int) lPfmUpgradeResultValue,ip_remote_agent);
		}else{
			// Error: Value is not evaluated!
			std::cout << "ERROR";
		}
		// Print closed part
		std::cout << "]" << std::endl;
		break;
	}

	case enmFwUpgrade:
	{
		// Print common part
		std::cout << "*********** TRAP: loadFwComplete lFwUpgradeResultValue = [";
		// Check value
		if(lFwUpgradeResultValue != INT_TRAPVALUE_NONE){
			// Value OK
			std::cout << lFwUpgradeResultValue;
			// Send Ok
			iRet = TRAP_HANDLER_OK;
			FIXS_CCH_UpgradeManager::getInstance()->fwUpgradeTrap( (int) lFwUpgradeResultValue, ip_remote_agent);
		}else{
			// Error: Value is not evaluated!
			std::cout << "ERROR";
		}
		// Print closed part
		std::cout << "]" << std::endl;
		break;
	}
	//
	default:
		// Trap unknown!
		//std::cout << "*********** UNKNOWN TRAP: " << std::endl;
		break;

	}


}

void FIXS_CCH_SNMPTrapReceiver::OidToString(oid *iodIn, unsigned int len, std::string *strOut)
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
