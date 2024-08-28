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
 *  Created on: Jul 7, 2016
 *      Author: xvincon
 */

#ifndef REMOVE_BFD_SESSION_FOR_NEXTHOP_H_
#define REMOVE_BFD_SESSION_FOR_NEXTHOP_H_

#include "operation/operationbase.h"

#include <string>

namespace operation
{

	class Remove_BFD_Session_For_NextHop: public OperationBase
	{
		public:

			Remove_BFD_Session_For_NextHop();

			virtual ~Remove_BFD_Session_For_NextHop();

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

			void searchBfdSessionsToRemove(const std::set<std::string>& bfdSessions, const std::set<std::string>& nextHopsAddress, std::set<std::string>& bfdSessionToRemove);

			int removeFromIMM_BFD_Sessions(const std::set<std::string>& ipAddresses);

			std::string m_routerDN;

			std::string m_routerName;
	};

} /* namespace operation */

#endif /* REMOVE_BFD_SESSION_FOR_NEXTHOP_H_ */
