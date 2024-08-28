/*
 * FIXS_TREFC_TimeChecker.h
 *
 *  Created on: Jan 9, 2012
 *      Author: eanform
 */

#ifndef FIXS_TREFC_TIMECHECKER_H_
#define FIXS_TREFC_TIMECHECKER_H_

#include <time.h>
#include <iostream>
#include <ace/Task.h>
#include <ace/ACE.h>

#include "ACS_TRA_trace.h"
#include "FIXS_TREFC_Manager.h"
#include "FIXS_TREFC_Event.h"
#include "FIXS_TREFC_Util.h"
//#include "FIXS_TREFC_CriticalSectionGuard.h"
//#include "FIXS_TREFC_thread.h"

//namespace{
//	typedef int EventHandle;
//}

class FIXS_TREFC_TimeChecker : public ACE_Task_Base //FIXS_TREFC_thread
{
public:

	FIXS_TREFC_TimeChecker ();

	virtual ~FIXS_TREFC_TimeChecker();

	void setTimeOnAllScx ();

	virtual int svc(void);

	virtual int open (void *args = 0);

//	virtual unsigned run (void *procArgs);

	void stop ();

	void startTimeSet ();

	void stopTimeSet ();

	void setScxTrap (bool trap, std::string ip, bool apzMag, unsigned short slot);

	bool getTimeSet ();

	bool isRunningThread ();



protected:
	// Additional Protected Declarations
	bool m_running;

private:
	// Data Members for Class Attributes

//	CRITICAL_SECTION *m_timeChkAccessControlPtr;

	FIXS_TREFC_EventHandle m_shutdownEvent;

	bool m_exit;

	bool m_setTime;

	bool m_scxTrap;

	std::string m_scxIp;

	unsigned short  m_scxSlot;

	bool m_isApzMag;

	ACS_TRA_trace* traceTimeChecker;

};

inline  bool FIXS_TREFC_TimeChecker::isRunningThread ()
{
	return m_running;
}

#endif /* FIXS_TREFC_TIMECHECKER_H_ */
