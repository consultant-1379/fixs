/*
 * FIXS_TREFC_TrapSubscriber.h
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */

#ifndef FIXS_TREFC_TRAPSUBSCRIBER_H_
#define FIXS_TREFC_TRAPSUBSCRIBER_H_

#include <ace/Task.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <list>

//#include "Client_Server_Interface.h"

#include "FIXS_TREFC_SNMPTrapReceiver.h"
#include "FIXS_TREFC_Event.h"

using namespace std;

class FIXS_TREFC_TrapSubscriber: public ACE_Task_Base {
 public:
	/**
		@brief	Constructor of FIXS_TREFC_TrapSubscriber class
	*/
	FIXS_TREFC_TrapSubscriber(FIXS_TREFC_SNMPTrapReceiver* trapManager);

	/**
		@brief	Destructor of FIXS_TREFC_TrapSubscriber class
	*/
	virtual ~FIXS_TREFC_TrapSubscriber();

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
	virtual int close ();


 private:

	void printTRA(std::string mesg);
	/**
	   @brief  	svc_run: svc state flag
	*/
	bool tExit;

	FIXS_TREFC_SNMPTrapReceiver* m_trapManager;

	FIXS_TREFC_EventHandle shutdownEvent;

	ACS_TRA_trace* _trace;



};


#endif /* FIXS_TREFC_TrapSubscriber_H_ */
