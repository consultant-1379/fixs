/*
 *
 * COPYRIGHT Ericsson 2016
 *	All rights reserved.
 *
 *	The Copyright to the computer program(s) herein
 *	is the property of Ericsson 2016.
 *	The program(s) may be used and/or copied only with
 *	the written permission from Ericsson 2016 or in
 *	accordance with the terms and conditions stipulated in
 *	the agreement/contract under which the program(s) have
 *	been supplied.
 *
 *
 *  Created on: Sep 8, 2016
 *      Author: xludesi
 */

#ifndef ITHADM_CAA_INC_OPERATION_MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE_H_
#define ITHADM_CAA_INC_OPERATION_MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE_H_

#include <string>

#include "fixs_ith_logger.h"

#include "operation/operationbase.h"

class fixs_ith_switchboardinfo;

namespace operation
{
	class Magazine_switchboards_config_restore: public OperationBase
	{// TR_HZ29732
		public:

			enum op_state
			{
				NOT_STARTED = 0,
				CHECK_SWITCHBOARDS_UP_AND_RUNNING,
				FIRST_SWITCHBOARD_RESET,
				FIRST_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM,
				FIRST_SWITCHBOARD_CONFIG_RESTORE_CREATE_BNC_OBJECTS,
				FIRST_SWITCHBOARD_CONFIG_RESTORE_PIM_ADAPTION,
				FIRST_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS,

				SECOND_SWITCHBOARD_RESET,
				SECOND_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM,
				SECOND_SWITCHBOARD_CONFIG_RESTORE_CREATE_BNC_OBJECTS,
				SECOND_SWITCHBOARD_CONFIG_RESTORE_PIM_ADAPTION,
				SECOND_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS,

				CHANGE_MAGAZINE,
				EXIT,

				OP_STATE_COUNT
			 };

			struct op_parameters {
				fixs_ith::magazine_identifier_t  mag_id;
				fixs_ith::board_slotposition_t   slot;
				Magazine_switchboards_config_restore::op_state op_state;
				int retries_counter;
			};

			Magazine_switchboards_config_restore():
				OperationBase(MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE),
				_mag_id(fixs_ith::UNDEFINED_MAGAZINE),
				_slot(fixs_ith::BOARD_SMX_UNDEFINED),
				_op_state(NOT_STARTED),
				_retries_counter(0)
			{}

			Magazine_switchboards_config_restore(fixs_ith::magazine_identifier_t mag_id,
										   fixs_ith::board_slotposition_t  slot,
										   Magazine_switchboards_config_restore::op_state op_state)
			: OperationBase(MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE),
			  _mag_id(mag_id),
			  _slot(slot),
			  _op_state(op_state),
				_retries_counter(0) {}

			virtual ~Magazine_switchboards_config_restore() {}

			/**
			 * @brief  Implementation of the operation.
			 */
			virtual int call();

			/**
			 * @brief Set an operation info details.
			 *
			 * @param  op_details Operation details.
			 */
			virtual void setOperationDetails (const void * op_details);

		private:
			inline fixs_ith::board_slotposition_t select_slot_random () {
				srand(static_cast<unsigned>(time(0) % 16183));
				return ((rand() % 2) ? fixs_ith::BOARD_SMX_RIGHT : fixs_ith::BOARD_SMX_LEFT);
			}

                        inline fixs_ith::board_slotposition_t select_cmx_slot_random () {
                                srand(static_cast<unsigned>(time(0) % 16183));
                                return ((rand() % 2) ? fixs_ith::BOARD_CMX_RIGHT : fixs_ith::BOARD_CMX_LEFT);
                        }


			inline const char * op_state_str () const {
				return (((NOT_STARTED <= _op_state) && (_op_state < OP_STATE_COUNT)) ? op_state_names[_op_state] : "<UNDEFINED>");
			}

			int load_switchboards_info (
				fixs_ith_switchboardinfo & switchboardInfo_left,
				std::string & switch_board_mag_str_left,
				fixs_ith_switchboardinfo & switchboardInfo_right,
				std::string & switch_board_mag_str_right);

			unsigned map_switch_board_index (fixs_ith::magazine_identifier_t mag_id, int32_t slot);

			fixs_ith::magazine_identifier_t  _mag_id;	// indicates the magazine: APZ or APT
			fixs_ith::board_slotposition_t   _slot; 	// indicates the position in the magazine of the switchboard to restore: LEFT or RIGTH
			Magazine_switchboards_config_restore::op_state _op_state;	// indicates the state of the operation
			int _retries_counter;

			static const char * op_state_names [OP_STATE_COUNT];
	};

} /* namespace operation */

#endif /* ITHADM_CAA_INC_OPERATION_MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE_H_ */
