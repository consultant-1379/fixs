/*
 * reload_context_from_imm.h
 *
 *  Created on: Apr 4, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_OPERATION_RELOAD_CONTEXT_FROM_IMM_H_
#define ITHADM_CAA_INC_OPERATION_RELOAD_CONTEXT_FROM_IMM_H_

#include "operation/operationbase.h"

namespace operation {

class reload_context_from_imm: public OperationBase
{
public:
	reload_context_from_imm();
	virtual ~reload_context_from_imm();

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
	std::string m_switchboard_key;
};

} /* namespace operation */

#endif /* ITHADM_CAA_INC_OPERATION_RELOAD_CONTEXT_FROM_IMM_H_ */
