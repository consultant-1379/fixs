/*
 * fixs_ith_snmpmanager
 *
 *  Created on: Feb 2, 2016
 *      Author: xassore
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <ace/Guard_T.h>

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <set>

#include "common/utility.h"
#include "fixs_ith_programconstants.h"
#include "fixs_ith_snmpconstants.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_snmpsessionhandler.h"
#include "fixs_ith_snmpsessioninfo.h"
#include "fixs_ith_csreader.h"
#include "fixs_ith_snmpmibdefinitions.h"
#include "fixs_ith_snmpmanagercallbacks.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_snmpmanager.h"

//TODO: find a better place for this
#define ASCII_IP_SIZE 10

const char * fixs_ith_snmpmanager::_snmp_v3_passphrase = "ITH Server NMS Passphrase";

bool scxColdRestartFlag = false;

namespace {
	inline bool fx_compare_slot_ascending (fixs_ith_switchboardinfo left, fixs_ith_switchboardinfo right) {
		return (left.slot_position < right.slot_position);
	}

	inline bool fx_compare_magazine_slot_ascending (fixs_ith_switchboardinfo left, fixs_ith_switchboardinfo right) {
		return ((left.magazine < right.magazine) || ((left.magazine == right.magazine) && (left.slot_position < right.slot_position)));
	}

	class match_sb_magazine_slot {
		//==============//
		// Constructors //
		//==============//
	public:
		inline match_sb_magazine_slot (uint32_t magazine, int32_t slot_position) : _magazine(magazine), _slot_position(slot_position) {}


		//============//
		// Destructor //
		//============//
	public:
		inline ~match_sb_magazine_slot () {}


		//===========//
		// Operators //
		//===========//
	public:
		inline bool operator() (const fixs_ith_switchboardinfo & switch_board) const {
					// BEGIN: DEBUGGING and TRACING: TO BE DELETE
			FIXS_ITH_LOG(LOG_LEVEL_TRACE, "TRACING CLASS [match_sb_magazine_slot] : Current switch board item to match is: magazine == %u, slot == %d", switch_board.magazine, switch_board.slot_position);
			FIXS_ITH_LOG(LOG_LEVEL_TRACE, "TRACING CLASS [match_sb_magazine_slot] : Parameter to match are: magazine == %u, slot == %d", _magazine, _slot_position);
			FIXS_ITH_LOG(LOG_LEVEL_TRACE, "TRACING CLASS [match_sb_magazine_slot] : Match result == %s",
					((switch_board.magazine == _magazine) && (switch_board.slot_position == _slot_position)) ? "TRUE" : "FALSE");
			// END: DEBUGGING and TRACING: TO BE DELETE
			return ((switch_board.magazine == _magazine) && (switch_board.slot_position == _slot_position));
				}

		//========//
		// Fields //
		//========//
	private:
		uint32_t _magazine;
		int32_t _slot_position;
	};
}



int fixs_ith_snmpmanager::send_trap_ack (int switch_board_slot_position, const char * ap_ip, const char * switch_board_ip) {
	//TODO - Should it be put here some synchronization system!?!
	UNUSED(switch_board_slot_position);
	UNUSED(ap_ip);
	UNUSED(switch_board_ip);
	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}


/****   snmp-set handling subscription/unsubscrption requests   ****/
int fixs_ith_snmpmanager::manage_ColdStart(const char * switch_board_ip)
{
// TODO: to be implemented
	UNUSED(switch_board_ip);
	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
} //+


int fixs_ith_snmpmanager::set_port_admin_status(fixs_ith_switchboardinfo & switch_board,
		uint16_t port_ifIndex, fixs_ith_snmp::port_status_t port_status,
		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Set Port admin status: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = set_port_admin_status(switch_board_ipn_str_addresses[ipn_index],port_ifIndex,port_status);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Set Port admin status: Call 'set_port_admin_status' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	return call_result;

}

int fixs_ith_snmpmanager::set_port_admin_status (const char * switch_board_ip, uint16_t port_ifIndex, fixs_ith_snmp::port_status_t port_status)
 {
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = 0;

	// Prepare the oid arrays
	oid oid_name [] = { IF_ADMIN_STATUS, port_ifIndex };
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);;
	int oid_value = port_status;

	call_result= set_port_mib_object<int>(const_cast<char *>(switch_board_ip),oid_name,oid_name_size,oid_value);

	return call_result;
 }


int fixs_ith_snmpmanager::set_port_default_vlan(fixs_ith_switchboardinfo & switch_board,
    		uint16_t port_ifIndex, uint16_t vid,
    		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Set Port default vlanId: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = set_port_default_vlan(switch_board_ipn_str_addresses[ipn_index],port_ifIndex,vid);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Set Port default vlanId: Call 'set_port_default_vlan' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	return call_result;
}

int fixs_ith_snmpmanager::set_port_default_vlan(const char * switch_board_ip,
    		uint16_t port_ifIndex, uint16_t vid)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = 0;

	// Prepare the oid arrays
	oid oid_name [] = {FIXS_ITH_SMX_OID_PORTVLAN_DOT1QPVID, port_ifIndex };
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
	int oid_value = vid;

	call_result= set_port_mib_object<int>(const_cast<char *>(switch_board_ip),oid_name,oid_name_size,oid_value);

	return call_result;

}

int fixs_ith_snmpmanager::set_port_oam_admin_status(fixs_ith_switchboardinfo & switch_board,
		uint16_t port_ifIndex, fixs_ith_snmp::oam_status_t port_oam_status,
		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Set Port OAM admin status: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = set_port_oam_admin_status(switch_board_ipn_str_addresses[ipn_index],port_ifIndex,port_oam_status);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Set Port admin status: Call 'set_port_oam_admin_status' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	return call_result;

}

int fixs_ith_snmpmanager::set_port_oam_admin_status (const char * switch_board_ip, uint16_t port_ifIndex, fixs_ith_snmp::oam_status_t port_oam_status)
 {
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = 0;

	// Prepare the oid arrays
	oid oid_name [] = { SNMP_DOT3_OAM_ADMIN_STATE, port_ifIndex };
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);;
	int oid_value = port_oam_status;

	call_result= set_port_mib_object<int>(const_cast<char *>(switch_board_ip),oid_name,oid_name_size,oid_value);

	return call_result;
 }

int fixs_ith_snmpmanager::set_eth_max_frame_size(fixs_ith_switchboardinfo & switch_board,
    		uint16_t port_ifIndex, uint32_t eth_max_frame_size,
    		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Set eth max frame size: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = set_eth_max_frame_size(switch_board_ipn_str_addresses[ipn_index],port_ifIndex,eth_max_frame_size);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Set eth max frame size: Call 'set_port_default_vlan' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	return call_result;
}

int fixs_ith_snmpmanager::set_eth_max_frame_size(const char * switch_board_ip,
    		uint16_t port_ifIndex, uint32_t eth_max_frame_size)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = 0;

	// Prepare the oid arrays
	oid oid_name [] = {IF_MAX_FRAME_SIZE, port_ifIndex };
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
	int oid_value = eth_max_frame_size;

	call_result= set_port_mib_object<uint32_t>(const_cast<char *>(switch_board_ip),oid_name,oid_name_size,oid_value);

	return call_result;

}

