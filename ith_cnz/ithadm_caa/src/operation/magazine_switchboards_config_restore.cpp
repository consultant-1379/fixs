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

#include <stdlib.h>
#include <time.h>

#include "switch_interface/full_adapter.h"
#include "fixs_ith_brf_policy_handler.h"
#include "operation/magazine_switchboards_config_restore.h"
#include "fixs_ith_workingset.h"
#include "common/utility.h"


//As part of TR HZ29732 fix, delay has been introduced between two SMXB resets
// To make delay working, PIM adaption procedure is moved to burrestore procedure
// Otherwise delay would not work

namespace operation {
const char * Magazine_switchboards_config_restore::op_state_names[OP_STATE_COUNT] =
		{ "NOT_STARTED", "CHECK_SWITCHBOARDS_UP_AND_RUNNING",
				"FIRST_SWITCHBOARD_RESET",
				"FIRST_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM",
				"FIRST_SWITCHBOARD_CONFIG_RESTORE_CREATE_BNC_OBJECTS",
				"FIRST_SWITCHBOARD_CONFIG_RESTORE_PIM_ADAPTION",
				"FIRST_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS",
				"SECOND_SWITCHBOARD_RESET",
				"SECOND_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM",
				"SECOND_SWITCHBOARD_CONFIG_RESTORE_CREATE_BNC_OBJECTS",
				"SECOND_SWITCHBOARD_CONFIG_RESTORE_PIM_ADAPTION",
				"SECOND_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS",
				"CHANGE_MAGAZINE", "EXIT" };

int Magazine_switchboards_config_restore::call() {
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
			" ->>> 'MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
					"_slot == <%d>, _op_state == <%s>)' operation requested",
			_mag_id, _slot, op_state_str());

	fixs_ith_switchboardinfo switchboardInfo_left;
	std::string switch_board_mag_str_left;
	fixs_ith_switchboardinfo switchboardInfo_right;
	std::string switch_board_mag_str_right;

	//GEP2_GEP5 to GEP7 HWSWAP
	bool isSMX = common::utility::isSwitchBoardSMX();
	bool isCMX = common::utility::isSwitchBoardCMX();

	fixs_ith_switchInterfaceAdapter si_adapter;
	int call_result = 0;
	bool looping = true;
	uint8_t shelf_manager_control_state = 0; //TR_HY52528
	
	switch_interface::op_result si_call_result = switch_interface::SBIA_OK;

	bool re_schedule_with_other_params = false;
	uint32_t re_schedule_delay = 0;

	std::string sb_left_key;
	std::string sb_right_key;

