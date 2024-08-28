
#include "FIXS_CCH_SNMPManager.h"


	/************************************************************/
	/*							OID								*/
	/************************************************************/

	//-----------------UPGPKG------------------------//
	//GEN-SWM-MIB
	std::string oidSwmAction =					"1.3.6.1.4.1.193.177.2.2.3.3.1";
	std::string oidSwmActionVersion =			"1.3.6.1.4.1.193.177.2.2.3.3.2";
	std::string oidSwmActionURL =				"1.3.6.1.4.1.193.177.2.2.3.3.3";
	std::string oidSwmActionStatus =			"1.3.6.1.4.1.193.177.2.2.3.3.4";

	std::string oidSwmLoadedVersion =			"1.3.6.1.4.1.193.177.2.2.3.2.1.1.2";
	std::string oidSwmLoadedType =				"1.3.6.1.4.1.193.177.2.2.3.2.1.1.3";
	std::string oidSwmLoadedAttribute =			"1.3.6.1.4.1.193.177.2.2.3.2.1.1.4";

	std::string oidBladeReset =					"1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.12";
	std::string oidGprDataValue =               "1.3.6.1.4.1.193.177.2.2.1.3.3.1.1.3";
	std::string oidBladePower	=               "1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.43";
	std::string oidBladeRestart	=               "1.3.6.1.4.1.193.177.2.2.5.5.4";

	std::string oidShelfMgrCmd1 =	            "1.3.6.1.4.1.193.154.2.1.2.1.1.1.12"; //to check SCB_RP master
	//AXE-ETH-SHELF-MIB
	std::string oidXshmcGPRData =				"1.3.6.1.4.1.193.154.2.1.2.2.1.1.2";
	std::string oidxshmcBoardYellowLed =		"1.3.6.1.4.1.193.154.2.1.2.1.1.1.8";
	std::string oidXshmcBoardReset =			"1.3.6.1.4.1.193.154.2.1.2.1.1.1.9";
	std::string oidXshmcBoardPwrOff =			"1.3.6.1.4.1.193.154.2.1.2.1.1.1.10";
	std::string oidXshmcBoardPwrOn =			"1.3.6.1.4.1.193.154.2.1.2.1.1.1.11";
	//std::string oidXshmcBoardPresenceSCX[] =	"1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.19"; // Board present
	std::string oidXshmcBoardPresenceSCX =	"1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.19"; // Board present


	//------------------ GEN-BLADE-MIB ------------------
	std::string oidBmBladeReturnToFactoryDefault = "1.3.6.1.4.1.193.177.2.2.2.2.1.22.0";
	std::string oidCMXBladeReset =				   "1.3.6.1.4.1.193.177.2.2.2.2.1.17";
	//-------------------IPMI------------------------//
	std::string oidBoardIpmiUpgradeStatusSCX =	"1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.17";
	std::string oidBoardIpmiUpgradeSCX =		"1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.16";
	std::string oidBoardIpmiDataSCX =			"1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.15";

	//OID ShelfMgrCtrlName
	std::string oidShelfMgrCtrlSCX =            "1.3.6.1.4.1.193.177.2.2.1.2.1.8";

	//-----------------PFMUPG------------------------//
	std::string oidPfmScanning =				"1.3.6.1.4.1.193.177.2.2.6.2.3.1";
	std::string oidPfmFwUpgrade =   			"1.3.6.1.4.1.193.177.2.2.6.2.2.1.1.3";
	std::string oidPfmFwUpgrageStatus =			"1.3.6.1.4.1.193.177.2.2.6.2.2.1.1.4";
	std::string oidPfmFwProductNumber =			"1.3.6.1.4.1.193.177.2.2.6.2.2.1.1.9";
	std::string oidPfmFwProductRevisionState =	"1.3.6.1.4.1.193.177.2.2.6.2.2.1.1.10";
	std::string oidPfmFwModuleDeviceType =	    "1.3.6.1.4.1.193.177.2.2.6.2.1.1.1.11";
	
	//-----------------PFMSWUPG------------------------//
	std::string oidPfmSwUpgrade = "1.3.6.1.4.1.193.177.2.2.6.2.2.2.1.4";
	std::string oidPfmSwInventoryBlProductNumber =	"1.3.6.1.4.1.193.177.2.2.6.2.2.3.1.9";
	std::string oidPfmSwInventoryBlProductRevisionState =	"1.3.6.1.4.1.193.177.2.2.6.2.2.3.1.10";
	std::string oidPfmSwInventoryUpgProductNumber =	"1.3.6.1.4.1.193.177.2.2.6.2.2.3.1.11";
	std::string oidPfmSwInventoryUpgProductRevisionState =	"1.3.6.1.4.1.193.177.2.2.6.2.2.3.1.12";	
	std::string oidpfmSwInventoryCurrentSwType = "1.3.6.1.4.1.193.177.2.2.6.2.2.3.1.21";
	std::string oidPfmSwUpgrageStatus =			"1.3.6.1.4.1.193.177.2.2.6.2.2.2.1.5";
	std::string oidPfmInitialProductNumber = "1.3.6.1.4.1.193.177.2.2.6.2.2.3.1.3";											 
	std::string oidPfmInitialProductRevision = "1.3.6.1.4.1.193.177.2.2.6.2.2.3.1.4";
	
	//FWM LOADED TABLE
	std::string oidFwmLoadedAttributeSCX =      "1.3.6.1.4.1.193.177.2.2.3.2.2.1.4";
	std::string oidFwmLoadedTypeSCX =           "1.3.6.1.4.1.193.177.2.2.3.2.2.1.2";
	std::string oidFwmLoadedVersionSCX =        "1.3.6.1.4.1.193.177.2.2.3.2.2.1.3";
	std::string oidbmBladeProductRevisionState = "1.3.6.1.4.1.193.177.2.2.2.2.1.3";
	std::string oidFwmAction =					"1.3.6.1.4.1.193.177.2.2.3.3.6";
	std::string oidFwmActionURL =				"1.3.6.1.4.1.193.177.2.2.3.3.5";
	std::string oidFwmActionStatus =			"1.3.6.1.4.1.193.177.2.2.3.3.7";
	std::string oidBladeBIOSPointer =			"1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.23";
	std::string oidFwmActionResult =			"1.3.6.1.4.1.193.177.2.2.3.3.9.0";


namespace {
	int SWM_LOADEDTABLE_ENTRIES = 4;
	//int DUMMY_SLOT = 100;
}


FIXS_CCH_SNMPManager::FIXS_CCH_SNMPManager(std::string ipAddress){

	bExit = false;
	bRun = false;
	ipAddr = ipAddress;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " ipAddr: " << ipAddr << std::endl;

	ACS_TRA_Logging *FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_SNMPManager (NEW) on IP Address(%s) ", ipAddress.c_str());
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);


	init_snmp("SnmpManager");
	m_cba_environment = CCH_Util::UNKNOWN;
}

FIXS_CCH_SNMPManager::~FIXS_CCH_SNMPManager(){

}

