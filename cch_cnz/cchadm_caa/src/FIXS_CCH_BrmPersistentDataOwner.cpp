/*
 * FIXS_CCH_BrmPersistentDataOwner.cpp
 *
 *  Created on: Jul 11, 2012
 *      Author: tcslali
 */

#include "FIXS_CCH_BrmPersistentDataOwner.h"

using namespace std;


FIXS_CCH_BrmPersistentDataOwner::FIXS_CCH_BrmPersistentDataOwner():acs_apgcc_objectimplementerinterface_V3(IMM_Util::IMPLEMENTER_BRMPERSISTENTDATAOWNER),
m_ImmClassName(IMM_Util::classBrmPersistentDataOwner)
{
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
}

FIXS_CCH_BrmPersistentDataOwner::~FIXS_CCH_BrmPersistentDataOwner()
{
	FIXS_CCH_logging = 0;
}


ACS_CC_ReturnType FIXS_CCH_BrmPersistentDataOwner::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentName, ACS_APGCC_AttrValues **attr)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " class Name: " << className << endl;
	cout << " parent Name: " << parentName << endl;

	UNUSED(attr);
	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}


ACS_CC_ReturnType FIXS_CCH_BrmPersistentDataOwner::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
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


ACS_CC_ReturnType FIXS_CCH_BrmPersistentDataOwner::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectModifyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	UNUSED(attrMods);
	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;

}


ACS_CC_ReturnType FIXS_CCH_BrmPersistentDataOwner::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << "COMPLETECallback " << endl;
	cout << endl;

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}


void FIXS_CCH_BrmPersistentDataOwner::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbAbortCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << "ABORTCallback " << endl;
	cout << endl;
}


void FIXS_CCH_BrmPersistentDataOwner::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbApplyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << "APPLYCallback " << endl;
	cout << endl;
}


ACS_CC_ReturnType FIXS_CCH_BrmPersistentDataOwner::updateRuntime(const char* p_objName, const char** p_attrName)
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

int FIXS_CCH_BrmPersistentDataOwner::ResponseToBrfc(unsigned long long requestId,int brfStatus,int responseCode){
	acs_apgcc_adminoperation admOp;
//	ACS_CC_ReturnType result1;
	std::vector<ACS_APGCC_AdminOperationParamType> vectorIN;
	ACS_APGCC_AdminOperationParamType firstElem;
	ACS_APGCC_AdminOperationParamType secondElem;
	char firstAttName[]= "requestId"; // attName1
	char secondAttNameVal1[]= "resultCode"; // attName2
	char secondAttNameVal2[]= "progressCount"; //attName3
	char thirdAttName[]= "message"; // attName3
	unsigned long long requestIdValue = requestId; // intValue1
	int brfStatusVal = brfStatus; // BRF_SUCCESS - intValue2
	char* strValue = const_cast<char*>("");
	void* valueStr[1]={ reinterpret_cast<void*>(strValue) };
	int intAttrValue = 50; // 50 %   - intValue3

	if( responseCode  == BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT )
	{
		ACS_APGCC_AdminOperationParamType thirdElem;

		//create first Element of parameter list
		firstElem.attrName =firstAttName;
		firstElem.attrType=ATTR_UINT64T;
		// return requestId previously checked
		firstElem.attrValues=reinterpret_cast<void*>(&requestIdValue);

		// create second Element of parameter list
		secondElem.attrName =secondAttNameVal1;
		secondElem.attrType=ATTR_INT32T ;
		secondElem.attrValues=reinterpret_cast<void*>(&brfStatusVal);

		//create the third Element of parameter list
		thirdElem.attrName =thirdAttName;
		thirdElem.attrType=ATTR_STRINGT ;
		thirdElem.attrValues=valueStr;

		//insert into the vector
		vectorIN.push_back(firstElem);
		vectorIN.push_back(secondElem);
		vectorIN.push_back(thirdElem);
	}
	else if ( responseCode == BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_PROGRESS)
	{
		//create first Element of parameter list
		firstElem.attrName =firstAttName;
		firstElem.attrType=ATTR_UINT64T;
		firstElem.attrValues=reinterpret_cast<void*>(&requestIdValue);

		// create second Element of parameter list
		secondElem.attrName =secondAttNameVal2;
		secondElem.attrType=ATTR_UINT32T ;
		secondElem.attrValues=reinterpret_cast<void*>(&intAttrValue);

		//insert into the vector
		vectorIN.push_back(firstElem);
		vectorIN.push_back(secondElem);
	}

	long long int timeOutVal_30sec = 30*(1000000000LL);
	int res1 = admOp.init();
	if (res1 != ACS_CC_SUCCESS){
		std::cout <<"ERROR ::admOp.init()FAILED"<< std::endl;
		std::cout <<"ERROR CODE = "<< admOp.getInternalLastError()<< std::endl << "ERROR MESSAGE = " << admOp.getInternalLastErrorText()<<std::endl;
		return -1;
	}
	int returnValue1 = 1;

	int retry = 0;
	while (admOp.adminOperationInvoke(IMM_Util::RDN_BRMPARTICIPANTCONTAINER, 0 , responseCode , vectorIN , &returnValue1, timeOutVal_30sec) == ACS_CC_FAILURE)
	{
		std::cout <<"ERROR ::admOp.adminOperationInvoke()FAILED"<< std::endl;
		std::cout <<"ERROR CODE = "<< admOp.getInternalLastError()<< "ERROR MESSAGE = " << admOp.getInternalLastErrorText()<<std::endl;

		if ((admOp.getInternalLastError()!=((-1)*SA_AIS_ERR_EXIST)) && (admOp.getInternalLastError()!=((-1)*SA_AIS_ERR_BAD_OPERATION))) break;

		sleep(1);
		if (++retry > 60) break;
	}

	res1 = admOp.finalize();
	if (res1 != ACS_CC_SUCCESS){
		std::cout <<"ERROR ::admOp.finalize()FAILED"<< std::endl;
		std::cout <<"ERROR CODE = "<< admOp.getInternalLastError()<< std::endl << "ERROR MESSAGE = " << admOp.getInternalLastErrorText()<<std::endl;
		return -1;
	}

	return 0;
}

