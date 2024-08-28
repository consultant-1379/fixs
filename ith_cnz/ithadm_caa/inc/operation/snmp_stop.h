//	********************************************************
//
//	 COPYRIGHT Ericsson 2016
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2016.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2015 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************


#ifndef ITHADM_CAA_INC_OPERATION_SNMP_STOP_H_
#define ITHADM_CAA_INC_OPERATION_SNMP_STOP_H_

#include "operation/operationbase.h"

/**
 * @class snmp_trap_unsubscribe
 *
 * @brief SNMP TRAP Unsubscribe operation request, to receive SNMP traps.
 *
 * @sa OperationBase
**/

namespace operation
{

	class snmp_trap_unsubscribe : public OperationBase
	{
 		public:

		/// Constructor.
		snmp_trap_unsubscribe();

		/// Destructor.
		virtual ~snmp_trap_unsubscribe() { }

		/**
		 * @brief  Implementation of the operation.
		 */
		virtual int call();
};

}

#endif /* ITHADM_CAA_INC_OPERATION_SNMP_STOP_H_ */
