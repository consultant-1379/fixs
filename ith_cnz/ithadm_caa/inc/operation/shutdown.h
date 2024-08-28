//	********************************************************
//
//	 COPYRIGHT Ericsson 2015
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2015.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2015 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************
#ifndef ITHADM_CAA_INC_FIXS_ITH_SHUTDOWN_H_
#define ITHADM_CAA_INC_FIXS_ITH_SHUTDOWN_H_

#include "operation/operationbase.h"
#include "fixs_ith_server.h"

namespace operation
{
	/**
	 * @class Shutdown
	 *
	 * @brief  Terminate operation request.
	 *
	 * @sa OperationBase
	 */
	class Shutdown : public OperationBase
	{
	 public:

		/// Constructor.
		Shutdown();

		/// Destructor.
		virtual ~Shutdown() {}

		/**
		 * @brief  Implementation of the operation.
		 */
		virtual int call();
	};

} /* namespace operation */

#endif /* ITHADM_CAA_INC_FIXS_ITH_SHUTDOWN_H_ */
