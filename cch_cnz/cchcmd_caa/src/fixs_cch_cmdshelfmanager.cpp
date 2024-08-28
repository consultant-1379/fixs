#include "fixs_cch_cmdshelfmanager.h"
#include "ACS_TRA_trace.h"
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/XMLChar.hpp>
#include "xcountls_Util.h"
#include <iomanip>
#include <sstream>
#include "ACS_CS_API.h"
#include "cmd_Utils.h"
#include "Client_Server_Interface.h"

/************************************************************/
/*					DMX STATIC VALUES						*/
/************************************************************/
//#define DMX_ADDRESS_A 	"192.254.15.1"
//#define DMX_ADDRESS_B 	"192.254.16.1"
#define DMX_PORT 		831
//#define DMX_USER 		"root"
//#define DMX_PWD 		"tre,14"
#define DMX_ACTION_NS	"urn:com:ericsson:ecim:1.0"

#define DMX_DEFAULT_TIMEOUT_MS	20000


/************************************************************/
/*							OID								*/
/************************************************************/

//********
// IF-MIB
//********
//IF Table
std::string oidIfNumber =	   				"1.3.6.1.2.1.2.1";

std::string oidIfMTU =                      "1.3.6.1.2.1.2.2.1.4"; //Integer
std::string oidIfphysaddress =              "1.3.6.1.2.1.2.2.1.6"; //PhysAddress ??
std::string oidIfLastChange =               "1.3.6.1.2.1.2.2.1.9"; //TimeTicks
std::string oidIfOperStatus = 				"1.3.6.1.2.1.2.2.1.8"; //Integer
std::string oidIfInDiscards =               "1.3.6.1.2.1.2.2.1.13"; //Counter32
std::string oidIfInErrors =                 "1.3.6.1.2.1.2.2.1.14"; //Counter32
std::string oidIfInUnknownProtos =          "1.3.6.1.2.1.2.2.1.15"; //Counter32
std::string oidIfOutDiscards =              "1.3.6.1.2.1.2.2.1.19"; //Counter32
std::string oidIfOutErrors =                "1.3.6.1.2.1.2.2.1.20"; //Counter32
std::string oidIfType =                     "1.3.6.1.2.1.2.2.1.3";

//IFX Table
std::string oidIfHCInOctets =               "1.3.6.1.2.1.31.1.1.1.6"; //Counter64
std::string oidIfHCInUcastPkts =            "1.3.6.1.2.1.31.1.1.1.7"; //Counter64
std::string oidIfHCInMulticastPkts =        "1.3.6.1.2.1.31.1.1.1.8"; //Counter64
std::string oidIfHCInBroadcastPkts =        "1.3.6.1.2.1.31.1.1.1.9"; //Counter64
std::string oidIfHCOutOctets =              "1.3.6.1.2.1.31.1.1.1.10"; //Counter64
std::string oidIfHCOutUcastPkts =           "1.3.6.1.2.1.31.1.1.1.11"; //Counter64
std::string oidIfHCOutMulticastPkts =       "1.3.6.1.2.1.31.1.1.1.12"; //Counter64
std::string oidIfHCOutBroadcastPkts =       "1.3.6.1.2.1.31.1.1.1.13"; //Counter64
std::string oidIfHighSpeed =                "1.3.6.1.2.1.31.1.1.1.15"; //GAUGE32
std::string oidIfAlias =                    "1.3.6.1.2.1.31.1.1.1.18"; //String
std::string oidIfCounterDiscontinuityTime = "1.3.6.1.2.1.31.1.1.1.19"; //TimeStamp (TIMETICKS)
std::string oidIfName =			    "1.3.6.1.2.1.31.1.1.1.1"; //String

//*************
//EtherLike-MIB
//*************

//Dot3HCStatsTable
std::string oidDot3HCStatsAlignmentErrors =				"1.3.6.1.2.1.10.7.11.1.1"; //Counter64
std::string oidDot3HCStatsFCSErrors =					"1.3.6.1.2.1.10.7.11.1.2"; //Counter64
std::string oidDot3HCStatsInternalMacTransmitErrors =	"1.3.6.1.2.1.10.7.11.1.3"; //Counter64
std::string oidDot3HCStatsFrameTooLongs =				"1.3.6.1.2.1.10.7.11.1.4"; //Counter64
std::string oidDot3HCStatsInternalMacReceiveErrors =	"1.3.6.1.2.1.10.7.11.1.5"; //Counter64
std::string oidDot3HCStatsSymbolErrors =				"1.3.6.1.2.1.10.7.11.1.6"; //Counter64

//Dot3StatsTable
std::string oidDot3StatsSingleCollisionFrames =			"1.3.6.1.2.1.10.7.2.1.4"; //Counter32
std::string oidDot3StatsMultipleCollisionFrames =		"1.3.6.1.2.1.10.7.2.1.5"; //Counter32
std::string oidDot3StatsSQETestErrors =					"1.3.6.1.2.1.10.7.2.1.6"; //Counter32
std::string oidDot3StatsDeferredTransmissions =			"1.3.6.1.2.1.10.7.2.1.7"; //Counter32
std::string oidDot3StatsLateCollisions =				"1.3.6.1.2.1.10.7.2.1.8"; //Counter32
std::string oidDot3StatsExcessiveCollisions =			"1.3.6.1.2.1.10.7.2.1.9"; //Counter32
std::string oidDot3StatsCarrierSenseErrors =			"1.3.6.1.2.1.10.7.2.1.11"; //Counter32
std::string oidDot3StatsDuplexStatus =      			"1.3.6.1.2.1.10.7.2.1.19"; //INTEGER

//Dot3ControlTable
std::string oidDot3HCControlInUnknownOpcodes =			"1.3.6.1.2.1.10.7.9.1.3"; //Counter64

//Dot3PauseTable
std::string oidDot3HCInPauseFrames =					"1.3.6.1.2.1.10.7.10.1.5"; //Counter64
std::string oidDot3HCOutPauseFrames =					"1.3.6.1.2.1.10.7.10.1.6"; //Counter64

//FWM LOADED TABLE
std::string oidFwmLoadedAttributeSCX =      "1.3.6.1.4.1.193.177.2.2.3.2.2.1.4";
std::string oidFwmLoadedTypeSCX =           "1.3.6.1.4.1.193.177.2.2.3.2.2.1.2";
std::string oidFwmLoadedVersionSCX =        "1.3.6.1.4.1.193.177.2.2.3.2.2.1.3";
std::string oidbmBladeProductRevisionState = "1.3.6.1.4.1.193.177.2.2.2.2.1.3";

std::string oidBladeBIOSPointer = "1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.23";
std::string oidShelfMgrCtrlSCX =  "1.3.6.1.4.1.193.177.2.2.1.2.1.8";
std::string oidFwmActionResult =  "1.3.6.1.4.1.193.177.2.2.3.3.9.0"; //GEN-SWM-MIB::swmActions.9.0 noerror(0),inProgress(1),faultCode(>=2)

/************************************************************/
/*							OID								*/
/************************************************************/
ACS_TRA_trace _trace = ACS_TRA_trace("fixs_cch_cmdshelfmanager");

class TraceInOut
{
public:
	TraceInOut(const char * function) : m_function(function)
	{
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] In", m_function.c_str());
			_trace.ACS_TRA_event(1, tmpStr);
		}
	};

	~TraceInOut()
	{
		if (_trace.ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Out", m_function.c_str());
			_trace.ACS_TRA_event(1, tmpStr);
		}
	}
private:
	std::string m_function;
};

fixs_cch_cmdshelfmanager::fixs_cch_cmdshelfmanager(std::string ipAddress):
		m_ncSession(acs_nclib::TCP){
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************
	ipAddr = ipAddress;
	m_snmpRequestResult = false;

	m_ncSessionOpen = false;

	acs_nclib_library::init();
	ACS_CS_API_NetworkElement::getDmxcAddress(_dmxc_addresses[0],_dmxc_addresses[1]);

	init_snmp("SnmpManager");

	initArray();
}

fixs_cch_cmdshelfmanager::~fixs_cch_cmdshelfmanager(){

	//std::cout << __FUNCTION__ << "@" << __LINE__ << " ~fixs_cch_cmdshelfmanager " << std::endl;
	acs_nclib_library::exit();
}

