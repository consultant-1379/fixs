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

#include <stdint.h>

#include "operation/creator.h"
#include "operation/timer_handler.h"

#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"

#include "fixs_ith_workingset.h"

namespace operation
{

ACE_Atomic_Op<ACE_Recursive_Thread_Mutex, int> timer_handler::_next_available_id = 0;

timer_handler::timer_handler(boost::shared_ptr<operation::Scheduler> scheduler, operation::identifier_t op_id, const void * op_details, size_t op_details_size, unsigned int delay, unsigned int interval)
:_scheduler(scheduler), _op_id(op_id),_op_data(0),_op_data_size(0), _op_result(), _timer_id(-1), _timer_delay(delay), _timer_interval(interval), _enqueue_op_on_timer_tick(delay > 0)
{
	_obj_id = _next_available_id ++;

	if(op_details)
	{
		_op_data = new uint8_t[_op_data_size = op_details_size];
		if(!_op_data)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NO memory available to allocate operation parameters. Requested memory: [%lu ] bytes", op_details_size);
			_op_id = operation::INVALID_OP;
			_op_data_size = 0;
		}
		else
		{
			memcpy(_op_data, op_details, _op_data_size);
		}
	}
}


int timer_handler::handle_timeout (const ACE_Time_Value & /*current_time*/, const void * )
{
	// 1. If not already done, enqueue the OP using the ITH Scheduler object
	if(_enqueue_op_on_timer_tick)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : scheduling OP '%d'- timer_id == %ld ... ", _obj_id, _op_id, _timer_id);
		operation::Creator operationFactoryCreator(_scheduler);
		int schedule_res = operationFactoryCreator.schedule(_op_id, & _op_result, _op_data);
		if(schedule_res != fixs_ith::ERR_NO_ERRORS)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, " *** TMR_HDLR ## %03d : unable to schedule OP '%d'. error_code == %d", _obj_id, _op_id, schedule_res);
			_timer_id = -1;
			return -1;
		}
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : OP '%d' successfully scheduled - timer_id == %ld ... ", _obj_id, _op_id, _timer_id);
	}

	// 2.  Wait for OP result
	operation::result op_res;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : waiting for OP '%d' result ", _obj_id, _op_id);
	// TODO: here we could use an ACE_Time_value to wait for result with a timeout
	int get_call_res = _op_result.get(op_res);
	if(get_call_res)
	{	// an error occurred while getting operation result
		int errno_save = errno;
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, " *** TMR_HDLR ## %03d : Unable to get OP '%d' result. Call 'ACE_Future<operation::result>::get()' returned '%d' [ERRNO == %d].",
				_obj_id, _op_id, get_call_res, errno_save);
	}
	else {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : OP '%d' result obtained. ErrorCode = %d", _obj_id, _op_id, op_res.errorCode);
	}

	// 3. Check OP result if available, and reschedule on "NOT OK" error code
	if(get_call_res || (op_res.errorCode == fixs_ith::ERR_NO_ERRORS))
	{
		// Unable to get operation result, or operation was successfully completed with "no errors".
		// In either cases we do NOT reschedule the operation. Mark the timer event handler as no more valid.
		_timer_id = -1;
	}
	else
	{	// operation result is "not ok"; we must enqueue the operation again, after the specified timer interval
		_enqueue_op_on_timer_tick = true;

		// reset the ACE_Future object '_op_result' in order to reuse it
		_op_result.cancel();

		// reschedule the timer on the main reactor
		ACE_Time_Value tv(_timer_interval, 0);
		_timer_id = this->reactor()->schedule_timer(this,0,tv);
		if(_timer_id == -1)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, " *** TMR_HDLR ## %03d : unable to reschedule the operation '%d' with delay '%u' ('schedule_timer' failure) !", _obj_id, _op_id, _timer_interval);
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : operation '%d' successfully rescheduled. timer_id == %ld, timer_interval == %u", _obj_id, _op_id, _timer_id, _timer_interval);
		}
	}

	return (_timer_id == -1) ? -1: 0; // request for handle_close() only if timer event handler object is no more valid
}

int timer_handler::handle_close (ACE_HANDLE /*handle*/, ACE_Reactor_Mask /*close_mask*/)
{
//	if(_timer_id == -1)
//	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : Releasing memory allocated for OP %d ", _obj_id, _op_id);
		delete this;	// destroy the object, which was allocated using 'operation_timer_handler::create_instance()'
//	}

	return 0;
}

}
