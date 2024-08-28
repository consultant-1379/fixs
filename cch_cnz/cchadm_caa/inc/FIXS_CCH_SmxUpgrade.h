#ifndef FIXS_CCH_SmxUpgrade_h
#define FIXS_CCH_SmxUpgrade_h 1

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
class FIXS_CCH_SmxUpgrade : public FIXS_CCH_StateMachine
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

    } SmxStateId;

    class SmxState 
    {
      public:
          SmxState();

          SmxState (SmxStateId stateId);

          virtual ~SmxState();

          void setContext (FIXS_CCH_SmxUpgrade* context);

           FIXS_CCH_SmxUpgrade::SmxStateId getStateId () const;

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

          FIXS_CCH_SmxUpgrade::SmxStateId m_id;

          FIXS_CCH_SmxUpgrade* m_context;

        // Additional Protected Declarations

      private:

          SmxState(const SmxState &right);

          SmxState & operator=(const SmxState &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations
    };

    typedef enum 
    { 
    		SMX_PREPARE_OK,	        //0
    		SMX_WARMRESTART_NEW_COMPLETED, //1
    		SMX_COMMIT_OK,			//2
    		SMX_ROLLBACK_OK,		//3
    		SMX_PREPARE_RECEIVED,	//4
    		SMX_ACTIVATE_RECEIVED,	//5
    		SMX_COMMIT_RECEIVED,	//6
    		SMX_CANCEL_RECEIVED,	//7
    		SMX_FAILURE_RECEIVED,	//8			
    		SMX_DEFAULT_NO_EVENT,	//9
    		SMX_WARMRESTART_OLD_COMPLETED,  //10
            SMX_COMMIT_RECEIVED_FAILED  //11
    } SmxEventId;


    class State_Activated : public SmxState
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

    class State_Activating : public SmxState
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

    class State_Automatic_Rollback_Ended : public SmxState
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

    class State_Manual_Rollback_Ended : public SmxState
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

    class State_Idle : public SmxState
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


    class State_Preparing : public SmxState
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



    class State_Prepared : public SmxState
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


    class State_Manual_Rollback_Ongoing : public SmxState
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


    class State_Failed : public SmxState
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

           FIXS_CCH_SmxUpgrade::SmxEventId getFirst ();

          void popData ();

          void pushData (SmxEventId info);

          bool queueEmpty ();

          size_t queueSize ();

           FIXS_CCH_SmxUpgrade::SmxEventId getItem (int index);

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


          std::list<SmxEventId> m_qEventInfo;

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

      FIXS_CCH_SmxUpgrade();

      FIXS_CCH_SmxUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string Smx_IP_EthA, std::string Smx_IP_EthB, std::string dn);


      virtual ~FIXS_CCH_SmxUpgrade();

      void setObjectDN(const char* dn);

      void setSwInvObjectDN(std::string dn);

      std::string getSwInvObjectDN();

      std::string getSwPercistanceDN();

      int activateSW ();

      int commit ();

      int prepare (std::string packageToUpgrade, std::string productToUpgrade);

      int setEvent (SmxEventId event);

      FIXS_CCH_JTPHandler *m_jtpHandler[2];
//      virtual DWORD run ();

  	  virtual int svc(void);

  	  virtual int open (void *args = 0);

      virtual void stop ();

      void switchState (SmxEventId eventId);

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


      FIXS_CCH_SmxUpgrade::SmxStateId getStateId () const;


      //	Set the Snmp OID to force Smx to load the new software
      virtual bool loadRelease ();


      std::string  getSmxFileName(std::string package);

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

      //	Set the Snmp OID to force Smx to cold restart loading using the default
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

      std::string m_neighborSmxIPA;

      std::string m_neighborSmxIPB;

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

      FIXS_CCH_SmxUpgrade(const FIXS_CCH_SmxUpgrade &right);

      FIXS_CCH_SmxUpgrade & operator=(const FIXS_CCH_SmxUpgrade &right);

      void handleCommandEvent ();

      //	//TO DO
      //	1) when the LoadReleaseCompleted trap is missing, check the loaded software
      //	on Smx
      //	2) when the cold start trap is missing, check the current software on Smx
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

      std::string m_Smx_IP_EthA;

      std::string m_Smx_IP_EthB;

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

      SmxState *m_currentState;

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

      std::string m_previouSmxp;

      std::string dn_blade_persistance;

      std::string m_Smx_IP;

    // Additional Private Declarations


  private:
    // Additional Implementation Declarations
};




