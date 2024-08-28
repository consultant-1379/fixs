
#ifndef SNMPMANAGER_H_
#define SNMPMANAGER_H_ 1

#include <ace/Task.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/mib_api.h>
#include <net-snmp/library/mib.h>

#include "acs_nclib_session.h"
#include "acs_nclib_library.h"
#include "acs_nclib_factory.h"
#include "acs_nclib_message.h"
#include "acs_nclib_udp.h"
#include "acs_nclib_udp_reply.h"

#include <string.h>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <algorithm>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cmd_Utils.h"

#include "fixs_cch_netconfparser.h"

class fixs_cch_cmdshelfmanager // : public ACE_Task_Base
{

public:

	static const int GETSET_SNMP_OK = 0;
	static const int GETSET_SNMP_OK_LEN_ZERO = 1;
	static const int OPEN_SESS_INIT_ERROR = 2;
	static const int OPEN_SESS_ERROR = 3;
	static const int GETSET_SNMP_PARSE_OID_ERROR = 4;
	static const int GETSET_SNMP_PDU_CREATE_ERROR = 5;
	static const int GETSET_SNMP_ADD_NULL_VAR = 6;
	static const int GETSET_SNMP_RESPONSE_ERROR = 7;
	static const int GETSET_SNMP_STATUS_ERROR = 8;
	static const int GETSET_SNMP_STATUS_TIMEOUT = 9;
	static const int GETSET_SNMP_BUFFERSMALL = 10;

	enum {
		LOADRELEASE = 1,
		STARTRELEASE = 2,
		MAKEDEFAULT = 3,
		REMOVERELEASE = 4
	};

	enum {
		NOATTRIBUTE = 0,
		CURRENTSW = 1,
		DEFAULTSW = 2,
		CURRENTDEFAULTSW = 3
	};

	enum {
		FAILSAFE = 1,
		EXCHANGEBLE = 2,
		PROGRAM = 3
	};

	enum {
		SNMPGET_STD_STRING = 0,
		SNMPGET_HEX_STRING = 1,
		SNMPGET_ASCII_STRING = 2
	};

	fixs_cch_cmdshelfmanager(std::string ipAddress);
	~fixs_cch_cmdshelfmanager();

	std::string getIpAddress(void);
	void setIpAddress(std::string ip);

	bool testBulk(const unsigned int &index,
					const std::string &OID,
					const bool &isHexString = false);

	bool testget();

	/**********************************/
	//Methods with the same base sign
	bool getIfNumber(int &interface);
	bool getBulkIfAlias (const int &no_of_interface);
	bool getBulkIfMTU (const int &no_of_interface);
	bool getBulkIfLastChange (const int &no_of_interface);
	bool getBulkIfphysaddress (const int &no_of_interface);
	bool getBulkIfHighSpeed (const int &no_of_interface);
	bool getBulkIfCounterDiscontinuityTime (const int &no_of_interface);
	bool getBulkIfOperStatus (const int &no_of_interface);
	bool getBulkDot3StatsDuplexStatus (const int &no_of_interface);
	bool getBulkIfHCInOctets (const int &no_of_interface);
	bool getBulkIfHCInUcastPkts (const int &no_of_interface);
	bool getBulkIfHCInMulticastPkts (const int &no_of_interface);
	bool getBulkIfHCInBroadcastPkts (const int &no_of_interface);
	bool getBulkIfHCOutOctets (const int &no_of_interface);
	bool getBulkIfHCOutUcastPkts (const int &no_of_interface);
	bool getBulkIfHCOutMulticastPkts (const int &no_of_interface);
	bool getBulkIfHCOutBroadcastPkts (const int &no_of_interface);
	bool getBulkIfInErrors (const int &no_of_interface);
	bool getBulkIfOutErrors (const int &no_of_interface);
	bool getBulkIfOutDiscards (const int &no_of_interface);
	bool getBulkIfInDiscards (const int &no_of_interface);
	bool getBulkIfInUnknownProtos (const int &no_of_interface);
	bool getBulkDot3HCStatsAlignmentErrors (const int &no_of_interface);
	bool getBulkDot3HCStatsFCSErrors (const int &no_of_interface);
	bool getBulkDot3StatsSingleCollisionFrames (const int &no_of_interface);
	bool getBulkDot3StatsMultipleCollisionFrames (const int &no_of_interface);
	bool getBulkDot3StatsSQETestErrors (const int &no_of_interface);
	bool getBulkDot3StatsDeferredTransmissions (const int &no_of_interface);
	bool getBulkDot3StatsLateCollisions (const int &no_of_interface);
	bool getBulkDot3StatsExcessiveCollisions (const int &no_of_interface);
	bool getBulkDot3HCStatsInternalMacTransmitErrors (const int &no_of_interface);
	bool getBulkDot3StatsCarrierSenseErrors (const int &no_of_interface);
	bool getBulkDot3HCStatsFrameTooLongs (const int &no_of_interface);
	bool getBulkDot3HCStatsInternalMacReceiveErrors (const int &no_of_interface);
	bool getBulkDot3HCStatsSymbolErrors (const int &no_of_interface);
	bool getBulkDot3HCControlInUnknownOpcodes (const int &no_of_interface);
	bool getBulkDot3HCInPauseFrames (const int &no_of_interface);
	bool getBulkDot3HCOutPauseFrames (const int &no_of_interface);
	bool getBulkIfType (const int &no_of_interface);
	void populateIfHCInPkts (const int &no_of_interface);
	void populateIfHCOutPkts (const int &no_of_interface);
	bool getBulkIfName (const int &no_of_interface);

