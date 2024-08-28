#ifndef FIXS_CCH_ScxUpgrade_h
#define FIXS_CCH_ScxUpgrade_h 1

#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <list>
#include <string>
#include <sys/eventfd.h>

#include "ACS_APGCC_Util.H"
#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"
#include "acs_apgcc_omhandler.h"

#include "ACS_JTP.h"
#include "FIXS_CCH_JTPHandler.h"

#include "ACS_APGCC_OiHandler.h"
#include "ACS_CC_Types.h"
#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include "ace/Task.h"

#include "FIXS_CCH_SoftwareMap.h"
#include "FIXS_CCH_FSMDataStorage.h"
#include "Client_Server_Interface.h"
#include "FIXS_CCH_UpgradeManager.h"

#include "FIXS_CCH_Event.h"
//#include "FIXS_CCH_Util.h"
//#include "FIXS_CCH_IMM_Util.h"
#include "FIXS_CCH_ImmInterface.h"
#include "FIXS_CCH_SNMPManager.h"
#include "FIXS_CCH_StateMachine.h"
#include "FIXS_CCH_TrapTimer.h"


class MyImplementer;
class FIXS_CCH_JTPHandler;
class FIXS_CCH_SNMPManager;


// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

namespace{
	typedef int EventHandle;
}
class FIXS_CCH_ScxUpgrade : public FIXS_CCH_StateMachine
{
  private:
	FIXS_CCH_TrapTimer *trapTimer;
  public:

    typedef enum{
    STATE_IDLE,
    STATE_PREPARING,
    STATE_PREPARED,
    STATE_ACTIVATING,
    STATE_ACTIVATED,
    STATE_MANUAL_ROLLBACK_ONGOING,
    STATE_MANUAL_ROLLBACK_ENDED,
    STATE_AUTOMATIC_ROLLBACK_ONGOING,
    STATE_AUTOMATIC_ROLLBACK_ENDED,
    STATE_FAILED,
    STATE_UNDEFINED

    } ScxStateId;

    class ScxState 
    {
      public:
          ScxState();

          ScxState (ScxStateId stateId);

          virtual ~ScxState();

          void setContext (FIXS_CCH_ScxUpgrade* context);

           FIXS_CCH_ScxUpgrade::ScxStateId getStateId () const;

          virtual int commit ();

          virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade);

          virtual int cancel ();

          virtual int loadReleaseCompleted (int loadResult);

          virtual int coldStart ();

          virtual int activateSW ();

        // Data Members for Class Attributes

          bool m_set_status;

        // Additional Public Declarations


      protected:
        // Data Members for Class Attributes

          FIXS_CCH_ScxUpgrade::ScxStateId m_id;

          FIXS_CCH_ScxUpgrade* m_context;

        // Additional Protected Declarations

      private:

          ScxState(const ScxState &right);

          ScxState & operator=(const ScxState &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations
    };

    typedef enum 
    { 
    		SCX_PREPARE_OK,	        //0
    		SCX_WARMRESTART_NEW_COMPLETED, //1
    		SCX_COMMIT_OK,			//2
    		SCX_ROLLBACK_OK,		//3
    		SCX_PREPARE_RECEIVED,	//4
    		SCX_ACTIVATE_RECEIVED,	//5
    		SCX_COMMIT_RECEIVED,	//6
    		SCX_CANCEL_RECEIVED,	//7
    		SCX_FAILURE_RECEIVED,	//8			
    		SCX_DEFAULT_NO_EVENT,	//9
    		SCX_WARMRESTART_OLD_COMPLETED,  //10
            SCX_COMMIT_RECEIVED_FAILED  //11
    } ScxEventId;


    class State_Activated : public ScxState
    {
      public:

          State_Activated();

          virtual ~State_Activated();

          virtual int commit ();

          virtual int cancel ();

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:

          State_Activated(const State_Activated &right);

          State_Activated & operator=(const State_Activated &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations
    };

    class State_Activating : public ScxState
    {
      public:

          State_Activating();

          virtual ~State_Activating();

          virtual int coldStart ();

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:

          State_Activating(const State_Activating &right);

