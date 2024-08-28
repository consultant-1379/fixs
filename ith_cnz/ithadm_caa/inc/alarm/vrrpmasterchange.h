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

#ifndef VRRPMASTERCHANGE_H_
#define VRRPMASTERCHANGE_H_

#include "alarm/alarm.h"

namespace alarms
{
	namespace vrrp_alarms
	{
		const int vrrpMasterChangeAlarmDelay = 20*60; //seconds
	}

	class VrrpMasterChange: public Alarm
	{
		public:

			VrrpMasterChange(const std::string& objOfReference);

			virtual ~VrrpMasterChange();
	};

} /* namespace alarm */

#endif /* VRRPMASTERCHANGE_H_ */
