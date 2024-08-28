//	********************************************************
//
//	 COPYRIGHT Ericsson 2016
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2016.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2016 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************

#ifndef ITHADM_CAA_INC_FIXS_ITH_HA_TASK_H_
#define ITHADM_CAA_INC_FIXS_ITH_HA_TASK_H_

#include <ace/ACE.h>
#include <ace/Reactor.h>
#include <ace/TP_Reactor.h>
#include <ace/Task.h>

#include "fixs_ith_ha_eventhandler.h"
#include "fixs_ith_server.h"

class fixs_ith_ha_task: public ACE_Task_Base
{
public:
	fixs_ith_ha_task(fixs_ith_server * fixs_ith_server, ACE_HANDLE ha_pipe_handle);

	virtual ~fixs_ith_ha_task();

	int start();

	int stop();

	virtual int svc();

	/// Indicates if the task is active
	inline bool is_active() { return this->thr_count_ > 0; }

private:
	ACE_TP_Reactor _ha_reactor_impl;
	ACE_Reactor _ha_reactor;
	fixs_ith_ha_eventhandler _ha_evt_handler;
	fixs_ith_server * _fixs_ith_server;
};



#endif /* ITHADM_CAA_INC_FIXS_ITH_HA_TASK_H_ */