void fixs_cch_cmdshelfmanager::initArray() {

	//****************DEBUG**************************
//	std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	for(int i=0; i<MAX_NO_OF_INTERFACE;i++)
	{
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Value of i : " <<i<< std::endl;
		m_ifAlias[i]="-";
		m_ifMTU[i]=-1;
	        m_ifType[i] = -1;
		m_ifLastChange[i]="-";
		m_ifphysaddress[i]="-";
		m_ifHighSpeed[i]= ULONG_MAX;
		m_ifCounterDiscontinuityTime[i]="-";
		m_ifOperStatus[i]="-";
		m_dot3StatsDuplexStatus[i]="-";
		m_ifHCInOctets[i]= ULLONG_MAX;
		m_ifHCInUcastPkts[i]= ULLONG_MAX;
		m_ifHCInMulticastPkts[i]= ULLONG_MAX;
		m_ifHCInBroadcastPkts[i]= ULLONG_MAX;
		m_ifHCOutOctets[i]= ULLONG_MAX;
		m_ifHCOutUcastPkts[i]=ULLONG_MAX;
		m_ifHCOutMulticastPkts[i]= ULLONG_MAX;
		m_ifHCOutBroadcastPkts[i]= ULLONG_MAX;
		m_ifHCInPkts[i]= ULLONG_MAX;
		m_ifHCOutPkts[i]= ULLONG_MAX;
		m_ifInErrors[i]= ULONG_MAX;
		m_ifOutErrors[i]= ULONG_MAX;
		m_ifOutDiscards[i]= ULONG_MAX;
		m_ifInDiscards[i]= ULONG_MAX;
		m_ifInUnknownProtos[i] = ULONG_MAX;
		m_dot3HCStatsAlignmentErrors[i]= ULLONG_MAX;
		m_dot3HCStatsFCSErrors[i]= ULLONG_MAX;
		m_dot3StatsSingleCollisionFrames[i] = ULONG_MAX;
		m_dot3StatsMultipleCollisionFrames[i]= ULONG_MAX;
		m_dot3StatsSQETestErrors[i]= ULONG_MAX;
		m_dot3StatsDeferredTransmissions[i]= ULONG_MAX;
		m_dot3StatsLateCollisions[i]= ULONG_MAX;
		m_dot3StatsExcessiveCollisions[i]= ULONG_MAX;
		m_dot3HCStatsInternalMacTransmitErrors[i]= ULLONG_MAX;
		m_dot3StatsCarrierSenseErrors[i]= ULONG_MAX;
		m_dot3HCStatsFrameTooLongs[i]= ULLONG_MAX;
		m_dot3HCStatsInternalMacReceiveErrors[i]= ULLONG_MAX;
		m_dot3HCStatsSymbolErrors[i]= ULLONG_MAX;
		m_dot3HCControlInUnknownOpcodes[i]=ULLONG_MAX;
		m_dot3HCInPauseFrames[i]=ULLONG_MAX;
		m_dot3HCOutPauseFrames[i]= ULLONG_MAX;
	}
}


std::string fixs_cch_cmdshelfmanager::getIpAddress(void)
{
	return std::string(ipAddr);
}

void fixs_cch_cmdshelfmanager::setIpAddress(std::string ip)
{
	ipAddr = ip;
}


//**********************************************************************************
/*
 *
 * Generic GET-BULK
 *
*/
//**********************************************************************************
int fixs_cch_cmdshelfmanager::snmpBulk( std::vector<std::string> &outVector,
		const char* community, const char* peername,
		const char* usrOID, const int &maxRep,
		const bool &isHexString ) {

	int res;
	snmp_session snmpSession;
	snmp_session *snmpSessionHandler;

	int getResult = GETSET_SNMP_OK;
	struct snmp_pdu *snmpPDU;
	struct snmp_pdu *snmpResp=NULL;

	struct variable_list *varsList;

	snmp_sess_init( &snmpSession );

	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = 5;
	snmpSession.timeout = 1000000;
	snmpSession.community_len = strlen((const char *)snmpSession.community);
	snmpSession.peername = strdup(peername);

	snmpSessionHandler = snmp_open(&snmpSession);

	if(snmpSessionHandler == NULL){
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Impossible to open an SNMP session: " << snmp_errno << std::endl;
		return OPEN_SESS_ERROR;
	}

	oid snmp_oid[MAX_OID_LEN];
	size_t snmp_oid_len = MAX_OID_LEN;

	if (! read_objid(usrOID, snmp_oid, &snmp_oid_len) ) {

		//std::cout << __FUNCTION__ << "@" << __LINE__ << " OID transformation failed " <<std::endl;
		snmp_close(snmpSessionHandler);
		return GETSET_SNMP_PARSE_OID_ERROR;
	}

	//Create a GetBulk PDU
	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GETBULK)) != NULL) {

		snmpPDU->non_repeaters = 0;
		snmpPDU->max_repetitions = maxRep;
		snmp_add_null_var(snmpPDU, snmp_oid, snmp_oid_len);

		res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);

		switch(res){
			case STAT_SUCCESS:
				if (snmpResp->errstat == SNMP_ERR_NOERROR) {

					//int vbCount = 0; //dummy counter !
					for (varsList = snmpResp->variables ; varsList ; varsList = varsList->next_variable) {

						//std::cout << (++vbCount) << " -------------------------------------------------------------" << std::endl;
						//print_variable(varsList->name, varsList->name_length, varsList);

						if (varsList->val_len != 0) {

							char  valBuf[SNMP_MAXBUF];
							enum_list *enList = NULL;

							switch (varsList->type) {
								case ASN_INTEGER:{
									snprintf(valBuf, varsList->val_len + 1, "%d", (int) (*varsList->val.integer));
									//std::cout << __FUNCTION__ << "@" << __LINE__ << " ->" << valBuf << "<-" << std::endl;
								}
								break;

								case ASN_COUNTER: {
									snprint_counter(valBuf, SNMP_MAXBUF, varsList, enList, NULL, NULL);
									//std::cout << __FUNCTION__ << "@" << __LINE__ << " ->" << valBuf << "<-" << std::endl;
								}
								break;

								case ASN_COUNTER64: {
									snprint_counter64(valBuf, SNMP_MAXBUF, varsList, enList, NULL, NULL);
									//std::cout << __FUNCTION__ << "@" << __LINE__ << " ->" << valBuf << "<-" << std::endl;
								}
								break;

								case ASN_GAUGE:{
									snprint_gauge(valBuf, SNMP_MAXBUF, varsList, enList, NULL, NULL);
									//std::cout << __FUNCTION__ << "@" << __LINE__ << " ->" << valBuf << "<-" << std::endl;
								}
								break;

								case ASN_TIMETICKS: {
									snprint_timeticks(valBuf, SNMP_MAXBUF, varsList, enList, NULL, NULL);
									//std::cout << __FUNCTION__ << "@" << __LINE__ << " ->" << valBuf << "<-" << std::endl;
								}
								break;
								case ASN_OCTET_STR: {
									if (isHexString) {
										snprint_hexstring( valBuf, SNMP_MAXBUF, varsList->val.string, varsList->val_len);
										//std::cout << __FUNCTION__ << "@" << __LINE__ << " hex ->" << valBuf << "<-" << std::endl;
									} else {
										snprint_asciistring( valBuf, SNMP_MAXBUF, varsList->val.string, varsList->val_len);
										//std::cout << __FUNCTION__ << "@" << __LINE__ << " ascii ->" << valBuf << "<-" << std::endl;
									}
								}
								break;
							}

							std::string strBuf(valBuf);

							//NET SNMP ADD A TOKEN ! ... it's needed to remove it
							//E.g. IF-MIB::ifHCInOctets.31 = Counter64: 215269719
							size_t pos = strBuf.find(':');
							if ( pos != std::string::npos) {
								strBuf = strBuf.substr(pos + 2, strBuf.length() - (pos + 2) );
								//std::cout << " str: |" << strBuf.c_str() << "|" << std::endl;
								outVector.push_back(strBuf);
							}
							else {
								//std::cout << " No ':' found pos= " << pos << " - str= " << strBuf << std::endl;
								outVector.push_back(strBuf);
							}
						} //if (varsList->val_len != 0) {
					} //for (varsList = snmpResp->variables ; varsList ; varsList = varsList->next_variable)
				}
				else //if (snmpResp->errstat == SNMP_ERR_NOERROR)
				{
					//std::cout << __FUNCTION__ << "@" << __LINE__ << " GET-Response Error: " << snmp_errstring(snmpResp->errstat) << std::endl;
					outVector.clear();
					getResult =  GETSET_SNMP_RESPONSE_ERROR;
				}

				break;

			case STAT_TIMEOUT:
				//std::cout << __FUNCTION__ << "@" << __LINE__ << " STAT_TIMEOUT - Error: " << snmp_errstring(snmp_errno) << std::endl;

				outVector.clear();
				getResult = GETSET_SNMP_STATUS_TIMEOUT;
				break;

			case STAT_ERROR:
				//std::cout << __FUNCTION__ << "@" << __LINE__ << " STAT_ERROR - Error: " << snmp_errstring(snmp_errno) << std::endl;

				outVector.clear();
				getResult = GETSET_SNMP_STATUS_ERROR;
				break;
		}
	}
	else //if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GET)) != NULL)
	{
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: " << snmp_errno << std::endl;
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: " << snmp_errstring(snmp_errno) << std::endl;

		getResult = GETSET_SNMP_PDU_CREATE_ERROR;
	}

	if(snmpResp)
	{
		snmp_free_pdu(snmpResp);
	}

	snmp_close(snmpSessionHandler);

	return getResult;
}

