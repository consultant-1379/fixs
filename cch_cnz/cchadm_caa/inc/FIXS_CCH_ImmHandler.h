/*
 * FIXS_CCH_ImmHandler.h
 *
 *  Created on: 29 Jun 2011
 *      Author: eanform
 */

#ifndef FIXS_CCH_IMMHANDLER_H_
#define FIXS_CCH_IMMHANDLER_H_


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

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"

#include "FIXS_CCH_BladeSwManagement.h"
#include "FIXS_CCH_ApManagedSw.h"
#include "FIXS_CCH_CpManagedSw.h"
#include "FIXS_CCH_BladePercistanceValue.h"
#include "FIXS_CCH_BrmPersistentDataOwner.h"
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_Util.h"
#include "FIXS_CCH_IMM_Util.h"



class FIXS_CCH_BrfSubscriber: public ACE_Task_Base {
public:
	FIXS_CCH_BrfSubscriber(acs_apgcc_oihandler_V3 *handler, FIXS_CCH_BrmPersistentDataOwner* br, FIXS_CCH_EventHandle subscriptionEvent):
		m_oiHandlerBrmPersistentDataOwner(handler),
		m_oi_BrmPersistentDataOwner(br),
		m_subscriptionEvent(subscriptionEvent),
		m_stopEvent(FIXS_CCH_Event::CreateEvent(false,false,0)),
		svc_run(false)
	{};
	virtual ~FIXS_CCH_BrfSubscriber(){FIXS_CCH_Event::CloseEvent(m_stopEvent);};

	virtual int svc(void);

	virtual int open (void ) {activate(THR_JOINABLE);return 0;};

	virtual void stop (){FIXS_CCH_Event::SetEvent(m_stopEvent);};

private:
	bool setBrfImplementer();

	acs_apgcc_oihandler_V3 *m_oiHandlerBrmPersistentDataOwner;
	FIXS_CCH_BrmPersistentDataOwner * m_oi_BrmPersistentDataOwner;
	FIXS_CCH_EventHandle m_subscriptionEvent;
	FIXS_CCH_EventHandle m_stopEvent;

	bool svc_run;

};

class FIXS_CCH_Implementer: public ACE_Task_Base {
public:
	FIXS_CCH_Implementer(acs_apgcc_oihandler_V3 **handler, acs_apgcc_objectimplementerinterface_V3** implementer, FIXS_CCH_EventHandle *subscriptionEvent, std::string *names, uint32_t size):
		m_oiHandler(handler),
		m_oImplementer(implementer),
		m_subscriptionEvent(subscriptionEvent),
		m_names(names),
		m_stopEvent(FIXS_CCH_Event::CreateEvent(false,false,0)),
		m_size(size),
		svc_run(false)
	{};
	virtual ~FIXS_CCH_Implementer(){FIXS_CCH_Event::CloseEvent(m_stopEvent);};

	virtual int svc(void);

	virtual int open (void ) {activate(THR_JOINABLE);return 0;};

	virtual void stop (){FIXS_CCH_Event::SetEvent(m_stopEvent);};

private:
	bool setImplementer(uint32_t);

	acs_apgcc_oihandler_V3 **m_oiHandler;
	acs_apgcc_objectimplementerinterface_V3 **m_oImplementer;
	FIXS_CCH_EventHandle *m_subscriptionEvent;
	std::string *m_names;
	FIXS_CCH_EventHandle m_stopEvent;
	uint32_t m_size;
	//bool *implementerSet;

	bool svc_run;

};

/*=====================================================================
					CLASS DECLARATION SECTION
==================================================================== */
class FIXS_CCH_ImmHandler: public ACE_Task_Base {
 public:

	/**
		@brief		Constructor
	*/
	FIXS_CCH_ImmHandler();

	/**
		@brief		Destructor
	*/
	virtual ~FIXS_CCH_ImmHandler();

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

	virtual void stop ();


 private:

	/**
	   @brief  		This method signal the svc thread termination
	*/
	void signalSvcTermination();

	/**
	   @brief  	m_oiHandler
	 */

	ACS_TRA_trace* _trace;

	acs_apgcc_oihandler_V3 m_oiHandlerBladeSwManagement;

	FIXS_CCH_BladeSwManagement * m_oi_BladeSwManagement;

	acs_apgcc_oihandler_V3 m_oiHandlerApManagedSw;

	FIXS_CCH_ApManagedSw * m_oi_ApManagedSw;

	acs_apgcc_oihandler_V3 m_oiHandlerCpManagedSw;

	FIXS_CCH_CpManagedSw * m_oi_CpManagedSw;

	acs_apgcc_oihandler_V3 m_oiHandlerBladePercistance;

	FIXS_CCH_BladePercistanceValue * m_oi_BladePercistanceValue;

	acs_apgcc_oihandler_V3 m_oiHandlerBrmPersistentDataOwner;

	FIXS_CCH_BrmPersistentDataOwner * m_oi_BrmPersistentDataOwner;

	FIXS_CCH_EventHandle m_StopEvent;

	/**
	   @brief  	m_svcTerminated: to signal out-side the svc termination
	*/
	int m_svcTerminated;

	/**
	   @brief  	svc_run: svc state flag
	*/
	bool svc_run;

	CCH_Util::EnvironmentType env;

	ACS_TRA_Logging *FIXS_CCH_logging;


};

#endif /* FIXS_CCH_IMMHANDLER_H_ */

