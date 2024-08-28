/*
 * FIXS_TREFC_SnmpManager.cpp
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */


#include "FIXS_TREFC_SNMPManager.h"


	/************************************************************/
	/*							OID								*/
	/************************************************************/

	//GEN-NTP_MIB
	std::string oidGenNtpEntServer =					"1.3.6.1.4.1.193.177.2.3.4.1.4.2.1.3";
	std::string oidGenNtpEntSvrRowStatus =				"1.3.6.1.4.1.193.177.2.3.4.1.4.2.1.4";
	std::string oidGenNtpEntConfigComplete =			"1.3.6.1.4.1.193.177.2.3.4.1.4.3.0";
	//std::string oidTempGenNtpEntConfigComplete =		"1.3.6.1.4.1.193.177.2.3.4.1.4.3";
	std::string oidGenNtpEntNtpdRestart =			    "1.3.6.1.4.1.193.177.2.3.4.1.4.4.0";
	std::string oidTempGenNtpEntNtpdRestart =			"1.3.6.1.4.1.193.177.2.3.4.1.4.4";
	std::string oidGenNtpEntStatusCurrentMode =			"1.3.6.1.4.1.193.177.2.3.4.1.2.1";
	std::string oidGenNTPEntStatusActiveRefSourceId =	"1.3.6.1.4.1.193.177.2.3.4.1.2.3";
	std::string oidGenNtpEntWhiteInterface =			"1.3.6.1.4.1.193.177.2.3.4.1.1.3.1.2";
	std::string oidGenNtpEntWhiteRowStatus =			"1.3.6.1.4.1.193.177.2.3.4.1.1.3.1.3";


	//GEN-STSI-MIB
	std::string oidStsiSyncInput =						"1.3.6.1.4.1.193.177.2.3.1.3.2.0";
	std::string oidStsiTuRole =						    "1.3.6.1.4.1.193.177.2.3.1.3.1.0";
	std::string oidStsiTodServer =						"1.3.6.1.4.1.193.177.2.3.1.4.3.0";
	std::string oidStsiTodRef =							"1.3.6.1.4.1.193.177.2.3.1.3.10.0";
	std::string oidstsiNtpRefRowStatus =				"1.3.6.1.4.1.193.177.2.3.1.3.6.1.2";
	std::string oidstsiNtpRefIp =						"1.3.6.1.4.1.193.177.2.3.1.3.6.1.3";
	std::string oidStsiSntpIfRowStatus =				"1.3.6.1.4.1.193.177.2.3.1.4.1.1.2";
	std::string oidStsiSntpIfIp =						"1.3.6.1.4.1.193.177.2.3.1.4.1.1.3";
	std::string oidStsiSntpIfIpMask =					"1.3.6.1.4.1.193.177.2.3.1.4.1.1.4";
	std::string oidStsiSntpIfVlan =						"1.3.6.1.4.1.193.177.2.3.1.4.1.1.5";
	std::string oidStsiSntpSetCpuClock =				"1.3.6.1.4.1.193.177.2.3.1.4.7.0";
	std::string oidStsiSntpClientMode =					"1.3.6.1.4.1.193.177.2.3.1.4.8.0";
	std::string oidStsiIslEthIn =						"1.3.6.1.4.1.193.177.2.3.1.3.3.0";
	std::string oidStsiSntpPartnerIp =					"1.3.6.1.4.1.193.177.2.3.1.4.9.1.2";
	std::string oidStsiSntpStatus =						"1.3.6.1.4.1.193.177.2.3.1.2.8";
	std::string oidStsiSntpSelectedRefIndex =			"1.3.6.1.4.1.193.177.2.3.1.4.6";
	std::string oidStsiSntpAssocIp =					"1.3.6.1.4.1.193.177.2.3.1.4.10.1.2";
	std::string oidStsiSntpAssocStatus =				"1.3.6.1.4.1.193.177.2.3.1.4.10.1.3";
	std::string oidStsiGpsStatus =						"1.3.6.1.4.1.193.177.2.3.1.2.12";


	//GEN-SYSMGR-MIB
	std::string oidSystemTime =							"1.3.6.1.4.1.193.177.2.2.5.5.3.0";
	std::string oidSysMgrAliasIpAddr =					"1.3.6.1.4.1.193.177.2.2.5.7.1.1.2";
	std::string oidSysMgrAliasDefaultGateway =			"1.3.6.1.4.1.193.177.2.2.5.7.1.1.3";
	std::string oidSysMgrAliasNetMask =					"1.3.6.1.4.1.193.177.2.2.5.7.1.1.4";
	std::string oidSysMgrAliasRealDevName =				"1.3.6.1.4.1.193.177.2.2.5.7.1.1.5";
	std::string oidSysMgrAliasRowStatus =				"1.3.6.1.4.1.193.177.2.2.5.7.1.1.6";

	//GEN-SHELF-MIB
	std::string oidShelfMgrCtrlSCX =            "1.3.6.1.4.1.193.177.2.2.1.2.1.8.0";
	//std::string oidShelfMgrCtrl2SCX =            "1.3.6.1.4.1.193.177.2.2.1.2.1.8";

	//IF-MIB --- TO BE REMOVED WHEN APBM IS READY
	std::string oidIfAdmin27 = "1.3.6.1.2.1.2.2.1.7.27";

