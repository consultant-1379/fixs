/*
 * FIXS_CMXH_TrapHandler.h
 *
 */

#ifndef FIXS_CMXH_TRAPHANDLER_H_
#define FIXS_CMXH_TRAPHANDLER_H_

#include <iostream>
#include <ace/Task.h>

#include "ACS_TRAPDS_API.h"
#include "ACS_TRA_Logging.h"

#include "FIXS_CMXH_SNMPTrapReceiver.h"
#include "FIXS_CMXH_Event.h"
#include "FIXS_CMXH_Util.h"
#include "FIXS_CMXH_Logger.h"

class FIXS_CMXH_TrapHandler: public ACE_Task_Base {
 public:
	/**
		@brief	Constructor of FIXS_CMXH_CmdHandler class
	*/
	FIXS_CMXH_TrapHandler();

	/**
		@brief	Destructor of FIXS_CMXH_CmdHandler class
	*/
	virtual ~FIXS_CMXH_TrapHandler();

	/**
	   @brief  		Run by a daemon thread
	*/
	virtual int svc(void);

	/**
	   @brief  		This method initializes a task and prepare it for execution
	*/
	int open (void *args = 0);
	

 private:

	/**
	   @brief  	svc_run: svc state flag
	*/
	bool setSubscribe;

	bool shutdownSubscribe;

	FIXS_CMXH_EventHandle shutdownEvent;

	FIXS_CMXH_SNMPTrapReceiver *trapManager;
	ACS_TRA_Logging *FIXS_CMXH_logging;

};


#endif /* FIXS_CMXH_TRAPHANDLER_H_ */