int fixs_ith_snmpmanager::enable_vrrp_new_master_notification(fixs_ith_switchboardinfo & switch_board )
{
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	fixs_ith::switchboard_plane_t sb_plane = fixs_ith::TRANSPORT_PLANE;

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Enable VRRP new master notification on SMX having IPNA == %s and IPNB == %s",
			switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
	{
		call_result = enable_vrrp_new_master_notification(switch_board_ipn_str_addresses[ipn_index]);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Enable VRRP new master notification: Call 'enable_vrrp_new_master_notification' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	return call_result;
}

int fixs_ith_snmpmanager::enable_vrrp_new_master_notification(const char* switch_board_ip)
{
	int call_result = 0;

	// Prepare the oid arrays
	oid oid_name [] = {SNMP_GEN_VRRP_NEW_MASTER_ENABLE};
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
	int oid_value = 1; // If set to true(1), notifications are enabled. If set to false(2), notifications are disabled.

	call_result= set_port_mib_object<int>(const_cast<char *>(switch_board_ip),oid_name,oid_name_size,oid_value);

	return call_result;
}


int fixs_ith_snmpmanager::enable_bfd_session_state_notification(fixs_ith_switchboardinfo & switch_board )
{
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	fixs_ith::switchboard_plane_t sb_plane = fixs_ith::TRANSPORT_PLANE;

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Enable BFD SESSION UP/DOWN notification on SMX having IPNA == %s and IPNB == %s",
			switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
	{
		call_result = enable_bfd_session_state_notification(switch_board_ipn_str_addresses[ipn_index]);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Enable BFD SESSION UP/DOWN notification: Call 'enable_bfd_session_state_notification' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::enable_bfd_session_state_notification(const char* switch_board_ip)
{
	// Prepare the oid arrays
	oid oid_name [] = {SNMP_GEN_BFD_UP_DOWN_NOTIFICATION_ENABLE};
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
	int oid_value = 1; // If set to true(1), notifications are enabled. If set to false(2), notifications are disabled.

	int call_result = set_port_mib_object<int>(const_cast<char *>(switch_board_ip), oid_name, oid_name_size, oid_value);

	return call_result;
}

int fixs_ith_snmpmanager::reset_board(const fixs_ith_switchboardinfo& switch_board)
{
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

        fixs_ith::switchboard_plane_t sb_plane;

        if(common::utility::isSwitchBoardSMX())
        {
	   sb_plane = fixs_ith::CONTROL_PLANE;
        }
         
        if(common::utility::isSwitchBoardCMX())
        {
           sb_plane = fixs_ith::TRANSPORT_PLANE;
        }

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_WARN, "RESET SMX BOARD with IPNA:<%s> and IPNB: <%s>",
			switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
	{
		call_result = reset_board(switch_board_ipn_str_addresses[ipn_index], switch_board.slot_position);
	}

	return call_result;
}


//TR HY52528 BEGIN
int fixs_ith_snmpmanager::reset_board(const fixs_ith_switchboardinfo& switch_board, fixs_ith::board_slotposition_t slot)
{
        char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
        fixs_ith::switchboard_plane_t sb_plane = fixs_ith::CONTROL_PLANE;
        int slot_position;

        if(slot == fixs_ith::BOARD_SMX_UNDEFINED)
        {
                slot_position = switch_board.slot_position;
        }
        else
        {
                slot_position = slot;
        }

        if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
                        switch_board_ipn_str_addresses[1],
                        switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

        FIXS_ITH_LOG(LOG_LEVEL_WARN, "RESET SMX BOARD with IPNA:<%s> and IPNB: <%s>",
                        switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
        {
                call_result = reset_board(switch_board_ipn_str_addresses[ipn_index], slot_position);
        }

        return call_result;
}
//TR HY52528 END

int fixs_ith_snmpmanager::reset_board(char* switch_board_ip, int32_t slot)
{

     int call_result = 0;
	
     if(common::utility::isSwitchBoardSMX())
     {
        // Prepare the oid arrays
	uint32_t index = slot;
	oid oid_name[] = {FIXS_ITH_SMX_OID_BLADE_RESET, index};
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	int reset_value = 0U;	// Setting this value will cause a blade reset at the blade specified.
							//The supported types of reset are: -coldreset(0)

	call_result = set_port_mib_object<int>(switch_board_ip, oid_name, oid_name_size, reset_value);

	if(call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "RESET SMX Board IP:<%s> slot:<%d> FAILED, error:<%d>",
				switch_board_ip, slot, call_result);
	}
     }

     if(common::utility::isSwitchBoardCMX())
     {
        // Prepare the oid arrays
        uint32_t index = 0;
        oid oid_name[] = {FIXS_ITH_CMX_OID_BLADE_RESET, index};
        size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

        int reset_value = 0U;   // Setting this value will cause a blade reset at the blade specified.
                                                        //The supported types of reset are: -coldreset(0)

        call_result = set_port_mib_object<int>(switch_board_ip, oid_name, oid_name_size, reset_value);

        if(call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "RESET CMX Board IP:<%s> slot:<%d> FAILED, error:<%d>",
                                switch_board_ip, slot, call_result);
        }
     }

   return call_result;
}

int fixs_ith_snmpmanager::disable_stp_port_status(fixs_ith_switchboardinfo& switch_board, uint16_t port_ifindex,fixs_ith::switchboard_plane_t sb_plane)
{
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Disable STP on the port if_index '%u' on SMX having IPNA == %s and IPNB == %s", port_ifindex,
			switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = disable_stp_port_status(switch_board_ipn_str_addresses[ipn_index], port_ifindex);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Disable STP port status: Call 'disable_stp_port_status' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::disable_stp_port_status(const char* switch_board_ip, uint16_t port_ifindex)
{
	// Prepare the oid arrays
	oid oid_name[] = {FIXS_ITH_SMX_OID_STP_PORT_STATUS, port_ifindex};
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	int  stp_status = fixs_ith_snmp::STP_PORT_DISABLED;

	int call_result = set_port_mib_object<int>(const_cast<char *> (switch_board_ip), oid_name, oid_name_size, stp_status);

	if(call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Disable STP on the port if_index <%u>, switch board IPN = '%s' FAILED, error:<%d>",
				 port_ifindex, switch_board_ip, call_result);
	}

	return call_result;

}

int fixs_ith_snmpmanager::setBridgePortasEdgePort(fixs_ith_switchboardinfo& switch_board, uint16_t port_ifindex, fixs_ith::switchboard_plane_t sb_plane)
{
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Set Bridge port <%u> as 'EDGE PORT' on SMX having IPNA == %s and IPNB == %s", port_ifindex,
			switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = setBridgePortasEdgePort(switch_board_ipn_str_addresses[ipn_index], port_ifindex);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Set Bridge port as EDGE PORT: Call 'setStpEdgePort' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::setBridgePortasEdgePort(const char* switch_board_ip, uint16_t port_ifindex)
{
	// Prepare the oid arrays
	oid oid_name[] = {FIXS_ITH_SMX_OID_STP_ADMINEDGE_PORT, port_ifindex};
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	int  stp_port_type = fixs_ith_snmp::STP_EDGE_PORT;

	int call_result = set_port_mib_object<int>(const_cast<char *> (switch_board_ip), oid_name, oid_name_size, stp_port_type);

	if(call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Set Bridge port <%u> as EDGE PORT on switch board IPN = '%s' FAILED, error:<%d>",
				 port_ifindex, switch_board_ip, call_result);
	}

	return call_result;

}

int
fixs_ith_snmpmanager::get_if_mau_type (
		std::vector<bool> & if_mau_type,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms) {
	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "IF MAU TYPE get info: [IF_INDEX = %d] on switch having IPNA == %s and IPNB == %s ",
			if_index, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_if_mau_type(if_mau_type, if_index, switch_board.slot_position, switch_board.magazine,
				switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"IF MAU TYPE get info: Call 'get_if_mau_type()' failed: if_index == %d, switch board slot == %d, switch board IPN = '%s': call_result == %d",
					if_index, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int
fixs_ith_snmpmanager::get_if_mau_type (
		std::vector<bool> & if_mau_type,
		uint16_t if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms) {
	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IF MAU TYPE get info: Trying to get 'interface MAU type' from switch '%s'", switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine,
			FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if (call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_if_mau_type_callback callback(if_mau_type, if_index, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU TYPE get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_if_mau_type [] = { IF_MAU_TYPE_LIST_BITS, if_index, 1 };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_if_mau_type; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_if_mau_type);

	call_result = get_if_mau_type(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}

int
fixs_ith_snmpmanager::get_if_mau_type (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms) {
	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IF MAU TYPE get info: Sending the PDU GET request to retrieve the 'if MAU type' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU TYPE get info: Timeout expired while sending the PDU GET request to retrieve 'if MAU type'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"IF MAU TYPE get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve "
				"'if Mau type': call_result == %d", call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU TYPE get info: Callback detected error on reading 'if MAU type' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

int
fixs_ith_snmpmanager::get_if_mau_default_type (
		std::vector<oid> & default_type,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms) {
	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if (get_switch_board_target_ipns(switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1],
			switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "IF MAU DEFAULT TYPE get info: [IF_INDEX = %d] on switch having IPNA == %s and IPNB == %s ",
			if_index, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;

	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_if_mau_default_type(default_type, if_index, switch_board.slot_position, switch_board.magazine,
				switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"IF MAU DEFAULT TYPE get info: Call 'get_if_mau_default_type()' failed: if_index == %d, switch board slot == %d, "
					"switch board IPN = '%s': call_result == %d", if_index, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index],
					call_result);
	}

	return call_result;
}

int
fixs_ith_snmpmanager::get_if_mau_default_type (
		std::vector<oid> & default_type,
		uint16_t if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms) {
	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IF MAU DEFAULT TYPE get info: Trying to get 'interface MAU default type' from switch '%s'", switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine,
			FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if (call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_if_mau_default_type_callback callback(default_type, if_index, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU DEFAULT TYPE get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_if_mau_default_type [] = { IF_MAU_DEFAULT_TYPE, if_index, 1 };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_if_mau_default_type; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_if_mau_default_type);

	call_result = get_if_mau_default_type(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}

int
fixs_ith_snmpmanager::get_if_mau_default_type (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {
	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IF MAU DEFAULT TYPE get info: Sending the PDU GET request to retrieve the 'if MAU default type' value "
			"into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU DEFAULT TYPE get info: Timeout expired while sending the PDU GET request to retrieve 'if MAU default type'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"IF MAU DEFAULT TYPE get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve "
				"'if Mau default type': call_result == %d", call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU DEFAULT TYPE get info: Callback detected error on reading 'if MAU default type' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

int
fixs_ith_snmpmanager::set_if_mau_default_type (
		fixs_ith_snmp::IfMauTypeListBitsConstants default_type,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane) {
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if (get_switch_board_target_ipns(switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Set MAU DEFAULT TYPE: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = set_if_mau_default_type(default_type, if_index, switch_board_ipn_str_addresses[ipn_index]);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"SET IF MAU DEFAULT TYPE: Call 'set_if_mau_default_type' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int
fixs_ith_snmpmanager::set_if_mau_default_type (
		fixs_ith_snmp::IfMauTypeListBitsConstants default_type,
		uint16_t if_index,
		const char * switch_board_ip) {
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = 0;

	// Prepare the oid arrays
	oid oid_name [] = { IF_MAU_DEFAULT_TYPE, if_index, 1 };
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	oid oid_value [] = { DOT3_IF_MAU_TYPE_BASE_OID, default_type };
	size_t oid_value_size = FIXS_ITH_ARRAY_SIZE(oid_value);

	call_result = set_if_mau_default_type(oid_name, oid_name_size, oid_value, oid_value_size, switch_board_ip);

	return call_result;
}

int
fixs_ith_snmpmanager::set_if_mau_default_type (
		oid * oid_name,
		size_t oid_name_size,
		oid * oid_value,
		size_t oid_value_size,
		const char * switch_board_ip) {
	fixs_ith_snmpsessioninfo session_info;

	session_info.version(SNMP_VERSION_2c);
	session_info.community(const_cast<char *>(ITH_SNMP_COMMUNITY_RW_STRING));

	// snmp session communication handler used to send request and the callback used to receive the ACK response
	fixs_ith_snmpsessionhandler session_handler;
	fixs_ith_snmploggercallback logger_callback("SET IF MAU DEFAULT TYPE: SNMP Response received for PORT configuration");

	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"SET IF MAU DEFAULT TYPE: sending request to switch board IP %s", switch_board_ip);

	session_info.peername(const_cast<char *>(switch_board_ip));
	logger_callback.switch_board_ip(switch_board_ip);

	int call_result = 0;

	if ((call_result = session_handler.open(session_info, logger_callback))) { // ERROR: opening the communication session towards the switch board
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"SET IF MAU DEFAULT TYPE: Call 'open()' failed: cannot open the communication session towards the above switch board: "
				"system error == %d, snmp error == %d, snmp error text == '%s': call_result == %d",
				session_info.last_sys_error(), session_info.last_snmp_error(), session_info.last_snmp_error_text(), call_result);
		return fixs_ith_snmp::ERR_SNMP_SESSION_OPEN;
	}

	// Prepare the PDU subscription request ITH will send to the current switch board
	netsnmp_pdu * request_pdu = 0;

	if (!(request_pdu = snmp_pdu_create(SNMP_MSG_SET))) { // ERROR: creating a new PDU structure
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"SET IF MAU DEFAULT TYPE: Call 'snmp_pdu_create' failed: memory error (probably out-of-memory) on creating a new "
				"PDU structure to carry out the subscription request");
		session_handler.close();
		return fixs_ith_snmp::ERR_SNMP_PDU_CREATE;
	}
	// WARNING: NET-SNMP library missed to reset the PDU variable pointer to null
	request_pdu->variables = 0;

	snmp_pdu_add_variable(request_pdu, oid_name, oid_name_size, ASN_OBJECT_ID, oid_value, oid_value_size * sizeof(oid));

	char oid_name_str [512];
	char oid_value_str [512];

	oid_to_str(oid_name_str, FIXS_ITH_ARRAY_SIZE(oid_name_str), oid_name, oid_name_size);
	oid_to_str(oid_value_str, FIXS_ITH_ARRAY_SIZE(oid_value_str), oid_value, oid_value_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "SET IF MAU DEFAULT TYPE: Sending SET request: oid_name == <%s>: oid_value == <%s>: "
			"oid_name_size == <%lu>: oid_value_size == <%lu>", oid_name_str, oid_value_str, oid_name_size, oid_value_size);

	unsigned timeout_ms = 2000; // 2 seconds timeout
	errno = 0;
	call_result = session_handler.send_synch(request_pdu, &timeout_ms); // 2 sec timeout
	int errno_save = errno;
	snmp_free_pdu(request_pdu);

	if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR,
				"SET IF MAU DEFAULT TYPE: Call 'send_synch' failed: NET-SNMP library internal error on sending the request PDU: call_result == %d", call_result);
		session_handler.close();
		return call_result;
	}

	// SNMP set request sent. Now check PDU response to get operation result
	if (logger_callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS ) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "SET IF MAU DEFAULT TYPE failed: PDU response reported an error: "
									  "callback.error_code == %d, callback.pdu_error_stat == %d", logger_callback.error_code, logger_callback.pdu_error_stat);
		call_result = (logger_callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION
								? logger_callback.pdu_error_stat
								: logger_callback.error_code);
	}

	session_handler.close();

	//return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	return call_result;
}

int
fixs_ith_snmpmanager::get_if_mau_auto_neg_admin_status (
		int & status,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms) {
	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if (get_switch_board_target_ipns(switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1],
			switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "IF MAU AUTO NEG ADMIN STATUS get info: [IF_INDEX = %d] on switch having IPNA == %s and IPNB == %s ",
			if_index, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;

	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_if_mau_auto_neg_admin_status(status, if_index, switch_board.slot_position, switch_board.magazine,
				switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"IF MAU AUTO NEG ADMIN STATUS get info: Call 'get_if_mau_auto_neg_admin_status()' failed: if_index == %d, switch board slot == %d, "
					"switch board IPN = '%s': call_result == %d", if_index, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index],
					call_result);
	}

	return call_result;
}

int
fixs_ith_snmpmanager::get_if_mau_auto_neg_admin_status (
		int & status,
		uint16_t if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms) {
	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IF MAU AUTO NEG ADMIN STATUS get info: Trying to get 'interface MAU auto neg admin status' from switch '%s'",
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine,
			FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if (call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_if_mau_auto_neg_admin_status_callback callback(status, if_index, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU AUTO NEG ADMIN STATUS get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_if_mau_auto_neg_admin_status [] = { IF_MAU_AUTO_NEG_ADMIN_STATUS, if_index, 1 };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_if_mau_auto_neg_admin_status; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_if_mau_auto_neg_admin_status);

	call_result = get_if_mau_auto_neg_admin_status(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}

int
fixs_ith_snmpmanager::get_if_mau_auto_neg_admin_status (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {
	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IF MAU AUTO NEG ADMIN STATUS get info: Sending the PDU GET request to retrieve the 'if MAU "
			"auto neg admin status' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU AUTO NEG ADMIN STATUS get info: Timeout expired while sending the PDU GET request to retrieve "
				"'if MAU auto neg admin status'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"IF MAU AUTO NEG ADMIN STATUS get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve "
				"'if Mau auto neg admin status': call_result == %d", call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "IF MAU AUTO NEG ADMIN STATUS get info: Callback detected error on reading 'if MAU auto neg admin status' "
				"from response PDU: callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

int
fixs_ith_snmpmanager::set_if_mau_auto_neg_admin_status (
		fixs_ith_snmp::if_Mau_Auto_Neg_Admin_States status,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane) {
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if (get_switch_board_target_ipns(switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Set IF MAU AUTO NEG ADMIN STATUS: on SMX having IPNA == %s and IPNB == %s ",
			switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = set_if_mau_auto_neg_admin_status(status, if_index, switch_board_ipn_str_addresses[ipn_index]);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"SET IF MAU AUTO NEG ADIN STATUS: Call 'set_if_mau_auto_neg_admin_status' failed: switch board slot == %d, switch board IPN = '%s': "
					"call_result == %d", switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int
fixs_ith_snmpmanager::set_if_mau_auto_neg_admin_status (
		fixs_ith_snmp::if_Mau_Auto_Neg_Admin_States status,
		uint16_t if_index,
		const char * switch_board_ip) {
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = 0;

	// Prepare the oid arrays
	oid oid_name [] = { IF_MAU_AUTO_NEG_ADMIN_STATUS, if_index, 1 };
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	call_result = set_if_mau_auto_neg_admin_status(oid_name, oid_name_size, status, switch_board_ip);

	return call_result;
}

int
fixs_ith_snmpmanager::set_if_mau_auto_neg_admin_status (
		oid * oid_name,
		size_t oid_name_size,
		fixs_ith_snmp::if_Mau_Auto_Neg_Admin_States status,
		const char * switch_board_ip) {
	fixs_ith_snmpsessioninfo session_info;

	session_info.version(SNMP_VERSION_2c);
	session_info.community(const_cast<char *>(ITH_SNMP_COMMUNITY_RW_STRING));

	// snmp session communication handler used to send request and the callback used to receive the ACK response
	fixs_ith_snmpsessionhandler session_handler;
	fixs_ith_snmploggercallback logger_callback("SET IF MAU AUTO NEG STATUS: SNMP Response received for PORT configuration");

	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"SET IF MAU AUTO NEG STATUS: sending request to switch board IP %s", switch_board_ip);

	session_info.peername(const_cast<char *>(switch_board_ip));
	logger_callback.switch_board_ip(switch_board_ip);

	int call_result = 0;

	if ((call_result = session_handler.open(session_info, logger_callback))) { // ERROR: opening the communication session towards the switch board
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"SET IF MAU AUTO NEG STATUS: Call 'open()' failed: cannot open the communication session towards the above switch board: "
				"system error == %d, snmp error == %d, snmp error text == '%s': call_result == %d",
				session_info.last_sys_error(), session_info.last_snmp_error(), session_info.last_snmp_error_text(), call_result);
		return fixs_ith_snmp::ERR_SNMP_SESSION_OPEN;
	}

	// Prepare the PDU subscription request ITH will send to the current switch board
	netsnmp_pdu * request_pdu = 0;

	if (!(request_pdu = snmp_pdu_create(SNMP_MSG_SET))) { // ERROR: creating a new PDU structure
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"SET IF MAU AUTO NEG STATUS: Call 'snmp_pdu_create' failed: memory error (probably out-of-memory) on creating a new "
				"PDU structure to carry out the subscription request");
		session_handler.close();
		return fixs_ith_snmp::ERR_SNMP_PDU_CREATE;
	}
	// WARNING: NET-SNMP library missed to reset the PDU variable pointer to null
	request_pdu->variables = 0;

	snmp_pdu_add_variable(request_pdu, oid_name, oid_name_size, ASN_INTEGER, &status, sizeof(status));

	char oid_name_str [512];

	oid_to_str(oid_name_str, FIXS_ITH_ARRAY_SIZE(oid_name_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "SET IF MAU AUTO NEG STATUS: Sending SET request: oid_name == <%s>: oid_value == <%d>: "
			"oid_name_size == <%lu>: oid_value_size == <%lu>", oid_name_str, status, oid_name_size, sizeof(status));

	unsigned timeout_ms = 2000; // 2 seconds timeout
	errno = 0;
	call_result = session_handler.send_synch(request_pdu, &timeout_ms); // 2 sec timeout
	int errno_save = errno;
	snmp_free_pdu(request_pdu);

	if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR,
				"SET IF MAU AUTO NEG STATUS: Call 'send_synch' failed: NET-SNMP library internal error on sending the request PDU: call_result == %d",
				call_result);
		session_handler.close();
		return call_result;
	}

	// SNMP set request sent. Now check PDU response to get operation result
	if (logger_callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS ) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "SET IF MAU AUTO NEG STATUS failed: PDU response reported an error: "
									  "callback.error_code == %d, callback.pdu_error_stat == %d", logger_callback.error_code, logger_callback.pdu_error_stat);
		call_result = (logger_callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION
								? logger_callback.pdu_error_stat
								: logger_callback.error_code);
	}

	session_handler.close();

	//return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	return call_result;
}


//int fixs_ith_snmpmanager::get_my_switch_board_info (std::vector<fixs_ith_switchboardinfo> & board_infos) {
//	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
//
//	// Load the switch boards info from configuration
//	if (const int call_result = load_switch_boards_info_from_cs()) // ERROR: loading switch board info
//		return call_result;
//
//	if (const int call_result = no_my_switch_board_found_check()) return call_result;
//
//	if (_my_switch_board_infos.size()) board_infos.assign(_my_switch_board_infos.begin(), _my_switch_board_infos.end());
//	else board_infos.clear();
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//} //+

//int fixs_ith_snmpmanager::get_other_switch_board_info (std::vector<fixs_ith_switchboardinfo> & board_infos) {
//	if (_other_switch_board_infos.size()) board_infos.assign(_other_switch_board_infos.begin(), _other_switch_board_infos.end());
//	else board_infos.clear();
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

//int fixs_ith_snmpmanager::get_all_switch_board_info (std::vector<fixs_ith_switchboardinfo> & board_infos) {
//	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
//
//	if (_other_switch_board_infos.size()) board_infos.assign(_other_switch_board_infos.begin(), _other_switch_board_infos.end());
//	else board_infos.clear();
//
//	if (_my_switch_board_infos.size()) board_infos.insert(board_infos.begin(), _my_switch_board_infos.begin(), _my_switch_board_infos.end());
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//} //-

//int fixs_ith_snmpmanager::get_switch_boards_info (std::vector<fixs_ith_switchboardinfo> & board_infos, uint32_t magazine) {
//	std::vector<fixs_ith_switchboardinfo> tmp_board_infos;
//
//	get_all_switch_board_info(tmp_board_infos);
//
//	board_infos.clear();
//
//	for (size_t i = 0; i < tmp_board_infos.size(); ++i)
//		if (tmp_board_infos[i].magazine == magazine)
//			board_infos.push_back(tmp_board_infos[i]);
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}


int fixs_ith_snmpmanager::map_net_snmp_error_stat (long net_snmp_error_stat) {
	int error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERROR_UNDEFINED;

	switch (net_snmp_error_stat) {
	case SNMP_ERR_NOERROR: error_code = fixs_ith_snmp::ERR_SNMP_NO_ERRORS; break;
	case SNMP_ERR_TOOBIG: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_TOOBIG; break;
	case SNMP_ERR_NOSUCHNAME: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_NOSUCHNAME; break;
	case SNMP_ERR_BADVALUE: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_BADVALUE; break;
	case SNMP_ERR_READONLY: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_READONLY; break;
	case SNMP_ERR_GENERR: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_GENERR; break;
	case SNMP_ERR_NOACCESS: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_NOACCESS; break;
	case SNMP_ERR_WRONGTYPE: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_WRONGTYPE; break;
	case SNMP_ERR_WRONGLENGTH: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_WRONGLENGTH; break;
	case SNMP_ERR_WRONGENCODING: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_WRONGENCODING; break;
	case SNMP_ERR_WRONGVALUE: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_WRONGVALUE; break;
	case SNMP_ERR_NOCREATION: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_NOCREATION; break;
	case SNMP_ERR_INCONSISTENTVALUE: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_INCONSISTENTVALUE; break;
	case SNMP_ERR_RESOURCEUNAVAILABLE: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_RESOURCEUNAVAILABLE; break;
	case SNMP_ERR_COMMITFAILED: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_COMMITFAILED; break;
	case SNMP_ERR_UNDOFAILED: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_UNDOFAILED; break;
	case SNMP_ERR_AUTHORIZATIONERROR: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_AUTHORIZATIONERROR; break;
	case SNMP_ERR_NOTWRITABLE: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_NOTWRITABLE; break;
	case SNMP_ERR_INCONSISTENTNAME: error_code = fixs_ith_snmp::ERR_SNMP_NETSNMP_ERR_INCONSISTENTNAME; break;
	}

	return error_code;
}

//int fixs_ith_snmpmanager::load_my_switch_boards_info_from_cs (bool force) {
//	static int info_loaded = 0;
//
//	if (info_loaded && !force) return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//
//	std::vector <fixs_ith_switchboardinfo> boards;
//
//	if (const int cs_call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().get_my_switch_boards_info(boards)) {
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
//				"Call 'get_my_switch_boards_info' failed: cannot get the switch board info in my magazine: cs_call_result == %d", cs_call_result);
//		return cs_call_result;
//	}
//
//	if (boards.size() <= 0) { // ERROR: No switch board found in the CS configuration
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "No switch board found in my magazine from CS configuration");
//		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
//	}
//
//	if (boards.size() > fixs_ith_snmp::SWITCH_BOARD_COUNT_PER_MAGAZINE) { // ERROR: Too many switch board in magazine
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
//				"Too many switch board found in my magazine from CS configuration: there are %uz switch board in my magazine but them should be at most %d",
//				boards.size(), fixs_ith_snmp::SWITCH_BOARD_COUNT_PER_MAGAZINE);
//		return fixs_ith::ERR_CS_TOO_MANY_BOARD_FOUND;
//	}
//
//	// Sort the vector by slot position
//	std::sort(boards.begin(), boards.end(), fx_compare_slot_ascending);
//
//	_my_switch_board_infos.assign(boards.begin(), boards.end());
//
//	info_loaded = 1;
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

//int fixs_ith_snmpmanager::load_other_switch_boards_info_from_cs (bool force) {
//	static int info_loaded = 0;
//
//	if (info_loaded && !force) return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//
//	std::vector <fixs_ith_switchboardinfo> boards;
//
//	if (const int cs_call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().get_other_switch_boards_info(boards)) {
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
//				"Call 'get_other_switch_boards_info' failed: cannot get the switch board info in other magazines: cs_call_result == %d", cs_call_result);
//		return cs_call_result;
//	}
//
//	if (boards.size() <= 0) { // ERROR: No switch board found in the CS configuration
//		FIXS_ITH_LOG(LOG_LEVEL_WARN, "No switch board found in other magazines from CS configuration");
//		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
//	}
//
//	// Sort the vector by slot position
//	std::sort(boards.begin(), boards.end(), fx_compare_magazine_slot_ascending);
//
//	_other_switch_board_infos.assign(boards.begin(), boards.end());
//
//	info_loaded = 1;
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

//int fixs_ith_snmpmanager::load_switch_boards_info_from_cs (bool force) {
//	load_my_switch_boards_info_from_cs(force);
//
//	load_other_switch_boards_info_from_cs(force);
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

//int fixs_ith_snmpmanager::load_switch_boards_info_from_agents(bool force)
//{
//
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

//int fixs_ith_snmpmanager::no_my_switch_board_found_check () const {
//	if (_my_switch_board_infos.size() <= 0) { // ERROR: No switch board found in the CS configuration
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "No switch board found in my magazine from CS configuration");
//		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
//	}
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

//int fixs_ith_snmpmanager::no_other_switch_board_found_check () const {
//	if (_other_switch_board_infos.size() <= 0) { // ERROR: No switch board found in the CS configuration
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "No switch board found in other magazines from CS configuration");
//		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
//	}
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

int fixs_ith_snmpmanager::send_get_request (
		fixs_ith_snmpsessionhandler & session_handler,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {
	netsnmp_pdu * snmp_pdu = 0;

	// Create the PDU for GET
	if (const int call_result = create_snmp_pdu(snmp_pdu, SNMP_MSG_GET)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_snmp_pdu' failed: call_result == %d", call_result);
		return call_result;
	}

	// Add the OID into the PDU request
	if (const int call_result = add_snmp_null_var(snmp_pdu, oid_name, oid_name_size)) { // ERROR: adding the OID
		snmp_free_pdu(snmp_pdu);
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'add_snmp_null_var' failed: call_result == %d", call_result);
		return call_result;
	}

	{
		errno=0;
		int call_result = session_handler.send_synch(snmp_pdu, timeout_ms);
		int errno_save = errno;
		snmp_free_pdu(snmp_pdu);

		if (call_result) {
			char oid_str [512];
			oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Call 'send_synch' failed: cannot send the GET pdu request for OID '%s'", oid_str);
		}

		return call_result;
	}
}

int fixs_ith_snmpmanager::send_set_request (
		fixs_ith_snmpsessionhandler & session_handler,
		const oid * oid_name,
		size_t oid_name_size,
		const char * oid_value,
		const unsigned * timeout_ms) {
	netsnmp_pdu * snmp_pdu = 0;

	// Create the PDU for GET
	if (const int call_result = create_snmp_pdu(snmp_pdu, SNMP_MSG_SET)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_snmp_pdu' failed: call_result == %d", call_result);
		return call_result;
	}

	// Add the OID name and value into the PDU request
	if (!snmp_pdu_add_variable(snmp_pdu, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(oid_value), ::strlen(oid_value))) {
		snmp_free_pdu(snmp_pdu);
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'snmp_pdu_add_variable' failed in NET-SNMP library");
		return fixs_ith_snmp::ERR_SNMP_PDU_ADD_VARIABLE_VALUE;
	}

	// Sending the SET request
	{
		errno=0;
		int call_result = session_handler.send_synch(snmp_pdu, timeout_ms);
		int errno_save = errno;
		snmp_free_pdu(snmp_pdu);

		if (call_result) {
			char oid_str [512];
			oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Call 'send_synch' failed: cannot send the SET pdu request for OID '%s'", oid_str);
		}

		return call_result;
	}
}

int fixs_ith_snmpmanager::send_set_request (
		fixs_ith_snmpsessionhandler & session_handler,
		const oid * oid_name,
		size_t oid_name_size,
		int oid_value,
		const unsigned * timeout_ms) {
	netsnmp_pdu * snmp_pdu = 0;

	// Create the PDU for GET
	if (const int call_result = create_snmp_pdu(snmp_pdu, SNMP_MSG_SET)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_snmp_pdu' failed: call_result == %d", call_result);
		return call_result;
	}

	// Add the OID name and value into the PDU request
	if (!snmp_pdu_add_variable(snmp_pdu, oid_name, oid_name_size, ASN_INTEGER, reinterpret_cast<const u_char *>(&oid_value), sizeof(oid_value))) {
		snmp_free_pdu(snmp_pdu);
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'snmp_pdu_add_variable' failed in NET-SNMP library");
		return fixs_ith_snmp::ERR_SNMP_PDU_ADD_VARIABLE_VALUE;
	}

	// Sending the SET request
	{
		errno=0;
		int call_result = session_handler.send_synch(snmp_pdu, timeout_ms);
		int errno_save = errno;
		snmp_free_pdu(snmp_pdu);

		if (call_result) {
			char oid_str [512];
			oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Call 'send_synch' failed: cannot send the SET pdu request for OID '%s'", oid_str);
		}

		return call_result;
	}
}

int fixs_ith_snmpmanager::send_set_request (
		fixs_ith_snmpsessionhandler & session_handler,
		const oid * oid_name,
		size_t oid_name_size,
		u_char asn1_val_type,
		const u_char * value,
		size_t value_size,
		const unsigned * timeout_ms) {
	netsnmp_pdu * snmp_pdu = 0;

	// Create the PDU for GET
	if (const int call_result = create_snmp_pdu(snmp_pdu, SNMP_MSG_SET)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_snmp_pdu' failed: call_result == %d", call_result);
		return call_result;
	}

	// Add the OID name and value into the PDU request
	if (!snmp_pdu_add_variable(snmp_pdu, oid_name, oid_name_size, asn1_val_type, value, value_size)) {
		snmp_free_pdu(snmp_pdu);
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'snmp_pdu_add_variable' failed in NET-SNMP library");
		return fixs_ith_snmp::ERR_SNMP_PDU_ADD_VARIABLE_VALUE;
	}

	// Sending the SET request
	{
		errno=0;
		int call_result = session_handler.send_synch(snmp_pdu, timeout_ms);
		int errno_save = errno;
		snmp_free_pdu(snmp_pdu);

		if (call_result) {
			char oid_str [512];
			oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Call 'send_synch' failed: cannot send the SET pdu request for OID '%s'", oid_str);
		}

		return call_result;
	}
}


// NEW METHOD

int fixs_ith_snmpmanager::open_snmp_session_handler (fixs_ith_snmpsessionhandler & session_handler, fixs_ith_snmpsessioncallback & callback, const char * switch_board_ip) {
	fixs_ith_snmpsessioninfo session_info;
	session_info.version(SNMP_VERSION_2c);
	session_info.community(const_cast<char *>(ITH_SNMP_COMMUNITY_RW_STRING));
	session_info.peername(const_cast<char *>(switch_board_ip));

	int call_result = 0;

	if ((call_result = session_handler.open(session_info, callback))) { // ERROR: opening the communication session towards the switch board
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open' failed: cannot open the communication session towards the SNMP agent at address '%s': "
				"system error == %d, snmp error == %d, snmp error text == '%s': call_result == %d",
				switch_board_ip, session_info.last_sys_error(), session_info.last_snmp_error(), session_info.last_snmp_error_text(), call_result);

		return fixs_ith_snmp::ERR_SNMP_SESSION_OPEN;
	}

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_snmpmanager::create_snmp_pdu (netsnmp_pdu * (& snmp_pdu), int command) {
	netsnmp_pdu * pdu = 0;

	if (!(pdu = snmp_pdu_create(command))) { // ERROR: creating a new PDU structure
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'snmp_pdu_create' failed: memory error (probably out-of-memory) on creating a new PDU structure: command == %d", command);

		return fixs_ith_snmp::ERR_SNMP_PDU_CREATE;
	}
	// WARNING: NET-SNMP library missed to reset the PDU variable pointer to null
	pdu->variables = 0;

	snmp_pdu = pdu;

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_snmpmanager::add_snmp_null_var (netsnmp_pdu * pdu, const oid * name, size_t name_size) {
	if (!snmp_add_null_var(pdu, name, name_size)) { // ERROR: adding the requested data the PDU
		char name_str [1024] = {0};

		fixs_ith_snmpmanager::oid_to_str(name_str, FIXS_ITH_ARRAY_SIZE(name_str), name, name_size);

		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'snmp_add_null_var' failed: cannot add the requested OID '%s' to the PDU request", name_str);

		return fixs_ith_snmp::ERR_SNMP_PDU_ADD_NULL_VARIABLE;
	}

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

//int fixs_ith_snmpmanager::get_switch_board_info_ptr_by_ip(fixs_ith_switchboardinfo * & p_switch_board_info_ptr, bool & scb_is_in_local_magazine, const char * switch_board_ip)
//{
//	if(!switch_board_ip || !*switch_board_ip)
//		return fixs_ith::ERR_NULL_POINTER;
//
//	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
//
//	// search first in own magazine info
//	bool scb_found = false;
//	for(std::vector<fixs_ith_switchboardinfo>::iterator it = _my_switch_board_infos.begin(); !scb_found && it != _my_switch_board_infos.end(); ++ it)
//	{
//		if( !strncmp(switch_board_ip, it->control_ipna_str, FIXS_ITH_ARRAY_SIZE(it->control_ipnb_str)) || !strncmp(switch_board_ip, it->control_ipnb_str, FIXS_ITH_ARRAY_SIZE(it->control_ipnb_str)))
//		{
//			p_switch_board_info_ptr = & (*it);
//			scb_found = true;
//			scb_is_in_local_magazine = true;
//		}
//	}
//
//	// ... and then search in other magazine info
//
//	for(std::vector<fixs_ith_switchboardinfo>::iterator it = _other_switch_board_infos.begin(); !scb_found && it != _other_switch_board_infos.end(); ++ it)
//	{
//		if( !strncmp(switch_board_ip, it->control_ipna_str, FIXS_ITH_ARRAY_SIZE(it->control_ipna_str)) || !strncmp(switch_board_ip, it->control_ipnb_str, FIXS_ITH_ARRAY_SIZE(it->control_ipnb_str)))
//		{
//			p_switch_board_info_ptr = & (*it);
//			scb_found = true;
//			scb_is_in_local_magazine = false;
//		}
//	}
//
//
//	if(!scb_found)
//	{
//		FIXS_ITH_LOG(LOG_LEVEL_WARN, "No SCB found having IP == '%s'", switch_board_ip);
//		return fixs_ith::ERR_CS_BOARD_NOT_FOUND;
//	}
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

int fixs_ith_snmpmanager::oid_to_str (char * buf, size_t size, const oid * id, size_t id_length) {
	if (!buf) return fixs_ith_snmp::ERR_SNMP_NULL_PARAMETER;

	*buf = 0;

	if (id)
		for (size_t chars_wrote = 0; id_length-- && (size > 1); buf += chars_wrote, size -= chars_wrote)
			if ((chars_wrote = ::snprintf(buf, size, ".%lu", *id++)) >= size) return fixs_ith_snmp::ERR_SNMP_NOT_ENOUGH_SPACE;

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_snmpmanager::oid_compare (const oid * id1, size_t id1_length, const oid * id2, size_t id2_length) {
	if (id1) { if (!id2) return 1;}
	else return id2 ? -1: 0; // id1 is null

	// Here id1 and id2 are both not null.
	size_t length = (id1_length < id2_length ? id1_length : id2_length) + 1;

	while (--length && (*id1++ == *id2++)) ;

	return (length ? ((*(id1 - 1) < *(id2 - 1)) ? -1 : 1) : 0);
}


// *BASIC_TEST* --- ONLY for basic test:  TO BE deleted!!!!!!!!!!!!!!!!!
int fixs_ith_snmpmanager::test_port_operations(fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t sb_plane ) {

	//TODO: must be checked. Error conditions must be handled and must be added the neighbour presence handling
	int result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST for getting PORT INFOs on SMX in magazine == 0x%08X, slot == %d plane == %s - starting ...", switch_board.magazine, switch_board.slot_position,
			                                                           (sb_plane == fixs_ith::CONTROL_PLANE)? "CONTROL":"TRANSPORT");

	// input parameters
	uint16_t if_index = 6;   //BP_2 on Transport - BP_6 on CONTROL
//	const char * port_name = 0;
	const unsigned timeout_ms = 10000;

	//output parameters
	fixs_ith_snmp::qBridge::pvid_t pvid;
	int  num_of_ports; uint32_t speed;
	uint32_t  eth_frame_size, num_of_vlans;

//	fixs_ith_snmp::mac_address_t port_macaddr;
	fixs_ith_snmp::mac_address_t bridge_address;
	fixs_ith_snmp::port_status_t  port_status;


	if((result = get_port_operational_status (port_status, if_index, switch_board, sb_plane, &timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_port_operational_status() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_port_operational_status() : port_status == %d", port_status);

	if((result = get_port_admin_status (port_status,if_index,switch_board, sb_plane, &timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_port_admin_status() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_port_admin_status() : port_status == %d", port_status);



	if((result =  get_port_default_vlan(pvid, if_index, switch_board, sb_plane, &timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_port_default_vlan() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_port_default_vlan() : pvid == %u", pvid);

	if((result =  get_number_of_ports( num_of_ports, switch_board, sb_plane, &timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_number_of_ports() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_number_of_ports() : num_of_ports == %d", num_of_ports);

	if((result =  get_eth_max_frame_size (eth_frame_size, if_index, switch_board, sb_plane,&timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_eth_max_frame_size() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_eth_max_frame_size() : eth_frame_size == %u", eth_frame_size);

	if((result =  get_interface_speed (speed, if_index, switch_board, sb_plane, &timeout_ms )))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_interface_speed() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_interface_speed() : speed == %u", speed);;

	//result =  get_port_physical_address( port_macaddr,port_name, switch_board, sb_plane, &timeout_ms);


	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST for get bridge infos .....  starting");

	if((result =  get_bridge_address (bridge_address, switch_board, sb_plane, &timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_bridge_address() failed: result == %d ", result);
	else{
		for(int i=0; i < fixs_ith_snmp::SNMP_MACADDRESS_FIELD_SIZE; i++)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_bridge_address() : bridge_address[%d] == %2x", i, bridge_address[i]);
	}

	if((result =  get_number_of_vlans (num_of_vlans, switch_board, sb_plane, &timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_number_of_vlans() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_number_of_vlans() : num_of_vlans == %u", num_of_vlans);

	return result;
}

int fixs_ith_snmpmanager::test_lag_operations(fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t sb_plane )
{
	//TODO: must be checked. Error conditions must be handled and must be added the neighbour presence handling
	int result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST for getting LAG INFOs on SMX in magazine == 0x%08X, slot == %d plane == %s - starting ...", switch_board.magazine, switch_board.slot_position,
			(sb_plane == fixs_ith::CONTROL_PLANE)? "CONTROL":"TRANSPORT");

	// input parameters
	uint16_t lag_if_index;

	const unsigned timeout_ms = 10000;


// SNMP operations to configure a LAG
	lag_if_index = 201;
	const char * lag_name = " LAG_01";
	if((result = create_lag(switch_board, lag_if_index, lag_name, sb_plane)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- create_lag() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- create_lag() : lag_if_index == %d lag_name == %s",
				lag_if_index, lag_name);

	std::set<uint16_t> if_list;
	if_list.insert(166); // if_index 166  --> B2_5-8
	if_list.insert(136); // if_index 166  --> B2_9-12
	// enable interfaces before to add them as aggregation members of LAG

	if((result = set_port_admin_status(switch_board,136,fixs_ith_snmp::PORT_STATUS_UP,sb_plane)))
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_port_admin_status failed: result == %d ", result);
		else
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_port_admin_status() : administrative status of interface 136 is set to UP");

	if((result = set_port_admin_status(switch_board,166,fixs_ith_snmp::PORT_STATUS_UP,sb_plane)))
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_port_admin_status failed: result == %d ", result);
		else
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_port_admin_status() : administrative status of interface 166 is set to UP");

	if((result = add_lag_ports(switch_board,lag_if_index, if_list, sb_plane)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- add_lag_ports() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- add_lag_ports() : lag_if_index == %d lag_name == %s",
				lag_if_index, lag_name);

	// to get the ports that are currently members of a LAG
	//output parameters
	fixs_ith_snmp::if_bitmap_t port_list = {0};

	if((result =  get_lag_operMembers( port_list, lag_if_index, switch_board, sb_plane, &timeout_ms )))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_lag_operMembers() failed: result == %d ", result);
	else
	{
		for(int i=0; i < SWITCH_IF_BITMAP_SIZE; i++)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_lag_operMembers() : port_list[%d] == %2x", i, port_list[i]);
	}

	fixs_ith_snmp::port_status_t lag_status = fixs_ith_snmp::PORT_STATUS_UP ;
	if((result = set_port_admin_status(switch_board, lag_if_index, lag_status, sb_plane)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_port_admin_status() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_port_admin_status() : lag_status == %2x",lag_status);

	return result;

}

int fixs_ith_snmpmanager::test_vlan_operations(fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t sb_plane )
{
	//TODO: must be checked. Error conditions must be handled and must be added the neighbour presence handling
	int result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST for VLAN operations on SMX in magazine == 0x%08X, slot == %d plane == %s - starting ...", switch_board.magazine, switch_board.slot_position,
			(sb_plane == fixs_ith::CONTROL_PLANE)? "CONTROL":"TRANSPORT");

	// input parameters
	int32_t  vlan_id;
	const char * vlan_name = "VLAN_TEST";
	const unsigned timeout_ms = 10000;

	fixs_ith_snmp::vlanPort_type_t port_type = fixs_ith_snmp::TAGGED;
	fixs_ith_snmp::if_bitmap_t port_list = {0};
	vlan_id=200;
	if((result = get_vlan_ports( port_list,vlan_id, port_type, switch_board, sb_plane, &timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - get_vlan_ports() failed: result == %d ", result);
	else
	{
		for(int i=0; i < SWITCH_IF_BITMAP_SIZE; i++)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_vlan_ports()  : port_list[%d] == %02x", i, port_list[i]);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - get_vlan_ports() : vlan_id == %d successful executed!", vlan_id);
	}

	// delete old definition
	vlan_id=1000;
	if((result = delete_vlan( switch_board,vlan_id, sb_plane)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - delete_vlan() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - delete_vlan() : vlan_id == %d successful executed!", vlan_id);

	// create vlan
	if((result =  create_vlan(switch_board, vlan_id, vlan_name, sb_plane)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- create_vlan() failed: result == %d ", result);
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- create_vlan() : vlan_id == %d vlan_name == %s successful executed!", vlan_id, vlan_name);


	std::set<uint16_t> tagged_ports;
	tagged_ports.insert(26);
	tagged_ports.insert(36);

	std::set<uint16_t> untagged_ports;
	untagged_ports.insert(32);
	untagged_ports.insert(48);

	if((result = set_vlan_ports(switch_board, vlan_id,  tagged_ports, untagged_ports, sb_plane)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_vlan_ports() failed: result == %d ", result);
	else
	{
		for(std::set<uint16_t>::iterator it = tagged_ports.begin();  it != tagged_ports.end(); ++it)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_vlan_ports()  : tagged_ports[] == %u",  *it);

		for(std::set<uint16_t>::iterator it = untagged_ports.begin();  it != untagged_ports.end(); ++it)
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_vlan_ports()  : untagged_ports[] == %u",  *it);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- set_vlan_ports() successful executed!");
	}

	//fixs_ith_snmp::if_bitmap_t port_list;
	if((result = get_vlan_ports( port_list,vlan_id, port_type, switch_board, sb_plane, &timeout_ms)))
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - get_vlan_ports() failed: result == %d ", result);
	else
	{
		if(common::utility::isSwitchBoardCMX())
		{
			for(int i=0; i < SWITCH_IF_BITMAP_SIZE_CMX; i++)
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_vlan_ports()  : port_list[%d] == %02x", i, port_list[i]);
		}

		if(common::utility::isSwitchBoardSMX())
		{
			for(int i=0; i < SWITCH_IF_BITMAP_SIZE; i++)
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST- get_vlan_ports()  : port_list[%d] == %02x", i, port_list[i]);
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - get_vlan_ports() : vlan_id == %d successful executed!", vlan_id);
	}


//	if((result = remove_vlan_ports(switch_board, vlan_id, port_type, sb_plane)))
//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - remove_vlan_ports() failed: result == %d ", result);
//	else
//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - remove_vlan_ports() : vlan_id == %d successful executed!", vlan_id);
//
//
//	if((result = delete_vlan( switch_board,vlan_id, sb_plane)))
//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - delete_vlan() failed: result == %d ", result);
//	else
//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BASIC TEST - delete_vlan() : vlan_id == %d successful executed!", vlan_id);


	return result;

}

// *BASIC_TEST* --- ONLY for basic test:  END /////




template <>
int fixs_ith_snmpmanager::set_mib_object (fixs_ith_snmpsessionhandler & session_handler, oid * oid_name ,size_t oid_size,char * oidValue)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char oid_str [512];

	netsnmp_pdu * snmp_pdu = 0;

	// Create the PDU for GET
	if (const int call_result = create_snmp_pdu(snmp_pdu, SNMP_MSG_SET)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_snmp_pdu' failed: call_result == %d", call_result);
		return call_result;
	}
	// Add the OID name and value into the PDU request
	netsnmp_variable_list * snmp_result = 0;

	snmp_result = snmp_pdu_add_variable(snmp_pdu, oid_name, oid_size,ASN_OCTET_STR, reinterpret_cast<u_char *>(oidValue),::strlen((const char*)oidValue));
	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_size);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "set_mib_object: Sending SET (oid: %s)(value = %s) request...",oid_str, oidValue);

	if (!snmp_result) {
		snmp_free_pdu(snmp_pdu);
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'snmp_pdu_add_variable' failed in NET-SNMP library");
		return fixs_ith_snmp::ERR_SNMP_PDU_ADD_VARIABLE_VALUE;
	}

	{
		unsigned timeout_ms = 2000; // 2 seconds timeout
		errno=0;
		int call_result = session_handler.send_synch(snmp_pdu, &timeout_ms); // 2 sec timeout
		int errno_save = errno;
		snmp_free_pdu(snmp_pdu);

		if (call_result) { // ERROR: sending the request
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR,
					"LAG configure: Call 'send_synch' failed: NET-SNMP library internal error on sending the request PDU: call_result == %d", call_result);
			return fixs_ith_snmp::ERR_SNMP_PDU_CREATE;
		}

		return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	}
}



//int fixs_ith_snmpmanager::switch_board_delete (uint32_t magazine, int32_t slot_pos) {
//	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
//
//	// BEGIN: DEBUGGING and TRACING: TO BE DELETE
//	FIXS_ITH_LOG(LOG_LEVEL_TRACE, "The switch board being deleted is: magazine == %u, slot_position == %d",
//			magazine, slot_pos);
//
//	FIXS_ITH_LOG(LOG_LEVEL_TRACE, "Dumping _my_switch_board_infos internal vector before deleting a SwitchBoard: _my_switch_board_infos.size() == %zu", _my_switch_board_infos.size());
//
//	for (size_t index = 0; index < _my_switch_board_infos.size(); ++index) {
//		fixs_ith_switchboardinfo & sbi_item = _my_switch_board_infos[index];
//
//		FIXS_ITH_LOG(LOG_LEVEL_TRACE, "MY SWITCH_BOARD_INFO[%zu] --> magazine == %u, slot_position == %d, IPN-A == '%s', IPN-B == '%s'",
//				index, sbi_item.magazine, sbi_item.slot_position, sbi_item.control_ipna_str, sbi_item.control_ipnb_str);
//	}
//	// END: DEBUGGING and TRACING: TO BE DELETE
//
//	// Searching and deleting element in _my_switch_board_infos vector
//	std::vector<fixs_ith_switchboardinfo>::size_type old_size = _my_switch_board_infos.size();
//	std::vector<fixs_ith_switchboardinfo>::iterator new_end = std::remove_if(_my_switch_board_infos.begin(), _my_switch_board_infos.end(), match_sb_magazine_slot(magazine, slot_pos));
//	_my_switch_board_infos.erase(new_end, _my_switch_board_infos.end());
//	std::vector<fixs_ith_switchboardinfo>::size_type new_size = _my_switch_board_infos.size();
//
//	if (old_size == new_size) // No elements found and deleted
//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "No switch boards deleted from internal MY-SWITCH-BOARDS vector");
//	else FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Found and deleted %zu switch board(s) from MY-SWITCH-BOARDS vector", (old_size - new_size));
//
//	// BEGIN: DEBUGGING and TRACING: TO BE DELETE
//		FIXS_ITH_LOG(LOG_LEVEL_TRACE, "Dumping _other_switch_board_infos internal vector before deleting a SwitchBoard: _other_switch_board_infos.size() == %zu", _other_switch_board_infos.size());
//
//		for (size_t index = 0; index < _other_switch_board_infos.size(); ++index) {
//			fixs_ith_switchboardinfo & sbi_item = _other_switch_board_infos[index];
//
//			FIXS_ITH_LOG(LOG_LEVEL_TRACE, "OTHER SWITCH_BOARD_INFO[%zu] --> magazine == %u, slot_position == %d, IPN-A == '%s', IPN-B == '%s'",
//					index, sbi_item.magazine, sbi_item.slot_position, sbi_item.control_ipna_str, sbi_item.control_ipnb_str);
//		}
//		// END: DEBUGGING and TRACING: TO BE DELETE
//
//	// Searching and deleting element in _other_switch_board_infos vector
//	old_size = _other_switch_board_infos.size();
//	new_end = std::remove_if(_other_switch_board_infos.begin(), _other_switch_board_infos.end(), match_sb_magazine_slot(magazine, slot_pos));
//	_other_switch_board_infos.erase(new_end, _other_switch_board_infos.end());
//	new_size = _other_switch_board_infos.size();
//
//	if (old_size == new_size) // No elements found and deleted
//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "No switch boards deleted from internal OTHER-SWITCH-BOARDS vector");
//	else FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Found and deleted %zu switch board(s) from OTHER-SWITCH-BOARDS vector", (old_size - new_size));
//
//	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//}

//int fixs_ith_snmpmanager::switch_board_add (fixs_ith_switchboardinfo & new_switch_board) {
//	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
//
//	// BEGIN: DEBUGGING and TRACING: TO BE DELETE
//		FIXS_ITH_LOG(LOG_LEVEL_TRACE, "The new switch board being added is: magazine == %u, slot_position == %d, IPN-A == '%s', IPN-B == '%s'",
//				new_switch_board.magazine, new_switch_board.slot_position, new_switch_board.control_ipna_str, new_switch_board.control_ipnb_str);
//
//		FIXS_ITH_LOG(LOG_LEVEL_TRACE, "Dumping _my_switch_board_infos internal vector before adding a new SwitchBoard: _my_switch_board_infos.size() == %zu", _my_switch_board_infos.size());

//		for (size_t index = 0; index < _my_switch_board_infos.size(); ++index) {
//			fixs_ith_switchboardinfo & sbi_item = _my_switch_board_infos[index];
//
//			FIXS_ITH_LOG(LOG_LEVEL_TRACE, "MY SWITCH_BOARD_INFO[%zu] --> magazine == %u, slot_position == %d, IPN-A == '%s', IPN-B == '%s'",
//					index, sbi_item.magazine, sbi_item.slot_position, sbi_item.ipna_str, sbi_item.ipnb_str);
//		}
//		// END: DEBUGGING and TRACING: TO BE DELETE
//
//	uint32_t my_magazine = ~0;
//
//	if (const int call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().get_my_magazine(my_magazine)) {
//			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_my_magazine' failed: erro_ getting my magazine address from CS: call_result == %d", call_result);
//			return call_result;
//	}
//
//	if(new_switch_board.magazine == my_magazine){
//			char myMagazine_str [16] = {0};
//			int call_result = -1;
//			for (int i = 0; (call_result && i < 3); ++i) {
//				call_result = fixs_ith_csreader::uint32_to_ip_format(myMagazine_str, my_magazine);
//				if(!call_result){
//					call_result = fixs_ith::workingSet_t::instance()->imm_lag_handler->createLAGRootClass(myMagazine_str,new_switch_board.slot_position);
//					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG loading lagid objects as otherboard is added in myMagazine:%s slot:%d",myMagazine_str,new_switch_board.slot_position);
//				}
//			}
//	}
//
//	// The switch board is already into _my_switch_board_infos vector?
//	if (std::find_if(_my_switch_board_infos.begin(), _my_switch_board_infos.end(),
//			match_sb_magazine_slot(new_switch_board.magazine, new_switch_board.slot_position)) != _my_switch_board_infos.end()) { // YES
//		FIXS_ITH_LOG(LOG_LEVEL_WARN,
//				"The new switch board (magazine == 0x%08X, slot == %d) is already into MY-SWITCH-BOARD vector: no action will be performed",
//				new_switch_board.magazine, new_switch_board.slot_position);
//
//		return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//	}
//
//	// BEGIN: DEBUGGING and TRACING: TO BE DELETE
//		FIXS_ITH_LOG(LOG_LEVEL_TRACE, "Dumping _other_switch_board_infos internal vector before adding a new SwitchBoard: _other_switch_board_infos.size() == %zu", _other_switch_board_infos.size());
//
//		for (size_t index = 0; index < _other_switch_board_infos.size(); ++index) {
//			fixs_ith_switchboardinfo & sbi_item = _other_switch_board_infos[index];
//
//			FIXS_ITH_LOG(LOG_LEVEL_TRACE, "OTHER SWITCH_BOARD_INFO[%zu] --> magazine == %u, slot_position == %d, IPN-A == '%s', IPN-B == '%s'",
//					index, sbi_item.magazine, sbi_item.slot_position, sbi_item.ipna_str, sbi_item.ipnb_str);
//		}
//		// END: DEBUGGING and TRACING: TO BE DELETE
//
//	// The switch board is already into _other_switch_board_infos vector?
//	if (std::find_if(_other_switch_board_infos.begin(), _other_switch_board_infos.end(),
//			match_sb_magazine_slot(new_switch_board.magazine, new_switch_board.slot_position)) != _other_switch_board_infos.end()) { // YES
//		FIXS_ITH_LOG(LOG_LEVEL_WARN,
//				"The new switch board (magazine == 0x%08X, slot == %d) is already into OTHER-SWITCH-BOARD vector: no action will be performed",
//				new_switch_board.magazine, new_switch_board.slot_position);
//
//		return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//	}
//
//
//	std::vector<fixs_ith_switchboardinfo> * switch_board_infos = ((new_switch_board.magazine == my_magazine) ? &_my_switch_board_infos : &_other_switch_board_infos);
//
//	// Adding the new switch board
//	switch_board_infos->push_back(new_switch_board);
//
//	// Starting communication (un/subscription) with the new board
//	const int call_result =  start_smx_communication(switch_board_infos->at(switch_board_infos->size() - 1));
//
//	if (call_result) { // ERROR: subscribing to the new switch board
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
//				"Call 'start_smx_communication' failed: cannot start the communication (trap subscription) towards the new inserted switch board: "
//				"call_result == %d", call_result);
//	}
//
//
//	// Reorder the collection
//	std::sort(switch_board_infos->begin(), switch_board_infos->end(),
//			(new_switch_board.magazine == my_magazine) ? fx_compare_slot_ascending : fx_compare_magazine_slot_ascending);

	//return call_result;
//		return 0;
//}
//
//int fixs_ith_snmpmanager::switch_board_change (fixs_ith_switchboardinfo & switch_board) {
//	// First delete the switch board ...
//	switch_board_delete(switch_board.magazine, switch_board.slot_position);
//
//	// ... next add the switch board
//	return switch_board_add(switch_board);
//}

int fixs_ith_snmpmanager::create_lag (char * switch_board_ip, uint16_t lag_ifIndex, const char * lag_ifname)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send create lag on switch board %s", switch_board_ip);

	int call_result = 0;

	fixs_ith_snmploggercallback callback("SNMP Response received for create lag request towards SMX");
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	char oid_str[512];
	// create a new genIfTable entry for adding LAG Interface Index
	if(!call_result){
		oid oid_mib_object [] = { FIXS_ITH_SMX_OID_LAG_GEN_IF_ROW_STATUS, lag_ifIndex};
		int oidValue = fixs_ith_snmp::snmptarget::CREATE_AND_WAIT;

		size_t oid_size = FIXS_ITH_ARRAY_SIZE(oid_mib_object);
		//oid * oid_name = oid_mib_object;
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_mib_object, oid_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set LAG_GEN_IF_ROW_STATUS (oid: %s)(value = %d) request...",oid_str, oidValue);
		call_result = set_mib_object<int>( session_handler, oid_mib_object,oid_size, oidValue);
		if(call_result){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set LAG_GEN_IF_ROW_STATUS (oid: %s)(value = %d) request ... FAILED !! [call_result == '%d'] !!", oid_str, oidValue, call_result);
		} else if(callback.error_code) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set LAG_GEN_IF_ROW_STATUS (oid: %s)(value = %d) request ... FAILED !! "
					"PDU response reported an error: callback.error_code == %d, callback.pdu_error_stat == %d",
					oid_str, oidValue, callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);

		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set LAG_GEN_IF_ROW_STATUS (oid: %s)(value = %d) request ... SUCCESSFUL !!",oid_str, oidValue);
		}
	}
	// set Interface type to 'ieee8023adLag'
	if(!call_result){
		oid oid_mib_object [] = { FIXS_ITH_SMX_OID_LAG_GEN_IF_TYPE, lag_ifIndex };
		int oidValue = IANA_LAG_IF_TYPE;

		size_t oid_size = FIXS_ITH_ARRAY_SIZE(oid_mib_object);
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_mib_object, oid_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set interface type (oid: %s)(value = %d) request...",oid_str, oidValue);
		call_result = set_mib_object<int>(session_handler, oid_mib_object,oid_size, oidValue);
		if(call_result){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set interface type (oid: %s)(value = %d) request ... FAILED !! [call_result == '%d'] !!", oid_str, oidValue, call_result);
		} else if(callback.error_code) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set interface type (oid: %s)(value = %d) request ... FAILED !! "
					"PDU response reported an error: callback.error_code == %d, callback.pdu_error_stat == %d",
					oid_str, oidValue, callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);

		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set interface type (oid: %s)(value = %d) request ... SUCCESSFUL !!",oid_str, oidValue);
		}
	}
	// set the genIfRowStatus object to Active(1) to complete LAG creation
	if(!call_result){
		oid oid_mib_object [] = { FIXS_ITH_SMX_OID_LAG_GEN_IF_ROW_STATUS, lag_ifIndex };
		int oidValue = fixs_ith_snmp::snmptarget::ACTIVE;
		size_t oid_size = FIXS_ITH_ARRAY_SIZE(oid_mib_object);
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_mib_object, oid_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set LAG_GEN_IF_ROW_STATUS to active (oid: %s)(value = %d) request...",oid_str, oidValue);
		call_result = set_mib_object<int>(session_handler, oid_mib_object,oid_size, oidValue);
		if(call_result){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set LAG_GEN_IF_ROW_STATUS to active (oid: %s)(value = %d) request ... FAILED !! [call_result == '%d'] !!", oid_str, oidValue, call_result);
		} else if(callback.error_code) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set LAG_GEN_IF_ROW_STATUS to active (oid: %s)(value = %d) request ... FAILED !! "
					"PDU response reported an error: callback.error_code == %d, callback.pdu_error_stat == %d",
					oid_str, oidValue, callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);

		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set LAG_GEN_IF_ROW_STATUS to active (oid: %s)(value = %d) request ... SUCCESSFUL !!",oid_str, oidValue);
		}
	}
	// set alias' name for LAG interface
	if(!call_result){
		oid oid_mib_object [] = { FIXS_ITH_SMX_OID_LAG_TABLE_ENTRY_IF_ALIAS, lag_ifIndex };
		char oidValue[fixs_ith_snmp::IF_ALIAS_NAME_MAX_LEN]= {};
		::strncpy(oidValue, lag_ifname, FIXS_ITH_ARRAY_SIZE(oidValue));
		size_t oid_size = FIXS_ITH_ARRAY_SIZE(oid_mib_object);
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_mib_object, oid_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set alias name for LAG Interface  (oid: %s)(value = %s) request...",oid_str, oidValue);
		call_result = set_mib_object<char*>(session_handler, oid_mib_object,oid_size, oidValue);
		if(call_result){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set alias name for LAG Interface (oid: %s)(value = %d) request ... FAILED !! [call_result == '%d'] !!", oid_str, oidValue, call_result);
		} else if(callback.error_code) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set alias name for LAG Interface (oid: %s)(value = %d) request ... FAILED !! "
					"PDU response reported an error: callback.error_code == %d, callback.pdu_error_stat == %d",
					oid_str, oidValue, callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);

		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set alias name for LAG Interface (oid: %s)(value = %d) request ... SUCCESSFUL !!",oid_str, oidValue);
		}
	}
	// Set the LAG Interface administrative state to up(1) in the IF-MIB:
	if(!call_result){
		oid oid_mib_object [] = { IF_ADMIN_STATUS, lag_ifIndex };
		int oidValue = fixs_ith_snmp::ENABLE_LAG;
		size_t oid_size = FIXS_ITH_ARRAY_SIZE(oid_mib_object);
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_mib_object, oid_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set LAG administrative state to 'ENABLE' (oid: %s)(value = %d) request...",oid_str, oidValue);
		call_result = set_mib_object<int>( session_handler, oid_mib_object,oid_size, oidValue);
		if(call_result){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set LAG administrative state to 'ENABLE' (oid: %s)(value = %d) request ... FAILED !! [call_result == '%d'] !!", oid_str, oidValue, call_result);
		} else if(callback.error_code) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG creating: set LAG administrative state to 'ENABLE' (oid: %s)(value = %d) request ... FAILED !! "
					"PDU response reported an error: callback.error_code == %d, callback.pdu_error_stat == %d",
					oid_str, oidValue, callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);

		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG creating: set LAG administrative state to 'ENABLE' (oid: %s)(value = %d) request ... SUCCESSFUL !!",oid_str, oidValue);
		}
	}

    session_handler.close();
	return call_result;
}


//Add  physical ports as members to the LAG
int fixs_ith_snmpmanager::add_lag_ports(const char * switch_board_ip, uint16_t lag_ifIndex,  const std::set<uint16_t>& port_if_list)
{
	if (*switch_board_ip == 0 ) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cannot send 'add lag port' request because switch board IP is empty '%s'", switch_board_ip);
		return 0;
	}
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send 'add lag ports' request on switch board %s", switch_board_ip);

	int call_result = 0;

	fixs_ith_snmploggercallback callback("SNMP Response received for add ports to vlan request towards SMX");
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	for(std::set<uint16_t>::const_iterator it = port_if_list.begin(); it != port_if_list.end(); ++it)
	{
		uint16_t if_index = *it;
		oid oid_scx_front_port_mib_object [] = { FIXS_ITH_SMX_OID_LAG_PORT_ACTOR_ADMIN_KEY, if_index };
		int oidValue = lag_ifIndex;
		size_t oid_fprt_size = FIXS_ITH_ARRAY_SIZE(oid_scx_front_port_mib_object);
		char oid_str[512];
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_scx_front_port_mib_object, oid_fprt_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Add port to LAG: set (oid: %s)(value = %d) request...",oid_str, oidValue);
		if((call_result = set_mib_object<int>( session_handler, oid_scx_front_port_mib_object,oid_fprt_size, oidValue))){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_mib_object' failed: call_result == %d", call_result);
			break;
		}else if(callback.error_code) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Add port to LAG: set (oid: %s)(value = %d) request... FAILED !! "
					"PDU response reported an error: callback.error_code == %d, callback.pdu_error_stat == %d",
					oid_str, oidValue, callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
			break;
		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Add port to LAG: set (oid: %s)(value = %d) request... SUCCESSFUL !!",oid_str, oidValue);
		}
	}
	session_handler.close();
	return call_result;
}


template <class Type>
int fixs_ith_snmpmanager::set_port_mib_object (char * switch_board_ip, oid * oid_fprt,size_t oid_fprt_size,Type oidValue){
	fixs_ith_snmpsessioninfo session_info;
	int call_result = 0;

	char oid_str [512];
	session_info.version(SNMP_VERSION_2c);
	session_info.community(const_cast<char *>(ITH_SNMP_COMMUNITY_RW_STRING));

	// snmp session communication handler used to send request and the callback used to receive the ACK response
	fixs_ith_snmpsessionhandler session_handler;
	fixs_ith_snmploggercallback logger_callback("Set PORT mib object: SNMP Response received for PORT configuration");

	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"Port configuration : sending Port setting request to switch board IP %s", switch_board_ip);

	session_info.peername(switch_board_ip);
	logger_callback.switch_board_ip(switch_board_ip);

	if ((call_result = session_handler.open(session_info, logger_callback))) { // ERROR: opening the communication session towards the switch board
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"PORT configure: Call 'open' failed: cannot open the communication session towards the above switch board: system error == %d, snmp error == %d, "
				"snmp error text == '%s': call_result == %d",
				session_info.last_sys_error(), session_info.last_snmp_error(), session_info.last_snmp_error_text(), call_result);
		return fixs_ith_snmp::ERR_SNMP_SESSION_OPEN;
	}

	// Prepare the PDU subscription request ITH will send to the current switch board
	netsnmp_pdu * request_pdu = 0;

	if (!(request_pdu = snmp_pdu_create(SNMP_MSG_SET))) { // ERROR: creating a new PDU structure
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"PORT configure: Call 'snmp_pdu_create' failed: memory error (probably out-of-memory) on creating a new PDU structure to carry out the subscription request");
		session_handler.close();
		return fixs_ith_snmp::ERR_SNMP_PDU_CREATE;
	}
	// WARNING: NET-SNMP library missed to reset the PDU variable pointer to null
	request_pdu->variables = 0;

	if(typeid(oidValue)== typeid(int)){
		snmp_pdu_add_variable(request_pdu, oid_fprt, oid_fprt_size,ASN_INTEGER, reinterpret_cast<u_char *>(&oidValue), sizeof(oidValue));
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_fprt, oid_fprt_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Port configuration: Sending PORT SET (oid: %s)(value = %d) request...",oid_str, oidValue);
	}
	if(typeid(oidValue)== typeid(uint32_t)){
		snmp_pdu_add_variable(request_pdu, oid_fprt, oid_fprt_size,ASN_UNSIGNED, reinterpret_cast<u_char *>(&oidValue), sizeof(oidValue));
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_fprt, oid_fprt_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Port configuration: Sending PORT SET (oid: %s)(value = %d) request...",oid_str, oidValue);
	}
	if(typeid(oidValue)== typeid(char*)){
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_fprt, oid_fprt_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "PORT configuration: Sending PORT SET (oid: %s)(value = %s) request...",oid_str, oidValue);
	}

	unsigned timeout_ms = 2000; // 2 seconds timeout
	errno=0;
	call_result = session_handler.send_synch(request_pdu, &timeout_ms); // 2 sec timeout
	int errno_save = errno;
	snmp_free_pdu(request_pdu);

	if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR,
				"POR configuration: Call 'send_synch' failed: NET-SNMP library internal error on sending the request PDU: call_result == %d", call_result);
		session_handler.close();
		return call_result;
	}

	// SNMP set request sent. Now check PDU response to get operation result
	if(logger_callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PORT configuration failed: PDU response reported an error: "
									  "callback.error_code == %d, callback.pdu_error_stat == %d", logger_callback.error_code, logger_callback.pdu_error_stat);
		call_result = (logger_callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? logger_callback.pdu_error_stat : logger_callback.error_code);
	}

	session_handler.close();

	//return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	return call_result;
}

// ******************************************************** //
// ********** Number of network interfaces *****************//
// ****  (regardless of their current state) present    ****//
// ****             on this system.                     ****//
// ******************************************************** //

int fixs_ith_snmpmanager::get_network_interfaces_number (
		int32_t & if_number,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	//char * switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT] = { switch_board.control_ipna_str, switch_board.control_ipnb_str };

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "NETWORK INTERFACES NUM get info: on SCXB having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_network_interfaces_number(if_number,switch_board.magazine,switch_board.slot_position,switch_board_ipn_str_addresses[ipn_index],timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"NETWORK INTERFACES NUM get info: Call 'get_network_interfaces_number' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_network_interfaces_number (
		int32_t & if_number,
		uint32_t board_magazine,
		int32_t board_slot,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NETWORK INTERFACES NUM get info: :Trying to get the number of network interfaces: from switch board '%s'",switch_board_ip);

	fixs_ith_get_number_of_ports_callback callback(if_number, fixs_ith_snmp::ETHERNETPORT, mag_str, board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NETWORK INTERFACES NUM get info: :Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_front_port [] = { IF_NUMBER, 0 };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_front_port; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_front_port);

	call_result = get_mib_object(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}


// ******************************************************** //
// **********  Switch Board ports handling  *************** //
// ******************************************************** //
int fixs_ith_snmpmanager::get_port_operational_status (
		fixs_ith_snmp::port_status_t & port_status,
		uint16_t port_ifIndex,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	//char * switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT] = { switch_board.control_ipna_str, switch_board.control_ipnb_str };

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "PORT OPERATIONAL STATUS get info: on SCXB having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_port_operational_status(port_status,port_ifIndex,switch_board.magazine,switch_board.slot_position,switch_board_ipn_str_addresses[ipn_index],timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"PORT OPERATIONAL STATUS  get info: Call 'get_port_operational_status' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_port_operational_status (
		fixs_ith_snmp::port_status_t & port_status,
		uint16_t port_ifIndex,
		uint32_t board_magazine,
		int32_t board_slot,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "PORT get info:Trying to get port status of port_ifIndex: %d from switch board '%s'",
				port_ifIndex, switch_board_ip);

	fixs_ith_getportstatuscallback callback(port_status, fixs_ith_snmp::OPERATIVE_STATUS, port_ifIndex, mag_str, board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PORT get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_front_port [] = { IF_OPER_STATUS, port_ifIndex };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_front_port; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_front_port);

	call_result = get_mib_object(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}


/* **** Get port admin status  *** */

int fixs_ith_snmpmanager::get_port_admin_status (
		fixs_ith_snmp::port_status_t & port_status,
		uint16_t port_ifIndex,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_addresses[0],
			switch_board_ipn_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "PORT get info: on SCXB having IPNA == %s and IPNB == %s ", switch_board_ipn_addresses[0], switch_board_ipn_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_port_admin_status(port_status,port_ifIndex,switch_board.magazine, switch_board.slot_position,switch_board_ipn_addresses[ipn_index],timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"LAG get info: Call 'get_front_port_operational_status' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_addresses[ipn_index], call_result);

	}

	return call_result;
}

int fixs_ith_snmpmanager::get_port_admin_status (
		fixs_ith_snmp::port_status_t & port_status,
		uint16_t port_ifIndex,
		uint32_t board_magazine,
		int32_t board_slot,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ADMIN PORT STATUS get info:Trying to get port status of port_ifIndex: %d of from switch board '%s'",
			port_ifIndex, switch_board_ip);

	fixs_ith_getportstatuscallback callback(port_status, fixs_ith_snmp::ADMIN_STATUS,port_ifIndex,mag_str,board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "LAG get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_front_port [] = { IF_ADMIN_STATUS, port_ifIndex };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_front_port; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_front_port);

	call_result = get_mib_object(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}

int fixs_ith_snmpmanager::get_mib_object (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "GET info: Sending the PDU GET request to retrieve value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "GET info: Timeout expired while sending the PDU GET request");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"GET info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request : call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "GET info: Callback detected error on reading response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_port_physical_address(fixs_ith_snmp::mac_address_t & port_macaddr,
			uint16_t port_ifIndex,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms){

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_addresses[0],
			switch_board_ipn_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Port physical address get info: on SMX having IPNA == %s and IPNB == %s ",switch_board_ipn_addresses[0], switch_board_ipn_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_port_physical_address(port_macaddr,port_ifIndex, switch_board.slot_position,switch_board.magazine,switch_board_ipn_addresses[ipn_index],timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Port physical address get info: Call 'get_port_physical_address' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_port_physical_address(fixs_ith_snmp::mac_address_t & port_macaddr,
			uint16_t port_ifIndex,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms){

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send get port physical address request to switch board %s in slot %d", switch_board_ip, switch_board_slot);

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_getportphysaddresscallback callback(port_macaddr, port_ifIndex, mag_str, switch_board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_name [] = { IF_PHYSICAL_ADDRESS, port_ifIndex };

	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending the PDU GET request to retrieve the mac address of interface == %d into the OID '%s'", port_ifIndex, oid_str);

	call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU GET request to retrieve port mac address");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve port mac address: call_result == %d",
				call_result);
	}

	if (call_result) { session_handler.close(); return call_result; }

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Callback detected error on reading port mac address from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_bridge_address (
		fixs_ith_snmp::mac_address_t & bridge_address,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	switch(sb_plane)
	{
	case CONTROL_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	case TRANSPORT_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	default:
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "BRIDGE ADDRESS get info: on SMX having IPNA == %s and IPNB == %s ", switch_board.control_ipna_str, switch_board.control_ipnb_str);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_bridge_address(bridge_address, switch_board.slot_position, switch_board.magazine,  switch_board_ipn_str_addresses[ipn_index],timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"BRIDGE ADDRESS get info: Call 'get_bridge_address' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);

	}

	return call_result;
}

int fixs_ith_snmpmanager::get_bridge_address (
					fixs_ith_snmp::mac_address_t & bridge_address,
					int32_t switch_board_slot,
					uint32_t switch_board_magazine,
					const char * switch_board_ip,
					const unsigned * timeout_ms)
{
	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRIDGE ADDRESS get info: Trying to get 'bridge address' from switch '%s'", switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_brigeaddress_callback callback(bridge_address, mag_str, switch_board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "BRIDGE ADDRESS get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_bridge_address [] = {FIXS_ITH_SMX_OID_BRIDGE_ADDRESS, 0 };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_bridge_address; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_bridge_address);

	call_result = get_bridge_address(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();


	return call_result;
}


int fixs_ith_snmpmanager::get_bridge_address (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRIDGE ADDRESS get info: Sending the PDU GET request to retrieve the bridge address into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "BRIDGE ADDRESS get info: Timeout expired while sending the PDU GET request to retrieve front port status");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"BRIDGE ADDRESS get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve front port status: call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "BRIDGE ADDRESS get info: Callback detected error on reading 'bridge address' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_bridgeport_if_index(
		uint16_t & if_index,
		uint16_t port_number,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;


	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Bridge Port interface index get info: on switch having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_bridgeport_if_index(if_index,port_number,switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Bridge Port interface index get info: Call 'get_bridgeport_if_index' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_bridgeport_if_index (
		uint16_t & if_index,
		uint16_t port_number,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Bridge Port interface index get info: Trying to get 'port interface index' from switch '%s'",
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_port_if_index_callback callback(if_index, port_number, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Bridge Port interface index get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_number_of_vlans [] = { FIXS_ITH_SMX_OID_BRIDGEPORT_IF_INDEX, port_number };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_number_of_vlans; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_number_of_vlans);

	call_result = get_bridgeport_if_index(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}

int fixs_ith_snmpmanager::get_bridgeport_if_index (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRIDGE PORT INTERFACE INDEX get info: Sending the PDU GET request to retrieve the 'bridge port if index' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "BRIDGE PORT INTERFACE INDEX  get info: Timeout expired while sending the PDU GET request to retrieve 'bridge port if index'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"BRIDGE PORT INTERFACE INDEX get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve 'bridge port if index': call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "BRIDGE PORT INTERFACE INDEX get info: Callback detected error on reading 'bridge port if index' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_number_of_vlans (
		uint32_t & num_of_vlans,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	switch(sb_plane)
	{
	case CONTROL_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	case TRANSPORT_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	default:
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "NUMBER OF VLANS get info: on switch having IPNA == %s and IPNB == %s ", switch_board.control_ipna_str, switch_board.control_ipnb_str);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_number_of_vlans(num_of_vlans, switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"NUMBER OF VLANS get info: Call 'get_number_of_vlans' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_number_of_vlans (
		uint32_t & num_of_vlans,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NUMBER OF VLANS get info: Trying to get 'number of vlans' from switch '%s'",
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_number_of_vlans_callback callback(num_of_vlans,mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NUMBER OF VLANS get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_number_of_vlans [] = { FIXS_ITH_SMX_OID_BRIDGE_NUM_VLANS, 0 };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_number_of_vlans; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_number_of_vlans);

	call_result = get_number_of_vlans(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_number_of_vlans (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NUMBER OF VLANS get info: Sending the PDU GET request to retrieve the 'number of vlans' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NUMBER OF VLANS get info: Timeout expired while sending the PDU GET request to retrieve 'number of vlans'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"NUMBER OF VLANS get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve 'number of vlans': call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NUMBER OF VLANS get info: Callback detected error on reading 'number of vlans' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_interface_speed (
		uint32_t & speed,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	switch(sb_plane)
	{
	case CONTROL_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	case TRANSPORT_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	default:
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "INTERFACE SPEED get info: [IF_INDEX = %d], on switch having IPNA == %s and IPNB == %s ", if_index, switch_board.control_ipna_str, switch_board.control_ipnb_str);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_interface_speed(speed, if_index, switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"INTERFACE SPEED get info: Call 'get_interface_speed' failed: if_index == %d, switch board slot == %d, switch board IPN = '%s': call_result == %d",
					if_index, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_interface_speed (
		uint32_t & speed,
		uint16_t if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "INTERFACE SPEED get info: Trying to get 'interface speed' from switch '%s'",
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_interface_speed_callback callback(speed, if_index, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "INTERFACE SPEED get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_if_speed [] = { SNMP_OID_IF_HIGH_SPEED, if_index };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_if_speed; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_if_speed);

	call_result = get_interface_speed(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_interface_speed (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms)
{
	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "INTERFACE SPEED get info: Sending the PDU GET request to retrieve the 'interface speed' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "INTERFACE SPEED get info: Timeout expired while sending the PDU GET request to retrieve 'interface speed'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"INTERFACE SPEED get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve 'interface speed': call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "INTERFACE SPEED get info: Callback detected error on reading 'interface speed' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_lag_operMembers(
		fixs_ith_snmp::if_bitmap_t & port_list,
		uint16_t lag_if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	oid oid_lag_operMembers [] = { FIXS_ITH_SMX_OID_LAG_PORT_LIST_PORTS, lag_if_index };
	const oid * oid_name = 0;
	size_t oid_name_size = 0;
	oid_name = oid_lag_operMembers; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_lag_operMembers);

	// TODO: improve LOG message
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG OPERATIVE MEMBERS get info: Trying to get 'LAG operative members' of LAG '%d'", lag_if_index);

	return get_portlist(port_list, oid_name, oid_name_size, switch_board, sb_plane, timeout_ms);
}


int fixs_ith_snmpmanager::get_lag_operMembers(
		fixs_ith_snmp::if_bitmap_t & port_list,
		uint16_t lag_if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{
		oid oid_lag_operMembers [] = { FIXS_ITH_SMX_OID_LAG_PORT_LIST_PORTS, lag_if_index };
		const oid * oid_name = 0;
		size_t oid_name_size = 0;
		oid_name = oid_lag_operMembers; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_lag_operMembers);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "LAG OPERATIVE MEMBERS get info: Trying to get 'LAG operative members' of LAG '%d' from switch '%s'",
						lag_if_index, switch_board_ip);

		return get_portlist(port_list, oid_name, oid_name_size, switch_board_slot, switch_board_magazine, switch_board_ip, timeout_ms);
}

int fixs_ith_snmpmanager::check_lag (const fixs_ith_switchboardinfo & switch_board,
			const fixs_ith::switchboard_plane_t sb_plane,
			const uint16_t lag_if_index,
			const unsigned * timeout_ms)
{
   char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

   if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1], switch_board, sb_plane))
		return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

   oid lag_index_oid [] = { FIXS_ITH_SMX_OID_LAG_GEN_IF_ROW_STATUS, lag_if_index};
   size_t lag_index_oid_size = FIXS_ITH_ARRAY_SIZE(lag_index_oid);

   // try first on IPNA
   int check_oid_res = check_oid(lag_index_oid, lag_index_oid_size, switch_board.magazine, switch_board.slot_position, switch_board_ipn_str_addresses[0], timeout_ms);
   if(check_oid_res  &&  (check_oid_res !=  fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE))
   {
	   // failure ! Try on IPNB
	   check_oid_res =  check_oid(lag_index_oid, lag_index_oid_size, switch_board.magazine, switch_board.slot_position, switch_board_ipn_str_addresses[1], timeout_ms);
   }

   return  check_oid_res;
}


int fixs_ith_snmpmanager::check_oid(
       const oid * oid_name,
       size_t oid_name_size,
       uint32_t switch_board_magazine,
	   int32_t switch_board_slot,
       const char * switch_board_ip,
       const unsigned * timeout_ms)
{
	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	char oid_str [512]= {0};
	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Checking existence of OID '%s' on '%s' ... ", oid_str, switch_board_ip);

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_checkoidcallback callback(oid_name, oid_name_size, mag_str, switch_board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	// open SNMP session
	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip)))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	// send SNMP GET request
	call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);
    if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT)
    {   // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU GET request to check existence of OID '%s' on '%s'", oid_str, switch_board_ip);
	}
    else if (call_result)
	{   // ERROR: sending the request
	    FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to check existence of OID '%s' on '%s'", oid_str, switch_board_ip);
	}

	if (call_result)
		return call_result;

	// check SNMP GET result
	if (callback.error_code)
	{
	   if(callback.error_code ==  fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE)
	   {
		   FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "... OID '%s' on '%s' NOT found ! ", oid_str, switch_board_ip);
		   call_result = callback.error_code;
	   }
	   else
	   {
           FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Callback detected error from response PDU: "
						"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
           call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	   }
	}
	else
	{
	   FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "... found OID '%s' on '%s' !", oid_str, switch_board_ip);
	}

	return call_result;
}


template < int PORTLIST_ARRAY_SIZE >
	int fixs_ith_snmpmanager::get_portlist(
			uint8_t (& port_list) [PORTLIST_ARRAY_SIZE],
			const oid * oid_name,
			size_t oid_name_size,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	switch(sb_plane)
	{
	case CONTROL_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	case TRANSPORT_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	default:
		return -1;
	}

	//TODO: prepare a useful log message
	//FIXS_ITH_LOG(LOG_LEVEL_INFO, "PORTLIST get info: on switch having IPNA == %s and IPNB == %s ", switch_board.control_ipna_str, switch_board.control_ipnb_str);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_portlist(port_list, oid_name, oid_name_size, switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Call 'get_portlist' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

template <int PORTLIST_ARRAY_SIZE >
	int fixs_ith_snmpmanager::get_portlist(
			uint8_t (& port_list) [PORTLIST_ARRAY_SIZE],
			const oid * oid_name,
			size_t oid_name_size,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char oid_str [512];
	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to retrieve the OID '%s' from switch '%s'", oid_str,
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_portlist_callback<PORTLIST_ARRAY_SIZE> callback(port_list, oid_name, oid_name_size, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Error while trying to retrieve the OID '%s' from switch '%s': Call 'open_snmp_session_handler' failed: call_result == %d",
				oid_str,	switch_board_ip, call_result);
		return call_result;
	}

	call_result = get_portlist(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}

int fixs_ith_snmpmanager::get_portlist (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending the PDU GET request to retrieve the OID '%s' having syntax 'PortList' ", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU GET request to retrieve the OID '%s'", oid_str);
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve the OID '%s': call_result == %d",
				oid_str,
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Callback detected error on reading the value of the OID '%s' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", oid_str, callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

//SNMP operations to configure a VLAN
int fixs_ith_snmpmanager::create_vlan(fixs_ith_switchboardinfo & switch_board,
		uint16_t vlan_id, const char * vlan_name,
		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result_c = 0; int call_result_t = 0;

	char switch_board_ipn_str_addresses[2][16] = {};

	if (( sb_plane & CONTROL_PLANE_MASK)){
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Create VLAN '%d': on SMX having IPNA == %s and IPNB == %s ", vlan_id, switch_board.control_ipna_str, switch_board.control_ipnb_str);

		int call_result_c = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
		::srandom(static_cast<unsigned>(::time(0)));
		int ipn_index = ::random() & 0x01;
		for (int i = 0; (isCommunicationFailure(call_result_c) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
			call_result_c = create_vlan(switch_board_ipn_str_addresses[ipn_index], vlan_id, vlan_name);
			if (call_result_c)
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"Create VLAN '%d' : Call 'create_vlan' failed: switch board slot == %d, switch board Control IPN = '%s': call_result == %d",
						vlan_id, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result_c);
		}
		if (call_result_c)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'create_vlan' failed on both control ipns for switch board in slot %d",
					switch_board.slot_position);
		}
	}
	if (sb_plane & TRANSPORT_PLANE_MASK){
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Vlan Create: on SMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

		int call_result_t = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
		::srandom(static_cast<unsigned>(::time(0)));
		int ipn_index = ::random() & 0x01;
		for (int i = 0; (isCommunicationFailure(call_result_t) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
			call_result_t = create_vlan(switch_board_ipn_str_addresses[ipn_index], vlan_id, vlan_name);
			if (call_result_t)
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"Create Vlan: Call 'create_vlan' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",
						switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result_t);
		}
		if (call_result_t)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'create_vlan' failed on both control ipns for switch board in slot %d",
					switch_board.slot_position);
		}
	}

	if (call_result_c)  return fixs_ith_snmp::ERR_SNMP_VLAN_CREATE_CONTROL_PLANE;
	if (call_result_t)  return fixs_ith_snmp::ERR_SNMP_VLAN_CREATE_TRANSPORT_PLANE;
	if (call_result_c && call_result_t)  return fixs_ith_snmp::ERR_SNMP_VLAN_CREATE_BOTH_PLANES;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_snmpmanager::create_lag(fixs_ith_switchboardinfo & switch_board,
		uint16_t lag_ifIndex, const char * lag_name,
		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "LAG Create: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = create_lag(switch_board_ipn_str_addresses[ipn_index], lag_ifIndex, lag_name);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Create Lag: Call 'create_lag' failed: switch board slot == %d, switch board Control IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'create_lag' failed on both control ipns for switch board in slot %d",
				switch_board.slot_position);
	}
	return call_result;
}

// Add ethernet ports as link aggregation member to LAG
int fixs_ith_snmpmanager::add_lag_ports(fixs_ith_switchboardinfo & switch_board,
		uint16_t lag_ifIndex, const std::set<uint16_t>& port_if_list,
		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "ADD LAG ports: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = add_lag_ports(switch_board_ipn_str_addresses[ipn_index], lag_ifIndex, port_if_list);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"SET LAG ports: Call 'add_lag_ports' failed: switch board slot == %d, switch board Control IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'add_lag_ports' failed on both control ipns for switch board in slot %d",
				switch_board.slot_position);
	}
	return call_result;
}

int fixs_ith_snmpmanager::create_vlan(const char * switch_board_ip,uint16_t vlan_id, const char * vlan_name)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (*switch_board_ip == 0 ) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'create vlan' request because switch board IP is empty '%s'", switch_board_ip);
		return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send create vlan request to switch board %s", switch_board_ip);

	int call_result = 0;

	fixs_ith_snmploggercallback callback("SNMP Response received for Create Vlan request towards SMX");
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	unsigned timeout_ms = 2000;

	{
		// Setting qVlanStaticRowStatus
		oid oid_vlan_static_row_status [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_ROW_STATUS, vlan_id };
		size_t oid_row_status_size = FIXS_ITH_ARRAY_SIZE(oid_vlan_static_row_status);

		if ((call_result = set_qVlanStaticRowStatus(fixs_ith_snmp::snmptarget::CREATE_AND_GO, session_handler, oid_vlan_static_row_status, oid_row_status_size, &timeout_ms))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_qVlanStaticRowStatus' failed: call_result == %d", call_result);
			session_handler.close();
			return call_result;
		}

		// SNMP set request sent. Now check PDU response to see operation result
		if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_qVlanStaticRowStatus()' reported an error: "
					                      "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
			session_handler.close();
			return call_result;
		}
	}
	{
		// Setting qVlanStaticName
		oid oid_vlan_static_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_NAME, vlan_id };
		size_t oid_vlan_static_name_size = FIXS_ITH_ARRAY_SIZE(oid_vlan_static_name);

		if ((call_result = set_qVlanStaticName(vlan_name, session_handler,oid_vlan_static_name,oid_vlan_static_name_size, &timeout_ms))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_qVlanStaticName' failed: call_result == %d", call_result);
			session_handler.close();
			return call_result;
		}

		// SNMP set request sent. Now check PDU response to see if the requested operation was successfully
		if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_qVlanStaticName()' reported an error: "
										  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
			session_handler.close();
			return call_result;
		}
	}

	session_handler.close();

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}


int fixs_ith_snmpmanager::set_qVlanStaticRowStatus(
			fixs_ith_snmp::snmptarget::addr_row_status_t row_status,
			fixs_ith_snmpsessionhandler & session_handler,
			oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char oid_str [512];
	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Row Status': OID '%s': value %d", oid_str, row_status);

	int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_INTEGER, reinterpret_cast<const u_char *>(&row_status), sizeof(row_status), timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::set_qVlanStaticName (
					const char * vlan_name,
					fixs_ith_snmpsessionhandler & session_handler,
					oid * oid_name,
					size_t oid_name_size,
					const unsigned * timeout_ms
					)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char oid_str [512];
	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Vlan Name': OID '%s': value '%s'", oid_str, vlan_name);

	int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(vlan_name), ::strlen(vlan_name), timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
	}

	return call_result;
}


int fixs_ith_snmpmanager::set_vlan_ports(fixs_ith_switchboardinfo & switch_board,
		uint16_t vlan_id, const std::set<uint16_t> & tagged_ports,
		const std::set<uint16_t> & untagged_ports,
		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Setting (TAGGED and UNTAGGED) ports for VLAN '%d' on SMX bridge having IPNA == %s and IPNB == %s ", vlan_id, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = set_vlan_ports(switch_board_ipn_str_addresses[ipn_index], vlan_id, tagged_ports, untagged_ports);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Call 'set_vlan_ports' for VLAN '%d' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					vlan_id, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to set (TAGGED and UNTAGGED) ports or VLAN '%d' on SMX bridge having IPNA == %s and IPNB == %s. "
				"Call 'set_vlan_ports' failed on both IPNS !",
				vlan_id, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);
	}

	return call_result;
}

int fixs_ith_snmpmanager::set_vlan_ports(const char * switch_board_ip,
					uint16_t vlan_id, const std::set<uint16_t>& tagged_ports,
					const std::set<uint16_t>& untagged_ports)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to configure (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s",
			vlan_id, switch_board_ip);
        if(common::utility::isSwitchBoardSMX())
        {
		fixs_ith_snmp::if_bitmap_t  ifIndex_bitmap_tagged = {0};
		if (get_ifIndex_bitmap(tagged_ports, ifIndex_bitmap_tagged))
        	{
                	FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to configure (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s (invalid TAGGED port values provided) !",
                                vlan_id, switch_board_ip);
                	return fixs_ith_snmp::ERR_SNMP_L2_INTERFACE_TO_BITMAP;
        	}

        	fixs_ith_snmp::if_bitmap_t  ifIndex_bitmap_untagged = {0};
        	if (get_ifIndex_bitmap(untagged_ports, ifIndex_bitmap_untagged))
        	{
                	FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to configure (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s (invalid UNTAGGED port values provided) !",
                                vlan_id, switch_board_ip);
                	return fixs_ith_snmp::ERR_SNMP_L2_INTERFACE_TO_BITMAP;
        	}
		int call_result = set_vlan_ports(switch_board_ip, vlan_id, ifIndex_bitmap_tagged, ifIndex_bitmap_untagged);
       if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to configure (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s "
                                "[ERROR CODE %d]", vlan_id, switch_board_ip, call_result);
                return call_result;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully configured (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s",
                                vlan_id, switch_board_ip);
        //return 0;



        }
        if(common::utility::isSwitchBoardCMX())
        {
		fixs_ith_snmp::if_bitmap_t_cmx  ifIndex_bitmap_tagged = {0};
                if (get_ifIndex_bitmap(tagged_ports, ifIndex_bitmap_tagged))
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to configure (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s (invalid TAGGED port values provided) !",
                                vlan_id, switch_board_ip);
                        return fixs_ith_snmp::ERR_SNMP_L2_INTERFACE_TO_BITMAP;
                }

                fixs_ith_snmp::if_bitmap_t_cmx  ifIndex_bitmap_untagged = {0};
                if (get_ifIndex_bitmap(untagged_ports, ifIndex_bitmap_untagged))
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to configure (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s (invalid UNTAGGED port values provided) !",
                                vlan_id, switch_board_ip);
                        return fixs_ith_snmp::ERR_SNMP_L2_INTERFACE_TO_BITMAP;
                }
	int call_result = set_vlan_ports(switch_board_ip, vlan_id, ifIndex_bitmap_tagged, ifIndex_bitmap_untagged);
	if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to configure (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s "
                                "[ERROR CODE %d]", vlan_id, switch_board_ip, call_result);
                return call_result;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully configured (TAGGED and UNTAGGED) ports of VLAN '%d' on SMX bridge having IP %s",
                                vlan_id, switch_board_ip);
        //return 0;

        }

	return 0; 
}

