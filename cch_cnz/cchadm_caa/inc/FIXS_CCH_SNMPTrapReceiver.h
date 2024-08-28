/*
 * snmpTrapReceiver.h
 */

#ifndef SNMPTRAPRECEIVER_H_
#define SNMPTRAPRECEIVER_H_

#include <iostream>
#include <sstream>
#include <fstream>

#include "ACS_TRAPDS_API.h"

#include "FIXS_CCH_Util.h"
#include "FIXS_CCH_UpgradeManager.h"

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

class FIXS_CCH_SNMPTrapReceiver : public ACS_TRAPDS_API {

public:
	FIXS_CCH_SNMPTrapReceiver(int l);
	FIXS_CCH_SNMPTrapReceiver();
	virtual ~FIXS_CCH_SNMPTrapReceiver();
	virtual void handleTrap(ACS_TRAPDS_StructVariable var);

	static const oid m_oidSnmpTrap[];
	static const  oid m_oidTLoadReleaseComplete[];
	static const  oid m_oidColdStart[];
	static const  oid m_oidWarmRestart[];
	static const  oid m_oidLoadResult[];
	static const  oid m_oidTIpmiUpgrade[];
	static const  oid m_oidShelfNum[];
	static const  oid m_oidSlotPos[];
	static const  oid m_oidIpmiUpgradeResultValue[];
	static const  oid m_oidPfmUpgradeResult[];
	static const  oid m_oidPfmSwUpgradeResult[];
	static const  oid m_oidPfmUpgradeResultValue[];
	static const  oid m_oidFwUpgrade[];
	static const  oid m_oidFwUpgradeResultValue[];

private:

	void OidToString(oid *iodIn, unsigned int len, std::string *strOut);

	enum typTrapEnm{
		enmTrapNull = 0,
		enmTrapColdStart,
		enmWarmRestart,
		enmLoadResult,
		enmLoadReleaseComplete,
		enmIpmiUpgrade,
		enmColdStartWarmStart,
		enmPfmUpgrade,
		enmFwUpgrade
	};

};

#endif /* SNMPTRAPRECEIVER_H_ */