int FIXS_CCH_SNMPManager::snmpGet(const char* community, const char* peername,const char* usrOID, int type, void* out, unsigned int iBufSize, int strType)
{
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

									case FIXS_CCH_SNMPManager::SNMPGET_STD_STRING:
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

									case FIXS_CCH_SNMPManager::SNMPGET_HEX_STRING:
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

									case FIXS_CCH_SNMPManager::SNMPGET_ASCII_STRING:
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
							else if ((varsList->type == type) && (type == ASN_GAUGE))
                                                        {

                                                                if(iBufSize >= sizeof(int)){
                                                                        piBuffer = reinterpret_cast<int*>(out);
                                                                        *piBuffer = (int)(*(varsList->val.integer));
                                                                }else{
                                                                        // Error. Is the Buffer too small?
                                                                        std::cout << __FUNCTION__ << "@" << __LINE__ << "### ERROR: ASN_GAUGE buffer error!" << std::endl;
                                                                        getResult = GETSET_SNMP_BUFFERSMALL;
                                                                }
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


int FIXS_CCH_SNMPManager::snmpSet(const char* community, const char* peername,const char* usrOID,  char type, const char *value)
{
	/*
		TYPE: one of i, u, t, a, o, s, x, d, b
			i: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS
			o: OBJID, s: STRING, x: HEX STRING, d: DECIMAL STRING, b: BITS
			U: unsigned int64, I: signed int64, F: float, D: double
	*/

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
//	snmpSession.timeout = 300000;
        snmpSession.timeout = 600000;  //HU23028 
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


std::string FIXS_CCH_SNMPManager::getIpAddress(void)
{
	return std::string(ipAddr);
}

void FIXS_CCH_SNMPManager::setIpAddress(std::string ip)
{
	ipAddr = ip.c_str();
}

bool FIXS_CCH_SNMPManager::getSwmActionStatus (int &actStatus)
{
	std::string tmpOID = oidSwmActionStatus + ".0";

	int res;
	BYTE out[SNMP_MAXBUF];

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);

	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		actStatus = atoi(buffer);

		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << actStatus << std::endl;
	}
	else {
		actStatus = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}


bool FIXS_CCH_SNMPManager::getSwmLoadedVersion (unsigned int index, std::string &loadVer)
{
	std::stringstream ss;
	ss << index;
	std::string tmpOID = oidSwmLoadedVersion + "." +  ss.str();

	int res;
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out,SNMP_MAXBUF);
	loadVer.clear();
	if (res == GETSET_SNMP_OK) {
		char buffer[128];
		sprintf(buffer, "%s", ( char *) out);
		//loadVer = std::string(buffer);
		loadVer = std::string((char*) buffer);
		//std::string *tmp = (std::string*) out;
		//loadVer = std::string(tmp->c_str());
		//loadVer = std::string( ((std::string*)out)->c_str() );
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << loadVer << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		loadVer = "";
		return false;
	}

	return true;
}



bool FIXS_CCH_SNMPManager::getCurrentDefaultVersion (std::string &loadVer)
{
	int res;
	BYTE out[SNMP_MAXBUF];
	int indx = 0;
	std::stringstream ss;

	loadVer = "";
	int iLoadedType = -1;
	int iLoadedAttr = -1;

	for (indx = 1; indx <= SWM_LOADEDTABLE_ENTRIES; indx++)
	{
		ss.str("");
		ss << indx;
		std::string tmpOID_LoadedType = oidSwmLoadedType + "." +  ss.str();
		std::cout << __FUNCTION__ << "@" << __LINE__ << " OID: " << tmpOID_LoadedType.c_str() << std::endl;
		res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID_LoadedType.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);

		if (res == GETSET_SNMP_OK) {
			char buffer[128];
			sprintf(buffer, "%d", *((int*)&out));
			std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

			iLoadedType = atoi(buffer);

			std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << iLoadedType << std::endl;
		}
		else {
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID_LoadedType.c_str() << std::endl;
			return false;
		}

		std::string tmpOID_LoadedAttr = oidSwmLoadedAttribute + "." +  ss.str();
		std::cout << __FUNCTION__ << "@" << __LINE__ << " OID: " << tmpOID_LoadedAttr.c_str() << std::endl;
		res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID_LoadedAttr.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);

		if (res == GETSET_SNMP_OK) {
			char buffer[128];
			sprintf(buffer, "%d", *((int*)&out));
			std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

			iLoadedAttr = atoi(buffer);

			std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << iLoadedAttr << std::endl;
		}
		else {
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID_LoadedAttr.c_str() << std::endl;
			return false;
		}

		if((iLoadedType == FIXS_CCH_SNMPManager::EXCHANGEBLE) && (iLoadedAttr == FIXS_CCH_SNMPManager::CURRENTDEFAULTSW))
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " EXCHANGEBLE - CURRENTDEFAULTSW = OK" << std::endl;
			break;
		}
	}

	if (indx <= SWM_LOADEDTABLE_ENTRIES) {

		std::string tmpOID_LoadedVer = oidSwmLoadedVersion + "." +  ss.str();
		res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID_LoadedVer.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);

		loadVer.clear();
		if (res == GETSET_SNMP_OK) {

			static char buffer[128];
			sprintf(buffer, "%s\n", (char *) out);
			loadVer = std::string((char*) out);
		//	char buffer[128]={0};
		//	memcpy(buffer,( char *) out,SNMP_MAXBUF);
			//sprintf(buffer, "%s", ( char *) out);
			//loadVer = std::string(buffer);
			//loadVer = std::string( ((std::string*)out)->c_str() );
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << loadVer.c_str() << std::endl;
		}
		else {
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID_LoadedVer.c_str() << std::endl;
			return false;
		}
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Data not found !" << std::endl;
		loadVer = "";
		return false;
	}

	return true;
}

bool FIXS_CCH_SNMPManager::getCurrentLoadedVersion (std::string &loadVer)
{

	int res;
	BYTE out[SNMP_MAXBUF];
	int indx = 0;
	std::stringstream ss;

	loadVer = "";
	int iLoadedType = -1;
	int iLoadedAttr = -1;

	for (indx = 1; indx <= SWM_LOADEDTABLE_ENTRIES; indx++)
	{
		ss.str("");
		ss << indx;
		std::string tmpOID_LoadedType = oidSwmLoadedType + "." +  ss.str();
		std::cout << __FUNCTION__ << "@" << __LINE__ << " OID: " << tmpOID_LoadedType.c_str() << std::endl;
		res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID_LoadedType.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);

		if (res == GETSET_SNMP_OK) {
			char buffer[128];
			sprintf(buffer, "%d", *((int*)&out));
			std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

			iLoadedType = atoi(buffer);

			std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << iLoadedType << std::endl;
		}
		else {
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID_LoadedType.c_str() << std::endl;
			return false;
		}

		std::string tmpOID_LoadedAttr = oidSwmLoadedAttribute + "." +  ss.str();
		std::cout << __FUNCTION__ << "@" << __LINE__ << " OID: " << tmpOID_LoadedAttr.c_str() << std::endl;
		res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID_LoadedAttr.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);

		if (res == GETSET_SNMP_OK) {
			char buffer[128];
			sprintf(buffer, "%d", *((int*)&out));
			std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

			iLoadedAttr = atoi(buffer);

			std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << iLoadedAttr << std::endl;
		}
		else {
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID_LoadedAttr.c_str() << std::endl;
			return false;
		}

		if((iLoadedType == FIXS_CCH_SNMPManager::EXCHANGEBLE) &&
				(iLoadedAttr == FIXS_CCH_SNMPManager::CURRENTSW || iLoadedAttr == FIXS_CCH_SNMPManager::CURRENTDEFAULTSW))
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " EXCHANGEBLE - CURRENTSW - CURRENTDEFAULTSW = OK" << std::endl;
			break;
		}
	}

	if (indx <= SWM_LOADEDTABLE_ENTRIES) {

		std::string tmpOID_LoadedVer = oidSwmLoadedVersion + "." +  ss.str();
		res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID_LoadedVer.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);

		loadVer.clear();
		if (res == GETSET_SNMP_OK) {
			static char buffer[128];
			sprintf(buffer, "%s\n", (char *) out);
			loadVer = std::string((char*) out);
			//char buffer[128];
			//sprintf(buffer, "%s", ( char *) out);
			//loadVer = std::string(buffer);

			//loadVer = std::string( ((std::string*)out)->c_str() );
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << loadVer << std::endl;
		}
		else {
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID_LoadedVer.c_str() << std::endl;
			return false;
		}
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Data not found !" << std::endl;
		loadVer = "";
		return false;
	}

	return true;
}