int fixs_ith_snmpmanager::set_vlan_ports(const char * switch_board_ip, uint16_t vlan_id,
					const fixs_ith_snmp::if_bitmap_t & if_bitmap_tagged,
					const fixs_ith_snmp::if_bitmap_t & if_bitmap_untagged)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (*switch_board_ip == 0 ) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cannot configure vlan ports for VLAN '%d' because switch board IP is empty '%s'", vlan_id, switch_board_ip);
		return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to configure (tagged and untagged) ports of VLAN '%d' on SMX bridge having IP %s",
			vlan_id, switch_board_ip);

	int call_result = 0;
	
	fixs_ith_snmp::if_bitmap_t if_bitmap_tagged_OR_untagged = {0};

	for(size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(if_bitmap_tagged_OR_untagged); ++i) {
		if_bitmap_tagged_OR_untagged[i] = if_bitmap_tagged[i] | if_bitmap_untagged[i];
		FIXS_ITH_LOG(LOG_LEVEL_TRACE, "if_bitmap_tagged_OR_untagged[%d] == %02x ", i, if_bitmap_tagged_OR_untagged[i]);
		/* For Debug purpose, to be removed later */
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "if_bitmap_tagged_OR_untagged[%d] == %02x ", i, if_bitmap_tagged_OR_untagged[i]);
	}
	/* For Debug purpose, to be removed later */
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "size of if_bitmap_tagged_OR_untagged[]: %d" , sizeof(if_bitmap_tagged_OR_untagged));

	fixs_ith_snmploggercallback callback("SNMP Response received for 'set VLAN ports' request towards SMX");
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	const unsigned timeout_ms = 2000;
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting VLAN STATIC EGRESS ports {VLAN_ID == %d} on SMX bridge having IP %s", vlan_id, switch_board_ip);

		// Setting qVlanStaticEgressPorts
		oid oid_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_EGRESS_PORTS, vlan_id };
		size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

		int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(if_bitmap_tagged_OR_untagged), sizeof(if_bitmap_tagged_OR_untagged), &timeout_ms);

		/* Debug purpose, remove later*/

		 FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "oid name: %s, oid_name_size: %d" , oid_str ,oid_name_size);
		if(call_result){
			if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
			} else if (call_result) { // ERROR: sending the request
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
			}
			session_handler.close();
			return call_result;
		}

		// SNMP set request sent. Now check PDU response to get operation result
		if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of SNMP set request [VLAN STATIC EGRESS ports (OID '%s') ] reported an error: "
										  "callback.error_code == %d, callback.pdu_error_stat == %d", oid_str, callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
			session_handler.close();
			return call_result;
		}
	}

	{
		// Setting qVlanStaticUntaggedPorts
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting VLAN STATIC UNTAGGED ports {VLAN_ID == %d} on SMX bridge having IP %s", vlan_id, switch_board_ip);

		oid oid_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_UNTAGGED_PORTS, vlan_id };
		size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

		int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(if_bitmap_untagged), sizeof(if_bitmap_untagged), &timeout_ms);
		if(call_result){
			if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
			} else if (call_result) { // ERROR: sending the request
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
			}
			session_handler.close();
			return call_result;
		}

		// SNMP set request sent. Now check PDU response to get operation result
		if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of SNMP set request [VLAN STATIC UNTAGGED ports (OID '%s') ] reported an error: "
										  "callback.error_code == %d, callback.pdu_error_stat == %d", oid_str, callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
			session_handler.close();
			return call_result;
		}
	}

	session_handler.close();

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

