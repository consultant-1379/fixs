/*
 * FIXS_TREFC_JTPClient.h
 *
 *  Created on: Jan 24, 2012
 *      Author: eiolbel
 */

#ifndef FIXS_TREFC_JTPClient_h
#define FIXS_TREFC_JTPClient_h 1

#include "FIXS_TREFC_Util.h"
#include "ACS_JTP.h"
#include "ACS_TRA_trace.h"
#include "FIXS_TREFC_Event.h"

class FIXS_TREFC_JTPClient
{

  public:
    //## Constructors (specified)
      //## Operation: FIXS_TREFC_JTPClient%4E3BDB9002F6
      FIXS_TREFC_JTPClient (JTP_HANDLE aShutDown);

    //## Destructor (generated)
      virtual ~FIXS_TREFC_JTPClient();


    //## Other Operations (specified)
      //## Operation: newJTPClient%4E3BDD310333
      int newJTPClient (unsigned short Len, ACS_JTP_Job &J1, unsigned short U1, unsigned short U2, char *Msg);

      //## Operation: waitForConfermation%4E3BE08F0342
      int waitForConfermation (ACS_JTP_Job &J1, unsigned short U1, unsigned short U2);

      //## Operation: waitForJTPResult%4E3BE1360111
      bool waitForJTPResult (ACS_JTP_Job &J1, unsigned short Len, unsigned short U1, unsigned short U2, char *Msg);

    // Additional Public Declarations
      //## begin FIXS_TREFC_JTPClient%4E3BDB3603A9.public preserve=yes
	   enum CAN_START_RESULT
	  {
		  NO_URC = 0x00,
		  RP_URC = 0x01,
		  SIS_URC= 0x02,
		  SCX_URC= 0X03,
		  SMX_URC= 0X06
	  };
      //## end FIXS_TREFC_JTPClient%4E3BDB3603A9.public

  protected:
    // Additional Protected Declarations
      //## begin FIXS_TREFC_JTPClient%4E3BDB3603A9.protected preserve=yes
      //## end FIXS_TREFC_JTPClient%4E3BDB3603A9.protected

  private:
    // Additional Private Declarations
      //## begin FIXS_TREFC_JTPClient%4E3BDB3603A9.private preserve=yes
      //## end FIXS_TREFC_JTPClient%4E3BDB3603A9.private

  public: //## implementation
    // Data Members for Class Attributes

      //## Attribute: JOB_REJECTED%4E3BE2320084
      //## begin FIXS_TREFC_JTPClient::JOB_REJECTED%4E3BE2320084.attr preserve=no  public: static  int {UC} -1
      static const  int JOB_REJECTED;
      //## end FIXS_TREFC_JTPClient::JOB_REJECTED%4E3BE2320084.attr

      //## Attribute: JOB_FAILED%4E3BE2BC0219
      //## begin FIXS_TREFC_JTPClient::JOB_FAILED%4E3BE2BC0219.attr preserve=no  public: static  int {UC} -2
      static const  int JOB_FAILED;
      //## end FIXS_TREFC_JTPClient::JOB_FAILED%4E3BE2BC0219.attr

      //## Attribute: JOB_MISMATCH%4E3BE2DF0047
      //## begin FIXS_TREFC_JTPClient::JOB_MISMATCH%4E3BE2DF0047.attr preserve=no  public: static  int {UC} -3
      static const  int JOB_MISMATCH;
      //## end FIXS_TREFC_JTPClient::JOB_MISMATCH%4E3BE2DF0047.attr

	private: //## implementation
    // Data Members for Class Attributes
      //## Attribute: shutdown%4E3BE36402DA
      //## begin FIXS_TREFC_JTPClient::shutdown%4E3BE36402DA.attr preserve=no  private:  HANDLE {U} 0
      JTP_HANDLE m_shutdown;

      ACS_TRA_trace* traceObj;
      //## end FIXS_TREFC_JTPClient::shutdown%4E3BE36402DA.attr

    // Additional Implementation Declarations
      //## begin FIXS_TREFC_JTPClient%4E3BDB3603A9.implementation preserve=yes
      //## end FIXS_TREFC_JTPClient%4E3BDB3603A9.implementation

};


#endif
