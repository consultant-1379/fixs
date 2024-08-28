/*
 * FIXS_CCH_TrapHandler.h
 *
 *  Created on: Mar 8, 2012
 *      Author: eanform
 */

#ifndef FIXS_CCH_TRAPHANDLER_H_
#define FIXS_CCH_TRAPHANDLER_H_

#include <iostream>
#include <ace/Task.h>

#include "ACS_TRAPDS_API.h"
#include "ACS_TRA_Logging.h"

#include "FIXS_CCH_SNMPTrapReceiver.h"
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_Util.h"

class FIXS_CCH_TrapHandler: public ACE_Task_Base {
 public:
	/**
		@brief	Constructor of FIXS_CCH_CmdHandler class
	*/
	FIXS_CCH_TrapHandler();

	/**
		@brief	Destructor of FIXS_CCH_CmdHandler class
	*/
	virtual ~FIXS_CCH_TrapHandler();

	/**
	   @brief  		Run by a daemon thread
	*/
	virtual int svc(void);

	/**
	   @brief  		This method initializes a task and prepare it for execution
	*/
	int open (void *args = 0);

	inline bool getSvcState() {return svc_run;}


 private:

	/**
	   @brief  	svc_run: svc state flag
	*/
	bool setSubscribe;

	bool shutdownSubscribe;

	bool svc_run;

	FIXS_CCH_EventHandle shutdownEvent;

	FIXS_CCH_SNMPTrapReceiver *trapManager;

	ACS_TRA_Logging *FIXS_CCH_logging;

};


#endif /* FIXS_CCH_TRAPHANDLER_H_ */