/* Setting VLAN Ports for CMX */
int fixs_ith_snmpmanager::set_vlan_ports(const char * switch_board_ip, uint16_t vlan_id,
                                        const fixs_ith_snmp::if_bitmap_t_cmx & if_bitmap_tagged,
                                        const fixs_ith_snmp::if_bitmap_t_cmx & if_bitmap_untagged)
{
        //ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cannot configure vlan ports for VLAN '%d' because switch board IP is empty '%s'", vlan_id, switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to configure (tagged and untagged) ports of VLAN '%d' on CMX bridge having IP %s",
                        vlan_id, switch_board_ip);

        int call_result = 0;

        fixs_ith_snmp::if_bitmap_t_cmx if_bitmap_tagged_OR_untagged = {0};

        for(size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(if_bitmap_tagged_OR_untagged); ++i) {
                if_bitmap_tagged_OR_untagged[i] = if_bitmap_tagged[i] | if_bitmap_untagged[i];
                FIXS_ITH_LOG(LOG_LEVEL_TRACE, "if_bitmap_tagged_OR_untagged[%d] == %02x ", i, if_bitmap_tagged_OR_untagged[i]);
                /* For Debug purpose, to be removed later */
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "if_bitmap_tagged_OR_untagged[%d] == %02x ", i, if_bitmap_tagged_OR_untagged[i]);
        }
        /* For Debug purpose, to be removed later */
        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "size of if_bitmap_tagged_OR_untagged[]: %d" , sizeof(if_bitmap_tagged_OR_untagged));

        fixs_ith_snmploggercallback callback("SNMP Response received for 'set VLAN ports' request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        const unsigned timeout_ms = 2000;
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting VLAN STATIC EGRESS ports {VLAN_ID == %d} on CMX bridge having IP %s", vlan_id, switch_board_ip);

                // Setting qVlanStaticEgressPorts
                oid oid_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_EGRESS_PORTS, vlan_id };
                size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);
                int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(if_bitmap_tagged_OR_untagged), sizeof(if_bitmap_tagged_OR_untagged), &timeout_ms);

                /* Debug purpose, remove later*/

                 FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "oid name: %s, oid_name_size: %d" , oid_str ,oid_name_size);
                if(call_result){
                        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
                        } else if (call_result) { // ERROR: sending the request
                                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
                        }
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to get operation result
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of SNMP set request [VLAN STATIC EGRESS ports (OID '%s') ] reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", oid_str, callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }
        }

        {
                // Setting qVlanStaticUntaggedPorts
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Setting VLAN STATIC UNTAGGED ports {VLAN_ID == %d} on CMX bridge having IP %s", vlan_id, switch_board_ip);

		oid oid_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_UNTAGGED_PORTS, vlan_id };
		size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
        	char oid_str [512];
        	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

                int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(if_bitmap_untagged), sizeof(if_bitmap_untagged), &timeout_ms);
                if(call_result){
                        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
                        } else if (call_result) { // ERROR: sending the request
                                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
                        }
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to get operation result
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of SNMP set request [VLAN STATIC UNTAGGED ports (OID '%s') ] reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", oid_str, callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }
        }

        session_handler.close();

        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_snmpmanager::remove_vlan_ports(fixs_ith_switchboardinfo & switch_board,
				uint16_t vlan_id,
				fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);


	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Removing ports from Vlan %d", vlan_id);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Removing ports on SMX having IPNA == %s and IPNB == %s ",switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = remove_vlan_ports(switch_board_ipn_str_addresses[ipn_index], vlan_id);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Remove vlan ports : Call 'remove_vlan_ports' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'remove_vlan_ports' failed on both ipns for switch board in slot %d",
				switch_board.slot_position);
	}

	return call_result;
}

