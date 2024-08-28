
#include "FIXS_CMXH_SNMPManager.h"
#include "FIXS_CMXH_Logger.h"

/************************************************************/
/*							OID								*/
/************************************************************/

//GEN-IF_MIB
std::string oidIfAdminStatus =		"1.3.6.1.2.1.2.2.1.7";
//RSTP_MIB
std::string oidDot1dStpPortAdminEdgePort = "1.3.6.1.2.1.17.2.19.1.2";
//BRIDGE-MIB
std::string oidDot1dStpPortEnable = "1.3.6.1.2.1.17.2.15.1.4";
// VLAN OIDS DECLARATION
std::string oidDot1qVlanStaticRowStatus = "1.3.6.1.2.1.17.7.1.4.3.1.5";
std::string oidDot1qVlanStaticName = "1.3.6.1.2.1.17.7.1.4.3.1.1";
std::string oidDot1qVlanStaticEgressPorts = "1.3.6.1.2.1.17.7.1.4.3.1.2";
std::string oidDot1qVlanStaticUntaggedPorts = "1.3.6.1.2.1.17.7.1.4.3.1.4";
std::string oidDot1dPortDefaultUserPriority = "1.3.6.1.2.1.17.6.1.2.1.1.1";

// CMX LOG COLLECTION START
std::string oidContTransferSrvIP = "1.3.6.1.4.1.193.177.2.2.4.4.1.1.0";
std::string oidContTransferSrvPath = "1.3.6.1.4.1.193.177.2.2.4.4.1.2.0";
std::string oidContNotificationsCtrl = "1.3.6.1.4.1.193.177.2.2.4.4.1.3.0";
std::string oidContAutomaticTransfer = "1.3.6.1.4.1.193.177.2.2.4.4.1.4.0";
std::string oidContFileEncryption = "1.3.6.1.4.1.193.177.2.2.4.4.1.6.0";
std::string oidCreateContFile = "1.3.6.1.4.1.193.177.2.2.4.4.6.0";
std::string oidTransContFile = "1.3.6.1.4.1.193.177.2.2.4.4.3.0";
std::string oidContFileValue =	"1.3.6.1.4.1.193.177.2.2.4.2.1.0";
std::string oidContFileName = "1.3.6.1.4.1.193.177.2.2.4.4.2.1.2";
std::string oidDeleteContFile = "1.3.6.1.4.1.193.177.2.2.4.4.4";
std::string oidContFileTransferComplete	= "1.3.6.1.4.1.193.177.2.2.4.1.2";
std::string oidContFileResult =	"1.3.6.1.4.1.193.177.2.2.4.2.2.0";
std::string oidContFileLevel = "1.3.6.1.4.1.193.177.2.2.4.2.3.0";

ACE_Recursive_Thread_Mutex FIXS_CMXH_SNMPManager::m_lock;       //TR_HX98360

FIXS_CMXH_SNMPManager::FIXS_CMXH_SNMPManager(std::string ipAddress){

	FIXS_CMXH_logging = FIXS_CMXH_Logger::getLogInstance();
	bExit = false;
	bRun = false;
	ipAddr = ipAddress;

	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CMXH_SNMPManager (NEW) on IP Address(%s) ", ipAddress.c_str());
	std::cout << tmpStr << std::endl;
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);


	init_snmp("SnmpManager");
	m_cba_environment = CMXH_Util::UNKNOWN;

}

FIXS_CMXH_SNMPManager::~FIXS_CMXH_SNMPManager(){
	FIXS_CMXH_logging = 0;
}

