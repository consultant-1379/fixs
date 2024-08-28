/*
 * FIXS_TREFC_OI_SwitchTimeService.cpp
 *
 *  Created on: Jan 3, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_OI_SwitchTimeService.h"
#include "FIXS_TREFC_Logger.h"

/*============================================================================
	ROUTINE: FIXS_TREFC_OI_SwitchTimeService
 ============================================================================ */
FIXS_TREFC_OI_SwitchTimeService::FIXS_TREFC_OI_SwitchTimeService() :  acs_apgcc_objectimplementerinterface_V2(IMM_Util::IMPLEMENTER_SWITCHTIMESERVICE),
m_ImmClassName(IMM_Util::classSwitchTimeService)
{
//	FIXS_TREFC_OI_SwitchTimeServiceTrace = new ACS_TRA_trace("FIXS_TREFC_OI_SwitchTimeService","C400");
}

/*============================================================================
	ROUTINE: ~FIXS_TREFC_OI_SwitchTimeService
 ============================================================================ */
FIXS_TREFC_OI_SwitchTimeService::~FIXS_TREFC_OI_SwitchTimeService()
{
}


ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeService::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
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
	//int res;
	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	result = ACS_CC_SUCCESS;
	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeService::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectDeleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;
	//int res;
	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeService::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
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

	bool servType = false;
	bool refType = false;
	IMM_Util::TimeServiceType valueServ = IMM_Util::NONE;
	IMM_Util::ExternalReferenceType valueRef = IMM_Util::NOT_CONFIGURED;
	std::string servStr = "";
	std::string refStr = "";
	bool isScxObj=false;
	for(size_t idx = 0; attrMods[idx] != NULL ; idx++)
	{

		if( attrMods[idx]->modAttr.attrValuesNum == 0 )
		{
			return ACS_CC_FAILURE;
		}

		if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMESERVICE_TYPE, attrMods[idx]->modAttr.attrName) )
		{
			valueServ = *reinterpret_cast<IMM_Util::TimeServiceType *>(*attrMods[idx]->modAttr.attrValues);
			servType = true;
			{
				if (valueServ == IMM_Util::NTP) {
					servStr = "NTP";
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"modify callback NTP" <<std::endl; //mychange
				} else if (valueServ == IMM_Util::NTPHP) {
					servStr = "NTPHP";
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"modify callback NTPHP" <<std::endl; //mychange
				} else
					servStr = "NOT_CONFIGURED";
					std::cout << __FUNCTION__ << "@" << __LINE__ <<"modify callback NOT_CONFIG" <<std::endl; //mychange

				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_SwitchTimeService] Trying to change Service Type attribute to %s", servStr.c_str());
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
			}

		}
		else if ( 0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMESERVICE_REF_TYPE, attrMods[idx]->modAttr.attrName) )
		{
			valueRef = *reinterpret_cast<IMM_Util::ExternalReferenceType *>(*attrMods[idx]->modAttr.attrValues);
			refType = true;
			{
				if (valueRef == IMM_Util::NETWORK_REFERENCE) {
					refStr = "NETWORK_REFERENCE";
				} else if (valueRef == IMM_Util::GPS_REFERENCE) {
					refStr = "GPS_REFERENCE";
				} else
					refStr = "NOT_CONFIGURED";

				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_SwitchTimeService] Trying to change Reference Type attribute to %s", refStr.c_str());
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
			}

		}

	}

	if(servType) {
		int err = 0;
		
		std::cout << __FUNCTION__ << "@" << __LINE__ <<"before setSeviceType in switch" <<std::endl; 		
		
		if (!FIXS_TREFC_Manager::getInstance()->setSeviceType(valueServ,err)) {
			this->setExitCode(err,TREFC_Util::getErrrorText(err));
			return ACS_CC_FAILURE;
		}
	}
	if (refType) {
		int err = 0;
		if (!FIXS_TREFC_Manager::getInstance()->setReferenceType(valueRef,err)) {
			this->setExitCode(err,TREFC_Util::getErrrorText(err));
			return ACS_CC_FAILURE;
		}
		
	}
	//check whether callback is from scx or switch
	if( 0 == ACE_OS::strcmp(objName, IMM_Util::ATT_SCXTIMESERVICE_DN) )
	{
		cout<<"callback from SCX"<<endl;
		isScxObj=true;
	}
	else
		cout<<"callback from SWITCH"<<endl;

	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	
	if(environment!=TREFC_Util::SMX)// only in SCX environment	
	{	
		if((refType==true)||(servType==true)) {
			int servVal=valueServ;
			int refVal=valueRef;			
			
			ACS_CC_ImmParameter ServTypePar = IMM_Util::defineParameterInt(IMM_Util::ATT_SCXTIMESERVICE_TYPE,ATTR_INT32T,&servVal,1);
			ACS_CC_ImmParameter RefTypePar = IMM_Util::defineParameterInt(IMM_Util::ATT_SCXTIMESERVICE_REF_TYPE,ATTR_INT32T,&refVal,1);

			ACS_APGCC_CcbHandle ccbHandleVal;
			ACS_APGCC_AdminOwnerHandle adminOwnwrHandlerVal;
			ACS_CC_ReturnType retVal = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnwrHandlerVal);
			if (retVal != ACS_CC_SUCCESS)
			{
				cout<<"failed in init"<<endl;
				delete [] ServTypePar.attrValues;
				delete [] RefTypePar.attrValues;
				return ACS_CC_FAILURE;				
			}
				
			if(isScxObj==true){
				if(servType)
					retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, IMM_Util::ATT_SWITCHTIMESERVICE_DN, &ServTypePar);
				if(refType)
					retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, IMM_Util::ATT_SWITCHTIMESERVICE_DN, &RefTypePar);
			}
			else
			{
				if(servType)
					retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, IMM_Util::ATT_SCXTIMESERVICE_DN, &ServTypePar);
				if(refType)
					retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, IMM_Util::ATT_SCXTIMESERVICE_DN, &RefTypePar);
			}
			if (retVal != ACS_CC_SUCCESS)
			{
				cout<<"failed in modify"<<endl;
				delete [] ServTypePar.attrValues;
				delete [] RefTypePar.attrValues;		 
				return ACS_CC_FAILURE;
			}
			retVal = applyAugmentCcb(ccbHandleVal);
			if (retVal != ACS_CC_SUCCESS)
			{
				cout<<"failed in apply"<<endl;
				delete [] ServTypePar.attrValues;
				delete [] RefTypePar.attrValues;
				return ACS_CC_FAILURE;
			}
			
			cout<<"succ in modifying"<<endl;
			std::cout << __FUNCTION__ << "@" << __LINE__ <<"after setSeviceType" <<std::endl; //mychange
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_SwitchTimeService] Service Type successfully set to %s", servStr.c_str());
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

			delete [] ServTypePar.attrValues;
			delete [] RefTypePar.attrValues;
		}
	}
		return ACS_CC_SUCCESS;
 
}
	

ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeService::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " COMPLETECallback " << endl;
	cout << endl;

	
					//error = TREFC_Util::SERVICE_TYPE_CHANGE_NOT_ALLOWED;
	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}

void FIXS_TREFC_OI_SwitchTimeService::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbAbortCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " ABORTCallback " << endl;
	cout << endl;

}

void FIXS_TREFC_OI_SwitchTimeService::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbApplyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " APPLYCallback " << endl;
	cout << endl;

}

ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeService::updateRuntime(const char* p_objName, const char* p_attrName)
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


void FIXS_TREFC_OI_SwitchTimeService::adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
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



//############## ACTION 1 ----

	if (operationId == 1)
	{
		cout << " ----------- ACTION 1 ------------------\n  " << endl;
		retVal = SA_AIS_OK;
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;

	}

}

