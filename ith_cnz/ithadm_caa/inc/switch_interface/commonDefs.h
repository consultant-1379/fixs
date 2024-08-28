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

#ifndef ITHADM_CAA_INC_FIXS_ITH_SWITCHINTERFACECOMMONS_H_
#define ITHADM_CAA_INC_FIXS_ITH_SWITCHINTERFACECOMMONS_H_

#include <stdint.h>
#include <string>
#include "fixs_ith_programconstants.h"
#include "imm/imm.h"

namespace switch_interface
{
	// Return codes for switch interface operations
	enum op_result
	{
		SBIA_OK = 0,
		SBIA_ERROR = -1,
		SBIA_OP_NOT_IMPLEMENTED = -2,
		SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE = -3,
		SBIA_INVALID_PLANE = -4,
		SBIA_OBJ_ALREADY_EXIST = -5,
		SBIA_OBJ_NOT_FOUND = -6,
		SBIA_OP_NOT_PERMITTED = -7,
		SBIA_OP_DATA_MISSING = -8,
		SBIA_OBJECT_CORRUPTED = -9
	};

	// Operative states of an interface
	enum IF_oper_state
	{
		IF_DISABLED = 0,    	// DISABLED
		IF_ENABLED = 1,     	// ENABLED
		IF_STATE_UNKNOWN = 2
	};

	// Administrative states of a BridgePort
	enum BP_admin_state
	{
		BP_LOCKED = 0,	    // LOCKED
		BP_UNLOCKED = 1,	// UNLOCKED
		BP_UNDEFINED = 2	// UNDEFINED
	};

	// Administrative states of a BridgePort
	enum OAM_admin_state
	{
		OAM_ENABLED = 1,	 // OAM ENABLED
		OAM_DISABLED = 2	// OAM DISABLED
	};

	enum VLAN_port_type
	{
		VLAN_UNTAGGED = 0,
		VLAN_TAGGED = 1,
	};

	// Operating mode of an interface
	enum IF_oper_mode {
		IF_OPERMODE_UNKNOWN = 1,
		IF_OPERMODE_10M_HALF = 2,
		IF_OPERMODE_10M_FULL = 3,
		IF_OPERMODE_100M_HALF = 4,
		IF_OPERMODE_100M_FULL = 5,
		IF_OPERMODE_1G_FULL = 6,
		IF_OPERMODE_1G_FULL_SLAVE = 7,
		IF_OPERMODE_1G_FULL_MASTER = 8,
		IF_OPERMODE_10G_FULL = 9,
		IF_OPERMODE_10G_FULL_MASTER = 10,
		IF_OPERMODE_10G_FULL_SLAVE = 11,
		IF_OPERMODE_40G_FULL = 12,
		IF_OPERMODE_100G_FULL = 13,
	};

	// *** Helper functions for switch interface adapters  ***
	namespace adapters_utils
	{
		int build_sb_key_from_magazine_and_slot(std::string & key, const std::string & mag_str, fixs_ith::BoardSlotPosition slot_pos);

		const char * plane_descr( fixs_ith::switchboard_plane_t plane);

		const char * if_operativeState_descr(IF_oper_state if_op_state);

		const char * if_adminState_descr(imm::admin_State if_admin_state);

		enum imm::oper_State if_operState_from_descr_to_enum(const std::string & if_operState_str);

		enum imm::admin_State if_adminState_from_descr_to_enum(const std::string & if_adminState_str);

		enum imm::vrrpvPrioSession if_vrrpvPrioSession_from_descr_to_enum (
				const std::string & if_vrrpvPrioSession_str);

		enum imm::vrrpSessionState vrrpSessionState_from_descr_to_enum(const std::string & vrrpSessionState_str);

		enum imm::BfdSessionOperState bfdSessionState_from_descr_to_enum(const std::string& bfdSessionState_str);

		enum imm::BfdSessionDiagnCode bfdSessionDiagnosticCode_from_descr_to_enum(const std::string& bfdSessionDiagCode_str);
	}
}

namespace fixs_ith_switchInterface = switch_interface;


#endif /* ITHADM_CAA_INC_FIXS_ITH_SWITCHINTERFACECOMMONS_H_ */