int FIXS_CMXH_SNMPManager::snmpGet(const char* community, const char* peername,const char* usrOID, int type, void* out, unsigned int iBufSize, int strType)
{
        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);       //TR_HX98360

	int res;
	snmp_session snmpSession;
	//	snmp_session *snmpSessionHandler;
	void *snmpSess;
	char *pchBuffer;
	int *piBuffer;
	int iReqSize;
	int getResult = GETSET_SNMP_OK;
	struct snmp_pdu *snmpPDU;
	struct snmp_pdu *snmpResp=NULL;

	struct variable_list *varsList;

	pchBuffer = NULL;
	piBuffer = NULL;
	iReqSize = 0;
	snmp_sess_init( &snmpSession );


	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = 5;
	//	snmpSession.timeout = 500000;
	snmpSession.timeout = 200000;
	snmpSession.community_len = strlen((const char *)snmpSession.community);
	snmpSession.peername = strdup(peername);

	//	snmpSessionHandler = snmp_open(&snmpSession);
	snmpSess = snmp_sess_open(&snmpSession);

	free(snmpSession.community);
	free(snmpSession.peername);

	if(snmpSess == NULL)
	{
		std::cout << "DBG: " <<__FUNCTION__ << "@" << __LINE__ <<" ERROR: snmp_open failed - snmp_errno: " << snmp_errno << std::endl;

		return OPEN_SESS_ERROR;
	}

	oid snmp_oid[MAX_OID_LEN];
	size_t snmp_oid_len = MAX_OID_LEN;

	if (! read_objid(usrOID, snmp_oid, &snmp_oid_len) )
	{
		std::cout <<"DBG: "<< __FUNCTION__ << "@" << __LINE__ <<" ERROR: Transform OID Failed " <<std::endl;
		//		snmp_close(snmpSessionHandler);
		snmp_sess_close(snmpSess);
		return GETSET_SNMP_PARSE_OID_ERROR;
	}

	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GET)) != NULL)
	{
		snmp_add_null_var(snmpPDU, snmp_oid, snmp_oid_len);
		std::cout <<"DBG: "<< __FUNCTION__ << "@" << __LINE__ <<" Send request ! " <<std::endl;

		//usleep(100000);
		//		res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);
		res = snmp_sess_synch_response(snmpSess, snmpPDU, &snmpResp);

		switch(res)
		{
		case STAT_SUCCESS:
			std::cout <<"DBG: "<< __FUNCTION__ << "@" << __LINE__ <<" STAT_SUCCESS " << std::endl;
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

							switch (strType)
							{

							case FIXS_CMXH_SNMPManager::SNMPGET_STD_STRING:
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
									std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: SNMPGET_STD_STRING buffer is too small! " << std::endl;
									getResult = GETSET_SNMP_BUFFERSMALL;
								}
								//out = (void *) Buf;
								//									std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_STD_STRING " << Buf << std::endl;

								//std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_STD_STRING " << out << std::endl;
								// //out = (void *) varsList->val.string;
								//out = &varsList->val.string;
								//std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_STD_STRING " << out << std::endl;
								break;
							}

							case FIXS_CMXH_SNMPManager::SNMPGET_HEX_STRING:
							{
								//char  hexBuf[SNMP_MAXBUF];
								pchBuffer = reinterpret_cast<char*>(out);
								iReqSize = snprint_hexstring( pchBuffer, iBufSize, varsList->val.string, varsList->val_len);
								// Check error
								if(iReqSize < 0){
									// Error. Is the Buffer too small?
									std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: SNMPGET_HEX_STRING can't write in the buffer. Is buffer too small?" << std::endl;
									getResult = GETSET_SNMP_BUFFERSMALL;
								}
								//out = (void *) hexBuf;
								//									std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_HEX_STRING " << hexBuf << std::endl;
								break;
							}

							case FIXS_CMXH_SNMPManager::SNMPGET_ASCII_STRING:
							{
								pchBuffer = reinterpret_cast<char*>(out);
								iReqSize = snprint_asciistring( pchBuffer, iBufSize, varsList->val.string, varsList->val_len);
								// Check error
								if(iReqSize < 0){
									// Error. Is the Buffer too small?
									std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: SNMPGET_ASCII_STRING can't write in the buffer. Is buffer too small?" << std::endl;
									getResult = GETSET_SNMP_BUFFERSMALL;
								}
								//									std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMPGET_ASCII_STRING " << hexBuf << std::endl;
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
								std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: ASN_INTEGER buffer is too small!" << std::endl;
								getResult = GETSET_SNMP_BUFFERSMALL;
							}
							//out = (int *) *varsList->val.integer;
							break;
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
					std::cout << __FUNCTION__ << "@" << __LINE__ << " ERROR: GET-Response Error: "
							<< snmp_errstring(snmpResp->errstat) << std::endl;
					//snmp_free_pdu(snmpResp);
					//out = NULL;
					getResult =  GETSET_SNMP_RESPONSE_ERROR;
				}
			}else{	// if((out != NULL) && (iBufSize))
				// Error buffer
				std::cout << __FUNCTION__ << "@" << __LINE__ <<" ## ERROR: incorrect argument [buffer].  " << std::endl;
				getResult =  GETSET_SNMP_RESPONSE_ERROR;
			}
			break; //EXIT FOR !

		case STAT_TIMEOUT:
			std::cout << __FUNCTION__ << "@" << __LINE__ <<" STAT_TIMEOUT " << std::endl;
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "<< snmp_errstring(snmp_errno) << std::endl;

			//snmp_free_pdu(snmpResp);
			//out = NULL;
			getResult = GETSET_SNMP_STATUS_TIMEOUT;
			break;

		case STAT_ERROR:
			std::cout << __FUNCTION__ << "@" << __LINE__ <<	" STAT_ERROR " << std::endl;

			std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "	<< snmp_errstring(snmp_errno) << std::endl;

			//snmp_free_pdu(snmpResp);
			//out = NULL;
			getResult = GETSET_SNMP_STATUS_ERROR;
			break;
		}


	}
	else //if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GET)) != NULL)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: "	<< snmp_errno << std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: "	<< snmp_errstring(snmp_errno) << std::endl;

		getResult = GETSET_SNMP_PDU_CREATE_ERROR;
	}

	std::cout << __FUNCTION__ << "@" << __LINE__ <<	" Close snmp session and exit " << std::endl;

	if(snmpResp)
	{
		snmp_free_pdu(snmpResp);
	}

	//	snmp_close(snmpSessionHandler);
	snmp_sess_close(snmpSess);

	return getResult;
}


