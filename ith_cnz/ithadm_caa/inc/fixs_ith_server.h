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

#ifndef ITHADM_CAA_INC_FIXS_ITH_SERVER_H_
#define ITHADM_CAA_INC_FIXS_ITH_SERVER_H_

#include <ace/Task.h>
#include <ace/Signal.h>

#include "fixs_ith_cmdoptionparser.h"
#include "fixs_ith_procsignalseventhandler.h"

class fixs_ith_haappmanager; // To avoid problems (??????) on compilation

class fixs_ith_server : public ACE_Task_Base
{
	//==============//
	// Constructors //
	//==============//
public:
	inline explicit fixs_ith_server (fixs_ith_cmdoptionparser & cmdoption_parser)
	: ACE_Task_Base(),
	 _cmdoption_parser(cmdoption_parser),
	 _proc_signals_event_handler(this),
	 _proc_catched_signals(), _ha_app_manager_ptr(0)
	{}

private:
	fixs_ith_server (const fixs_ith_server & rhs);

	//============//
	// Destructor //
	//============//
public:
	inline virtual ~fixs_ith_server () {}

	//===========//
	// Functions //
	//===========//
public:
	// Start ITH Server work
	int work ();

	// Start ITH Server worker threads (invoked when the ITH Server component receives ACTIVE assignment)
	int start_jobs();

	// Stop ITH Server worker threads
	int stop_jobs(bool wait_termination = true);

	// Indicates if ITH Server worker threads are running
	inline bool is_active() { return this->thr_count()> 0; };

	inline bool is_running_in_noha_mode() { return _cmdoption_parser.noha(); }

	// Wait for termination of the ITH Server worker threads, and release the related resources
	int wait_and_release_jobs_resources();

	// BEGIN: ACE_Task_Base interface //
	virtual int svc ();
	// END: ACE_Task_Base interface //

private:
	int multiple_process_instance_running_check ();

	void enter_reactor_event_loop();

	// install/remove the ITS Server signals handler
	int init_signals_handler();
	int reset_signals_handler();

	int execute_tests ();

	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_server & operator= (const fixs_ith_server & rhs);

	//========//
	// Fields //
	//========//
private:
	// ITH Server command line parser
	fixs_ith_cmdoptionparser & _cmdoption_parser;

	// ITH Server signals handler
	fixs_ith_procsignalseventhandler _proc_signals_event_handler;
	ACE_Sig_Set _proc_catched_signals;

	fixs_ith_haappmanager * _ha_app_manager_ptr;

	static int _proc_signals_to_catch [];
};


#endif /* ITHADM_CAA_INC_FIXS_ITH_SERVER_H_ */

