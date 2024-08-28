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

#include "alarm/alarmhandler.h"
#include "alarm/alarm.h"
#include "alarm/vrrpmasterchange.h"
#include "alarm/bfdsessionfault.h"
#include "common/utility.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_programconstants.h"
#include "fixs_ith_workingset.h"

#include <ACS_APGCC_Util.H>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

#include "alarm/pimfault.h"
#include "alarm/interfaceportfault.h"
#include "alarm/smx_configuration_fault.h"

namespace alarms
{

AlarmHandler::AlarmHandler():ACE_Event_Handler(),m_processName(),m_evrep(),m_alarms(),m_mutex()
{
	ACS_APGCC::getProcessName(&m_processName);
}

AlarmHandler::~AlarmHandler()
{

}

int AlarmHandler::handle_timeout(const ACE_Time_Value & /*current_time*/, const void * arg)
{

	void* new_arg = const_cast<void*>(arg);
	std::pair<string, Alarm::Type>* magic_cookie = static_cast<std::pair<string, Alarm::Type>*>(new_arg);

	std::string objOfReference = magic_cookie->first;
	int alarm_type = magic_cookie->second;
	delete magic_cookie;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "handle_timeout called for alarm type:<%d>", alarm_type);

	switch(alarm_type)
	{
	case Alarm::VRRP_MASTER_CHANGE:
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Ceasing VRRP master change alarm, objOfReference:<%s>", objOfReference.c_str());
		ceaseVrrpMasterChangeAlarm(objOfReference);
		break;

	default:
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unknown alarm type");
		break;
	}


	return 0;
}

bool AlarmHandler::raiseVrrpMasterChangeAlarm(const std::string& objOfReference)
{
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	if(!exists(objOfReference, Alarm::VRRP_MASTER_CHANGE, alarmData))
	{
		alarmData = boost::make_shared<alarms::VrrpMasterChange>(objOfReference);

		if(!raise(alarmData))
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to raise VRRP master change alarm, objectReference:<%s>", objOfReference.c_str());
			result = false;
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "VRRP master change alarm has been raised, objectReference:<%s>", objOfReference.c_str());

			ACE_Time_Value vrrpMasterChangeAlarmTimer(alarms::vrrp_alarms::vrrpMasterChangeAlarmDelay, 0);
			int timerId = schedule_timer(objOfReference, Alarm::VRRP_MASTER_CHANGE, vrrpMasterChangeAlarmTimer);

			if ( -1 == timerId )
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to schedule timer for VRRP master change alarm, objectReference:<%s>", objOfReference.c_str());
			else
				alarmData->setTimerId(timerId);
		}
	}
	else if (::difftime(::time(NULL), alarmData->getTimestamp()) > 1) //Caching alarm if time difference is not greater than 1 second
	{
		// Alarm already exists and is old. Let's cease old alarm and raise new one.
		if(ceaseVrrpMasterChangeAlarm(objOfReference))
			result = raiseVrrpMasterChangeAlarm(objOfReference);
		else
			result = false;
	}
	else
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "VRRP master change alarm already raised, objectReference:<%s>", objOfReference.c_str());

	return result;
}

bool AlarmHandler::ceaseVrrpMasterChangeAlarm(const std::string& objOfReference)
{
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	if(exists(objOfReference, Alarm::VRRP_MASTER_CHANGE, alarmData))
	{
		int timerId = alarmData->getTimerId();
		this->cancel_timer(timerId);

		if(!cease(alarmData))
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to cease VRRP master change alarm, objectReference:<%s>", objOfReference.c_str());
			result = false;
		}
	}

	return result;
}