int FIXS_CMXH_SNMPManager::snmpSet(const char* community, const char* peername,const char* usrOID,  char type, const char *value)
{
	/*
TYPE: one of i, u, t, a, o, s, x, d, b
	i: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS
	o: OBJID, s: STRING, x: HEX STRING, d: DECIMAL STRING, b: BITS
	U: unsigned int64, I: signed int64, F: float, D: double
	 */
        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);         //TR_HX98360

	snmp_session snmpSession;
	//	snmp_session *snmpSessionHandler;
	void *snmpSess;

	int setResult = GETSET_SNMP_OK;
	struct snmp_pdu *snmpPDU;
	struct snmp_pdu *snmpResp=NULL;
	int res;

	snmp_sess_init( &snmpSession );

	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = 5;
	//	snmpSession.timeout = 1000000;
	snmpSession.timeout = 300000;
	snmpSession.community_len = strlen((const char *)snmpSession.community);
	snmpSession.peername = strdup(peername);

	std::cout << __FUNCTION__ << "@" << __LINE__ <<" init snmp session ok ... IP: " << snmpSession.peername <<std::endl;

	//	snmpSessionHandler = snmp_open(&snmpSession);
	snmpSess = snmp_sess_open(&snmpSession);

	free(snmpSession.community);
	free(snmpSession.peername);

	if(snmpSess == NULL)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ <<	" snmp_open failed - snmp_errno: " << snmp_errno << std::endl;
		return OPEN_SESS_ERROR;
	}

	//snmpPDU = snmp_pdu_create(SNMP_MSG_SET);

	oid snmp_oid[MAX_OID_LEN];
	size_t snmp_oid_len = MAX_OID_LEN;

	if (! read_objid(usrOID, snmp_oid, &snmp_oid_len) )
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "	<< snmp_errstring(snmp_errno) << std::endl;
		//		snmp_close(snmpSessionHandler);
		snmp_sess_close(snmpSess);
		return GETSET_SNMP_PARSE_OID_ERROR;
	}

	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_SET)) != NULL)
	{

		if (snmp_add_var(snmpPDU, snmp_oid, snmp_oid_len, type, value) )
		{

			std::cout << __FUNCTION__ << "@" << __LINE__ << " snmp_add_var error: "	<< snmp_errstring(snmpResp->errstat) << std::endl;
			//			snmp_close(snmpSessionHandler);
			snmp_sess_close(snmpSess);
			return GETSET_SNMP_PARSE_OID_ERROR;
		}
		// send pdu...
		//		res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);
		res = snmp_sess_synch_response(snmpSess, snmpPDU, &snmpResp);

		switch(res)
		{
		case STAT_SUCCESS:

			//Check the response
			if (snmpResp->errstat == SNMP_ERR_NOERROR)
			{
				// SET successful done ;) !!
				//snmp_free_pdu(snmpResp);
			}
			else
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " SET-Response error: "	<< snmp_errstring(snmpResp->errstat) << std::endl;
				//snmp_free_pdu(snmpResp);
				setResult =  GETSET_SNMP_RESPONSE_ERROR;
			}

			break;

		case STAT_TIMEOUT:
			std::cout << __FUNCTION__ << "@" << __LINE__ <<	" STAT_TIMEOUT " << std::endl;

			std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "	<< snmp_errstring(snmp_errno) << std::endl;

			setResult = GETSET_SNMP_STATUS_TIMEOUT;
			//snmp_free_pdu(snmpResp);
			break;

		case STAT_ERROR:
			std::cout << __FUNCTION__ << "@" << __LINE__ <<	" STAT_ERROR " << std::endl;

			std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: " << snmp_errstring(snmp_errno) << std::endl;

			//snmp_free_pdu(snmpResp);
			setResult = GETSET_SNMP_STATUS_ERROR;
			break;

		} //switch(res)
	}
	else //if ((snmpPDU = snmp_pdu_create(SNMP_MSG_SET)) != NULL)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation num: "	<< snmp_errno << std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error during the PDU creation: "	<< snmp_errstring(snmp_errno) << std::endl;
		setResult = GETSET_SNMP_PDU_CREATE_ERROR;
	}

	if(snmpResp)
	{
		snmp_free_pdu(snmpResp);
	}
	//	snmp_close(snmpSessionHandler);
	snmp_sess_close(snmpSess);
	return setResult;
}


