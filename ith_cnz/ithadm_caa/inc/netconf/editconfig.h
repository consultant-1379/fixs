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

#ifndef NETCONF_EDITCONFIG_H_
#define NETCONF_EDITCONFIG_H_

#include "netconf/requestbase.h"

namespace netconf
{

	class EditConfig: public RequestBase
	{
		public:

			EditConfig(const std::string& xmlData);

			virtual ~EditConfig() {};
	};

} /* namespace netconf */

#endif /* NETCONF_EDITCONFIG_H_ */
