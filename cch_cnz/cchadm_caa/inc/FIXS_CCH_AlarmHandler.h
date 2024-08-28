/*
 * FIXS_CCH_AlarmHandler.h
 *
 *  Created on: Nov 15 2012
 *      Author: xcsrpad
 */
//	*****************************************************************************

#ifndef FIXS_CCH_AlarmHandler_h
#define FIXS_CCH_AlarmHandler_h 1


#include <time.h>
#include <iostream>
#include <ace/Task.h>
#include <ace/ACE.h>
#include "FIXS_CCH_DiskHandler.h"
#include "ACS_TRA_trace.h"
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_TrapTimer.h"
//#include "FIXS_CCH_UpgradeManager.h"
#include "FIXS_CCH_Util.h"

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

class FIXS_CCH_AlarmHandler : public ACE_Task_Base //FIXS_CCH_AlarmHandler
{
public:
       FIXS_CCH_AlarmHandler();

       virtual ~FIXS_CCH_AlarmHandler();


      virtual int svc(void);
	  virtual int open (void *args = 0);

      //## Operation: stop%50011A5A0016
      void stop ();

     //## Operation: initOnStart%50011A6E0382
      void initOnStart ();
	  void resetEvents();

      //## Operation: startScxAlarmTimer%50011A8A02D6
      void startEgem2L2SwitchAlarmTimer ();

      //## Operation: cancelScxAlarmTimer%50011AAB01CD
      void cancelEgem2L2SwitchAlarmTimer ();

      //## Operation: isSCXAlarmTimerOngoing%50011AC800D4
      bool isEgem2L2SwitchAlarmTimerOngoing ();

	  //## Operation: startCmxAlarmTimer%50011AEF03D2
      void startCmxAlarmTimer ();

      //## Operation: cancelCmxAlarmTimer%50011B10001A
      void cancelCmxAlarmTimer ();

      //## Operation: isCMXAlarmTimerOngoing%50011B2B0059
      bool isCMXAlarmTimerOngoing ();
	  FIXS_CCH_TrapTimer *m_cmxTrapTimer;
	  FIXS_CCH_TrapTimer *m_egem2L2SwitchTrapTimer;
    ACS_TRA_Logging *FIXS_CCH_logging; 
  private:
    // Data Members for Class Attributes

	
      bool m_isEgem2L2SwitchTimerOn;
      bool m_isCmxTimerOn;
      bool m_running;
      int m_egem2L2SwitchAlarmTimeoutEvent;
      int m_cmxAlarmTimeoutEvent;
      int m_stopEvent;
      int m_shutdownEvent;
      bool m_exit;
      CCH_Util::EnvironmentType environment;
      ACS_TRA_trace* traceObj;

    private://## implementation

};


// Class FIXS_CCH_AlarmHandler 

inline  bool FIXS_CCH_AlarmHandler::isEgem2L2SwitchAlarmTimerOngoing ()
{
  	return m_isEgem2L2SwitchTimerOn;
 }

inline  bool FIXS_CCH_AlarmHandler::isCMXAlarmTimerOngoing ()
{
	return m_isCmxTimerOn;
}
#endif