// Class FIXS_CCH_SmxUpgrade::SmxState 

// Class FIXS_CCH_SmxUpgrade::State_Activated 

// Class FIXS_CCH_SmxUpgrade::State_Activating 

// Class FIXS_CCH_SmxUpgrade::State_Automatic_Rollback_Ended 

// Class FIXS_CCH_SmxUpgrade::State_Manual_Rollback_Ended 

// Class FIXS_CCH_SmxUpgrade::State_Idle 

// Class FIXS_CCH_SmxUpgrade::State_Preparing 

// Class FIXS_CCH_SmxUpgrade::State_Prepared 

// Class FIXS_CCH_SmxUpgrade::State_Manual_Rollback_Ongoing 

// Class FIXS_CCH_SmxUpgrade::State_Failed 

// Class FIXS_CCH_SmxUpgrade::EventQueue 



inline  int FIXS_CCH_SmxUpgrade::EventQueue::getHandle () const
{
 	return m_queueHandle;
}

// Class FIXS_CCH_SmxUpgrade 

inline  void FIXS_CCH_SmxUpgrade::setExpectedCXP (std::string cxp)
{

	m_expectedCxp = cxp;

}

inline  std::string FIXS_CCH_SmxUpgrade::getExpectedCXP () const
{

	return m_expectedCxp;

}

inline  void FIXS_CCH_SmxUpgrade::setCurrentCXP (std::string cxp)
{

	m_currentCxp = cxp;

}

inline  std::string FIXS_CCH_SmxUpgrade::getCurrentCXP () const
{

	return m_currentCxp;

}

inline  bool FIXS_CCH_SmxUpgrade::isFullUpgrade (/*std::string cxp*/)
{

		return m_fullUpradeType;

}

inline  void FIXS_CCH_SmxUpgrade::setFullUpgradeType ()
{

	m_fullUpradeType = true;

}

inline  void FIXS_CCH_SmxUpgrade::setPartialUpgradeType ()
{

	m_fullUpradeType = false;

}

inline  std::string FIXS_CCH_SmxUpgrade::getCurrentCXC () const
{

	return m_currentCxc;

}

inline  void FIXS_CCH_SmxUpgrade::setCurrentCXC (std::string cxc)
{

	m_currentCxc = cxc;

}

inline  std::string FIXS_CCH_SmxUpgrade::getExpectedCXC () const
{

	return m_expectedCxc;

}

inline  void FIXS_CCH_SmxUpgrade::setExpectedCXC (std::string cxc)
{

	m_expectedCxc = cxc;

}

inline  std::string FIXS_CCH_SmxUpgrade::getLastUpgradeDate () const
{

	return m_lastUpgradeDate;

}

inline  std::string FIXS_CCH_SmxUpgrade::getLastUpgradeTime () const
{

	return m_lastUpgradeTime;

}

inline  void FIXS_CCH_SmxUpgrade::setLastUpgradeResult (int result)
{

	m_lastSwUpgradeResult = static_cast<CCH_Util::RpUpgradeResult>(result);

}

inline  void FIXS_CCH_SmxUpgrade::setLastUpgradeReason (int reason)
{

	m_lastSwUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(reason);

}

inline  int FIXS_CCH_SmxUpgrade::getLastUpgradeResult () const
{

	return m_lastSwUpgradeResult;

}

inline  int FIXS_CCH_SmxUpgrade::getLastUpgradeReason () const
{

	return m_lastSwUpgradeReason;

}

inline  void FIXS_CCH_SmxUpgrade::setLastErrorCode (int code)
{

	m_lastErrorCode = code;

}

inline  bool FIXS_CCH_SmxUpgrade::isRunningThread ()
{
	return m_running;
}



#endif
