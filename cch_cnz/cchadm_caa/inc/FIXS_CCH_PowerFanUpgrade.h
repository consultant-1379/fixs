/*
 * FIXS_CCH_PowerFanUpgrade.h
 *
 *  Created on: Nov 28, 2011
 *      Author: eanform
 */

#ifndef FIXS_CCH_POWERFANUPGRADE_H_
#define FIXS_CCH_POWERFANUPGRADE_H_

#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>

#include "FIXS_CCH_SNMPManager.h"
#include "FIXS_CCH_FSMDataStorage.h"
#include "FIXS_CCH_UpgradeManager.h"
//#include "FIXS_CCH_PowerFanModule.h"
#include "FIXS_CCH_Util.h"
#include "FIXS_CCH_StateMachine.h"
#include "Client_Server_Interface.h"
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_TrapTimer.h"
#include "FIXS_CCH_JTPHandler.h"

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sys/eventfd.h>
#include <list>

#include "ACS_TRA_Logging.h"

#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"

XERCES_CPP_NAMESPACE_USE
XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

class FIXS_CCH_JTPHandler;

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

class FIXS_CCH_PowerFanUpgrade : public FIXS_CCH_StateMachine
{

  private:
   typedef enum { SNMP_SET_OK, SNMP_SET_NOT_OK, SNMP_TRAP_OK, SNMP_TRAP_NOT_OK, DEFAULT_NO_EVENT } PfmEventId;

    typedef enum { STATE_IDLE, STATE_ONGOING, STATE_UNDEFINED } PfmStateId;

  public:

    class PFMState
    {

      public:

          PFMState();

          PFMState (PfmStateId stateId);

          virtual ~PFMState();

          virtual int upgrade (std::string completePathPfm, std::string pfmTypeIndex, std::string pfmInstanceIndex, std::string slot) = 0;

          virtual int pfmFwUpgradeResultTrap (int trapResult) = 0;

          void setContext (FIXS_CCH_PowerFanUpgrade* context);

           FIXS_CCH_PowerFanUpgrade::PfmStateId getStateId () const;

          void snmpSet (bool result);

        // Data Members for Class Attributes
         bool m_set_status;

        // Additional Public Declarations

      protected:
        // Data Members for Class Attributes

          FIXS_CCH_PowerFanUpgrade* m_context;

          FIXS_CCH_PowerFanUpgrade::PfmStateId m_id;

        // Additional Protected Declarations

      private:

          PFMState(const PFMState &right);

