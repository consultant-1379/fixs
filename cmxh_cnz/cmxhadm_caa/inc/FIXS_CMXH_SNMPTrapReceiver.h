/*
 * snmpTrapReceiver.h
 */

#ifndef SNMPTRAPRECEIVER_H_
#define SNMPTRAPRECEIVER_H_

#include <iostream>
#include <sstream>
#include <fstream>

#include "ACS_TRAPDS_API.h"
#include "ACS_CS_API.h"

#include "FIXS_CMXH_Util.h"
#include "FIXS_CMXH_Manager.h"
#include "ACS_TRA_Logging.h"
#include "FIXS_CMXH_Logger.h"

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

struct trapInfo
{
	std::string Ipadd;
	uint64_t trapTime;
};

class FIXS_CMXH_SNMPTrapReceiver : public ACS_TRAPDS_API {

public:
	FIXS_CMXH_SNMPTrapReceiver(int l);
	FIXS_CMXH_SNMPTrapReceiver();
	virtual ~FIXS_CMXH_SNMPTrapReceiver();
	virtual void handleTrap(ACS_TRAPDS_StructVariable var);
	bool checkTrapRejectStatus(std::string & ip);

	static const oid m_oidSnmpTrap[];
	static const  oid m_oidColdStart[];
	static const  oid m_oidWarmRestart[];
	static const  oid m_oidShelfNum[];
	static const  oid m_oidSlotPos[];
	static const  oid m_oidContFileResult[];
	static const  oid m_oidContFileValue[];
	static const oid m_oidContFileTransferComplete[];
private:

	void OidToString(oid *iodIn, unsigned int len, std::string *strOut);

	enum typTrapEnm{
		enmTrapNull = 0,
		enmTrapColdStart,
		enmWarmRestart,
		enmColdStartWarmStart,
	};
	std::vector<trapInfo>  trapInfoVector;
	ACS_TRA_Logging *FIXS_CMXH_logging;

        static ACE_Recursive_Thread_Mutex m_lock;  //TR_HX98360

};

#endif /* SNMPTRAPRECEIVER_H_ */
