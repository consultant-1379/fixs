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
 *  Created on: Jul 6, 2016
 *      Author: xvincon
 */

#ifndef ADD_BFD_SESSION_FOR_NEXTHOP_H_
#define ADD_BFD_SESSION_FOR_NEXTHOP_H_

#include "operationbase.h"

#include <string>

namespace operation
{

	class Add_BFD_Session_For_NextHop: public OperationBase
	{
		public:

			Add_BFD_Session_For_NextHop();

			virtual ~Add_BFD_Session_For_NextHop();

			/**
			 * @brief  Implementation of the operation.
			 */
			virtual int call();

			/**
			 * @brief Set an operation info details.
			 *
			 * @param  op_details Operation details.
			 */
			virtual void setOperationDetails(const void* op_details);

		private:

			bool isNextHopAddressMonitored(const std::string& address, const std::set<std::string>& interfaceSubnets);

			int addToIMM_BFD_Session(const std::string& address);

			std::string m_nextHopDN;

			std::string m_routerName;

			std::string m_nextHopName;
	};

} /* namespace operation */

#endif /* ADD_BFD_SESSION_FOR_NEXTHOP_H_ */
