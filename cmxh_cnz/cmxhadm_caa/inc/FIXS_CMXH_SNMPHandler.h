#ifndef FIXS_CMXH_SNMPHandler_h
#define FIXS_CMXH_SNMPHandler_h 1

#include "FIXS_CMXH_SNMPManager.h"
#include "FIXS_CMXH_Event.h"
#include "FIXS_CMXH_ImmInterface.h"
#include <iostream>
#include <ace/Task.h>
#include "ACS_TRA_trace.h"
#include "FIXS_CMXH_Logger.h"
#include "ACS_TRAPDS_API.h"
#include "ACS_TRA_Logging.h"

const ACE_UINT32 MAX_VLAN_COUNT=10;

class FIXS_CMXH_SNMPHandler : public ACE_Task_Base {
public:
	FIXS_CMXH_SNMPHandler();

	virtual ~FIXS_CMXH_SNMPHandler();
	/**
	   @brief  		Run by a daemon thread
	 */
	virtual int svc(void);

	/**
	   @brief  		This method initializes a task and prepare it for execution
	 */
	int open (void *args = 0);

	//	This method is used to perform all the SNMP settings to
	//	CMXB like opening Frontplane ports, opening Backplane
	//	ports etc
	void openCMXBPorts ();
	void openport (int i);
	void configureBackPortsAsEdgePort ();
	void BackPortsAsEdgePort (int i);
	void performVlanSettings ();
        void handleRSTP();
	void disableRSTP(int i);
	void deleteOldLogsSNMP ();
	void setCMXBAddress (std::string &IP_EthA, std::string &IP_EthB);
	void setCMXSwVersion(int version);
	void setTrap ();
	void stop();
	inline bool isRunningThread() { return m_running; };

private:
	FIXS_CMXH_EventHandle m_shutdownEvent;
	FIXS_CMXH_EventHandle m_stopEvent;
	FIXS_CMXH_EventHandle	m_trapEvent;
	bool m_exit;
	bool m_running;
	std::string m_IP_EthA;
	std::string m_IP_EthB;
	int m_SW_Version;
	ACS_TRA_trace* _trace;
    ACS_TRA_Logging *FIXS_CMXH_logging;

	void checkVlanSettings();
	bool checkVlan(vlanParameterStruct data);
	void assignDefaults(vlanParameterStruct data[], ACE_UINT32 &SIZE);
	void vlanList(vlanParameterStruct data[], ACE_UINT32 &SIZE);
	bool applyVlanSettings(vlanParameterStruct data);
};

#endif
