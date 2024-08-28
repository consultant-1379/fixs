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
 *      Author: xvincon
 */

#ifndef PIMFAULT_H_
#define PIMFAULT_H_

#include "alarm/alarm.h"

namespace alarms {
	class PimFault: public Alarm {
		//=======//
		// CTORs //
		//=======//
	public:
		inline explicit PimFault(
				const std::string& objOfReference,
				const std::string& magazine,
				const std::string& slot,
				const std::string& port_name)
		: Alarm(PIM_FAULT, "APZ", objOfReference, "O2", 35204,
						"SWITCH PLUGIN MODULE NOT SUPPORTED", "", false)
		{
			setObjectOfReference(objOfReference);
			char printout_buffer [1024] = {0};
			snprintf(printout_buffer, FIXS_ITH_ARRAY_SIZE(printout_buffer),
					"\n"
					"MAGAZINE   SLOT   PORT\n"
					"%-10s %-6s %-4s\n", magazine.c_str(), slot.c_str(), port_name.c_str());
			setProblemText(printout_buffer);
			setTimestamp();
		}

		//======//
		// DTOR //
		//======//
	public:
		inline virtual ~PimFault () {}

		//========//
		// Fields //
		//========//
	private:
	};
} /* namespace alarm */

#endif /* PIMFAULT_H_ */
