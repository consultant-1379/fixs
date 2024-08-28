/*
 * FIXS_TREFC_JTPHandler.h
 *
 *  Created on: Jan 24, 2012
 *      Author: eiolbel
 */

#ifndef FIXS_TREFC_JTPHandler_h
#define FIXS_TREFC_JTPHandler_h 1

#include <ace/Task.h>
#include "FIXS_TREFC_Event.h"
#include "FIXS_TREFC_JTPClient.h"
#include "ACS_TRA_trace.h"
typedef int FIXS_TREFC_EventHandle;

class FIXS_TREFC_JTPHandler : public ACE_Task_Base
{

  public:
    //## Constructors (specified)
      //## Operation: FIXS_TREFC_JTPHandler%4E3A852003C4
      FIXS_TREFC_JTPHandler (unsigned char shelf, bool slot0, bool slot25);

    //## Destructor (generated)
      virtual ~FIXS_TREFC_JTPHandler();


    //## Other Operations (specified)
      //## Operation: setJtpNotificationEvent%4E3B7D5201C1
      void setJtpNotificationEvent (unsigned char msgType, int servType);

      //## Operation: run%4E3B7D980023
      virtual int svc(void);

      virtual int open (void *args = 0);


      //## Operation: sendJOBNotificationToCP%4E3B7DBF016A
      bool sendJOBNotificationToCP (int event_index);

      //## Operation: stop%4E3B7DE2016E
      virtual void stop ();

      //## Operation: waitCanStartQueryToFinish%4E3B7DFB03AE
      void waitCanStartQueryToFinish ();

      //## Operation: checkCanSendStartR%4E3B7E1401E5
      bool checkCanSendStartR ();

      //## Operation: setReferenceValues%4E3B7E2C0009
      void setReferenceValues (unsigned long ipVal, unsigned short refType);

      void updateApzScx(unsigned short slot, bool add);

    // Additional Public Declarations
      //## begin FIXS_TREFC_JTPHandler%4E3A7708020D.public preserve=yes
	  enum JOB_REQUEST_CODE
	  {
		TIME_SERVICE_START = 0x30,
		TIME_SERVICE_STOP = 0x3E,
		TIME_SERVICE_CAN_START = 0x31,
		TIME_REFERENCE_ADD = 0x50,
		TIME_REFERENCE_REMOVE = 0x5E,
		TIME_SERVICE_STATUS_REQUEST = 0x41
	  };

	enum SERVICE_TYPE
	{
		NTP = 0x00,
		NTPHP = 0x01,
		NTPHP_GPS = 0x02
	};

      //## end FIXS_TREFC_JTPHandler%4E3A7708020D.public
  protected:
    // Additional Protected Declarations
      //## begin FIXS_TREFC_JTPHandler%4E3A7708020D.protected preserve=yes
      //## end FIXS_TREFC_JTPHandler%4E3A7708020D.protected

  private:

    //## Other Operations (specified)
      //## Operation: sendNotification%4E3B8C160387
      bool sendNotification (unsigned char msgType, int event_index, const int noTries);

      //## Operation: handleNotification%4E3B8D2B0121
      int handleNotification (int version, unsigned short msgType, const int noTries);

    // Additional Private Declarations
      //## begin FIXS_TREFC_JTPHandler%4E3A7708020D.private preserve=yes
      //## end FIXS_TREFC_JTPHandler%4E3A7708020D.private

  private: //## implementation
    // Data Members for Class Attributes

      //## Attribute: jtpAccessControlPtr%4E3B8E02005C
      //## begin FIXS_TREFC_JTPHandler::jtpAccessControlPtr%4E3B8E02005C.attr preserve=no  private:  CRITICAL_SECTION * {U} 0
     //  CRITICAL_SECTION *m_jtpAccessControlPtr;
      //## end FIXS_TREFC_JTPHandler::jtpAccessControlPtr%4E3B8E02005C.attr

      //## Attribute: shutdownEvent%4E3B9810020A
      //## begin FIXS_TREFC_JTPHandler::shutdownEvent%4E3B9810020A.attr preserve=no  private:  HANDLE {U} 0
       FIXS_TREFC_EventHandle m_shutdownEvent;
      //## end FIXS_TREFC_JTPHandler::shutdownEvent%4E3B9810020A.attr

