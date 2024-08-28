/*
 * FIXS_TREFC_CmdHandler.h
 *
 *  Created on: 29 Jun 2011
 *      Author: eanform
 */

#ifndef FIXS_TREFC_CMDHANDLER_H_
#define FIXS_TREFC_CMDHANDLER_H_

#include <ace/Task.h>
//#include <ace/Activation_Queue.h>
//#include <ace/Method_Request.h>
#include "FIXS_TREFC_Event.h"
#include "FIXS_TREFC_Util.h"
#include <iostream>




class FIXS_TREFC_CmdHandler: public ACE_Task_Base {
 public:
	/**
		@brief	Constructor of FIXS_TREFC_CmdHandler class
	*/
	FIXS_TREFC_CmdHandler();

	/**
		@brief	Destructor of FIXS_TREFC_CmdHandler class
	*/
	virtual ~FIXS_TREFC_CmdHandler();

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
	bool svc_run;


	FIXS_TREFC_EventHandle shutdownEvent;



	/**
	   @brief  	m_Activation_Queue: queue of command to execute
	*/
//	ACE_Activation_Queue m_Activation_Queue;


};

#endif /* FIXS_TREFC_CMDHANDLER_H_ */
