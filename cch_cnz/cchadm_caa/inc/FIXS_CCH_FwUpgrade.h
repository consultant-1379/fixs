/*
 * FIXS_CCH_FwUpgrade.h
 *
 *  Created on: Mar 14, 2012
 *      Author: eanform
 */

#ifndef FIXS_CCH_FWUPGRADE_H_
#define FIXS_CCH_FWUPGRADE_H_

#include <stdio.h>
#include <string.h>
#include <vector>
#include <iomanip>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <algorithm>
#include <iostream> //debugging
#include <sys/eventfd.h>
#include <vector>
#include <map>
#include <string>

#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Client_Server_Interface.h"
#include "FIXS_CCH_FSMDataStorage.h"
#include "FIXS_CCH_Util.h"
#include "FIXS_CCH_UpgradeManager.h"
#include "FIXS_CCH_DiskHandler.h"
#include "FIXS_CCH_StateMachine.h"
#include "FIXS_CCH_SNMPManager.h"
#include "FIXS_CCH_TrapTimer.h"

#include "ACS_TRA_trace.h"

class FIXS_CCH_JTPHandler;

XERCES_CPP_NAMESPACE_USE
XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

namespace{
	typedef int EventHandle;
}

class FIXS_CCH_FwUpgrade : public FIXS_CCH_StateMachine
{

  public:

    typedef enum { DEFAULT_NO_EVENT, SNMP_SET_OK, SNMP_SET_NOT_OK, SNMP_TRAP_OK, SNMP_TRAP_NOT_OK, SNMP_TRAP_PENDING_NOT_OK } FwEventId;

    typedef enum { STATE_IDLE, STATE_ONGOING, STATE_UNDEFINED } FwStateId;

    class EventQueue
    {

      public:

          EventQueue();

          virtual ~EventQueue();

          FIXS_CCH_FwUpgrade::FwEventId getFirst ();

          void popData ();

          void pushData (FwEventId info);

          bool queueEmpty ();

          size_t queueSize ();

          FIXS_CCH_FwUpgrade::FwEventId getItem (int index);

          int getHandle () const;

          void cleanQueue();

        // Additional Public Declarations


      protected:
        // Additional Protected Declarations


      private:

          EventQueue(const EventQueue &right);

          EventQueue & operator=(const EventQueue &right);

          int m_queueHandle;

          ACE_thread_mutex_t  m_queueCriticalSection;

          std::list<FwEventId> m_qEventInfo;

      private:

    };


    class FwState
    {

      public:

          FwState();

          FwState (FwStateId id);

          virtual ~FwState();

          virtual int fwUpgradeResultTrap (int result) = 0;

           FIXS_CCH_FwUpgrade::FwStateId getStateId () const;

          void setContext (FIXS_CCH_FwUpgrade* context);


          virtual void setSnmpStatus (bool status);


          virtual int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
          std::string slot, 	// contains the path and the IP address of the software version to set
          std::string tmpIPA, 	// contains the path and the IP address of the software version to set
          std::string tmpIPB	// contains the path and the IP address of the software version to set
          ) = 0;

        // Additional Public Declarations


      protected:
        // Data Members for Class Attributes
          FIXS_CCH_FwUpgrade *m_context;

          FIXS_CCH_FwUpgrade::FwStateId m_id;

           bool m_set_status;

        // Additional Protected Declarations
      private:

          FwState(const FwState &right);