int fixs_ith_snmpmanager::remove_vlan_ports(const char * switch_board_ip, uint16_t vlan_id)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (*switch_board_ip == 0 ) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cannot send remove vlan ports request because switch board IP is empty '%s'", switch_board_ip);
		return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send remove vlan ports request to switch board %s", switch_board_ip);

	fixs_ith_snmp::if_bitmap_t  ifIndex_bitmap = {0};

	int  call_result = set_vlan_ports(switch_board_ip, vlan_id, ifIndex_bitmap, ifIndex_bitmap);

	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'remove_vlan_ports' failed on switch board %s", switch_board_ip);
	}

	return call_result;
}


int fixs_ith_snmpmanager::delete_vlan(fixs_ith_switchboardinfo & switch_board,
		uint16_t vlan_id,
		fixs_ith::switchboard_plane_t sb_plane)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Deleting Vlan on SMX having IPNA == %s and IPNB == %s ",switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = delete_vlan(switch_board_ipn_str_addresses[ipn_index], vlan_id);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Delete Vlan : Call 'delete_vlan' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'delete_vlan' failed on both ipns for switch board in slot %d",
				switch_board.slot_position);
	}

	return call_result;
}


int fixs_ith_snmpmanager::delete_vlan(const char * switch_board_ip, uint16_t vlan_id)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (*switch_board_ip == 0 ) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cannot send delete  vlan request because switch board IP is empty '%s'", switch_board_ip);
		return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send delete vlan request to switch board %s", switch_board_ip);

	int call_result = 0;

	fixs_ith_snmploggercallback callback("SNMP Response received for Delete Vlan request towards SMX");
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	unsigned timeout_ms = 2000;

	// Setting qVlanStaticRowStatus
	oid oid_vlan_static_row_status [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_ROW_STATUS, vlan_id };
	size_t oid_row_status_size = FIXS_ITH_ARRAY_SIZE(oid_vlan_static_row_status);

	if ((call_result = set_qVlanStaticRowStatus(fixs_ith_snmp::snmptarget::DESTROY, session_handler, oid_vlan_static_row_status, oid_row_status_size, &timeout_ms))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_qVlanStaticRowStatus' failed: call_result == %d", call_result);
	}
	else {
		// SNMP set request sent. Now check PDU response to get operation result
		if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_qVlanStaticRowStatus()' reported an error: "
										  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
		}
	}

	session_handler.close();
	return call_result;
}

int fixs_ith_snmpmanager::check_vlan(
		const fixs_ith_switchboardinfo & switch_board,
		const fixs_ith::switchboard_plane_t sb_plane,
		const uint16_t vlan_id,
		const unsigned * timeout_ms)
{
   char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

   if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1], switch_board, sb_plane))
   		return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

   oid vlan_oid [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_ROW_STATUS, vlan_id };
   size_t vlan_oid_size = FIXS_ITH_ARRAY_SIZE(vlan_oid);

   // try first on IPNA
   int check_oid_res = check_oid(vlan_oid, vlan_oid_size, switch_board.magazine, switch_board.slot_position, switch_board_ipn_str_addresses[0], timeout_ms);
   if(check_oid_res  &&  (check_oid_res !=  fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE))
   {
	   // failure ! Try on IPNB
	   check_oid_res =  check_oid(vlan_oid, vlan_oid_size, switch_board.magazine, switch_board.slot_position, switch_board_ipn_str_addresses[1], timeout_ms);
   }

   return  check_oid_res;
}