      //## Attribute: waitHandles%4E3B982C0376
      //## begin FIXS_TREFC_JTPHandler::waitHandles%4E3B982C0376.attr preserve=no  private:  HANDLE * {U} NULL
       FIXS_TREFC_EventHandle *m_waitHandles;
      //## end FIXS_TREFC_JTPHandler::waitHandles%4E3B982C0376.attr

      //## Attribute: canStartEvent%4E3B984D00A6
      //## begin FIXS_TREFC_JTPHandler::canStartEvent%4E3B984D00A6.attr preserve=no  private:  HANDLE {U} 0
       FIXS_TREFC_EventHandle m_canStartEvent;
      //## end FIXS_TREFC_JTPHandler::canStartEvent%4E3B984D00A6.attr

      //## Attribute: exit%4E3B986A0178
      //## begin FIXS_TREFC_JTPHandler::exit%4E3B986A0178.attr preserve=no  private:  bool {U} false
       bool m_exit;
      //## end FIXS_TREFC_JTPHandler::exit%4E3B986A0178.attr

      //## Attribute: canSendStartR%4E3B9881028A
      //## begin FIXS_TREFC_JTPHandler::canSendStartR%4E3B9881028A.attr preserve=no  private:  bool {U} false
       bool m_canSendStartR;
      //## end FIXS_TREFC_JTPHandler::canSendStartR%4E3B9881028A.attr

      //## Attribute: shelf%4E3B989C03A6
      //## begin FIXS_TREFC_JTPHandler::shelf%4E3B989C03A6.attr preserve=no  private:  char unsigned {U} 0
       char unsigned m_shelf;
      //## end FIXS_TREFC_JTPHandler::shelf%4E3B989C03A6.attr

      //## Attribute: slot0%4E3B98C30142
      //## begin FIXS_TREFC_JTPHandler::slot0%4E3B98C30142.attr preserve=no  private:  bool {U} false
       bool m_slot0;
      //## end FIXS_TREFC_JTPHandler::slot0%4E3B98C30142.attr

      //## Attribute: slot25%4E3B98D60016
      //## begin FIXS_TREFC_JTPHandler::slot25%4E3B98D60016.attr preserve=no  private:  bool {U} false
       bool m_slot25;
      //## end FIXS_TREFC_JTPHandler::slot25%4E3B98D60016.attr

      //## Attribute: stopRetry%4E3B98F202FB
      //## begin FIXS_TREFC_JTPHandler::stopRetry%4E3B98F202FB.attr preserve=no  private:  bool * {U} NULL
       //bool *m_stopRetry;
      //## end FIXS_TREFC_JTPHandler::stopRetry%4E3B98F202FB.attr

      //## Attribute: servType%4E3B9913022F
      //## begin FIXS_TREFC_JTPHandler::servType%4E3B9913022F.attr preserve=no  private:  int {U} 0
       int m_servType;
      //## end FIXS_TREFC_JTPHandler::servType%4E3B9913022F.attr

      //## Attribute: ref%4E3B99260152
      //## begin FIXS_TREFC_JTPHandler::ref%4E3B99260152.attr preserve=no  private:  int unsigned {U}
       int unsigned m_ref;
      //## end FIXS_TREFC_JTPHandler::ref%4E3B99260152.attr

      //## Attribute: ipValue%4E3B9941028D
      //## begin FIXS_TREFC_JTPHandler::ipValue%4E3B9941028D.attr preserve=no  private:  long unsigned {U}
       long unsigned ipValue;
      //## end FIXS_TREFC_JTPHandler::ipValue%4E3B9941028D.attr

      //## Attribute: retryOn%4E4CD6AB0353
      //## begin FIXS_TREFC_JTPHandler::retryOn%4E4CD6AB0353.attr preserve=no  private:  bool {U} false
       bool m_retryOn;
      //## end FIXS_TREFC_JTPHandler::retryOn%4E4CD6AB0353.attr

       ACS_TRA_trace* traceObj;

    // Additional Implementation Declarations

};


#endif