//namespace {
//	int SWM_LOADEDTABLE_ENTRIES = 4;
//	//int DUMMY_SLOT = 100;
//}

//FIXS_TREFC_SNMPTrapReceiver *trapReceiver;

ACE_Recursive_Thread_Mutex FIXS_TREFC_SNMPManager::m_lock;  // TR_HX84357

FIXS_TREFC_SNMPManager::FIXS_TREFC_SNMPManager(std::string ipAddress){

	bExit = false;
	bRun = false;
//	trapReceiver = NULL;

	ipAddr = ipAddress;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " ipAddr: " << ipAddr << std::endl;

	init_snmp("TrefcSnmpManager");
	//if (initialize()) {
	//	std::cout << __FUNCTION__ << "@" << __LINE__ << " Init Failed, thread not started" << std::endl;
	//}
}

FIXS_TREFC_SNMPManager::~FIXS_TREFC_SNMPManager(){

	//std::cout << __FUNCTION__ << "@" << __LINE__ << " destroyer Begin " << std::endl;

//	if (trapReceiver != NULL) {
//		if (trapReceiver->isRunning()) this->stopTrapReceiver();
//
//		delete trapReceiver;
//		trapReceiver = NULL;
//	}
	//std::cout << __FUNCTION__ << "@" << __LINE__ << " destroyer end " << std::endl;
}


int FIXS_TREFC_SNMPManager::snmpGet(const char* community, const char* peername,const char* usrOID, int type, void* out, unsigned int iBufSize, int strType)
{

	if (bExit)
		return GETSET_SNMP_STATUS_TIMEOUT;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);  // TR_HX84357

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

//	std::cout << __FUNCTION__ << "@" << __LINE__ <<	" init snmp session ok ... IP: " << peername <<std::endl;

	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = 5;
	//snmpSession.timeout = 1000000;
	snmpSession.timeout = 200000;
	snmpSession.community_len = strlen((const char *)snmpSession.community);
	snmpSession.peername = strdup(peername);

