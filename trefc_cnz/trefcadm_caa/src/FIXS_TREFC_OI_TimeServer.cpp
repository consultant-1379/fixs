/*
 * FIXS_TREFC_OI_TimeServer.cpp
 *
 *  Created on: Jan 3, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_OI_TimeServer.h"
#include "FIXS_TREFC_Logger.h"
#include "FIXS_TREFC_ImmInterface.h"



/*============================================================================
	ROUTINE: FIXS_TREFC_OI_TimeServer
 ============================================================================ */
FIXS_TREFC_OI_TimeServer::FIXS_TREFC_OI_TimeServer() :  acs_apgcc_objectimplementerinterface_V2(IMM_Util::IMPLEMENTER_TIMESERVER),
m_ImmClassName(IMM_Util::classTimeServer),_trace(NULL),m_lock()
{
	_trace = new ACS_TRA_trace("FIXS_TREFC_Manager");
}

/*============================================================================
	ROUTINE: ~FIXS_TREFC_OI_TimeServer
 ============================================================================ */
FIXS_TREFC_OI_TimeServer::~FIXS_TREFC_OI_TimeServer()
{
	if (_trace) {
		delete(_trace);
		_trace = NULL;
	}

}


ACS_CC_ReturnType FIXS_TREFC_OI_TimeServer::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " class Name: " << className << endl;
	cout << " parent Name: " << parentname << endl;

	ACS_CC_ReturnType result = ACS_CC_FAILURE;
	//int res;
	string rdn("");
	string ipAddress("");

	if( 0 == ACE_OS::strcmp(m_ImmClassName.c_str(), className ) )
	{
		for(size_t idx = 0; attr[idx] != NULL ; idx++)
		{
			if( 0 == ACE_OS::strcmp(IMM_Util::ATT_TIMESERVER_IP, attr[idx]->attrName) )
			{
				ipAddress = reinterpret_cast<char *>(*attr[idx]->attrValues);

			}
			if( 0 == ACE_OS::strcmp(IMM_Util::ATT_TIMESERVER_RDN, attr[idx]->attrName) )
			{
				rdn =reinterpret_cast<char *>(*attr[idx]->attrValues);

			}
		}


		//Check if is a valid IP address
		unsigned long int ip = 0;
		if (TREFC_Util::checkIPAddresses(ipAddress.c_str(),ip) == false) {
			cout << "Invalid IP address" << std::endl;
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Time Server creation rejected: Invalid IP address = %s", ipAddress.c_str());
			printTRA(tmpStr);
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
			int err = TREFC_Util::WRONG_IP_ADDRESS;
			this->setExitCode(err,TREFC_Util::getErrrorText(err));

			return ACS_CC_FAILURE;
		}

		TimeServer server;
		server.ipAddress=ipAddress;
		server.name=IMM_Util::getIdValueFromRdn(rdn);

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Adding Time Server name: %s, IP address = %s.", server.name.c_str(), server.ipAddress.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
		}

		addToTransaction(ccbId, server, IMM_Util::CREATE);

		result = ACS_CC_SUCCESS;
	}
	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_TimeServer::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectDeleteCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	ACS_CC_ReturnType result = ACS_CC_FAILURE;
	std::string ipAddress("");
	std::string rdn("");
	//int res;

	if (IMM_Util::getImmAttributeString(objName,IMM_Util::ATT_TIMESERVER_IP,ipAddress)) {
		TimeServer server;
		server.ipAddress=ipAddress;
		server.name=IMM_Util::getIdValueFromRdn(objName);

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Removing Time Server name: %s, IP address = %s.", server.name.c_str(), server.ipAddress.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
		}

		addToTransaction(ccbId, server, IMM_Util::DELETE);
		result = ACS_CC_SUCCESS;
	}


	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_TimeServer::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectModifyCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	cout << " MODIFYCallback " << endl;
	cout << " MODIFY Not allowed for Time Server " << endl;
	cout << endl;

	UNUSED(attrMods);
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Action Rejected. Modify not allowed for Time Server.");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
	}
	int err = TREFC_Util::MODIFICATION_NOT_ALLOWED;
	this->setExitCode(err,TREFC_Util::getErrrorText(err));
	ACS_CC_ReturnType result = ACS_CC_FAILURE;
	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_TimeServer::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " COMPLETECallback " << endl;
	cout << endl;

	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();

	if(environment != SINGLECP_CBA && environment != MULTIPLECP_CBA && environment != SMX) {
			cout << "ERROR: Operation not allowed in this system configuration." << endl;
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Action Rejected. Operation not allowed in this system configuration.");
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
				int err = TREFC_Util::ILLEGAL_COMMAND;
				this->setExitCode(err,TREFC_Util::getErrrorText(err));
			}
			return ACS_CC_FAILURE;
		}

	ACS_CC_ReturnType result;
	result = validateTransaction(ccbId);

//	if (result != ACS_CC_SUCCESS) {
//		setExitCode(100,"Time Server Transaction Failed!");
//	}

	return result;
}

void FIXS_TREFC_OI_TimeServer::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbAbortCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " ABORTCallback " << endl;
	cout << endl;

	abortTransaction(ccbId);
}

void FIXS_TREFC_OI_TimeServer::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbApplyCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " APPLYCallback " << endl;
	cout << endl;

