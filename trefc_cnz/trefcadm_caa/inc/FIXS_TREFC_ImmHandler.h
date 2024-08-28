/*
 * FIXS_TREFC_ImmHandler.h
 *
 *  Created on: 29 Jun 2011
 *      Author: eanform
 */

#ifndef FIXS_TREFC_IMMHANDLER_H_
#define FIXS_TREFC_IMMHANDLER_H_


/*===================================================================
                        INCLUDE DECLARATION SECTION
=================================================================== */
#include <ace/Task.h>
#include "ace/OS_NS_poll.h"
#include <sys/eventfd.h>
#include <poll.h>
#include <iostream>
#include <string>
#include <list>
#include <map>

#include <acs_apgcc_oihandler_V2.h>
#include <acs_apgcc_oihandler_V3.h>
#include "acs_apgcc_omhandler.h"
#include "acs_apgcc_adminoperation.h"

#include "ACS_TRA_trace.h"

#include "FIXS_TREFC_Event.h"
#include "FIXS_TREFC_OI_TimeServer.h"
#include "FIXS_TREFC_OI_SwitchTimeService.h"
#include "FIXS_TREFC_OI_SwitchTimeReference.h"
#include "FIXS_TREFC_OI_TimeService.h"
#include "FIXS_TREFC_OI_NetworkConfiguration.h"
#include "FIXS_TREFC_Util.h"
#include "FIXS_TREFC_IMM_Util.h"


//class ACS_TRA_trace;

/*=====================================================================
					CLASS DECLARATION SECTION
==================================================================== */
class FIXS_TREFC_ImmHandler: public ACE_Task_Base {
 public:

	/**
		@brief		Constructor
	*/
	FIXS_TREFC_ImmHandler();

	/**
		@brief		Destructor
	*/
	virtual ~FIXS_TREFC_ImmHandler();

	/**
	   @brief  		Run by a daemon thread
	*/
	virtual int svc(void);

	/**
	   @brief  		This method initializes a task and prepare it for execution
	*/
	virtual int open (void *args = 0);

	/**
	   @brief  		This method get the stop handle to terminate the svc thread
	*/
	void getStopHandle(int& stopFD){stopFD = m_StopEvent;};

	/**
	   @brief  		This method get the svc thread termination handle
	*/
	int& getSvcEndHandle() {return m_svcTerminated;};

	/**
	   @brief  		This method get the svc thread state
	*/
	bool getSvcState() const {return svc_run;};


 private:

	/**
	   @brief  		This method signal the svc thread termination
	*/
	void signalSvcTermination();

	bool setImplementer(bool isSMX);

	/**
	   @brief  	m_oiHandler
	*/

	ACS_TRA_trace* _trace;

	FIXS_TREFC_OI_TimeServer* m_oi_TimeServer;
	
	FIXS_TREFC_OI_SwitchTimeService* m_oi_SwitchTimeService;

	FIXS_TREFC_OI_SwitchTimeReference* m_oi_SwitchTimeReference;

	FIXS_TREFC_OI_TimeService* m_oi_TimeService;

	FIXS_TREFC_OI_NetworkConfiguration* m_oi_NetworkConfiguration;

	acs_apgcc_oihandler_V2 m_oiHandler_TimeServer;

	acs_apgcc_oihandler_V2 m_oiHandler_SwitchTimeService;

	acs_apgcc_oihandler_V2 m_oiHandler_SwitchTimeReference;

	acs_apgcc_oihandler_V2 m_oiHandler_TimeService;

	acs_apgcc_oihandler_V2 m_oiHandler_NetworkConfiguration;


	int m_StopEvent;

	/**
	   @brief  	m_svcTerminated: to signal out-side the svc termination
	*/
	int m_svcTerminated;

	/**
	   @brief  	svc_run: svc state flag
	*/
	bool svc_run;

	bool m_env;


};

#endif /* FIXS_TREFC_IMMHANDLER_H_ */