int fixs_ith_snmpmanager::get_vlan_name(char (& vlan_name) [fixs_ith_snmp::VLAN_NAME_MAX_LEN + 1],
		uint16_t vlan_id,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Vlan name get info: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_vlan_name(vlan_name,vlan_id, switch_board.slot_position,switch_board.magazine,switch_board_ipn_str_addresses[ipn_index],timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Port physical address get info: Call 'get_vlan_name' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_vlan_name(char (& vlan_name) [fixs_ith_snmp::VLAN_NAME_MAX_LEN + 1],
		uint16_t vlan_id,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send get vlan name request for vlan_id %d to switch board %s in slot %d", vlan_id, switch_board_ip, switch_board_slot);

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	//int default_vlan_id = 0;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_getvlanNamecallback callback(vlan_name, vlan_id, mag_str, switch_board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_NAME, vlan_id };

	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending the PDU GET request to retrieve the Vlan Name of vlan_id == %d into the OID '%s'", vlan_id, oid_str);

	call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU GET request to retrieve vlan name");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve the vlan name: call_result == %d",
				call_result);
	}

	if (call_result) { session_handler.close(); return call_result; }

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Callback detected error on reading the  vlan name from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_vlan_ports(fixs_ith_snmp::if_bitmap_t & port_list,
			uint16_t vlan_id,
			fixs_ith_snmp::vlanPort_type_t port_type,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	switch(port_type){
		case fixs_ith_snmp::TAGGED:
			call_result = get_vlan_tagged_ports(port_list, vlan_id,switch_board,sb_plane,timeout_ms);
		break;
		case fixs_ith_snmp::UNTAGGED:
			// GET qVlanStaticEgressPorts
			call_result = get_vlan_untagged_ports(port_list, vlan_id,switch_board,sb_plane,timeout_ms);
			break;
		default:
			call_result=  fixs_ith_snmp::ERR_SNMP_UNKNOWN_VLAN_PORT_TYPE;
			break;
	}
    return call_result;
}


int fixs_ith_snmpmanager::get_vlan_untagged_ports(fixs_ith_snmp::if_bitmap_t & port_list,
			uint16_t vlan_id,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	oid oid_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_UNTAGGED_PORTS, vlan_id };
	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VLAN Port get info: Trying to get 'VLAN UNTAGGED Ports' of Vlan '%d' ", vlan_id);

	return get_portlist(port_list,oid_name,oid_name_size,switch_board,sb_plane, timeout_ms);

}

int fixs_ith_snmpmanager::get_vlan_tagged_ports(fixs_ith_snmp::if_bitmap_t & port_list,
			uint16_t vlan_id,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	oid oid_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_EGRESS_PORTS, vlan_id };

	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VLAN Port get info: Trying to get 'VLAN TAGGED Ports' of Vlan '%d'",vlan_id);

	return get_portlist(port_list,oid_name,oid_name_size,switch_board,sb_plane, timeout_ms);

}
int fixs_ith_snmpmanager::get_vlan_ports(fixs_ith_snmp::if_bitmap_t & port_list,
			uint16_t vlan_id,
			fixs_ith_snmp::vlanPort_type_t port_type,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms)
{

	oid oid_tagged_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_EGRESS_PORTS, vlan_id };
	oid oid_untagged_name [] = { FIXS_ITH_SMX_OID_VLAN_STATIC_UNTAGGED_PORTS, vlan_id };

	const oid *oid_name = 0;
	if(port_type == fixs_ith_snmp::TAGGED)  		oid_name = oid_tagged_name;
	else if (port_type == fixs_ith_snmp::UNTAGGED) 	oid_name = oid_untagged_name;
	else return fixs_ith_snmp::ERR_SNMP_UNKNOWN_VLAN_PORT_TYPE;

	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VLAN Port get info: Trying to get 'VLAN %s Ports' of Vlan '%d' from switch '%s'",
			(port_type == fixs_ith_snmp::TAGGED)? "TAGGED":"UNTAGGED",vlan_id, switch_board_ip);

	return get_portlist(port_list, oid_name, oid_name_size, switch_board_slot, switch_board_magazine, switch_board_ip, timeout_ms);
}

int fixs_ith_snmpmanager::get_vlan_ports(fixs_ith_snmp::if_bitmap_t & tagged_port_list,
			fixs_ith_snmp::if_bitmap_t & untagged_port_list,
			uint16_t vlan_id,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VLAN Port get info: Trying to get 'ALL VLAN Ports' of Vlan '%d' from switch in the magazine '%u' slot '%d' on the %s plane",
			vlan_id, switch_board.magazine, switch_board.slot_position, (fixs_ith::CONTROL_PLANE == sb_plane)? "CONTROL": "TRANSPORT");

	int call_result = get_vlan_ports(untagged_port_list, vlan_id, fixs_ith_snmp::UNTAGGED, switch_board, sb_plane, timeout_ms);
	if(call_result != fixs_ith_snmp::ERR_SNMP_NO_ERRORS)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "CANNOT get 'VLAN UNTAGGED Ports' of Vlan '%d' from switch in the magazine '%u' slot '%d' on the %s plane",
					vlan_id, switch_board.magazine, switch_board.slot_position, (fixs_ith::CONTROL_PLANE == sb_plane)? "CONTROL": "TRANSPORT");
		return call_result;
	}
	fixs_ith_snmp::if_bitmap_t egress_port_list;
	memset(egress_port_list,0,FIXS_ITH_ARRAY_SIZE(egress_port_list));

	call_result = get_vlan_ports(egress_port_list, vlan_id, fixs_ith_snmp::TAGGED, switch_board, sb_plane, timeout_ms);
	if(call_result != fixs_ith_snmp::ERR_SNMP_NO_ERRORS)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "CANNOT get 'VLAN EGRESS Ports' of Vlan '%d' from switch in the magazine '%u' slot '%d' on the %s plane",
						vlan_id, switch_board.magazine, switch_board.slot_position, (fixs_ith::CONTROL_PLANE == sb_plane)? "CONTROL": "TRANSPORT");
		return call_result;
	}

	for(size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(tagged_port_list); ++i) {
		tagged_port_list[i] = egress_port_list[i] & (~untagged_port_list[i]);
			FIXS_ITH_LOG(LOG_LEVEL_TRACE, "tagged_port_list[%d] == %02x ", i, tagged_port_list[i]);
	}

	return fixs_ith::ERR_NO_ERRORS;
}


// END VLAN operations


// BridgePort operations

int fixs_ith_snmpmanager::get_port_default_vlan(fixs_ith_snmp::qBridge::pvid_t & pvid,
			uint16_t port_ifIndex,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms)
{

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "PORT DEFAULT VLAN get info: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_port_default_vlan(pvid,port_ifIndex, switch_board.slot_position, switch_board.magazine,switch_board_ipn_str_addresses[ipn_index],timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Port physical address get info: Call 'get_port_default_vlan' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}
int fixs_ith_snmpmanager::get_port_default_vlan(fixs_ith_snmp::qBridge::pvid_t & pvid, uint16_t port_number, int32_t switch_board_slot, uint32_t switch_board_magazine, const char * switch_board_ip, const unsigned * timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send get default vlan id  request to switch board %s in slot %d", switch_board_ip, switch_board_slot);

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	//int default_vlan_id = 0;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_getdefaultVlanidcallback callback(pvid, port_number, mag_str, switch_board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_name [] = { FIXS_ITH_SMX_OID_PORTVLAN_DOT1QPVID, port_number };

	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending the PDU GET request to retrieve the default Vlan Id of interface == %d into the OID '%s'", port_number, oid_str);

	call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU GET request to retrieve default vlan id");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve the default vlan id: call_result == %d",
				call_result);
	}

	if (call_result) { session_handler.close(); return call_result; }

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Callback detected error on reading the default vlan id from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	session_handler.close();

	return call_result;
}

int fixs_ith_snmpmanager::get_number_of_ports (
		int & num_of_ports,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	switch(sb_plane)
	{
	case CONTROL_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	case TRANSPORT_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	default:
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "NUMBER OF PORTS get info: on switch having IPNA == %s and IPNB == %s ", switch_board.control_ipna_str, switch_board.control_ipnb_str);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_number_of_ports(num_of_ports, switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"NUMBER OF PORTS get info: Call 'get_number_of_ports' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_number_of_ports (
					int & num_of_ports,
					int32_t switch_board_slot,
					uint32_t switch_board_magazine,
					const char * switch_board_ip,
					const unsigned * timeout_ms)
{
	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NUMBER OF PORTS get info: Trying to get number of ports from switch board '%s'", switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_number_of_ports_callback callback(num_of_ports,fixs_ith_snmp::BRIDGEPORT, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NUMBER OF PORTS get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_num_of_port [] = {FIXS_ITH_SMX_OID_BRIDGE_NUM_PORTS, 0 };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_num_of_port; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_num_of_port);

	call_result = get_number_of_ports(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}

int fixs_ith_snmpmanager::get_number_of_ports (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NUM of PORTs get info: Sending the PDU GET request to retrieve the the number of ports into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NUM of PORTs get info: Timeout expired while sending the PDU GET request to retrieve the number of ports");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"NUM of PORTs get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve the number of ports: call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NUM of PORTs get info: Callback detected error on reading the number of ports from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_port_name(char (& port_name)[fixs_ith_snmp::IF_NAME_MAX_LEN],
		uint16_t port_ifIndex,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "PORT name get info: on SMX having IPNA == %s and IPNB == %s ", switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_port_name(port_name,port_ifIndex,switch_board.slot_position,switch_board.magazine, switch_board_ipn_str_addresses[ipn_index],timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Port name get info: Call 'get_port_name' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_port_name(char (& port_name) [fixs_ith_snmp::IF_NAME_MAX_LEN],
		uint16_t port_ifIndex,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send get port name request for interface  %d to switch board %s in slot %d", port_ifIndex, switch_board_ip, switch_board_slot);

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_getportNamecallback callback(port_name, port_ifIndex, mag_str, switch_board_slot, switch_board_ip);
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_name [] = { SNMP_OID_IF_NAME, port_ifIndex };

	size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending the PDU GET request to retrieve the Port Name of interface == %d into the OID '%s'", port_ifIndex, oid_str);

	call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU GET request to retrieve port name");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve the port name: call_result == %d",
				call_result);
	}

	if (call_result) { session_handler.close(); return call_result; }

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Callback detected error on reading the  port name from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	session_handler.close();

	return call_result;
}

int fixs_ith_snmpmanager::get_eth_max_frame_size (
		uint32_t & eth_frame_size,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	switch(sb_plane)
	{
	case CONTROL_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	case TRANSPORT_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	default:
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "MAX FRAME SIZE get info: [IF_INDEX = %d] on switch having IPNA == %s and IPNB == %s ", if_index, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_eth_max_frame_size(eth_frame_size, if_index, switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"MAX FRAME SIZE get info: Call 'get_eth_max_frame_size' failed: if_index == %d, switch board slot == %d, switch board IPN = '%s': call_result == %d",
					if_index, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_eth_max_frame_size(
		uint32_t & eth_frame_size,
		uint16_t if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "MAX FRAME SIZE get info: Trying to get 'interface max frame size' from switch '%s'",
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_eth_frame_size_callback callback(eth_frame_size, if_index, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "MAX FRAME SIZE get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_eth_frame_size [] = { IF_MAX_FRAME_SIZE, if_index };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_eth_frame_size; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_eth_frame_size);

	call_result = get_eth_max_frame_size(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_eth_max_frame_size (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "MAX FRAME SIZE get info: Sending the PDU GET request to retrieve the 'max frame size' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "MAX FRAME SIZE get info: Timeout expired while sending the PDU GET request to retrieve 'max frame size'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"MAX FRAME SIZE get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve 'max frame size': call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "MAX FRAME SIZE get info: Callback detected error on reading 'max frame size' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_eth_duplex_status (
		uint16_t & eth_duplex_status,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "ETH DUPLEX STATUS get info: [IF_INDEX = %d] on switch having IPNA == %s and IPNB == %s ", if_index, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_eth_duplex_status(eth_duplex_status, if_index, switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"ETH DUPLEX STATUS get info: Call 'get_eth_duplex_status' failed: if_index == %d, switch board slot == %d, switch board IPN = '%s': call_result == %d",
					if_index, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}
int fixs_ith_snmpmanager::get_eth_duplex_status(
		uint16_t & eth_duplex_status,
		uint16_t if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ETH DUPLEX STATUS get info: Trying to get 'interface duplex status' from switch '%s'",
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_eth_duplex_status_callback callback(eth_duplex_status, if_index, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ETH DUPLEX STATUS get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_eth_duplex_status [] = { SNMP_DOT3_STATSDUPLEX_STATUS, if_index };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_eth_duplex_status; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_eth_duplex_status);

	call_result = get_eth_duplex_status(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_eth_duplex_status(
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ETH DUPLEX STATUS get info: Sending the PDU GET request to retrieve the 'eth_duplex_status' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ETH DUPLEX STATUS get info: Timeout expired while sending the PDU GET request to retrieve 'eth_duplex_status'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ETH DUPLEX STATUS get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve 'eth_duplex_status': call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ETH DUPLEX STATUS get info: Callback detected error on reading 'eth_duplex_status' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

int fixs_ith_snmpmanager::get_phy_masterslave_status (
		uint16_t & masterslave_status,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "PHY MASTER/SLAVE STATUS get info: [IF_INDEX = %d] on switch having IPNA == %s and IPNB == %s ", if_index, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_phy_masterslave_status(masterslave_status, if_index, switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"ETH DUPLEX STATUS get info: Call 'get_phy_masterslave_status' failed: if_index == %d, switch board slot == %d, switch board IPN = '%s': call_result == %d",
					if_index, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}
int fixs_ith_snmpmanager::get_phy_masterslave_status(
		uint16_t & masterslave_status,
		uint16_t if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "PHY MASTER/SLAVE STATUS get info: Trying to get 'phy master slave status' from switch '%s'",
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_phyMasterSlave_status_callback callback(masterslave_status, if_index, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PHY MASTER/SLAVE STATUS get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_phy_masterslave_status [] = { SNMP_GEN_EXTPHY_MASTERSLAVE_STATUS, if_index };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_phy_masterslave_status; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_phy_masterslave_status);

	call_result = get_phy_masterslave_status(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_phy_masterslave_status(
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "PHY MASTER SLAVE STATUS get info: Sending the PDU GET request to retrieve the 'phy_master_slave_status' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PHY MASTER SLAVE STATUS get info: Timeout expired while sending the PDU GET request to retrieve 'phy_master_slave_status'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"PHY MASTER SLAVE STATUS get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve 'phy_master_slave_status': call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PHY MASTER SLAVE STATUS  get info: Callback detected error on reading 'phy_master_slave_status' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}

//////////////////////////////////////////////////////////////

int fixs_ith_snmpmanager::get_agg_actor_admin_key (
		int32_t & agg_actor_admin_key,
		uint16_t if_index,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		const unsigned * timeout_ms)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	// get target SNMP agent IPs
	char switch_board_ipn_str_addresses[FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	switch(sb_plane)
	{
	case CONTROL_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	case TRANSPORT_PLANE_MASK:
		::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
		::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
		break;
	default:
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "aggActorAdminKey get info: [IF_INDEX = %d] on switch having IPNA == %s and IPNB == %s ", if_index, switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = get_agg_actor_admin_key(agg_actor_admin_key, if_index, switch_board.slot_position, switch_board.magazine, switch_board_ipn_str_addresses[ipn_index], timeout_ms);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"aggActorAdminKey get info: Call 'get_agg_actor_admin_key' failed: if_index == %d, switch board slot == %d, switch board IPN = '%s': call_result == %d",
					if_index, switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}


int fixs_ith_snmpmanager::get_agg_actor_admin_key(
		int32_t & agg_actor_admin_key,
		uint16_t if_index,
		int32_t switch_board_slot,
		uint32_t switch_board_magazine,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{

	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "aggActorAdminKey get info: Trying to get 'interface max frame size' from switch '%s'",
			switch_board_ip);

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, switch_board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_get_agg_actor_admin_key_callback callback(agg_actor_admin_key, if_index, mag_str, switch_board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "aggActorAdminKey get info:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	oid oid_agg_actor_admin_key [] = { FIXS_ITH_SMX_OID_LAG_PORT_ACTOR_ADMIN_KEY, if_index };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_agg_actor_admin_key; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_agg_actor_admin_key);

	call_result = get_agg_actor_admin_key(session_handler, callback, oid_name, oid_name_size, timeout_ms);

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_agg_actor_admin_key (
		fixs_ith_snmpsessionhandler & session_handler,
		fixs_ith_snmpsessioncallback & callback,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms) {

	char oid_str [512];

	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "aggActorAdminKey get info: Sending the PDU GET request to retrieve the 'aggActorAdminKey' value into the OID '%s'", oid_str);

	int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "aggActorAdminKey get info: Timeout expired while sending the PDU GET request to retrieve 'aggActorAdminKey'");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"aggActorAdminKey get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve 'aggActorAdminKey': call_result == %d",
				call_result);
	}

	if (call_result) return call_result;

	// Here a possible error in pdu data should be verified
	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "aggActorAdminKey get info: Callback detected error on reading 'interface speed' from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	return call_result;
}
// END BridgePort operations

template <class Type>
int fixs_ith_snmpmanager::set_mib_object (fixs_ith_snmpsessionhandler & session_handler, oid * oid_name ,size_t oid_size,Type oidValue)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char oid_str [512];

	netsnmp_pdu * snmp_pdu = 0;

	// Create the PDU for GET
	if (const int call_result = create_snmp_pdu(snmp_pdu, SNMP_MSG_SET)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_snmp_pdu' failed: call_result == %d", call_result);
		return call_result;
	}
	// Add the OID name and value into the PDU request
	netsnmp_variable_list * snmp_result = 0;
	if(typeid(oidValue)== typeid(int)){
		snmp_result = snmp_pdu_add_variable(snmp_pdu, oid_name, oid_size,ASN_INTEGER, reinterpret_cast<u_char *>(&oidValue), sizeof(oidValue));
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "set_mib_object: Sending  SET  (oid: %s)(value = %d) request...",oid_str, oidValue);
	}
	else if(typeid(oidValue)== typeid(uint32_t)){
		snmp_result = snmp_pdu_add_variable(snmp_pdu, oid_name, oid_size,ASN_UNSIGNED, reinterpret_cast<u_char *>(&oidValue), sizeof(oidValue));
		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_size);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "set_mib_object: Sending  SET  (oid: %s)(value = %u) request...",oid_str, oidValue);
	}
//	else if(typeid(oidValue)== typeid(char*)){
//		snmp_result = snmp_pdu_add_variable(snmp_pdu, oid_name, oid_size,ASN_OCTET_STR, reinterpret_cast<u_char *>(oidValue),::strlen((const char*)oidValue));
//		oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_size);
//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "set_mib_object: Sending SET (oid: %s)(value = %s) request...",oid_str, oidValue);
//	}
	else
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "set_mib_object Failed: Unsupported TYPE %s of oidValue %d",typeid(oidValue).name(), oidValue);
		return fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_BAD_TYPE;
	}


	if (!snmp_result) {
		snmp_free_pdu(snmp_pdu);
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'snmp_pdu_add_variable' failed in NET-SNMP library");
		return fixs_ith_snmp::ERR_SNMP_PDU_ADD_VARIABLE_VALUE;
	}

	{
		unsigned timeout_ms = 2000; // 2 seconds timeout
		errno=0;
		int call_result = session_handler.send_synch(snmp_pdu, &timeout_ms); // 2 sec timeout
		int errno_save = errno;
		snmp_free_pdu(snmp_pdu);

		if (call_result) { // ERROR: sending the request
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR,
					"LAG configure: Call 'send_synch' failed: NET-SNMP library internal error on sending the request PDU: call_result == %d", call_result);
			return fixs_ith_snmp::ERR_SNMP_PDU_CREATE;
		}

		return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	}
}

//template <>
//int fixs_ith_snmpmanager::set_mib_object (fixs_ith_snmpsessionhandler & session_handler, oid * oid_name ,size_t oid_size,char * oidValue)
//{
//	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
//
//	char oid_str [512];
//
//	netsnmp_pdu * snmp_pdu = 0;
//
//	// Create the PDU for GET
//	if (const int call_result = create_snmp_pdu(snmp_pdu, SNMP_MSG_SET)) {
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_snmp_pdu' failed: call_result == %d", call_result);
//		return call_result;
//	}
//	// Add the OID name and value into the PDU request
//	netsnmp_variable_list * snmp_result = 0;
//
//
//	snmp_result = snmp_pdu_add_variable(snmp_pdu, oid_name, oid_size,ASN_OCTET_STR, reinterpret_cast<u_char *>(oidValue),::strlen((const char*)oidValue));
//	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_size);
//	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "set_mib_object: Sending SET (oid: %s)(value = %s) request...",oid_str, oidValue);
//
//	if (!snmp_result) {
//		snmp_free_pdu(snmp_pdu);
//		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'snmp_pdu_add_variable' failed in NET-SNMP library");
//		return fixs_ith_snmp::ERR_SNMP_PDU_ADD_VARIABLE_VALUE;
//	}
//
//	{
//		unsigned timeout_ms = 2000; // 2 seconds timeout
//		errno=0;
//		int call_result = session_handler.send_synch(snmp_pdu, &timeout_ms); // 2 sec timeout
//		int errno_save = errno;
//		snmp_free_pdu(snmp_pdu);
//
//		if (call_result) { // ERROR: sending the request
//			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR,
//					"LAG configure: Call 'send_synch' failed: NET-SNMP library internal error on sending the request PDU: call_result == %d", call_result);
//			return fixs_ith_snmp::ERR_SNMP_PDU_CREATE;
//		}
//
//		return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
//	}
//}
//




int fixs_ith_snmpmanager::create_ipv4subnet(const fixs_ith_switchboardinfo & switch_board, const fixs_ith::switchboard_plane_t sb_plane, const std::string  & ipv4subnet)
{
	int retcode = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1], switch_board,sb_plane))
		return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating IPV4 subnet '%s' on SMX BRIDGE having IPNA == %s and IPNB == %s ",
			ipv4subnet.c_str(), switch_board.control_ipna_str, switch_board.control_ipnb_str);

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = create_ipv4subnet(switch_board_ipn_str_addresses[ipn_index], ipv4subnet);
		if (call_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"IPV4 SUBNET {%s} Create: Call 'create_ipv4subnet' failed on IPN = '%s': call_result == %d",
					ipv4subnet.c_str(), switch_board_ipn_str_addresses[ipn_index], call_result);
		}
	}

	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable create the SUBNET '%s' on SMX bridge having IPNA == %s and IPNB == %s. "
						"Call 'create_ipv4subnet()' failed on both IPNS !",
						ipv4subnet.c_str(), switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

		retcode = fixs_ith_snmp::ERR_SNMP_SUBNET_CREATE;
	}
	else
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Successfully created IPV4 subnet '%s' on SMX BRIDGE having IPNA == %s and IPNB == %s ",
				ipv4subnet.c_str(), switch_board.control_ipna_str, switch_board.control_ipnb_str);
	}

	return retcode;
}