	// LOAD Switch boards info for the LEFT and RIGHT boards
	call_result = load_switchboards_info(switchboardInfo_left,
			switch_board_mag_str_left, switchboardInfo_right,
			switch_board_mag_str_right);
	if (call_result) {
		// Operation will be considered failed
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
				" ->>> 'MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
						"_slot == <%d>, _op_state == <%s>)' operation requested: call to 'load_switchboards_info()' failed: call_result == <%d>",
				_mag_id, _slot, op_state_str(), call_result);
		looping = false;
	} else {
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
				"call to 'load_switchboards_info()' successful");
		fixs_ith::workingSet_t::instance()->get_sbdatamanager().build_sb_key_from_magazine_and_slot(
				sb_left_key, switchboardInfo_left.magazine,
				switchboardInfo_left.slot_position);

		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
				"Restore::CMX Left Board Information");
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "sb_left_key:%s",
				sb_left_key.c_str());
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "sb_left.magazine:%d",
				switchboardInfo_left.magazine);
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "slot_position:%d",
				switchboardInfo_left.slot_position);

		fixs_ith::workingSet_t::instance()->get_sbdatamanager().build_sb_key_from_magazine_and_slot(
				sb_right_key, switchboardInfo_right.magazine,
				switchboardInfo_right.slot_position);
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
				"Restore::CMX Right Board Information");
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "sb_right_key:%s",
				sb_right_key.c_str());
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "sb_right.magazine:%d",
				switchboardInfo_right.magazine);
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "slot_position:%d",
				switchboardInfo_right.slot_position);

	}

	bool next_state = true;

	while (looping) {
		switch (_op_state) {
		case NOT_STARTED:
			// Load common context for empty key
			call_result =
					fixs_ith::workingSet_t::instance()->getConfigurationManager().loadConfiguration(
							std::string());
			if (call_result != fixs_ith::ERR_NO_ERRORS) {
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): load of common objects failed: call_result = <%d>",
						_mag_id, _slot, op_state_str(), call_result);
				looping = false;
				break;
			} else {
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
						"Restore::loadConfiguration() is successful");
			}

			if (isSMX) {
				fixs_ith::workingSet_t::instance()->get_snmpmanager().getBoardState(switchboardInfo_left, shelf_manager_control_state); //TR_HY52528

				if (_slot == fixs_ith::BOARD_SMX_UNDEFINED)
				{
					if(shelf_manager_control_state == 3)
					{
						//active is left smx, so set slot to right smx
						_slot = fixs_ith::BOARD_SMX_RIGHT;
						FIXS_ITH_LOG(LOG_LEVEL_WARN, "Left SMX Board is Master");
					}
					else
					{
						//active is right smx, so set slot to left smx
						_slot = fixs_ith::BOARD_SMX_LEFT;
						FIXS_ITH_LOG(LOG_LEVEL_WARN, "Right SMX Board is Master");
					}
				}

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN, " ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
						"_slot == <%d>, _op_state == <%s>): _slot selected == <%d>",
						_mag_id, _slot, op_state_str(), _slot);
			}
			if (isCMX) {
				if (_slot == fixs_ith::BOARD_CMX_UNDEFINED)
					_slot = select_cmx_slot_random();
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): _slot selected == <%d>",
						_mag_id, _slot, op_state_str(), _slot);
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
						"Restore::Selected slot randomly and selected slot is:%d",
						_slot);

			}

			_op_state = CHECK_SWITCHBOARDS_UP_AND_RUNNING;
			break;

		case CHECK_SWITCHBOARDS_UP_AND_RUNNING:
			/* WARNING: Here it must be checked if a restore operation is ongoing for a switch board to allow cases
			 * where a ITH lock and/or unlock service was performed while a restore operation had not been yet completed.
			 * This remark remains valid all times a call to board_restore_ongoing() function is done in the following sources
			 */
			next_state = true;

			if (fixs_ith_brf_policy_handler::board_restore_ongoing(
					map_switch_board_index(_mag_id,
							switchboardInfo_left.slot_position))) {
				si_call_result = si_adapter.get_board_status(
						switchboardInfo_left);
				if (si_call_result != switch_interface::SBIA_OK) {
					// ERROR: Left switch board is not up and running
					looping = false;
					re_schedule_with_other_params = true;
					call_result = -1;

					FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): unable to get left board status: left switch board is not up and running: "
									"raising CONFIGURATION FAULT alarm",
							_mag_id, _slot, op_state_str());

					_retries_counter = 0;

					next_state = false;
				} else {
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
							"Restore::Left switch board is up and running");
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): Left switch board is up and running: "
									"ceasing the CONFIGURATION FAULT alarm",
							_mag_id, _slot, op_state_str());
				}
			} else {
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): This switch board was already restored: nothing to do",
						_mag_id, switchboardInfo_left.slot_position,
						op_state_str());
			}

			if (fixs_ith_brf_policy_handler::board_restore_ongoing(
					map_switch_board_index(_mag_id,
							switchboardInfo_right.slot_position))) {
				si_call_result = si_adapter.get_board_status(
						switchboardInfo_right);
				if (si_call_result != switch_interface::SBIA_OK) {
					// ERROR: Right switch board is not up and running
					looping = false;
					re_schedule_with_other_params = true;
					call_result = -1;

					FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): unable to get right board status: right switch board is not up and running: "
									"raising the CONFIGURATION FAULT alarm",
							_mag_id, _slot, op_state_str());

					_retries_counter = 0;

					next_state = false;
				} else {
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
							"Restore::Right switch board is up and running");
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): Right switch board is up and running: "
									"ceasing the CONFIGURATION FAULT alarm",
							_mag_id, _slot, op_state_str());
				}
			} else {
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): This switch board was already restored: nothing to do",
						_mag_id, switchboardInfo_right.slot_position,
						op_state_str());
			}

			if (next_state) {
				// OK: Cease all alrams
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						sb_left_key);
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						sb_right_key);
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
						"Restore::Ceasing the alarms and set the op_state to FIRST_SWITCHBOARD_RESET");

				_op_state = FIRST_SWITCHBOARD_RESET;
			} else {
				// ERROR: At least one smx board is not reachable, so configuration fatul alarm should be raised for all the smx boards
				fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
						sb_left_key, switch_board_mag_str_left,
						switchboardInfo_left.slot_position);

				fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
						sb_right_key, switch_board_mag_str_right,
						switchboardInfo_right.slot_position);
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
						"Restore:: NO swith board is working and hence alarms are raised");
			}

			break;

		case FIRST_SWITCHBOARD_RESET:
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
					"Restore::Entered FIRST_SWITCHBOARD_RESET");
			if (fixs_ith_brf_policy_handler::board_restore_ongoing(
					map_switch_board_index(_mag_id, _slot))) {

				// Disable traps handling
				if (isSMX) {
					fixs_ith::workingSet_t::instance()->get_trapdsobserver().disable_traps(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key, trap_mask::ALL);
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
							"Restore::Disabled the traps for SMX board");
				}
				if (isCMX) {
					fixs_ith::workingSet_t::instance()->get_trapdsobserver().disable_traps(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key, trap_mask::ALL);
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
							"Restore::Disabled the traps for CMX board");
				}

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): all trap disabled",
						_mag_id, _slot, op_state_str());

				// Reset the first board
				if (isSMX) {
					si_call_result = si_adapter.reset_board((_slot == fixs_ith::BOARD_SMX_LEFT) ? sb_right_key : sb_left_key, _slot); ////TR_HY52528
				}
				if (isCMX) {
					si_call_result = si_adapter.reset_board(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key);
				}

				//si_call_result = si_adapter.reset_board((_slot == fixs_ith::BOARD_SMX_LEFT) ? sb_left_key : sb_right_key);
				if (si_call_result == switch_interface::SBIA_OK) {

					// we successfully reset the FIRST switchboard. Wait that switchboard become again UP an RUNNING
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
							"Restore::Successfully reset LEFT board reset");
					_op_state = FIRST_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM;
					re_schedule_delay = 20; //TODO: define a constant
					call_result = 0;

					// Cease the configuration fault alarm
					fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key);

					FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): reset board command successfully ordered: CONFIGURATION FAULT alarm ceased",
							_mag_id, _slot, op_state_str());
				} else { // ERROR
					re_schedule_delay = 0; //TODO: define a constant
					call_result = -1;

					// Raise the configuration fault alarm
					if (isSMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key,
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										switch_board_mag_str_left :
										switch_board_mag_str_right, _slot);
					}
					if (isCMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key,
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										switch_board_mag_str_left :
										switch_board_mag_str_right, _slot);
					}

					/*fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
					 (_slot == fixs_ith::BOARD_SMX_LEFT) ? sb_left_key : sb_right_key,
					 (_slot == fixs_ith::BOARD_SMX_LEFT) ? switch_board_mag_str_left : switch_board_mag_str_right, _slot);*/

					FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): unable to order board reset command: si_call_result == <%d>: "
									"CONFIGURATION FAULT alarm raised", _mag_id,
							_slot, op_state_str(), si_call_result);
				}

				re_schedule_with_other_params = true;
				looping = false;
			} else {
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): This switch board was already restored: nothing to do",
						_mag_id, _slot, op_state_str());

				_op_state = SECOND_SWITCHBOARD_RESET;

				// Change the slot
				if (isSMX) {
					_slot = (
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									fixs_ith::BOARD_SMX_RIGHT :
									fixs_ith::BOARD_SMX_LEFT);
				}
				if (isCMX) {
					_slot = (
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									fixs_ith::BOARD_CMX_RIGHT :
									fixs_ith::BOARD_CMX_LEFT);
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
							"Restore::Changed the slot to: %d", _slot);
				}

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): _slot selected == <%d>",
						_mag_id, _slot, op_state_str(), _slot);
			}

			_retries_counter = 0;

			break;

		case FIRST_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM:
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
					"Restore::Entered FIRST_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM");
			// Check the first board is up and running
			if (isSMX) {
				si_call_result = si_adapter.get_board_status(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								switchboardInfo_left : switchboardInfo_right);
			}
			// Check the first board is up and running
			if (isCMX) {
				si_call_result = si_adapter.get_board_status(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								switchboardInfo_left : switchboardInfo_right);
			}

			if (si_call_result != switch_interface::SBIA_OK) {
				// ERROR: First switch board is not up and running
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>, _retries_counter == <%d>): the board is not yet up and running: si_call_result == <%d>",
						_mag_id, _slot, op_state_str(), _retries_counter,
						si_call_result);

				if (_retries_counter++ == 7) {
					_retries_counter = 0;

					// Raise the configuration fault alarm
					if (isSMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key,
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										switch_board_mag_str_left :
										switch_board_mag_str_right, _slot);
					}

					if (isCMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key,
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										switch_board_mag_str_left :
										switch_board_mag_str_right, _slot);
					}

					FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>, _retries_counter == <%d>): CONFIGURATION FAULT alarm raised",
							_mag_id, _slot, op_state_str(), _retries_counter);
				}

				break;
			}

			// Cease the configuration fault alarm
			if (isSMX) {
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								sb_left_key : sb_right_key);
			}
			if (isCMX) {
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								sb_left_key : sb_right_key);
			}

			_retries_counter = 0;

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): the board is now up and running: CONFIGURATION FAULT alarm ceased",
					_mag_id, _slot, op_state_str());

			// Here it's possible to restore the first board
			if (isSMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->getConfigurationManager().loadConfiguration(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key);
			}

			if (isCMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->getConfigurationManager().loadConfiguration(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key);
			}
			if (call_result) {
				// ERROR
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): unable to restore configuration over the switch board: call_result == <%d>: "
								"raising the CONFIGURATION FAULT alarm",
						_mag_id, _slot, op_state_str(), call_result);
				if (isSMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				if (isCMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				break;
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): the board configuration successfully reloaded from IMM",
					_mag_id, _slot, op_state_str());
			if (isSMX) {
				fixs_ith::workingSet_t::instance()->getConfigurationManager().removeFromReloadInProgressSet(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								sb_left_key : sb_right_key);
			}
			if (isCMX) {
				fixs_ith::workingSet_t::instance()->getConfigurationManager().removeFromReloadInProgressSet(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								sb_left_key : sb_right_key);
			}

			_op_state = FIRST_SWITCHBOARD_CONFIG_RESTORE_CREATE_BNC_OBJECTS;

			break;

		case FIRST_SWITCHBOARD_CONFIG_RESTORE_CREATE_BNC_OBJECTS:
			// BNC Handling
			if (isSMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->get_transportBNChandler().create_bnc_objects(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key);
			}

			if (isCMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->get_transportBNChandler().create_bnc_objects(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key);
			}
			if ((call_result != fixs_ith::ERR_NO_ERRORS)
					&& (call_result != fixs_ith::ERR_CS_NO_BOARD_FOUND)) {
				// ERROR
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): unable to create BNC objects on the switch board: call_result == <%d>: "
								"raising the CONFIGURATION FAULT alarm",
						_mag_id, _slot, op_state_str(), call_result);
				if (isSMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}
				if (isCMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				break;
			} else {
				// Objects created or board removed: in both cases do not reschedule the operation
				call_result = fixs_ith::ERR_NO_ERRORS;
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): BNC objects successfully created on the switch board",
					_mag_id, _slot, op_state_str());

			_retries_counter = 0;

			if (isSMX) {
				_op_state = FIRST_SWITCHBOARD_CONFIG_RESTORE_PIM_ADAPTION;
			}

			if (isCMX) {
				_op_state = FIRST_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS;
			}

			break;

		case FIRST_SWITCHBOARD_CONFIG_RESTORE_PIM_ADAPTION:
			call_result = fixs_ith::workingSet_t::instance()->getConfigurationManager().setPimAdaptionOfPorts((_slot == fixs_ith::BOARD_SMX_LEFT) ?
					sb_left_key : sb_right_key);
			if (call_result != fixs_ith::ERR_NO_ERRORS) {
				// ERROR
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
						"_slot == <%d>, _op_state == <%s>): unable to set PIM on the switch board: call_result == <%d>: "
						"raising the CONFIGURATION FAULT alarm",
						_mag_id, _slot, op_state_str(), call_result);
				if (isSMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key,
									(_slot == fixs_ith::BOARD_SMX_LEFT) ?
											switch_board_mag_str_left :
											switch_board_mag_str_right, _slot);
				}
				if (isCMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key,
									(_slot == fixs_ith::BOARD_CMX_LEFT) ?
											switch_board_mag_str_left :
											switch_board_mag_str_right, _slot);
				}

				break;
			} else {
				// Objects created or board removed: in both cases do not reschedule the operation
				call_result = fixs_ith::ERR_NO_ERRORS;
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
					"_slot == <%d>, _op_state == <%s>): PIM successfully created on the switch board",
					_mag_id, _slot, op_state_str());

			_retries_counter = 0;
			_op_state = FIRST_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS;
			break;
					
		case FIRST_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS:
			// Front ports activation handling
			if (isSMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->getConfigurationManager().setAdminStateOfDelayedPorts(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key,
								imm::EMPTY_ADMIN_STATE);
			}
			if (isCMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->getConfigurationManager().setAdminStateOfDelayedPorts(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key,
								imm::EMPTY_ADMIN_STATE);
			}

			if (call_result != fixs_ith::ERR_NO_ERRORS) {
				// ERROR
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): unable to set front ports admin state to PORT_STATUS_UP: "
								"call_result == <%d>: raising the CONFIGURATION FAULT alarm",
						_mag_id, _slot, op_state_str(), call_result);
				if (isSMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				if (isCMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				break;
			} else {
				// Objects created or board removed: in both cases do not reschedule the operation
				call_result = fixs_ith::ERR_NO_ERRORS;
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): front ports admin state successfully set to PORT_STATUS_UP",
					_mag_id, _slot, op_state_str());

			// Cease the alarm
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): Ceasing the SMXConfigurationFault alarm...",
					_mag_id, _slot, op_state_str());

			if (isSMX) {
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								sb_left_key : sb_right_key);
			}
			if (isCMX) {
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								sb_left_key : sb_right_key);
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): SMXConfigurationFault alarm ceased",
					_mag_id, _slot, op_state_str());

			// Store that the restore procedure on current board was successfull
			fixs_ith_brf_policy_handler::board_restore_ongoing(
					map_switch_board_index(_mag_id, _slot), 0);

			// Re-enable traps
			if (isSMX) {
				fixs_ith::workingSet_t::instance()->get_trapdsobserver().enable_traps(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								sb_left_key : sb_right_key, trap_mask::ALL);
			}
			if (isCMX) {
				fixs_ith::workingSet_t::instance()->get_trapdsobserver().enable_traps(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								sb_left_key : sb_right_key, trap_mask::ALL);
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): all switch board traps reenabled",
					_mag_id, _slot, op_state_str());

			// Change the slot
			if (isSMX) {
				_slot = (
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								fixs_ith::BOARD_SMX_RIGHT :
								fixs_ith::BOARD_SMX_LEFT);
			}
			if (isCMX) {
				_slot = (
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								fixs_ith::BOARD_CMX_RIGHT :
								fixs_ith::BOARD_CMX_LEFT);
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): _slot selected == <%d>",
					_mag_id, _slot, op_state_str(), _slot);

			// Change state
			_op_state = SECOND_SWITCHBOARD_RESET;
			re_schedule_with_other_params = true;
			re_schedule_delay = 0;
			_retries_counter = 0;
			looping = false;
			call_result = fixs_ith::ERR_NO_ERRORS;
			re_schedule_delay = 10;
			break;

		case SECOND_SWITCHBOARD_RESET:
			if (fixs_ith_brf_policy_handler::board_restore_ongoing(
					map_switch_board_index(_mag_id, _slot))) {
				// Disable traps handling
				if (isSMX) {
					fixs_ith::workingSet_t::instance()->get_trapdsobserver().disable_traps(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key, trap_mask::ALL);
				}
				if (isCMX) {
					fixs_ith::workingSet_t::instance()->get_trapdsobserver().disable_traps(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key, trap_mask::ALL);

				}

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): all trap disabled",
						_mag_id, _slot, op_state_str());

				// Reset the second board
				if (isSMX) {
					si_call_result = si_adapter.reset_board(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key);
				}
				if (isCMX) {
					si_call_result = si_adapter.reset_board(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key);
				}
				if (si_call_result == switch_interface::SBIA_OK) {
					// we successfully reset the SECOND switchboard. Wait that switchboard become again UP an RUNNING
					_op_state = SECOND_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM;
					re_schedule_delay = 20; //TODO: define a constant
					call_result = 0;

					// Cease the configuration fault alarm
					if (isSMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key);
					}
					if (isCMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key);
					}

					FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): reset board command successfully ordered: CONFIGURATION FAULT alarm ceased",
							_mag_id, _slot, op_state_str());
				} else { //ERROR
					re_schedule_delay = 0; //TODO: define a constant
					call_result = -1;

					// Raise the configuration fault alarm
					if (isSMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key,
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										switch_board_mag_str_left :
										switch_board_mag_str_right, _slot);
					}
					if (isCMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key,
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										switch_board_mag_str_left :
										switch_board_mag_str_right, _slot);
					}

					FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): unable to order board reset command: si_call_result == <%d>: "
									"CONFIGURATION FAULT alarm reaised",
							_mag_id, _slot, op_state_str(), si_call_result);
				}

				re_schedule_with_other_params = true;
				looping = false;
			} else {
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): This switch board was already restored: nothing to do",
						_mag_id, _slot, op_state_str());

				_op_state = CHANGE_MAGAZINE;