void FIXS_CCH_BrmPersistentDataOwner:: adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_InvocationType invocation,
		const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
		ACS_APGCC_AdminOperationParamType**paramList)
{
	printf("------------------------------------------------------------\n");
	printf("                   adminOperationCallback called             \n");
	printf("------------------------------------------------------------\n");
	cout<<"oiHandle   : "<< oiHandle <<endl;
	cout<<"invocation : "<< invocation <<endl;
	cout<<"p_objName  : "<< p_objName <<endl;
	cout<<"operationId: "<< operationId <<endl;

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BrmPersistentDataOwner : adminOperationCallBack() called ",LOG_LEVEL_DEBUG);

	/*start*/
	int dim=0;
	int i=0;
	while(paramList[i]){
		i++;
		dim++;
	}
	unsigned long long requestId;
	i = 0;
	while( paramList[i] )
	{
		switch ( paramList[i]->attrType ) {
		/* ONLY FOR requested parameter */
		case ATTR_UINT64T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<unsigned long long *>(paramList[i]->attrValues) << endl;
			if(strcmp(paramList[i]->attrName,"requestId")==0){
				requestId = *reinterpret_cast<unsigned long long *>(paramList[i]->attrValues);
				cout << "requestId assigned " << requestId << endl;
			}
			break;
		default:
			break;

		}
		i++;
	}

	cout << " Received Operation with Identification = " << operationId << endl;
	int retVal =1;
	int retAdminOperationResult = this->adminOperationResult( oiHandle , invocation, retVal );
	cout << "Called adminOperationResult with retVal = " << retVal << " returned = " << retAdminOperationResult << endl;

	switch (operationId){
	case   BRM_PARTICIPANT_PERMIT_BACKUP:
		cout << "RECV: BRM_PARTICIPANT_PERMIT_BACKUP " <<endl;
		//Check if any type of upgrade is in progress
		if(FIXS_CCH_UpgradeManager::getInstance()->isAnyUpgradeOngoing())
			retVal = this->ResponseToBrfc(requestId, BRF_ERROR_OTHERS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT);
		else
		{
			retVal = this->ResponseToBrfc(requestId, BRF_SUCCESS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT);
			cout << "Setting backup ongoing to TRUE" << endl;
			FIXS_CCH_UpgradeManager::getInstance()->setBackupOngoing(true);
		}
//		retVal = this->ResponseToBrfc(requestId, BRF_SUCCESS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT);
		break;
	case BRM_PARTICIPANT_COMMIT_BACKUP:
		cout << "RECV: BRM_PARTICIPANT_COMMIT_BACKUP"<< endl;
		cout << "Setting backup ongoing to FALSE" << endl;
		FIXS_CCH_UpgradeManager::getInstance()->setBackupOngoing(false);
		retVal = this->ResponseToBrfc(requestId, BRF_SUCCESS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT);
		break;
	case BRM_PERSISTENT_DATA_OWNER_PREPARE_BACKUP:
		cout << "RECV: BRM_PERSISTENT_DATA_OWNER_PREPARE_BACKUP "<<endl;
		FIXS_CCH_UpgradeManager::getInstance()->setBackupOngoing(true);
		cout << "Setting backup ongoing to TRUE" << endl;
		retVal = this->ResponseToBrfc(requestId, BRF_SUCCESS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT);
		break;
	case BRM_PARTICIPANT_CANCEL_BACKUP:
		cout << "RECV: BRM_PARTICIPANT_CANCEL_BACKUP"<<endl;
		cout << "Setting backup ongoing to FALSE" << endl;
		FIXS_CCH_UpgradeManager::getInstance()->setBackupOngoing(false);
		retVal = this->ResponseToBrfc(requestId, BRF_SUCCESS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT);
		break;
	}


	// HERE YOU CAN RESPONDE WITH ACTION RESULT OR ACTION IN PROGRESS
	// BRM_PARTICIPANT_CONTAINER__REPORT_ACTION_PROGRESS simulation
	//	if (operationId == BRM_PERSISTENT_DATA_OWNER_PREPARE_BACKUP))
	//	{
	//		cout << ">>>PREPARING REPORT ACTION PROGRESS DATA"<<endl;
	//		retVal = this->ResponseToBrfc(requestId, BRF_SUCCESS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_PROGRESS);
	//		sleep(2); // Simulation your elaboration
	//		cout << ">>>PREPARING REPORT ACTION RESULT DATA "<<endl;
	//		//AT the end you must responde with a Report Action Result
	//		retVal = this->ResponseToBrfc(requestId, BRF_SUCCESS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT);
	//	}
	//	else{
	//		cout << ">>>PREPARING REPORT ACTION RESULT DATA"<<endl;
	//		retVal = this->ResponseToBrfc(requestId, BRF_SUCCESS, BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT);
	//	}
}

