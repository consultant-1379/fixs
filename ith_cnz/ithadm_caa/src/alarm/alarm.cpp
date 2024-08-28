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

#include "alarm/alarm.h"

namespace alarms
{

	Alarm::Alarm(const Type& alarmType)
	: m_type(alarmType),
	  m_perceivedSeverity(),
	  m_specificProblem(0U),
	  m_probableCause(),
	  m_objectOfReference(),
	  m_problemData(),
	  m_problemText(),
	  m_manualCease(true),
	  m_timerId(-1),
	  m_timestamp(0)
	{

	}

	Alarm::Alarm(const Type& alarmType,
			const std::string& objClassOfRef,
			const std::string& objName,
			const std::string& percSeverity,
			const uint64_t& specificProblem,
			const std::string& probableCause,
			const std::string& problemData,
			const bool& manualCease)
	: m_type(alarmType),
	  m_objectClassOfReference(objClassOfRef),
	  m_objectName(objName),
	  m_perceivedSeverity(percSeverity),
	  m_specificProblem(specificProblem),
	  m_probableCause(probableCause),
	  m_objectOfReference(),
	  m_problemData(problemData),
	  m_problemText(),
	  m_manualCease(manualCease),
	  m_timerId(-1),
	  m_timestamp(0)
	{

	}

	Alarm::~Alarm() {

	}

	void Alarm::setPercSeverity(const std::string& percSeverity) {
		m_perceivedSeverity.assign(percSeverity);
	}

	const char* Alarm::getPercSeverity() {
		return m_perceivedSeverity.c_str();
	}

	void Alarm::setSpecificProblem(const uint64_t& specificProblem) {
		m_specificProblem = specificProblem;
	}

	uint64_t Alarm::getSpecificProblem() {
		return m_specificProblem;
	}

	void Alarm::setProbableCause(const std::string& probableCause) {
		m_probableCause.assign(probableCause);
	}

	const char* Alarm::getProbableCause() {
		return m_probableCause.c_str();
	}

	void Alarm::setObjectClassOfReference(const std::string& objectOfReference) {
		m_objectClassOfReference.assign(objectOfReference);
	}

	const char* Alarm::getObjectClassOfReference() {
		return m_objectClassOfReference.c_str();
	}

	void Alarm::setObjectOfReference(const std::string& objectOfReference) {
		m_objectOfReference.assign(objectOfReference);
	}

	const char* Alarm::getObjectOfReference() {
		return m_objectOfReference.c_str();
	}

	void Alarm::setProblemData(const std::string& problemData) {
		m_problemData.assign(problemData);
	}

	const char* Alarm::getProblemData() {
		return m_problemData.c_str();
	}

	void Alarm::setProblemText(const std::string& problemText) {
		m_problemText.assign(problemText);
	}

	const char* Alarm::getProblemText() {
		return m_problemText.c_str();
	}

	bool Alarm::getManualCease() {
		return m_manualCease;
	}

	void Alarm::setTimerId(const int& timerId) {
		m_timerId = timerId;
	}

	int Alarm::getTimerId() {
		return m_timerId;
	}

	void Alarm::setObjName(const std::string& objName) {
		m_objectName.assign(objName);
	}

	const char* Alarm::getObjName() {
		return m_objectName.c_str();

	}


} /* namespace alarm */
