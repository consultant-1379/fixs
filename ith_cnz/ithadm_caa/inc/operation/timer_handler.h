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

#ifndef ITHADM_CAA_INC_OPERATION_TIMER_HANDLER_H_
#define ITHADM_CAA_INC_OPERATION_TIMER_HANDLER_H_

#include <stdint.h>

#include <ace/Event_Handler.h>
#include <ace/Atomic_Op.h>
#include <ace/Recursive_Thread_Mutex.h>
#include "operation/operationbase.h"

#include "fixs_ith_workingset.h"

namespace operation
{

class timer_handler: public ACE_Event_Handler
{
	//friend int fixs_ith::workingset::schedule_until_completion(operation::Scheduler* , operation::identifier_t id, unsigned int delay, unsigned int interval, const void * op_details,  size_t op_details_size);

	friend class fixs_ith::workingset;

public:

	inline virtual ~timer_handler() { delete [] (_op_data); }

	inline bool valid() { return _op_id != operation::INVALID_OP; }

	static timer_handler * create_instance(boost::shared_ptr<operation::Scheduler> scheduler, operation::identifier_t op_id, const void * op_details, size_t op_details_size, unsigned int delay, unsigned int interval)
		{ return new (std::nothrow) timer_handler(scheduler, op_id, op_details, op_details_size, delay, interval); }

	virtual int handle_timeout (const ACE_Time_Value &current_time, const void * = 0);

	virtual int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask);

	inline void set_timer_id(long timer_id) { _timer_id = timer_id; }

	inline void set_timer_delay(unsigned int delay) { _timer_delay = delay;}

	inline void set_timer_interval(unsigned int interval) { _timer_interval = interval;}

private:

	timer_handler(boost::shared_ptr<operation::Scheduler> scheduler, operation::identifier_t op_id, const void * op_details, size_t op_details_size, unsigned int delay, unsigned int interval);

	timer_handler(const timer_handler &);

	boost::shared_ptr<operation::Scheduler> _scheduler;

	operation::identifier_t _op_id;
	uint8_t * _op_data;
	size_t _op_data_size;
	ACE_Future<operation::result> _op_result;

	long _timer_id;
	unsigned int _timer_delay;
	unsigned int _timer_interval;

	bool _enqueue_op_on_timer_tick;

	int _obj_id;


	static ACE_Atomic_Op<ACE_Recursive_Thread_Mutex, int> _next_available_id;

};

}

#endif /* ITHADM_CAA_INC_OPERATION_TIMER_HANDLER_H_ */