bool AlarmHandler::raisePimFaultAlarm (
		const std::string & objOfReference,
		const std::string & magazine,
		const std::string & slot,
		const std::string & port_name) {
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "objOfReference == <%s>, magazine == <%s>, slot == <%s>, port_name == <%s>",
			objOfReference.c_str(), magazine.c_str(), slot.c_str(), port_name.c_str());

	if (exists(objOfReference, Alarm::PIM_FAULT, alarmData)) {
		// Alarm already into the internal map.
	/*	if (::difftime(::time(NULL), alarmData->getTimestamp()) > 1) {
			//Caching alarm if time difference is not greater than 1 second
			//Alarm already exists and is old. Cease old alarm and raise new one.
			//Re-raise the same alarm after ceasing the old one
			result = ceasePimFaultAlarm(objOfReference) &&
							 raisePimFaultAlarm(objOfReference, magazine, slot, port_name);
		}
		else*/
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "PIM fault alarm already raised, objectReference == <%s>",objOfReference.c_str());
	}
	else
	{
		// The alarm does not exist
		alarmData = boost::make_shared<alarms::PimFault>(objOfReference, magazine, slot, port_name);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Alarm <%zu> raised for <%s> for severity== <%s> and problemtext==<%s>", alarmData->getSpecificProblem(), alarmData->getObjName(),alarmData->getPercSeverity(),alarmData->getProblemText());
		((result = raise(alarmData)))
			? FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"PIM fault alarm has been raised, objectReference == <%s>", objOfReference.c_str())
			: FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Failed to raise PIM fault alarm, objectReference == <%s>", objOfReference.c_str());
	}
	return result;
}
bool AlarmHandler::raiseInterfacePortFaultAlarm (
		const std::string & objOfReference,
		const std::string & magazine,
		const std::string & slot,
		const std::string & port_name) {
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "objOfReference == <%s>, magazine == <%s>, slot == <%s>, port_name == <%s>",
			objOfReference.c_str(), magazine.c_str(), slot.c_str(), port_name.c_str());

	if (exists(objOfReference, Alarm::INTERFACE_PORT_FAULT, alarmData)) {
		// Alarm already into the internal map.
		if (::difftime(::time(NULL), alarmData->getTimestamp()) > 1) {
			//Caching alarm if time difference is not greater than 1 second
			//Alarm already exists and is old. Cease old alarm and raise new one.
			//Re-raise the same alarm after ceasing the old one
			result = ceaseInterfacePortFaultAlarm(objOfReference) &&
							 raiseInterfacePortFaultAlarm(objOfReference, magazine, slot, port_name);
		}
		else
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Interface Port fault alarm already raised, objectReference == <%s>",
					objOfReference.c_str());
	}
	else
	{
		// The alarm does not exist
		alarmData = boost::make_shared<alarms::InterfacePortFault>(objOfReference, magazine, slot, port_name);

		((result = raise(alarmData)))
			? FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"Interface Port fault alarm has been raised, objectReference == <%s>", objOfReference.c_str())
			: FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Failed to raise Interface Port fault alarm, objectReference == <%s>", objOfReference.c_str());
	}

	return result;
}

bool AlarmHandler::ceasePimFaultAlarm (const std::string & objOfReference) {
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	if (exists(objOfReference, Alarm::PIM_FAULT, alarmData))
	{
		((result = cease(alarmData)))
				? FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"Ceased Pim fault alarm, objectReference:<%s>", objOfReference.c_str())
				: FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Failed to cease Pim fault alarm, objectReference:<%s>", objOfReference.c_str());
	}

	return result;
}


bool AlarmHandler::ceaseInterfacePortFaultAlarm (const std::string & objOfReference) {
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	if (exists(objOfReference, Alarm::INTERFACE_PORT_FAULT, alarmData))
	{
		((result = cease(alarmData)))
				? FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Ceased Interface Port fault alarm, objectReference:<%s>", objOfReference.c_str())
				: FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Failed to cease Interface Port fault alarm, objectReference:<%s>", objOfReference.c_str());
	}

	return result;
}

bool AlarmHandler::raiseSMXConfigurationFaultAlarm (
		const std::string & objOfReference,
		const std::string & magazine,
		const std::string & slot)
{
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "objOfReference == <%s>, magazine == <%s>, slot == <%s>",
			objOfReference.c_str(), magazine.c_str(), slot.c_str());

	if(exists(objOfReference, Alarm::SMX_CONFIGURATION_FAULT, alarmData))
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "SMX Configuration Fault alarm already raised, objectReference == <%s>",
					objOfReference.c_str());
	}
	else
	{
		// The alarm does not exist
		alarmData = boost::make_shared<alarms::SMXConfigurationFaultAlarm>(objOfReference, magazine, slot);

		((result = raise(alarmData)))
			? FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"SMX Configuration Fault alarm has been raised, objectReference == <%s>", objOfReference.c_str())
			: FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Failed to raise SMX Configuration Fault alarm, objectReference == <%s>", objOfReference.c_str());
	}

	return result;
}

bool AlarmHandler::ceaseSMXConfigurationFaultAlarm(const std::string & objOfReference)
{
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	if (exists(objOfReference, Alarm::SMX_CONFIGURATION_FAULT, alarmData))
	{

		((result = cease(alarmData)))
			? FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Ceased SMX Configuration Fault alarm, objectReference:<%s>", objOfReference.c_str())
			: FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Failed to cease SMX Configuration Fault alarm, objectReference:<%s>", objOfReference.c_str());
	}

	return result;
}

bool AlarmHandler::raiseBfdSessionFaultAlarm(const std::string& objOfReference, const std::string& router, const std::string& neighbor )
{
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	if(!exists(objOfReference, Alarm::BFD_SESSION_FAULT, alarmData))
	{
		alarmData = boost::make_shared<alarms::BfdSessionFault>(objOfReference, router, neighbor );

		result = raise(alarmData);

		if(result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "BFD Session fault alarm has been raised, objectReference:<%s>", objOfReference.c_str());
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to raise BFD Session fault alarm, objectReference:<%s>", objOfReference.c_str());
		}
	}
	else
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "BFD Session fault alarm already raised, objectReference:<%s>", objOfReference.c_str());

	return result;
}

bool AlarmHandler::ceaseBfdSessionFaultAlarm(const std::string& objOfReference)
{
	boost::shared_ptr<alarms::Alarm> alarmData;
	bool result = true;

	if(exists(objOfReference, Alarm::BFD_SESSION_FAULT, alarmData))
	{
		result = cease(alarmData);
		if(result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Ceased BFD Session fault alarm, objectReference:<%s>", objOfReference.c_str());
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to cease BFD Session fault alarm, objectReference:<%s>", objOfReference.c_str());
		}
	}

	return result;
}


