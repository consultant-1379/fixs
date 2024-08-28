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

#ifndef ITHADM_CAA_INC_FIXS_ITH_WORKINGSET_H_
#define ITHADM_CAA_INC_FIXS_ITH_WORKINGSET_H_

#include <ace/TP_Reactor.h>
#include <ace/Reactor.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>
#include <ace/RW_Thread_Mutex.h>


#include "fixs_ith_cmdoptionparser.h"
#include "fixs_ith_csreader.h"
#include "fixs_ith_snmpmanagercallbacks.h"
#include "fixs_ith_snmpmanager.h"
#include "fixs_ith_cshwctablechangeobserver.h"
#include "fixs_ith_procsignalseventhandler.h"
#include "fixs_ith_trapdsobserver.h"
#include "operation/scheduler.h"
#include "engine/configurationManager.h"
#include "fixs_ith_transportBNChandler.h"
#include "fixs_ith_sbdatamanager.h"
#include "alarm/alarmhandler.h"
#include "netconf/manager.h"
#include "imm/handler.h"


template <class T>
class ACE_Future;

namespace fixs_ith
{
	class workingset
	{
		friend class ACE_Singleton<workingset, ACE_Recursive_Thread_Mutex>;

		public:

			inline boost::shared_ptr<operation::Scheduler> get_main_scheduler()  { return _main_scheduler; };
			inline int start_main_scheduler() { return _main_scheduler->start(); };
			inline int stop_main_scheduler(bool wait_termination = true) { return _main_scheduler->stop(wait_termination); };

			int stop_context_schedulers(bool wait_termination = true);

			int schedule(operation::identifier_t id, const void * op_details = 0);
			int schedule(operation::identifier_t id, ACE_Future<operation::result>* op_result, const void* op_details);

			int schedule(const std::string& contextKey, operation::identifier_t id, const void * op_details = 0);
			int schedule(const std::string& contextKey, operation::identifier_t id, ACE_Future<operation::result>* op_result, const void* op_details);

			int schedule_until_completion(operation::identifier_t id, unsigned int delay, unsigned int interval, const void * op_details = 0,  size_t op_details_size = 0);
			int schedule_until_completion(const std::string& contextKey, operation::identifier_t id, unsigned int delay, unsigned int interval, const void * op_details = 0,  size_t op_details_size = 0);

			inline ACE_Reactor& get_main_reactor()  { return _main_reactor; };
			inline int stop_main_reactor() { return _main_reactor.end_reactor_event_loop(); };

			inline fixs_ith_csreader & get_cs_reader() { return _cs_reader; }
			inline fixs_ith_cshwctablechangeobserver & get_cs_hwctablechangeobserver() { return _cs_hwc_table_change_observer; }

			inline fixs_ith_trapdsobserver & get_trapdsobserver() { return _trapds_observer; }

			inline fixs_ith_snmpmanager & get_snmpmanager() { return _snmpmanager; }

			inline netconf::manager & get_netconfmanager() { return _netconfmanager; }

			inline fixs_ith_transportBNChandler & get_transportBNChandler() { return _transportBNChandler; }

			inline fixs_ith::ProgramStateConstants get_program_state() { return _program_state; }
			inline void set_program_state(fixs_ith::ProgramStateConstants state) {  _program_state = state; }

			inline int registerObjectImplementers() { return m_immHandler.take_ownership(); };
			inline int unregisterObjectImplementers() { return m_immHandler.release_ownership(); };

			inline int registerAsBrfPartecipant() { return m_immHandler.registerAsBrfPartecipant(); };
			inline int unregisterAsBrfPartecipant() { return m_immHandler.unregisterAsBrfPartecipant(); };
			inline int isBackupOngoing() { return m_immHandler.isBackupOngoing(); };

			inline engine::ConfigurationManager& getConfigurationManager()  { return m_configurationManager; };

			inline fixs_ith_sbdatamanager& get_sbdatamanager() { return _sbdataManager; }

			inline alarms::AlarmHandler& getAlarmHandler() { return m_alarmHandler; }

			inline int createPatchDirectory() { return m_configurationManager.createPatchDirectory();}

		private:

			workingset();

			virtual ~workingset();

			workingset(const workingset& rhs);

			workingset& operator=(const workingset& rhs);

			boost::shared_ptr<operation::Scheduler> get_context_scheduler(const std::string& contextKey);

			int schedule(boost::shared_ptr<operation::Scheduler>, operation::identifier_t id, const void * op_details = 0);
			int schedule(boost::shared_ptr<operation::Scheduler>, operation::identifier_t id, ACE_Future<operation::result>* op_result, const void* op_details);
			int schedule_until_completion(boost::shared_ptr<operation::Scheduler>, operation::identifier_t id, unsigned int delay, unsigned int interval, const void * op_details = 0,  size_t op_details_size = 0);


		private:
			// ITH Server main reactor
			ACE_TP_Reactor _main_reactor_impl;
			ACE_Reactor _main_reactor;

			// CS facilities
			fixs_ith_csreader _cs_reader;	// used to read HWC table
			fixs_ith_cshwctablechangeobserver _cs_hwc_table_change_observer; // used to be notified about HWC table changes

			// Object used to to receive SNMP traps from TRAPD Service
			fixs_ith_trapdsobserver _trapds_observer;

			// Object used to interact with a Switch board via SNMP
			fixs_ith_snmpmanager _snmpmanager;

			// Object used to interact with a Switch board via NETCONF
			netconf::manager _netconfmanager;

			// ITH Transport Basic Network Configuration handler
			fixs_ith_transportBNChandler _transportBNChandler;

			// Objects containing the info about all the switchboards that are present in the system
			fixs_ith_sbdatamanager _sbdataManager;

			// Object used to interact with IMM
			imm::OI_Handler m_immHandler;

			// ITH Server scheduler
			boost::shared_ptr<operation::Scheduler> _main_scheduler;

			// ITH Context scheduler
			typedef std::map<std::string, boost::shared_ptr<operation::Scheduler> > scheduler_map_t;
			scheduler_map_t _context_scheduler_map;

			ACE_Recursive_Thread_Mutex m_synch_mutex;

			//ITH configuration manager
			engine::ConfigurationManager m_configurationManager;

			alarms::AlarmHandler m_alarmHandler;

			fixs_ith::ProgramStateConstants _program_state;
	};

	typedef ACE_Singleton< workingset, ACE_Recursive_Thread_Mutex> workingSet_t;
}


#endif /* ITHADM_CAA_INC_FIXS_ITH_WORKINGSET_H_ */
