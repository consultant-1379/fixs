/*
 * FIXS_TREFC_ApzScxHandler.h
 *
 *  Created on: Jan 9, 2012
 *      Author: elalkak
 */

#ifndef FIXS_TREFC_APZSCXHANDLER_H_
#define FIXS_TREFC_APZSCXHANDLER_H_


#include <ace/Task.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
//#include <net-snmp/net-snmp-config.h>
//#include <net-snmp/net-snmp-includes.h>
//#include <net-snmp/library/snmp_transport.h>

#include <string.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <map>

#include "ACS_TRA_trace.h"
#include "FIXS_TREFC_Util.h"
#include "FIXS_TREFC_Event.h"
//#include "FIXS_TREFC_Manager.h"
#include "FIXS_TREFC_SNMPManager.h"


//namespace{
//	typedef int EventHandle;
//}

class FIXS_TREFC_SNMPManager;

class FIXS_TREFC_ApzScxHandler: public ACE_Task_Base {
// constructor-distructor
public:
	FIXS_TREFC_ApzScxHandler(string ipA, string ipB);
	virtual ~FIXS_TREFC_ApzScxHandler();

// public methods
public:

//	typedef enum
//	{
//		TIMEREF_ADD_NTP,	        //0
//		TIMEREF_RM_NTP, //1
//		TIMEREF_ADD_NTPHP,			//2
//		TIMEREF_RM_NTPHP,		//3
//		TIMEREF_START_NTP,	//4
//		TIMEREF_START_NTPHP,	//5
//		TIMEREF_STOP_NTP,	//6
//		TIMEREF_STOP_NTPHP,	//7
//		TIMEREF_RECONF_NTP,	//8
//		TIMEREF_RECONF_NTPHP,	//9
//		TIMEREF_STATUS_NTP,  //10
//		TIMEREF_STATUS_NTPHP,  //11
//		TIMEREF_EXTIP_ADD,	//12
//		TIMEREF_EXTIP_RM,	//13
//		TIMEREF_DEFAULT_NO_EVENT	//14
//	} ApzScxEventId;
//
//	class EventQueue
//	{
//
//	public:
//
//		EventQueue();
//
//		virtual ~EventQueue();
//
//		FIXS_CCH_ScxUpgrade::ScxEventId getFirst ();
//
//		void popData ();
//
//		void pushData (ScxEventId info);
//
//		bool queueEmpty ();
//
//		size_t queueSize ();
//
//		FIXS_TREFC_ApzScxHandler::ApzScxEventId getItem (int index);
//
//		int getHandle () const;
//
//
//		// Additional Public Declarations
//
//
//	protected:
//		// Additional Protected Declarations
//
//	private:
//
//		EventQueue(const EventQueue &right);
//
//		EventQueue & operator=(const EventQueue &right);
//
//		// Data Members for Class Attributes
//
//
//		//	Handle to wake up the thread that is waiting for events on this queue
//
//		int m_queueHandle;
//
//		ACE_thread_mutex_t  m_queueCriticalSection;
//
//
//		std::list<ScxEventId> m_qEventInfo;
//
//		// Additional Private Declarations
//
//	private:
//		// Additional Implementation Declarations
//
//	};


	virtual int svc(void);
	virtual int close ();
	int initialize(void);


	inline bool isRunning(void){return m_running;};


	int addExtIp(string ip, string defaultGateway, string netmask, bool ntpRestart);
	int rmExtIp();

	int addTimeRefNtp(string timeRefIp);
	int addTimeRefNtphp(string timeRefIp);
	int rmTimeRefNtp(string timeRefIp);
	int rmTimeRefNtphp(string timeRefIp);

	int startNtp(map<string, string> tRefList);
	int startNtphp(map<string, string> tRefList, string otherScxIpA, string otherScxIpB);
	int startNtphpGps(string otherScxIpA, string otherScxIpB);

	int stopNtp();
	int stopNtphp();

	int getTimeRefStatusNtp(int *currentMode, string *activeRef);
	int getTimeRefStatusNtphp(int *sntpStatus, string *activeRef);

	bool reconfNtp();
	bool reconfNtphp();

	bool cleanup();

	void setNtpConf(map<string, string> RefList, string ip, string defGateway, string nmask);
	void setNtpHpConf(map<string, string> RefList, string ip, string defGateway, string nmask, string otherScxIpA, string otherScxIpB);

	void setAddNtpRefRetry(string refIp);
	void setAddNtpHpRefRetry(string refIp);
	void setRmNtpRefRetry(string refIp);
	void setRmNtpHpRetry(string refIp);
	void setAddExtIpRetry(string ip, string defGateway, string nmask, bool restart);
	void setRmExtIpRetry();
	void setStopNtpRetry();
	void setStopNtpHpRetry();
	void setTime();
	void setCleanup();


private:
	bool switchAddress();


//	attributes
private:


	FIXS_TREFC_EventHandle m_shutdownEvent;
	bool m_exit;
	bool m_running;
	bool m_NtpRetry;
	bool m_NtpHpRetry;
	map<string, string> tRefList;
	string extIp;
	string defaultGateway;
	string netmask;
	bool ntpdRestart;
	bool m_addNtpRefRetry;
	bool m_addNtpHpRefRetry;
	bool m_stopNtpRetry;
	bool m_stopNtpHpRetry;
	bool m_addExtIpRetry;
	bool m_rmExtIpRetry;
	bool m_rmNtpRefRetry;
	bool m_rmNtpHpRefRetry;
	bool m_isRetryOngoing;
	bool m_setTime;
	bool m_cleanup;

	std::vector<string> m_timeRefAdd;
	std::vector<string> m_timeRefRemove;
	string m_otherScxA;
	string m_otherScxB;
//	bool bExit;
//	bool bRun;
	FIXS_TREFC_SNMPManager* m_snmpManager;
	ACS_TRA_trace* traceApzScxHandler;
	ACE_Recursive_Thread_Mutex m_lock;

// static attributes
public:
	string m_ipA;
	string m_ipB;



};


#endif /* FIXS_TREFC_ApzScxHandler_H_ */
