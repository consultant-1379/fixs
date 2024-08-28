/*
 * fixs_ith_switchboardinfo.h
 *
 *  Created on: Jan 27, 2016
 *      Author: xludesi
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_SWITCHBOARDINFO_H_
#define ITHADM_CAA_INC_FIXS_ITH_SWITCHBOARDINFO_H_

#include <stdint.h>

#include "fixs_ith_programconstants.h"


class fixs_ith_switchboardinfo {

	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_switchboardinfo constructor
	 */
	inline fixs_ith_switchboardinfo ()
	: slot_position(fixs_ith::BOARD_SLOT_UNDEFINED), magazine(fixs_ith::BOARD_MAGAZINE_UNDEFINED),
	  control_ipna_str(), control_ipnb_str(), transport_ipna_str(), transport_ipnb_str(), state(-1), neighbour_state(-1), snmp_link_down_time(0), magazine_id(fixs_ith::UNDEFINED_MAGAZINE) /*, coldstart_cnt(0)*/ {
		*control_ipna_str = 0;
		*control_ipnb_str = 0;
		*transport_ipna_str = 0;
		*transport_ipnb_str = 0;
	}

	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_switchboardinfo Destructor
	 */
	inline ~fixs_ith_switchboardinfo () {}

	//===========//
	// Functions //
	//===========//
public:
	//===========//
	// Operators //
	//===========//
public:
	//========//
	// Fields //
	//========//
	int32_t slot_position;

	uint32_t magazine;

	char control_ipna_str [16];
	char control_ipnb_str [16];
	char transport_ipna_str [16];
	char transport_ipnb_str [16];

	int state;
	int neighbour_state;

	// store the time of detected link down
	int64_t snmp_link_down_time;

	fixs_ith::magazine_identifier_t magazine_id;

private:
	//========//
		// Fields //
		//========//

};


#endif /* ITHADM_CAA_INC_FIXS_ITH_SWITCHBOARDINFO_H_ */
