/*
 * fixs_ith_happmanager.h
 *
 *  Created on: Jan 22, 2016
 *      Author: xludesi
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_HAAPPMANAGER_H_
#define ITHADM_CAA_INC_FIXS_ITH_HAAPPMANAGER_H_

#include <ACS_APGCC_ApplicationManager.h>
#include <ace/ACE.h>

#include "fixs_ith_server.h"
#include "fixs_ith_ha_task.h"

/** @class fixs_ith_haappmanager fixs_ith_haappmanager.h
 *	@brief fixs_ith_haappmanager class
 *	@author xludesi
 *	@date 2016-01-22
 *
 *	Class used to integrate the ITH Service with the HA framework
 */
class fixs_ith_haappmanager: public ACS_APGCC_ApplicationManager
{
	//==============//
	// Constructors //
	//==============//
public:
	explicit fixs_ith_haappmanager (fixs_ith_server * fixs_ith_server);

	fixs_ith_haappmanager (const char * daemon_name, fixs_ith_server * fixs_ith_server);

	fixs_ith_haappmanager (const char * daemon_name, const char * username, fixs_ith_server * fixs_ith_server);

	int start_ha_task();

	int stop_ha_task();

private:
	fixs_ith_haappmanager (const fixs_ith_haappmanager & rhs);

	//============//
	// Destructor //
	//============//
public:
	/** @brief acs_apbm_haappmanager Destructor
	 */
	virtual ~fixs_ith_haappmanager ();


	//===========//
	// Functions //
	//===========//
public:
	/*
	 * BEGIN: ACS_APGCC_ApplicationManager interface implementation
	 */
	virtual ACS_APGCC_ReturnType performStateTransitionToActiveJobs (ACS_APGCC_AMF_HA_StateT previous_state);
	virtual ACS_APGCC_ReturnType performStateTransitionToPassiveJobs (ACS_APGCC_AMF_HA_StateT previous_state);
	virtual ACS_APGCC_ReturnType performStateTransitionToQueisingJobs (ACS_APGCC_AMF_HA_StateT previous_state);
	virtual ACS_APGCC_ReturnType performStateTransitionToQuiescedJobs (ACS_APGCC_AMF_HA_StateT previous_state);
	virtual ACS_APGCC_ReturnType performComponentHealthCheck ();
	virtual ACS_APGCC_ReturnType performComponentTerminateJobs ();
	virtual ACS_APGCC_ReturnType performComponentRemoveJobs ();
	virtual ACS_APGCC_ReturnType performApplicationShutdownJobs ();
	/*
	 * END: ACS_APGCC_ApplicationManager interface implementation
	 */


private:
	fixs_ith_haappmanager & operator= (const fixs_ith_haappmanager & rhs);

	int init();

	//========//
	// Fields //
	//========//
public:

private:
	ACE_HANDLE _ha_pipe[2];
	fixs_ith_ha_task * _ha_task;
	fixs_ith_server * _fixs_ith_server;
	bool _initialized;
};



#endif /* ITHADM_CAA_INC_FIXS_ITH_HAAPPMANAGER_H_ */
