/*
 * register_imm.h
 *
 *  Created on: Apr 4, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_OPERATION_REGISTER_IMM_H_
#define ITHADM_CAA_INC_OPERATION_REGISTER_IMM_H_

#include "operation/operationbase.h"

namespace operation {

/**
 * @class register_imm
 *
 * @brief IMM implementer registration, to receive IMM callbacks..
 *
 * @sa OperationBase
 */
class register_imm: public OperationBase
{
public:
	register_imm();

	virtual ~register_imm() {}

	/**
	 * @brief  Implementation of the operation.
	 */
	virtual int call();
};

} /* namespace operation */

#endif /* ITHADM_CAA_INC_OPERATION_REGISTER_IMM_H_ */
