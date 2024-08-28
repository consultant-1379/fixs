#ifndef FIXS_CCH_IplbUpgrade_h
#define FIXS_CCH_IplbUpgrade_h 1
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
#include "FIXS_CCH_IronsideManager.h"
#include "FIXS_CCH_StateMachine.h"
#include "FIXS_CCH_TrapTimer.h"

class MyImplementer;
class FIXS_CCH_IronsideManager;

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

namespace{
typedef int EventHandle;
}
class FIXS_CCH_IplbUpgrade : public FIXS_CCH_StateMachine
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
	IplbStateId;
	class IplbState
	{
	public:
		IplbState();

		IplbState (IplbStateId stateId);

		virtual ~IplbState();


		void setContext (FIXS_CCH_IplbUpgrade* context);

		FIXS_CCH_IplbUpgrade::IplbStateId getStateId () const;

		virtual int activateSw ();

		virtual int commit (bool setAsDefaultPackage);

		virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation = true);

		virtual int cancel ();

		// Data Members for Class Attributes
		bool m_set_status;
		// Additional Public Declarations
	protected:
		// Data Members for Class Attributes

		FIXS_CCH_IplbUpgrade::IplbStateId m_id;

		FIXS_CCH_IplbUpgrade* m_context;

	private:

		IplbState(const IplbState &right);

		IplbState & operator=(const IplbState &right);

	private: //## implementation
		// Additional Implementation Declarations

	};

	class IplbStateActivated : public IplbState  //## Inherits: <unnamed>%4D2DCD8A020C
	{
		//## begin FIXS_CCH_IplbUpgrade::IplbStateActivated%4D2DCC9402C7.initialDeclarations preserve=yes
		//## end FIXS_CCH_IplbUpgrade::IplbStateActivated%4D2DCC9402C7.initialDeclarations

	public:
		IplbStateActivated();

		virtual ~IplbStateActivated();

		virtual int commit (bool setAsDefaultPackage);

		virtual int cancel ();

	protected:

	private:

		IplbStateActivated(const IplbStateActivated &right);

		IplbStateActivated & operator=(const IplbStateActivated &right);

	private: //## implementation
	};

	class IplbStateActivating : public IplbState  //## Inherits: <unnamed>%4D2DCD6B0193
	{
	public:

		IplbStateActivating();

		virtual ~IplbStateActivating();

	protected:
	private:

		IplbStateActivating(const IplbStateActivating &right);


		IplbStateActivating & operator=(const IplbStateActivating &right);


	private: //## implementation

	};

	class IplbStateFailed : public IplbState  //## Inherits: <unnamed>%4D2DCD8F00C3
	{
	public:

		IplbStateFailed();

		virtual ~IplbStateFailed();

		virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation = true);

	protected:

	private:

		IplbStateFailed(const IplbStateFailed &right);

		IplbStateFailed & operator=(const IplbStateFailed &right);

	private: //## implementation
	};

	class IplbStateIdle : public IplbState  //## Inherits: <unnamed>%4D2DCC2B01DB
	{
	public:

		IplbStateIdle();

		virtual ~IplbStateIdle();


		virtual int prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation = true);

	protected:

	private:

		IplbStateIdle(const IplbStateIdle &right);

		IplbStateIdle & operator=(const IplbStateIdle &right);

	private: //## implementation
	};

	class IplbStatePrepared : public IplbState  //## Inherits: <unnamed>%4D2DCD530251
	{
	public:

		IplbStatePrepared();

		virtual ~IplbStatePrepared();
		virtual int activateSw ();

		virtual int cancel ();

	protected:

	private:
		IplbStatePrepared(const IplbStatePrepared &right);

		IplbStatePrepared & operator=(const IplbStatePrepared &right);

	private: //## implementation
	};

	typedef enum
	{
		IPLB_PREPARE_OK, //0
		IPLB_ACTIVATE_OK,
		IPLB_COMMIT_OK,
		IPLB_MANUAL_ROLLBACK_OK,
		IPLB_AUTOMATIC_ROLLBACK_OK,
		IPLB_ACTIVATE_RECEIVED,
		IPLB_COMMIT_RECEIVED,
		IPLB_CANCEL_RECEIVED,
		IPLB_FAILURE_RECEIVED,
		IPLB_DEFAULT_NO_EVENT,
		IPLB_RESET_FAILURE_RECEIVED
	} IplbEventId;

	class EventQueue
	{
	public:
		EventQueue();

		virtual ~EventQueue();

		FIXS_CCH_IplbUpgrade::IplbEventId  getFirst ();

		void popData ();

		void pushData (IplbEventId info);

		bool queueEmpty ();

		size_t queueSize ();

		FIXS_CCH_IplbUpgrade::IplbEventId getItem (int index);

		int getHandle () const;

		void cleanQueue();
	protected:

	private:
		EventQueue(const EventQueue &right);

		EventQueue & operator=(const EventQueue &right);

		// Data Members for Class Attributes

		int m_queueHandle;

		ACE_thread_mutex_t m_queueCriticalSection;

		std::list<IplbEventId> m_qEventInfo;

	private:
	};


	class IplbAutomaticRollbackEnded : public IplbState  //## Inherits: <unnamed>%4D2DCD96021A
	{
	public:

		IplbAutomaticRollbackEnded();

		virtual ~IplbAutomaticRollbackEnded();

		virtual int commit (bool setAsDefaultPackage);

	protected:

	private:

		IplbAutomaticRollbackEnded(const IplbAutomaticRollbackEnded &right);

		IplbAutomaticRollbackEnded & operator=(const IplbAutomaticRollbackEnded &right);

	private:
	};

	class IplbAutomaticRollbackOngoing : public IplbState
	{

	public:
		IplbAutomaticRollbackOngoing();

		virtual ~IplbAutomaticRollbackOngoing();

	protected:

	private:

		IplbAutomaticRollbackOngoing(const IplbAutomaticRollbackOngoing &right);

		IplbAutomaticRollbackOngoing & operator=(const IplbAutomaticRollbackOngoing &right);

	private:

	};

	class IplbManualRollbackEnded : public IplbState
	{
	public:
		IplbManualRollbackEnded();

		virtual ~IplbManualRollbackEnded();

		virtual int commit (bool setAsDefaultPackage);

	protected:

	private:
		IplbManualRollbackEnded(const IplbManualRollbackEnded &right);

		IplbManualRollbackEnded & operator=(const IplbManualRollbackEnded &right);

	private:

	};

	class IplbManualRollbackOngoing : public IplbState  //## Inherits: <unnamed>%4D2DCD9B01FA
	{
	public:
		IplbManualRollbackOngoing();

		virtual ~IplbManualRollbackOngoing();

	protected:

	private:

		IplbManualRollbackOngoing(const IplbManualRollbackOngoing &right);

		IplbManualRollbackOngoing & operator=(const IplbManualRollbackOngoing &right);

	private:
	};

	FIXS_CCH_IplbUpgrade (unsigned short boardId, unsigned long magazine, unsigned short slot, std::string installedPackage,std::string iptAddress_EthA, std::string iptAddress_EthB,std::string dn);

	virtual ~FIXS_CCH_IplbUpgrade();
	void setObjectDN(const char* dn);

	void setSwInvObjectDN(std::string dn);

	std::string getSwInvObjectDN();

	std::string getSwPercistanceDN();

	int activateSw ();

	int commit (bool setAsDefaultPackage);

	int prepare (std::string packageToUpgrade, std::string productToUpgrade, bool partial_installation	// partial == normal
	);

	int setEvent (IplbEventId event);

	virtual int svc(void);

	virtual int open (void *args = 0);

	virtual void stop ();

	virtual void setLastErrorCode (int code);

	void switchPriSecIp ();

	void switchState (IplbEventId eventId);

	void saveStatus ();

	int cancel ();

	std::string getStateName ();

	int getStateImm ();

	void setPath (std::string completePathIplb);

	//	Set the expected Load Module package (cxp) on prepare
	void setUpgradeType(int upgradeType);

	void setForcedUpgrade (bool forcedUpgrade);

	void setExpectedCXP (std::string cxp = "");

	std::string getExpectedCXP () const;

	void setCurrentCXP (std::string cxp);

	std::string getCurrentCXP () const;

	int getUpgradeType ();

	FIXS_CCH_IplbUpgrade::IplbStateId getStateId () const;

	virtual bool configureDHCP (bool rollback);

	//	set xshmcBoardReset in xshmcBoardTable of SCB-RP/4
	bool resetBoard ();

	std::string  getIptFileName(std::string package);

	int onActivating ();
	
	void onPreparedOrActivated(bool );

	std::string getCurrentCXC () const;

	void setCurrentCXC (std::string cxc);

	std::string getExpectedCXC () const;

	//	Set the expected Load Module package (cxc) on prepare
	void setExpectedCXC (std::string cxc = "");

	int onPreparing ();

	bool initSwInventory();

	void getOverallStatus (CCH_Util::SWData &iptdata);

	std::string getLastUpgradeDate () const;

	std::string getLastUpgradeTime () const;

	void setLastUpgradeResult (int result);

	void setLastUpgradeReason (int reason);

	int getLastUpgradeResult () const;

	int getLastUpgradeReason () const;

	void restoreStatus (CCH_Util::SWData &boardSWData);

	int onAutomaticRollback ();

	int onManualRollback ();

	int checkUpgradeStatus ();

	int getIthbyteOfGprData(std::string grpData,int i);

