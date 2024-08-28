#include <ace/Reactor.h>
#include <ace/Event_Handler.h>
#include <ace/Time_Value.h>
#include <iostream>
#include <sys/eventfd.h>
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_Util.h"
#include "FIXS_CCH_DiskHandler.h"
#include "unistd.h"
#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include <ace/Task.h>
#ifndef FIXS_CCH_TRAPTIMER_H_
#define FIXS_CCH_TRAPTIMER_H_

namespace{
	typedef int EventHandle;
}


class FIXS_CCH_TrapTimer : public ACE_Task_Base {
public:
	virtual ~FIXS_CCH_TrapTimer();

	FIXS_CCH_TrapTimer();
	int setTrapTime(int sec);
    int getEvent();
    int closeHandle();
    int stopTimer();
    virtual int svc(void);
    virtual int open (void *args = 0);
    bool isRunningThread(){ return m_running;};


private:
    FIXS_CCH_EventHandle eventToNotify;
    FIXS_CCH_EventHandle m_StopEvent;
    std::string eventStopName;
    int sec;
    int timerId;
    FIXS_CCH_EventHandle m_shutdownEvent;
    ACS_TRA_trace *trace;
    ACS_TRA_Logging *FIXS_CCH_logging;

    bool m_exit;
    bool m_running;

};


#endif /* FIXS_CCH_TRAPTIMER_H_ */



