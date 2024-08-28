#ifndef FIXS_CCH_IptbUpgrade_h
#define FIXS_CCH_IptbUpgrade_h 1
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
#include "FIXS_CCH_ImmInterface.h"
#include "FIXS_CCH_SNMPManager.h"
#include "FIXS_CCH_StateMachine.h"
#include "FIXS_CCH_TrapTimer.h"

class MyImplementer;
class FIXS_CCH_SNMPManager;

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

namespace{
	typedef int EventHandle;
}
class FIXS_CCH_IptbUpgrade : public FIXS_CCH_StateMachine
{
  private:

  public:

    typedef enum
    {
       STATE_IDLE = 0,
       STATE_PREPARED = 2,
       STATE_ACTIVATING = 3,
       STATE_ACTIVATED = 4,
       STATE_AUTOMATIC_ROLLBACK_ONGOING = 7,
       STATE_AUTOMATIC_ROLLBACK_ENDED = 8,
       STATE_MANUAL_ROLLBACK_ONGOING = 5, //cancelling
       STATE_MANUAL_ROLLBACK_ENDED = 6, //cancelled
       STATE_FAILED = 10,
       STATE_UNDEFINED = 9
    }
     IptbStateId;
       class IptbState
     {
     public:
    	 IptbState();

    	 IptbState (IptbStateId stateId);

    	 virtual ~IptbState();


    	 void setContext (FIXS_CCH_IptbUpgrade* context);

    	 FIXS_CCH_IptbUpgrade::IptbStateId getStateId () const;

    	 virtual int activateSw ();

    	 virtual int commit (bool setAsDefaultPackage);

    	 //virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation = true);
	 
	 virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, int upgradeType = 0);

    	 virtual int cancel ();

    	 // Data Members for Class Attributes
    	 bool m_set_status;
    	 // Additional Public Declarations
     protected:
    	 // Data Members for Class Attributes

    	 FIXS_CCH_IptbUpgrade::IptbStateId m_id;

    	 FIXS_CCH_IptbUpgrade* m_context;

     private:

    	 IptbState(const IptbState &right);

    	 IptbState & operator=(const IptbState &right);

