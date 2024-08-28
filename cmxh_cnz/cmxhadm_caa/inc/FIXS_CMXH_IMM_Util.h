/*
 * FIXS_CMXH_IMM_Util.h
 *
 */

#ifndef FIXS_CMXH_IMM_UTIL_H_
#define FIXS_CMXH_IMM_UTIL_H_

#include "FIXS_CMXH_Event.h"
//#include "Client_Server_Interface.h"

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "ace/OS_NS_poll.h"
#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include <ace/Task.h>

#include "ACS_APGCC_OiHandler.h"
#include "ACS_CC_Types.h"
#include "ACS_APGCC_Util.H"
#include "acs_apgcc_omhandler.h"


namespace IMM_Util
{
	//static const string NAMESPACE_CMXH[]= "AxeBladeSwManagement";
	
	//NEW MODEL
	static const char classVlanParameter[] = "ComponentMainSwitchHandlerVlanParameter";
	static const char classSnmpPolling[] = "ComponentMainSwitchHandlerSnmpPolling";
	


	//CLASS IMPLEMENTERS
	static const char IMPLEMENTER_VLANPARAMETER[] = "FIXS_CMXH_VlanParameter";
	static const char IMPLEMENTER_SNMPPOLLING[] = "FIXS_CMXH_SnmpPolling";
	
	// CMXHVlanParameter CLASS
	static const char ATT_VLANPARAMETER_RDN[] = "ComponentMainSwitchHandlerVlanParameterId";
	static const char ATT_VLAN_NAME[] = "VlanName";
	static const char ATT_VLAN_ID[] = "VlanId";
	static const char ATT_VLAN_PRIORITY[] = "VlanPriority";
	static const char ATT_VLAN_PORTMASK[] = "VlanPortMask";
	static const char ATT_VLAN_UNTAGGED_PORTMASK[] = "VlanUntaggedPortMask";
	static const char ATT_VLAN_PORTMASK_4[] = "vlanPortMask_4";
	static const char ATT_VLAN_UNTAGGED_PORTMASK_4[] = "vlanUntaggedPortMask_4";
	
	//CMXHSnmpPolling CLASS STRUCT
	static const char ATT_SNMPPOLLING_RDN[] = "ComponentMainSwitchHandlerSnmpPollingId";
	static const char ATT_SNMP_POLLING_TIME[] = "SnmpPollingTime";

	//board type
	static const int CMXB = 380;

	// default drbd vlan tag
	static const int DEFAULT_DRBD_ID=99999;

	//default attribute value
	static const char ATT_DEFAULT_STRING[] = "-";
	static const char ATT_DEFAULT_INT[] = "0";

	//functions
	bool getImmAttributeString (std::string object, std::string attribute, std::string &value);
	bool getImmAttributeInt(std::string object, std::string attribute, int &value);
	bool modifyAttribute(std::string dn, ACE_UINT32 id);
	bool fetchDn(std::vector<std::string> &dn_list);
}; // End of namespace


#endif /* FIXS_CMXH_IMM_UTIL_H_ */
