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

#ifndef ITHADM_CAA_INC_FIXS_ITH_HA_EVENTHANDLER_H_
#define ITHADM_CAA_INC_FIXS_ITH_HA_EVENTHANDLER_H_

#include <ace/Event_Handler.h>

#include "fixs_ith_server.h"

// Event handler for handling requests coming from the HA framework
class fixs_ith_ha_eventhandler : public ACE_Event_Handler
{
	//==============//
	// Constructors //
	//==============//
public:
	inline explicit fixs_ith_ha_eventhandler (fixs_ith_server *fixs_ith_server, ACE_HANDLE handle = ACE_INVALID_HANDLE)
	: ACE_Event_Handler()
	{
		_fixs_ith_server = fixs_ith_server;
		  _handle = handle;
	}

private:
	fixs_ith_ha_eventhandler (const fixs_ith_ha_eventhandler & rhs);

	//============//
	// Destructor //
	//============//
public:
	inline virtual ~fixs_ith_ha_eventhandler () {}

	//===========//
	// Functions //
	//===========//
public:

	// BEGIN: ACE_Event_Handler interface //
	inline virtual ACE_HANDLE get_handle () const { return _handle; }

	inline virtual void set_handle (ACE_HANDLE handle) { _handle = handle; }

	virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);

	inline virtual int handle_close (ACE_HANDLE /*handle*/, ACE_Reactor_Mask /*close_mask*/) {return 0;}
	// END: ACE_Event_Handler interface //

private:

	int handle_ha_framework_request (ACE_HANDLE fd, int op);

	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_ha_eventhandler & operator= (const fixs_ith_ha_eventhandler & rhs);

	//========//
	// Fields //
	//========//
private:
	ACE_HANDLE _handle;
	fixs_ith_server * _fixs_ith_server;
};


#endif /* ITHADM_CAA_INC_FIXS_ITH_HA_EVENTHANDLER_H_ */