int AlarmHandler::schedule_timer(const std::string& objRef, const Alarm::Type& type, const ACE_Time_Value& delay)
{
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Scheduling timer for alarm type:<%d>", type);
	std::pair<string, Alarm::Type>* magic_cookie = new std::pair<string, Alarm::Type>(objRef, type);
	int result = fixs_ith::workingSet_t::instance()->get_main_reactor().schedule_timer(this, static_cast<const void *>(magic_cookie), delay,  ACE_Time_Value::zero);

	if (-1 == result)
		delete magic_cookie;

	return result;
}

void AlarmHandler::cancel_timer(long int timerId)
{
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Canceling alarm timer for alarm timer id:<%li>", timerId);
	const void* arg[1] = {0};
	int result = fixs_ith::workingSet_t::instance()->get_main_reactor().cancel_timer(timerId, arg);
	if ((0 != result) && (0 != arg[0]))
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Magic cookie found. Freeing the memory!");
		void* new_arg = const_cast<void*>(arg[0]);
		std::pair<string, Alarm::Type>* magic_cookie = static_cast<std::pair<string, Alarm::Type>*>(new_arg);
		delete magic_cookie;
	}
}

bool AlarmHandler::raise(boost::shared_ptr<Alarm> alarmData)
{
	// Synchronize alarm map
	boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

	/*FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"m_processName.c_str() == <%s>, alarmData->getSpecificProblem() == <%llu>, alarmData->getPercSeverity() == <%s>, "
			"alarmData->getProbableCause() == <%s>, alarmData->getObjectClassOfReference() == <%s>, "
			"alarmData->getObjectOfReference() == <%s>, alarmData->getProblemData() == <%s>, alarmData->getProblemText() == <%s>, "
			"alarmData->getManualCease() == <%d>", m_processName.c_str(), alarmData->getSpecificProblem(),
			alarmData->getPercSeverity(), alarmData->getProbableCause(), alarmData->getObjectClassOfReference(),
			alarmData->getObjectOfReference(), alarmData->getProblemData(), alarmData->getProblemText(), alarmData->getManualCease());*/

	bool result = false;
	ACS_AEH_ReturnType status =  m_evrep.sendEventMessage( m_processName.c_str(),
			alarmData->getSpecificProblem(),
			alarmData->getPercSeverity(),
			alarmData->getProbableCause(),
			alarmData->getObjectClassOfReference(),
			alarmData->getObjectOfReference(),
			alarmData->getProblemData(),
			alarmData->getProblemText(),
			alarmData->getManualCease());

	if( (result = (ACS_AEH_ok == status)) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Alarm <%zu> raised for <%s> ", alarmData->getSpecificProblem(), alarmData->getObjName());

		m_alarms[std::make_pair(alarmData->getObjName(), alarmData->getType())] = alarmData;
	}

	return result;
}

bool AlarmHandler::cease(boost::shared_ptr<Alarm> alarmData )
{
	// Synchronize alarm map
	boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

	bool result = false;
	ACS_AEH_ReturnType status =  m_evrep.sendEventMessage( m_processName.c_str(),
			alarmData->getSpecificProblem(),
			alarmData->getCeasingSeverity(),
			alarmData->getProbableCause(),
			alarmData->getObjectClassOfReference(),
			alarmData->getObjectOfReference(),
			alarmData->getProblemData(),
			alarmData->getProblemText() );

	if( (result = (ACS_AEH_ok == status)) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Alarm <%zu> ceased for :<%s>", alarmData->getSpecificProblem(), alarmData->getObjName());

		m_alarms.erase(std::make_pair(alarmData->getObjName(), alarmData->getType()));
		alarmData.reset();
	}
	return result;
}

bool AlarmHandler::exists(const std::string& objName, const Alarm::Type& type, boost::shared_ptr<Alarm>& alarmData)
{
	// Synchronize alarm map
	boost::lock_guard<boost::recursive_mutex> guard(m_mutex);
	bool exists = false;

	// check for alarm, if not present then raise.
	mapOfAlarms_t::const_iterator element = m_alarms.find(std::make_pair(objName, type));

	if( (exists = m_alarms.end() != element) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Alarm of type <%d> found for <%s> ", type, objName.c_str());
		alarmData = element->second;
	}
	else
	{
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "Alarm of type <%d> NOT found for <%s> ", type, objName.c_str());
	}

	return exists;
}

void AlarmHandler::ceaseAll()
{
	// Synchronize alarm map
	boost::lock_guard<boost::recursive_mutex> guard(m_mutex);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Ceasing of <%zu> Alarms", m_alarms.size());

	mapOfAlarms_t::const_iterator element = m_alarms.begin();

	for(; m_alarms.end() != element; ++element)
	{
		int timerId = element->second->getTimerId();
		if (-1 != timerId)
		{
			this->cancel_timer(timerId);
		}
		cease(element->second);
	}

	m_alarms.clear();
}

} /* namespace alarm */
