/*
 * FIXS_TREFC_AlarmHandler.h
 *
 *  Created on: Nov 28 2012
 *      Author: xcsrpad
 */
//	*****************************************************************************

#ifndef FIXS_TREFC_AlarmHandler_h
#define FIXS_TREFC_AlarmHandler_h 1


#include <time.h>
#include <iostream>
#include <ace/Task.h>
#include <ace/ACE.h>
#include "ACS_TRA_trace.h"
#include "FIXS_TREFC_Event.h"


#include "FIXS_TREFC_Util.h"

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

class FIXS_TREFC_AlarmHandler : public ACE_Task_Base //FIXS_TREFC_AlarmHandler
{
public:
       FIXS_TREFC_AlarmHandler();

       virtual ~FIXS_TREFC_AlarmHandler();


      virtual int svc(void);
      virtual int open (void *args = 0);
     
    
      void initOnStart ();
      void stop();
      void startAlarmTimer ();
      void cancelAlarmTimer ();
      bool isAlarmTimerOngoing ();
      bool isRunningThread(){ return m_running;};

private:
    // Data Members for Class Attributes

	
      bool m_isAlarmTimerOn;
      bool m_running;
      int m_stopEvent;
      int m_shutdownEvent;
      bool m_exit;
      ACS_TRA_trace* traceObj;

    private://## implementation

};


// Class FIXS_TREFC_AlarmHandler 

inline  bool FIXS_TREFC_AlarmHandler::isAlarmTimerOngoing ()
{
  	return m_isAlarmTimerOn;
}
#endif


