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
 *  Created on: Jun 1, 2016
 *      Author: xvincon
 */

#ifndef NETCONF_GETCONFIG_H_
#define NETCONF_GETCONFIG_H_

#include "netconf/requestbase.h"

namespace netconf
{

	class GetConfig: public RequestBase
	{
		public:

			GetConfig(acs_nclib::Operation getType, const std::string& xmlFilter);

			virtual ~GetConfig() {};
	};

} /* namespace netconf */

#endif /* NETCONF_GETCONFIG_H_ */