     private: //## implementation
    	 // Additional Implementation Declarations

     };

     class IptbStateActivated : public IptbState  //## Inherits: <unnamed>%4D2DCD8A020C
     {
    	 //## begin FIXS_CCH_IptbUpgrade::IptbStateActivated%4D2DCC9402C7.initialDeclarations preserve=yes
    	 //## end FIXS_CCH_IptbUpgrade::IptbStateActivated%4D2DCC9402C7.initialDeclarations

     public:
    	 IptbStateActivated();

    	 virtual ~IptbStateActivated();

    	 virtual int commit (bool setAsDefaultPackage);

    	 virtual int cancel ();

     protected:

     private:

    	 IptbStateActivated(const IptbStateActivated &right);

    	 IptbStateActivated & operator=(const IptbStateActivated &right);

     private: //## implementation
     };

     class IptbStateActivating : public IptbState  //## Inherits: <unnamed>%4D2DCD6B0193
     {
     public:

    	 IptbStateActivating();

    	 virtual ~IptbStateActivating();

     protected:
     private:

    	 IptbStateActivating(const IptbStateActivating &right);


    	 IptbStateActivating & operator=(const IptbStateActivating &right);


     private: //## implementation

     };

     class IptbStateFailed : public IptbState  //## Inherits: <unnamed>%4D2DCD8F00C3
     {
     public:

    	 IptbStateFailed();

    	 virtual ~IptbStateFailed();

    	 //virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation = true);
	
	virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, int upgradeType = 0);   
     protected:

     private:

    	 IptbStateFailed(const IptbStateFailed &right);

    	 IptbStateFailed & operator=(const IptbStateFailed &right);

     private: //## implementation
     };

     class IptbStateIdle : public IptbState  //## Inherits: <unnamed>%4D2DCC2B01DB
     {
     public:

    	 IptbStateIdle();

    	 virtual ~IptbStateIdle();


    	 //virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation = true);
	
	 virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, int upgradeType = 0);
     protected:

     private:

    	 IptbStateIdle(const IptbStateIdle &right);

    	 IptbStateIdle & operator=(const IptbStateIdle &right);

     private: //## implementation
     };

     class IptbStatePrepared : public IptbState  //## Inherits: <unnamed>%4D2DCD530251
     {
     public:

    	 IptbStatePrepared();

    	 virtual ~IptbStatePrepared();
    	 virtual int activateSw ();

    	 virtual int cancel ();

     protected:

     private:
    	 IptbStatePrepared(const IptbStatePrepared &right);

    	 IptbStatePrepared & operator=(const IptbStatePrepared &right);

     private: //## implementation
     };

     typedef enum
     {
    	 IPT_PREPARE_OK, //0
    	 IPT_ACTIVATE_OK,
    	 IPT_COMMIT_OK,
    	 IPT_MANUAL_ROLLBACK_OK,
    	 IPT_AUTOMATIC_ROLLBACK_OK,
    	 IPT_ACTIVATE_RECEIVED,
    	 IPT_COMMIT_RECEIVED,
    	 IPT_CANCEL_RECEIVED,
    	 IPT_FAILURE_RECEIVED,
    	 IPT_DEFAULT_NO_EVENT,
    	 IPT_RESET_FAILURE_RECEIVED
     } IptbEventId;

     class EventQueue
     {
		 public:
			 EventQueue();

			 virtual ~EventQueue();

			 FIXS_CCH_IptbUpgrade::IptbEventId  getFirst ();

			 void popData ();

			 void pushData (IptbEventId info);

			 bool queueEmpty ();

			 size_t queueSize ();

			 FIXS_CCH_IptbUpgrade::IptbEventId getItem (int index);

			 int getHandle () const;

			 void cleanQueue();
		 protected:

		 private:
			 EventQueue(const EventQueue &right);

			 EventQueue & operator=(const EventQueue &right);

			 // Data Members for Class Attributes

			 int m_queueHandle;

			 ACE_thread_mutex_t m_queueCriticalSection;

			 std::list<IptbEventId> m_qEventInfo;

		 private:
     };


     class IptbAutomaticRollbackEnded : public IptbState  //## Inherits: <unnamed>%4D2DCD96021A
     {
		 public:

			 IptbAutomaticRollbackEnded();

			 virtual ~IptbAutomaticRollbackEnded();

			 virtual int commit (bool setAsDefaultPackage);

		 protected:

		 private:

			 IptbAutomaticRollbackEnded(const IptbAutomaticRollbackEnded &right);

			 IptbAutomaticRollbackEnded & operator=(const IptbAutomaticRollbackEnded &right);

		 private:
     };

     class IptbAutomaticRollbackOngoing : public IptbState
     {

		 public:
			 IptbAutomaticRollbackOngoing();

			 virtual ~IptbAutomaticRollbackOngoing();

		 protected:

		 private:

			 IptbAutomaticRollbackOngoing(const IptbAutomaticRollbackOngoing &right);

			 IptbAutomaticRollbackOngoing & operator=(const IptbAutomaticRollbackOngoing &right);

		 private:

     };

     class IptbManualRollbackEnded : public IptbState
     {
		 public:
			 IptbManualRollbackEnded();

			 virtual ~IptbManualRollbackEnded();

			 virtual int commit (bool setAsDefaultPackage);

		 protected:

		 private:
			 IptbManualRollbackEnded(const IptbManualRollbackEnded &right);

			 IptbManualRollbackEnded & operator=(const IptbManualRollbackEnded &right);

		 private:

     };

     class IptbManualRollbackOngoing : public IptbState  //## Inherits: <unnamed>%4D2DCD9B01FA
     {
		 public:
			 IptbManualRollbackOngoing();

			 virtual ~IptbManualRollbackOngoing();

		 protected:

		 private:

			 IptbManualRollbackOngoing(const IptbManualRollbackOngoing &right);

			 IptbManualRollbackOngoing & operator=(const IptbManualRollbackOngoing &right);

		 private:
     };

      FIXS_CCH_IptbUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string switch0_IP_EthA, std::string switch0_IP_EthB, std::string switch25_IP_EthA, std::string switch25_IP_EthB, std::string iptAddress_EthA, std::string iptAddress_EthB,std::string dn);

      virtual ~FIXS_CCH_IptbUpgrade();
      void setObjectDN(const char* dn);

      void setSwInvObjectDN(std::string dn);

      std::string getSwInvObjectDN();

      std::string getSwPercistanceDN();

      int activateSw ();

      int commit (bool setAsDefaultPackage);

      /*int prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation	// partial == normal
      );*/
      
      int prepare (std::string packageToUpgrade, std::string productToUpgrade, int upgradeType = 0);

      int setEvent (IptbEventId event);

      virtual int svc(void);

      virtual int open (void *args = 0);

      virtual void stop ();

      virtual void setLastErrorCode (int code);

      void switchPriSecIp ();

      void switchState (IptbEventId eventId);

      void saveStatus ();

      int cancel ();

      std::string getStateName ();

      int getStateImm ();

      void setPath (std::string completePathIptb);

      //	Set the expected Load Module package (cxp) on prepare
      void setExpectedCXP (std::string cxp = "");

      std::string getExpectedCXP () const;

      void setCurrentCXP (std::string cxp);

      std::string getCurrentCXP () const;

      void setUpgradeType (int upgradeType);
      
      int getUpgradeType ();

      FIXS_CCH_IptbUpgrade::IptbStateId getStateId () const;

      virtual bool configureDHCP (bool rollback);

      //	set xshmcBoardReset in xshmcBoardTable of SCB-RP/4
      bool resetBoard ();

      std::string  getIptFileName(std::string package);

      int onActivating ();

      std::string getCurrentCXC () const;

      void setCurrentCXC (std::string cxc);

      std::string getExpectedCXC () const;

      //	Set the expected Load Module package (cxc) on prepare
      void setExpectedCXC (std::string cxc = "");

      int onPreparing ();

      bool initSwInventory();

      void getOverallStatus (CCH_Util::BoardSWData &iptdata);

      std::string getLastUpgradeDate () const;

      std::string getLastUpgradeTime () const;

      void setLastUpgradeResult (int result);

      void setLastUpgradeReason (int reason);

      int getLastUpgradeResult () const;

      int getLastUpgradeReason () const;

      void restoreStatus (CCH_Util::BoardSWData &boardSWData);

      bool checkMasterScbrp ();

      int onAutomaticRollback ();

      int onManualRollback ();

      int checkUpgradeStatus ();

      void setResultAndReason (CCH_Util::LmErrors status);

      //	This method runs the following steps:
      //	- configure DHCP
      //	- read master SCB-RP
      //	- in case of full upgrade sets SNMP GPR VALUE to network boot
      //	- resetBoard
      int executeBasicStepsBeforeRollingBack ();

      bool isInitialized ();

      bool resetGprRegister7 ();

      bool isBoardBlocked ();

      bool isRunningThread ();

    // Data Members for Class Attributes

      unsigned long m_magazine;

      unsigned short m_slot;

      std::string m_iptAddress_EthA;

      std::string m_iptAddress_EthB;

    // Data Members for Associations

      FIXS_CCH_SNMPManager *m_snmpManager;

  public:

  protected:
      // Data Members for Class Attributes
      bool m_running;

      int m_lastErrorCode;
  private:
      FIXS_CCH_IptbUpgrade();

      FIXS_CCH_IptbUpgrade(const FIXS_CCH_IptbUpgrade &right);

      FIXS_CCH_IptbUpgrade & operator=(const FIXS_CCH_IptbUpgrade &right);

      bool modifySwInventory(int state);

      void handleCommandEvent ();

      //	This method takes care of all initialization of the state machine.
      void initOnStart ();

      //	Takes proper actions when CCH restarts
      void onRestart ();
      void resetEvents();
      int setBootType (int bootType);

    // Data Members for Class Attributes

      unsigned short m_boardId;

      //	this attribute stores the filename used to build paths like:
      //	"192.168.169.1:/filename"
      std::string m_path;

      int m_upgradeStatus;

      EventQueue m_eventQueue;

      //	This handle is enabled when a command is received.
      EventHandle m_cmdEvent;

      //	This handle is enabled when the stop is signalled
      EventHandle m_shutdownEvent;

      ACS_TRA_trace *traceObj;

      ACS_TRA_Logging *FIXS_CCH_logging;

      IptbState *m_currentState;

      std::string m_previousCxp;

      std::string m_switch0_IP_EthA;

      std::string m_switch0_IP_EthB;

      std::string m_switch25_IP_EthA;

      std::string m_switch25_IP_EthB;

      std::string m_currentCxp;

      // bool m_fullUpradeType;

      std::string m_currentCxc;

      std::string m_expectedCxp;

      std::string m_expectedCxc;

      std::string m_Object_DN;

      std::string dn_blade_persistance;

      std::string m_lastUpgradeDate;

      std::string m_lastUpgradeTime;

      CCH_Util::RpUpgradeResult m_lastSwUpgradeResult;

      CCH_Util::RpUpgradeResultInfo m_lastSwUpgradeReason;

      std::string m_switch_Master_IP;

      std::string m_SwInventory_Object_DN;

      int m_previousState;

      int m_failureCode;

      bool m_initialized;

      bool m_gprRegister7resetted;

      bool m_egem2environment;
      
      bool m_forcedUpgrade;

      int m_upgradeType;

    // Data Members for Associations

      IptbStateIdle m_idle;

      IptbStatePrepared m_prepared;

      IptbStateActivating m_activating;

      IptbStateActivated m_activated;

      IptbStateFailed m_failed;

      IptbManualRollbackOngoing m_manualRollbackOngoing;

      IptbManualRollbackEnded m_manualRollbackEnded;

      IptbAutomaticRollbackOngoing m_automaticRollbackOngoing;

      IptbAutomaticRollbackEnded m_automaticRollbackEnded;

      // Additional Private Declarations
  private:

      // Additional Implementation Declarations
};


