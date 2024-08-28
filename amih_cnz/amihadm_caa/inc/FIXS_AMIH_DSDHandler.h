
#ifndef FIXS_AMIH_DSDHANDLER_H_
#define FIXS_AMIH_DSDHANDLER_H_ 45

#include <ace/Task.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <list>

#include "ACS_DSD_Server.h"
#include "ACS_DSD_Session.h"
#include "ACS_TRA_trace.h"


class FIXS_AMIH_DSDHandler: public ACE_Task_Base
{
public:
	/**
		@brief	Constructor of FIXS_CCH_DSDHandler class
	 */
	FIXS_AMIH_DSDHandler();

	/**
		@brief	Destructor of FIXS_CCH_DSDHandler class
	 */
	virtual ~FIXS_AMIH_DSDHandler();

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

	void printTRA(std::string mesg);

private:

	bool tExit;

	ACS_DSD_Server server;
	ACS_DSD_Session session;

	ACS_TRA_trace *_trace;

	std::string getOpt(char *buf, int bufSize);

	int splitParams(char *buf, std::list<std::string> &lstParams);

};

#endif /* FIXS_AMIH_DSDHandler_H_ */