//	std::cout << __FUNCTION__ << "@" << __LINE__ <<	" init snmp session ok ... IP: " << snmpSession.peername <<std::endl;

	//snmpSessionHandler = snmp_open(&snmpSession);
	snmpSess = snmp_sess_open(&snmpSession);

	if(snmpSess == NULL)
	{
		std::cout << "DBG: " <<__FUNCTION__ << "@" << __LINE__ <<" ERROR: snmp_open failed - snmp_errno: " << snmp_errno << std::endl;

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
		std::cout <<"DBG: "<< __FUNCTION__ << "@" << __LINE__ <<" ERROR: Transform OID Failed " <<std::endl;
		//snmp_close(snmpSessionHandler);
		snmp_sess_close(snmpSess);
		return GETSET_SNMP_PARSE_OID_ERROR;
	}

	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_GET)) != NULL)
	{
		snmp_add_null_var(snmpPDU, snmp_oid, snmp_oid_len);
		std::cout <<"DBG: "<< __FUNCTION__ << "@" << __LINE__ <<" Send request ! " <<std::endl;

		//usleep(100000);
		//res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);
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
								cout << "received string" << endl;
								switch (strType)
								{

									case FIXS_TREFC_SNMPManager::SNMPGET_STD_STRING:
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

									case FIXS_TREFC_SNMPManager::SNMPGET_HEX_STRING:
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

									case FIXS_TREFC_SNMPManager::SNMPGET_ASCII_STRING:
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
							else if ((varsList->type == type) && (type == ASN_IPADDRESS))
							{
								pchBuffer = reinterpret_cast<char*>(out);
								iReqSize = snprint_ipaddress( pchBuffer, iBufSize, varsList, NULL, NULL, NULL );
								// Check error
								if(iReqSize < 0){
									// Error. Is the Buffer too small?
									std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: SNMPGET_IPADDRESS can't write in the buffer. Is buffer too small?" << std::endl;
									getResult = GETSET_SNMP_BUFFERSMALL;
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

	snmp_free_pdu(snmpResp);

	//snmp_close(snmpSessionHandler);
	snmp_sess_close(snmpSess);

	return getResult;
}


int FIXS_TREFC_SNMPManager::snmpSet(const char* community, const char* peername,const char* usrOID,  char type, const char *value)
{
	/*
		TYPE: one of i, u, t, a, o, s, x, d, b
			i: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS
			o: OBJID, s: STRING, x: HEX STRING, d: DECIMAL STRING, b: BITS
			U: unsigned int64, I: signed int64, F: float, D: double
	*/

	if (bExit)
		return GETSET_SNMP_STATUS_TIMEOUT;

        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);     // TR_HX84357

	snmp_session snmpSession;
	//snmp_session *snmpSessionHandler;
	void *snmpSess;

	int setResult = GETSET_SNMP_OK;
	struct snmp_pdu *snmpPDU;
	struct snmp_pdu *snmpResp;
	int res;

	snmp_sess_init( &snmpSession );

	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = 3;
	snmpSession.timeout = 1000000;
	//snmpSession.timeout = 500000;
	snmpSession.community_len = strlen((const char *)snmpSession.community);
	snmpSession.peername = strdup(peername);

	std::cout << __FUNCTION__ << "@" << __LINE__ <<" init snmp session ok ... IP: " << snmpSession.peername <<std::endl;

	//snmpSessionHandler = snmp_open(&snmpSession);
	snmpSess = snmp_sess_open(&snmpSession);
	if(snmpSess == NULL)
	{
		std::cout << "DBG: " <<__FUNCTION__ << "@" << __LINE__ <<" ERROR: snmp_open failed - snmp_errno: " << snmp_errno << std::endl;

		return OPEN_SESS_ERROR;
	}

	//snmpSessionHandler=snmp_sess_session(snmpSess);

	free(snmpSession.community);
	free(snmpSession.peername);

//	if(snmpSessionHandler == NULL)
//	{
//		std::cout << __FUNCTION__ << "@" << __LINE__ <<	" snmp_sess_session failed - snmp_errno: " << snmp_errno << std::endl;
//		return OPEN_SESS_ERROR;
//	}

	//snmpPDU = snmp_pdu_create(SNMP_MSG_SET);

	oid snmp_oid[MAX_OID_LEN];
	size_t snmp_oid_len = MAX_OID_LEN;

	if (! read_objid(usrOID, snmp_oid, &snmp_oid_len) )
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Error: "	<< snmp_errstring(snmp_errno) << std::endl;
		//snmp_close(snmpSessionHandler);
		snmp_sess_close(snmpSess);
		return GETSET_SNMP_PARSE_OID_ERROR;
	}

	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_SET)) != NULL)
	{

		if (snmp_add_var(snmpPDU, snmp_oid, snmp_oid_len, type, value) )
		{

			//std::cout << __FUNCTION__ << "@" << __LINE__ << " snmp_add_var error: "	<< snmp_errstring(snmpResp->errstat) << std::endl;

			//snmp_close(snmpSessionHandler);
			snmp_sess_close(snmpSess);
			return GETSET_SNMP_PARSE_OID_ERROR;

		}

		// send pdu...
		//res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);
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

	snmp_free_pdu(snmpResp);
	//snmp_close(snmpSessionHandler);
	snmp_sess_close(snmpSess);
	return setResult;
}


std::string FIXS_TREFC_SNMPManager::getIpAddress(void)
{
	return std::string(ipAddr);
}

void FIXS_TREFC_SNMPManager::setIpAddress(std::string ip)
{
	ipAddr = ip.c_str();
}



