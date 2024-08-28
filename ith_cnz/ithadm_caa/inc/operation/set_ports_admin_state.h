/*
 * set_ports_admin_state.h
 *
 *  Created on: Nov 22, 2016
 *      Author: xassore
 */

#ifndef ITHADM_CAA_INC_OPERATION_SET_PORTS_ADMIN_STATE_H_
#define ITHADM_CAA_INC_OPERATION_SET_PORTS_ADMIN_STATE_H_

#include "operation/operationbase.h"

namespace operation
{
	/**
	 * @class set_ports_admin_state
	 *
	 * @brief set admin state of bridge ports
	 *
	 * @sa OperationBase
	 */
	class set_ports_admin_state: public OperationBase
	{
	public:

		struct op_parameters {
			char switchboard_key[SWITCHBOARD_KEY_NAME_MAX_SIZE] ;
			imm::admin_State adminStateValue;
		};
		/// Constructor.
		set_ports_admin_state();

		/// Destructor.
		virtual ~set_ports_admin_state() { }

		/**
		 * @brief Set an operation info details.
		 *
		 * @param  op_details Operation details.
		 */
		virtual void setOperationDetails(const void* op_details);

		/**
		 * @brief  Implementation of the operation.
		 */
		virtual int call();

	private:

		std::string m_switchboard_key;
		imm::admin_State m_adminStateValue;
	};




} /* namespace operation */




#endif /* ITHADM_CAA_INC_OPERATION_SET_PORTS_ADMIN_STATE_H_ */
