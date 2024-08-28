/*
 * FIXS_TREFC_OI_TimeService.cpp
 *
 *  Created on: Jan 3, 2012
 *      Author: eanform
 */


#include "FIXS_TREFC_OI_TimeService.h"
#include "FIXS_TREFC_ThreadActions.h"
#include "FIXS_TREFC_Logger.h"

FIXS_TREFC_OI_TimeService* FIXS_TREFC_OI_TimeService::m_instance = 0;

/*============================================================================
	ROUTINE: FIXS_TREFC_OI_TimeService
 ============================================================================ */
FIXS_TREFC_OI_TimeService::FIXS_TREFC_OI_TimeService() :  acs_apgcc_objectimplementerinterface_V2(IMM_Util::IMPLEMENTER_TIMESERVICE),
m_ImmClassName(IMM_Util::classTimeReferenceConfiguration),m_actionThread(NULL),startAction(0)
{
	m_actionThread = new FIXS_TREFC_ThreadActions();
}

/*============================================================================
	ROUTINE: ~FIXS_TREFC_OI_TimeService
 ============================================================================ */
FIXS_TREFC_OI_TimeService::~FIXS_TREFC_OI_TimeService()
{
	if (m_actionThread) {
		int retry = 1;
		while(m_actionThread->isRunningThread() && retry <=3)
		{
		 retry++;
		 sleep(1);
		}
		delete(m_actionThread);
		m_actionThread = NULL;
	}

}

FIXS_TREFC_OI_TimeService* FIXS_TREFC_OI_TimeService::getInstance ()
{
	//Lock lock;
	if (m_instance == 0)
	{
		m_instance = new FIXS_TREFC_OI_TimeService();
	}
	return m_instance;

}

void FIXS_TREFC_OI_TimeService::finalize () {
	if (m_instance) {
		delete(m_instance);
		m_instance = NULL;
	}
}


ACS_CC_ReturnType FIXS_TREFC_OI_TimeService::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " class Name: " << className << endl;
	cout << " parent Name: " << parentname << endl;

	UNUSED(attr);

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}


ACS_CC_ReturnType FIXS_TREFC_OI_TimeService::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectDeleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_TimeService::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectModifyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	cout << " MODIFYCallback " << endl;
	cout << endl;

	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();

	if(environment != SINGLECP_CBA && environment != MULTIPLECP_CBA && environment != TREFC_Util::SMX) {
		cout << "ERROR: Operation not allowed in this system configuration." << endl;

		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeService] Change of administrativeState not allowed in this system configuration.");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);
		int err = TREFC_Util::ILLEGAL_COMMAND;
		this->setExitCode(err,TREFC_Util::getErrrorText(err));

		return ACS_CC_FAILURE;
	}

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	FIXS_TREFC_ThreadActions::actionType value = FIXS_TREFC_ThreadActions::UNDEFINED;

	for(size_t idx = 0; attrMods[idx] != NULL ; idx++)
	{
		if( 0 == ACE_OS::strcmp(IMM_Util::ATT_TIMEREFERENCE_ADMIN_STATE, attrMods[idx]->modAttr.attrName) )
		{
			if( attrMods[idx]->modAttr.attrValuesNum == 0 )
			{
				result = ACS_CC_FAILURE;
				break;
			}

			value = *reinterpret_cast<FIXS_TREFC_ThreadActions::actionType *>(*attrMods[idx]->modAttr.attrValues);

			string actionStr = "";

			if (value == FIXS_TREFC_ThreadActions::LOCK)
				actionStr = "LOCK";
			else if (value == FIXS_TREFC_ThreadActions::UNLOCK)
				actionStr = "UNLOCK";
			else
				actionStr = "UNDEFINED";

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeService] Received %s action", actionStr.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);


			if (m_actionThread && !m_actionThread->isRunningThread())
			{
				m_actionThread->setActionType(value);

				startAction = true;
			}
			else if (!m_actionThread->isRollbackPhase()) {
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeService] %s action rejected: Thread action is busy, another action in progress!", actionStr.c_str());
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_ERROR);

				result = ACS_CC_FAILURE;
			} else {
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeService] %s during Rollback Phase, no action required!", actionStr.c_str());
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
			}

			break;
		}
	}


	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_TimeService::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " COMPLETECallback " << endl;
	cout << endl;

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}

void FIXS_TREFC_OI_TimeService::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbAbortCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " ABORTCallback " << endl;
	cout << endl;

	startAction = false;
}

void FIXS_TREFC_OI_TimeService::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbApplyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " APPLYCallback " << endl;
	cout << endl;

	if (startAction){
		if (m_actionThread) {

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeService] Starting action thread!");
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);

			m_actionThread->open();
		}
		startAction = false;
	}

//	TRACE(FIXS_TREFC_OI_TimeServiceTrace, "%s", "Entering apply(...)");

}

ACS_CC_ReturnType FIXS_TREFC_OI_TimeService::updateRuntime(const char* p_objName, const char* p_attrName)
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


void FIXS_TREFC_OI_TimeService::adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
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

	std::string implementer = getImpName();
	cout << "implementer name : " << implementer << endl;

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

	SaAisErrorT retVal = SA_AIS_ERR_NO_OP;

	this->adminOperationResult( oiHandle , invocation, retVal );
	return;

}