//	void setResultAndReason (CCH_Util::LmErrors status);
        
        void setResultAndReason (int result,int reason); 

	std::string getUpgradedCXP();

	//	This method runs the following steps:
	//	- configure DHCP
	//	- read master SCB-RP
	//	- in case of full upgrade sets SNMP GPR VALUE to network boot
	//	- resetBoard

	bool isInitialized ();

	bool resetGprRegister7 ();

	int isBoardBlocked ();

	bool isRunningThread ();

	// Data Members for Class Attributes

	unsigned long m_magazine;

	unsigned short m_slot;

	std::string m_iplbAddress_EthA;

	std::string m_iplbAddress_EthB;

	// Data Members for Associations


	FIXS_CCH_IronsideManager *m_ironsideManager;

	public:

	protected:
	// Data Members for Class Attributes
	bool m_running;

	int m_lastErrorCode;
	private:
	FIXS_CCH_IplbUpgrade();

	FIXS_CCH_IplbUpgrade(const FIXS_CCH_IplbUpgrade &right);

	FIXS_CCH_IplbUpgrade & operator=(const FIXS_CCH_IplbUpgrade &right);

	bool modifySwInventory(int state);

	void handleCommandEvent ();

	//	This method takes care of all initialization of the state machine.
	void initOnStart ();

	//	Takes proper actions when CCH restarts
	void onRestart ();
	void resetEvents();
	int setBootType (int bootType);


	std::string getIplbFileName( std::string package);
	int executeRollbackSteps(bool isAutoRollback = false);

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

	IplbState *m_currentState;

	std::string m_previousCxp;

	std::string m_currentCxp;

