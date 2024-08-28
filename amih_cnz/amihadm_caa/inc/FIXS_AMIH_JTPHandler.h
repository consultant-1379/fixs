//
//	.NAME
//	    FIXS_AMIH_JTPHandler
//	.LIBRARY 3C++
//	.PAGENAME FIXS_CCH
//	.HEADER  FIXS_CCH
//
//	.COPYRIGHT
//	 COPYRIGHT Ericsson 2010.
//	 All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2009.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2009 or in
//	accordance with the terms and conditions stipulated in the
//	agreement/contract under which the program(s) have been
//	supplied.

#ifndef FIXS_AMIH_JTPHandler_h
#define FIXS_AMIH_JTPHandler_h 1

#include <ace/Task.h>
#include "ace/OS_NS_poll.h"
#include <sys/eventfd.h>
#include <poll.h>
#include "FIXS_AMIH_JTPClient.h"
#include "FIXS_AMIH_Event.h"
#include "FIXS_AMIH_Util.h"
#include "ACS_TRA_trace.h"

class FIXS_AMIH_JTPHandler : public ACE_Task_Base  //## Inherits: <unnamed>%4C08D69402CC
{

  public:

	FIXS_AMIH_JTPHandler();

	virtual ~FIXS_AMIH_JTPHandler();

	void isNotificationNeeded ();

	void setNotificationNeeded ();

	int handleNotification (int version);

	void waitForNotifications (int numOfWaitHandles);

	virtual int svc(void);

	virtual int open (void *args = 0);

	static const int MAX_RETRIES;

	static const int JOB_REJECTED;

	static const int JOB_FAILED;

	static const int JOB_MISMATCH;

	void printTRA(std::string mesg);

  private:

      ACS_TRA_trace* _trace;
      int *m_waitHandles;
      int m_noOfJTPHandles;
      bool m_stopRequested;
      int  m_shutdownEvent;
      bool m_notificationOn;
};

#endif