bool FIXS_CCH_SNMPManager::checkLoadedVersion (std::string loadVer)
{
	int res;
	//void *out = NULL;
	int indx = 0;
	std::stringstream ss;
	std::string tmpVal("");
	//BYTE out[SNMP_MAXBUF]={0};

	for (indx = 1; indx <= SWM_LOADEDTABLE_ENTRIES; indx++)
	{
		ss.str("");
		ss << indx;
		BYTE out[SNMP_MAXBUF]={0};

		tmpVal.clear();
		std::string tmpOID_LoadedVer = oidSwmLoadedVersion + "." +  ss.str();
		res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID_LoadedVer.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);

		if (res == GETSET_SNMP_OK)
		{
			//char buffer[128]={0};
			//sprintf(buffer, "%s\n", (char *) out);
			tmpVal = std::string((char*) out);

			//std::string tmpVal = std::string( ((std::string*)out)->c_str() );
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << tmpVal.c_str() << std::endl;

			if (strcmp(tmpVal.c_str(),"") == 0) {
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Mib values is empty! " << std::endl;
				return false;
			}
			else if (strcmp(tmpVal.c_str(),loadVer.c_str()) == 0) {
				std::cout << __FUNCTION__ << "@" << __LINE__ << " Check OK, Mib value: " << tmpVal << std::endl;
				return true;
			}

		}
		else if (res == GETSET_SNMP_OK_LEN_ZERO) {
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Values is empty on: " << tmpOID_LoadedVer.c_str() << std::endl;
			return false;
		}
		else {
			std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID_LoadedVer.c_str() << std::endl;
			return false;
		}
	}
	return false;
}

bool FIXS_CCH_SNMPManager::isMaster ()
{
	//oidShelfMgrCtrlSCX
	int res;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	if (isCBAenvironment())
	{
		tmpOID = oidShelfMgrCtrlSCX + ".0";
	}
	else
		return false;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF, FIXS_CCH_SNMPManager::SNMPGET_HEX_STRING);

	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		std::string shelfMgrCtrlState = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << shelfMgrCtrlState.c_str() << std::endl;

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

int FIXS_CCH_SNMPManager::isMaster (std::string ip)
{
	if (ip.empty())
	{
		return HARDWARE_NOT_PRESENT;
	}

	//oidShelfMgrCtrlSCX
	int resp = 0;
	int res;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	setIpAddress(ip);

	if (isCBAenvironment())
	{
		tmpOID = oidShelfMgrCtrlSCX + ".0";
	}
	else
		return false;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF,FIXS_CCH_SNMPManager::SNMPGET_HEX_STRING);

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
		resp = HARDWARE_NOT_PRESENT;
	}

	return resp;
}

int FIXS_CCH_SNMPManager::setSWMAction (unsigned int action){
	int res = 0;
	std::stringstream ss;
	ss << action;

	std::string tmpOID = oidSwmAction + ".0";

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', ss.str().c_str());
	if (res == GETSET_SNMP_OK){
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
		res = 1;
	}
	else if (res == GETSET_SNMP_STATUS_TIMEOUT){
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		res = 2;
	}
	else
	{
		res = HARDWARE_NOT_PRESENT;
	}

	return res;

}


bool FIXS_CCH_SNMPManager::setSWMActionVersion (std::string version) {
	int res = 0;

	std::string tmpOID = oidSwmActionVersion + ".0";

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 's', version.c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CCH_SNMPManager::setSWMActionURL (std::string url){
	int res = 0;

	std::string tmpOID = oidSwmActionURL + ".0";

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 's', url.c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}


bool FIXS_CCH_SNMPManager::resetSwitch (unsigned int slot)
{
	int res = 0;
	std::stringstream ss;
	ss << slot;
	std::string tmpOID("");

	if (isCBAenvironment() && (slot == 0 || slot == 25)) tmpOID = oidBladeReset + "." + ss.str();
	else if (isCBAenvironment() && (slot == 26 || slot == 28)) tmpOID = oidCMXBladeReset + ".0";
	else return false; //TODO

	const char* val = "0"; //cold start

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', val);
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CCH_SNMPManager::getIpmiUpgStatus (std::string slot, int &input)
{
	int res;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	if (isCBAenvironment()) tmpOID = oidBoardIpmiUpgradeStatusSCX + "." + slot;
	else return false; //TODO

	std::cout << __FUNCTION__ << "@" << __LINE__ << " Perform get ip: " << ipAddr << std::endl;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));

		input = atoi(buffer);
	}
	else {
		input = -1;
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool  FIXS_CCH_SNMPManager::setIpmiUpg (std::string slot, std::string valueToSet)
{
	int res = 0;
	std::string tmpOID("");

	if (isCBAenvironment()) tmpOID = oidBoardIpmiUpgradeSCX + "." + slot;
	else return false; //TODO

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

bool FIXS_CCH_SNMPManager::getIPMIBoardData(std::string slot, std::string &running, std::string &product, std::string &revision){

	int res;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	if (isCBAenvironment()) tmpOID = oidBoardIpmiDataSCX + "." + slot;
	else return false; //TODO

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF, FIXS_CCH_SNMPManager::SNMPGET_ASCII_STRING);
	std::cout << __FUNCTION__ << "@" << __LINE__ << " tmpOID: " << tmpOID.c_str() << std::endl;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " -----------------------------------   res snmpGet: " << res << std::endl;

	if (res == GETSET_SNMP_OK) {

		static char buffer[1024];
		sprintf(buffer, "%s", (char *) out);
		std::string boardData = std::string((char*) buffer);

		if ((boardData.length() >= 126) &&
			(boardData.length() <= 169))
		{
			running = boardData.substr(126,1);
			product = boardData.substr(127,25);
			revision = boardData.substr(152,8);
		}
		else
		{
			running = "";
			product = "";
			revision = "";
		}

		//-----------------
		//Remove white char
		//-----------------
		const char *validChar = "ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvxyz0123456789/\\";
		std::string::size_type pos;


		while((pos = product.find_first_not_of(validChar)) != std::string::npos)
		{
			product = product.erase(pos,1);
		}
		while((pos = revision.find_first_not_of(validChar)) != std::string::npos)
		{
			revision = revision.erase(pos,1);
		}


	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
		running = "";
		return false;
	}

	return true;
}

bool FIXS_CCH_SNMPManager::isPassiveDeg ()
{
	//oidShelfMgrCtrlSCX
	int res;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	if (isCBAenvironment()) tmpOID = oidShelfMgrCtrlSCX + ".0";
	else return false; //TODO

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF, FIXS_CCH_SNMPManager::SNMPGET_HEX_STRING);

	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		std::string shelfMgrCtrlState = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << shelfMgrCtrlState << std::endl;

		int intStatus = 0;
		if (shelfMgrCtrlState.length() >= 2)
		{
			std::string tmp = shelfMgrCtrlState.substr(0,2);
			intStatus = ::atoi(tmp.c_str());
		}
		else
		{
			intStatus = ::atoi(shelfMgrCtrlState.c_str());
		}

		if (intStatus == 2) //PassiveDegraded
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ipAddr: " << ipAddr << " PassiveDegraded ." << std::endl;
			return true;
		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ipAddr: " << ipAddr << " SCX OK." << std::endl;
			return false;
		}
	}
	else
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;

	return false;
}