//	bool m_fullUpgradeType;

	std::string m_currentCxc;

	std::string m_expectedCxp;

	std::string m_expectedCxc;

	std::string m_Object_DN;

	std::string dn_blade_persistance;

	std::string m_lastUpgradeDate;

	std::string m_lastUpgradeTime;

	CCH_Util::RpUpgradeResult m_lastSwUpgradeResult;

	CCH_Util::RpUpgradeResultInfo m_lastSwUpgradeReason;

	std::string m_SwInventory_Object_DN;

	int m_previousState;

	int m_failureCode;

	bool m_initialized;

	bool m_gprRegister7resetted;

	bool m_egem2environment;

	bool m_forcedUpgrade;

	int m_upgradeType;

	// Data Members for Associations

	IplbStateIdle m_idle;

	IplbStatePrepared m_prepared;

	IplbStateActivating m_activating;

	IplbStateActivated m_activated;

	IplbStateFailed m_failed;

	IplbManualRollbackOngoing m_manualRollbackOngoing;

	IplbManualRollbackEnded m_manualRollbackEnded;

	IplbAutomaticRollbackOngoing m_automaticRollbackOngoing;

	IplbAutomaticRollbackEnded m_automaticRollbackEnded;

	// Additional Private Declarations
	private:

	// Additional Implementation Declarations
};