int fixs_cch_cmdshelfmanager::snmpGet(std::string &outVal,
									const char* community, const char* peername,
									const char* usrOID, int type, int strType){

	int res;
	snmp_session snmpSession;
	snmp_session *snmpSessionHandler;

	int getResult = GETSET_SNMP_OK;
	struct snmp_pdu *snmpPDU;
	struct snmp_pdu *snmpResp=NULL;

	struct variable_list *varsList;

	snmp_sess_init( &snmpSession );

	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = 5;
	snmpSession.timeout = 1000000;
	snmpSession.community_len = strlen((const char *)snmpSession.community);
	snmpSession.peername = strdup(peername);

	snmpSessionHandler = snmp_open(&snmpSession);

	if(snmpSessionHandler == NULL){
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " snmp_open failed - snmp_errno: " << snmp_errno << std::endl;
		return OPEN_SESS_ERROR;
	}

	oid snmp_oid[MAX_OID_LEN];
	size_t snmp_oid_len = MAX_OID_LEN;

	if (! read_objid(usrOID, snmp_oid, &snmp_oid_len) ) {

		snmp_close(snmpSessionHandler);
		return GETSET_SNMP_PARSE_OID_ERROR;
	}

	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GET)) != NULL) {

		snmp_add_null_var(snmpPDU, snmp_oid, snmp_oid_len);

		//Send the get !
		res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);

		switch(res){
			case STAT_SUCCESS:

				if (snmpResp->errstat == SNMP_ERR_NOERROR) {

					for (varsList = snmpResp->variables ; varsList ; varsList = varsList->next_variable) {

						if (varsList->val_len == 0) {
							outVal = "";
							getResult = GETSET_SNMP_OK_LEN_ZERO;
							break;
						}

						// return response as string
						if ((varsList->type == type) && (type == ASN_OCTET_STR))
						{
							//print_variable(varsList->name, varsList->name_length, varsList);

							switch (strType) {

								case fixs_cch_cmdshelfmanager::SNMPGET_STD_STRING: {
									char  Buf[SNMP_MAXBUF];
									snprintf(Buf, varsList->val_len + 1, "%s", varsList->val.string);
									outVal = std::string(Buf);
									break;
								}

								case fixs_cch_cmdshelfmanager::SNMPGET_HEX_STRING: {
									char  hexBuf[SNMP_MAXBUF];
									snprint_hexstring( hexBuf, SNMP_MAXBUF, varsList->val.string, varsList->val_len);
									outVal = std::string(hexBuf);
									break;
								}

								case fixs_cch_cmdshelfmanager::SNMPGET_ASCII_STRING: {
									char  hexBuf[SNMP_MAXBUF];
									snprint_asciistring( hexBuf, SNMP_MAXBUF, varsList->val.string, varsList->val_len);
									outVal = std::string(hexBuf);
									break;
								}
							}
							break;
						}
						else if ((varsList->type == type) && (type == ASN_INTEGER )) {

							char  Buf[SNMP_MAXBUF];
							snprintf(Buf, varsList->val_len + 1, "%d", (int) (*varsList->val.integer));
							outVal = std::string(Buf);
							break;
						}
						else if ((varsList->type == type) && (type == ASN_GAUGE )) { //as query is send as gauge32

							char  Buf[SNMP_MAXBUF];
							snprintf(Buf, varsList->val_len + 1, "%d", (int) (*varsList->val.integer));
							outVal = std::string(Buf);
							break;
						}
						else {
						break;
							//UNMANAGED TYPE !!
						}
					} //for (varsList = snmpResp->variables ; varsList ; varsList = varsList->next_variable)
				}
				else //if (snmpResp->errstat == SNMP_ERR_NOERROR)
				{
					//std::cout << __FUNCTION__ << "@" << __LINE__ << " GET-Response Error: " << snmp_errstring(snmpResp->errstat) << std::endl;
					outVal = "";
					getResult =  GETSET_SNMP_RESPONSE_ERROR;
				}
				break; //EXIT FOR !

			case STAT_TIMEOUT:
				//std::cout << __FUNCTION__ << "@" << __LINE__ << " STAT_TIMEOUT " << std::endl;
				//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: " << snmp_errstring(snmp_errno) << std::endl;

				outVal = "";
				getResult = GETSET_SNMP_STATUS_TIMEOUT;
				break;

			case STAT_ERROR:
				//std::cout << __FUNCTION__ << "@" << __LINE__ << " STAT_ERROR " << std::endl;
				//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: " << snmp_errstring(snmp_errno) << std::endl;

				outVal = "";
				getResult = GETSET_SNMP_STATUS_ERROR;
				break;
		}
	}
	else //if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GET)) != NULL)
	{
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: " << snmp_errno << std::endl;
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: " << snmp_errstring(snmp_errno) << std::endl;

		getResult = GETSET_SNMP_PDU_CREATE_ERROR;
	}

	if(snmpResp)
	{
		snmp_free_pdu(snmpResp);
	}

	snmp_close(snmpSessionHandler);

	return getResult;
}

int fixs_cch_cmdshelfmanager::snmpSet(const char* community, const char* peername,
		const char* usrOID,  char type, const char *value)
{
	/*
		TYPE: one of i, u, t, a, o, s, x, d, b
			i: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS
			o: OBJID, s: STRING, x: HEX STRING, d: DECIMAL STRING, b: BITS
			U: unsigned int64, I: signed int64, F: float, D: double
	*/

	snmp_session snmpSession;
	snmp_session *snmpSessionHandler;

	int setResult = GETSET_SNMP_OK;
	struct snmp_pdu *snmpPDU;
	struct snmp_pdu *snmpResp=NULL;
	int res;

	snmp_sess_init( &snmpSession );
	std::cout << __FUNCTION__ << "@" << __LINE__ <<
			" init snmp session ok" <<std::endl;

	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = 5;
	snmpSession.timeout = 1000000;
	snmpSession.community_len = strlen((const char *)snmpSession.community);
	snmpSession.peername = strdup(peername);

	std::cout << __FUNCTION__ << "@" << __LINE__ <<
			" init snmp session ok ... IP: " << snmpSession.peername <<std::endl;

	snmpSessionHandler = snmp_open(&snmpSession);

	if(snmpSessionHandler == NULL){
		std::cout << __FUNCTION__ << "@" << __LINE__ <<
				" snmp_open failed - snmp_errno: " << snmp_errno << std::endl;

		return OPEN_SESS_ERROR;
	}

	//snmpPDU = snmp_pdu_create(SNMP_MSG_SET);

	oid snmp_oid[MAX_OID_LEN];
	size_t snmp_oid_len = MAX_OID_LEN;

	if (! read_objid(usrOID, snmp_oid, &snmp_oid_len) ) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "
					<< snmp_errstring(snmp_errno) << std::endl;

		snmp_close(snmpSessionHandler);
		return GETSET_SNMP_PARSE_OID_ERROR;
	}

	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_SET)) != NULL) {

		if (snmp_add_var(snmpPDU, snmp_oid, snmp_oid_len, type, value) ) {

//			std::cout << __FUNCTION__ << "@" << __LINE__ << " snmp_add_var error: "
//				<< snmp_errstring(snmpResp->errstat) << std::endl;

			snmp_close(snmpSessionHandler);
			return GETSET_SNMP_PARSE_OID_ERROR;
		}

		// send pdu...
		res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);

		switch(res){
			case STAT_SUCCESS:
				std::cout << __FUNCTION__ << "@" << __LINE__ <<
					" STAT_SUCCESS " << std::endl;

				//Check the response
				if (snmpResp->errstat == SNMP_ERR_NOERROR) {
					// SET successful done ;) !!
					//snmp_free_pdu(snmpResp);
				}
				else {
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SET-Response error: "
							<< snmp_errstring(snmpResp->errstat) << std::endl;

					//snmp_free_pdu(snmpResp);
					setResult =  GETSET_SNMP_RESPONSE_ERROR;
				}

				break;

			case STAT_TIMEOUT:
				std::cout << __FUNCTION__ << "@" << __LINE__ <<
					" STAT_TIMEOUT " << std::endl;

				std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "
						<< snmp_errstring(snmp_errno) << std::endl;

				setResult = GETSET_SNMP_STATUS_TIMEOUT;
				//snmp_free_pdu(snmpResp);
				break;

			case STAT_ERROR:
				std::cout << __FUNCTION__ << "@" << __LINE__ <<
					" STAT_ERROR " << std::endl;

				std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "
										<< snmp_errstring(snmp_errno) << std::endl;

				//snmp_free_pdu(snmpResp);
				setResult = GETSET_SNMP_STATUS_ERROR;
				break;

		} //switch(res)
	}
	else //if ((snmpPDU = snmp_pdu_create(SNMP_MSG_SET)) != NULL)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation num: "
				<< snmp_errno << std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: "
				<< snmp_errstring(snmp_errno) << std::endl;

		setResult = GETSET_SNMP_PDU_CREATE_ERROR;
	}

	if(snmpResp)
	{
		snmp_free_pdu(snmpResp);
	}

	snmp_close(snmpSessionHandler);

	return setResult;
}

