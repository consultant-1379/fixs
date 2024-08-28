/*
 * FIXS_TREFC_SNMPTrapReceiver.cpp
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_SNMPTrapReceiver.h"

// Valorize attribute (TRAP)
const oid FIXS_TREFC_SNMPTrapReceiver::m_oidSnmpTrap[] = 						{1,3,6,1,6,3,1,1,4,1,0};
const oid FIXS_TREFC_SNMPTrapReceiver::m_oidColdStart[] =                     {1,3,6,1,6,3,1,1,5,1};
const oid FIXS_TREFC_SNMPTrapReceiver::m_oidWarmRestart[] =                   {1,3,6,1,4,1,193,177,2,2,5,0,4};


FIXS_TREFC_SNMPTrapReceiver::FIXS_TREFC_SNMPTrapReceiver():
		ACS_TRAPDS_API(),
		trapSubscr(NULL)
{
	trapSubscr = new FIXS_TREFC_TrapSubscriber(this);
}

FIXS_TREFC_SNMPTrapReceiver::FIXS_TREFC_SNMPTrapReceiver(int l):
		ACS_TRAPDS_API(l),
		trapSubscr(NULL)
{
	trapSubscr = new FIXS_TREFC_TrapSubscriber(this);
}

FIXS_TREFC_SNMPTrapReceiver::~FIXS_TREFC_SNMPTrapReceiver() {

	if (trapSubscr) {
		delete(trapSubscr);
		trapSubscr=NULL;
	}
}


void FIXS_TREFC_SNMPTrapReceiver::handleTrap(ACS_TRAPDS_StructVariable v2)
{

	int iRet;
	std::string ip_remote_agent("");
	// Init
	iRet = TRAP_HANDLER_ERROR;

	// Loop on all pdu variable list
	ACS_TRAPDS_varlist td;



	for (std::list<ACS_TRAPDS_varlist>::iterator it = v2.nvls.begin(); it != v2.nvls.end(); it++)
	{
		td=*it;

		if(netsnmp_oid_equals(FIXS_TREFC_SNMPTrapReceiver::m_oidSnmpTrap,sizeof(FIXS_TREFC_SNMPTrapReceiver::m_oidSnmpTrap)/sizeof(oid),td.name.oids, td.name.length) == 0)
		{
			if(netsnmp_oid_equals(FIXS_TREFC_SNMPTrapReceiver::m_oidColdStart,sizeof(FIXS_TREFC_SNMPTrapReceiver::m_oidColdStart)/sizeof(oid),td.val.objid.oids, td.val.objid.length) == 0 ||
					netsnmp_oid_equals(FIXS_TREFC_SNMPTrapReceiver::m_oidWarmRestart,sizeof(FIXS_TREFC_SNMPTrapReceiver::m_oidWarmRestart)/sizeof(oid),	td.val.objid.oids, td.val.objid.length) == 0)
			{
				// Trap type: "ColdStart" or "WarmStart"
				ip_remote_agent = td.ipTrap;

				std::string oid_start("");
				OidToString(td.val.objid.oids,td.val.objid.length,&oid_start);
				std::cout<< "oid : "<<  oid_start << std::endl;

				std::cout<< "COLD/WARM START TRAP RECEIVED!!!!!!!!!!!"<<std::endl;
				std::cout << "*********** TRAP: ColdStart or WarmStart received " << std::endl;
				cout<<"Trap received from ip : "<<ip_remote_agent<<endl;

				FIXS_TREFC_Manager::getInstance()->coldStart(ip_remote_agent);

			}
		} //end if trap
	} // end for
}


void FIXS_TREFC_SNMPTrapReceiver::OidToString(oid *iodIn, unsigned int len, std::string *strOut)
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

void FIXS_TREFC_SNMPTrapReceiver::startSubscriber() {
	if (trapSubscr)
		trapSubscr->open();
}
