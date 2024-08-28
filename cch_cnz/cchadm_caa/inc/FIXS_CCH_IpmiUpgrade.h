#ifndef FIXS_CCH_IpmiUpgrade_h
#define FIXS_CCH_IpmiUpgrade_h 1

#include <string>
#include <map>
#include <vector>
#include <sys/eventfd.h>
#include <iostream>
#include <list>

#include "ACS_TRA_Logging.h"

#include "Client_Server_Interface.h"
#include "FIXS_CCH_FSMDataStorage.h"
#include "FIXS_CCH_Util.h"
#include "FIXS_CCH_UpgradeManager.h"
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_SNMPManager.h"
#include "FIXS_CCH_StateMachine.h"
#include "FIXS_CCH_TrapTimer.h"

class FIXS_CCH_JTPHandler;

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

namespace{
	typedef int EventHandle;
}

class FIXS_CCH_IpmiUpgrade : public FIXS_CCH_StateMachine
{
  private:
    typedef enum { STATE_IDLE, STATE_ONGOING } IpmiStateId;

  public:
    typedef enum { DEFAULT_NO_EVENT, SNMP_SET_OK, SNMP_SET_NOT_OK, SNMP_TRAP_OK, SNMP_TRAP_NOT_OK } IpmiEventId;

    class IpmiState
    {

      public:

          IpmiState();

          IpmiState (IpmiStateId id);

          virtual ~IpmiState();

          virtual int ipmiUpgradeResultTrap (int result) = 0;

          FIXS_CCH_IpmiUpgrade::IpmiStateId getStateId () const;

          void setContext (FIXS_CCH_IpmiUpgrade* context);

          virtual void setSnmpStatus (bool status);

          virtual int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
          std::string slot, 	// contains the path and the IP address of the software version to set
          std::string ipMasterSCX	// contains the path and the IP address of the software version to set
          ) = 0;

         // void setUpgradeResult(int res);

          int getUpgradeResult();

        // Additional Public Declarations

      protected:
        // Data Members for Class Attributes

          FIXS_CCH_IpmiUpgrade* m_context;

          FIXS_CCH_IpmiUpgrade::IpmiStateId m_id;

          bool m_set_status;

         // int m_upgradeResult;

        // Additional Protected Declarations

      private:

          IpmiState(const IpmiState &right);