bool fixs_cch_cmdshelfmanager::testget() {
	int res;
	//void *out = NULL;
	std::string out;
	//std::string tmpOID = "1.3.6.1.4.1.193.177.2.2.1.2.1.8.0"; //HEX
	//std::string tmpOID = "1.3.6.1.4.1.193.177.2.2.6.2.1.1.1.3.0"; //ASCII
	std::string tmpOID = "1.3.6.1.4.1.193.177.2.2.2.2.1.6.0"; //STR (standard)
	//res = snmpGet(out, "PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, fixs_cch_cmdshelfmanager::SNMPGET_HEX_STRING);
	//res = snmpGet(out, "PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, fixs_cch_cmdshelfmanager::SNMPGET_ASCII_STRING);
	res = snmpGet(out, "PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, fixs_cch_cmdshelfmanager::SNMPGET_STD_STRING);
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " out: " << out << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}


bool fixs_cch_cmdshelfmanager::testBulk (const unsigned int &index,
										const std::string &OID,
										const bool &isHexString)
{
	std::stringstream ss;
	ss << index;
	//std::string tmpOID = "1.3.6.1.2.1.31.1.1.1.6"; // oidSwmLoadedVersion + "." +  ss.str();

	int res;
	std::vector<std::string> out;
	res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), OID.c_str(), index, isHexString);
	//cout << "RES = " << res << std::endl;
	//char cx;
	//cout << "Premi un tasto....." << std::endl;
	//std::cin >> cx;

	for (unsigned int i = 0 ; i < out.size() ; i++) {
		std::cout << i << " - SNMP -> " << out[i].c_str() << std::endl;
	}

	//todo ... use:
	//unsigned long long ull1 = atoll(strBuf.c_str());
	//to convert in long !

	return true;
}