// Class FIXS_CCH_IptbUpgrade::IptbState

// Class FIXS_CCH_IptbUpgrade::IptbStateActivated

// Class FIXS_CCH_IptbUpgrade::IptbStateActivating

// Class FIXS_CCH_IptbUpgrade::IptbStateFailed

// Class FIXS_CCH_IptbUpgrade::IptbStateIdle

// Class FIXS_CCH_IptbUpgrade::IptbStatePrepared

// Class FIXS_CCH_IptbUpgrade::EventQueue


inline  int FIXS_CCH_IptbUpgrade::EventQueue::getHandle () const
{
	return m_queueHandle;
}

// Class FIXS_CCH_IptbUpgrade::IptbAutomaticRollbackEnded

// Class FIXS_CCH_IptbUpgrade::IptbAutomaticRollbackOngoing

// Class FIXS_CCH_IptbUpgrade::IptbManualRollbackEnded

// Class FIXS_CCH_IptbUpgrade::IptbManualRollbackOngoing

// Class FIXS_CCH_IptbUpgrade


inline  void FIXS_CCH_IptbUpgrade::setExpectedCXP (std::string cxp)
{
	m_expectedCxp = cxp;
}

inline  std::string FIXS_CCH_IptbUpgrade::getExpectedCXP () const
{
	return m_expectedCxp;
}

