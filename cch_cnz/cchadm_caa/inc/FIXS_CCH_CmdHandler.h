/*
 * FIXS_CCH_CmdHandler.h
 *
 *  Created on: 29 Jun 2011
 *      Author: eanform
 */

#ifndef FIXS_CCH_CMDHANDLER_H_
#define FIXS_CCH_CMDHANDLER_H_

#include <ace/Task.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <list>
#include <sstream>
#include <string>
//#include <ace/Activation_Queue.h>
//#include <ace/Method_Request.h>

#include "ACS_DSD_Server.h"
#include "ACS_DSD_Session.h"
#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"


#include "Client_Server_Interface.h"
#include "FIXS_CCH_DiskHandler.h"
#include "FIXS_CCH_UpgradeManager.h"
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_Util.h"

class FIXS_CCH_UpgradeManager;



class FIXS_CCH_CmdHandler: public ACE_Task_Base {
 public:
	/**
		@brief	Constructor of FIXS_CCH_CmdHandler class
	*/
	FIXS_CCH_CmdHandler();

	/**
		@brief	Destructor of FIXS_CCH_CmdHandler class
	*/
	virtual ~FIXS_CCH_CmdHandler();

	/**
	   @brief  		Run by a daemon thread
	*/
	virtual int svc(void);

	/**
	   @brief  		This method initializes a task and prepare it for execution
	*/
	virtual int open (void *args = 0);

	/**
	   @brief  		This method enqueue a command in the queue
	*/
//	int enqueue(ACE_Method_Request* cmdRequest);


 private:

	/**
	   @brief  	svc_run: svc state flag
	*/
	bool tExit;

	ACS_DSD_Server server;
	ACS_DSD_Session session;

	ACS_TRA_trace *_trace;

	ACS_TRA_Logging *FIXS_CCH_logging;

	std::string getOpt(char *buf, int bufSize);

	int splitParams(char *buf, std::list<std::string> &lstParams);


	void fxProcessRequest(ACS_DSD_Session* session, char* request, int requestSize);

	bool sendResponse(ACS_DSD_Session* session, respCode code);

	bool sendMessage(ACS_DSD_Session* ses,respCode code,unsigned int len, const char* data);

	//PFM Fw Upgrade
	void fxProcessFwUpgrade(ACS_DSD_Session* session, const char* request, unsigned int requestSize);
	void fxProcessFwDisplay(ACS_DSD_Session* session, const char* request, unsigned int requestSize);
	void fxProcessFwStatus(ACS_DSD_Session* session, const char* request, unsigned int requestSize);

	//Remote FW Upgrade
	void fxProcessRemoteFwStatus(ACS_DSD_Session* session, const char* request, unsigned int requestSize);
	void fxProcessRemoteFwUpg(ACS_DSD_Session* session, const char* request, unsigned int requestSize);

	//IPMI Upgrade
	void fxProcessIpmiUpg(ACS_DSD_Session* session, const char* request, unsigned int requestSize);
	void fxProcessIpmiStatus(ACS_DSD_Session* session, const char* request, unsigned int requestSize);
	void fxProcessIpmiDisplay(ACS_DSD_Session* session, const char* request, unsigned int requestSize);


	int printList(const char response[], unsigned int respSize);
	/**
	   @brief  	m_Activation_Queue: queue of command to execute
	 */
	//	ACE_Activation_Queue m_Activation_Queue;


};

#endif /* FIXS_CCH_CMDHANDLER_H_ */
