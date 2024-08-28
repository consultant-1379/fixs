/*
 * FIXS_CCH_JTPComm.h
 *
 *  Created on: Oct 20, 2011
 *      Author: eiolbel
 */

#ifndef FIXS_CCH_JTPComm_h
#define FIXS_CCH_JTPComm_h 1

#include <iostream>
#include <stdio.h>
#include <ACS_TRA_trace.h>
#include "FIXS_CCH_UpgradeManager.h"
#include "ACS_JTP.h"
#include "FIXS_CCH_Util.h"

const int PROBLEMDATASIZE = 192;	// Max size of problem data string
const int MAINTENANCE_FRAME_LEN = 21;
const int MAINTENANCE_MAX_BUFFER = MAINTENANCE_FRAME_LEN * 3 + 1;  // 64 bytes

class FIXS_CCH_JTPComm
{
  public:

      FIXS_CCH_JTPComm (DWORD interval, unsigned short aSystemid, ACS_JTP_Conversation *conv);

      virtual ~FIXS_CCH_JTPComm();

      int conversation ();

      DWORD receiveMsg ();

      void data (unsigned short frameType, unsigned short frameLen);

    // Additional Public Declarations

  protected:
    // Additional Protected Declarations

  private:

      void IssueJTPevent (int code);
    // Data Members for Class Attributes

      unsigned short m_systemid;

      ACS_JTP_Conversation *m_conversation;

      DWORD m_interval;

      //	received JTP buffer.
      char *m_rec_msg;

      //	buffer length.

      unsigned short m_rec_length;

      unsigned short m_rec_data1;

      unsigned short m_rec_data2;

      //	buffer length.

      unsigned short m_send_length;

      unsigned short m_send_data1;

      unsigned short m_send_data2;

      unsigned short m_send_result;

      //	JTP buffer to send.

      char *m_send_buffer;

      CCH_Util::RecOfMaintenance *m_listOfMaitenance;

      int m_iteratorCount;

      ACS_TRA_trace* traceObj;

    // Additional Private Declarations

  private:
    // Additional Implementation Declarations

	  void log_JTP_data(int boardListLen);
};

// Class FIXS_CCH_JTPComm

#endif

