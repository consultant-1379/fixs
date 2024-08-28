/*
 * FIXS_CCH_JTPHandler.h
 *
 *  Created on: Oct 20, 2011
 *      Author: eiolbel
 */
#ifndef FIXS_CCH_JTPHandler_h
#define FIXS_CCH_JTPHandler_h 1

/*===================================================================
                        INCLUDE DECLARATION SECTION
=================================================================== */

#include <ace/Task.h>
#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"

#include "FIXS_CCH_JTPClient.h"
#include "ACS_JTP.h"
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_IMM_Util.h"
//#include "FIXS_CCH_Util.h"

#include "ace/OS_NS_poll.h"
#include <sys/eventfd.h>
#include <poll.h>
#include <iostream>
#include <string>
#include <list>
#include <map>



typedef int FIXS_CCH_EventHandle;

/*=====================================================================
					CLASS DECLARATION SECTION
==================================================================== */

class FIXS_CCH_JTPHandler : public ACE_Task_Base
{
  public:

      FIXS_CCH_JTPHandler (unsigned char shelf, unsigned char slot, int id);

      virtual ~FIXS_CCH_JTPHandler();

      virtual int svc(void);
      /**
  	   @brief  		This method initializes a task and prepare it for execution
       */
      virtual int open (void *args = 0);

      virtual void stop ();

      /**
  	   @brief  		This method get the svc thread state
       */
      bool getSvcState() const {return svc_run;};

      void boardRestarted (unsigned char msgType);

      void maintenanceWindow (unsigned char msgType);

      //virtual unsigned run (void *procArgs);

      bool stateMachine (int event_index);

     // virtual void stop ();

      void maintenanceWindow (unsigned char msgType, unsigned char slot);

      bool checkStartReqStatus ();

      void setStartReqStatus (bool value);

      void waitStartReqToFinish ();

      void waitStartReqToFinish (bool & shutdownsignaled);

      inline bool isRunningThread() { return m_running; };

      inline bool isSendingNotification() { return sendingNotification; };

      //virtual int exec(){return 0;};

    // Additional Public Declarations

      enum JOB_REQUEST_CODE
      {
    	  POWER_START_UPGRADE = 0x10,
    	  FAN_START_UPGRADE = 0x11,
    	  IMBSW_START_UPGRADE = 0x21,
    	  IPMI_START_UPGRADE = 0x20,
    	  POWER_END_UPGRADE = 0x1F,
    	  FAN_END_UPGRADE = 0x1E,
    	  IMBSW_END_UPGRADE = 0x2E,
    	  IPMI_END_UPGRADE = 0x2F,
    	  IMB_UPG_START_REQ = 0X60
      };

  protected:
    // Additional Protected Declarations

	  bool m_exit;

  private:

	  bool sendNotification (unsigned char msgType, int event_index, const int noTries);

      int handleNotification (int version, unsigned short msgType, unsigned char shelf, unsigned char slot, const int noTries);

      /**
      	   @brief  		This method signal the svc thread termination
       */
//      void signalSvcTermination();

      /**
      	   @brief  	m_oiHandler
       */

      FIXS_CCH_EventHandle m_shutdownEvent;

      FIXS_CCH_EventHandle m_StopEvent;

      FIXS_CCH_EventHandle* m_waitHandles;

      FIXS_CCH_EventHandle m_startReqEvent;

      unsigned char m_shelf;

      unsigned char m_slot;

      bool *m_stopRetry;

      bool *m_windowIsOpen;

      int m_cpId;

      bool m_startReqStatus;

      ACS_TRA_trace* _trace;

      /**
      	   @brief  	svc_run: svc state flag
       */
      bool svc_run;

      bool m_running;

      bool sendingNotification;

      ACS_TRA_Logging* FIXS_CCH_logging;


    // Additional Private Declarations

  private:
    // Additional Implementation Declarations
};

// Class FIXS_CCH_JTPHandler

#endif



