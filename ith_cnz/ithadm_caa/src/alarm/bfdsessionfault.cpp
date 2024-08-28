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
 *  Created on: Aug 18, 2016
 *      Author: xludesi
 */

#include "alarm/bfdsessionfault.h"
#include "common/utility.h"

namespace alarms
{
	namespace BfdSessionFaultFields
	{
		const uint32_t specificProblem = 35200;
		const std::string severity = "A3";
		const std::string probableCause = "SWITCH IP NETWORK INTERFACE BFD SESSION FAULT";
		const std::string objectClassOfReference = "APZ";
		const std::string problemData = "One Bidirectional Forwarding Detection (BFD) session has stopped working";
		const std::string problemText = "";
		const bool manualCease = false;

	}

	/*
	 *
	 *
	   ROUTER NEIGHBOR
	   router neighbor
	 */
	BfdSessionFault::BfdSessionFault(const std::string& objOfReference, const std::string& router, const std::string& neighbor)
	: Alarm(Alarm::BFD_SESSION_FAULT,
			BfdSessionFaultFields::objectClassOfReference,
			objOfReference,
			BfdSessionFaultFields::severity,
			BfdSessionFaultFields::specificProblem,
			BfdSessionFaultFields::probableCause,
			BfdSessionFaultFields::problemData,
			BfdSessionFaultFields::manualCease
			)
	{
		char printoutHeader[256] = {0};
		snprintf(printoutHeader, sizeof(printoutHeader)/sizeof(printoutHeader[0]),
				"%-27s %-16s", "ROUTER", "NEIGHBOR");

		char printout_buffer[1024] = {0};
		snprintf(printout_buffer, sizeof(printout_buffer)/ sizeof(printout_buffer[0]),
				"\n%s"
				"\n%-27s %-16s",
				printoutHeader, router.c_str(), neighbor.c_str());

		setProblemText(printout_buffer);

		//Build ObjOfRef

		//TODO: ALH currently doesn't support obj of reference longer than 64 chars. Using a simplified DN for the moment
		{
			//TODO: Remove this once ALH supports longer DNs
			char simplifiedObjectOfRef[256] = {0};
			snprintf(simplifiedObjectOfRef, sizeof(simplifiedObjectOfRef)/ sizeof(simplifiedObjectOfRef[0]),
			"Router=%s,BfdSessionIPv4=%s", router.c_str(), neighbor.c_str());

			setObjectOfReference(simplifiedObjectOfRef);
		}

	}


	BfdSessionFault::~BfdSessionFault()
	{

	}

} /* namespace alarm */
