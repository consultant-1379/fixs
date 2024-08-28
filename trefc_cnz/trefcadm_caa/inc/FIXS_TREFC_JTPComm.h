/*
 * FIXS_TREFC_JTPComm.h
 *
 *  Created on: Jan 24, 2012
 *      Author: eiolbel
 */

#ifndef FIXS_TREFC_JTPComm_h
#define FIXS_TREFC_JTPComm_h 1

#include "ACS_JTP.h"
#include "FIXS_TREFC_Util.h"
#include "FIXS_TREFC_Manager.h"
#include "ACS_TRA_trace.h"

const int PROBLEMDATASIZE = 192;	// Max size of problem data string
const int MAINTENANCE_FRAME_LEN = 21;
const int MAINTENANCE_MAX_BUFFER = MAINTENANCE_FRAME_LEN * 3 + 1;  // 64 bytes

class FIXS_TREFC_JTPComm
{

  public:
    //## Constructors (specified)
      //## Operation: FIXS_TREFC_JTPComm%4E4E247C0020
      FIXS_TREFC_JTPComm (unsigned short aSystemid, ACS_JTP_Job* job);

    //## Destructor (generated)
      virtual ~FIXS_TREFC_JTPComm();


    //## Other Operations (specified)
      //## Operation: jobMode%4E4E23EA0385
      void jobMode ();

      //## Operation: setData%4E4E240B0018
      void setData ();

    // Additional Public Declarations
      //## begin FIXS_TREFC_JTPComm%4E4E0D180355.public preserve=yes
      //## end FIXS_TREFC_JTPComm%4E4E0D180355.public

  protected:
    // Additional Protected Declarations
      //## begin FIXS_TREFC_JTPComm%4E4E0D180355.protected preserve=yes
      //## end FIXS_TREFC_JTPComm%4E4E0D180355.protected

  private:
    // Additional Private Declarations
      //## begin FIXS_TREFC_JTPComm%4E4E0D180355.private preserve=yes
      //## end FIXS_TREFC_JTPComm%4E4E0D180355.private

  private: //## implementation
    // Data Members for Class Attributes

      //## Attribute: m_systemid%4E4E258103B9
      //## begin FIXS_TREFC_JTPComm::m_systemid%4E4E258103B9.attr preserve=no  private:  short unsigned {U} 0
       short unsigned m_systemid;
      //## end FIXS_TREFC_JTPComm::m_systemid%4E4E258103B9.attr

      //## Attribute: m_conversation%4E4E258F0260
      //## begin FIXS_TREFC_JTPComm::m_conversation%4E4E258F0260.attr preserve=no  private:  ACS_JTP_Job* {U} 0
       ACS_JTP_Job* m_conversation;
      //## end FIXS_TREFC_JTPComm::m_conversation%4E4E258F0260.attr

      //## Attribute: m_rec_msg%4E4E25AE0193
      //## begin FIXS_TREFC_JTPComm::m_rec_msg%4E4E25AE0193.attr preserve=no  private:  char * {U} 0
       char *m_rec_msg;
      //## end FIXS_TREFC_JTPComm::m_rec_msg%4E4E25AE0193.attr

      //## Attribute: m_rec_length%4E4E25BC0200
      //## begin FIXS_TREFC_JTPComm::m_rec_length%4E4E25BC0200.attr preserve=no  private:  short unsigned {U} 0
       short unsigned m_rec_length;
      //## end FIXS_TREFC_JTPComm::m_rec_length%4E4E25BC0200.attr

      //## Attribute: m_rec_data1%4E4E25CA0347
      //## begin FIXS_TREFC_JTPComm::m_rec_data1%4E4E25CA0347.attr preserve=no  private:  short unsigned {U} 0
       short unsigned m_rec_data1;
      //## end FIXS_TREFC_JTPComm::m_rec_data1%4E4E25CA0347.attr

      //## Attribute: m_rec_data2%4E4E260201DC
      //## begin FIXS_TREFC_JTPComm::m_rec_data2%4E4E260201DC.attr preserve=no  private:  short unsigned {U} 0
       short unsigned m_rec_data2;
      //## end FIXS_TREFC_JTPComm::m_rec_data2%4E4E260201DC.attr

      //## Attribute: m_send_length%4E4E261003BF
      //## begin FIXS_TREFC_JTPComm::m_send_length%4E4E261003BF.attr preserve=no  private:  short unsigned {U} 0
       short unsigned m_send_length;
      //## end FIXS_TREFC_JTPComm::m_send_length%4E4E261003BF.attr

      //## Attribute: m_send_data1%4E4E261E010F
      //## begin FIXS_TREFC_JTPComm::m_send_data1%4E4E261E010F.attr preserve=no  private:  short unsigned {U} 0
       short unsigned m_send_data1;
      //## end FIXS_TREFC_JTPComm::m_send_data1%4E4E261E010F.attr

      //## Attribute: m_send_data2%4E4E262A0237
      //## begin FIXS_TREFC_JTPComm::m_send_data2%4E4E262A0237.attr preserve=no  private:  short unsigned {U} 0
       short unsigned m_send_data2;
      //## end FIXS_TREFC_JTPComm::m_send_data2%4E4E262A0237.attr

      //## Attribute: m_send_result%4E4E2636013C
      //## begin FIXS_TREFC_JTPComm::m_send_result%4E4E2636013C.attr preserve=no  private:  short unsigned {U} 0
       short unsigned m_send_result;
      //## end FIXS_TREFC_JTPComm::m_send_result%4E4E2636013C.attr

      //## Attribute: m_send_buffer%4E4E264602F1
      //## begin FIXS_TREFC_JTPComm::m_send_buffer%4E4E264602F1.attr preserve=no  private:  char * {U}
       char *m_send_buffer;
      //## end FIXS_TREFC_JTPComm::m_send_buffer%4E4E264602F1.attr

       ACS_TRA_trace* traceObj;
    // Additional Implementation Declarations


};


#endif
