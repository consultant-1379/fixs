/*
 * FIXS_TREFC_AptScxHandler.h
 *
 *  Created on: Jan 10, 2012
 *      Author: elalkak
 */

#ifndef FIXS_TREFC_APTSCXHANDLER_H_
#define FIXS_TREFC_APTSCXHANDLER_H_

#include <ace/Task.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>


#include <string.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <vector>

#include "ACS_TRA_trace.h"
#include "FIXS_TREFC_Util.h"
#include "FIXS_TREFC_Event.h"
//#include "FIXS_TREFC_Manager.h"
#include "FIXS_TREFC_SNMPManager.h"


namespace{
	typedef int EventHandle;
}

class FIXS_TREFC_SNMPManager;
using namespace TREFC_Util;

//struct SCXAddress
//{
//	std::string IP_EthA;
//	std::string IP_EthB;
//	unsigned short slotPos;
//};


class FIXS_TREFC_AptScxHandler: public ACE_Task_Base {
// constructor-destructor
public:
	FIXS_TREFC_AptScxHandler(string ipA, string ipB,int port);
	virtual ~FIXS_TREFC_AptScxHandler();

// public methods
public:

	virtual int svc(void);
	virtual int close ();
	int initialize(void);


	inline bool isRunning(void){return m_running;};


	bool confAsNtpClient();
	bool stopNtpClient();
	bool cleanup();
	void setNtpClientConf(vector<SWITCHAPZIPAddress> apzSwitchVector, int port);
	void setStopNtpClient();
	void setTime();
	void setCleanup();
	void setPort(int port){ m_Port = port; }

private:
	bool switchAddress();


//	attributes
private:


	FIXS_TREFC_EventHandle m_shutdownEvent;
	bool m_exit;
	bool m_running;
	bool m_Retry;
	bool m_stopRetry;
	bool m_setTime;
	bool m_cleanup;
	vector<SWITCHAPZIPAddress> m_ApzSwitchVector;
	int m_Port;
//	bool bExit;
//	bool bRun;

	FIXS_TREFC_SNMPManager* m_snmpManager;
	ACS_TRA_trace* traceAptScxHandler;
	ACE_Recursive_Thread_Mutex m_lock;

// static attributes
public:
	string m_ipA;
	string m_ipB;
};


#endif /* FIXS_TREFC_APTSCXHANDLER_H_ */