	bool getFwmLoadedAttributeSCX (unsigned int index, int *Val, int &retCode);
	bool getFwmLoadedType (unsigned int index, std::string &loadVer);
	bool getFwmLoadedVersion (unsigned int index, std::string &loadVer);
	bool getBladeRevisionState (std::string *actState);
	std::string intToString (int ivalue);


	/***********************************/
	//NETCONF Methods

	//bool getIfNumber(std::string &shelfId, std::string &slotId, int &interface) {return true;}

	bool checkShelfFromAddress(std::string magAddr, int &retcode);
	bool checkShelfFromAddressUDP(std::string magAddr, int &retcode);
	bool checkShelfFromId(std::string shelfId);

	bool getShelfList(std::vector<std::string> &magAddrs);
	bool getIfDataUDP(std::string shelfAddr, int slot, int &no_of_interface, int verbose = false);
	int getShelfListUDP(std::vector<std::string> &magAddrs);

	/***********************************/
	bool getActiveArea (std::string slot,int &activeArea);
	int isMaster (std::string ip);
	bool getFwmActionResult(int&);

	/**********************************/
	//Base structure

	//NOTE !!!
	//SNMP methods are developed without limits!
	//in order to "re-use" the code it's needed
	//to use an "hard coded" number of interfaces

//	#define MAX_NO_OF_INTERFACE 35
	/**********************************/

	//Defined for porting !
	typedef unsigned long long ULONGLONG;