          FwState & operator=(const FwState &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class StateIdle : public FwState
    {

      public:

          StateIdle();

          virtual ~StateIdle();

          virtual int fwUpgradeResultTrap (int result);

          int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
          std::string slot, 	// contains the path and the IP address of the software version to set
          std::string tmpIPA, 	// contains the path and the IP address of the software version to set
          std::string tmpIPB	// contains the path and the IP address of the software version to set
          );

        // Additional Public Declarations


      protected:
        // Additional Protected Declarations

      private:

          StateIdle(const StateIdle &right);

          StateIdle & operator=(const StateIdle &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class StateOngoing : public FwState
    {

      public:

          StateOngoing();

          virtual ~StateOngoing();

          virtual int fwUpgradeResultTrap (int result);

          int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
          std::string slot, 	// contains the path and the IP address of the software version to set
          std::string tmpIPA, 	// contains the path and the IP address of the software version to set
          std::string tmpIPB	// contains the path and the IP address of the software version to set
          );

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:

          StateOngoing(const StateOngoing &right);

          StateOngoing & operator=(const StateOngoing &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class StateUndefined : public FwState
    {

      public:

          StateUndefined();

          virtual ~StateUndefined();

          virtual int fwUpgradeResultTrap (int result);

          int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
          std::string slot, 	// contains the path and the IP address of the software version to set
          std::string tmpIPA, 	// contains the path and the IP address of the software version to set
          std::string tmpIPB	// contains the path and the IP address of the software version to set
          );

        // Additional Public Declarations


      protected:
        // Additional Protected Declarations


      private:

          StateUndefined(const StateUndefined &right);

          StateUndefined & operator=(const StateUndefined &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations


    };

    class SaxHandler : virtual public HandlerBase
    {

      public:

          SaxHandler (const std::string &xmlFilePath, CCH_Util::fwRemoteData &fwData);

          virtual ~SaxHandler();

          //	Method to handle the start events that are generated for each element when
          //	an XML file is parsed
          void startElement (const XMLCh* const  name, AttributeList &attributes);

          //	Metod to handle errors that occured during parsing
          void warning (const SAXParseException &exception);


          //	Metod to handle errors that occured during parsing
          void error (const SAXParseException &exception);


          //	Metod to handle errors that occured during parsing
          void fatalError (const SAXParseException &exception);


          //	Method to handle the start events that are generated for each element when
          //	an XML file is parsed
          void characters (const XMLCh* const  chars, const XMLSize_t length);

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:

          SaxHandler(const SaxHandler &right);

          SaxHandler & operator=(const SaxHandler &right);

        // Data Members for Class Attributes

           CCH_Util::fwRemoteData &m_fwData;
           bool m_name;
           bool m_cxc;
           bool m_rState;
           bool m_cxp;
           bool m_cxp_rState;

           ACS_TRA_trace *traceSax;
        // Additional Private Declarations
      private:
        // Additional Implementation Declarations

    };

      FIXS_CCH_FwUpgrade();

      FIXS_CCH_FwUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string IP_EthA, std::string IP_EthB, std::string dn);

      virtual ~FIXS_CCH_FwUpgrade();

      int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
      std::string slot, 	// contains the path and the IP address of the software version to set
      std::string tmpIPA, std::string tmpIPB);

      int fwUpgradeResultTrap (int result);

      std::string getStateName ();

      void switchState (FwEventId eventId);

      virtual void setSnmpStatus (bool status);

      virtual int svc(void);

      int open(void *args = 0);

      void stop ();

//      virtual DWORD run ();
//
//      virtual void stop ();

      bool startTrapEventTimer ();

      bool cancelTrapEventTimer ();

      void setEvent (FwEventId eventId);

      int checkFWStatus (std::string ip_EthA, std::string ip_EthB, std::string slot,int env_switch);

      virtual void restoreStatus (CCH_Util::FwData &fwdata);

      virtual void saveStatus ();


      void turnOffStateMachine ();


      void turnOnStateMachine (int iSlot);


      bool startTrapEventTimerFiveMin ();


      bool cancelTrapEventTimerFiveMin ();


      void setDateandTime ();

      bool isRunningThread();

      bool getCPAuthorization ();

      void  stopJtpThread();	

      int getActiveFwArea (std::string IP_EthA_str, std::string IP_EthB_str, bool &isAreaA);
      int fwUpgradeWhenAreaInactive(std::string prodNo, std::string argMagazine, std::string argSlot);
      int activeAreaSwitch (unsigned int slot,int value);
      bool waitTillFwmActionResultFetched(FIXS_CCH_SNMPManager *handler, bool &stopRetry, bool isCmx4, int &actionStatus);
    // Data Members for Class Attributes

       int m_slotUpgOnGoing;

       std::string m_ipMaster;

       std::string m_slotMaster;
    // Data Members for Associations


  public:
      // Additional Public Declarations

      FIXS_CCH_TrapTimer *m_trapTimer;

      FIXS_CCH_TrapTimer *m_trapTimer_FiveM;

  protected:
    // Additional Protected Declarations
      bool m_running;

  private:

      void handleCommandEvent ();

      //	1) when the LoadReleaseCompleted trap is missing, check the loaded software
      //	on the board SCX or CMX
      //	2) when the cold start trap is missing, check the current software on the board SCX or CMX
      void handleTenMinutesTrapTimeout ();

      bool initializeParser (std::string &filenameXml, CCH_Util::fwRemoteData &fwData);

      bool parseJsonFile(std::string &metadataFilename, CCH_Util::fwRemoteData &fwData);

      bool readFwFromJson(boost::property_tree::ptree const& pt, CCH_Util::fwRemoteData &fwData);

      void sequenceFWType(CCH_Util::fwRemoteData &info);

      void initOnStart ();

      void resetEvents ();

    // Data Members for Class Attributes

       short unsigned m_boardId;

       long unsigned m_magazine;


      //	This handle is enabled when a command is received.

       EventHandle m_cmdEvent;

      //	This handle is enabled when the stop is signalled

       EventHandle m_shutdownEvent;

       EventQueue m_eventQueue;

       ACE_thread_mutex_t *m_snmpSyncPtr;

       std::string m_IP_EthA;

       std::string m_IP_EthB;

       //	This handle is enabled when the 5 minutes TRAP timeout is signalled
       int m_timeOutTrapEvent;

       int m_timeOutTrapEventFiveMin;

       short unsigned m_slot;

       std::string m_otherBlade_dn;

       std::string m_percistance_dn;


      //	Defining this critical section as a pointer helps to understand if
      //	everything went ok during the initialization.


       FIXS_CCH_JTPHandler *m_jtpHandlerFw[2];

       std::string m_lastUpgradeDate;

       std::string m_lastUpgradeTime;

       std::string m_lastFWUpgradeResult;

       std::string m_lastFWUpgradeReason;

    // Data Members for Associations

      StateIdle m_idle;

      StateUndefined m_undefined;

      StateOngoing m_ongoing;

      FwState *m_currentState;

      EventHandle m_StopEvent;

      std::string eventNameStop;

      ACS_TRA_trace *traceObj;

      ACS_TRA_Logging *FIXS_CCH_logging;

    // Additional Private Declarations


  private:
    // Additional Implementation Declarations

};


// Class FIXS_CCH_FwUpgrade::EventQueue



inline  int FIXS_CCH_FwUpgrade::EventQueue::getHandle () const
{
	return m_queueHandle;
}

inline  bool FIXS_CCH_FwUpgrade::isRunningThread ()
{
	return m_running;
}


#endif /* FIXS_CCH_FWUPGRADE_H_ */
