/*
 * FIXS_TREFC_SNMPTrapReceiver.h
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */
/*
 * snmpTrapReceiver.h
 */

#ifndef SNMPTRAPRECEIVER_H_
#define SNMPTRAPRECEIVER_H_

#include <iostream>
#include <sstream>
#include <fstream>

#include "ACS_TRAPDS_API.h"
#include "FIXS_TREFC_Manager.h"
#include "FIXS_TREFC_TrapSubscriber.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/snmp_transport.h>

#define TRAP_RECEIVER_SVC_OK 1
#define TRAP_RECEIVER_SVC_ERR 0

#define TRAP_HANDLER_ERROR (int)0
#define TRAP_HANDLER_OK 	(int)1

#define INT_TRAPVALUE_NONE		(int)-1

#define INT_TRAP_BUFFERSIZE				(int)256

#define SIZE_DATA 500000

class FIXS_TREFC_SNMPTrapReceiver : public ACS_TRAPDS_API {

public:
	FIXS_TREFC_SNMPTrapReceiver(int l);
	FIXS_TREFC_SNMPTrapReceiver();
	virtual ~FIXS_TREFC_SNMPTrapReceiver();
	virtual void handleTrap(ACS_TRAPDS_StructVariable var);
	void startSubscriber();

	static const oid m_oidSnmpTrap[];
	static const  oid m_oidColdStart[];
	static const  oid m_oidWarmRestart[];

private:

	void OidToString(oid *iodIn, unsigned int len, std::string *strOut);

	FIXS_TREFC_TrapSubscriber* trapSubscr;

	enum typTrapEnm{
		enmTrapNull = 0,
		enmTrapColdStart,
		enmWarmRestart
	};

};

#endif /* SNMPTRAPRECEIVER_H_ */

