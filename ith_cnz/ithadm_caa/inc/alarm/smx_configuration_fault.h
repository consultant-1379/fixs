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
 *  Created on: Jul 11, 2016
 *      Author: xnicmut
 */

#ifndef SMS_CONFIGURATION_FAULT_ALARM_H_
#define SMS_CONFIGURATION_FAULT_ALARM_H_

#include "alarm/alarm.h"

namespace alarms {
	class SMXConfigurationFaultAlarm: public Alarm {
		//=======//
		// CTORs //
		//=======//
	public:
		inline explicit SMXConfigurationFaultAlarm(const std::string & objOfReference, const std::string& magazine, const std::string& slot)
		: Alarm(SMX_CONFIGURATION_FAULT, "APZ", objOfReference, "A3", 35202,
						"SWITCH BOARD CONFIGURATION FAULT", "", false)
		{
			char printout_buffer [1024] = {0};
			snprintf(printout_buffer, FIXS_ITH_ARRAY_SIZE(printout_buffer),
					"\n"
					"MAGAZINE   SLOT\n"
					"%-10s %-6s\n", magazine.c_str(), slot.c_str());
			setProblemText(printout_buffer);

			setObjectOfReference(objOfReference);
			setTimestamp();
		}

		//======//
		// DTOR //
		//======//
	public:
		inline virtual ~SMXConfigurationFaultAlarm () {}

		//========//
		// Fields //
		//========//
	private:
	};
} /* alarm namespace */

#endif /* SMS_CONFIGURATION_FAULT_ALARM_H_ */
