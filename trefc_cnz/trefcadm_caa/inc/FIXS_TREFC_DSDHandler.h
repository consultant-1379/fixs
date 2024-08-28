/*
 * FIXS_TREFC_DSDHandler.h
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */

#ifndef FIXS_TREFC_DSDHANDLER_H_
#define FIXS_TREFC_DSDHANDLER_H_

#include <ace/Task.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <list>

//#include "Client_Server_Interface.h"

#include "ACS_DSD_Server.h"
#include "ACS_DSD_Session.h"
#include "ACS_TRA_trace.h"

using namespace std;

class FIXS_TREFC_DSDHandler: public ACE_Task_Base {
 public:
	/**
		@brief	Constructor of FIXS_TREFC_DSDHandler class
	*/
	FIXS_TREFC_DSDHandler();

	/**
		@brief	Destructor of FIXS_TREFC_DSDHandler class
	*/
	virtual ~FIXS_TREFC_DSDHandler();

	/**
	   @brief  		Run by a daemon thread
	*/
	virtual int svc(void);

	/**
	   @brief  		This method initializes a task and prepare it for execution
	*/
	virtual int open (void *args = 0);

	/**
	   @brief  		This method will stop the execution
	*/
	virtual int close (u_long flags = 0);


 private:

	/**
	   @brief  	svc_run: svc state flag
	*/
	bool tExit;

	/**
	   @brief  	DSD objects
	*/
	ACS_DSD_Server server;
	ACS_DSD_Session session;

	ACS_TRA_trace* _trace;

	std::string getOpt(char *buf, int bufSize);

	int splitParams(char *buf, std::list<std::string> &lstParams);

};


#endif /* FIXS_TREFC_DSDHANDLER_H_ */
