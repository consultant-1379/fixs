#include <ace/Guard_T.h>

#include "fixs_ith_logger.h"
#include "fixs_ith_snmpconstants.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_snmptrapmessagehandler.h"
#include "fixs_ith_server.h"
#include "fixs_ith_workingset.h"

#include "fixs_ith_trapdsobserver.h"

void fixs_ith_trapdsobserver::handleTrap (ACS_TRAPDS_StructVariable var) {
	// Check the program running state: ITH server handles traps only when
	// it is running in NODE ACTIVE mode
	fixs_ith::ProgramStateConstants program_state = fixs_ith::workingSet_t::instance()->get_program_state();
	if (program_state == fixs_ith::PROGRAM_STATE_RUNNING_NODE_ACTIVE) {
		fixs_ith_snmptrapmessagehandler trap_handler;

		if (const int call_result = trap_handler.handle_trap(var))
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'handle_trap' failed: handling the PDU trap message: call_result == %d", call_result);
	} else FIXS_ITH_LOG(LOG_LEVEL_WARN, "ITH server is not running in ACTIVE NODE mode: this trap message notification will be ignored: program_state == %d", program_state);
}

int fixs_ith_trapdsobserver::subscribe (std::string serviceName) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Subscribing observer " FIXS_ITH_STRINGIZE(fixs_ith_trapdsobserver) " into ITH server to the Trap Dispatcher Service...");

	if (_subscribed) {
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "Observer " FIXS_ITH_STRINGIZE(fixs_ith_trapdsobserver) " already subscribed");
		return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	}

	ACS_TRAPDS::ACS_TRAPDS_API_Result trapds_call_result = ACS_TRAPDS::Result_Failure;
	for(int i =0;i<5 && trapds_call_result != ACS_TRAPDS::Result_Success; ++i )
	{
		trapds_call_result = ACS_TRAPDS_API::subscribe(serviceName);
		if(trapds_call_result != ACS_TRAPDS::Result_Success)
		{
			sleep(1);
            FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ITH failed to subscribe with TRAPDS trying again");
			continue;
		}
	}
	if (trapds_call_result != ACS_TRAPDS::Result_Success) { // ERROR: subscribing myself to the trap dispatcher service
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'ACS_TRAPDS_API::subscribe' failed: cannot subscribe ITH server to the trap dispatcher service: trapds_call_result == %d",
				trapds_call_result);
		return fixs_ith_snmp::ERR_SNMP_TRAP_SUBSCRIPTION;
	}

	_subscribed = 1;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Observer " FIXS_ITH_STRINGIZE(fixs_ith_trapdsobserver) " into ITH server successfully subscribed");

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

int fixs_ith_trapdsobserver::unsubscribe () {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "UnSubscribing observer " FIXS_ITH_STRINGIZE(fixs_ith_trapdsobserver) " into ITH server to the Trap Dispatcher Service...");

	if (!_subscribed) {
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "Observer " FIXS_ITH_STRINGIZE(fixs_ith_trapdsobserver) " already unsubscribed");
		return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	}

	ACS_TRAPDS::ACS_TRAPDS_API_Result trapds_call_result = ACS_TRAPDS_API::unsubscribe();

	if (trapds_call_result != ACS_TRAPDS::Result_Success) { // ERROR: subscribing myself to the trap dispatcher service
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'ACS_TRAPDS_API::unsubscribe' failed: cannot unsubscribe ITH server to the trap dispatcher service: trapds_call_result == %d",
				trapds_call_result);
		return fixs_ith_snmp::ERR_SNMP_TRAP_SUBSCRIPTION;
	}

	_subscribed = 0;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Observer " FIXS_ITH_STRINGIZE(fixs_ith_trapdsobserver) " into ITH server successfully unsubscribed");

	return fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
}

void fixs_ith_trapdsobserver::enable_traps(const std::string& board_key, int traps_mask)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Enable traps:<%d> on board:<%s>", traps_mask, board_key.c_str());

	if(m_trap_status.end() != m_trap_status.find(board_key))
	{
		// board key already present update the status
		m_trap_status[board_key] &= (trap_mask::ALL - traps_mask);
	}
	else
	{
		// board key not found, add it
		m_trap_status[board_key] = 0U;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Current traps status:<%d> on board:<%s>", m_trap_status[board_key], board_key.c_str() );
}

void fixs_ith_trapdsobserver::disable_traps(const std::string& board_key, int traps_mask)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Disable traps:<%d> on board:<%s>", traps_mask, board_key.c_str());

	if(m_trap_status.end() == m_trap_status.find(board_key))
	{
		// board not found
		m_trap_status[board_key] = 0U;
	}

	m_trap_status[board_key] |= traps_mask;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Current traps status:<%d> on board:<%s>", m_trap_status[board_key], board_key.c_str() );
}

bool fixs_ith_trapdsobserver::is_trap_enabled(const std::string& board_key, int trap_mask)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	bool result = true;

	if(m_trap_status.end() != m_trap_status.find(board_key))
	{
		result = (trap_mask::ALL - m_trap_status[board_key]) & trap_mask;
	}

	return result;
}