int fixs_ith_snmpmanager::build_ipv4subnet_index_oid(ipv4subnet_index_oid_t & ipv4subnet_index_oid, const char * ipv4subnet)
{

	if(!ipv4subnet)
		return fixs_ith_snmp::ERR_SNMP_INVALID_SUBNET;

	// Initialize the output parameter 'ipv4subnet_index_oid'
	ipv4subnet_index_oid[0] = 4;   // number of octets in an ipv4 type of address
	ipv4subnet_index_oid[4] = ipv4subnet_index_oid[3]= ipv4subnet_index_oid[2] = ipv4subnet_index_oid[1] = 0;  /* default ipv4 address  (0.0.0.0) */
	ipv4subnet_index_oid[5] = 4;   // number of octets in an ipv4 type of mask
	ipv4subnet_index_oid[9] = ipv4subnet_index_oid[8]= ipv4subnet_index_oid[7] = ipv4subnet_index_oid[6] = 255;  /* default ipv4 mask  (255.255.255.255) */
	ipv4subnet_index_oid[10] = 1;  // SNMP value indicating 'ipv4' address type

	// Parse the ipv4subnet string and fill the "IPv4 address" part of the output parameter
	unsigned int  subnetmask_prefix_length = 0;
	int sscanf_res = ::sscanf(ipv4subnet,"%lu.%lu.%lu.%lu/%u", ipv4subnet_index_oid + 1, ipv4subnet_index_oid + 2, ipv4subnet_index_oid + 3, ipv4subnet_index_oid + 4, &  subnetmask_prefix_length);
	if((sscanf_res != 5) || subnetmask_prefix_length > 32)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to build IPv4 SUBNET index OID from '%s' [ INVALID SUBNET ] ", ipv4subnet);
		return  fixs_ith_snmp::ERR_SNMP_INVALID_SUBNET;
	}

	// Convert the IPv4 subnet mask prefix length to XXX.XXX.XXX.XXX format and fill the 'subnet mask' part of the output parameter
	uint32_t mask = ::htonl(0xffffffff << (32 - subnetmask_prefix_length));
	uint8_t * p = reinterpret_cast<uint8_t*>(& mask);
	for(int i = 0; i < 4; ++i)
		ipv4subnet_index_oid[6 + i] = p[i];	// 6 is the starting position of "subnet mask" in 'ipv4subnet_oid_suffix'

	for(unsigned int i = 0; i < FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid); ++i)
	{
		FIXS_ITH_LOG(LOG_LEVEL_TRACE, "ipv4subnet_index_oid [%u] == %lu ", i, ipv4subnet_index_oid[i]);
	}

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}


int fixs_ith_snmpmanager::create_ipv4subnet(const char * switch_board_ip, const std::string  & ipv4subnet)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (*switch_board_ip == 0 ) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'create ipv4subnet' request because switch board IP is empty '%s'", switch_board_ip);
		return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send 'create IPv4 subnet' request to '%s'", switch_board_ip);

	int call_result = 0;

	fixs_ith_snmploggercallback callback("SNMP Response received for 'Create IPv4 Subnet' request towards SMX");
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	ipv4subnet_index_oid_t ipv4subnet_index_oid;
	if(const int ret_code = build_ipv4subnet_index_oid(ipv4subnet_index_oid, ipv4subnet.c_str()))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_ipv4subnet_index_oid' failed ret_code == %d", ret_code);
		return ret_code;
	}

	// Setting genSubnetVlanRowStatus
	oid base_oid [] = { SNMP_GEN_SUBNET_VLAN_ROW_STATUS, 0 };
	oid oid_row_status [FIXS_ITH_ARRAY_SIZE(base_oid) + FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid) ] = {0};

	for(unsigned int i = 0; i < FIXS_ITH_ARRAY_SIZE(base_oid); ++i) {
		oid_row_status[i] = base_oid[i];
	}
	for(unsigned int j = 0; j < FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid); ++j) {
		oid_row_status[j + FIXS_ITH_ARRAY_SIZE(base_oid)] = ipv4subnet_index_oid[j];
	}

	size_t oid_size = FIXS_ITH_ARRAY_SIZE(oid_row_status);
	int oidValue = fixs_ith_snmp::snmptarget::CREATE_AND_WAIT;

	char oid_str[512];
	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_row_status, oid_size);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IPV4 Subnet create: setting genSubnetVlanRowStatus (oid: %s)(value = %d) request...",oid_str, oidValue);

	if((call_result = set_mib_object<int>( session_handler, oid_row_status,oid_size, oidValue))){

		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_mib_object' failed: call_result == %d", call_result);
		session_handler.close();
		return call_result;
	}
	// SNMP set request sent. Now check PDU response to see operation result
	if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_mib_object()' reported an error: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
		session_handler.close();
		return call_result;
	}

	session_handler.close();

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

}

int fixs_ith_snmpmanager::delete_ipv4subnet(const fixs_ith_switchboardinfo & switch_board,
		const fixs_ith::switchboard_plane_t sb_plane,
		const std::string & ipv4subnet)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Deleting IPV4 Subnet on SMX having IPNA == %s and IPNB == %s ",switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = delete_ipv4subnet(switch_board_ipn_str_addresses[ipn_index], ipv4subnet);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Delete IPV4 Subnet : Call 'delete_ipv4subnet' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'delete_ipv4subnet' failed on both ipns for switch board in slot %d",
				switch_board.slot_position);
	}

	return call_result;
}


int fixs_ith_snmpmanager::set_ipv4subnet_vid(const fixs_ith_switchboardinfo & switch_board,
		const fixs_ith::switchboard_plane_t sb_plane,
		const std::string  & ipv4subnet,
		const uint16_t vid)
{

	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Setting vlan id %d for IPV4 Subnet %s on SMX having IPNA == %s and IPNB == %s ",
			vid, ipv4subnet.c_str(), switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		call_result = set_ipv4subnet_vid(switch_board_ipn_str_addresses[ipn_index], ipv4subnet, vid);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Set IPV4 Subnet vlan id  : Call 'set_ipv4subnet_vid' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}
	if (call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'set_ipv4subnet_vid' failed on both ipns for switch board in slot %d",
				switch_board.slot_position);
	}

	return call_result;
}

int fixs_ith_snmpmanager::delete_ipv4subnet(const char * switch_board_ip, const std::string & ipv4subnet)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (*switch_board_ip == 0 ) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cannot send 'delete ipv4 subnet' request because switch board IP is empty '%s'", switch_board_ip);
		return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send 'delete IPV4 SUBNET' request to switch board %s", switch_board_ip);

	int call_result = 0;

	fixs_ith_snmploggercallback callback("SNMP Response received for 'delete ipv4 subnet' request towards SMX");
	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	ipv4subnet_index_oid_t ipv4subnet_index_oid;
	if(const int ret_code = build_ipv4subnet_index_oid(ipv4subnet_index_oid, ipv4subnet.c_str()))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_ipv4subnet_index_oid' failed ret_code == %d", ret_code);
		return ret_code;
	}

	// Setting genSubnetVlanRowStatus
	oid base_oid [] = { SNMP_GEN_SUBNET_VLAN_ROW_STATUS, 0 };
	oid oid_row_status [FIXS_ITH_ARRAY_SIZE(base_oid) + FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid) ] = {0};

	for(unsigned int i = 0; i < FIXS_ITH_ARRAY_SIZE(base_oid); ++i) {
		oid_row_status[i] = base_oid[i];
	}
	for(unsigned int j = 0; j < FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid); ++j) {
		oid_row_status[j + FIXS_ITH_ARRAY_SIZE(base_oid)] = ipv4subnet_index_oid[j];
	}

	size_t oid_size = FIXS_ITH_ARRAY_SIZE(oid_row_status);

	char oid_str[512];
	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_row_status, oid_size);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IPV4 Subnet delete : setting genSubnetVlanRowStatus (oid: %s) request...",oid_str);
    int oidValue = fixs_ith_snmp::snmptarget::DESTROY;
	if ((call_result = set_mib_object(session_handler, oid_row_status, oid_size, oidValue))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_mib_object' failed: call_result == %d", call_result);
	}
	else {
		// SNMP set request sent. Now check PDU response to get operation result
		if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_mib_object()' reported an error: "
										  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
		}
	}

	session_handler.close();
	return call_result;
}

int fixs_ith_snmpmanager::set_ipv4subnet_vid(const char * switch_board_ip, const std::string & ipv4subnet,const uint16_t vid)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (*switch_board_ip == 0 ) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cannot send 'set vid for ipv4 subnet' request because switch board IP is empty '%s'", switch_board_ip);
		return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send 'set vid for ipv4 subnet' request to switch board %s", switch_board_ip);

	int call_result = 0;

	fixs_ith_snmploggercallback callback("SNMP Response received for 'set vid for ipv4 subnet' request towards SMX");
	fixs_ith_snmpsessionhandler session_handler;

	// open SNMP session
	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	// prepare the "index" part of the IPv4 subnet OID
	ipv4subnet_index_oid_t ipv4subnet_index_oid;
	if(const int ret_code = build_ipv4subnet_index_oid(ipv4subnet_index_oid, ipv4subnet.c_str()))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_ipv4subnet_index_oid' failed ret_code == %d", ret_code);
		return ret_code;
	}

	// Prepare for setting 'genSubnetVlanVid' and 'genSubnetVlanRowStatus'
	oid base_oid_genSubnetVlanVid [] = { SNMP_GEN_SUBNET_VLAN_ID, 0 };
	oid oid_genSubnetVlanVid[FIXS_ITH_ARRAY_SIZE(base_oid_genSubnetVlanVid) + FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid) ] = {0};
	oid base_oid_genSubnetVlanRowStatus [] = { SNMP_GEN_SUBNET_VLAN_ROW_STATUS, 0 };
	oid oid_genSubnetVlanRowStatus [FIXS_ITH_ARRAY_SIZE(base_oid_genSubnetVlanRowStatus) + FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid) ] = {0};

	for(unsigned int i = 0; i < FIXS_ITH_ARRAY_SIZE(base_oid_genSubnetVlanVid); ++i) {
		oid_genSubnetVlanVid[i] = base_oid_genSubnetVlanVid[i];
	}
	for(unsigned int j = 0; j < FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid); ++j) {
		oid_genSubnetVlanVid[j + FIXS_ITH_ARRAY_SIZE(base_oid_genSubnetVlanVid)] = ipv4subnet_index_oid[j];
	}

	for(unsigned int i = 0; i < FIXS_ITH_ARRAY_SIZE(base_oid_genSubnetVlanRowStatus); ++i) {
		oid_genSubnetVlanRowStatus[i] = base_oid_genSubnetVlanRowStatus[i];
	}
	for(unsigned int j = 0; j < FIXS_ITH_ARRAY_SIZE(ipv4subnet_index_oid); ++j) {
		oid_genSubnetVlanRowStatus[j + FIXS_ITH_ARRAY_SIZE(base_oid_genSubnetVlanRowStatus)] = ipv4subnet_index_oid[j];
	}

	size_t oid_genSubnetVlanVid_size = FIXS_ITH_ARRAY_SIZE(oid_genSubnetVlanVid);
	int oid_genSubnetVlanVid_value = (int) vid;

	size_t oid_genSubnetVlanRowStatus_size = FIXS_ITH_ARRAY_SIZE(oid_genSubnetVlanRowStatus);
	int oid_genSubnetVlanRowStatus_value = fixs_ith_snmp::snmptarget::ACTIVE;

	// Step 1: set 'genSubnetVlainVid'
	if ((call_result = set_mib_object(session_handler, oid_genSubnetVlanVid, oid_genSubnetVlanVid_size, oid_genSubnetVlanVid_value))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to set 'getSubnetVlanVid'. Call 'set_mib_object' failed: call_result == %d", call_result);
	}
	else {
		// SNMP set request sent. Now check PDU response to get operation result
		if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_mib_object()' reported an error: "
										  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
			call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
		}
	}

	if(call_result == 0) {
		// Step 2 : set 'genSubnetVlanRowStatus'
		if ((call_result = set_mib_object(session_handler, oid_genSubnetVlanRowStatus, oid_genSubnetVlanRowStatus_size, oid_genSubnetVlanRowStatus_value))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to set 'oid_genSubnetVlanRowStatus'. Call 'set_mib_object' failed: call_result == %d", call_result);
		}
		else {
			// SNMP set request sent. Now check PDU response to get operation result
			if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_mib_object()' reported an error: "
											  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
				call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
			}
		}
	}

	session_handler.close();
	return call_result;
}


int fixs_ith_snmpmanager::reset_userpasswd(const fixs_ith_switchboardinfo & switch_board, const uint16_t userType  )
{
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

        fixs_ith::switchboard_plane_t sb_plane;
     
        if(common::utility::isSwitchBoardSMX())
        {
           sb_plane = fixs_ith::CONTROL_PLANE;
        }

        if(common::utility::isSwitchBoardCMX())
        {
           sb_plane = fixs_ith::TRANSPORT_PLANE;
        }
       


	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
			switch_board_ipn_str_addresses[1],
			switch_board, sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Reset User Password [userType == %d] on SMX having IPNA == %s and IPNB == %s", userType,
			switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
	{
		call_result = reset_userpasswd(switch_board_ipn_str_addresses[ipn_index], userType);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Reset User Password : Call 'reset_userpasswd' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
					switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
	}

	return call_result;
}


int fixs_ith_snmpmanager::reset_userpasswd(const char * switch_board_ip, const uint16_t userType)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

		if (*switch_board_ip == 0 ) {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Cannot send 'reset_userpasswd' request because switch board IP is empty '%s'", switch_board_ip);
			return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send 'reset_userpasswd' request to switch board %s", switch_board_ip);

		// Prepare the oid arrays
		oid oid_name [] = {FIXS_ITH_SMX_OID_USER_RESETPWD, userType };
		size_t oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_name);
		int oid_value = 1;

		int call_result = set_port_mib_object<int>(const_cast<char *>(switch_board_ip), oid_name, oid_name_size, oid_value);

		return call_result;
}

int fixs_ith_snmpmanager::check_ipv4subnet(const fixs_ith_switchboardinfo & switch_board,
		const fixs_ith::switchboard_plane_t sb_plane,
		const std::string  & ipv4subnet,
		const unsigned * timeout_ms)
{
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1], switch_board, sb_plane))
		return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

	// prepare elements to build the OID to be checked
	oid base_oid [] = { SNMP_GEN_SUBNET_VLAN_ROW_STATUS, 0 };
	ipv4subnet_index_oid_t subnet_index_oid;
	build_ipv4subnet_index_oid(subnet_index_oid, ipv4subnet.c_str());

	// now ready to build the OID to be checked
	oid oid_row_status [FIXS_ITH_ARRAY_SIZE(base_oid) + FIXS_ITH_ARRAY_SIZE(subnet_index_oid) ] = {0};
	for(unsigned int i = 0; i < FIXS_ITH_ARRAY_SIZE(base_oid); ++i) {
		oid_row_status[i] = base_oid[i];
	}
	for(unsigned int j = 0; j < FIXS_ITH_ARRAY_SIZE(subnet_index_oid); ++j) {
		oid_row_status[j + FIXS_ITH_ARRAY_SIZE(base_oid)] = subnet_index_oid[j];
	}
	size_t oid_size = FIXS_ITH_ARRAY_SIZE(oid_row_status);

	// check OID existence. Try first on IPNA
	int check_oid_res = check_oid(oid_row_status, oid_size, switch_board.magazine, switch_board.slot_position, switch_board_ipn_str_addresses[0], timeout_ms);
	if(check_oid_res  &&  (check_oid_res !=  fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE))
	{
	   // failure ! Try on IPNB
	   check_oid_res =  check_oid(oid_row_status, oid_size, switch_board.magazine, switch_board.slot_position, switch_board_ipn_str_addresses[1], timeout_ms);
	}

	return  check_oid_res;
}


// SNMP Static methods

int fixs_ith_snmpmanager::get_ifIndex_bitmap( const std::set<uint16_t>& interface_list, fixs_ith_snmp::if_bitmap_t & ifIndex_bitmap)
{
	uint16_t interfaceId;
	for (std::set<uint16_t>::const_iterator it = interface_list.begin() ; it != interface_list.end(); ++it)
	{
		interfaceId = *it;
		unsigned int shift_bits_num = interfaceId % 8;
		unsigned int hexbyte_pos = (shift_bits_num > 0)? interfaceId / 8 : interfaceId / 8 - 1;
		unsigned char bytemask = 0x80;
		unsigned char hexbyte_value = (shift_bits_num > 0)? bytemask >> (shift_bits_num -1): bytemask >> 7;

//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "get_ifIndex_bitmap():  Interface == %u shift_bits_num == %u hexbyte_pos == %d hexbyte_value == %02x",
//				interfaceId , shift_bits_num, hexbyte_pos, hexbyte_value);
		if (hexbyte_pos  >= SWITCH_IF_BITMAP_SIZE) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "get_ifIndex_bitmap(): Interface == %u is OUT of allowed RANGE [1 .. %u]. Details: [shift_bits_num == %u hexbyte_pos == %d hexbyte_value == %02x",interfaceId , 8 * SWITCH_IF_BITMAP_SIZE, shift_bits_num, hexbyte_pos, hexbyte_value);
				return -1;
			}
		ifIndex_bitmap[hexbyte_pos] |= hexbyte_value;

	}
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "get_ifIndex_bitmap(): ");

	for(int i=0;i<SWITCH_IF_BITMAP_SIZE;i++)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ifIndex_bitmap[%d] == %02x ", i, ifIndex_bitmap[i]);
	}

    return 0;
}

/* BEGIN: get_ifIndex_bitmap for CMX */

int fixs_ith_snmpmanager::get_ifIndex_bitmap( const std::set<uint16_t>& interface_list, fixs_ith_snmp::if_bitmap_t_cmx & ifIndex_bitmap)
{
        uint16_t interfaceId;
        for (std::set<uint16_t>::const_iterator it = interface_list.begin() ; it != interface_list.end(); ++it)
        {
                interfaceId = *it;
                unsigned int shift_bits_num = interfaceId % 8;
                unsigned int hexbyte_pos = (shift_bits_num > 0)? interfaceId / 8 : interfaceId / 8 - 1;
                unsigned char bytemask = 0x80;
                unsigned char hexbyte_value = (shift_bits_num > 0)? bytemask >> (shift_bits_num -1): bytemask >> 7;

//              FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "get_ifIndex_bitmap():  Interface == %u shift_bits_num == %u hexbyte_pos == %d hexbyte_value == %02x",
//                              interfaceId , shift_bits_num, hexbyte_pos, hexbyte_value);

		if (hexbyte_pos  >= SWITCH_IF_BITMAP_SIZE_CMX) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "get_ifIndex_bitmap(): Interface == %u is OUT of allowed RANGE [1 .. %u]. Details: [shift_bits_num == %u hexbyte_pos == %d hexbyte_value == %02x",interfaceId , 8 * SWITCH_IF_BITMAP_SIZE_CMX, shift_bits_num, hexbyte_pos, hexbyte_value);
                                return -1;
		}
		ifIndex_bitmap[hexbyte_pos] |= hexbyte_value;

        }
        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "get_ifIndex_bitmap(): ");
	for(int i=0;i<SWITCH_IF_BITMAP_SIZE_CMX;i++)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ifIndex_bitmap[%d] == %02x ", i, ifIndex_bitmap[i]);
	}

    return 0;
}


/* END: get_ifIndex_bitmap for CMX */


//TODO :  eliminare da snmpmanager
int fixs_ith_snmpmanager::get_switch_board_target_ipns (char (& switch_board_ipna) [16], char (& switch_board_ipnb) [16], const fixs_ith_switchboardinfo & switch_board,fixs_ith::switchboard_plane_t sb_plane)
{
	switch(sb_plane){
	case fixs_ith::CONTROL_PLANE:
		::strncpy(switch_board_ipna, switch_board.control_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipna));
		::strncpy(switch_board_ipnb, switch_board.control_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipnb));
		break;
	case fixs_ith::TRANSPORT_PLANE:
		::strncpy(switch_board_ipna, switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipna));
		::strncpy(switch_board_ipnb, switch_board.transport_ipnb_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipnb));
		break;
	default:
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot get target ipns: passed switch board plane value (%d) is not correct ", sb_plane);
		return -1;
	}

	return 0;
} //+


int fixs_ith_snmpmanager::get_pimPresence_status (
		int16_t & port_presencestatus,
		uint32_t board_magazine,
		int32_t board_slot,
		const char * switch_board_ip,
		const unsigned * timeout_ms,int16_t interface_num)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send get PIM board presence status request to switch board %s in slot %d", switch_board_ip, board_slot);
	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_getpimpresencecallback callback(port_presencestatus, interface_num, mag_str, board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}
	uint16_t ifIndex = interface_num;
	oid oid_pim [] = { FIXS_ITH_SMX_OID_PIM_PRESENCE_STATUS, ifIndex };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_pim; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_pim);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending the PDU GET request to retrieve the pim presence status of interface == %d into the OID '%s'", interface_num, oid_pim);

	call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU GET request to retrieve pim presence status");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve pim presence status: call_result == %d",
				call_result);
	}

	if (call_result) { session_handler.close(); return call_result; }

	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Callback detected error on reading pim presence status from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	session_handler.close();

	return call_result;
}


int fixs_ith_snmpmanager::get_pimMatching_status (
		int16_t & port_matchingstatus,
		uint32_t board_magazine,
		int32_t board_slot,
		int16_t interface_num,
		const char * switch_board_ip,
		const unsigned * timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to send get PIM board presence status request to switch board %s in slot %d", switch_board_ip, board_slot);
	int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

	char mag_str[16] = {0};
	call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
	if(call_result < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
		return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
	}

	fixs_ith_getpimmatchingcallback callback(port_matchingstatus, interface_num, mag_str, board_slot, switch_board_ip);

	fixs_ith_snmpsessionhandler session_handler;

	if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
		return call_result;
	}

	uint16_t ifIndex = interface_num;
	oid oid_pim [] = { FIXS_ITH_SMX_OID_PIM_MATCHING_STATUS, ifIndex };

	const oid * oid_name = 0;
	size_t oid_name_size = 0;

	oid_name = oid_pim; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_pim);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending the PDU GET request to retrieve the pim matching status of interface == %d into the OID '%s'", interface_num, oid_pim);

	call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

	if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU GET request to retrieve pim matching status");
	} else if (call_result) { // ERROR: sending the request
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve pim matching status: call_result == %d",
				call_result);
	}

	if (call_result) { session_handler.close(); return call_result; }


	if (callback.error_code) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Callback detected error on reading pim matching status from response PDU: "
				"callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
		call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
	}

	session_handler.close();
	return call_result;
}

/* Get the Container File name of CMX */
int fixs_ith_snmpmanager::get_cont_file_name (
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane,
		uint16_t port_Index,
		std::string &fileName,
		const unsigned * timeout_ms)
{
        char switch_board_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

        if(get_switch_board_target_ipns(switch_board_ipn_addresses[0],
                        switch_board_ipn_addresses[1],
                        switch_board,sb_plane))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

        FIXS_ITH_LOG(LOG_LEVEL_INFO, "GET Container File info: on CMXB having IPNA == %s and IPNB == %s ", switch_board_ipn_addresses[0], switch_board_ipn_addresses[1]);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
                call_result = get_cont_file_name(port_Index,switch_board.magazine,switch_board.slot_position,switch_board_ipn_addresses[ipn_index],fileName,timeout_ms);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'get_cont_file_file_name' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_addresses[ipn_index], call_result);

        }

        return call_result;
}

