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
 *  Created on: Jul 11, 2016
 *      Author: xvincon
 */

#include "alarm/vrrpmasterchange.h"

#include <boost/format.hpp>

namespace alarms
{
	namespace VrrpMasterChangeFields
	{
		const uint32_t specificProblem = 35203;
		const std::string severity = "O2";
		const std::string probableCause = "VRRP MASTER CHANGE";
		const std::string objectClassOfReference = "APZ";
		const std::string problemData;
		const std::string problemText = "VRRP INTERFACE\n%s";
		const bool manualCease = true;
	}

	VrrpMasterChange::VrrpMasterChange(const std::string& objOfReference)
	: Alarm(Alarm::VRRP_MASTER_CHANGE,
			VrrpMasterChangeFields::objectClassOfReference,
			objOfReference,
			VrrpMasterChangeFields::severity,
			VrrpMasterChangeFields::specificProblem,
			VrrpMasterChangeFields::probableCause,
			VrrpMasterChangeFields::problemData,
			VrrpMasterChangeFields::manualCease
			)
	{
		setObjectOfReference(objOfReference);
		setTimestamp();

		//Build problemText
		boost::format problemTextFormat(VrrpMasterChangeFields::problemText);
		problemTextFormat.exceptions( boost::io::all_error_bits ^ ( boost::io::too_many_args_bit | boost::io::too_few_args_bit ) );
		problemTextFormat % objOfReference;

		setProblemText(problemTextFormat.str());
	}


	VrrpMasterChange::~VrrpMasterChange()
	{

	}

} /* namespace alarm */
