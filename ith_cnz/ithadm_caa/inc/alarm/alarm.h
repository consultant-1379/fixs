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

#ifndef ITHADM_CAA_INC_ALARM_ALARM_H_
#define ITHADM_CAA_INC_ALARM_ALARM_H_
#include <string>
#include <stdint.h>

namespace alarms
{
	namespace Severity
	{
		const std::string ceasing = "CEASING";
	}

	/**
	 * @class Alarm
	 *
	 * @brief
	 *
	 * This class stores the information of an alarm.
	 *
	 */

	class Alarm {

		public:
			/**
			 * @brief
			 * This enumeration specifies various of Types of alarms
			 */
			enum Type
			{
				UNKNOWN = 0,	/*!< UNKNOWN */
				VRRP_MASTER_CHANGE,	/*!< VRRP MASTER CHANGE ALARM */
				BFD_SESSION_FAULT, /*!< BFD SESSION FAULT ALARM */
				INTERFACE_PORT_FAULT, /*!< INTERFACE PORT FAULT */
				SMX_CONFIGURATION_FAULT,
				PIM_FAULT	/* WRONG PIM MODULE FAULT*/	
			};

			/**
			 * @brief
			 * This is a Constructor that sets the type of the alarm to a given value.
			 *
			 * @param alarmType: type of alarm
			 */
			Alarm(const Type& alarmType);

			/**
			 * @brief
			 * This is a Constructor that sets the type of the alarm, object name,
			 * severity, specific problem, probable cause and problem data to
			 * a given values.
			 *
			 * @param alarmType: type of alarm
			 * @param objClassOfRef: class of reference
			 * @param objName: object name
			 * @param percSeverity: severity of alarm
			 * @param specificProblem: specific problem
			 * @param probableCause: probable cause for the alarm
			 * @param problemData: problem data
			 */
			Alarm(	const Type& alarmType,
					const std::string& objClassOfRef,
					const std::string& objName,
					const std::string& percSeverity,
					const uint64_t& specificProblem,
					const std::string& probableCause,
					const std::string& problemData,
					const bool& manualCease);

			///	Destructor
			virtual ~Alarm();

			/** @brief
			 *
			 *	This method sets the severity of an alarm.
			 *
			 *	@param percSeverity : Severity of the alarm.
			 *
			 *	@remarks Remarks
			 */
			virtual void setPercSeverity(const std::string& percSeverity);

			/** @brief
			 *
			 *	This method gets the severity of an alarm.
			 *
			 *	@return Returns alarm severity ( A2, A1 etc.,).
			 *
			 *	@remarks Remarks
			 */
			virtual const char* getPercSeverity();

			/** @brief
			 *
			 *	This method sets the specific problem of an alarm.
			 *
			 *	@param specificProblem : Alarm identifier.
			 *
			 *	@remarks Remarks
			 */
			virtual void setSpecificProblem(const uint64_t& specificProblem);

			/** @brief
			 *
			 *	This method gets the specific problem of an alarm.
			 *
			 *	@return Returns alarm identifier number.
			 *
			 *	@remarks Remarks
			 */
			virtual uint64_t getSpecificProblem();

			/** @brief
			 *
			 *	This method sets the probable cause of an alarm.
			 *
			 *	@param probableCause : Alarm cause.
			 *
			 *	@remarks Remarks
			 */
			virtual void setProbableCause(const std::string& probableCause);

			/** @brief
			 *
			 *	This method gets the probable cause of an alarm.
			 *
			 *	@return Returns Alarm cause.
			 *
			 *	@remarks Remarks
			 */
			virtual const char* getProbableCause();

			/** @brief
			 *
			 *	This method sets the objectClassOfReference of an alarm.
			 *
			 *	@param probableCause : objectClassOfReference of an alarm.
			 *
			 *	@remarks Remarks
			 */
			virtual void setObjectClassOfReference(const std::string& objectOfReference);

			/** @brief
			 *
			 *	This method gets the objectClassOfReference of an alarm.
			 *
			 *	@return Returns objectClassOfReference of an alarm.
			 *
			 *	@remarks Remarks
			 */
			virtual const char* getObjectClassOfReference();