bool FIXS_CCH_SNMPManager::queryBoardPresencePower (int type, std::string &oidShelfpresence)
{
	if (isCBAenvironment())
	{
		if (type==1)
			oidShelfpresence = oidBladePower;

		else if (type==3)
			oidShelfpresence = oidXshmcBoardPresenceSCX;

		else if (type==2)
			oidShelfpresence = oidBladeRestart;

		else if (type==4)
			oidShelfpresence = oidBladeReset;

		else
			return false;
	}
	//else return false; //TODO
	else
	{
		if (type == 1)
			oidShelfpresence = oidBladePower;

		else if (type==3)
			oidShelfpresence = oidXshmcBoardPresenceSCX;

		else if (type==2)
			oidShelfpresence = oidBladeRestart;

		else if (type==4)
			oidShelfpresence = oidBladeReset;

		else
			return false;
	}

	return true;
}

//To check if the board is physically present
int FIXS_CCH_SNMPManager::getBoardPresence (std::string shelfMgrIp,int slot)
{
	int res,boardPresence;
	std::string oidShelfpresence("");

	queryBoardPresencePower (3,oidShelfpresence);
	char* oidBoardpresence= (char*)malloc(100);
	memset(oidBoardpresence,0,100);
	sprintf (oidBoardpresence,"%s.%d",oidShelfpresence.c_str(),slot);
	BYTE out[SNMP_MAXBUF];
	res= snmpGet("PUBLIC",shelfMgrIp.c_str(),oidBoardpresence,ASN_INTEGER,out,SNMP_MAXBUF,0);

	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));
		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		boardPresence = atoi(buffer);

		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << boardPresence << std::endl;
	}
	else {
		boardPresence= -1;

	}
	free(oidBoardpresence);

	return boardPresence;
}

//To check if the board is ON/OFF
int FIXS_CCH_SNMPManager::getBoardPower (string shelfMgrIp,int slot)
{
	int res,boardPower;
	std::string oidShelfPower("");

	queryBoardPresencePower(1,oidShelfPower);
	char* oidBoardPower= (char*)malloc(100);
	memset(oidBoardPower,0,100);
	sprintf(oidBoardPower,"%s.%d",oidShelfPower.c_str(),slot);
	BYTE out[SNMP_MAXBUF];
	res= snmpGet("PUBLIC",shelfMgrIp.c_str(),oidBoardPower,ASN_INTEGER,out,SNMP_MAXBUF,0);

	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer,"%d", *((int*)&out));
		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		boardPower = atoi(buffer);

		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << boardPower << std::endl;
	}
	else {
		boardPower= -1;

	}

	free(oidBoardPower);
	return boardPower;
}


bool FIXS_CCH_SNMPManager::isCBAenvironment()
{
	m_cba_environment = FIXS_CCH_DiskHandler::getEnvironment();

	if ((m_cba_environment == CCH_Util::SINGLECP_CBA) || (m_cba_environment == CCH_Util::MULTIPLECP_CBA) || (m_cba_environment == CCH_Util::MULTIPLECP_SMX) || (m_cba_environment == CCH_Util::SINGLECP_SMX))
	   return true;
	else
	   return false;

}


bool FIXS_CCH_SNMPManager::setCurrentPfmScanningState (int scanningState)
{

	/*  scanningState
			0FF = 0
			ON  = 1 */

	int res = 0;
	std::string sState("");
	sState = CCH_Util::intToString(scanningState);
	std::string oidScanning = oidPfmScanning + ".0";

	cout << "sState: " << sState.c_str() << endl;
	res = snmpSet("NETMAN", ipAddr.c_str(), oidScanning.c_str(), 'i', sState.c_str());

	if (res == GETSET_SNMP_OK)
	{
		return true;
	}

	return false;
}

bool FIXS_CCH_SNMPManager::setPfmFWUpgrade (std::string fwUpgrade, std::string typeIndex, std::string typeInstance)
{
	 int res = 0;
	 std::string oidPfmFWUpgrade = oidPfmFwUpgrade + "." + typeIndex + "." + typeInstance;
	 cout << "ipAddr: " << ipAddr.c_str() << endl;
	 cout << "oidPfmFWUpgrade: " << oidPfmFWUpgrade.c_str() << endl;
	 cout << "fwUpgrade: " << fwUpgrade.c_str() << endl;
	 res = snmpSet("NETMAN", ipAddr.c_str(), oidPfmFWUpgrade.c_str(), 's', fwUpgrade.c_str());

	 if (res == GETSET_SNMP_OK)
	 {
		 return true;
	 }

	 return false;
}

bool FIXS_CCH_SNMPManager::setPfmSWUpgrade (std::string swUpgrade, std::string typeIndex, std::string typeInstance)
{
	 int res = 0;
	 //typeIndex is A/B
	 //tpeInstance is 0 (lower), 1(upper)
	 
	 std::string oidPfmSWUpgrade = oidPfmSwUpgrade + "." + typeIndex + "." + typeInstance ;
	 cout << "ipAddr: " << ipAddr.c_str() << endl;
	 cout << "oidPfmSWUpgrade: " << oidPfmSWUpgrade.c_str() << endl;
	 cout << "SwUpgrade: " << swUpgrade.c_str() << endl;
	 res = snmpSet("NETMAN", ipAddr.c_str(), oidPfmSWUpgrade.c_str(), 's', swUpgrade.c_str());

	 if (res == GETSET_SNMP_OK)
	 {
		 return true;
	 }

	 return false;
}

 bool FIXS_CCH_SNMPManager::getPfmFWUpgrade (std::string *installedFWUpgrade, int typeIndex, int typeInstance)
 {

	 std::string m_installedFW("");
	 int res = 0;
	 char* oidPfmFWUpgrade = (char*)malloc(100);
	 memset(oidPfmFWUpgrade,0,100);
	 sprintf(oidPfmFWUpgrade,"%s.%d.%d",oidPfmFwUpgrade.c_str(),typeIndex,typeInstance);
	 BYTE out[SNMP_MAXBUF];

	 res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmFWUpgrade,ASN_OCTET_STR,out,SNMP_MAXBUF);
	 if (res == GETSET_SNMP_OK)
	 {
		 static char buffer[128];
		 sprintf(buffer, "%s\n", (char *) out);
		 m_installedFW = std::string((char*) out);
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedFWProductNumber: " << m_installedFW.c_str() << std::endl;
		 *installedFWUpgrade = m_installedFW;
		 free(oidPfmFWUpgrade);
		 return true;
	 }

	 free(oidPfmFWUpgrade);
	 return false;
 }