//	TRACE(FIXS_TREFC_OI_TimeServerTrace, "%s", "Entering apply(...)");

	commitTransaction(ccbId);

}

ACS_CC_ReturnType FIXS_TREFC_OI_TimeServer::updateRuntime(const char* p_objName, const char* p_attrName)
{
	printf("------------------------------------------------------------\n");
	printf("                   updateRuntime called                     \n");
	printf("------------------------------------------------------------\n");

	cout<<"Object Name: "<<p_objName<<endl;
	cout<<"Attribute Name: "<<p_attrName<<endl;

	printf("------------------------------------------------------------\n");

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;


	return result;
}


void FIXS_TREFC_OI_TimeServer::adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_InvocationType invocation,
		const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
		ACS_APGCC_AdminOperationParamType**paramList) {

	printf("------------------------------------------------------------\n");
	printf("                   adminOperationCallback called             \n");
	printf("------------------------------------------------------------\n");

	cout<<"oiHandle   : "<< oiHandle <<endl;
	cout<<"invocation : "<< invocation <<endl;
	cout<<"p_objName  : "<< p_objName <<endl;
	cout<<"operationId: "<< operationId <<endl;

	/*start*/
	cout << endl;

	int dim=0;
	int i=0;
	while(paramList[i]){
		i++;
		dim++;
	}

	i = 0;
	while( paramList[i] ){
		switch ( paramList[i]->attrType ) {
		case ATTR_INT32T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<int *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_UINT32T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<unsigned int *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_INT64T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<long long *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_UINT64T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<unsigned long long *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_FLOATT:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<float *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_DOUBLET:
			cout << " paramListName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<double *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_NAMET:
		{
			cout << " attrName: " << paramList[i]->attrName << "\t value: " << reinterpret_cast<char *>(paramList[i]->attrValues) << endl;

		}
			break;
		case ATTR_STRINGT:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " << reinterpret_cast<char *>(paramList[i]->attrValues) << endl;
			break;
		default:
			break;

		}
		i++;
	}

	cout << " Implementing Admin Operation with ID = " << operationId << endl;

	SaAisErrorT retVal = SA_AIS_OK;



//############## ACTION 1 ----IPMI upgrade

	if (operationId == 1)
	{
		cout << " ----------- ACTION 1 ----IPMI upgrade--------------\n  " << endl;
		retVal = SA_AIS_OK;
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;

	}

}

void FIXS_TREFC_OI_TimeServer::printTRA(std::string mesg)
{
	if (_trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, mesg.c_str());
		_trace->ACS_TRA_event(1, tmpStr);

	}
}


void FIXS_TREFC_OI_TimeServer::addToTransaction (ACS_APGCC_CcbId ccbid, TimeServer server, IMM_Util::Action action) {

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	std::map <ACS_APGCC_CcbId, std::vector<TimeServerAction> >::iterator it;

	TimeServerAction temp;
	temp.element = server;
	temp.action=action;

	it=transaction.find(ccbid);
	if (it != transaction.end()) {
		it->second.push_back(temp);
	} else {
		std::vector <TimeServerAction> temp_vector;
		temp_vector.clear();
		temp_vector.push_back(temp);
		transaction.insert( pair <ACS_APGCC_CcbId, std::vector <TimeServerAction> > (ccbid,temp_vector));
	}

}


void FIXS_TREFC_OI_TimeServer::commitTransaction(ACS_APGCC_CcbId ccbId) {
	//Commit Transaction

	FIXS_TREFC_Manager* objTrefcManager = FIXS_TREFC_Manager::getInstance();

	std::map <ACS_APGCC_CcbId, std::vector<TimeServerAction> >::iterator it;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	it=transaction.find(ccbId);

	if (it != transaction.end()) {
		int size = it->second.size();
		for (int i=0; i< size; i++) {

			TimeServerAction temp = it->second[i];
			if (temp.action == IMM_Util::CREATE ) {
				objTrefcManager->addTRefToList(temp.element.ipAddress,temp.element.name);
			}
			else if (temp.action == IMM_Util::DELETE) {
				objTrefcManager->rmTRefFromList(temp.element.ipAddress,temp.element.name);
			}
		}
		transaction.erase(it);
	}
}

void FIXS_TREFC_OI_TimeServer::abortTransaction(ACS_APGCC_CcbId ccbId) {
	//Abort Transaction
	std::map <ACS_APGCC_CcbId, std::vector<TimeServerAction> >::iterator it;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	it=transaction.find(ccbId);
	if (it != transaction.end()) {
		transaction.erase(it);

	}
}

ACS_CC_ReturnType FIXS_TREFC_OI_TimeServer::validateTransaction(ACS_APGCC_CcbId ccbId) {

	FIXS_TREFC_Manager* objTrefcManager = FIXS_TREFC_Manager::getInstance();

	std::map <ACS_APGCC_CcbId, std::vector<TimeServerAction> >::iterator it;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	it=transaction.find(ccbId);
	int res = 0;

	if (it != transaction.end()) {
		//int size = it->second.size();
		res = objTrefcManager->validateTimeServerTransaction(it->second);
		if (res != 0) {
			this->setExitCode(res,TREFC_Util::getErrrorText(res));
			return ACS_CC_FAILURE;
		}

	} else
		return ACS_CC_FAILURE;

	return ACS_CC_SUCCESS;
}