			/** @brief
			 *
			 *	This method sets the objectOfReference of an alarm.
			 *
			 *	@param probableCause : objectOfReference of an alarm.
			 *
			 *	@remarks Remarks
			 */
			virtual void setObjectOfReference(const std::string& objectOfReference);

			/** @brief
			 *
			 *	This method gets the objectOfReference of an alarm.
			 *
			 *	@return Returns objectOfReference of an alarm.
			 *
			 *	@remarks Remarks
			 */
			virtual const char* getObjectOfReference();

			/** @brief
			 *
			 *	This method sets the problem data of an alarm.
			 *
			 *	@param probableCause : problem data of an alarm.
			 *
			 *	@remarks Remarks
			 */
			virtual void setProblemData(const std::string& problemData);

			/** @brief
			 *
			 *	This method gets the problem data of an alarm.
			 *
			 *	@return Returns problem data of an alarm.
			 *
			 *	@remarks Remarks
			 */
			virtual const char* getProblemData();

			/** @brief
			 *
			 *	This method sets the problem text of an alarm.
			 *
			 *	@param probableCause : problem text of an alarm.
			 *
			 *	@remarks Remarks
			 */
			virtual void setProblemText(const std::string& problemText);

			/** @brief
			 *
			 *	This method gets the probable text of an alarm.
			 *
			 *	@return Returns Alarm text.
			 *
			 *	@remarks Remarks
			 */
			virtual const char* getProblemText();

			/** @brief
			 *
			 *	This method gets the manual cease flag of an alarm.
			 *
			 *	@return Returns manual cease flag value.
			 *
			 *	@remarks Remarks
			 */
			virtual bool getManualCease();

			/** @brief
			 *
			 *	This method sets timer id of a scheduled alarm timer.
			 *
			 *	@remarks Remarks
			 */
			virtual void setTimerId(const int& timerId);

			/** @brief
			 *
			 *	This method gets timer id of a scheduled alarm timer.
			 *
			 *	@remarks Remarks
			 */
			virtual int getTimerId();

			/** @brief
			 *
			 *	This method sets the object name
			 *
			 *	@param objName : Name of the object for which alarm to be raised.
			 *
			 *	@remarks Remarks
			 */
			virtual void setObjName(const std::string& objName);

			/** @brief
			 *
			 *	This method gets the object name (Name of data source, data sink or peer etc.,).
			 *
			 *	@return Returns object name.
			 *
			 *	@remarks Remarks
			 */
			virtual const char* getObjName();

			/** @brief
			 *
			 *	This method gets the alarm type.
			 *
			 *	@return Returns alarm type.
			 *
			 *	@remarks Remarks
			 */
			inline Type getType() const { return m_type; };

			/** @brief
			 *
			 *	This method gets the alarm ceasing severity.
			 *
			 *	@return Returns alarm ceasing severity.
			 *
			 *	@remarks Remarks
			 */
			inline const char* getCeasingSeverity() const { return Severity::ceasing.c_str(); };

			/** @brief
			 *
			 *	This method sets alarm timestamp.
			 *
			 *	@remarks Remarks
			 */
			inline void setTimestamp() { m_timestamp = ::time(NULL); };

			/** @brief
			 *
			 *	This method gets the alarm timestamp.
			 *
			 *	@return Returns alarm timestamp.
			 *
			 *	@remarks Remarks
			 */
			inline time_t getTimestamp() { return m_timestamp; };

		protected:

			///Type
			Type m_type;

			///object Class of reference
			std::string m_objectClassOfReference;

			///object Name ( DataSource or DataSink name etc.,)
			std::string m_objectName;

			/// Alarm severity
			std::string m_perceivedSeverity;

			///Alarm identifier.
			uint32_t m_specificProblem;

			///Probable cause
			std::string m_probableCause;

			///Object of Ref
			std::string m_objectOfReference;

			///Problem data
			std::string m_problemData;

			///Problem text
			std::string m_problemText;

			//Manual cease
			bool m_manualCease;

			//Timer id
			int m_timerId;

			//Timestamp
			time_t m_timestamp;

	};

} /* namespace alarm */

#endif /* ITHADM_CAA_INC_ALARM_ALARM_H_ */