          State_Activating & operator=(const State_Activating &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class State_Automatic_Rollback_Ended : public ScxState
    {
     public:

          State_Automatic_Rollback_Ended();

          virtual ~State_Automatic_Rollback_Ended();

          virtual int commit ();

        // Additional Public Declarations


      protected:
        // Additional Protected Declarations

      private:

          State_Automatic_Rollback_Ended(const State_Automatic_Rollback_Ended &right);

          State_Automatic_Rollback_Ended & operator=(const State_Automatic_Rollback_Ended &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class State_Manual_Rollback_Ended : public ScxState
    {
      public:

          State_Manual_Rollback_Ended();

          virtual ~State_Manual_Rollback_Ended();

          virtual int commit ();

        // Additional Public Declarations

     protected:
        // Additional Protected Declarations

      private:

          State_Manual_Rollback_Ended(const State_Manual_Rollback_Ended &right);

          State_Manual_Rollback_Ended & operator=(const State_Manual_Rollback_Ended &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class State_Idle : public ScxState
    {

      public:
          State_Idle();

          virtual ~State_Idle();

          virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade);

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations


      private:

          State_Idle(const State_Idle &right);

          State_Idle & operator=(const State_Idle &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations
    };


    class State_Preparing : public ScxState
    {

      public:

          State_Preparing();

          virtual ~State_Preparing();

          virtual int cancel ();

          virtual int loadReleaseCompleted (int loadResult);

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:

          State_Preparing(const State_Preparing &right);


          State_Preparing & operator=(const State_Preparing &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };



    class State_Prepared : public ScxState
    {

      public:

          State_Prepared();

          virtual ~State_Prepared();

          virtual int cancel ();


          virtual int activateSW();

        // Additional Public Declarations


      protected:
        // Additional Protected Declarations


      private:

          State_Prepared(const State_Prepared &right);

          State_Prepared & operator=(const State_Prepared &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };


    class State_Manual_Rollback_Ongoing : public ScxState
    {

      public:

          State_Manual_Rollback_Ongoing();

          virtual ~State_Manual_Rollback_Ongoing();

          virtual int coldStart ();

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:

          State_Manual_Rollback_Ongoing(const State_Manual_Rollback_Ongoing &right);

          State_Manual_Rollback_Ongoing & operator=(const State_Manual_Rollback_Ongoing &right);

        // Additional Private Declarations


      private:
        // Additional Implementation Declarations

    };


    class State_Failed : public ScxState
    {

      public:

          State_Failed();

          virtual ~State_Failed();

          virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade);

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:

          State_Failed(const State_Failed &right);

          State_Failed & operator=(const State_Failed &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };


    class EventQueue 
    {

	 public:

          EventQueue();

          virtual ~EventQueue();

           FIXS_CCH_ScxUpgrade::ScxEventId getFirst ();

          void popData ();

          void pushData (ScxEventId info);

          bool queueEmpty ();

          size_t queueSize ();

           FIXS_CCH_ScxUpgrade::ScxEventId getItem (int index);

          int getHandle () const;

          void cleanQueue();

        // Additional Public Declarations


      protected:
        // Additional Protected Declarations

      private:

          EventQueue(const EventQueue &right);

          EventQueue & operator=(const EventQueue &right);

        // Data Members for Class Attributes


          //	Handle to wake up the thread that is waiting for events on this queue

          int m_queueHandle;

          ACE_thread_mutex_t  m_queueCriticalSection;


          std::list<ScxEventId> m_qEventInfo;

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

      FIXS_CCH_ScxUpgrade();

      FIXS_CCH_ScxUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string scx_IP_EthA, std::string scx_IP_EthB, std::string dn);


      virtual ~FIXS_CCH_ScxUpgrade();

      void setObjectDN(const char* dn);

      void setSwInvObjectDN(std::string dn);

      std::string getSwInvObjectDN();

      std::string getSwPercistanceDN();

      int activateSW ();

      int commit ();

      int prepare (std::string packageToUpgrade, std::string productToUpgrade);

      int setEvent (ScxEventId event);

      FIXS_CCH_JTPHandler *m_jtpHandler[2];
//      virtual DWORD run ();

  	  virtual int svc(void);

  	  virtual int open (void *args = 0);

      virtual void stop ();

      void switchState (ScxEventId eventId);

      int cancel ();

      std::string getStateName ();

      int getStateImm ();

      //	Set the expected Load Module package (cxp) on prepare
      void setExpectedCXP (std::string cxp = "");

      std::string getExpectedCXP () const;

      void setCurrentCXP (std::string cxp);

      std::string getCurrentCXP () const;

      bool isFullUpgrade (/*std::string cxp = ""*/);

      void setFullUpgradeType ();

      void setPartialUpgradeType ();

      virtual bool configureDHCP (bool rollback);

      std::string getCurrentCXC () const;

      void setCurrentCXC (std::string cxc);

      std::string getExpectedCXC () const;

      //	Set the expected Load Module package (cxc) on prepare
      void setExpectedCXC (std::string cxc = "");


      std::string getLastUpgradeDate () const;


      std::string getLastUpgradeTime () const;


      void setLastUpgradeResult (int result);


      void setLastUpgradeReason (int reason);


      int getLastUpgradeResult () const;


      int getLastUpgradeReason () const;


      bool preliminaryCheckOnPrepare (std::string softwareVersion, std::string containerId);


      FIXS_CCH_ScxUpgrade::ScxStateId getStateId () const;


      //	Set the Snmp OID to force SCX to load the new software
      virtual bool loadRelease ();


      std::string  getScxFileName(std::string package);

      int loadReleaseCompleted (int loadResult	// 0 for Not OK, 1 for OK
      );

      virtual bool checkLoadedVersion ();

      //	Check through SNMP the loaded Software version.
      virtual bool checkCurrentLoadedVersion ();

      int coldStart ();

      virtual void maintenanceWindow (unsigned char msgType);

      //	Read the Snmp OID about the action status: SWMActionStatus
      virtual bool readActionStatus ();

      virtual void setLastErrorCode (int code);

      //	Set the Snmp OIDs:
      //	SWMActionVersion with the software version to be used
      //	SWMAction with the start release value
      virtual bool startRelease ();

      //	Set the Snmp OID to make the current version as default version
      virtual bool makeDefault ();

      //	Set the Snmp OID to force SCX to cold restart loading using the default
      //	software.
      virtual void resetSwitch ();

      virtual void saveStatus ();

      virtual void getOverallStatus (CCH_Util::SWData &swdata);

      bool initSwInventory();

      virtual void restoreStatus (CCH_Util::SWData &swdata);

      std::string getCurrentRunningSoftware ();

      //	Read the Snmp OID about the action status: SWMActionStatus
      virtual bool readResultStatus ();


      virtual bool checkRollBackVersion ();


      virtual bool checkCurrentDefaultVersion ();


      //	Set the Snmp OID to make the current version as default version
      virtual bool restoreMakeDefault ();

      bool isRunningThread ();

      bool sendStartRequestToCP (bool & shutdownsignaled);

      void WaitJtpNotificationRetriesToFinish();


    // Data Members for Class Attributes

      std::string m_neighborSCXIPA;

      std::string m_neighborSCXIPB;

    // Data Members for Associations


  public:
    // Additional Public Declarations


  protected:
    // Data Members for Class Attributes

      bool m_exit;

      //ACE_thread_mutex_t m_criticalSection;

      bool m_running;

      int m_lastErrorCode;

      std::string m_APNode;

    // Additional Protected Declarations



  private:

      FIXS_CCH_ScxUpgrade(const FIXS_CCH_ScxUpgrade &right);

      FIXS_CCH_ScxUpgrade & operator=(const FIXS_CCH_ScxUpgrade &right);

      void handleCommandEvent ();

      //	//TO DO
      //	1) when the LoadReleaseCompleted trap is missing, check the loaded software
      //	on SCX
      //	2) when the cold start trap is missing, check the current software on SCX
      void handleTrapTimeout ();


      bool cancelTrapTimer ();

      bool modifySwInventory(int state);


      bool startTrapTimer (int seconds);


      //	This method takes care of all initialization of the state machine.
      void initOnStart ();


      //	Takes proper actions when CCH restarts
      void onRestart ();

      void resetEvents();

    // Data Members for Class Attributes

      int m_upgradeStatus;

      bool m_fullUpradeType;

      EventQueue m_eventQueue;

      unsigned short m_slot;

      unsigned long m_magazine;

      unsigned short m_boardId;

      std::string m_scx_IP_EthA;

      std::string m_scx_IP_EthB;

      std::string m_currentCxp;

      std::string m_expectedCxp;

      std::string m_Object_DN;

      int m_active_commit;

      //	This handle is enabled when a command is received.

      EventHandle m_cmdEvent;

      //	This handle is enabled when the stop is signalled

      EventHandle m_shutdownEvent;

      //	This handle is enabled when the 5 minutes TRAP timeout is signalled

      EventHandle m_timeoutTrapEvent;

      std::string m_currentCxc;

      std::string m_expectedCxc;

      std::string m_lastUpgradeDate;

      std::string m_lastUpgradeTime;

      CCH_Util::RpUpgradeResult m_lastSwUpgradeResult;

      CCH_Util::RpUpgradeResultInfo m_lastSwUpgradeReason;

      //	Defining this critical section as a pointer helps to understand if
      //	everything went ok during the initialization.

      ACE_thread_mutex_t m_snmpSyncPtr;

      ACS_TRA_trace *traceObj;
     // FIXS_CCH_JTPHandler *   m_jtpHandler[2];

    // Data Members for Associations

      ScxState *m_currentState;

      //const char* m_SwInventory_Object_DN;
      std::string m_SwInventory_Object_DN;

      State_Idle m_idle;

      State_Preparing m_preparing;

      State_Prepared m_prepared;

      State_Activating m_activating;

      State_Activated m_activated;

      State_Failed m_failed;

      State_Automatic_Rollback_Ended m_automaticRollbackEnded;

      State_Manual_Rollback_Ended m_manualRollbackEnded;

      State_Manual_Rollback_Ongoing m_manualRollbackOngoing;

//      EventHandle m_StopEvent;

      ACS_TRA_Logging *FIXS_CCH_logging;

      std::string m_previousCxp;

      std::string dn_blade_persistance;

      std::string m_scx_IP;

    // Additional Private Declarations


  private:
    // Additional Implementation Declarations
};




// Class FIXS_CCH_ScxUpgrade::ScxState 

// Class FIXS_CCH_ScxUpgrade::State_Activated 

// Class FIXS_CCH_ScxUpgrade::State_Activating 

// Class FIXS_CCH_ScxUpgrade::State_Automatic_Rollback_Ended 

// Class FIXS_CCH_ScxUpgrade::State_Manual_Rollback_Ended 

// Class FIXS_CCH_ScxUpgrade::State_Idle 

// Class FIXS_CCH_ScxUpgrade::State_Preparing 

// Class FIXS_CCH_ScxUpgrade::State_Prepared 

// Class FIXS_CCH_ScxUpgrade::State_Manual_Rollback_Ongoing 

// Class FIXS_CCH_ScxUpgrade::State_Failed 

// Class FIXS_CCH_ScxUpgrade::EventQueue 



inline  int FIXS_CCH_ScxUpgrade::EventQueue::getHandle () const
{
 	return m_queueHandle;
}

// Class FIXS_CCH_ScxUpgrade 

inline  void FIXS_CCH_ScxUpgrade::setExpectedCXP (std::string cxp)
{

	m_expectedCxp = cxp;

}

inline  std::string FIXS_CCH_ScxUpgrade::getExpectedCXP () const
{

	return m_expectedCxp;

}

inline  void FIXS_CCH_ScxUpgrade::setCurrentCXP (std::string cxp)
{

	m_currentCxp = cxp;

}

inline  std::string FIXS_CCH_ScxUpgrade::getCurrentCXP () const
{

	return m_currentCxp;

}

inline  bool FIXS_CCH_ScxUpgrade::isFullUpgrade (/*std::string cxp*/)
{

		return m_fullUpradeType;

}

inline  void FIXS_CCH_ScxUpgrade::setFullUpgradeType ()
{

	m_fullUpradeType = true;

}

inline  void FIXS_CCH_ScxUpgrade::setPartialUpgradeType ()
{

	m_fullUpradeType = false;

}

inline  std::string FIXS_CCH_ScxUpgrade::getCurrentCXC () const
{

	return m_currentCxc;

}

inline  void FIXS_CCH_ScxUpgrade::setCurrentCXC (std::string cxc)
{

	m_currentCxc = cxc;

}

inline  std::string FIXS_CCH_ScxUpgrade::getExpectedCXC () const
{

	return m_expectedCxc;

}

inline  void FIXS_CCH_ScxUpgrade::setExpectedCXC (std::string cxc)
{

	m_expectedCxc = cxc;

}

inline  std::string FIXS_CCH_ScxUpgrade::getLastUpgradeDate () const
{

	return m_lastUpgradeDate;

}

inline  std::string FIXS_CCH_ScxUpgrade::getLastUpgradeTime () const
{

	return m_lastUpgradeTime;

}

inline  void FIXS_CCH_ScxUpgrade::setLastUpgradeResult (int result)
{

	m_lastSwUpgradeResult = static_cast<CCH_Util::RpUpgradeResult>(result);

}

inline  void FIXS_CCH_ScxUpgrade::setLastUpgradeReason (int reason)
{

	m_lastSwUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(reason);

}

inline  int FIXS_CCH_ScxUpgrade::getLastUpgradeResult () const
{

	return m_lastSwUpgradeResult;

}

inline  int FIXS_CCH_ScxUpgrade::getLastUpgradeReason () const
{

	return m_lastSwUpgradeReason;

}

inline  void FIXS_CCH_ScxUpgrade::setLastErrorCode (int code)
{

	m_lastErrorCode = code;

}

inline  bool FIXS_CCH_ScxUpgrade::isRunningThread ()
{
	return m_running;
}



#endif
