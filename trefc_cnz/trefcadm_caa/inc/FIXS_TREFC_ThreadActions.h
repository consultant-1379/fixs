/*
 * FIXS_TREFC_ThreadActions.h
 *
 *  Created on: Jul 11, 2012
 *      Author: eanform
 */

#ifndef FIXS_TREFC_ThreadActions_H_
#define FIXS_TREFC_ThreadActions_H_

#include <ace/Task.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <map>

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include "FIXS_TREFC_Manager.h"
#include "FIXS_TREFC_Util.h"



class FIXS_TREFC_ThreadActions: public ACE_Task_Base {

 public:

	enum actionType{
		LOCK 	= 0,
		UNLOCK 	= 1,
		UNDEFINED = -1
	};

	/**
		@brief	Constructor of FIXS_CCH_ThreadActions class
	*/
	FIXS_TREFC_ThreadActions();

	/**
		@brief	Destructor of FIXS_CCH_ThreadActions class
	*/
	virtual ~FIXS_TREFC_ThreadActions();

	/**
	   @brief  		Run by a daemon thread
	*/
	virtual int svc(void);

	/**
	   @brief  		This method initializes a task and prepare it for execution
	*/
	virtual int open (void *args = 0);

	inline bool isRunningThread(){ return running;};

	inline bool isRollbackPhase(){ return rollback;};

	inline void setActionType(actionType action){ m_action = action;};



 private:

	bool tExit;

	bool running;

	bool rollback;

	actionType m_action;

	ACS_TRA_trace *_trace;

};


#endif /* FIXS_TREFC_ThreadActions_H_ */
