/*
 * reload_all_from_imm.h
 *
 *  Created on: Apr 4, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_OPERATION_RELOAD_ALL_FROM_IMM_H_
#define ITHADM_CAA_INC_OPERATION_RELOAD_ALL_FROM_IMM_H_

#include "operation/operationbase.h"

namespace operation {

/**
 * @class reload_all_from_imm
 *
 * @brief Reload all content from IMM.
 *
 * @sa OperationBase
 */

class reload_all_from_imm: public OperationBase
{
public:
	reload_all_from_imm();
	virtual ~reload_all_from_imm();

	/**
	 * @brief  Implementation of the operation.
	 */
	virtual int call();

private:

};

} /* namespace operation */

#endif /* ITHADM_CAA_INC_OPERATION_RELOAD_ALL_FROM_IMM_H_ */