inline  void FIXS_CCH_IptbUpgrade::setCurrentCXP (std::string cxp)
{
	m_currentCxp = cxp;
}

inline  std::string FIXS_CCH_IptbUpgrade::getCurrentCXP () const
{
	return m_currentCxp;
}

inline  int FIXS_CCH_IptbUpgrade::getUpgradeType ()
{
	return m_upgradeType;
}

inline  void FIXS_CCH_IptbUpgrade::setUpgradeType (int upgradeType)
{
	m_upgradeType = upgradeType;
}

inline  std::string FIXS_CCH_IptbUpgrade::getCurrentCXC () const
{
	return m_currentCxc;
}

inline  void FIXS_CCH_IptbUpgrade::setCurrentCXC (std::string cxc)
{
	m_currentCxc = cxc;
}

inline  std::string FIXS_CCH_IptbUpgrade::getExpectedCXC () const
{
	return m_expectedCxc;
}

inline  void FIXS_CCH_IptbUpgrade::setExpectedCXC (std::string cxc)
{
	m_expectedCxc = cxc;
}

inline  std::string FIXS_CCH_IptbUpgrade::getLastUpgradeDate () const
{
	return m_lastUpgradeDate;
}

inline  std::string FIXS_CCH_IptbUpgrade::getLastUpgradeTime () const
{
	return m_lastUpgradeTime;
}

inline  void FIXS_CCH_IptbUpgrade::setLastUpgradeResult (int result)
{
	m_lastSwUpgradeResult = static_cast<CCH_Util::RpUpgradeResult>(result);
}

inline  void FIXS_CCH_IptbUpgrade::setLastUpgradeReason (int reason)
{
	m_lastSwUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(reason);
}

inline  int FIXS_CCH_IptbUpgrade::getLastUpgradeResult () const
{
	return m_lastSwUpgradeResult;
}

inline  int FIXS_CCH_IptbUpgrade::getLastUpgradeReason () const
{
	return m_lastSwUpgradeReason;
}

inline  bool FIXS_CCH_IptbUpgrade::isInitialized ()
{
	return m_initialized;
}
inline  void FIXS_CCH_IptbUpgrade::setLastErrorCode (int code)
{

	m_lastErrorCode = code;

}

inline  bool FIXS_CCH_IptbUpgrade::isRunningThread ()
{
	return m_running;
}

#endif