          IpmiState & operator=(const IpmiState &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class StateIdle : public IpmiState
    {

      public:

          StateIdle();

          virtual ~StateIdle();

          virtual int ipmiUpgradeResultTrap (int result);

          int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
          std::string slot, 	// contains the path and the IP address of the software version to set
          std::string ipMasterSCX);

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

    class StateOngoing : public IpmiState
    {

      public:

          StateOngoing();

          virtual ~StateOngoing();

          virtual int ipmiUpgradeResultTrap (int result);

          int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
          std::string slot, 	// contains the path and the IP address of the software version to set
          std::string ipMasterSCX);

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

    class EventQueue
    {
      public:

          EventQueue();

          virtual ~EventQueue();

          FIXS_CCH_IpmiUpgrade::IpmiEventId getFirst ();

          void popData ();

          void pushData (IpmiEventId info);

          bool queueEmpty ();

          size_t queueSize ();

          FIXS_CCH_IpmiUpgrade::IpmiEventId getItem (int index);

          //HANDLE getHandle () const;
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

          //HANDLE m_queueHandle;
          int m_queueHandle;

          //CRITICAL_SECTION m_queueCriticalSection;
          ACE_thread_mutex_t  m_queueCriticalSection;

          std::list<IpmiEventId> m_qEventInfo;

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

      FIXS_CCH_IpmiUpgrade();

      FIXS_CCH_IpmiUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage, std::string SCX_IP_EthA, std::string SCX_IP_EthB, std::string dn);

      virtual ~FIXS_CCH_IpmiUpgrade();

      FIXS_CCH_JTPHandler *   m_jtpHandlerIpmi[2];

      virtual int svc(void);

	  virtual int open(void *args = 0);

	  virtual void stop ();

      int upgrade (std::string valueToSet, 	// contains the path and the IP address of the software version to set
      std::string slot, 	// contains the path and the IP address of the software version to set
      std::string ipMasterSCX	// contains the path and the IP address of the software version to set
      );

      int ipmiUpgradeResultTrap (int result);

      std::string getStateName ();

      void switchState (IpmiEventId eventId);

      virtual void setSnmpStatus (bool status);

      void setUpgradeResult(int res);

      void  stopJtpThread();

      int getUpgradeResult();

      bool startTrapEventTimer (int seconds);

      bool cancelTrapEventTimer ();

      void setEvent (IpmiEventId eventId);

      int checkIPMIStatus (std::string ip_EthA, std::string ip_EthB, std::string slot);

      int getIPMIUpgrade (std::string ip_EthA, std::string ip_EthB, std::string slot, std::string &ipmiFw);

      virtual void restoreStatus (CCH_Util::IpmiData &ipmidata);

      virtual void saveStatus ();

      void turnOffStateMachine ();

      void turnOnStateMachine (int iSlot, std::string ipMasterSCX, std::string slotMaster);

      std::string getOtherBladeDn();

      std::string getOtherBladeHardwareInfoDn();

      void setOtherBladeHardwareInfoDn(std::string dn);

      void setOtherBladeDn(std::string dn);

      //swBladePersistence
      void setSwBladePersistanceDn(std::string dn);

      std::string getSwBladePersistanceDn();

      bool sendStartRequestToCP (unsigned char slotOngoing);

    // Data Members for Class Attributes

      int m_slotUpgOnGoing;

      bool isRunningThread();

      std::string m_ipMaster;

      std::string m_ipPassive;

      std::string m_slotMaster;

      std::string m_tftpPath;

    // Data Members for Associations

  public:
    // Additional Public Declarations

      FIXS_CCH_TrapTimer *m_trapTimer;

  protected:
    // Additional Protected Declarations
      bool m_running;

      int m_upgradeResult;

  private:

      void handleCommandEvent ();

      //	1) when the LoadReleaseCompleted trap is missing, check the loaded software
      //	on SCX
      //	2) when the cold start trap is missing, check the current software on SCX
      void handleFiveMinutesTrapTimeout ();

      bool removeTftpFileName();

      void initOnStart ();

      void onRestart ();

      void resetEvents ();

    // Data Members for Class Attributes

      unsigned short m_boardId;

      unsigned long m_magazine;

      unsigned short m_slot;

      std::string m_installedPackage;

      std::string m_SCX_IP_EthA;

      std::string m_SCX_IP_EthB;

      //destinguish name
      std::string m_otherBlade_dn;

      //HANDLE m_cmdEvent;
      EventHandle m_cmdEvent;

      //HANDLE m_shutdownEvent;
      EventHandle m_shutdownEvent;

      EventQueue m_eventQueue;

      ACE_thread_mutex_t *m_snmpSyncPtr;

      //HANDLE m_timeOutTrapEvent;
      int m_timeOutTrapEvent;

      //FIXS_CCH_JTPHandler *   m_jtpHandlerIpmi[2];

      // Data Members for Associations

      ACS_TRA_trace *traceObj;

      IpmiState *m_currentState;

      StateIdle m_idle;

      StateOngoing m_ongoing;

      EventHandle m_StopEvent;

      std::string eventNameStop;

      ACS_TRA_Logging *FIXS_CCH_logging;

      std::string m_otherBladeHwInfo_dn;

      std::string m_swBladePersistance_dn;
      // Additional Private Declarations

  //private:
    // Additional Implementation Declarations

};

// Class FIXS_CCH_IpmiUpgrade::IpmiState

// Class FIXS_CCH_IpmiUpgrade::StateIdle

// Class FIXS_CCH_IpmiUpgrade::StateOngoing

// Class FIXS_CCH_IpmiUpgrade::EventQueue


inline  int FIXS_CCH_IpmiUpgrade::EventQueue::getHandle () const
{
	return m_queueHandle;
}

inline  bool FIXS_CCH_IpmiUpgrade::isRunningThread ()
{
	return m_running;
}

inline  void FIXS_CCH_IpmiUpgrade::setUpgradeResult (int res)
{
	m_upgradeResult = res;
}

inline int FIXS_CCH_IpmiUpgrade::getUpgradeResult()
{
	return m_upgradeResult;
}

// Class FIXS_CCH_IpmiUpgrade

#endif