bool FIXS_TREFC_SNMPManager::isMaster ()
{
	//oidShelfMgrCtrlSCX
	int res;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	if (isCBAenvironment())
	{
		tmpOID = oidShelfMgrCtrlSCX + ".0";
	}
	else return false; //TODO

	//192.168.170.60
	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF, FIXS_TREFC_SNMPManager::SNMPGET_HEX_STRING);

	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		std::string shelfMgrCtrlState = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << shelfMgrCtrlState << std::endl;

		int masterSCB = 0;
		if (shelfMgrCtrlState.length() >= 2)
		{
			std::string tmp = shelfMgrCtrlState.substr(0,2);
			masterSCB = ::atoi(tmp.c_str());
		}
		else
		{
			masterSCB = ::atoi(shelfMgrCtrlState.c_str());
		}

		if (masterSCB == 3) //Active
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ipAddr: " << ipAddr << " is master." << std::endl;
			return true;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ipAddr: " << ipAddr << " is passive." << std::endl;
			return false;
		}
	}
	else
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;

	return false;
}

int FIXS_TREFC_SNMPManager::isMaster (std::string ip)
{
	//oidShelfMgrCtrlSCX
	int resp = 0;
	int res;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	if (isCBAenvironment())
	{
		tmpOID = oidShelfMgrCtrlSCX + ".0";
	}
	else return false; //TODO

	setIpAddress(ip);

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF,FIXS_TREFC_SNMPManager::SNMPGET_HEX_STRING);

	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		std::string shelfMgrCtrlState = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << shelfMgrCtrlState << std::endl;

		int masterSCB = 0;
		if (shelfMgrCtrlState.length() >= 2)
		{
			std::string tmp = shelfMgrCtrlState.substr(0,2);
			masterSCB = ::atoi(tmp.c_str());
		}
		else
		{
			masterSCB = ::atoi(shelfMgrCtrlState.c_str());
		}

		if (masterSCB == 3) //Active
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ipAddr: " << ipAddr << " is master." << std::endl;
			resp = 1;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ipAddr: " << ipAddr << " is passive." << std::endl;
			resp = 2;
		}
	}
	else if (res == GETSET_SNMP_STATUS_TIMEOUT)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		//resp = HARDWARE_NOT_PRESENT;
	}

	return resp;
}

bool FIXS_TREFC_SNMPManager::isCBAenvironment()
{
	//m_cba_environment = CCH_Util::getEnvironment();

	//if ((m_cba_environment == CCH_Util::SINGLECP_CBA) || (m_cba_environment == CCH_Util::MULTIPLECP_CBA)) return true;
	//else return false;
	return true;
}



bool FIXS_TREFC_SNMPManager::setGenNtpEntServer (int index, string valueToSet)
{
	int res = 0;
	string indexStr = TREFC_Util::intToString(index);
	//char indexStr = itoa(index);
	string oidToSet = oidGenNtpEntServer + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidGenNtpEntServer: " << oidToSet.c_str() << endl;
	cout << "server IP: " << valueToSet.c_str() << endl;
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 's', valueToSet.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setGenNtpEntSvrRowStatus (unsigned int action, unsigned int index)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidGenNtpEntSvrRowStatus + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidGenNtpEntSvrRowStatus: " << oidToSet.c_str() << endl;
	cout << "row status: " << action << endl;
	string actionStr("");
	actionStr = TREFC_Util::intToString(action);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'i', actionStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}

bool FIXS_TREFC_SNMPManager::setGenNtpEntSvrRowStatus (unsigned int action, unsigned int index, int &error)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidGenNtpEntSvrRowStatus + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidGenNtpEntSvrRowStatus: " << oidToSet.c_str() << endl;
	cout << "row status: " << action << endl;
	string actionStr("");
	actionStr = TREFC_Util::intToString(action);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'i', actionStr.c_str());

	error = res;

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setGenNtpEntConfigComplete ()
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidGenNtpEntConfigComplete: " << oidGenNtpEntConfigComplete.c_str() << endl;

	string actionStr("");
	actionStr = TREFC_Util::intToString(1);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidGenNtpEntConfigComplete.c_str(), 'u', actionStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setGenNtpEntNtpdRestart ()
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidGenNtpEntNtpdRestart: " << oidGenNtpEntNtpdRestart.c_str() << endl;

	string actionStr("");
	actionStr = TREFC_Util::intToString(1);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidGenNtpEntNtpdRestart.c_str(), 'u', actionStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiSyncInput (int syncInput)
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiSyncInput: " << oidStsiSyncInput.c_str() << endl;
	cout << "syncInput : " << syncInput << endl;

	string syncInputStr("");
	syncInputStr = TREFC_Util::intToString(syncInput);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidStsiSyncInput.c_str(), 'i', syncInputStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}

