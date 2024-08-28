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

#ifndef ITHADM_CAA_INC_OPERATION_CEASE_ALARM_H_
#define ITHADM_CAA_INC_OPERATION_CEASE_ALARM_H_

#include <string>

#include "alarm/alarm.h"
#include "operation/operationbase.h"

namespace operation {
	class cease_alarm: public OperationBase {
		//=======//
		// CTORs //
		//=======//
	public:
		inline cease_alarm ()
		: OperationBase(CEASE_ALARM), _alarm_type(alarms::Alarm::UNKNOWN), _alarm_obj_reference() {}

		inline cease_alarm (alarms::Alarm::Type alarm_type, const std::string & alarm_obj_reference = std::string())
		: OperationBase(CEASE_ALARM), _alarm_type(alarm_type), _alarm_obj_reference(alarm_obj_reference) {}

	private:
		cease_alarm (const cease_alarm & rhs);

		//======//
		// DTOR //
		//======//
	public:
		inline virtual ~cease_alarm () {}

		//===========//
		// Functions //
		//===========//
	public:
		/**
		 * @brief  Implementation of the operation.
		 */
		virtual int call ();

		/**
		 * @brief Set an operation info details.
		 *
		 * @param  op_details Operation details.
		 */
		virtual void setOperationDetails (const void * op_details);

		//===========//
		// Operators //
		//===========//
	private:
		cease_alarm & operator= (const cease_alarm & rhs);

		//=====================//
		// Internal Structures //
		//=====================//
	public:
		struct op_parameters {
			alarms::Alarm::Type alarm_type;
			std::string alarm_obj_reference;
		};

		//========//
		// Fields //
		//========//
	private:
		alarms::Alarm::Type _alarm_type;
		std::string _alarm_obj_reference;
	};
} /* namespace operation */

#endif /* ITHADM_CAA_INC_OPERATION_CEASE_ALARM_H_ */