bool FIXS_CCH_SNMPManager::getPfmSWUpgrade (std::string *installedSwUpgrade, int typeIndex, int typeInstance)
{
	 std::string m_installedSW("");
	 int res = 0;
	 char* oidPfmSWUpgrade = (char*)malloc(100);
	 memset(oidPfmSWUpgrade,0,100);
	 sprintf(oidPfmSWUpgrade,"%s.%d.%d",oidPfmSwUpgrade.c_str(),typeIndex,typeInstance);
	 BYTE out[SNMP_MAXBUF];

	 res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmSWUpgrade,ASN_OCTET_STR,out,SNMP_MAXBUF);
	 if (res == GETSET_SNMP_OK)
	 {
		 static char buffer[128];
		 sprintf(buffer, "%s\n", (char *) out);
		 m_installedSW = std::string((char*) out);
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedSWProductNumber: " << m_installedSW.c_str() << std::endl;
		 *installedSwUpgrade = m_installedSW;
		 free(oidPfmSWUpgrade);
		 return true;
	 }

	 free(oidPfmSWUpgrade);
	 return false;
}

bool FIXS_CCH_SNMPManager::getPfmFWProductNumber (std::string *installedFWProductNumber, int typeIndex, int typeInstance)
{	
	std::string m_installedFWProductNumber("");
	int res = 0;
	char* oidPfmProductNumber = (char*)malloc(100);
	memset(oidPfmProductNumber,0,100);
	sprintf(oidPfmProductNumber,"%s.%d.%d",oidPfmFwProductNumber.c_str(),typeIndex,typeInstance);
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmProductNumber,ASN_OCTET_STR,out,SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		m_installedFWProductNumber = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedFWProductNumber: " << m_installedFWProductNumber.c_str() << std::endl;
		*installedFWProductNumber = m_installedFWProductNumber;
		free(oidPfmProductNumber);
		return true;

	}
	free(oidPfmProductNumber);
	return false;	
}

int FIXS_CCH_SNMPManager::getPfmCurSwType ( int typeInstance)
{
	int res = 0;
	int swType=-1;
	char* oidPfmCurSwType = (char*)malloc(100);
	memset(oidPfmCurSwType,0,100);
	sprintf(oidPfmCurSwType,"%s.%d",oidpfmSwInventoryCurrentSwType.c_str(),typeInstance);
	BYTE out[SNMP_MAXBUF];
	res= snmpGet("PUBLIC",ipAddr.c_str(),oidPfmCurSwType,ASN_INTEGER,out,SNMP_MAXBUF,0);

	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer,"%d", *((int*)&out));
		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		swType = atoi(buffer);

		std::cout << __FUNCTION__ << "@" << __LINE__ << " swType: " << swType << std::endl;
	}
	else {
		swType= -1;
	}
	free(oidPfmCurSwType);	
	return swType;	
}

bool FIXS_CCH_SNMPManager::getPfmSwProductNumber (std::string *installedSWProductNumber, int swType, int typeInstance)
{	
	std::string m_installedSWProductNumber("");
	int res = 0;
	char* oidPfmSwProductNumber = (char*)malloc(100);
	memset(oidPfmSwProductNumber,0,100);
	if(swType==0)
	{
		sprintf(oidPfmSwProductNumber,"%s.%d",oidPfmSwInventoryBlProductNumber.c_str(),typeInstance);
	}
	else if (swType==1)
	{
		sprintf(oidPfmSwProductNumber,"%s.%d",oidPfmSwInventoryUpgProductNumber.c_str(),typeInstance);
	}
	
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmSwProductNumber,ASN_OCTET_STR,out,SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		m_installedSWProductNumber = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedSWProductNumber: " << m_installedSWProductNumber.c_str() << std::endl;
		*installedSWProductNumber = m_installedSWProductNumber;
		free(oidPfmSwProductNumber);
		return true;

	}
	free(oidPfmSwProductNumber);
	return false;	
}

bool FIXS_CCH_SNMPManager::getPfmSwProductRevisionState (std::string *installedSwProductRevisionState, int swType, int typeInstance)
{	
	std::string m_installedSWRevisionState("");
	int res = 0;
	char* oidPfmSwRevisionState = (char*)malloc(100);
	memset(oidPfmSwRevisionState,0,100);
	if(swType==0)
	{
		sprintf(oidPfmSwRevisionState,"%s.%d",oidPfmSwInventoryBlProductRevisionState.c_str(),typeInstance);
	}
	else if (swType==1)
	{
		sprintf(oidPfmSwRevisionState,"%s.%d",oidPfmSwInventoryUpgProductRevisionState.c_str(),typeInstance);
	}
	
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmSwRevisionState,ASN_OCTET_STR,out,SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		m_installedSWRevisionState = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedSWRevisionState: " << m_installedSWRevisionState.c_str() << std::endl;
		*installedSwProductRevisionState = m_installedSWRevisionState;
		free(oidPfmSwRevisionState);
		return true;

	}
	free(oidPfmSwRevisionState);
	return false;	
}

bool FIXS_CCH_SNMPManager::getPfmSwInitialProductNumber (std::string *initialSWProductNumber, int swType, int typeInstance)
{	
	std::string m_initialSWProductNumber("");
	int res = 0;
	char* oidPfmSwInitialProductNumber = (char*)malloc(100);
	memset(oidPfmSwInitialProductNumber,0,100);
	
	sprintf(oidPfmSwInitialProductNumber,"%s.%d",oidPfmInitialProductNumber.c_str(),swType);
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmSwInitialProductNumber,ASN_OCTET_STR,out,SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		m_initialSWProductNumber = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_initialSWProductNumber: " << m_initialSWProductNumber.c_str() << std::endl;
		*initialSWProductNumber = m_initialSWProductNumber;
		free(oidPfmSwInitialProductNumber);
		return true;

	}
	free(oidPfmSwInitialProductNumber);
	return false;	
}

bool FIXS_CCH_SNMPManager::getPfmSwInitialProductRevision (std::string *initialSWProductRevision, int swType, int typeInstance)
{	
	std::string m_initialSWProductRevision("");
	int res = 0;
	char* oidPfmSwInitialProductRevision = (char*)malloc(100);
	memset(oidPfmSwInitialProductRevision,0,100);
	
	sprintf(oidPfmSwInitialProductRevision,"%s.%d",oidPfmInitialProductRevision.c_str(),swType);
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmSwInitialProductRevision,ASN_OCTET_STR,out,SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		m_initialSWProductRevision = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_initialSWProductRevision: " << m_initialSWProductRevision.c_str() << std::endl;
		*initialSWProductRevision = m_initialSWProductRevision;
		free(oidPfmSwInitialProductRevision);
		return true;

	}
	free(oidPfmSwInitialProductRevision);
	return false;	
}

bool FIXS_CCH_SNMPManager::getPfmFWProductRevisionState (std::string *installedFWProductRevisionState, int typeIndex, int typeInstance)
{

	std::string m_installedFWProductRevisionState("");
	int res = 0;
	char* oidPfmRevisionState = (char*)malloc(100);
	memset(oidPfmRevisionState,0,100);
	sprintf(oidPfmRevisionState,"%s.%d.%d",oidPfmFwProductRevisionState.c_str(),typeIndex,typeInstance);
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmRevisionState,ASN_OCTET_STR,out,SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		static char buffer[128];
		sprintf(buffer, "%s\n", (char *) out);
		m_installedFWProductRevisionState = std::string((char*) out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedFWProductRevisionState: " << m_installedFWProductRevisionState.c_str() << std::endl;
		*installedFWProductRevisionState = m_installedFWProductRevisionState;
		free(oidPfmRevisionState);
		return true;

	}
	free(oidPfmRevisionState);
	return false;
}