bool FIXS_TREFC_SNMPManager::setStsiTuRole (int tuRole)
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiTuRole: " << oidStsiTuRole.c_str() << endl;
	cout << "tuRole : " << tuRole << endl;

	string tuRoleStr("");
	tuRoleStr = TREFC_Util::intToString(tuRole);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidStsiTuRole.c_str(), 'i', tuRoleStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiTodServer (int todServer)
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiTodServer: " << oidStsiTodServer.c_str() << endl;
	cout << "todServer : " << todServer << endl;

	string todServerStr("");
	todServerStr = TREFC_Util::intToString(todServer);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidStsiTodServer.c_str(), 'i', todServerStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}



bool FIXS_TREFC_SNMPManager::setStsiTodRef (int todRef)
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiTodRef: " << oidStsiTodRef.c_str() << endl;


	string todRefStr("");
	todRefStr = TREFC_Util::intToString(todRef);
	cout << "todRef : " << todRefStr << endl;
	res = snmpSet("NETMAN", ipAddr.c_str(), oidStsiTodRef.c_str(), 'i', todRefStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setSystemTime ()
{
	int res = 0;

	char byte[512] = {0};
	unsigned int sizeDt = 0;
	TREFC_Util::getHexTime(byte, sizeDt);

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidSystemTime: " << oidSystemTime.c_str() << endl;
	cout << "hex time : " << byte << endl;
	//string byteString = byte;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidSystemTime.c_str(), 'x', byte);

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}



bool FIXS_TREFC_SNMPManager::setStsiNtpRefIp (int index, string ntpRefIp)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidstsiNtpRefIp + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidstsiNtpRefIp: " << oidToSet.c_str() << endl;
	cout << "ntpRef ip address : " << ntpRefIp << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'a', ntpRefIp.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiNtpRefRowStatus (unsigned int action, unsigned int index)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidstsiNtpRefRowStatus + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidstsiNtpRefRowStatus: " << oidToSet.c_str() << endl;
	cout << "row status: " << action << endl;
	string actionStr("");
	actionStr = TREFC_Util::intToString(action);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'i', actionStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}

bool FIXS_TREFC_SNMPManager::setStsiNtpRefRowStatus (unsigned int action, unsigned int index, int &error)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidstsiNtpRefRowStatus + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidstsiNtpRefRowStatus: " << oidToSet.c_str() << endl;
	cout << "row status: " << action << endl;
	string actionStr("");
	actionStr = TREFC_Util::intToString(action);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'i', actionStr.c_str());

	error = res;

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setsysMgrAliasIpAddr (int index, string aliasIp)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidSysMgrAliasIpAddr + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidSysMgrAliasIpAddr: " << oidToSet.c_str() << endl;
	cout << "alias ip address : " << aliasIp << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'a', aliasIp.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setsysMgrAliasDefaultGateway (int index, string aliasDefGateway)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidSysMgrAliasDefaultGateway + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidSysMgrAliasDefaultGateway: " << oidToSet.c_str() << endl;
	cout << "Alias Default Gateway : " << aliasDefGateway << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'a', aliasDefGateway.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setsysMgrAliasNetMask (int index, string aliasNetmask)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidSysMgrAliasNetMask + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidSysMgrAliasNetMask: " << oidToSet.c_str() << endl;
	cout << "Alias Default Gateway : " << aliasNetmask << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'a', aliasNetmask.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setsysMgrAliasRowStatus (unsigned int action, unsigned int index)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidSysMgrAliasRowStatus + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidSysMgrAliasRowStatus: " << oidToSet.c_str() << endl;
	cout << "row status: " << action << endl;
	string actionStr("");
	actionStr = TREFC_Util::intToString(action);
	cout << "actionStr : " << actionStr << endl;
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'i', actionStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
		return true;
	}
	else if(res == GETSET_SNMP_RESPONSE_ERROR)
	{
		return true;
	}
	return false;
}