bool fixs_cch_cmdshelfmanager::getIfNumber(int &interface) {
	int res;
	std::string out;
	std::string tmpOID = oidIfNumber + ".0";
	res = snmpGet(out, "PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER);
	if (res == GETSET_SNMP_OK) {
		interface = atoi(out.c_str());
	}
	else {
		interface = -1;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfAlias (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfAlias.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {
			m_ifAlias[i] = out[i].c_str();
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifAlias[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfMTU (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfMTU.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {
			m_ifMTU[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifMTU[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfType (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfType.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {
			m_ifType[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifMTU[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}
bool fixs_cch_cmdshelfmanager::getBulkIfLastChange (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfLastChange.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			//Format provided by NET-SNMP:  (103080075) 11 days, 22:20:00.75
			//It's needed to remove this part -> (103080075)

			size_t pos = out[i].find(')');
			if ( pos != std::string::npos) {
				m_ifLastChange[i] = out[i].substr(pos + 2, out[i].length() - (pos + 2) );
			}
			else {
				m_ifLastChange[i] = out[i];
			}
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifLastChange[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfphysaddress (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfphysaddress.c_str(), no_of_interface, true);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {
			m_ifphysaddress[i] = out[i].c_str();
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifphysaddress[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfHighSpeed (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHighSpeed.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {
			m_ifHighSpeed[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHighSpeed[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfCounterDiscontinuityTime (const int &no_of_interface){
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfCounterDiscontinuityTime.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {
			//Format provided by NET-SNMP:  (103080075) 11 days, 22:20:00.75
			//It's needed to remove this part -> (103080075)

			size_t pos = out[i].find(')');
			if ( pos != std::string::npos) {
				m_ifCounterDiscontinuityTime[i] = out[i].substr(pos + 2, out[i].length() - (pos + 2) );
			}
			else {
				m_ifCounterDiscontinuityTime[i] = out[i];
			}
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifCounterDiscontinuityTime[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfOperStatus (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfOperStatus.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			int tempVal = atoi(out[i].c_str());
			switch (tempVal) {
				case 1: m_ifOperStatus[i] ="UP"; break;
				case 2: m_ifOperStatus[i] ="DOWN"; break;
				case 3: m_ifOperStatus[i] ="TESTING"; break;
				case 5: m_ifOperStatus[i] ="DORMANT"; break;
				case 6: m_ifOperStatus[i] ="NOTPRESENT"; break;
				case 7: m_ifOperStatus[i] ="LOWLAYDOWN"; break;
				default: m_ifOperStatus[i] ="UNKNOWN"; break;
			}
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifOperStatus[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3StatsDuplexStatus (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3StatsDuplexStatus.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			int tempVal = atoi(out[i].c_str());
			switch (tempVal) {
				case 2: m_dot3StatsDuplexStatus[i] ="HALFDUPLEX"; break;
				case 3: m_dot3StatsDuplexStatus[i] ="FULLDUPLEX"; break;
				default: m_dot3StatsDuplexStatus[i] ="UNKNOWN"; break;
			}
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3StatsDuplexStatus[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfHCInOctets (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHCInOctets.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifHCInOctets[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHCInOctets[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}


bool fixs_cch_cmdshelfmanager::getBulkIfHCInUcastPkts (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHCInUcastPkts.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifHCInUcastPkts[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHCInUcastPkts[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfHCInMulticastPkts (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHCInMulticastPkts.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifHCInMulticastPkts[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHCInMulticastPkts[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfHCInBroadcastPkts (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHCInBroadcastPkts.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifHCInBroadcastPkts[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHCInBroadcastPkts[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfHCOutOctets (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHCOutOctets.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifHCOutOctets[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHCOutOctets[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfHCOutUcastPkts (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHCOutUcastPkts.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifHCOutUcastPkts[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHCOutUcastPkts[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfHCOutMulticastPkts (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHCOutMulticastPkts.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifHCOutMulticastPkts[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHCOutMulticastPkts[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfHCOutBroadcastPkts (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfHCOutBroadcastPkts.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifHCOutBroadcastPkts[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifHCOutBroadcastPkts[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfInErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfInErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifInErrors[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifInErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfOutErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfOutErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifOutErrors[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifOutErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfOutDiscards (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfOutDiscards.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifOutDiscards[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifOutDiscards[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfInDiscards (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfInDiscards.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifInDiscards[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifInDiscards[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkIfInUnknownProtos (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfInUnknownProtos.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_ifInUnknownProtos[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifInUnknownProtos[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCStatsAlignmentErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCStatsAlignmentErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCStatsAlignmentErrors[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCStatsAlignmentErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCStatsFCSErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCStatsFCSErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCStatsFCSErrors[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCStatsFCSErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3StatsSingleCollisionFrames (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3StatsSingleCollisionFrames.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3StatsSingleCollisionFrames[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3StatsSingleCollisionFrames[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3StatsMultipleCollisionFrames (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3StatsMultipleCollisionFrames.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3StatsMultipleCollisionFrames[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3StatsMultipleCollisionFrames[i] << std::endl;
//**************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3StatsSQETestErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3StatsSQETestErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3StatsSQETestErrors[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3StatsSQETestErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3StatsDeferredTransmissions (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3StatsDeferredTransmissions.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3StatsDeferredTransmissions[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3StatsDeferredTransmissions[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3StatsLateCollisions (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3StatsLateCollisions.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3StatsLateCollisions[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3StatsLateCollisions[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

int fixs_cch_cmdshelfmanager::snmpGet(const char* community, const char* peername,const char* usrOID, int type, void* out, unsigned int iBufSize, int strType)
{
	int res;
	snmp_session snmpSession;
	//snmp_session *snmpSessionHandler;
	char *pchBuffer;
	int *piBuffer;
	int iReqSize;
	int getResult = GETSET_SNMP_OK;
	struct snmp_pdu *snmpPDU;
	struct snmp_pdu *snmpResp = NULL;
	void *snmpSess;

	struct variable_list *varsList;

	pchBuffer = NULL;
	piBuffer = NULL;
	iReqSize = 0;
	snmp_sess_init( &snmpSession );

	//std::cout << __FUNCTION__ << "@" << __LINE__ <<	" init snmp session ok ... IP: " << peername <<std::endl;

	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = 5;
	//snmpSession.timeout = 1000000;
	snmpSession.timeout = 200000;
	snmpSession.community_len = strlen((const char *)snmpSession.community);
	snmpSession.peername = strdup(peername);

	//std::cout << __FUNCTION__ << "@" << __LINE__ <<	" init snmp session ok ... IP: " << snmpSession.peername <<std::endl;

	//snmpSessionHandler = snmp_open(&snmpSession);
	snmpSess = snmp_sess_open(&snmpSession);

	if(snmpSess == NULL)
	{
		//std::cout << "DBG: " <<__FUNCTION__ << "@" << __LINE__ <<" ERROR: snmp_open failed - snmp_errno: " << snmp_errno << std::endl;

		return OPEN_SESS_ERROR;
	}

	//snmpSessionHandler=snmp_sess_session(snmpSess);


	free(snmpSession.community);
	free(snmpSession.peername);

//	if(snmpSessionHandler == NULL)
//	{
//		std::cout << "DBG: " <<__FUNCTION__ << "@" << __LINE__ <<" ERROR: snmp_sess_session failed - snmp_errno: " << snmp_errno << std::endl;
//
//		return OPEN_SESS_ERROR;
//	}

	oid snmp_oid[MAX_OID_LEN];
	size_t snmp_oid_len = MAX_OID_LEN;

	if (! read_objid(usrOID, snmp_oid, &snmp_oid_len) )
	{
		//std::cout <<"DBG: "<< __FUNCTION__ << "@" << __LINE__ <<" ERROR: Transform OID Failed " <<std::endl;
		//snmp_close(snmpSessionHandler);
		snmp_sess_close(snmpSess);
		return GETSET_SNMP_PARSE_OID_ERROR;
	}

	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GET)) != NULL)
	{
		snmp_add_null_var(snmpPDU, snmp_oid, snmp_oid_len);
		//std::cout <<"DBG: "<< __FUNCTION__ << "@" << __LINE__ <<" Send request ! " <<std::endl;

		//usleep(100000);
		//res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);
		res = snmp_sess_synch_response(snmpSess, snmpPDU, &snmpResp);

		switch(res)
		{
			case STAT_SUCCESS:
				//std::cout <<"DBG: "<< __FUNCTION__ << "@" << __LINE__ <<" STAT_SUCCESS " << std::endl;

				if((out != NULL) && (iBufSize)){
					if (snmpResp->errstat == SNMP_ERR_NOERROR)
					{
						for (varsList = snmpResp->variables ; varsList ; varsList = varsList->next_variable)
						{
	//						std::cout << __FUNCTION__ << "@" << __LINE__ <<" for .... var len: " << varsList->val_len << std::endl <<" " << varsList->name_loc << std::endl;
	//						int z = varsList->type;
	//						std::cout << __FUNCTION__ << "@" << __LINE__ <<
	//							" type: " << z << std::endl;
							if (varsList->val_len == 0)
							{
								//out = NULL;
								getResult = GETSET_SNMP_OK_LEN_ZERO;
								break;
							}

							// return response as string
							if ((varsList->type == type) && (type == ASN_OCTET_STR))
							{
	//							print_variable(varsList->name, varsList->name_length, varsList);
								//cout << "received string" << endl;
								switch (strType)
								{

									case fixs_cch_cmdshelfmanager::SNMPGET_STD_STRING:
									{
										//char  Buf[SNMP_MAXBUF];
										pchBuffer = reinterpret_cast<char*>(out);
										//sprintf(Buf, "%s", varsList->val.string);

										memset(pchBuffer,0,iBufSize);
										memcpy(pchBuffer,varsList->val.string,varsList->val_len);

										iReqSize=0;

										//iReqSize = snprintf(pchBuffer,iBufSize, "%s", varsList->val.string);

										// Check len of required buffer
										if((unsigned int)iReqSize >= iBufSize){
											// Error. Buffer is too small
											//std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: SNMPGET_STD_STRING buffer is too small! " << std::endl;
											getResult = fixs_cch_cmdshelfmanager::GETSET_SNMP_BUFFERSMALL;
										}

										/*out = (void *) pchBuffer;
										std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_STD_STRING " << (char *) out<< std::endl;

										//std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_STD_STRING " << out << std::endl;
										out = (void *) varsList->val.string;
										//out = &varsList->val.string;
										std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_STD_STRING " << (char *) out << std::endl;*/
										break;
									}

									case fixs_cch_cmdshelfmanager::SNMPGET_HEX_STRING:
									{
										//char  hexBuf[SNMP_MAXBUF];
										pchBuffer = reinterpret_cast<char*>(out);
										iReqSize = snprint_hexstring( pchBuffer, iBufSize, varsList->val.string, varsList->val_len);
										// Check error
										if(iReqSize < 0){
											// Error. Is the Buffer too small?
											//std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: SNMPGET_HEX_STRING can't write in the buffer. Is buffer too small?" << std::endl;
											getResult = fixs_cch_cmdshelfmanager::GETSET_SNMP_BUFFERSMALL;
										}
										//out = (void *) hexBuf;
										//std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_HEX_STRING " << pchBuffer << std::endl;
										break;
									}

									case fixs_cch_cmdshelfmanager::SNMPGET_ASCII_STRING:
									{
										pchBuffer = reinterpret_cast<char*>(out);
										iReqSize = snprint_asciistring( pchBuffer, iBufSize, varsList->val.string, varsList->val_len);
										// Check error
										if(iReqSize < 0){
											// Error. Is the Buffer too small?
											//std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: SNMPGET_ASCII_STRING can't write in the buffer. Is buffer too small?" << std::endl;
											getResult = fixs_cch_cmdshelfmanager::GETSET_SNMP_BUFFERSMALL;
										}
										//std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_ASCII_STRING " << pchBuffer << std::endl;
										break;
									}
								}
								/*
								if (! isHexStr) {
									std::cout << __FUNCTION__ << "@" << __LINE__ << " HEX FALSE " << out << std::endl;
									//out = (void *) varsList->val.string;
									out = &varsList->val.string;
								}
								else {
									std::cout << __FUNCTION__ << "@" << __LINE__ << " HEX TRUE " << out << std::endl;
									char  hexBuf[SNMP_MAXBUF];
									//snprint_hexstring( hexBuf, SNMP_MAXBUF, varsList->val.string, varsList->val_len);

									snprint_asciistring( hexBuf, SNMP_MAXBUF, varsList->val.string, varsList->val_len);
									out = (void *) hexBuf;
								}
								*/

								break;
							}
							else if ((varsList->type == type) && (type == ASN_INTEGER))
							{

								/*
								int i = *varsList->val.integer;

								std::cout << __FUNCTION__ << "@" << __LINE__ <<
										" Val intero !: " << i << std::endl;

								out = (void *) i;
								*/
								if(iBufSize >= sizeof(int)){
									piBuffer = reinterpret_cast<int*>(out);
									*piBuffer = (int)(*(varsList->val.integer));
								}else{
									// Error. Is the Buffer too small?
								//	std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: ASN_INTEGER buffer is too small!" << std::endl;
									getResult = fixs_cch_cmdshelfmanager::GETSET_SNMP_BUFFERSMALL;
								}
								//out = (int *) *varsList->val.integer;
								break;
							}
							else if ((varsList->type == type) && (type == ASN_IPADDRESS))
							{
								pchBuffer = reinterpret_cast<char*>(out);
								iReqSize = snprint_ipaddress( pchBuffer, iBufSize, varsList, NULL, NULL, NULL );
								// Check error
								if(iReqSize < 0){
									// Error. Is the Buffer too small?
									//std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: SNMPGET_IPADDRESS can't write in the buffer. Is buffer too small?" << std::endl;
									getResult = fixs_cch_cmdshelfmanager::GETSET_SNMP_BUFFERSMALL;
								}


							}
							else
							{
								//todo
							}

						} //for (varsList = snmpResp->variables ; varsList ; varsList = varsList->next_variable)

						//snmp_free_pdu(snmpResp);
					}
					else //if (snmpResp->errstat == SNMP_ERR_NOERROR)
					{
						//std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: GET-Response Error: "
						//		<< snmp_errstring(snmpResp->errstat) << std::endl;
						//snmp_free_pdu(snmpResp);
						//out = NULL;
						getResult =  GETSET_SNMP_RESPONSE_ERROR;
					}
			}else{	// if((out != NULL) && (iBufSize))
				// Error buffer
				//std::cout << __FUNCTION__ << "@" << __LINE__ <<" ## ERROR: incorrect argument [buffer].  " << std::endl;
				getResult =  GETSET_SNMP_RESPONSE_ERROR;
			}
				break; //EXIT FOR !

			case STAT_TIMEOUT:
				//std::cout << __FUNCTION__ << "@" << __LINE__ <<" STAT_TIMEOUT " << std::endl;
				//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "<< snmp_errstring(snmp_errno) << std::endl;

				//snmp_free_pdu(snmpResp);
				//out = NULL;
				getResult = GETSET_SNMP_STATUS_TIMEOUT;
				break;

			case STAT_ERROR:
				//std::cout << __FUNCTION__ << "@" << __LINE__ <<	" STAT_ERROR " << std::endl;

				//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "	<< snmp_errstring(snmp_errno) << std::endl;

				//snmp_free_pdu(snmpResp);
				//out = NULL;
				getResult = GETSET_SNMP_STATUS_ERROR;
				break;
		}


	}
	else //if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GET)) != NULL)
	{
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: "	<< snmp_errno << std::endl;
		//std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: "	<< snmp_errstring(snmp_errno) << std::endl;

		getResult = GETSET_SNMP_PDU_CREATE_ERROR;
	}

	//std::cout << __FUNCTION__ << "@" << __LINE__ <<	" Close snmp session and exit " << std::endl;

	snmp_free_pdu(snmpResp);

	//snmp_close(snmpSessionHandler);
	snmp_sess_close(snmpSess);

	return getResult;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3StatsExcessiveCollisions (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3StatsExcessiveCollisions.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3StatsExcessiveCollisions[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3StatsExcessiveCollisions[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCStatsInternalMacTransmitErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCStatsInternalMacTransmitErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCStatsInternalMacTransmitErrors[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCStatsInternalMacTransmitErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3StatsCarrierSenseErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3StatsCarrierSenseErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3StatsCarrierSenseErrors[i] = atol(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3StatsCarrierSenseErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCStatsFrameTooLongs (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCStatsFrameTooLongs.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCStatsFrameTooLongs[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCStatsFrameTooLongs[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCStatsInternalMacReceiveErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCStatsInternalMacReceiveErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCStatsInternalMacReceiveErrors[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCStatsInternalMacReceiveErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCStatsSymbolErrors (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCStatsSymbolErrors.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCStatsSymbolErrors[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCStatsSymbolErrors[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCControlInUnknownOpcodes (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCControlInUnknownOpcodes.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCControlInUnknownOpcodes[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCControlInUnknownOpcodes[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCInPauseFrames (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCInPauseFrames.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCInPauseFrames[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCInPauseFrames[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

bool fixs_cch_cmdshelfmanager::getBulkDot3HCOutPauseFrames (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidDot3HCOutPauseFrames.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {

			m_dot3HCOutPauseFrames[i] = atoll(out[i].c_str());
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_dot3HCOutPauseFrames[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}

void fixs_cch_cmdshelfmanager::populateIfHCInPkts (const int &no_of_interface) {

	//****************DEBUG**************************
//	std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	for(int i=0;i<no_of_interface;i++)
	{
//*****************DEBUG***************************************
//		std::cout << "I: " << i << std::endl;
//*************************************************************
		ULONGLONG tempIfHCInUcastPkts=0,tempIfHCInMulticastPkts=0,tempIfHCInBroadcastPkts=0;
		if( m_ifHCInUcastPkts[i] != ULLONG_MAX)
			tempIfHCInUcastPkts = m_ifHCInUcastPkts[i];
		if( m_ifHCInMulticastPkts[i] != ULLONG_MAX)
			tempIfHCInMulticastPkts = m_ifHCInMulticastPkts[i];
		if(m_ifHCInBroadcastPkts[i] != ULLONG_MAX)
			tempIfHCInBroadcastPkts = m_ifHCInBroadcastPkts[i];
		if(( m_ifHCInUcastPkts[i] == ULLONG_MAX) || ( m_ifHCInMulticastPkts[i] == ULLONG_MAX) || (m_ifHCInBroadcastPkts[i] == ULLONG_MAX))
			m_ifHCInPkts[i] = ULLONG_MAX;
		else
			m_ifHCInPkts[i]= tempIfHCInUcastPkts + tempIfHCInMulticastPkts + tempIfHCInBroadcastPkts;
	}
}

void fixs_cch_cmdshelfmanager::populateIfHCOutPkts (const int &no_of_interface){

	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	for(int i=0;i<no_of_interface;i++)
	{
//*****************DEBUG***************************************
//		std::cout << "I: " << i << std::endl;
//*************************************************************

		ULONGLONG tempIfHCOutUcastPkts=0,tempIfHCOutMulticastPkts=0,tempIfHCOutBroadcastPkts=0;
		if( m_ifHCOutUcastPkts[i] != ULLONG_MAX)
			tempIfHCOutUcastPkts = m_ifHCOutUcastPkts[i];
		if( m_ifHCOutMulticastPkts[i] != ULLONG_MAX)
			tempIfHCOutMulticastPkts = m_ifHCOutMulticastPkts[i];
		if( m_ifHCOutBroadcastPkts[i] != ULLONG_MAX)
			tempIfHCOutBroadcastPkts = m_ifHCOutBroadcastPkts[i];
		if(( m_ifHCOutUcastPkts[i] == ULLONG_MAX) || ( m_ifHCOutMulticastPkts[i] == ULLONG_MAX) || (m_ifHCOutBroadcastPkts[i] == ULLONG_MAX))
			m_ifHCOutPkts[i] = ULLONG_MAX;
		else
			m_ifHCOutPkts[i]= tempIfHCOutUcastPkts + tempIfHCOutMulticastPkts + tempIfHCOutBroadcastPkts;

	}
}

std::string fixs_cch_cmdshelfmanager::intToString (int ivalue)
{
	std::stringstream ss_value("");
	ss_value << ivalue; //try to convert operand to value number
	return ss_value.str();
}

bool fixs_cch_cmdshelfmanager::getFwmLoadedAttributeSCX (unsigned int index, int *Val, int &retCode)
{
	int res = 0;
	bool retValue = false;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = intToString(index);

	tmpOID = oidFwmLoadedAttributeSCX + "." + indexStr;

	//std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	retCode = res;

	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		*Val = atoi(buffer);
		retValue = true;

		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else
		retValue = false;

	return retValue;
}

bool fixs_cch_cmdshelfmanager::getFwmLoadedType (unsigned int index, std::string &loadVer)
{
	int res = 0;
	bool retValue = false;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = intToString(index);

	tmpOID = oidFwmLoadedTypeSCX + "." + indexStr;

	//std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		loadVer = string((char *)out);
		retValue = true;
	}
	else
		retValue = false;

	return retValue;
}

bool fixs_cch_cmdshelfmanager::getFwmLoadedVersion (unsigned int index, std::string &loadVer)
{
	int res = 0;
	bool retValue = false;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = intToString(index);

	tmpOID = oidFwmLoadedVersionSCX + "." + indexStr;

	//std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		loadVer = string((char *)out);
		retValue = true;
	}
	else
		retValue = false;

	return retValue;
}
bool fixs_cch_cmdshelfmanager::getBladeRevisionState (std::string *actState)
{
	int res = 0;
	bool retValue = false;
	int index = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = intToString(index);

	tmpOID = oidbmBladeProductRevisionState + "." + indexStr;

	//std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		*actState = string((char *)out);
		retValue = true;
	}
	else
		retValue = false;

	return retValue;
}

/*############################# IRONSIDE METHODS FOR DMX ENVIRONMENT ###############################################*/


int fixs_cch_cmdshelfmanager::shelfAddrToPhysicalAddr(const char* shelfaddr)
{

	std::stringstream ss(shelfaddr);
	std::string item;
	char delim = '.';

	std::vector<std::string> elems;
	while(std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}

	char tmp[32] = {0};
	if (elems.size() == 4)
	{
		int plug0 = atoi(elems.at(0).c_str());
		int plug1 = atoi(elems.at(1).c_str());
		int plug3 = atoi(elems.at(3).c_str());

		sprintf(tmp,"%x%x%x",plug3,plug1,plug0);

	}
	else
		return -1;

	char* endPtr;
	int mag = strtol(tmp, &endPtr,16);

	return mag;

}

std::string fixs_cch_cmdshelfmanager::physicalAddrToShelfAddr(int phyAddr)
{

	 //phyAddr = 15;


	char tmp[32] = {0};
	sprintf(tmp, "%03x", phyAddr);

//	std::cout << "@@@DBG: Temp converted string: " << tmp << std::endl;

	char plug0_str[2] = {tmp[2], 0};
	char plug1_str[2] = {tmp[1], 0};
	char plug3_str[2] = {tmp[0], 0};

	char* endPtr;
	int plug0 = strtol(plug0_str, &endPtr,16);
	int plug1 = strtol(plug1_str, &endPtr,16);
	int plug3 = strtol(plug3_str, &endPtr,16);

	char addr[16] = {0};

	sprintf(addr, "%d.%d.0.%d", plug0, plug1, plug3);

	std::string ret_val = addr;

	return ret_val;

}

int fixs_cch_cmdshelfmanager::executeUDP(std::string & data)
{
	acs_nclib_library::init();
	acs_nclib_session session(acs_nclib::UDP);
	bool success = false;
	int retcode = XCOUNTLS_Util::EXECUTED;
	
	for(int i=0; ((i<2) && (!success)) ;i++)
	{
		if (session.open(ipAddr.c_str(),DMX_PORT,acs_nclib::USER_AUTH_NONE,"") != acs_nclib::ERR_NO_ERRORS)
		{
			retcode=XCOUNTLS_Util::IRONSIDE_FAILURE;
			changeIpAddress();
			continue;
		}

		acs_nclib_udp* udp = acs_nclib_factory::create_udp(acs_nclib::OP_UDP_GET);
		acs_nclib_message* answer = 0;
		udp->set_cmd(cmd);

		//acs_nclib_rpc_reply* answer = 0;
		int result = 0;

		if ((result = session.send(udp)) == 0)
		{
		//	std::cout << "\n\nUDP Get Message sent:" << udp << std::endl;

			acs_nclib_udp_reply * reply=0;
			if (session.receive(answer, 3000) == acs_nclib::ERR_NO_ERRORS)
			{
				reply=dynamic_cast<acs_nclib_udp_reply*>(answer);
				if((reply!=0) &&  (reply->get_message_id() == udp->get_message_id()))
				{
					reply->get_data(data);
					success=true;
					retcode=XCOUNTLS_Util::EXECUTED;
		//			std::cout << "\n\nAnswer Received:\n" << data.c_str() << std::endl;
				}
				else
				{
		//			std::cout << "DBG: Receive message have different id or reply error" << std::endl;
					retcode=XCOUNTLS_Util::IRONSIDE_FAILURE;

				}
			}
			else
			{
		//		std::cout << "DBG: Session Failed, error" << std::endl;
				retcode=XCOUNTLS_Util::IRONSIDE_FAILURE;  //Returning session open failed error to keep it backward compatible.
			}

		}
		else
		{
		//	std::cout << "Send Failed. RC = " << session.last_error_code() << " " << session.last_error_text() << std::endl;
			retcode=XCOUNTLS_Util::IRONSIDE_FAILURE;
		}
		if (answer)
			acs_nclib_factory::dereference(answer);
		if (udp)
			acs_nclib_factory::dereference(udp);

		if (session.close() != 0)
		{
			retcode=XCOUNTLS_Util::IRONSIDE_FAILURE;
		}
		if(!success)
			changeIpAddress(); // set other IP address
	}
	acs_nclib_library::exit();
	return retcode;
}

int fixs_cch_cmdshelfmanager::getShelfId(std::string physical_Address,std::string &ShelfID)
{
	int result = -1;
	std::string data;
	vector<string> tokens;
	std::map<int,std::string> shelfMap;
	string buf;

	std::map<int, std::string>::iterator it;
	int phAddr = atoi(physical_Address.c_str());
	
	it = shelfMap.find(phAddr);
	if (it != shelfMap.end())
	{
		ShelfID = it->second;		
	}
	else {
		std::string filter("SHLF--------");
		setudpcmd(filter);
		result=executeUDP(data);
	
		if (result !=0)
		{
			return result;
		}
		stringstream ss(data);
	
		while (ss >> buf){
			tokens.push_back(buf);
		}
        
		for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
		{
			std::string shelfid,physadd;
			string::size_type pos_start = 0, pos_end;
			pos_end = tokens[i] .find(":",pos_start);
        
			if (pos_end != string::npos)
			{
				shelfid = tokens[i].substr(0, pos_end);
				physadd = tokens[i].substr(pos_end+1);

			}

			int phAddr = atoi(physadd.c_str());
			shelfMap.insert(std::pair<int, std::string>(phAddr, shelfid));
		}
		it = shelfMap.find(phAddr);
		if (it != shelfMap.end())
		{
			ShelfID = it->second;
			result = XCOUNTLS_Util::EXECUTED;
		}
		else
		{
			result = XCOUNTLS_Util::HARDWARE_NOT_PRESENT;
		}
	
	}

	return result;
}

bool fixs_cch_cmdshelfmanager::checkShelfFromAddressUDP(std::string magAddr, int &retcode)
{
	retcode = XCOUNTLS_Util::EXECUTED;

	std::string filter;
	std::string data;


	int physAddr = shelfAddrToPhysicalAddr(magAddr.c_str());
	char physAddrStr[16] = {0};
	sprintf(physAddrStr,"%d",physAddr);

	std::string shelf_id;
	std::string IPA;
	convertIpAddres(_dmxc_addresses[0],IPA); //start querying on IPA dmx BGCI address
	setIpAddress(IPA);
	retcode = getShelfId(physAddrStr,shelf_id);
	if(retcode != XCOUNTLS_Util::EXECUTED)
		return false;

	dmx::shelf new_shelf(shelf_id,physAddrStr);
	m_shelfList.push_back(new_shelf);
	return true;
}


bool fixs_cch_cmdshelfmanager::getIfDataUDP(std::string shelfAddr, int slot, int &no_of_interface, int verbose)
{
	initArray();

	std::string filter;
	std::string data;
	vector<std::string> strList;
	int retcode= XCOUNTLS_Util::EXECUTED;
	
	int phAddr = shelfAddrToPhysicalAddr(shelfAddr.c_str());
	char phAdrrStr[16] = {0};

	sprintf(phAdrrStr,"%d",phAddr);

	std::string shelfId("");
	//Note: At this level we assume that shelf list has been already loaded!!!
	for (std::vector<dmx::shelf>::iterator it = m_shelfList.begin(); it != m_shelfList.end() && shelfId.empty(); it++)
	{
		if (it->physicalAddress.compare(phAdrrStr) == 0)
		{
			shelfId = it->shelfId;
			break;
		}
	}

	if (shelfId.empty())
		return false;
	stringstream ss;
	ss << slot;
	string slotStr = ss.str();
	std::string IPA;
	convertIpAddres(_dmxc_addresses[0],IPA); //start querying on IPA dmx BGCI address
	setIpAddress(IPA);
	retcode=getIfNumberUDP(shelfId,slotStr, no_of_interface);
	if((retcode != XCOUNTLS_Util::EXECUTED) || (no_of_interface == 0))
		return false;
	getIfOperStatusUDP(shelfId,slotStr);
	getIfInOctetsUDP(shelfId,slotStr);
	getIfHCInUcastPktsUDP(shelfId,slotStr);
	getIfHCInMulticastPktsUDP(shelfId,slotStr);
	getIfHCInBroadcastPktsUDP(shelfId,slotStr);
	getIfHCOutMulticastPktsUDP(shelfId,slotStr);
	getIfHCOutBroadcastPktsUDP(shelfId,slotStr);
	getIfHCOutUcastPktsUDP(shelfId,slotStr);
	getIfOutOctetsUDP(shelfId,slotStr);
	getIfInErrorsUDP(shelfId,slotStr);
	getIfOutErrorsUDP(shelfId,slotStr);
	getIfOutDiscardsUDP(shelfId,slotStr);
	getIfHCOutUcastPktsUDP(shelfId,slotStr);

	if(verbose)
	{
		getIfInDiscardsUDP(shelfId,slotStr);
	}
	
	return true;
}


int fixs_cch_cmdshelfmanager::getIfNumberUDP(std::string &shelfid,std::string & slot, int &no_of_interface)
{
	std::string cmd("APS:");
	std::string  as("AS__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;

	if(data.compare("N/A\n") == 0)
	{
		return XCOUNTLS_Util::HARDWARE_NOT_PRESENT;   //here we can sent ironside failure as well..for the moment we printno data available
	}
	
	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
        int count =0;
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0; ((i<tokens.size()) && (i < MAX_NO_OF_INTERFACE)) ;i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			this->m_ifAlias[i] = strval.substr(0, pos);
			count++;
		}
	}
	no_of_interface = count;
	return result;
}


int fixs_cch_cmdshelfmanager::getIfOperStatusUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("OS__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			if (str.compare("enabled") == 0)
			{
                        	this->m_ifOperStatus[i] = "UP";
                	}
			else
			{
                        	this->m_ifOperStatus[i] = "DOWN";
			}
		}
	}
	return result;
}


int fixs_cch_cmdshelfmanager::getIfInOctetsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("IO__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifHCInOctets[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}

int fixs_cch_cmdshelfmanager::getIfOutOctetsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("OO__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifHCOutOctets[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}


int fixs_cch_cmdshelfmanager::getIfHCOutMulticastPktsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("OM__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result != 0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifHCOutMulticastPkts[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}

int fixs_cch_cmdshelfmanager::getIfHCOutBroadcastPktsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("OB__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifHCOutBroadcastPkts[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}

int fixs_cch_cmdshelfmanager::getIfHCInMulticastPktsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("IM__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifHCInMulticastPkts[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}

int fixs_cch_cmdshelfmanager::getIfHCInBroadcastPktsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("IB__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result != 0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifHCInBroadcastPkts[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}

int fixs_cch_cmdshelfmanager::getIfOutErrorsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("OE__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;
	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifOutErrors[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}


int fixs_cch_cmdshelfmanager::getIfInErrorsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("IE__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;	

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifInErrors[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}

int fixs_cch_cmdshelfmanager::getIfInDiscardsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("ID__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result != 0)
		return result;
	
	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifInDiscards[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}


int fixs_cch_cmdshelfmanager::getIfOutDiscardsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("OD__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result != 0)
		return result;
	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifOutDiscards[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}


int fixs_cch_cmdshelfmanager::getIfHCOutUcastPktsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("OU__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifHCOutUcastPkts[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}


int fixs_cch_cmdshelfmanager::getIfHCInUcastPktsUDP(std::string &shelfid,std::string & slot)
{
	std::string cmd("APS:");
	std::string  as("IU__");
	std::ostringstream ss1,ss2;
  	ss1 << std::setw(2) << std::setfill('0') << shelfid.c_str();
  	ss2 << std::setw(2) << std::setfill('0') << slot.c_str();
  	std::string tmp_id(ss1.str());
  	std::string tmp_slot(ss2.str());
	cmd = cmd + tmp_id + tmp_slot + as;

	std::string data,buf;
	vector<string> tokens;	
	setudpcmd(cmd);
	int result=executeUDP(data);
	if(result !=0)
		return result;

	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}
	std::string delimiter = ":";
	size_t pos = 0;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string strval = tokens[i];
		if((pos = strval.find(delimiter)) != std::string::npos)
		{
			int length = strval.length();
			std::string str = strval.substr(pos+1, length-1);
			this->m_ifHCInUcastPkts[i] = strtoul(str.c_str(), NULL, 0);
		}
	}
	return result;
}


void fixs_cch_cmdshelfmanager::convertIpAddres(uint32_t address, std::string &ip)
{
	char addr_c[16];

        // convert address to human readable format
        uint32_t naddr = htonl(address);
        inet_ntop(AF_INET, &naddr, addr_c, sizeof(addr_c));
	std::string temp(addr_c);
	ip = temp;
}

int fixs_cch_cmdshelfmanager::getShelfListUDP(std::vector<std::string> &magAddrs)
{
	vector<string> tokens;
	std::string filter("SHLF--------");
	setudpcmd(filter);
	std::string IPA,data;
	convertIpAddres(_dmxc_addresses[0],IPA);

	setIpAddress(IPA);
	int result=executeUDP(data);
	if(result != XCOUNTLS_Util::EXECUTED)
		return result;	
	
	std::string buf;
	stringstream ss(data);

	while (ss >> buf){
		tokens.push_back(buf);
	}

	if(tokens.size() == 0)
		return XCOUNTLS_Util::HARDWARE_NOT_PRESENT;
	int count =0;
	bool setList = false;
	if(m_shelfList.size() > 0)
		setList = true;
	for(unsigned int i=0;((i < tokens.size()) && (i < MAX_NO_OF_INTERFACE));i++)
	{
		std::string shelfid,physadd;
		string::size_type pos_start = 0, pos_end;
		pos_end = tokens[i] .find(":",pos_start);

		if (pos_end != string::npos)
		{
			shelfid = tokens[i].substr(0, pos_end);
			physadd = tokens[i].substr(pos_end+1);
			if(setList == false)
			{
				dmx::shelf new_shelf(shelfid,physadd);
        			m_shelfList.push_back(new_shelf);
			}
			count++;

			int phAddr = atoi(physadd.c_str());
			std::string magAddr = physicalAddrToShelfAddr(phAddr);
			magAddrs.push_back(magAddr);
		}
	}
	if(count == 0)
		return XCOUNTLS_Util::HARDWARE_NOT_PRESENT;


	return XCOUNTLS_Util::EXECUTED;
}

void fixs_cch_cmdshelfmanager::changeIpAddress()
{
	std::string IPA,IPB;
	convertIpAddres(_dmxc_addresses[0],IPA);
	convertIpAddres(_dmxc_addresses[1],IPB);
	if(IPA.compare(ipAddr) == 0)
		ipAddr=IPB;
	else
		ipAddr=IPA;
}


bool fixs_cch_cmdshelfmanager::getActiveArea (std::string slot,int &activeArea)
{
	int res;
	activeArea= -1;
	std::string out;
	std::string tmpOID("");

	tmpOID = oidBladeBIOSPointer + "." + slot;

	res = snmpGet(out, "PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER);
	if (res == GETSET_SNMP_OK)
	{
		activeArea = atoi(out.c_str());
	}
	else {
		activeArea = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

int fixs_cch_cmdshelfmanager::isMaster (std::string ip)
{
	int resp = 0;
	int res;
	string temp_Ip = "";
	BYTE out[SNMP_MAXBUF];

	std::string tmpOID("");

	tmpOID = oidShelfMgrCtrlSCX + ".0";

	setIpAddress(ip);

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out,SNMP_MAXBUF, fixs_cch_cmdshelfmanager::SNMPGET_HEX_STRING);
	if (res == GETSET_SNMP_OK) {
		char buffer[128] = { 0 };
		sprintf(buffer, "%s\n", (char *) out);
		std::string shelfMgrCtrlState = std::string((char*) out);

		int masterSCB = 0;
		if (shelfMgrCtrlState.length() >= 2) {
			std::string tmp = shelfMgrCtrlState.substr(0, 2);
			masterSCB = ::atoi(tmp.c_str());
		} else {
			masterSCB = ::atoi(shelfMgrCtrlState.c_str());
		}

		if (masterSCB == 3) //Active
		{
			resp = 1;
		} else
		{
			resp = 2;
		}
	} else if (res == GETSET_SNMP_STATUS_TIMEOUT) {
		resp = HARDWARE_NOT_PRESENT;
	}
	return resp;
}

bool fixs_cch_cmdshelfmanager::getBulkIfName (const int &no_of_interface) {
	//****************DEBUG**************************
	//std::cout<< __FUNCTION__ << "@" << __LINE__ << "..............................." << std::endl;
	//***********************************************


	std::vector<std::string> out;
	int res = snmpBulk(out, "PUBLIC", ipAddr.c_str(), oidIfName.c_str(), no_of_interface, false);
	if (res == GETSET_SNMP_OK) {
		unsigned int maxIf;
		if (out.size() >= MAX_NO_OF_INTERFACE) {
			maxIf = MAX_NO_OF_INTERFACE;
		} else {
			maxIf = out.size();
		}

		for (unsigned int i = 0 ; i < maxIf /*out.size()*/ ; i++) {
			m_ifAlias[i] = out[i].c_str();
//*****************DEBUG***************************************
//			std::cout << i << " - " << out[i].c_str() << " - " << m_ifAlias[i] << std::endl;
//*************************************************************
		}
	} else {
		std::cout << "SNMP FAILURE" << std::endl;
		return false;
	}
	return true;
}


bool fixs_cch_cmdshelfmanager::getFwmActionResult(int &result)
{
	int res=0;
	result= -1;
	std::string out;
	std::string tmpOID("");

	tmpOID = oidFwmActionResult;

	res = snmpGet(out, "PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_GAUGE);
	if (res == GETSET_SNMP_OK)
	{
		result = atoi(out.c_str());
	}
	else {
		result = -1;
		return false;
	}
	
	return true;
}