int fixs_ith_snmpmanager::get_cont_file_name (
		uint16_t port_Index,
		uint32_t board_magazine,
		int32_t switch_board_slot,
		const char * switch_board_ip,
		std::string &fileName,
		const unsigned * timeout_ms)
{

        int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

        char mag_str[16] = {0};
        call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
        if(call_result < 0) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
                return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "get_cont_file_name:Trying to get the container file name at port_ifIndex: %d of from switch board '%s'",	       port_Index, switch_board_ip);

        fixs_ith_getcontainerfilename callback(fileName,port_Index,mag_str,switch_board_slot, switch_board_ip);
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "get_cont_file_name:Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        oid oid_cont_file_name [] = {FIXS_ITH_CMX_CONT_FILE_NAME, port_Index };

        const oid * oid_name = 0;
        size_t oid_name_size = 0;

        oid_name = oid_cont_file_name; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_cont_file_name);

        call_result = get_cont_file_name(session_handler, callback, oid_name, oid_name_size, timeout_ms);

        session_handler.close();

        return call_result;
}

int fixs_ith_snmpmanager::get_cont_file_name (
                        fixs_ith_snmpsessionhandler & session_handler,
                        fixs_ith_snmpsessioncallback & callback,
                        const oid * oid_name,
                        size_t oid_name_size,
                        const unsigned * timeout_ms) {
        char oid_str [512];

        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Fetching container file name: Sending the PDU GET request to retrieve the 'container_file_name' value into the OID '%s'", oid_str);

        int call_result = send_get_request(session_handler, oid_name, oid_name_size, timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Fetching container file name: Timeout expired while sending the PDU GET request to retrieve 'container file name'");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR,
                                "Container file name get info: Call 'send_get_request' failed: NET-SNMP library internal error on sending the GET request to retrieve "
                                "'get_cont_file_name': call_result == %d", call_result);
        }

        if (call_result) return call_result;

        // Here a possible error in pdu data should be verified
        if (callback.error_code) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Container file name get info: Callback detected error on reading 'container file name' from response PDU: "
                                "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
        }

        return call_result;

}




int fixs_ith_snmpmanager::delete_container_file(
		fixs_ith_switchboardinfo & switch_board,
		std::string file_name)

{
	char switch_board_ipn_str_addresses[2][16] = {};

	::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
	::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));
	
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "set_cont_file_to_delete: on CMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

	int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
	{
		call_result = set_cont_file_to_delete(switch_board_ipn_str_addresses[ipn_index], file_name);
		if (call_result)
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'set_cont_file_to_delete' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

	if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'set_cont_file_to_delete' failed on both ipns for switch board in slot %d",
                                switch_board.slot_position);
        }
        return call_result;
}

int fixs_ith_snmpmanager::set_cont_file_to_delete(
		const char * switch_board_ip,
		std::string file_name)
{

        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'set_cont_file_to_delete' request because switch board IP is empty '%s'", switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to delete container file request to switch board %s", switch_board_ip);

        int call_result = 0;

        fixs_ith_snmploggercallback callback("SNMP Response received for Container File deletion request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        unsigned timeout_ms = 2000;

        
                // Setting the container file name to delete
                oid oid_delete_cont_file [] = { FIXS_ITH_CMX_CONT_FILE_DELETE, 0 };

        const oid * oid_name = 0;
        size_t oid_name_size = 0;

        oid_name = oid_delete_cont_file; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_delete_cont_file);


                if ((call_result = set_delete_cont_file(file_name, session_handler,oid_name,oid_name_size, &timeout_ms))) {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_delete_cont_file' failed: call_result == %d", call_result);
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to see if the requested operation was successfully
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_delete_cont_file()' reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }
        

        session_handler.close();

        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_snmpmanager::set_delete_cont_file (
                                        std::string file_name,
                                        fixs_ith_snmpsessionhandler & session_handler,
                                        const oid * oid_name,
                                        size_t oid_name_size,
                                        const unsigned * timeout_ms
                                        )
{

        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Container File  to delete': OID '%s': value '%s'", oid_str, file_name.c_str());

        int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(file_name.c_str()), file_name.length(), timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
        }

        return call_result;
}


int fixs_ith_snmpmanager::create_cont_transfer_server_ip(
					fixs_ith_switchboardinfo & switch_board,
					const char * server_address)
{

        char switch_board_ipn_str_addresses[2][16] = {};

        ::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
        ::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

        FIXS_ITH_LOG(LOG_LEVEL_INFO, "create_cont_transfer_server_ip: on CMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
        {
                call_result = create_cont_transfer_server_ip(switch_board_ipn_str_addresses[ipn_index], server_address);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'create_cont_transfer_server_ip' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

        if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'create_cont_transfer_server_ip' failed on both ipns for switch board in slot %d",
                                switch_board.slot_position);
        }
        return call_result;

}

int fixs_ith_snmpmanager::create_cont_transfer_server_ip(
                const char * switch_board_ip,
                const char * server_address)
{

        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'create_cont_transfer_server_ip' request because switch board IP is empty '%s'", switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to set the transfer server ip address on switch board %s", switch_board_ip);

        int call_result = 0;

        fixs_ith_snmploggercallback callback("SNMP Response received for setting log transfer server ip address request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        unsigned timeout_ms = 2000;

	// Setting the container file name to delete
                oid oid_transfer_server_ip [] = { FIXS_ITH_CMX_CONT_TRANSFER_SRV_IP };

        	const oid * oid_name = 0;
        	size_t oid_name_size = 0;

        	oid_name = oid_transfer_server_ip; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_transfer_server_ip);



                if ((call_result = set_server_ip_address(server_address, session_handler,oid_name,oid_name_size,&timeout_ms))) {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_cont_transfer_server_ip' failed: call_result == %d", call_result);
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to see if the requested operation was successfully
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_delete_cont_file()' reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }


        session_handler.close();
        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_snmpmanager::set_server_ip_address (
					const char * server_address,
                                        fixs_ith_snmpsessionhandler & session_handler,
                                        const oid * oid_name,
                                        size_t oid_name_size,
                                        const unsigned * timeout_ms
                                        )
{

        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Set the transfer server IP': OID '%s': value '%s'", oid_str, server_address);

        int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(server_address), ::strlen(server_address), timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
        }

        return call_result;
}


int fixs_ith_snmpmanager::create_cont_transfer_server_path(
					fixs_ith_switchboardinfo & switch_board,
					const char *log_path)
{

        char switch_board_ipn_str_addresses[2][16] = {};

        ::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
        ::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

        FIXS_ITH_LOG(LOG_LEVEL_INFO, "create_cont_transfer_server_path: on CMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
        {
                call_result = create_cont_transfer_server_path(switch_board_ipn_str_addresses[ipn_index], log_path);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'create_cont_transfer_server_path' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

        if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'create_cont_transfer_server_path' failed on both ipns for switch board in slot %d",
                                switch_board.slot_position);
        }
        return call_result;
}

int fixs_ith_snmpmanager::create_cont_transfer_server_path(
                const char * switch_board_ip,
                const char * log_path)
{

        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'create_cont_transfer_server_path' request because switch board IP is empty '%s'", switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to set the logs transfer path on switch board %s", switch_board_ip);

        int call_result = 0;

        fixs_ith_snmploggercallback callback("SNMP Response received for setting log transfer path request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        unsigned timeout_ms = 2000;

        // Setting the container file name to delete
                oid oid_transfer_server_log_path [] = { FIXS_ITH_CMX_CONT_TRANSFER_SRV_PATH };

         const oid * oid_name = 0;
        size_t oid_name_size = 0;

        oid_name = oid_transfer_server_log_path; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_transfer_server_log_path);



                if ((call_result = set_server_path(log_path, session_handler,oid_name,oid_name_size, &timeout_ms))) {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_cont_transfer_server_path' failed: call_result == %d", call_result);
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to see if the requested operation was successfully
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_delete_cont_file()' reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }


        session_handler.close();
        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

}

int fixs_ith_snmpmanager::set_server_path (
                const char * log_path,
                fixs_ith_snmpsessionhandler & session_handler,
                const oid * oid_name,
                size_t oid_name_size,
                const unsigned * timeout_ms)

{

        char oid_str [512];
	oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Set the transfer server path': OID '%s': value '%s'", oid_str, log_path);

        int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_OCTET_STR, reinterpret_cast<const u_char *>(log_path), ::strlen(log_path), timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
        }

        return call_result;
}

int fixs_ith_snmpmanager::set_cont_control_notifications(
			fixs_ith_switchboardinfo & switch_board,
			int control_value)
{

        char switch_board_ipn_str_addresses[2][16] = {};

        ::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
        ::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

        FIXS_ITH_LOG(LOG_LEVEL_INFO, "create_cont_transfer_server_path: on CMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
        {
                call_result = set_cont_control_notifications(switch_board_ipn_str_addresses[ipn_index], control_value);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'create_cont_transfer_server_path' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

        if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'create_cont_transfer_server_path' failed on both ipns for switch board in slot %d",
                                switch_board.slot_position);
        }
        return call_result;
}


int fixs_ith_snmpmanager::set_cont_control_notifications(
			const char * switch_board_ip,
			int control_value)

{
        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'set_cont_control_notifications' request because switch board IP is empty '%s'", switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to set container file control notifications on switch board %s", switch_board_ip);

        int call_result = 0;

        fixs_ith_snmploggercallback callback("SNMP Response received for setting container file control notifications request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        unsigned timeout_ms = 2000;

        // Setting the container file name to delete
                oid oid_cont_file_notifications_control [] = { FIXS_ITH_CMX_CONT_NOTIFICATIONS_CONTROL };

        const oid * oid_name = 0;
        size_t oid_name_size = 0;

        oid_name =  oid_cont_file_notifications_control;oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_cont_file_notifications_control);



                if ((call_result = set_control_notifications_container(control_value, session_handler,oid_name,oid_name_size, &timeout_ms))) {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_cont_control_notifications' failed: call_result == %d", call_result);
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to see if the requested operation was successfully
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_cont_control_notifications()' reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }


        session_handler.close();
        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_snmpmanager::set_control_notifications_container(
		int control_value,
                fixs_ith_snmpsessionhandler & session_handler,
                const oid * oid_name,
                size_t oid_name_size,
                const unsigned * timeout_ms)
{
        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Set the transfer server path': OID '%s': value '%d'", oid_str, control_value);

        int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_INTEGER, reinterpret_cast<const u_char *>(&control_value), sizeof(control_value), timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
        }

        return call_result;

}

/********************** Setting Automatic File Transfer ****************/

int fixs_ith_snmpmanager::set_cont_automatic_transfer(
		fixs_ith_switchboardinfo & switch_board,
		int control_value)
{
        char switch_board_ipn_str_addresses[2][16] = {};

        ::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
        ::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

        FIXS_ITH_LOG(LOG_LEVEL_INFO, "set_cont_automatic_transfer: on CMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
        {
                call_result = set_cont_automatic_transfer(switch_board_ipn_str_addresses[ipn_index], control_value);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'set_cont_automatic_transfer' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

        if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'set_cont_automatic_transfer' failed on both ipns for switch board in slot %d",
                                switch_board.slot_position);
        }
        return call_result;
}


int fixs_ith_snmpmanager::set_cont_automatic_transfer(
		const char * switch_board_ip,
		int control_value)
{

        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'set_cont_automatic_transfer' request because switch board IP is empty '%s'", switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to set automatic container file transfer on switch board %s", switch_board_ip);

        int call_result = 0;

        fixs_ith_snmploggercallback callback("SNMP Response received for setting automatic container file transfer request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        unsigned timeout_ms = 2000;

        // Setting the container file name to delete
                oid oid_cont_file_automatic_transfer [] = { FIXS_ITH_CMX_CONT_NOTIFICATIONS_CONTROL };

        const oid * oid_name = 0;
        size_t oid_name_size = 0;

        oid_name = oid_cont_file_automatic_transfer; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_cont_file_automatic_transfer);

                if ((call_result = set_automatic_transfer_cont_file(control_value, session_handler,oid_name,oid_name_size, &timeout_ms))) {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_automatic_transfer_cont_file' failed: call_result == %d", call_result);
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to see if the requested operation was successfully
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_cont_control_notifications()' reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }


        session_handler.close();
        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_snmpmanager::set_automatic_transfer_cont_file(
		int control_value,
		fixs_ith_snmpsessionhandler & session_handler,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms)
{

        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Set the automatic transfer of container file': OID '%s': value '%d'", oid_str, control_value);

        int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_INTEGER, reinterpret_cast<const u_char *>(&control_value), sizeof(control_value), timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
        }

        return call_result;

}

/****************** Setting Container File Encryption *******/

int  fixs_ith_snmpmanager::set_cont_file_encryption(
		fixs_ith_switchboardinfo & switch_board,
		int encryption_setting)
{

        char switch_board_ipn_str_addresses[2][16] = {};

        ::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
        ::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

        FIXS_ITH_LOG(LOG_LEVEL_INFO, "set_cont_file_encryption: on CMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
        {
                call_result = set_cont_file_encryption(switch_board_ipn_str_addresses[ipn_index], encryption_setting);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'set_cont_file_encryption' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

        if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'set_cont_file_encryption' failed on both ipns for switch board in slot %d",
                                switch_board.slot_position);
        }
        return call_result;
}

int  fixs_ith_snmpmanager::set_cont_file_encryption(
		const char * switch_board_ip,
		int encryption_setting)
{

        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'set_cont_file_encryption' request because switch board IP is empty '%s'", switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to set container file encryption status on switch board %s", switch_board_ip);

        int call_result = 0;

        fixs_ith_snmploggercallback callback("SNMP Response received for setting container file encryption status request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        unsigned timeout_ms = 2000;

        // Setting the container file name to delete
                oid oid_cont_file_encryption_status [] = { FIXS_ITH_CMX_CONT_FILE_ENCRYPTION };

        const oid * oid_name = 0;
        size_t oid_name_size = 0;

        oid_name = oid_cont_file_encryption_status; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_cont_file_encryption_status);


                if ((call_result = set_encryption_setting_cont_file(encryption_setting, session_handler,oid_name,oid_name_size, &timeout_ms))) {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_encryption_setting_cont_file' failed: call_result == %d", call_result);
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to see if the requested operation was successfully
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_cont_control_notifications()' reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }


        session_handler.close();
        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int  fixs_ith_snmpmanager::set_encryption_setting_cont_file(
		int encryption_setting,
		fixs_ith_snmpsessionhandler & session_handler,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms)
{

        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Set the container file encryption status': OID '%s': value '%d'", oid_str,encryption_setting);

        int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_INTEGER, reinterpret_cast<const u_char *>(&encryption_setting), sizeof(encryption_setting), timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
        }

        return call_result;


}

/****************** Creating container file  *******/

int  fixs_ith_snmpmanager::create_cont_file(
                fixs_ith_switchboardinfo & switch_board,
                int value)
{

        char switch_board_ipn_str_addresses[2][16] = {};

        ::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
        ::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

        FIXS_ITH_LOG(LOG_LEVEL_INFO, "create_cont_file: on CMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
        {
                call_result = create_cont_file(switch_board_ipn_str_addresses[ipn_index], value);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'create_cont_file' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

        if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'create_cont_file' failed on both ipns for switch board in slot %d",
                                switch_board.slot_position);
        }
        return call_result;



}

int  fixs_ith_snmpmanager::create_cont_file(
                const char * switch_board_ip,
                int value)
{

        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'create_cont_file' request because switch board IP is empty '%s'", switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to creation of container file on switch board %s", switch_board_ip);

        int call_result = 0;

        fixs_ith_snmploggercallback callback("SNMP Response received for creating container file request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        unsigned timeout_ms = 2000;

        // Setting the container file name to delete
                oid oid_cont_file_creation [] = { FIXS_ITH_CMX_CONT_FILE_CREATION };
        const oid * oid_name = 0;
        size_t oid_name_size = 0;

        oid_name = oid_cont_file_creation; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_cont_file_creation);

                if ((call_result = set_container_file_creation(value, session_handler,oid_name,oid_name_size, &timeout_ms))) {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_container_file_creation", call_result);
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to see if the requested operation was successfully
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_cont_control_notifications()' reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }


        session_handler.close();
        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;


}

int  fixs_ith_snmpmanager::set_container_file_creation(
                int value,
                fixs_ith_snmpsessionhandler & session_handler,
                const oid * oid_name,
                size_t oid_name_size,
                const unsigned * timeout_ms)
{

        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Set the creation of container file': OID '%s': value '%d'", oid_str,value);

        int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_INTEGER, reinterpret_cast<const u_char *>(&value), sizeof(value), timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
        }

        return call_result;


}

/********************** Setting the default user priority for ports *********/
int fixs_ith_snmpmanager::set_mib_port_default_user_priority(
		fixs_ith_switchboardinfo & switch_board,
                uint16_t portIndex)
{

        char switch_board_ipn_str_addresses[2][16] = {};

        ::strncpy(switch_board_ipn_str_addresses[0], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[0]));
        ::strncpy(switch_board_ipn_str_addresses[1], switch_board.transport_ipna_str, FIXS_ITH_ARRAY_SIZE(switch_board_ipn_str_addresses[1]));

        FIXS_ITH_LOG(LOG_LEVEL_INFO, "set_mib_port_default_user_priority: on CMX having IPNA == %s and IPNB == %s ", switch_board.transport_ipna_str, switch_board.transport_ipnb_str);

        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01)
        {
                call_result = set_mib_port_default_user_priority(switch_board_ipn_str_addresses[ipn_index], portIndex);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Call 'set_mib_port_default_user_priority' failed: switch board slot == %d, switch board Transport IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

        if (call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Call 'set_mib_port_default_user_priority' failed on both ipns for switch board in slot %d",
                                switch_board.slot_position);
        }
        return call_result;

}




int fixs_ith_snmpmanager::set_mib_port_default_user_priority(
		 const char * switch_board_ip,
		 uint16_t portIndex)
{

        if (*switch_board_ip == 0 ) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot send 'set_mib_port_default_user_priority' request because switch board IP is empty '%s'", switch_board_ip);
                return fixs_ith_snmp::ERR_SNMP_INVALID_SWITCH_BOARD_IP;
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to set the default user priority of ports on switch board %s", switch_board_ip);

        int call_result = 0;

        fixs_ith_snmploggercallback callback("SNMP Response received for setting defaut user priority request towards CMX");
        fixs_ith_snmpsessionhandler session_handler;

        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        unsigned timeout_ms = 2000;

        // Setting the default priority of the ports
                oid oid_cmx_port_priority [] = { FIXS_ITH_CMX_PORT_PRIORITY, portIndex };
                //size_t oid_cmx_port_priority_size = FIXS_ITH_ARRAY_SIZE(oid_cmx_port_priority);
		int oid_value = FIXS_ITH_CMX_PRIORITY;
		const oid * oid_name = 0;
		size_t oid_name_size = 0;
		oid_name = oid_cmx_port_priority;
		oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_cmx_port_priority);

                if ((call_result = set_mib_port_default_user_priority(oid_value, session_handler,oid_name,oid_name_size, &timeout_ms))) {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'set_mib_port_default_user_priority' ", call_result);
                        session_handler.close();
                        return call_result;
                }

                // SNMP set request sent. Now check PDU response to see if the requested operation was successfully
                if(callback.error_code != fixs_ith_snmp::ERR_SNMP_NO_ERRORS )
                {
                        FIXS_ITH_LOG(LOG_LEVEL_ERROR, "PDU response of 'set_mib_port_default_user_priority()' reported an error: "
                                                                                  "callback.error_code == %d, callback.pdu_error_stat == %d", callback.error_code, callback.pdu_error_stat);
                        call_result = (callback.error_code == fixs_ith_snmp::ERR_SNMP_CALLBACK_ERRORSTAT_INDICATION ? callback.pdu_error_stat : callback.error_code);
                        session_handler.close();
                        return call_result;
                }


        session_handler.close();
        return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;


}

int fixs_ith_snmpmanager::set_mib_port_default_user_priority(
		int priority,
		fixs_ith_snmpsessionhandler & session_handler,
		const oid * oid_name,
		size_t oid_name_size,
		const unsigned * timeout_ms)
{

        char oid_str [512];
        oid_to_str(oid_str, FIXS_ITH_ARRAY_SIZE(oid_str), oid_name, oid_name_size);

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending a PDU SET request 'Set the default user priority of port': OID =='%s': prioirty == '%d'", oid_str,priority);

        int call_result = send_set_request(session_handler, oid_name, oid_name_size, ASN_INTEGER, reinterpret_cast<const u_char *>(&priority), sizeof(priority), timeout_ms);

        if (call_result == fixs_ith_snmp::ERR_SNMP_TIMEOUT) { // Timeout expired
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Timeout expired while sending the PDU SET request");
        } else if (call_result) { // ERROR: sending the request
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'send_set_request' failed: NET-SNMP library internal error on sending the SET request: call_result == %d", call_result);
        }

        return call_result;

}

//TR_HY52528 BEGIN

int fixs_ith_snmpmanager::getBoardState(fixs_ith_switchboardinfo& switch_board, uint8_t& shelf_manager_control_state)
{
        char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
        int call_result = fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;

        if(get_switch_board_target_ipns(switch_board_ipn_str_addresses[0],
                        switch_board_ipn_str_addresses[1],
                        switch_board,fixs_ith::CONTROL_PLANE))  return fixs_ith_snmp::ERR_SNMP_WRONG_SWITCH_BOARD_PLANE;

        FIXS_ITH_LOG(LOG_LEVEL_WARN, "Check status for switch board, slot == %d having IPNA == %s and IPNB == %s", switch_board.slot_position,
                        switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

        ::srandom(static_cast<unsigned>(::time(0)));
        int ipn_index = ::random() & 0x01;
        for (int i = 0; (isCommunicationFailure(call_result) && i < 2); ++i, ipn_index = (ipn_index + 1) & 0x01) {
                call_result = getBoardState(shelf_manager_control_state,switch_board.magazine,switch_board.slot_position,switch_board_ipn_str_addresses[ipn_index]);
                if (call_result)
                        FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
                                        "Call 'getBoardState' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",
                                        switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
        }

        return call_result;
}

int fixs_ith_snmpmanager::getBoardState(
                uint8_t& shelf_manager_control_state,
                uint32_t board_magazine,
                int32_t board_slot,
                const char * switch_board_ip)
{
        int call_result = fixs_ith_snmp::ERR_SNMP_NO_ERRORS;

        char mag_str[16] = {0};
        call_result = fixs_ith::workingSet_t::instance()->get_cs_reader().uint32_to_ip_format(mag_str, board_magazine, FIXS_ITH_CONFIG_MAGAZINE_ADDRESS_IMM_SEPARATOR);
        if (call_result < 0) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'uint32_to_ip_format' failed. call_result == %d", call_result);
                return fixs_ith_snmp::ERR_SNMP_INTERNAL_ERROR;
        }

        fixs_ith_get_shelf_manager_control_state_callback callback(shelf_manager_control_state, mag_str, board_slot, switch_board_ip);
        fixs_ith_snmpsessionhandler session_handler;


        if ((call_result = open_snmp_session_handler(session_handler, callback, switch_board_ip))) {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'open_snmp_session_handler' failed: call_result == %d", call_result);
                return call_result;
        }

        oid oid_front_port [] = { FIXS_ITH_SMX_OID_SHELF_CTRL_MGR };
        const oid * oid_name = 0;
        size_t oid_name_size = 0;
        oid_name = oid_front_port; oid_name_size = FIXS_ITH_ARRAY_SIZE(oid_front_port);

        call_result = get_mib_object(session_handler, callback, oid_name, oid_name_size, NULL);
        session_handler.close();

        return call_result;
}

//TR_HY52528 END
