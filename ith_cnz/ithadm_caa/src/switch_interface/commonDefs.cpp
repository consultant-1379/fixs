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

#include <stdio.h>
#include <strings.h>

#include "switch_interface/commonDefs.h"

#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_snmpconstants.h"
#include "netconf/netconf.h"
#include "imm/imm.h"

namespace switch_interface
{
	namespace adapters_utils
	{
		int build_sb_key_from_magazine_and_slot(std::string & key, const std::string & mag_str, fixs_ith::BoardSlotPosition slot_pos)
		{
			// build the key combining MAGAZINE string and SLOT position
			char mag_slot_str[32]= {0};
			::snprintf(mag_slot_str, FIXS_ITH_ARRAY_SIZE(mag_slot_str), "%s_%d", mag_str.c_str(), slot_pos);

			// return the key in the output parameter
			key.assign(mag_slot_str);
			return 0;
		}

		const char * plane_descr( fixs_ith::switchboard_plane_t plane)
		{
			switch(plane)
			{
			case fixs_ith::CONTROL_PLANE:
				return "CONTROL";
			case fixs_ith::TRANSPORT_PLANE:
				return "TRANSPORT";
			case fixs_ith::CONTROL_TRANSPORT_PLANES:
				return "CONTROL and TRANSPORT";
			default:
				return "UNKNOWN";
			}
		}

		const char * if_operativeState_descr(IF_oper_state if_op_state)
		{
			switch(if_op_state)
			{
			case IF_DISABLED:
				return "DISABLED";
			case IF_ENABLED:
				return "ENABLED";
			case IF_STATE_UNKNOWN:
				return "UNKNOWN";
			default:
				return "UNKNOWN";
			}
		}

		const char * if_adminState_descr(imm::admin_State if_admin_state)
		{
			switch(if_admin_state)
			{
			case imm::LOCKED:
				return "LOCKED";
			case imm::UNLOCKED:
				return "UNLOCKED";
			case imm::EMPTY_ADMIN_STATE:
				return "EMPTY_ADMIN_STATE";
			default:
				return "EMPTY_ADMIN_STATE";
			}
		}

		enum imm::oper_State if_operState_from_descr_to_enum(const std::string & if_operState_str)
		{
			if(if_operState_str.compare("ENABLED") == 0)
				return imm::ENABLED;
			else if(if_operState_str.compare("DISABLED") == 0)
				return imm::DISABLED;
			else
				return imm::EMPTY_OPER_STATE;
		}

		enum imm::admin_State if_adminState_from_descr_to_enum(const std::string & if_adminState_str)
		{
			if(if_adminState_str.compare("LOCKED") == 0)
				return imm::LOCKED;
			else if(if_adminState_str.compare("UNLOCKED") == 0)
				return imm::UNLOCKED;
			else
				return imm::EMPTY_ADMIN_STATE;
		}

		enum imm::vrrpvPrioSession if_vrrpvPrioSession_from_descr_to_enum (
				const std::string & if_vrrpvPrioSession_str) {
			return
				(strcasecmp(if_vrrpvPrioSession_str.c_str(), "TRUE") ? imm::VRRP_FALSE : imm::VRRP_TRUE);
		}

		enum imm::vrrpSessionState vrrpSessionState_from_descr_to_enum(const std::string & vrrpSessionState_str)
		{
			if(vrrpSessionState_str == "MASTER")
				return imm::MASTER;
			else if(vrrpSessionState_str == "INITIALIZE")
				return imm::INIT;
			else if(vrrpSessionState_str == "BACKUP")
				return imm::BACKUP;
			else
				return imm::INIT;  // TODO  It's good as default ???
		}

		enum imm::BfdSessionOperState bfdSessionState_from_descr_to_enum(const std::string& bfdSessionState_str)
		{
			if(bfdSessionState_str.compare("ADMIN_DOWN") == 0U) return imm::BFD_OPERSTATE_ADMIN_DOWN;

			if(bfdSessionState_str.compare("DOWN") == 0U) return imm::BFD_OPERSTATE_DOWN;

			if(bfdSessionState_str.compare("INIT") == 0U) return imm::BFD_OPERSTATE_INIT;

			if(bfdSessionState_str.compare("FAILING") == 0U) return imm::BFD_OPERSTATE_FAILING;

			if(bfdSessionState_str.compare("UP") == 0U) return imm::BFD_OPERSTATE_UP;

			return imm::BFD_OPERSTATE_INIT;
		}

		enum imm::BfdSessionDiagnCode bfdSessionDiagnosticCode_from_descr_to_enum(const std::string& bfdSessionDiagCode_str)
		{
			if(bfdSessionDiagCode_str.compare("NO_DIAG") == 0U) return imm::BFD_DC_NO_DIAG;

			if(bfdSessionDiagCode_str.compare("CONTROL_DETECTION_TIME_EXPIRED") == 0U) return imm::BFD_DC_CONTR_DETECTION_TIME_EXP;

			if(bfdSessionDiagCode_str.compare("ECHO_FUNCTION_FAILED") == 0U) return imm::BFD_DC_ECHO_FUNCTION_FAILED;

			if(bfdSessionDiagCode_str.compare("NEIGHBOR_SIGNALED_SESSION_DOWN") == 0U) return imm::BFD_DC_NEIGHBOR_SIG_SESSION_DOWN;

			if(bfdSessionDiagCode_str.compare("FORWARDING_PLANE_RESET") == 0U) return imm::BFD_DC_FORWARDING_PLANE_RESET;

			if(bfdSessionDiagCode_str.compare("PATH_DOWN") == 0U) return imm::BFD_DC_PATH_DOWN;

			if(bfdSessionDiagCode_str.compare("CONCATENATED_PATH_DOWN") == 0U) return imm::BFD_DC_CONCAT_PATH_DOWN;

			if(bfdSessionDiagCode_str.compare("ADMIN_DOWN") == 0U) return imm::BFD_DC_ADMIN_DOWN;

			if(bfdSessionDiagCode_str.compare("REVERSE_CONCATENATED_PATH_DOWN") == 0U) return imm::BFD_DC_REVERSE_CONCAT_PATH_DOWN;

			return imm::BFD_DC_NO_DIAG;
		}
	}
}

