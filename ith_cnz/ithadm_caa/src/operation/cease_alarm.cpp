/*
 *
 * COPYRIGHT Ericsson 2016
 *	All rights reserved.
 *
 *	The Copyright to the computer program(s) herein
 *	is the property of Ericsson 2016.
 *	The program(s) may be used and/or copied only with
 *	the written permission from Ericsson 2016 or in
 *	accordance with the terms and conditions stipulated in
 *	the agreement/contract under which the program(s) have
 *	been supplied.
 *
 *
 *  Created on: Sep 8, 2016
 *      Author: xludesi
 */


#include "alarm/alarmhandler.h"

#include "operation/cease_alarm.h"

#include "fixs_ith_workingset.h"

namespace {
	typedef bool (alarms::AlarmHandler::* cease_alarm_func_t) (const std::string & alarm_obj_reference);

	cease_alarm_func_t NULL_CEASE_ALARM_FUNCTION = 0;

	class cease_caller {
		//=======//
		// CTORs //
		//=======//
	public:
		inline cease_caller () : _handler(0), _cease_function(NULL_CEASE_ALARM_FUNCTION) {}

		inline cease_caller (alarms::AlarmHandler & handler, cease_alarm_func_t func)
		: _handler(&handler), _cease_function(func) {}

		inline cease_caller (const cease_caller & rhs) : _handler(rhs._handler), _cease_function(rhs._cease_function) {}

		//======//
		// DTOR //
		//======//
	public:
		inline ~cease_caller () {}

		//===========//
		// Functions //
		//===========//
	public:

		//===========//
		// Operators //
		//===========//
	public:
		inline cease_caller & operator= (const cease_caller & rhs) {
			if (this != &rhs) { _handler = rhs._handler; _cease_function = rhs._cease_function; }
			return *this;
		}

		inline bool operator() (const std::string & alarm_obj_reference) {
			return ((_handler && _cease_function) ? (_handler->*(_cease_function))(alarm_obj_reference) : false);
		}

		//========//
		// Fields //
		//========//
	private:
		alarms::AlarmHandler * _handler;
		cease_alarm_func_t _cease_function;
	};

	cease_caller select_alarm_ceaser (alarms::Alarm::Type alarm_type) {
		switch (alarm_type) {
		case alarms::Alarm::SMX_CONFIGURATION_FAULT:
			return cease_caller(fixs_ith::workingSet_t::instance()->getAlarmHandler(), &alarms::AlarmHandler::ceaseSMXConfigurationFaultAlarm);
			break;
		default: break;
		}

		return cease_caller();
	}
}

namespace operation {
	int cease_alarm::call () {
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO, " ->>> 'CEASE_ALARM(_alarm_type == <%d>, _alarm_obj_reference == <%s>)' operation requested",
			_alarm_type, _alarm_obj_reference.c_str());

		int call_result = 0;

		if (select_alarm_ceaser(_alarm_type)(_alarm_obj_reference)) {
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO, " ->>> 'CEASE_ALARM(_alarm_type == <%d>, _alarm_obj_reference == <%s>)': "
				"Alarm ceased successfully", _alarm_type, _alarm_obj_reference.c_str());
		} else {
			// ERROR: Ceasing the alarm failed
			call_result = -1;

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR, " ->>> 'CEASE_ALARM(_alarm_type == <%d>, _alarm_obj_reference == <%s>)': "
				"Failed to cease the alarm", _alarm_type, _alarm_obj_reference.c_str());
		}

		// set result to caller
		m_operationResult.setErrorCode(call_result);
		setResultToCaller();

		return fixs_ith::ERR_NO_ERRORS;
	}

	void cease_alarm::setOperationDetails (const void * op_details) {
    const op_parameters * params = reinterpret_cast<const op_parameters *>(op_details);
    _alarm_type = params->alarm_type;
    _alarm_obj_reference = params->alarm_obj_reference;
	}
}