          PFMState & operator=(const PFMState &right);

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class StateIdle : public PFMState
    {

    public:

          StateIdle();

          virtual ~StateIdle();

          virtual int upgrade (std::string completePathPfm, std::string pfmTypeIndex, std::string pfmInstanceIndex, std::string slot);

          virtual int pfmFwUpgradeResultTrap (int trapResult);

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

    class StateOngoing : public PFMState
    {
      public:

          StateOngoing();

          virtual ~StateOngoing();

          virtual int upgrade (std::string completePathPfm, std::string pfmTypeIndex, std::string pfmInstanceIndex, std::string slot);

          virtual int pfmFwUpgradeResultTrap (int trapResult);

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

    	FIXS_CCH_PowerFanUpgrade::PfmEventId getFirst ();

    	void popData ();

    	void pushData (PfmEventId info);

    	bool queueEmpty ();

    	size_t queueSize ();

    	FIXS_CCH_PowerFanUpgrade::PfmEventId getItem (int index);

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
          ACE_thread_mutex_t m_queueCriticalSection;

          std::list<PfmEventId> m_qEventInfo;

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations

    };

    class SaxHandler : virtual public HandlerBase
    {

      public:

          SaxHandler (const std::string &xmlFilePath, CCH_Util::pfmData &pfmSwData);

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

           CCH_Util::pfmData &m_pfmData;
           bool m_cxcProdnum;
           bool m_cxcRev;
           bool m_cxpProdnum;
           bool m_cxpRev;           

           ACS_TRA_trace *traceSax;
        // Additional Private Declarations
      private:
        // Additional Implementation Declarations

    };

      FIXS_CCH_PowerFanUpgrade();

      FIXS_CCH_PowerFanUpgrade (unsigned long magazine, unsigned short slot, std::string installedPackage, std::string SCX_IP_EthA, std::string SCX_IP_EthB, std::string dn_blade);

      virtual ~FIXS_CCH_PowerFanUpgrade();

      FIXS_CCH_JTPHandler *   m_jtpHandlerPFM[2];

      int upgrade (std::string completePathPfm, std::string pfmTypeIndex, std::string pfmInstanceIndex, std::string slot);

      int pfmFwUpgradeResultTrap (int trapResult);

      void switchState (PfmEventId eventId);

      std::string getStateName ();

      std::string getPowerResultState ();

      std::string getFanResultState ();

      bool setSnmp (std::string completePathFw, std::string pfmTypeIndex, std::string pfmInstanceIndex);
	  
	  bool setSnmpSwUpg (std::string completePathFw, std::string pfmTypeIndex, std::string pfmInstanceIndex);

      int setEvent (PfmEventId event);

      int open(void *args = 0);

      //virtual DWORD run ();
      virtual int svc(void);

      void stop ();

      void setType (std::string pfmTypeIndex);

      std::string getType ();

      bool setScanningOff ();

      bool setScanningOn ();

      virtual void restoreStatus (CCH_Util::PFMData &pfmdata);

      virtual void getOverallStatus (CCH_Util::PFMData &pfmdata);

      virtual void saveStatus ();

      void setModule (std::string pfmInstanceIndex);

      void setPath (std::string completePathPfm);

      std::string getCurrentRunningFirmware (CCH_Util::PFMType index);

      std::string getCurrentModuleDeviceType (bool isSmx = false);
	  
	  int getCurrentSwUpgradeStatus (int swType, int typeInstance);
	  
	  int getCurrentRunningPFMSoftwareType (int typeIndex=-1);
	  
	  std::string getCurrentRunningPFMSoftware (CCH_Util::PFMUpgType index);
	  
	  std::string getCurrentRunningPFMCxcSoftwareRev (CCH_Util::PFMUpgType index);
	  
	  std::string getCurrentRunningPFMCxcSoftwareProduct (CCH_Util::PFMUpgType index);

      void setUpgradeStatus (CCH_Util::PFMFwStatus upgradeFwStatus);

      void setUpgradePowerResult (CCH_Util::PFMResult upgradePowerStatus);

      void setUpgradeFanResult (CCH_Util::PFMResult upgradeFanStatus);

      int getUpgradeStatus ();

      bool isRunningThread();

      std::string getDnPfmObject(){ return m_pfm_dn;};

    // Data Members for Associations

      FIXS_CCH_TrapTimer *m_trapTimer;

  public:
    // Additional Public Declarations

  protected:
    // Additional Protected Declarations

      bool m_running;

  private:

      FIXS_CCH_PowerFanUpgrade(const FIXS_CCH_PowerFanUpgrade &right);

      FIXS_CCH_PowerFanUpgrade & operator=(const FIXS_CCH_PowerFanUpgrade &right);

      void handleCommandEvent ();

      //	1) when the LoadReleaseCompleted trap is missing, check the loaded software
      //	on SCX
      //	2) when the cold start trap is missing, check the current software on SCX
      void handleTrapTimeout ();

      bool cancelTrapTimer ();

      bool startTrapTimer (int seconds);

	  bool initializeParser (std::string &filenameXml, CCH_Util::pfmData &pfmdata);
      //	This method takes care of all initialization of the state machine.
      void initOnStart ();

      //	Takes proper actions when CCH restarts
      void onRestart ();
	  //	Takes proper actions when CCH restarts in SMX env
	  void onRestartInSmx ();

      void setPfmDn ();

      bool removeTftpFileName();

      void resetEvents();

    // Data Members for Class Attributes

      //	This handle is enabled when a command is received.

      int m_upgradeFwStatus;

      //HANDLE m_cmdEvent;
      EventHandle m_cmdEvent;

      //HANDLE m_shutdownEvent;
      EventHandle m_shutdownEvent;

      EventQueue m_eventQueue;

      unsigned short m_boardId;

      //HANDLE m_timeoutTrapEvent;
      int m_timeoutTrapEvent;

      unsigned long m_magazine;

      CCH_Util::PFMModule m_moduleId;

      CCH_Util::PFMType m_typeId;

      CCH_Util::PFMResult m_fanResult;

      CCH_Util::PFMResult m_powerResult;

      ACE_thread_mutex_t *m_snmpSyncPtr;

      std::string m_SCX_IP_EthA;

      std::string m_SCX_IP_EthB;

      unsigned short m_slot;

      std::string m_dn;

      std::string m_type;

      std::string m_pfm_dn;

      //	this attribute stores the filename used to build paths like:
      //	"192.168.169.1:/filename"

      std::string m_path;

      ACS_TRA_trace *traceObj;

      // Data Members for Associations

      PFMState *m_currentState;

      StateOngoing m_ongoing;

      StateIdle m_idle;

      EventHandle m_StopEvent;

      std::string eventNameStop;

      ACS_TRA_Logging *FIXS_CCH_logging;

  private:

};


inline  int FIXS_CCH_PowerFanUpgrade::EventQueue::getHandle () const
{
	return m_queueHandle;
}

// Class FIXS_CCH_PowerFanUpgrade
inline  void FIXS_CCH_PowerFanUpgrade::setUpgradeStatus (CCH_Util::PFMFwStatus upgradeFwStatus)
{
	m_upgradeFwStatus = upgradeFwStatus;
}

inline  void FIXS_CCH_PowerFanUpgrade::setUpgradePowerResult (CCH_Util::PFMResult upgradePowerStatus)
{
	m_powerResult = upgradePowerStatus;
}

inline  void FIXS_CCH_PowerFanUpgrade::setUpgradeFanResult (CCH_Util::PFMResult upgradeFanStatus)
{
	m_fanResult = upgradeFanStatus;
}

inline  int FIXS_CCH_PowerFanUpgrade::getUpgradeStatus ()
{
	return m_upgradeFwStatus;
}

inline  bool FIXS_CCH_PowerFanUpgrade::isRunningThread ()
{
	return m_running;
}


#endif /* FIXS_CCH_POWERFANUPGRADE_H_ */
