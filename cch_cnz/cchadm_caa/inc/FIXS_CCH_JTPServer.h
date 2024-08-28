/*
 * FIXS_CCH_JTPServer.h
 *
 *  Created on: Oct 20, 2011
 *      Author: eiolbel
 */
#ifndef FIXS_CCH_JTPServer_h
#define FIXS_CCH_JTPServer_h 1

// FIXS_CCH_JTPComm
#include "FIXS_CCH_JTPComm.h"
#include "FIXS_CCH_Event.h"
#include "ACS_JTP.h"
#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include <ace/Task.h>
#include <iostream>

class FIXS_CCH_JTPServer : public ACE_Task_Base
{
  public:

      FIXS_CCH_JTPServer();

      virtual ~FIXS_CCH_JTPServer();

      //int waitUntilSignaled (int no, FIXS_CCH_EventHandle *mixedHandles);
      int waitUntilSignaled (int no, JTP_HANDLE *mixedHandles);

     // virtual unsigned run (void *procArgs);

      virtual int open (void *args = 0);

      virtual int svc(void);

      virtual void stop ();

    // Additional Public Declarations

  protected:
    // Additional Protected Declarations

      bool m_exit;

  private:

      FIXS_CCH_JTPServer(const FIXS_CCH_JTPServer &right);

      FIXS_CCH_JTPServer & operator=(const FIXS_CCH_JTPServer &right);

    // Data Members for Class Attributes

      EventHandle m_hStopGracefully;

      ACS_TRA_trace *traceObj;

      ACS_TRA_Logging *FIXS_CCH_logging;

      //HANDLE m_hStopGracefully;

    // Additional Private Declarations

  private:
    // Additional Implementation Declarations

};

// Class FIXS_CCH_JTPServer

#endif