std::string FIXS_CMXH_SNMPManager::getIpAddress(void)
{
	return std::string(ipAddr);
}

void FIXS_CMXH_SNMPManager::setIpAddress(std::string ip)
{
	ipAddr = ip.c_str();
}

bool FIXS_CMXH_SNMPManager::setIfMibAdminStatus (int index, int valueToSet)
{
	int res = 0;
	std::stringstream ss,sValue;
	ss << index;
	sValue << valueToSet;
	std::string tmpOID = oidIfAdminStatus + "." +  ss.str();

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', sValue.str().c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::getIfMibAdminStatus (unsigned int index, int & value)
{
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::stringstream ss;
	ss << index;
	std::string tmpOID = oidIfAdminStatus + "." +  ss.str();
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;
		value = atoi(buffer);
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else
	{
		value = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_CMXH_SNMPManager::setBridgeMibStpPortEnable (int index, int valueToSet)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	int res = 0;
	std::stringstream ss,sValue;
	ss << index;
	sValue << valueToSet;
	std::string tmpOID = oidDot1dStpPortEnable + "." +  ss.str();

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', sValue.str().c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " RSTP SET to 2 successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " RSTP SET to 2 failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	return true;
}

bool FIXS_CMXH_SNMPManager::getBridgeMibStpPortEnable (unsigned int index, int & value)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::stringstream ss;
	ss << index;
	std::string tmpOID = oidDot1dStpPortEnable + "." +  ss.str();
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;
		value = atoi(buffer);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << value << std::endl;
	}
	else
	{
		value = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Begin" << std::endl;
	return true;
}


bool FIXS_CMXH_SNMPManager::setStpPortAsEdgePort (int index, int valueToSet)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] setStpPortAsEdgePort !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	std::stringstream ss,sValue;
	ss << index;
	sValue << valueToSet;
	std::string tmpOID = oidDot1dStpPortAdminEdgePort + "." +  ss.str();

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', sValue.str().c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::getStpPortAsEdgePort (unsigned int index, int &value)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] getStpPortAsEdgePort !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::stringstream ss;
	ss << index;
	std::string tmpOID = oidDot1dStpPortAdminEdgePort + "." +  ss.str();
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//              std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;
		value = atoi(buffer);
		//              std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else
	{
		value = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;

}

