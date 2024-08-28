/*
 * FIXS_CCH_JTPClient.h
 *
 *  Created on: Oct 20, 2011
 *      Author: eiolbel
 */

#ifndef FIXS_CCH_JTPClient_h
#define FIXS_CCH_JTPClient_h 1

#include "FIXS_CCH_JTPComm.h"

#include "ACS_JTP.h"
#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include "FIXS_CCH_Event.h"

//#include "FIXS_CCH_CriticalSectionGuard.h"
//#include "FIXS_CCH_Logger.h"


#include <iostream>
#include <sstream>



//#include "ACS_JTP.h"


class FIXS_CCH_JTPClient
{
 public:

	//FIXS_CCH_JTPClient(HANDLE aShutDown);
	FIXS_CCH_JTPClient(FIXS_CCH_EventHandle aShutDown);

      virtual ~FIXS_CCH_JTPClient();

      int newJTPClient (unsigned short Len, ACS_JTP_Job &J1, unsigned short U1, unsigned short U2, char *Msg, int cpId);

      int waitForConfermation (ACS_JTP_Job &J1, unsigned short U1, unsigned short U2);

      bool waitForJTPResult (ACS_JTP_Job &J1, unsigned short Len, unsigned short U1, unsigned short U2, char *Msg, unsigned short &resCode);
    // Data Members for Class Attributes

      static const int JOB_REJECTED;

      static const int JOB_FAILED;

      static const int JOB_MISMATCH;

      static const int JOB_START_REQUEST_REJECTED;

    // Additional Public Declarations

  protected:
    // Additional Protected Declarations

  private:
    // Data Members for Class Attributes

     // HANDLE m_shutdown;
      FIXS_CCH_EventHandle m_shutdown;

      ACS_TRA_Logging* FIXS_CCH_logging;

      ACS_TRA_trace* traceObj;

    // Additional Private Declarations

  private:
    // Additional Implementation Declarations

};

// Class FIXS_CCH_JTPClient

#endif