bool FIXS_TREFC_SNMPManager::setsysMgrAliasRealDevName (int index, string aliasRealDev)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidSysMgrAliasRealDevName + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidSysMgrAliasRealDevName: " << oidToSet.c_str() << endl;
	cout << "Alias RealDev  : " << aliasRealDev << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 's', aliasRealDev.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiIslEthIn (int value)
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiIslEthIn: " << oidStsiIslEthIn.c_str() << endl;


	string ethInStr("");
	ethInStr = TREFC_Util::intToString(value);
	cout << "eth in : " << ethInStr << endl;
	res = snmpSet("NETMAN", ipAddr.c_str(), oidStsiIslEthIn.c_str(), 'i', ethInStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiSntpSetCpuClock (int value)
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiSntpSetCpuClock: " << oidStsiSntpSetCpuClock.c_str() << endl;

	string cpuClockStr("");
	cpuClockStr = TREFC_Util::intToString(value);
	cout << "Cpu Clock Status : " << cpuClockStr << endl;
	res = snmpSet("NETMAN", ipAddr.c_str(), oidStsiSntpSetCpuClock.c_str(), 'i', cpuClockStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiSntpClientMode (int value)
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiSntpClientMode: " << oidStsiSntpClientMode.c_str() << endl;

	string clientModeStr("");
	clientModeStr = TREFC_Util::intToString(value);
	cout << "Client Mode : " << clientModeStr << endl;
	res = snmpSet("NETMAN", ipAddr.c_str(), oidStsiSntpClientMode.c_str(), 'i', clientModeStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}

bool FIXS_TREFC_SNMPManager::setStsiSntpIfIp (int index, string sntpIfIp)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidStsiSntpIfIp + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiSntpIfIp: " << oidToSet.c_str() << endl;
	cout << "sntpIfIp  : " << sntpIfIp << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'a', sntpIfIp.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}



bool FIXS_TREFC_SNMPManager::setStsiSntpIfRowStatus (unsigned int action, unsigned int index)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidStsiSntpIfRowStatus + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiSntpIfRowStatus: " << oidToSet.c_str() << endl;
	cout << "row status: " << action << endl;
	string actionStr("");
	actionStr = TREFC_Util::intToString(action);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'i', actionStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiSntpIfIpMask (int index, string sntpIfIpMask)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidStsiSntpIfIpMask + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiSntpIfIpMask: " << oidToSet.c_str() << endl;
	cout << "sntpIfIpMask  : " << sntpIfIpMask << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'a', sntpIfIpMask.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiSntpIfVlan (int index, unsigned int sntpIfVlan)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidStsiSntpIfVlan + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiSntpIfVlan: " << oidToSet.c_str() << endl;
	cout << "sntpIfVlan: " << sntpIfVlan << endl;
	string sntpIfVlanStr("");
	sntpIfVlanStr = TREFC_Util::intToString(sntpIfVlan);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'u', sntpIfVlanStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setGenNtpEntWhiteInterface (int index, string interface)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidGenNtpEntWhiteInterface + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidGenNtpEntWhiteInterface: " << oidToSet.c_str() << endl;
	cout << "interface  : " << interface << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 's', interface.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setGenNtpEntWhiteRowStatus (unsigned int action, unsigned int index)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidGenNtpEntWhiteRowStatus + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidGenNtpEntWhiteRowStatus: " << oidToSet.c_str() << endl;
	cout << "row status: " << action << endl;
	string actionStr("");
	actionStr = TREFC_Util::intToString(action);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'i', actionStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}

