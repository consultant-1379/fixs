/*
 * FIXS_TREFC_JTPServer.h
 *
 *  Created on: Jan 24, 2012
 *      Author: eiolbel
 */

#ifndef FIXS_TREFC_JTPServer_h
#define FIXS_TREFC_JTPServer_h 1

#include "ACS_JTP.h"
#include "ACS_TRA_trace.h"
#include <iostream>
#include "FIXS_TREFC_JTPComm.h"
#include <ace/Task.h>

class FIXS_TREFC_JTPServer : public ACE_Task_Base  //## Inherits: <unnamed>%4E3A772D0090
{

  public:
    //## Constructors (generated)
      FIXS_TREFC_JTPServer();

    //## Destructor (generated)
      virtual ~FIXS_TREFC_JTPServer();


    //## Other Operations (specified)
      //## Operation: waitUntilSignaled%4E3BD27902C9
      int waitUntilSignaled (int no, JTP_HANDLE *mixedHandles);

      //## Operation: run%4E3BD2D50093
      virtual int svc();
      virtual int open (void *args = 0);

      //## Operation: stop%4E3BD30F02D3
      virtual void stop ();

    // Additional Public Declarations
      //## begin FIXS_TREFC_JTPServer%4E3A772D008F.public preserve=yes
      //## end FIXS_TREFC_JTPServer%4E3A772D008F.public


  protected:
    // Additional Protected Declarations
      //## begin FIXS_TREFC_JTPServer%4E3A772D008F.protected preserve=yes
      //## end FIXS_TREFC_JTPServer%4E3A772D008F.protected
      bool m_exit;
  private:
    //## Assignment Operation (generated)
      FIXS_TREFC_JTPServer & operator=(const FIXS_TREFC_JTPServer &right);

    // Additional Private Declarations
      //## begin FIXS_TREFC_JTPServer%4E3A772D008F.private preserve=yes
      //## end FIXS_TREFC_JTPServer%4E3A772D008F.private

  private: //## implementation
    // Data Members for Class Attributes

      //## Attribute: hStopGracefully%4E3BD399039C
      //## begin FIXS_TREFC_JTPServer::hStopGracefully%4E3BD399039C.attr preserve=no  private:  HANDLE {U} 0
      EventHandle m_hStopGracefully;

      ACS_TRA_trace* traceObj;
      //## end FIXS_TREFC_JTPServer::hStopGracefully%4E3BD399039C.attr

    // Additional Implementation Declarations
      //## begin FIXS_TREFC_JTPServer%4E3A772D008F.implementation preserve=yes
      //## end FIXS_TREFC_JTPServer%4E3A772D008F.implementation

};

#endif