#if 0 // TODO: TO BE DELETED
				// Change the slot
				_slot = ((_slot == fixs_ith::BOARD_SMX_LEFT) ? fixs_ith::BOARD_SMX_RIGHT : fixs_ith::BOARD_SMX_LEFT);

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN, " ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
						"_slot == <%d>, _op_state == <%s>): _slot selected == <%d>",
						_mag_id, _slot, op_state_str(), _slot);
#endif
			}

			_retries_counter = 0;

			break;

		case SECOND_SWITCHBOARD_CONFIG_RESTORE_LOAD_FROM_IMM:
			// Check the first board is up and running
			if (isSMX) {
				si_call_result = si_adapter.get_board_status(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								switchboardInfo_left : switchboardInfo_right);
			}

			if (isCMX) {
				si_call_result = si_adapter.get_board_status(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								switchboardInfo_left : switchboardInfo_right);
			}

			if (si_call_result != switch_interface::SBIA_OK) {
				// ERROR: Second switch board is not up and running
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>, _retries_counter == <%d>): the board is not yet up and running: si_call_result == <%d>",
						_mag_id, _slot, op_state_str(), _retries_counter,
						si_call_result);

				if (_retries_counter++ == 7) {
					_retries_counter = 0;

					// Raise the configuration fault alarm
					if (isSMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key,
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										switch_board_mag_str_left :
										switch_board_mag_str_right, _slot);
					}

					if (isCMX) {
						fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key,
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										switch_board_mag_str_left :
										switch_board_mag_str_right, _slot);
					}

					FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>, _retries_counter == <%d>): CONFIGURATION FAULT alarm raised",
							_mag_id, _slot, op_state_str(), _retries_counter);
				}

				break;
			}

			// Cease the configuration fault alarm
			if (isSMX) {
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								sb_left_key : sb_right_key);
			}
			if (isCMX) {
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								sb_left_key : sb_right_key);
			}

			_retries_counter = 0;

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): the board is now up and running: CONFIGURATION FAULT alarm ceased",
					_mag_id, _slot, op_state_str());

			// Here it's possible to restore the second board
			if (isSMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->getConfigurationManager().loadConfiguration(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key);
			}

			if (isCMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->getConfigurationManager().loadConfiguration(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key);
			}

			if (call_result) {
				// ERROR
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): unable to restore configuration over the switch board: call_result == <%d>: "
								"raising the CONFIGURATION FAULT alarm",
						_mag_id, _slot, op_state_str(), call_result);

				if (isSMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}
				if (isCMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				break;
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): the board configuration successfully reloaded from IMM",
					_mag_id, _slot, op_state_str());
			if (isSMX) {
				fixs_ith::workingSet_t::instance()->getConfigurationManager().removeFromReloadInProgressSet(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								sb_left_key : sb_right_key);
			}
			if (isCMX) {
				fixs_ith::workingSet_t::instance()->getConfigurationManager().removeFromReloadInProgressSet(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								sb_left_key : sb_right_key);
			}

			_op_state = SECOND_SWITCHBOARD_CONFIG_RESTORE_CREATE_BNC_OBJECTS;

			break;

		case SECOND_SWITCHBOARD_CONFIG_RESTORE_CREATE_BNC_OBJECTS:
			// BNC Handling
			if (isSMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->get_transportBNChandler().create_bnc_objects(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key);
			}
			if (isCMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->get_transportBNChandler().create_bnc_objects(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key);
			}

			if ((call_result != fixs_ith::ERR_NO_ERRORS)
					&& (call_result != fixs_ith::ERR_CS_NO_BOARD_FOUND)) {
				// ERROR
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): unable to create BNC objects on the switch board: call_result == <%d>: "
								"raising the CONFIGURATION FAULT alarm",
						_mag_id, _slot, op_state_str(), call_result);

				if (isSMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				if (isCMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				break;
			} else {
				// Objects created or board removed: in both cases do not reschedule the operation
				call_result = fixs_ith::ERR_NO_ERRORS;
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): BNC objects successfully created on the switch board",
					_mag_id, _slot, op_state_str());

			_retries_counter = 0;

			if (isSMX) {
				_op_state = SECOND_SWITCHBOARD_CONFIG_RESTORE_PIM_ADAPTION;
			}

			if (isCMX) {
				_op_state = SECOND_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS;
			}

			break;
			
		case SECOND_SWITCHBOARD_CONFIG_RESTORE_PIM_ADAPTION:
			call_result = fixs_ith::workingSet_t::instance()->getConfigurationManager().setPimAdaptionOfPorts((_slot == fixs_ith::BOARD_SMX_LEFT) ?
					sb_left_key : sb_right_key);
			
			if (call_result != fixs_ith::ERR_NO_ERRORS) {
				// ERROR
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
						"_slot == <%d>, _op_state == <%s>): unable to set PIM on the switch board: call_result == <%d>: "
						"raising the CONFIGURATION FAULT alarm",
						_mag_id, _slot, op_state_str(), call_result);
				if (isSMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key,
									(_slot == fixs_ith::BOARD_SMX_LEFT) ?
											switch_board_mag_str_left :
											switch_board_mag_str_right, _slot);
				}
				if (isCMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key,
									(_slot == fixs_ith::BOARD_CMX_LEFT) ?
											switch_board_mag_str_left :
											switch_board_mag_str_right, _slot);
				}

				break;
			} else {
				// Objects created or board removed: in both cases do not reschedule the operation
				call_result = fixs_ith::ERR_NO_ERRORS;
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
					"_slot == <%d>, _op_state == <%s>): PIM successfully created on the switch board",
					_mag_id, _slot, op_state_str());

			_retries_counter = 0;
			_op_state = SECOND_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS;
			break;

		case SECOND_SWITCHBOARD_CONFIG_RESTORE_ACTIVATE_FRONT_PORTS:
			// Front ports activation handling
			if (isSMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->getConfigurationManager().setAdminStateOfDelayedPorts(
								(_slot == fixs_ith::BOARD_SMX_LEFT) ?
										sb_left_key : sb_right_key,
								imm::EMPTY_ADMIN_STATE);
			}
			if (isCMX) {
				call_result =
						fixs_ith::workingSet_t::instance()->getConfigurationManager().setAdminStateOfDelayedPorts(
								(_slot == fixs_ith::BOARD_CMX_LEFT) ?
										sb_left_key : sb_right_key,
								imm::EMPTY_ADMIN_STATE);
			}
			if (call_result != fixs_ith::ERR_NO_ERRORS) {
				// ERROR
				looping = false;
				re_schedule_with_other_params = true;
				call_result = -1;
				re_schedule_delay = 10;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): unable to set front ports admin state: "
								"call_result == <%d>: raising the CONFIGURATION FAULT alarm",
						_mag_id, _slot, op_state_str(), call_result);
				if (isSMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_SMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				if (isCMX) {
					fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									sb_left_key : sb_right_key,
							(_slot == fixs_ith::BOARD_CMX_LEFT) ?
									switch_board_mag_str_left :
									switch_board_mag_str_right, _slot);
				}

				break;
			} else {
				// Objects created or board removed: in both cases do not reschedule the operation
				call_result = fixs_ith::ERR_NO_ERRORS;
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): front ports admin state successfully set",
					_mag_id, _slot, op_state_str());

			// Cease the alarm
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): Ceasing the SMXConfigurationFault alarm...",
					_mag_id, _slot, op_state_str());
			if (isSMX) {
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								sb_left_key : sb_right_key);
			}
			if (isCMX) {
				fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								sb_left_key : sb_right_key);
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): SMXConfigurationFault alarm ceased",
					_mag_id, _slot, op_state_str());

			// Store that the restore procedure on current board was successfull
			fixs_ith_brf_policy_handler::board_restore_ongoing(
					map_switch_board_index(_mag_id, _slot), 0);

			// Re-enable traps
			if (isSMX) {
				fixs_ith::workingSet_t::instance()->get_trapdsobserver().enable_traps(
						(_slot == fixs_ith::BOARD_SMX_LEFT) ?
								sb_left_key : sb_right_key, trap_mask::ALL);
			}
			if (isCMX) {
				fixs_ith::workingSet_t::instance()->get_trapdsobserver().enable_traps(
						(_slot == fixs_ith::BOARD_CMX_LEFT) ?
								sb_left_key : sb_right_key, trap_mask::ALL);
			}

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
							"_slot == <%d>, _op_state == <%s>): all switch board traps reenabled",
					_mag_id, _slot, op_state_str());

			_op_state = CHANGE_MAGAZINE;
			_retries_counter = 0;

			break;

		case CHANGE_MAGAZINE:
			if (_mag_id == fixs_ith::APT_MAGAZINE) {
				FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
						" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
								"_slot == <%d>, _op_state == <%s>): Switch boards restore operation successfully terminated",
						_mag_id, _slot, op_state_str());

				// Store the success on restoring the configuration
				fixs_ith_brf_policy_handler::restore_ongoing(0);

				re_schedule_with_other_params = false;
			} else {
				// Reschedule the same operation for the APT magazine if needed. Check if there are boards in APT magazine
				if (fixs_ith::workingSet_t::instance()->get_sbdatamanager().search_for_magazine(
						fixs_ith::APT_MAGAZINE)
						== fixs_ith_sbdatamanager::SBM_OK) {
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): Going to restore configuration onto the other magazine APT_MAGAZINE",
							_mag_id, _slot, op_state_str());

					_mag_id = fixs_ith::APT_MAGAZINE;
					_op_state = NOT_STARTED;
					_slot = fixs_ith::BOARD_SMX_UNDEFINED;
					_retries_counter = 0;
					re_schedule_with_other_params = true;
					if (isCMX) {
						_slot = fixs_ith::BOARD_CMX_UNDEFINED;
					}
				} else {
					FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
							" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
									"_slot == <%d>, _op_state == <%s>): No APT magazine present: Switch boards restore operation "
									"successfully terminated", _mag_id, _slot,
							op_state_str());

					// Store the success on restoring the configuration
					fixs_ith_brf_policy_handler::restore_ongoing(0);

					re_schedule_with_other_params = false;
				}
			}

			looping = false;
			call_result = 0;
			re_schedule_delay = 0;

			break;

		default:
			break;
		}
	}

	// STEP 0  ( Do not miss to disable traps handling & Raise all alarms )
	// STEP 1  ( check that the two SMXs in the magazine are UP and RUNNING )
	// STEP 2  ( select randomly a position (SMX_LEFT, SMX_RIGHT )
	// STEP 3.1 ( order RESET on the selected BOARD )
	// STEP 3.2 ( wait for it to become UP and RUNNING )
	// STEP 3.3 ( load configuration on IT )
	// STEP 4  ( order RESET on the other BOARD, wait for it to become UP and RUNNING,
	//				 and load configuration on IT )
	// STEP 5  ( if the magazine is APZ we must order the same operation on APT )
	// STEP 6  ( Remember to create the storage clear area ith folder )
	// STEP X  ( Cease alarms as needed by the flow )

	if (re_schedule_with_other_params) {
		// we need to reschedule the same operation, but with different params
		Magazine_switchboards_config_restore::op_parameters op_params;

		op_params.mag_id = _mag_id;
		op_params.op_state = _op_state;

		if (_op_state == FIRST_SWITCHBOARD_RESET) {
			if (isCMX) {
				if (_slot == fixs_ith::BOARD_CMX_RIGHT)
					_slot = fixs_ith::BOARD_CMX_LEFT;
				else
					_slot = fixs_ith::BOARD_CMX_RIGHT;
			}

		}

		op_params.slot = _slot;
		op_params.retries_counter = _retries_counter;

		FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO,
				" ->>> MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE(_mag_id == <%d>, "
						"_slot == <%d>, _op_state == <%s>, _retries_counter == <%d>): Rescheduling operation with other parameters",
				_mag_id, _slot, op_state_str(), _retries_counter);

		call_result =
				fixs_ith::workingSet_t::instance()->schedule_until_completion(
						MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE, re_schedule_delay,
						3, &op_params, sizeof(op_params));
	}

	// set result to caller
	m_operationResult.setErrorCode(call_result);
	setResultToCaller();

	return fixs_ith::ERR_NO_ERRORS;
}