bool FIXS_CCH_SNMPManager::getPfmFWUpgradeStatus (int *actFwStatus, int typeIndex, int typeInstance)
{
	/*	 0 = ready
	 	 1 = ongoing
	 	 2 = idle
	 */

	int res = 0;
	char* oidPfmUpgrageStatus = (char*)malloc(100);
	int m_installedFWUpgradeStatus = -1;
	memset(oidPfmUpgrageStatus,0,100);
	sprintf(oidPfmUpgrageStatus,"%s.%d.%d",oidPfmFwUpgrageStatus.c_str(),typeIndex,typeInstance);
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmUpgrageStatus,ASN_INTEGER,out,SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		m_installedFWUpgradeStatus = *((int*)&out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedFWUpgradeStatus: " << m_installedFWUpgradeStatus << std::endl;
		*actFwStatus = m_installedFWUpgradeStatus;
		free(oidPfmUpgrageStatus);
		return true;
	}

	free(oidPfmUpgrageStatus);
	return false;
}

bool FIXS_CCH_SNMPManager::getPfmSWUpgradeStatus (int  *actSwStatus, int typeIndex, int typeInstance)
{
	/*	 0 = ready
	 	 1 = ongoing
	 	 2 = idle
	 */

	int res = 0;
	char* oidPfmSWUpgrageStatus = (char*)malloc(100);
	int m_installedSWUpgradeStatus = -1;
	memset(oidPfmSWUpgrageStatus,0,100);
	sprintf(oidPfmSWUpgrageStatus,"%s.%d.%d",oidPfmSwUpgrageStatus.c_str(),typeIndex,typeInstance);
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmSWUpgrageStatus,ASN_INTEGER,out,SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		m_installedSWUpgradeStatus = *((int*)&out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedSWUpgradeStatus: " << m_installedSWUpgradeStatus << std::endl;
		*actSwStatus = m_installedSWUpgradeStatus;
		free(oidPfmSWUpgrageStatus);
		return true;
	}

	free(oidPfmSWUpgrageStatus);
	return false;
}

bool FIXS_CCH_SNMPManager::getPfmModuleDeviceType (unsigned int typeIndex, int *ModuleType)
{
	/* device Type
	  	  Lod = 0
	  	  Hod = 1 */

	int res = 0;
	int m_moduleDeviceType = -1;
	char* oidPfmDevice= (char*)malloc(100);
	memset(oidPfmDevice,0,100);
	sprintf(oidPfmDevice,"%s.%d",oidPfmFwModuleDeviceType.c_str(),typeIndex);
	BYTE out[SNMP_MAXBUF];

	res = snmpGet("PUBLIC",ipAddr.c_str(),oidPfmDevice,ASN_INTEGER,out,SNMP_MAXBUF);

	if (res == GETSET_SNMP_OK)
	{
		m_moduleDeviceType = *((int*)&out);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " m_moduleDeviceType: " << m_moduleDeviceType << std::endl;
		*ModuleType = m_moduleDeviceType;
		free(oidPfmDevice);
		return true;
	}

	free(oidPfmDevice);
	return false;
}

bool FIXS_CCH_SNMPManager::getFwmLoadedAttribute (unsigned int index, int *Val)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = CCH_Util::intToString(index);

	tmpOID = oidFwmLoadedAttributeSCX + "." + indexStr;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		char buffer[128];
		sprintf(buffer, "%d", *((int*)&out));

		*Val = atoi(buffer);
	}

	return true;
}

bool FIXS_CCH_SNMPManager::getFwmLoadedType (unsigned int index, std::string &loadVer)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = CCH_Util::intToString(index);

	tmpOID = oidFwmLoadedTypeSCX + "." + indexStr;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET successfully done on: " << tmpOID.c_str() << std::endl;
		loadVer = string((char *)out);
	}else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool FIXS_CCH_SNMPManager::getFwmLoadedVersion (unsigned int index, std::string &loadVer)
{
	int res = 0;
	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");

	string indexStr("");
	indexStr = CCH_Util::intToString(index);

	tmpOID = oidFwmLoadedVersionSCX + "." + indexStr;

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET successfully done on: " << tmpOID.c_str() << std::endl;
		loadVer = string((char *)out);
	}else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}
bool FIXS_CCH_SNMPManager::getBladeRevisionState (std::string *actState)
{
	int res = 0;

	BYTE out[SNMP_MAXBUF];
	std::string tmpOID("");
	tmpOID = oidbmBladeProductRevisionState + ".0";

	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	if (res == GETSET_SNMP_OK)
	{
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET successfully done on: " << tmpOID.c_str() << std::endl;
		*actState = string((char *)out);
	}else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}

	return true;
}

