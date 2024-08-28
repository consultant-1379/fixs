#ifndef FIXS_CMXH_Manager_h
#define FIXS_CMXH_Manager_h 1

#include <string>
#include <list>
#include <set>
#include "FIXS_CMXH_SNMPManager.h"
#include "FIXS_CMXH_SNMPHandler.h"
#include <map>
#include "ACS_CS_API.h"

using namespace std;
#define MAX_NO_OF_CMXB 50

#define UNUSED(expr) do { (void)(expr); } while (0)

struct CMXIPAddress
{
	std::string IP_EthA;
	std::string IP_EthB;
	int index;
	int sw_version;
};

class FIXS_CMXH_Manager : public ACS_CS_API_HWCTableObserver
{

public:
	//## Constructors
	FIXS_CMXH_Manager();

	//## Destructor
	~FIXS_CMXH_Manager();


	//## Other Operations
	//## Operation: getInstance
	static  FIXS_CMXH_Manager * getInstance ();

	void finalize ();

	//## Operation: initCMXManager
	int initCMXManager ();

	//## Operation: update
	void update (const ACS_CS_API_HWCTableChange& observer);

	//## Operation: createHandleId
	int createHandleId ();

	//## Operation: getAllCMXBoardAddress
	int getAllCMXBoardAddress (std::vector<CMXIPAddress> &cmxVector);

	//## Operation: isCMXB
	bool isCMXB (int magazine, int slot, std::string ip);

	//## Operation: retrySettingAtCMXStart
	void retrySettingAtCMXStart (std::string ip);

	//## Operation: deleteLogsInCMX
	void deleteLogsInCMX (std::string &ip);

	//## Operation: getLogsFromCMX
	int getLogsFromCMX (string &ipNumber);

	//## Operation: RSTP
	void HandleRSTPforCMX4 (std::string ip);

	int getNewboardSwVersion ( std::string ipA_String, std::string ipB_String, int &sw_version);
	
	bool isCMXBVctrEmpty() {ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);return cmxVector.empty();}; //HY36723

	// Additional Public Declarations


protected:
	// Additional Protected Declarations

private:
	// Additional Private Declarations

private: //## implementation
	// Data Members for Class Attributes

	//## Attribute: instance
	static FIXS_CMXH_Manager* m_instance;

	//## Attribute: snmpManager
	FIXS_CMXH_SNMPManager* m_snmpManager;

	//## Attribute: lastAssignedHandleId
	int m_lastAssignedHandleId;

	//## Attribute: handleIdSet
	std::set<int> m_handleIdSet;

	//## Attribute: snmpHandler
	FIXS_CMXH_SNMPHandler* snmpHandler[MAX_NO_OF_CMXB];

	//## Attribute: cmxVector
	std::vector<CMXIPAddress>  cmxVector;

	// Additional Implementation Declarations
	/**
			@brief  fixs_cmxh_ManagerTrace
	 */
	ACS_TRA_trace* _trace;
	ACS_TRA_Logging *FIXS_CMXH_logging;

        static ACE_Recursive_Thread_Mutex m_lock;    // TR_HX98360


};

#endif
