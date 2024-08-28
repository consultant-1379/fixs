/*
 * fixs_ith_procsignalseventhandler.h
 *
 *  Created on: Feb 3, 2016
 *      Author: xludesi
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_PROCSIGNALSEVENTHANDLER_H_
#define ITHADM_CAA_INC_FIXS_ITH_PROCSIGNALSEVENTHANDLER_H_

#include <ace/Event_Handler.h>

/*
 * Forward declarations
 */
class fixs_ith_server;

/** @class fixs_ith_procsignalseventhandler fixs_ith_procsignalseventhandler.h
 *	@brief fixs_ith_procsignalseventhandler class
 *
 *	fixs_ith_procsignalseventhandler <PUT DESCRIPTION>
 */
class fixs_ith_procsignalseventhandler : public ACE_Event_Handler {
	//==============//
	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_procsignalseventhandler constructor
	 */
	inline explicit fixs_ith_procsignalseventhandler (fixs_ith_server * fixs_ith_server)
	: ACE_Event_Handler(), _fixs_ith_server(fixs_ith_server) {}

private:
	fixs_ith_procsignalseventhandler (const fixs_ith_procsignalseventhandler & rhs);

	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_procsignalseventhandler Destructor
	 */
	inline virtual ~fixs_ith_procsignalseventhandler () {}

	//=================//
	// Field Accessors //
	//=================//
public:

	//===========//
	// Functions //
	//===========//
public:

	// BEGIN: ACE_Event_Handler interface //
  virtual int handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0);
	// END: ACE_Event_Handler interface //

	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_procsignalseventhandler & operator= (const fixs_ith_procsignalseventhandler & rhs);

	//========//
	// Fields //
	//========//
private:
	fixs_ith_server * _fixs_ith_server;
};




#endif /* ITHADM_CAA_INC_FIXS_ITH_PROCSIGNALSEVENTHANDLER_H_ */