	std::string  m_ifAlias[MAX_NO_OF_INTERFACE];
	long  m_ifMTU[MAX_NO_OF_INTERFACE];
	std::string  m_ifLastChange[MAX_NO_OF_INTERFACE];
	std::string  m_ifphysaddress[MAX_NO_OF_INTERFACE];
	unsigned long  m_ifHighSpeed[MAX_NO_OF_INTERFACE];
	std::string  m_ifCounterDiscontinuityTime[MAX_NO_OF_INTERFACE];
	std::string  m_ifOperStatus[MAX_NO_OF_INTERFACE];
	std::string  m_dot3StatsDuplexStatus[MAX_NO_OF_INTERFACE];
	ULONGLONG    m_ifHCInOctets[MAX_NO_OF_INTERFACE];
	ULONGLONG    m_ifHCInUcastPkts[MAX_NO_OF_INTERFACE];
	ULONGLONG    m_ifHCInMulticastPkts[MAX_NO_OF_INTERFACE];
	ULONGLONG    m_ifHCInBroadcastPkts[MAX_NO_OF_INTERFACE];
	ULONGLONG    m_ifHCOutOctets[MAX_NO_OF_INTERFACE];
	ULONGLONG    m_ifHCOutUcastPkts[MAX_NO_OF_INTERFACE];
	ULONGLONG    m_ifHCOutMulticastPkts[MAX_NO_OF_INTERFACE];
	ULONGLONG    m_ifHCOutBroadcastPkts[MAX_NO_OF_INTERFACE];
	ULONGLONG	  m_ifHCInPkts[MAX_NO_OF_INTERFACE];
	ULONGLONG	  m_ifHCOutPkts[MAX_NO_OF_INTERFACE];
	unsigned long    m_ifInErrors[MAX_NO_OF_INTERFACE];
	unsigned long    m_ifOutErrors[MAX_NO_OF_INTERFACE];
	unsigned long    m_ifOutDiscards[MAX_NO_OF_INTERFACE];
	bool	m_snmpRequestResult;
	unsigned long    m_ifInDiscards[MAX_NO_OF_INTERFACE];
	unsigned long    m_ifInUnknownProtos[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCStatsAlignmentErrors[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCStatsFCSErrors[MAX_NO_OF_INTERFACE];
	unsigned long m_dot3StatsSingleCollisionFrames[MAX_NO_OF_INTERFACE];
	unsigned long m_dot3StatsMultipleCollisionFrames[MAX_NO_OF_INTERFACE];
	unsigned long m_dot3StatsSQETestErrors[MAX_NO_OF_INTERFACE];
	unsigned long m_dot3StatsDeferredTransmissions[MAX_NO_OF_INTERFACE];
	unsigned long m_dot3StatsLateCollisions[MAX_NO_OF_INTERFACE];
	unsigned long m_dot3StatsExcessiveCollisions[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCStatsInternalMacTransmitErrors[MAX_NO_OF_INTERFACE];
	unsigned long m_dot3StatsCarrierSenseErrors[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCStatsFrameTooLongs[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCStatsInternalMacReceiveErrors[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCStatsSymbolErrors[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCControlInUnknownOpcodes[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCInPauseFrames[MAX_NO_OF_INTERFACE];
	ULONGLONG m_dot3HCOutPauseFrames[MAX_NO_OF_INTERFACE];
        long  m_ifType[MAX_NO_OF_INTERFACE];
	int interface_no;
private:

        //SNMP Private
        std::string ipAddr;
	std::string cmd;

        int snmpBulk( std::vector<std::string> &outVector,
        		const char* community, const char* peername,
        		const char* usrOID, const int &maxRep,
        		const bool &isHexString = false);

        int snmpGet(std::string &outVal,
        		const char* community, const char* peername,
        		const char* usrOID, int type, int strType = 0);

        int snmpSet(const char* community, const char* peername, const char* usrOID,
        		char type, const char *value);

        int snmpGet(const char* community, const char* peername,const char* usrOID, int type, void* out, unsigned int iBufSize, int strType = 0);


        void initArray();
	void convertIpAddres(uint32_t address, std::string &ip);


        //NetConf Private
        bool m_ncSessionOpen;
        acs_nclib_session m_ncSession;
        std::vector<dmx::shelf> m_shelfList;
	map<std::string,vector<std::string> > m_portList;

        bool startDMXConnection();
	bool openNetconfSession(uint32_t ipAddr, int port, const char* user, const char* password);

        bool netconfGet(std::string &filter_content, std::string &data);

        bool netconfParse(std::string& data, fixs_cch_netconfparser* parser);

        int shelfAddrToPhysicalAddr(const char* shelfaddr);
        std::string physicalAddrToShelfAddr(int phyAddr);
	void changeIpAddress();
	void setudpcmd(std::string temp_cmd){ cmd = temp_cmd; };
	int getShelfId(std::string physical_Address,std::string &ShelfID);
	int executeUDP(std::string &data);
	int getIfNumberUDP(std::string &shelfid,std::string & slot, int &no_of_interface);
	int getIfOperStatusUDP(std::string &shelfid,std::string & slot);
	int getIfHCOutMulticastPktsUDP(std::string &shelfid,std::string & slot);
	int getIfHCOutBroadcastPktsUDP(std::string &shelfid,std::string & slot);
	int getIfHCInMulticastPktsUDP(std::string &shelfid,std::string & slot);
	int getIfHCInBroadcastPktsUDP(std::string &shelfid,std::string & slot);
	int getIfInOctetsUDP(std::string &shelfid,std::string & slot);
	int getIfOutOctetsUDP(std::string &shelfid,std::string & slot);
	int getIfOutErrorsUDP(std::string &shelfid,std::string & slot);
	int getIfInErrorsUDP(std::string &shelfid,std::string & slot);
	int getIfOutDiscardsUDP(std::string &shelfid,std::string & slot);
	int getIfInDiscardsUDP(std::string &shelfid,std::string & slot);
	int getIfHCOutUcastPktsUDP(std::string &shelfid,std::string & slot);
	int getIfHCInUcastPktsUDP(std::string &shelfid,std::string & slot);

	//bool setActiveArea (unsigned int slot, std::string valueToSet);
	uint32_t _dmxc_addresses[2];

};
#endif /* SNMPMANAGER_H_ */
