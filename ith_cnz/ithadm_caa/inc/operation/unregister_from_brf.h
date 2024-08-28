/*
 * unregister_from_brf.h
 *
 *  Created on: Sep 12, 2016
 *      Author: xassore
 */

#ifndef ITHADM_CAA_INC_OPERATION_UNREGISTER_FROM_BRF_H_
#define ITHADM_CAA_INC_OPERATION_UNREGISTER_FROM_BRF_H_

#include "operation/operationbase.h"

namespace operation {

/**
 * @class register_to_brf
 *
 * @brief register to BRF by
 * - creating an object of the BRF Participant class
 * - register to imm as OI of the new object to receive callbacks for administrative operations.
 *
 */
class unregister_from_brf: public OperationBase
{
public:
	unregister_from_brf();

	virtual ~unregister_from_brf() {}

	/**
	 * @brief  Implementation of the operation.
	 */
	virtual int call();

private:
	int delete_brf_participant();
};

}


#endif /* ITHADM_CAA_INC_OPERATION_UNREGISTER_FROM_BRF_H_ */
