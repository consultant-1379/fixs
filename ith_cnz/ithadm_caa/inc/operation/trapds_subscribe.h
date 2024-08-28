//	********************************************************
//
//	 COPYRIGHT Ericsson 2016
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2016.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2016 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************

#ifndef ITHADM_CAA_INC_TRAPDS_SUBSCRIBE_H_
#define ITHADM_CAA_INC_TRAPDS_SUBSCRIBE_H_

#include "operation/operationbase.h"

namespace operation
{
	/**
	 * @class trapds_subscribe
	 *
	 * @brief TRAPDS subscription operation request, to receive SNMP traps.
	 *
	 * @sa OperationBase
	 */
	class trapds_subscribe : public OperationBase
	{
	 public:

		/// Constructor.
		trapds_subscribe();

		/// Destructor.
		virtual ~trapds_subscribe() { }

		/**
		 * @brief  Implementation of the operation.
		 */
		virtual int call();
	};

} /* namespace operation */



#endif /* ITHADM_CAA_INC_TRAPDS_SUBSCRIBE_H_ */