void Magazine_switchboards_config_restore::setOperationDetails(
		const void* op_details) {
	const op_parameters * params =
			reinterpret_cast<const op_parameters *>(op_details);
	_mag_id = params->mag_id;
	_slot = params->slot;
	_op_state = params->op_state;
	_retries_counter = params->retries_counter;
}

int Magazine_switchboards_config_restore::load_switchboards_info(
		fixs_ith_switchboardinfo & switchboardInfo_left,
		std::string & switch_board_mag_str_left,
		fixs_ith_switchboardinfo & switchboardInfo_right,
		std::string & switch_board_mag_str_right) {

	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
			"Restore::load_switchboards_info: Entered");

	fixs_ith_sbdatamanager::sbm_constant sbm_call_result_left =
			fixs_ith_sbdatamanager::SBM_OK;
	fixs_ith_sbdatamanager::sbm_constant sbm_call_result_right =
			fixs_ith_sbdatamanager::SBM_OK;

	//GEP2_GEP5 to GEP7 HWSWAP
	bool isSMX = common::utility::isSwitchBoardSMX();
	bool isCMX = common::utility::isSwitchBoardCMX();

	if (isSMX) {
		sbm_call_result_left =
				fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
						switchboardInfo_left, switch_board_mag_str_left,
						_mag_id, fixs_ith::BOARD_SMX_LEFT);
	}
	if (isCMX) {
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
				"Restore::Fetching switch_board_info for CMX LEFT board during restore");
		sbm_call_result_left =
				fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
						switchboardInfo_left, switch_board_mag_str_left,
						_mag_id, fixs_ith::BOARD_CMX_LEFT);

	}
	if (sbm_call_result_left != fixs_ith_sbdatamanager::SBM_OK) {
		if (sbm_call_result_left
				== fixs_ith_sbdatamanager::SBM_BOARD_NOT_FOUND) {
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					"The left switch board on magazine <%s> was not found: _mag_id == <%d>, "
							"slot == <BOARD_SMX_LEFT>, sbm_call_result_left == <%d>",
					switch_board_mag_str_left.c_str(), _mag_id,
					sbm_call_result_left);
		} else {
			// ERROR: Cannot get switch board info
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
					"Call to 'get_switchboard_info()' failed: Cannot get switch board info: "
							"_mag_id == <%d>, slot == <BOARD_SMX_LEFT>, sbm_call_result_left == <%d>",
					_mag_id, sbm_call_result_left);
			return -1;
		}
	}

	if (isSMX) {

		sbm_call_result_right =
				fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
						switchboardInfo_right, switch_board_mag_str_right,
						_mag_id, fixs_ith::BOARD_SMX_RIGHT);
	}
	if (isCMX) {
		FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
				"Restore::Fetching switch_board_info for CMX RIGHT board during restore");

		sbm_call_result_right =
				fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
						switchboardInfo_right, switch_board_mag_str_right,
						_mag_id, fixs_ith::BOARD_CMX_RIGHT);
	}

	if (sbm_call_result_right != fixs_ith_sbdatamanager::SBM_OK) {
		if (sbm_call_result_right
				== fixs_ith_sbdatamanager::SBM_BOARD_NOT_FOUND) {
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_WARN,
					"The right switch board on magazine <%s> was not found: _mag_id == <%d>, "
							"slot == <BOARD_SMX_RIGHT>, sbm_call_result_right == <%d>",
					switch_board_mag_str_right.c_str(), _mag_id,
					sbm_call_result_right);
		} else {
			// ERROR: Cannot get switch board info
			FIXS_ITH_LOG_IMPL(LOG_LEVEL_ERROR,
					"Call to 'get_switchboard_info()' failed: Cannot get switch board info: "
							"_mag_id == <%d>, slot == <BOARD_SMX_RIGHT>, sbm_call_result_right == <%d>",
					_mag_id, sbm_call_result_right);
			return -1;
		}
	}
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "Restore::sbm_call_result_left ==%d",
			sbm_call_result_left);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "Restore::sbm_call_result_right ==%d",
			sbm_call_result_right);
	return (((sbm_call_result_left
			== fixs_ith_sbdatamanager::SBM_BOARD_NOT_FOUND)
			&& (sbm_call_result_right
					== fixs_ith_sbdatamanager::SBM_BOARD_NOT_FOUND)) ? 1 : 0);

}

unsigned Magazine_switchboards_config_restore::map_switch_board_index (fixs_ith::magazine_identifier_t mag_id, int32_t slot)
{

	if(common::utility::isSwitchBoardCMX())
	{
		return ((static_cast<unsigned>(mag_id) << 1) | ((slot == fixs_ith::BOARD_CMX_LEFT) ? 0 : 1));
	}
	else
	{
		return ((static_cast<unsigned>(mag_id) << 1) | ((slot == fixs_ith::BOARD_SMX_LEFT) ? 0 : 1));
	}
}
}