bool FIXS_CCH_SNMPManager::setFWMAction (unsigned int action)
{
	int res = 0;
	std::stringstream ss;
	ss << action;

	std::string tmpOID = oidFwmAction + ".0";

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', ss.str().c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}


 bool FIXS_CCH_SNMPManager::setFWMActionURL (std::string url)
 {
	 int res = 0;

	 std::string tmpOID = oidFwmActionURL + ".0";

	 res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 's', url.c_str());
	 if (res == GETSET_SNMP_OK) {
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	 }
	 else {
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		 return false;
	 }
	 return true;

 }

 bool FIXS_CCH_SNMPManager::getFwmActionStatus (std::string *actStatus)
 {
	 int res = 0;

	 BYTE out[SNMP_MAXBUF];
	 std::string tmpOID("");

	 tmpOID = oidFwmActionStatus + ".0";

	 res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_OCTET_STR, out, SNMP_MAXBUF);
	 if (res == GETSET_SNMP_OK)
	 {
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET successfully done on: " << tmpOID.c_str() << std::endl;
		 *actStatus = string((char *)out);
	 }
	 else {
	 		 std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " GET failed on: " << tmpOID.c_str() << std::endl;
	 		 return false;
	 }
	 return true;

 }

 bool FIXS_CCH_SNMPManager::getIpmiFWUpgrade (std::string *installedFWUpgrade, std::string slot)
 {

	 std::string m_installedFW("");
	 BYTE out[SNMP_MAXBUF];
	 std::string tmpOID("");

	 if (isCBAenvironment()) tmpOID = oidBoardIpmiUpgradeSCX + "." + slot;
	 else return false; //TODO

	 int res = 0;

	 res = snmpGet("PUBLIC",ipAddr.c_str(),tmpOID.c_str(),ASN_OCTET_STR,out,SNMP_MAXBUF);
	 if (res == GETSET_SNMP_OK)
	 {
		 static char buffer[128];
		 sprintf(buffer, "%s\n", (char *) out);
		 m_installedFW = std::string((char*) out);
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedFWProductNumber: " << m_installedFW.c_str() << std::endl;
		 *installedFWUpgrade = m_installedFW;
		 return true;
	 }

	 return false;
 }

 bool FIXS_CCH_SNMPManager::getFWMActionURL (std::string *installedFWUpgrade)
 {

	 std::string m_installedFW("");
	 BYTE out[SNMP_MAXBUF];
	 std::string tmpOID("");

	 tmpOID = oidFwmActionURL + ".0";

	 int res = 0;

	 res = snmpGet("PUBLIC",ipAddr.c_str(),tmpOID.c_str(),ASN_OCTET_STR,out,SNMP_MAXBUF);
	 if (res == GETSET_SNMP_OK)
	 {
		 static char buffer[128];
		 sprintf(buffer, "%s\n", (char *) out);
		 m_installedFW = std::string((char*) out);
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " m_installedFWProductNumber: " << m_installedFW.c_str() << std::endl;
		 *installedFWUpgrade = m_installedFW;
		 return true;
	 }

	 return false;
 }

 std::string FIXS_CCH_SNMPManager::getBladeReturnToFactoryDefaultOid() {
	 return oidBmBladeReturnToFactoryDefault;
 };

 bool FIXS_CCH_SNMPManager::getGprDataValue (std::string *gprDataVal, std::string slot, std::string typeInstance)
 {
	 std::string m_gprVal("");
	 BYTE out[SNMP_MAXBUF];
	 std::string tmpOID("");

	 tmpOID = oidGprDataValue + "." + slot + "." + typeInstance;

	 int res = 0;

 	 res= snmpGet("PUBLIC",ipAddr.c_str(),tmpOID.c_str(),ASN_OCTET_STR,out,SNMP_MAXBUF,FIXS_CCH_SNMPManager::SNMPGET_HEX_STRING);
	 if (res == GETSET_SNMP_OK)
	 {
		 static char buffer[128];
		 sprintf(buffer, "%s\n", (char *) out);
		 m_gprVal = std::string((char*) out);
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " m_gprVal:" << m_gprVal.c_str()<<"END"<< std::endl;
		 *gprDataVal = m_gprVal;
		 return true;
	 }
	 return false;
 }

 bool FIXS_CCH_SNMPManager::setGprDataValue (std::string gprDataVal, std::string slot, std::string typeInstance)
 {

 	int res = 0;

 	std::cout << __FUNCTION__ << "@" << __LINE__ << "VALUE to SET gprDataVal: " << gprDataVal.c_str() << std::endl;

 	std::string tmpOID = oidGprDataValue + "." + slot + "." + typeInstance;

 	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'x', gprDataVal.c_str());
 	if (res == GETSET_SNMP_OK) {
 		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
 	}
 	else {
 		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
 		return false;
 	}
 	return true;

 }
 bool FIXS_CCH_SNMPManager::setBladeReset (unsigned int action, unsigned int index)
{
	 int res = 0;
	 std::stringstream ss;
	 ss << action;

	 string indexStr("");
	 indexStr = CCH_Util::intToString(index);

	 std::string tmpOID = oidBladeReset + "."+ indexStr;

	 res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', ss.str().c_str());
	 if (res == GETSET_SNMP_OK) {
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	 }
	 else {
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		 return false;
	 }
	 return true;
}

 bool FIXS_CCH_SNMPManager::isBladePower (unsigned int index, int *pwrVal)
 {
	 int res = 0;
	 BYTE out[SNMP_MAXBUF];
	 std::string tmpOID("");

	 string indexStr("");
	 indexStr = CCH_Util::intToString(index);

	 tmpOID = oidBladePower + "." + indexStr;

	 res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
	 if (res == GETSET_SNMP_OK)
	 {
		 char buffer[128];
		 sprintf(buffer, "%d", *((int*)&out));

		 *pwrVal = atoi(buffer);
		 return true;
	 }
	 else
		 return false;
 }

 int FIXS_CCH_SNMPManager::isMasterSCB (std::string ip, ushort slot)
 {
	 if (ip.empty())
	 {
		 return HARDWARE_NOT_PRESENT;
	 }

	 //oidShelfMgrCtrlSCX
	 int resp = 0;
	 int res;
	 BYTE out[SNMP_MAXBUF];
	 std::string tmpOID("");

	 if (isCBAenvironment())
	 {
		 return false;
	 }
	 else
	 {
		 if (slot == 0)
			 tmpOID = oidShelfMgrCmd1 + ".0";
		 else if (slot == 25)
			 tmpOID = oidShelfMgrCmd1 + ".25";
	 }


	 setIpAddress(ip);

	 res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF,0);
	 if (res == GETSET_SNMP_OK)
	 {
		 char buffer[128];
		 sprintf(buffer, "%d", *((int*)&out));
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

		 int masterSCB = atoi(buffer);

		 if(masterSCB == 1)
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
		 resp = HARDWARE_NOT_PRESENT;
	 }
	 else
	 {
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << " With other error" << std::endl;
				 resp = SNMP_FAILURE;
	 }
	 return resp;
 }


 int FIXS_CCH_SNMPManager::getXshmcBoardPowerOff (std::string shelfMgrIp, int slot)
 {
 	int res,boardPower;
 	char* oidBoardPower= (char*)malloc(100);
 	memset(oidBoardPower,0,100);
 	sprintf(oidBoardPower,"%s.%d",oidXshmcBoardPwrOff.c_str(),slot);
 	std::cout <<"Getting board power for OID : " << oidBoardPower << std::endl;
 	BYTE out[SNMP_MAXBUF];
 	res= snmpGet("PUBLIC",shelfMgrIp.c_str(),oidBoardPower,ASN_INTEGER,out,SNMP_MAXBUF,0);

 	if (res == GETSET_SNMP_OK)
 	{
 		char buffer[128];
 		sprintf(buffer,"%d", *((int*)&out));
 		std::cout << __FUNCTION__ << "@" << __LINE__ << " buffer: " << buffer << std::endl;

 		boardPower = atoi(buffer);

 		std::cout << __FUNCTION__ << "@" << __LINE__ << " Value: " << boardPower << std::endl;
 	}
 	else {
 		boardPower= -1;

 	}

 	free(oidBoardPower);
 	return boardPower;
 }
 bool FIXS_CCH_SNMPManager::getXshmcBoardPwrOff (unsigned int index, int *Val)
 {
 	int res = 0;
 	BYTE out[SNMP_MAXBUF];
 	std::string tmpOID("");

 	string indexStr("");
 	indexStr = CCH_Util::intToString(index);

 	tmpOID = oidXshmcBoardPwrOff + "." + indexStr;

 	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
 	if (res == GETSET_SNMP_OK)
 	{
 		char buffer[128];
 		sprintf(buffer, "%d", *((int*)&out));

 		*Val = atoi(buffer);
 	}

 	return true;
 }
 bool FIXS_CCH_SNMPManager::setXshmcBoardPwrOff (unsigned int action, unsigned int index)
 {
 	int res = 0;
 	std::stringstream ss;
 	ss << action;

 	string indexStr("");
 	 	indexStr = CCH_Util::intToString(index);

 	std::string tmpOID = oidXshmcBoardPwrOff + "."+ indexStr;

 	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', ss.str().c_str());
 	if (res == GETSET_SNMP_OK) {
 		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
 	}
 	else {
 		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
 		return false;
 	}
 	return true;
 }
 bool FIXS_CCH_SNMPManager::setXshmcBoardYellowLed (unsigned int action, unsigned int index)
  {
  	int res = 0;
  	std::stringstream ss;
  	ss << action;

  	string indexStr("");
  	 	indexStr = CCH_Util::intToString(index);

  	std::string tmpOID = oidxshmcBoardYellowLed + "."+ indexStr;

  	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', ss.str().c_str());
  	if (res == GETSET_SNMP_OK) {
  		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
  	}
  	else {
  		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
  		return false;
  	}
  	return true;
  }
 bool FIXS_CCH_SNMPManager::getBladePresence (unsigned int index, int *Val)
  {
  	int res = 0;
  	BYTE out[SNMP_MAXBUF];
  	std::string tmpOID("");

  	string indexStr("");
  	indexStr = CCH_Util::intToString(index);

  	tmpOID = oidXshmcBoardPresenceSCX + "." + indexStr;

  	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
  	if (res == GETSET_SNMP_OK)
  	{
  		char buffer[128];
  		sprintf(buffer, "%d", *((int*)&out));
  		*Val = atoi(buffer);
  	}

  	return true;
  }
 bool FIXS_CCH_SNMPManager::getBladePwr (unsigned int index, int *Val)
   {
   	int res = 0;
   	BYTE out[SNMP_MAXBUF];
   	std::string tmpOID("");

   	string indexStr("");
   	indexStr = CCH_Util::intToString(index);

   	tmpOID = oidBladePower + "." + indexStr;

   	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
   	if (res == GETSET_SNMP_OK)
   	{
   		char buffer[128];
   		sprintf(buffer, "%d", *((int*)&out));

   		*Val = atoi(buffer);
   	}

   	return true;
   }

 bool FIXS_CCH_SNMPManager::setBladePwr (unsigned int action, unsigned int index)
   {
   	int res = 0;
   	std::stringstream ss;
   	ss << action;

   	string indexStr("");
   	 	indexStr = CCH_Util::intToString(index);

   	std::string tmpOID = oidBladePower + "."+ indexStr;

   	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', ss.str().c_str());
   	if (res == GETSET_SNMP_OK) {
   		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
   	}
   	else {
   		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
   		return false;
   	}
   	return true;
   }
 bool FIXS_CCH_SNMPManager::getXshmcBoardPwrOn (unsigned int index, int *Val)
    {
    	int res = 0;
    	BYTE out[SNMP_MAXBUF];
    	std::string tmpOID("");

    	string indexStr("");
    	indexStr = CCH_Util::intToString(index);

    	tmpOID = oidXshmcBoardPwrOn + "." + indexStr;

    	res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
    	if (res == GETSET_SNMP_OK)
    	{
    		char buffer[128];
    		sprintf(buffer, "%d", *((int*)&out));

    		*Val = atoi(buffer);
    	}

    	return true;
    }
 bool FIXS_CCH_SNMPManager::setXshmcBoardPwrOn (unsigned int action, unsigned int index)
    {
    	int res = 0;
    	std::stringstream ss;
    	ss << action;

    	string indexStr("");
    	 	indexStr = CCH_Util::intToString(index);

    	std::string tmpOID = oidXshmcBoardPwrOn + "."+ indexStr;

    	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', ss.str().c_str());
    	if (res == GETSET_SNMP_OK) {
    		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
    	}
    	else {
    		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
    		return false;
    	}
    	return true;
    }

 bool FIXS_CCH_SNMPManager::getXshmcGprDataValue(std::string *gprDataValue, std::string slot, std::string typeInstance)
 {
	 std::string m_gprVal("");
	 	 BYTE out[SNMP_MAXBUF];
	 	 std::string tmpOID("");

	 	 tmpOID = oidXshmcGPRData + "." + slot + "." + typeInstance;

	 	 int res = 0;

	 	 res = snmpGet("PUBLIC",ipAddr.c_str(),tmpOID.c_str(),ASN_OCTET_STR,out,SNMP_MAXBUF);
	 	 if (res == GETSET_SNMP_OK)
	 	 {
	 		 static char buffer[128];
	 		 sprintf(buffer, "%s\n", (char *) out);
	 		 m_gprVal = std::string((char*) out);
	 		 std::cout << __FUNCTION__ << "@" << __LINE__ << " m_gprVal: " << m_gprVal.c_str() << std::endl;
	 		 *gprDataValue = m_gprVal;
	 		 return true;
	 	 }

	 	 return false;
 }

 bool FIXS_CCH_SNMPManager::setXshmcGprDataValue (std::string gprDataVal, std::string slot, std::string typeInstance)
 {

 	int res = 0;

 	std::cout << __FUNCTION__ << "@" << __LINE__ << "VALUE to SET gprDataVal: " << gprDataVal.c_str() << std::endl;

 	std::string tmpOID = oidXshmcGPRData + "." + slot + "." + typeInstance;

 	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 's', gprDataVal.c_str());
 	if (res == GETSET_SNMP_OK) {
 		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
 	}
 	else {
 		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
 		return false;
 	}
 	return true;

 }

 bool FIXS_CCH_SNMPManager::setXshmcBoardReset (int slot)
 {
	 int res = 0;
	 const char* val = "1";
	 char* oidBladeReset_slot= (char*)malloc(100);
	 memset(oidBladeReset_slot,0,100);
	 sprintf(oidBladeReset_slot,"%s.%d",oidXshmcBoardReset.c_str(),slot);

	 res = snmpSet("NETMAN",ipAddr.c_str(), oidBladeReset_slot, 'i', val);
	 if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
	 {
		 res = snmpSet("NETMAN", ipAddr.c_str(), oidBladeReset_slot, 'i', val);
		 if(res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
		 {
			 std:: cout << "Board reset FAILED at slot : " << slot << std::endl;
			 free(oidBladeReset_slot);
			 return false;
		 }
		 std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMP set XshmcBoardReset successful " << std::endl;
	 }
	 std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMP set XshmcBoardReset successful " << std::endl;

	 free(oidBladeReset_slot);
	 return true;
 }

bool FIXS_CCH_SNMPManager::setActiveArea (unsigned int slot, int action)
{

	int res = 0;
	std::stringstream ss;
	ss << slot;
	std::string tmpOID("");

	tmpOID = oidBladeBIOSPointer+ "." + ss.str(); //1.3.6.1.4.1.193.177.2.2.1.3.1.1.1.23

	std::stringstream ss1;
	ss1 << action;

	res = snmpSet("NETMAN", ipAddr.c_str(), tmpOID.c_str(), 'i', ss1.str().c_str());
	if (res == GETSET_SNMP_OK) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET successfully done on: " << tmpOID.c_str() << std::endl;
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on: " << tmpOID.c_str() << std::endl;
		return false;
	}
	return true;
}

