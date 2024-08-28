/*
 * operationreload_context_after_cold_start_trap.h
 *
 *  Created on: Apr 8, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_OPERATIONRELOAD_CONTEXT_AFTER_COLD_START_TRAP_H_
#define ITHADM_CAA_INC_OPERATIONRELOAD_CONTEXT_AFTER_COLD_START_TRAP_H_

#include "operation/operationbase.h"

namespace operation
{

class reload_context_after_cold_start_trap: public OperationBase
{
public:
	reload_context_after_cold_start_trap();
	virtual ~reload_context_after_cold_start_trap();

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

#endif /* ITHADM_CAA_INC_OPERATIONRELOAD_CONTEXT_AFTER_COLD_START_TRAP_H_ */