bool FIXS_CMXH_SNMPManager::setMibVlanStaticRowStatus (int vlanID, int valueToSet)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] setMibVlanStaticRowStatus !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	std::stringstream ss,sValue;
	ss << vlanID;
	sValue << valueToSet;
	std::string tmpOID = oidDot1qVlanStaticRowStatus + "." +  ss.str();

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', sValue.str().c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::getMibVlanStaticRowStatus (unsigned int vlanID, int &value)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] getMibVlanStaticRowStatus !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::stringstream ss;
	ss << vlanID;
	std::string tmpOID = oidDot1qVlanStaticRowStatus + "." +  ss.str();
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//              std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;
		value = atoi(buffer);
		//              std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else
	{
		value = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;

}

bool  FIXS_CMXH_SNMPManager::setMibVlanStaticName (int vlanID, std::string valueToSet)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] setMibVlanStaticName !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	std::string tmpOID("");
	std::stringstream ss;
	ss << vlanID;

	tmpOID = oidDot1qVlanStaticName + "." + ss.str();

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 's', valueToSet.c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::getMibVlanStaticName (unsigned int vlanID, std::string &value)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] getMibVlanStaticName !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::string tmpOID("");

	std::stringstream ss;
	ss << vlanID;

	tmpOID = oidDot1qVlanStaticName + "." + ss.str();

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET successfully done on: " << tmpOID.c_str() << std::endl;
		value = std::string((char *)out);
	}else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool  FIXS_CMXH_SNMPManager::setMibVlanStaticEgressPorts (int vlanID, std::string valueToSet)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] setMibVlanStaticEgressPorts !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	std::string tmpOID("");
	std::stringstream ss;
	ss << vlanID;

	tmpOID = oidDot1qVlanStaticEgressPorts + "." + ss.str();
	int len = valueToSet.length();
	if(len != 16 && len != 52 )
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " SET failed for wrong length on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	else
	{
		size_t found;
		found=valueToSet.find_first_not_of("0123456789abcdefABCDEF");
		if (found != std::string::npos)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " SET failed for wrong input on: " << tmpOID.c_str() << std::endl;
			return false;
		}
	}

	/*int digit = 0;
	unsigned char byte[8]={0};
	std::string temp = "";
	int j=0;
	for(int i=0;i<8;i++)
	{
		//Convert hex string to hex code and assign it to unsigned char of byte.
		temp=valueToSet.substr(j,2);
		std::istringstream istr(temp);
		istr >> std::hex >> digit;
		byte[i]=(unsigned char)digit;
		j+=2;
	}

	std::string valueToBeSet(reinterpret_cast<const char *> (byte),
                     sizeof (byte) / sizeof (byte[0]));*/

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'x', valueToSet.c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::getMibVlanStaticEgressPorts (unsigned int vlanID, std::string &value)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] getMibVlanStaticEgressPorts !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::string tmpOID("");

	std::stringstream ss;
	ss << vlanID;

	tmpOID = oidDot1qVlanStaticEgressPorts + "." + ss.str();

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF,  FIXS_CMXH_SNMPManager::SNMPGET_HEX_STRING);
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET successfully done on: " << tmpOID.c_str() << std::endl;
		value = std::string((char *)out);
	}else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool  FIXS_CMXH_SNMPManager::setMibVlanStaticUntaggedPorts (int vlanID, std::string valueToSet)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] setMibVlanStaticUntaggedPorts !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	std::string tmpOID("");
	std::stringstream ss;
	ss << vlanID;
	tmpOID = oidDot1qVlanStaticUntaggedPorts + "." + ss.str();
	int len = valueToSet.length();
	if(len != 16 && len != 52)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " SET failed for wrong length on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	else
	{
		size_t found;
		found=valueToSet.find_first_not_of("0123456789abcdefABCDEF");
		if (found != std::string::npos)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " SET failed for wrong input on: " << tmpOID.c_str() << std::endl;
			return false;
		}
	}

	/*	int digit = 0;
unsigned char byte[8]={0};
std::string temp = "";
int j=0;
for(int i=0;i<8;i++)
{
//Convert hex string to hex code and assign it to unsigned char of byte.
temp=valueToSet.substr(j,2);
std::istringstream istr(temp);
istr >> std::hex >> digit;
byte[i]=(unsigned char)digit;
j+=2;
}

std::string valueToBeSet(reinterpret_cast<const char *> (byte),
sizeof (byte) / sizeof (byte[0])); */


	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'x', valueToSet.c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::getMibVlanStaticUntaggedPorts (unsigned int vlanID, std::string &value)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] getMibVlanStaticUntaggedPorts !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::string tmpOID("");

	std::stringstream ss;
	ss << vlanID;

	tmpOID = oidDot1qVlanStaticUntaggedPorts + "." + ss.str();

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF, FIXS_CMXH_SNMPManager::SNMPGET_HEX_STRING);
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET successfully done on: " << tmpOID.c_str() << std::endl;
		value = std::string((char *)out);
	}else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool FIXS_CMXH_SNMPManager::setMibPortDefaultUserPriority (int portNum, int portPriority)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_SNMPManager] setMibPortDefaultUserPriority !!!",LOG_LEVEL_DEBUG);
	int res = 0;
	std::stringstream ss,sValue;
	ss << portNum;
	sValue << portPriority;
	std::string tmpOID = oidDot1dPortDefaultUserPriority + "." +  ss.str();

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', sValue.str().c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::getMibPortDefaultUserPriority (unsigned int portNum, int &value)
{
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::stringstream ss;
	ss << portNum;
	std::string tmpOID = oidDot1dPortDefaultUserPriority + "." +  ss.str();
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//              std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;
		value = atoi(buffer);
		//              std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else
	{
		value = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;

}

bool FIXS_CMXH_SNMPManager::getContFileName (unsigned int Index, std::string &fileName)
{
	int res = 0;
	unsigned char out[SNMP_MAXBUF];
	std::string tmpOID("");

	std::stringstream ss;
	ss << Index;

	tmpOID = oidContFileName + "." + ss.str();

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET successfully done on: " << tmpOID.c_str() << std::endl;
		fileName = std::string((char *)out);
	}else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool  FIXS_CMXH_SNMPManager::setContFileToDelete (std::string  fileName)
{
	int res = 0;
	std::string tmpOID("");

	tmpOID = oidDeleteContFile + "." + "0";
	std::cout << "\n fileName = " << fileName.c_str() << std::endl;
	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 's', fileName.c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		int retryVal=0;

		while( retryVal<15 && (res != GETSET_SNMP_OK))
		{
			res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 's', fileName.c_str());
			retryVal++;
		}

		if( res == GETSET_SNMP_OK)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
			return false;
		}
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::setContTransferSrvIP (std::string &srvIP)
{
	int res = 0;
	res = snmpSet("NETMAN", ipAddr.c_str(),oidContTransferSrvIP.c_str(), 's',srvIP.c_str());
	if (res == GETSET_SNMP_OK) 
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContTransferSrvIP.c_str() << std::endl;
	}
	else 
	{
		int retryVal=0;

		while( retryVal<3  &&  res == GETSET_SNMP_STATUS_TIMEOUT)
		{
			res = snmpSet("NETMAN", ipAddr.c_str(),oidContTransferSrvIP.c_str(), 's',srvIP.c_str());	
			retryVal++;
		}
		if(res != 0)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << oidContTransferSrvIP.c_str() << std::endl;
			return false;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContTransferSrvIP.c_str() << std::endl;	
		}
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::setContTransferSrvPath (std::string & snmplogPath)
{
	int res = 0;
	res = snmpSet("NETMAN", ipAddr.c_str(),oidContTransferSrvPath.c_str(), 's',snmplogPath.c_str());
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContTransferSrvPath.c_str() << std::endl;
	}
	else
	{
		int retryVal=0;

		while( retryVal<3  &&  res == GETSET_SNMP_STATUS_TIMEOUT)
		{
			res = snmpSet("NETMAN", ipAddr.c_str(),oidContTransferSrvPath.c_str(), 's',snmplogPath.c_str());
			retryVal++;
		}
		if(res != 0)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << oidContTransferSrvPath.c_str() << std::endl;
			return false;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContTransferSrvPath.c_str() << std::endl;
		}
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::setContNotificationsCtrl (int & Value)
{
	int res = 0;
	std::stringstream sValue;
	sValue << Value;
	res = snmpSet("NETMAN", ipAddr.c_str(),oidContNotificationsCtrl.c_str(), 'i',sValue.str().c_str());
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContNotificationsCtrl.c_str() << std::endl;
	}
	else
	{
		int retryVal=0;

		while( retryVal<3  &&  res == GETSET_SNMP_STATUS_TIMEOUT)
		{
			res = snmpSet("NETMAN", ipAddr.c_str(),oidContNotificationsCtrl.c_str(), 'i',sValue.str().c_str());
			retryVal++;
		}
		if(res != 0)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << oidContNotificationsCtrl.c_str() << std::endl;
			return false;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContNotificationsCtrl.c_str() << std::endl;
		}
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::setContAutomaticTransfer (int & Value)
{
	int res = 0;
	std::stringstream sValue;
	sValue << Value;
	res = snmpSet("NETMAN", ipAddr.c_str(),oidContAutomaticTransfer.c_str(), 'i',sValue.str().c_str());
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContAutomaticTransfer.c_str() << std::endl;
	}
	else
	{
		int retryVal=0;

		while( retryVal<3  &&  res == GETSET_SNMP_STATUS_TIMEOUT)
		{
			res = snmpSet("NETMAN", ipAddr.c_str(),oidContAutomaticTransfer.c_str(), 'i',sValue.str().c_str());
			retryVal++;
		}
		if(res != 0)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << oidContAutomaticTransfer.c_str() << std::endl;
			return false;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContAutomaticTransfer.c_str() << std::endl;
		}
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::setContFileEncryption (int & Value)
{
	int res = 0;
	std::stringstream sValue;
	sValue << Value;
	res = snmpSet("NETMAN", ipAddr.c_str(),oidContFileEncryption.c_str(), 'i',sValue.str().c_str());
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContFileEncryption.c_str() << std::endl;
	}
	else
	{
		int retryVal=0;

		while( retryVal<3  &&  res == GETSET_SNMP_STATUS_TIMEOUT)
		{
			res = snmpSet("NETMAN", ipAddr.c_str(),oidContFileEncryption.c_str(), 'i',sValue.str().c_str());
			retryVal++;
		}
		if(res != 0)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << oidContFileEncryption.c_str() << std::endl;
			return false;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidContAutomaticTransfer.c_str() << std::endl;
		}
	}
	return true;
}

bool FIXS_CMXH_SNMPManager::setCreateContFile (int & Value)
{
	int res = 0;
	std::stringstream sValue;
	sValue << Value;
	res = snmpSet("NETMAN", ipAddr.c_str(),oidCreateContFile.c_str(), 'i',sValue.str().c_str());
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidCreateContFile.c_str() << std::endl;
	}
	else
	{
		int retryVal=0;

		while( retryVal<3  &&  res == GETSET_SNMP_STATUS_TIMEOUT)
		{
			res = snmpSet("NETMAN", ipAddr.c_str(),oidCreateContFile.c_str(), 'i',sValue.str().c_str());
			retryVal++;
		}
		if(res != 0)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << oidCreateContFile.c_str() << std::endl;
			return false;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << oidCreateContFile.c_str() << std::endl;
		}
	}
	return true;
}