bool FIXS_CCH_SNMPManager::getFwmActionResult(int &result)
{
        int res;
        BYTE out[SNMP_MAXBUF];
        std::string tmpOID("");
	tmpOID = oidFwmActionResult;

        std::cout << __FUNCTION__ << "@" << __LINE__ << "Entered getFwmActionResult()" << std::endl;

        res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_GAUGE, out, SNMP_MAXBUF);
        if (res == GETSET_SNMP_OK)
        {
                char buffer[128];
                sprintf(buffer, "%d", *((int*)&out));

                result = atoi(buffer);
		std::cout << __FUNCTION__ << "@" << __LINE__ << "getFwmActionResult() result: " << result << std::endl;
        }
        else {
                result = -1;
                std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
                return false;
        }

        return true;
}

bool FIXS_CCH_SNMPManager::getActiveArea (std::string slot,int &activeArea)
{
	int res = 0;
        BYTE out[SNMP_MAXBUF];
        std::string tmpOID("");

        tmpOID = oidBladeBIOSPointer + "." + slot;

        res = snmpGet("PUBLIC", ipAddr.c_str(), tmpOID.c_str(), ASN_INTEGER, out, SNMP_MAXBUF);
        if (res == GETSET_SNMP_OK)
        {
                char buffer[128];
                sprintf(buffer, "%d", *((int*)&out));

                activeArea = atoi(buffer);
        }
	else {
                activeArea = -1;
                std::cout << __FUNCTION__ << "@" << __LINE__ << " Get failed on: " << tmpOID.c_str() << std::endl;
                return false;
        }

        return true;
}

