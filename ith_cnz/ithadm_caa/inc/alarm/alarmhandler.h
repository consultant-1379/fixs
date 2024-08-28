//	********************************************************
//
//	 COPYRIGHT Ericsson 2015
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2015.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2015 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************

#ifndef ITHADM_CAA_INC_ALARM_ALARMHANDLER_H_
#define ITHADM_CAA_INC_ALARM_ALARMHANDLER_H_

#include <stdio.h>

#include "alarm/alarm.h"
#include "acs_aeh_evreport.h"

#include <ace/Event_Handler.h>
#include <boost/noncopyable.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <map>
#include <string>

namespace alarms
{
	class Alarm;

	/**
	 * @class AlarmHandler
	 *
	 * @brief
	 *
	 * This class provides the methods to raise and cease alarms.
	 *
	 * @sa boost::noncopyable
	 */
	class AlarmHandler : public ACE_Event_Handler, private boost::noncopyable
	{
	public:

		///  Constructor
		AlarmHandler();

		///  Destructor
		virtual ~AlarmHandler();

		/** @brief
		 *
		 *	This method handles timeout for scheduled alarm timer.
		 *
		 *	@remarks Remarks
		 */
		virtual int handle_timeout (const ACE_Time_Value &current_time, const void * arg);

		/** @brief
		 *
		 *	This method raises the VRRP master change alarm.
		 *
		 *	@remarks Remarks
		 */
		bool raiseVrrpMasterChangeAlarm(const std::string& objOfReference);

		/** @brief
		 *
		 *	This method ceases the VRRP master change alarm.
		 *
		 *	@remarks Remarks
		 */
		bool ceaseVrrpMasterChangeAlarm(const std::string& objOfReference);

		/** @brief
		 *
		 *	This method raises the Interface Port Fault alarm.
		 *
		 *	@remarks Remarks
		 */
		bool raiseInterfacePortFaultAlarm (
			const std::string & objOfReference,
			const std::string & magazine,
			const std::string & slot,
			const std::string & port_name);

		bool raisePimFaultAlarm (
			const std::string & objOfReference,
			const std::string & magazine,
			const std::string & slot,
			const std::string & port_name);

		/** @brief
		 *
		 *	This method ceases the Interface Port Fault alarm.
		 *
		 *	@remarks Remarks
		 */
		bool ceaseInterfacePortFaultAlarm (const std::string & objOfReference);
		bool ceasePimFaultAlarm (const std::string & objOfReference);

		/** @brief
		 *
		 *	This method raises the SMX Configuration Fault alarm.
		 *
		 *	@remarks Remarks
		 */
		bool raiseSMXConfigurationFaultAlarm (
				const std::string & objOfReference,
				const std::string & magazine,
				const std::string & slot);

		inline  bool raiseSMXConfigurationFaultAlarm (
				const std::string & objOfReference,
				const std::string & magazine,
				int32_t slot) {
			char slot_str [32] = {0};
			::snprintf(slot_str, sizeof(slot_str), "%d", slot);
			return raiseSMXConfigurationFaultAlarm(objOfReference, magazine, slot_str);
		}

		/** @brief
		 *
		 *	This method ceases the SMX Configuration Fault alarm.
		 *
		 *	@remarks Remarks
		 */
		bool ceaseSMXConfigurationFaultAlarm (const std::string & objOfReference);

		/** @brief
		 *
		 *	This method raises the BFD Session fault alarm.
		 *
		 *	@remarks Remarks
		 */
		bool raiseBfdSessionFaultAlarm(const std::string& objOfReference, const std::string& router, const std::string& neighbor );

		/** @brief
		 *
		 *	This method ceases the BFD Session fault alarm.
		 *
		 *	@remarks Remarks
		 */
		bool ceaseBfdSessionFaultAlarm(const std::string& objOfReference);


		/** @brief
		 *
		 *	This method ceases all the alarms raised by service.
		 *	Expected to be called during service shutdown.
		 *
		 *	@remarks Remarks
		 */
		void ceaseAll();

	private:

		/** @brief
		 *
		 *	This method schedules alarm timer.
		 *
		 *	@return scheduled timer id on success, -1 on failure.
		 *
		 *	@remarks Remarks
		 */
		int schedule_timer (const std::string& objRef, const Alarm::Type& type, const ACE_Time_Value& delay);

		/** @brief
		 *
		 *	This method cancels alarm timer.
		 *
		 *	@remarks Remarks
		 */
		void cancel_timer (long int timerId);

		/** @brief
		 *
		 *	This method raises alarm using the provided alarm data.
		 *
		 *	@param alarmData : boosts::shared_ptr for Alarm class.
		 *
		 *	@param alarmPrintout : alarm printout
		 *
		 *	@return true on success otherwise false.
		 *
		 *	@remarks Remarks
		 */
		bool raise(boost::shared_ptr<Alarm> alarmData);

		/** @brief
		 *
		 *	This method ceases the alarm using the provided alarm data.
		 *
		 *	@param alarmData : boosts::shared_ptr for Alarm class.
		 *
		 *	@return true on success otherwise false.
		 *
		 *	@remarks Remarks
		 */
		bool cease(boost::shared_ptr<Alarm> alarmData );

		/** @brief
		 *
		 *	This method checks for alarm already present or not,
		 *	with a specific object of reference and alarm identifier.
		 *
		 *	@param objRef : Alarm object of reference.
		 *
		 *	@param specificProblem : Alarm identifier.
		 *
		 *	@param alarmObj : boosts::shared_ptr for Alarm class.
		 *
		 *	@return true on success otherwise false.
		 *
		 *	@remarks Remarks
		 */
		bool exists(const std::string& objRef, const Alarm::Type& type, boost::shared_ptr<Alarm>& alarmData);

		///process name
		std::string m_processName;

		//Instance of acs_aeh_evreport class which provides APIs to raise and cease alarms.
		acs_aeh_evreport m_evrep;

		///map key composed by the pair : <objectOfReference, alarmType>
		typedef std::pair<std::string, Alarm::Type> alarmKey_t;

		///to store Alarm object for each pair of <dataSourceName,alarmIdentifier>
		typedef std::map<alarmKey_t, boost::shared_ptr<Alarm> > mapOfAlarms_t;

		///Map instance to store Alarm object for each pair of <dataSourceName,alarmIdentifier>
		mapOfAlarms_t m_alarms;

		// To synchronize alarm map access
		boost::recursive_mutex m_mutex;
	};

} /* namespace alarm */

#endif /* ITHADM_CAA_INC_ALARM_ALARMHANDLER_H_ */
