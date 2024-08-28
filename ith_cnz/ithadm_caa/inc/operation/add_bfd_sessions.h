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
 *  Created on: Jun 30, 2016
 *      Author: xvincon
 */

#ifndef ADD_BFD_SESSIONS_H_
#define ADD_BFD_SESSIONS_H_

#include "operation/operationbase.h"

#include <string>

// Class forward
namespace engine
{
	class Context;
}

namespace operation
{

	class Add_BFD_Sessions: public OperationBase
	{
		public:

			Add_BFD_Sessions();

			virtual ~Add_BFD_Sessions();

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

			bool getInterfaceIpAddress(engine::Context* context, std::string& ipAddress, uint32_t& subnetMask);

			int create_BFD_Session(const std::set<std::string>& ipAddresses);

			std::string m_interfaceIPv4DN;

			std::string m_routerName;

	};

} /* namespace operation */

#endif /* ADD_BFD_SESSIONS_H_ */