// Class FIXS_CCH_IplbUpgrade::IplbState

// Class FIXS_CCH_IplbUpgrade::IplbStateActivated

// Class FIXS_CCH_IplbUpgrade::IplbStateActivating

// Class FIXS_CCH_IplbUpgrade::IplbStateFailed

// Class FIXS_CCH_IplbUpgrade::IplbStateIdle

// Class FIXS_CCH_IplbUpgrade::IplbStatePrepared

// Class FIXS_CCH_IplbUpgrade::EventQueue


inline  int FIXS_CCH_IplbUpgrade::EventQueue::getHandle () const
{
	return m_queueHandle;
}

// Class FIXS_CCH_IplbUpgrade::IplbAutomaticRollbackEnded

// Class FIXS_CCH_IplbUpgrade::IplbAutomaticRollbackOngoing

// Class FIXS_CCH_IplbUpgrade::IplbManualRollbackEnded

// Class FIXS_CCH_IplbUpgrade::IplbManualRollbackOngoing

// Class FIXS_CCH_IplbUpgrade

inline  void FIXS_CCH_IplbUpgrade::setUpgradeType (int upgradeType)
{
	m_upgradeType = upgradeType;
}

inline  void FIXS_CCH_IplbUpgrade::setForcedUpgrade (bool forcedUpgrade)
{
	m_forcedUpgrade = forcedUpgrade;
}

inline  void FIXS_CCH_IplbUpgrade::setExpectedCXP (std::string cxp)
{
	m_expectedCxp = cxp;
}

inline  std::string FIXS_CCH_IplbUpgrade::getExpectedCXP () const
{
	return m_expectedCxp;
}

inline  void FIXS_CCH_IplbUpgrade::setCurrentCXP (std::string cxp)
{
	m_currentCxp = cxp;
}

inline  std::string FIXS_CCH_IplbUpgrade::getCurrentCXP () const
{
	return m_currentCxp;
}

inline  int FIXS_CCH_IplbUpgrade::getUpgradeType ()
{
	return m_upgradeType;
}

inline  std::string FIXS_CCH_IplbUpgrade::getCurrentCXC () const
{
	return m_currentCxc;
}

inline  void FIXS_CCH_IplbUpgrade::setCurrentCXC (std::string cxc)
{
	m_currentCxc = cxc;
}

inline  std::string FIXS_CCH_IplbUpgrade::getExpectedCXC () const
{
	return m_expectedCxc;
}

inline  void FIXS_CCH_IplbUpgrade::setExpectedCXC (std::string cxc)
{
	m_expectedCxc = cxc;
}

inline  std::string FIXS_CCH_IplbUpgrade::getLastUpgradeDate () const
{
	return m_lastUpgradeDate;
}

inline  std::string FIXS_CCH_IplbUpgrade::getLastUpgradeTime () const
{
	return m_lastUpgradeTime;
}

inline  void FIXS_CCH_IplbUpgrade::setLastUpgradeResult (int result)
{
	m_lastSwUpgradeResult = static_cast<CCH_Util::RpUpgradeResult>(result);
}

inline  void FIXS_CCH_IplbUpgrade::setLastUpgradeReason (int reason)
{
	m_lastSwUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(reason);
}

inline  int FIXS_CCH_IplbUpgrade::getLastUpgradeResult () const
{
	return m_lastSwUpgradeResult;
}

inline  int FIXS_CCH_IplbUpgrade::getLastUpgradeReason () const
{
	return m_lastSwUpgradeReason;
}

inline  bool FIXS_CCH_IplbUpgrade::isInitialized ()
{
	return m_initialized;
}
inline  void FIXS_CCH_IplbUpgrade::setLastErrorCode (int code)
{

	m_lastErrorCode = code;

}

inline  bool FIXS_CCH_IplbUpgrade::isRunningThread ()
{
	return m_running;
}

#endif


