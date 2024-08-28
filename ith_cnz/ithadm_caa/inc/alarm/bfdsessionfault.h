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
 *  Created on: Aug 17, 2016
 *      Author: xludesi
 */

#ifndef BFDSESSIONFAULT_H_
#define BFDSESSIONFAULT_H_

#include "alarm/alarm.h"

namespace alarms
{
	class BfdSessionFault: public Alarm
	{
		public:

			BfdSessionFault(const std::string& objOfReference, const std::string& router, const std::string& neighbor);

			virtual ~BfdSessionFault();
	};

} /* namespace alarm */

#endif /* BFDSESSIONFAULT_H_ */
