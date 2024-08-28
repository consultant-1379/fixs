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
#ifndef ITHADM_CAA_INC_FIXS_ITH_NOOP_H_
#define ITHADM_CAA_INC_FIXS_ITH_NOOP_H_

#include "operation/operationbase.h"

namespace operation
{
	/**
	 * @class NoOp
	 *
	 * @brief  Nothing to do request.
	 *
	 * @sa OperationBase
	 */
	class NoOp : public OperationBase
	{
	 public:

		/// Constructor.
		NoOp();

		/// Destructor.
		virtual ~NoOp() { };

		/**
		 * @brief  Implementation of the operation.
		 */
		virtual int call();

	};

} /* namespace operation */

#endif /* ITHADM_CAA_INC_FIXS_ITH_NOOP_H_ */
