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
//	******************************************************

#ifndef ITHADM_CAA_INC_FIXS_ITH_CSHWCTABLECHANGEOBSERVER_H_
#define ITHADM_CAA_INC_FIXS_ITH_CSHWCTABLECHANGEOBSERVER_H_

#include <ace/Recursive_Thread_Mutex.h>

#include <ACS_CS_API.h>


/** @class fixs_ith_cshwctablechangeobserver acs_apbm_cshwctablechangeobserver.h
 *	@brief fixs_ith_cshwctablechangeobserver class
 *	@author xnicmut (Nicola Muto)
 *	@date 2012-02-08
 *
 *	fixs_ith_cshwctablechangeobserver <PUT DESCRIPTION>
 */
class fixs_ith_cshwctablechangeobserver : public ACS_CS_API_HWCTableObserver {
	//=============//
	// Enumeration //
	//=============//
public:
	typedef enum ObserverStateConstants {
		OBSERVER_STATE_STOPPED = 0,
		OBSERVER_STATE_STARTED = 1
	} observer_state_t;


	//==============//
	// Constructors //
	//==============//
public:
	/** @brief acs_apbm_cshwctablechangeobserver constructor
	 */
	inline explicit fixs_ith_cshwctablechangeobserver ()
	: _state(OBSERVER_STATE_STOPPED), _sync_object() {}

private:
	fixs_ith_cshwctablechangeobserver (const fixs_ith_cshwctablechangeobserver & rhs);


	//============//
	// Destructor //
	//============//
public:
	/** @brief acs_apbm_cshwctablechangeobserver Destructor
	 */
	inline virtual ~fixs_ith_cshwctablechangeobserver () {}


	//===========//
	// Functions //
	//===========//
public:
	// BEGIN: ACS_CS_API_HWCTableObserver Interface
  virtual void update (const ACS_CS_API_HWCTableChange & observee);
	// END: ACS_CS_API_HWCTableObserver Interface

  int start(ACS_CS_API_SubscriptionMgr * subscription_manager = 0);

  int stop(ACS_CS_API_SubscriptionMgr * subscription_manager = 0);

private:
  const char * hwc_table_change_op_type_str (ACS_CS_API_TableChangeOperation::OpType op_type) const;


  //===========//
	// Operators //
	//===========//
private:
	fixs_ith_cshwctablechangeobserver & operator= (const fixs_ith_cshwctablechangeobserver & rhs);


	//========//
	// Fields //
	//========//
private:
	observer_state_t _state;

	// Access synchronization controllers
	ACE_Recursive_Thread_Mutex _sync_object;
};



#endif /* ITHADM_CAA_INC_FIXS_ITH_CSHWCTABLECHANGEOBSERVER_H_ */