bool FIXS_TREFC_SNMPManager::setGenNtpEntWhiteRowStatus (unsigned int action, unsigned int index, int &error)
{
	int res = 0;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidGenNtpEntWhiteRowStatus + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidGenNtpEntWhiteRowStatus: " << oidToSet.c_str() << endl;
	cout << "row status: " << action << endl;
	string actionStr("");
	actionStr = TREFC_Util::intToString(action);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'i', actionStr.c_str());

	error = res;

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::setStsiSntpPartnerIp (unsigned int index, string partnerIp)
{
	int res = 0;

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);
		//char indexStr = itoa(index);
	string oidToSet = oidStsiSntpPartnerIp + "." + indexStr;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidStsiSntpPartnerIp: " << oidToSet.c_str() << endl;
	cout << "partnerIp  : " << partnerIp << endl;

	res = snmpSet("NETMAN", ipAddr.c_str(), oidToSet.c_str(), 'a', partnerIp.c_str());

	if (res == GETSET_SNMP_OK)
	{
	 return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::getStsiSntpAssocIp (unsigned int index, string *sntpAssocIp)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");
	cout << "!!!!!!!!!!!!!!!!!!!!! index = " << index << endl;
	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidStsiSntpAssocIp + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_IPADDRESS, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		*sntpAssocIp = string((char *)out);
		cout << "------------------------------------" << endl;
		cout<<*sntpAssocIp<<endl;
	}
	else {

		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_TREFC_SNMPManager::getStsiSntpAssocStatus (unsigned int index, int *sntpAssocStatus)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidStsiSntpAssocStatus + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		*sntpAssocStatus = atoi(buffer);

		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else {
		//input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_TREFC_SNMPManager::getStsiSntpSelectedRefIndex (unsigned int index, int *sntpSelectedIndex)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidStsiSntpSelectedRefIndex + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		*sntpSelectedIndex = atoi(buffer);
		cout << "selected ref index = " << *sntpSelectedIndex << endl;
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else {
		//input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_TREFC_SNMPManager::getStsiSntpStatus (unsigned int index, int *sntpStatus)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidStsiSntpStatus + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		*sntpStatus = atoi(buffer);

		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else {
		//input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_TREFC_SNMPManager::getStsiNtpRefIp (unsigned int index, string *stsiNtpRefIp)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidstsiNtpRefIp + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_IPADDRESS, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		*stsiNtpRefIp = string((char *)out);
		*stsiNtpRefIp = ACS_APGCC::after(*stsiNtpRefIp, "IpAddress: ");
		cout<<*stsiNtpRefIp<<endl;
	}
	else {
		//input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool FIXS_TREFC_SNMPManager::getStsiNtpRefRowStatus (unsigned int index, int *value)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidstsiNtpRefRowStatus + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		*value = atoi(buffer);

		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else {
		//input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_TREFC_SNMPManager::getGenNtpEntStatusCurrentMode (unsigned int index, int *ntpCurrMode)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidGenNtpEntStatusCurrentMode + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
				std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		*ntpCurrMode = atoi(buffer);

				std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << ntpCurrMode << std::endl;
	}
	else {
		//input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_TREFC_SNMPManager::getGenNTPEntStatusActiveRefSourceId (unsigned int index, string *ntpAddress)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidGenNTPEntStatusActiveRefSourceId + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		*ntpAddress = string((char *)out);
		cout<<*ntpAddress<<endl;
	}
	else {

		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_TREFC_SNMPManager::getGenNTPEntServer (unsigned int index, string *ntpAddress)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidGenNtpEntServer + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		*ntpAddress = string((char *)out);
		cout<<*ntpAddress<<endl;
	}
	else {
		//input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool FIXS_TREFC_SNMPManager::getstsiGpsStatus (unsigned int index, int *gpsStatus)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = TREFC_Util::intToString(index);

	tmpOID = oidStsiGpsStatus + "." + indexStr;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		*gpsStatus = atoi(buffer);

		//		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << input << std::endl;
	}
	else {
		//input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}



//TO BE REMOVED AFTER APBM IS READY --begin
bool FIXS_TREFC_SNMPManager::setIfAdminStatus27up ()
{
	int res = 0;

	cout << "ipAddr: " << ipAddr.c_str() << endl;
	cout << "oidIfAdmin27: " << oidIfAdmin27.c_str() << endl;

	string actionStr("");
	actionStr = TREFC_Util::intToString(1);
	res = snmpSet("NETMAN", ipAddr.c_str(), oidIfAdmin27.c_str(), 'i', actionStr.c_str());

	if (res == GETSET_SNMP_OK)
	{
		return true;
	}

	return false;
}


bool FIXS_TREFC_SNMPManager::getIfAdminStatus27 (int *adminStatus)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];


	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get from ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), oidIfAdmin27.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));

		*adminStatus = atoi(buffer);

	}
	else
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << oidIfAdmin27.c_str() << std::endl;
		return false;
	}

	return true;
}
//TO BE REMOVED AFTER APBM IS READY --end

void FIXS_TREFC_SNMPManager::stopWork() {
	bExit = true;
}
