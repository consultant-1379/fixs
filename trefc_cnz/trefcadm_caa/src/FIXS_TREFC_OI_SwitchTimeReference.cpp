/*
 * FIXS_TREFC_OI_SwitchTimeReference.cpp
 *
 *  Created on: Jul 10, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_OI_SwitchTimeReference.h"


/*============================================================================
	ROUTINE: FIXS_TREFC_OI_SwitchTimeReference
 ============================================================================ */
FIXS_TREFC_OI_SwitchTimeReference::FIXS_TREFC_OI_SwitchTimeReference() :  acs_apgcc_objectimplementerinterface_V2(IMM_Util::IMPLEMENTER_SWITCHTIMEREFERENCE),
m_ImmClassName(IMM_Util::classSwitchTimeReference)
{
//	FIXS_TREFC_OI_SwitchTimeReferenceTrace = new ACS_TRA_trace("FIXS_TREFC_OI_SwitchTimeReference","C400");
}

/*============================================================================
	ROUTINE: ~FIXS_TREFC_OI_SwitchTimeReference
 ============================================================================ */
FIXS_TREFC_OI_SwitchTimeReference::~FIXS_TREFC_OI_SwitchTimeReference()
{
}


ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeReference::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
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

ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeReference::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
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

ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeReference::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
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
	int scxSlot = -1, switchSlot = -1;
	bool modifiedStatus = false;
	bool modifiednetDn = false;
	
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	
	if(environment!=TREFC_Util::SMX)// only in SCX environment
	{

		if (0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ, objName) )
			scxSlot = 0;
		else if (0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ, objName) )
			scxSlot = 25;
		else if (0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ, objName) )
			switchSlot = 0;
		else if (0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ, objName) )
			switchSlot = 25;
		else
			return ACS_CC_FAILURE;

		std::string  tempSwitchObj;
		std:: string  netconfigDn;
		if(scxSlot == 0)
		{
			std::string tempSwitch0(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ);
			tempSwitchObj = tempSwitch0;

			std::string tempNetconfigDn = "id=networkConfiguration_0," + tempSwitch0;
			netconfigDn = tempNetconfigDn;
		}
		else if(scxSlot == 25)
		{
			std::string tempSwitch25(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ);
			tempSwitchObj = tempSwitch25;

			std::string tempNetconfigDn = "id=networkConfiguration_0," + tempSwitch25;
			netconfigDn = tempNetconfigDn;
		}
		else if(switchSlot == 0)
		{
			std::string tempScx0(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ);
			tempSwitchObj = tempScx0;

			std::string tempNetconfigDn = "id=networkConfiguration_0," + tempScx0;
			netconfigDn = tempNetconfigDn;
		}
		else if(switchSlot == 25)
		{
			std::string tempScx25(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ);
			tempSwitchObj = tempScx25;

			std::string tempNetconfigDn = "id=networkConfiguration_0," + tempScx25;
			netconfigDn = tempNetconfigDn;
		}



		//	UNUSED(attrMods);
		char * timeStatus = NULL;
		char * tempNetconfigDn = NULL;

		for(size_t idx = 0; attrMods[idx] != NULL ; idx++)
		{
			if( ACS_APGCC_ATTR_VALUES_REPLACE == attrMods[idx]->modType )
			{
				if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_STATUS, attrMods[idx]->modAttr.attrName) )
				{
					timeStatus = reinterpret_cast<char *>(*attrMods[idx]->modAttr.attrValues);
					modifiedStatus = true;
				}
				else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_REF_STRUCT, attrMods[idx]->modAttr.attrName) )
				{
					tempNetconfigDn = reinterpret_cast<char *>(*attrMods[idx]->modAttr.attrValues);
					cout << " netconfDn value is : " << tempNetconfigDn <<endl;
					cout << " netconfDn length is : " << strlen(tempNetconfigDn) <<endl;
					modifiednetDn = true;
				}
			}
		}
		if (modifiedStatus || modifiednetDn) {

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_ScxTimeRefrence] Modifying Network Configuration for"
					"\n time refernce status   = %s"
					"\n network configuration dn =%s",  timeStatus, netconfigDn.c_str());
			cout << tmpStr << endl;
			//       FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);


			ACS_APGCC_CcbHandle ccbHandleVal;
			ACS_APGCC_AdminOwnerHandle adminOwnwrHandlerVal;

			ACS_CC_ReturnType retVal = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnwrHandlerVal);
			if (retVal != ACS_CC_SUCCESS)
			{
				cout << " getCcbAugmentationInitialize::ACS_CC_FAILURE " << endl;
				return ACS_CC_FAILURE;
			}
			else
			{
				cout << " getCcbAugmentationInitialize::ACS_CC_SUCCESS " << endl;
			}

			ACS_CC_ImmParameter timeStatusParam = IMM_Util::defineParameterString(IMM_Util::ATT_SCXTIMEREFERENCE_STATUS,ATTR_STRINGT,timeStatus,1);

			ACS_CC_ImmParameter netConfigParam;
			netConfigParam.attrName = const_cast<char *>(IMM_Util::ATT_SCXTIMEREFERENCE_REF_STRUCT);
			netConfigParam.attrType = ATTR_NAMET;
			netConfigParam.attrValuesNum = 1;
			netConfigParam.attrValues = new void*[netConfigParam.attrValuesNum];

			if(isValidNetworkDn(tempNetconfigDn))
			{
				char *rdnValue = const_cast<char *>(netconfigDn.c_str());
				cout << " netconfigDn = " << netconfigDn.c_str() << endl;
				netConfigParam.attrValues[0] = reinterpret_cast<void *>(rdnValue);
			}
			else
			{
				char rdnValue = (char)0;
				netConfigParam.attrValues[0] = reinterpret_cast<void *>(&rdnValue);
			}

			cout << " modifyObjectAugmentCcb::before calling " << tempSwitchObj.c_str()<<endl;
			cout << " modifyObjectAugmentCcb::before calling " << netconfigDn.c_str() <<endl;

			if(modifiedStatus)
			{
				retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, tempSwitchObj.c_str(), &timeStatusParam);
				if (retVal != ACS_CC_SUCCESS){
					cout << " modifyObjectAugmentCcb::ACS_CC_FAILURE " << endl;
					delete [] timeStatusParam.attrValues;
					delete [] netConfigParam.attrValues;
					return ACS_CC_FAILURE;
				}      
			}
			if (modifiednetDn)
			{
				cout << "tempSwitchObj= " << tempSwitchObj.c_str() <<endl;
				retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, tempSwitchObj.c_str(), &netConfigParam);
				if (retVal != ACS_CC_SUCCESS)
				{
					cout << " modifyObjectAugmentCcb::ACS_CC_FAILURE " << endl;
					delete [] timeStatusParam.attrValues;
					delete [] netConfigParam.attrValues;
					return ACS_CC_FAILURE;
				}
			}
			if (retVal == ACS_CC_SUCCESS)
			{
				retVal = applyAugmentCcb (ccbHandleVal);

				if (retVal != ACS_CC_SUCCESS){
					cout << " applyAugmentCcb::ACS_CC_FAILURE " << endl;
					delete [] timeStatusParam.attrValues;
					delete [] netConfigParam.attrValues;
					return ACS_CC_FAILURE;
				}else{
					cout << " applyAugmentCcb::ACS_CC_SUCCESS " << endl;
					cout << " modifyObjectAugmentCcb::SUCCESS value for object =  " << tempSwitchObj.c_str() << endl;
				}
			}

			delete [] timeStatusParam.attrValues;
			delete [] netConfigParam.attrValues;
		}

	}

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeReference::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

void FIXS_TREFC_OI_SwitchTimeReference::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

void FIXS_TREFC_OI_SwitchTimeReference::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

ACS_CC_ReturnType FIXS_TREFC_OI_SwitchTimeReference::updateRuntime(const char* p_objName, const char* p_attrName)
{
	printf("------------------------------------------------------------\n");
	printf("                   updateRuntime called                     \n");
	printf("------------------------------------------------------------\n");

	cout<<"Object Name: "<<p_objName<<endl;
	cout<<"Attribute Name: "<<p_attrName<<endl;

	printf("------------------------------------------------------------\n");

	FIXS_TREFC_Manager *objTrefcManager = NULL;
	objTrefcManager = FIXS_TREFC_Manager::getInstance();

	int switch_status = 0, intStatus = IMM_Util::NOT_CONNECTED;

	string activeRef_switch(""), ipAddress(""), activeRef_name("");

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	int switchSlot = -1, scxSlot = -1, slot = -1;

	if (0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ, p_objName) )
		scxSlot = 0;
	else if (0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ, p_objName) )
		scxSlot = 25;
	else if (0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ, p_objName) )
		switchSlot = 0;
	else if (0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ, p_objName) )
		switchSlot = 25;
	else
		return ACS_CC_FAILURE;


	cout << "Setting status for SWITCH object: " << p_objName << endl;

	if(switchSlot != -1)
	{
		slot = switchSlot;
		ACS_CC_ImmParameter switchAstat = IMM_Util::defineParameterInt(IMM_Util::ATT_SWITCHTIMEREFERENCE_STATUS,ATTR_INT32T,&intStatus,1);
		result = this->modifyRuntimeObj(p_objName, &switchAstat);
		if(result == ACS_CC_FAILURE) cout << "FAILED to set Switch status for " << p_objName << endl;

		ACS_CC_ImmParameter switchRef = IMM_Util::defineParameterString(IMM_Util::ATT_SWITCHTIMEREFERENCE_REF_ID,ATTR_STRINGT,"-",1);
		result = this->modifyRuntimeObj(p_objName, &switchRef);
		if(result == ACS_CC_FAILURE) cout << "FAILED to set Switch status for " << p_objName << endl;

		delete [] switchAstat.attrValues;
		delete [] switchRef.attrValues;
	}
	
	if(scxSlot != -1)
	{
		slot = scxSlot;
		ACS_CC_ImmParameter scxstat = IMM_Util::defineParameterInt(IMM_Util::ATT_SCXTIMEREFERENCE_STATUS,ATTR_INT32T,&intStatus,1);
		result = this->modifyRuntimeObj(p_objName, &scxstat);
		if(result == ACS_CC_FAILURE) cout << "FAILED to set Scx status for " << p_objName << endl;

		ACS_CC_ImmParameter scxRef = IMM_Util::defineParameterString(IMM_Util::ATT_SCXTIMEREFERENCE_REF_ID,ATTR_STRINGT,"-",1);
		result = this->modifyRuntimeObj(p_objName, &scxRef);
		if(result == ACS_CC_FAILURE) cout << "FAILED to set Scx status for " << p_objName << endl;
		delete [] scxstat.attrValues;
		delete [] scxRef.attrValues;
	}

	if(objTrefcManager->getTRefListStatus(&switch_status, &activeRef_switch, &activeRef_name, slot) == 0)
	{
		if((switch_status == 1) || (switch_status == 3) || (switch_status == 4) || (switch_status == 99))
		{
			intStatus = IMM_Util::NOT_CONNECTED;
		}
		else if(switch_status == 2)
		{
			intStatus = IMM_Util::RESYNCH;
		}
		else if((switch_status == 5) || (switch_status == 6))
		{

			cout << "\n!!!!!!!!!!!!!!!!!!!!!Active reference!!!!!!!!!!!!!!!!!!!!!!!!\n" << endl;
			intStatus = IMM_Util::CONNECTED;
		}

		if (intStatus != IMM_Util::NOT_CONNECTED) {
			if(switchSlot != -1)
			{

				cout << "Setting status for SWITCH for " << p_objName << endl;
				ACS_CC_ImmParameter switchAstat = IMM_Util::defineParameterInt(IMM_Util::ATT_SWITCHTIMEREFERENCE_STATUS,ATTR_INT32T,&intStatus,1);
				result = this->modifyRuntimeObj(p_objName, &switchAstat);
				if(result == ACS_CC_FAILURE) cout << "FAILED to set Switch status for " << p_objName << endl;

				ACS_CC_ImmParameter switchRef = IMM_Util::defineParameterString(IMM_Util::ATT_SWITCHTIMEREFERENCE_REF_ID,ATTR_STRINGT,activeRef_name.c_str(),1);
				result = this->modifyRuntimeObj(p_objName, &switchRef);
				if(result == ACS_CC_FAILURE) cout << "FAILED to set Switch status for " << p_objName << endl;

				delete [] switchAstat.attrValues;
				delete [] switchRef.attrValues;
			}

			if(scxSlot != -1)
			{

				cout << "Setting status for Scx for " << p_objName << endl;

				ACS_CC_ImmParameter scxstat = IMM_Util::defineParameterInt(IMM_Util::ATT_SCXTIMEREFERENCE_STATUS,ATTR_INT32T,&intStatus,1);
				result = this->modifyRuntimeObj(p_objName, &scxstat);
				if(result == ACS_CC_FAILURE) cout << "FAILED to set Scx time status for " << p_objName << endl;

				ACS_CC_ImmParameter scxRef = IMM_Util::defineParameterString(IMM_Util::ATT_SCXTIMEREFERENCE_REF_ID,ATTR_STRINGT,activeRef_name.c_str(),1);
				result = this->modifyRuntimeObj(p_objName, &scxRef);
				if(result == ACS_CC_FAILURE) cout << "FAILED to set Scx time status for " << p_objName << endl;
				
				delete [] scxstat.attrValues;
				delete [] scxRef.attrValues;
					
			}

		}

	}
	return result;
}


void FIXS_TREFC_OI_SwitchTimeReference::adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
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

bool FIXS_TREFC_OI_SwitchTimeReference::isValidNetworkDn(char *dn)
{
	std::string tempScxANetDn(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ);
	std::string tempScxBNetDn(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ);
	std::string tempSwitchANetDn(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ);
	std::string tempSwitchBNetDn(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ);

	tempScxANetDn = "id=networkConfiguration_0," + tempScxANetDn;
	tempScxBNetDn = "id=networkConfiguration_0," + tempScxBNetDn;
	tempSwitchANetDn = "id=networkConfiguration_0," + tempSwitchANetDn;
	tempSwitchBNetDn = "id=networkConfiguration_0," + tempSwitchBNetDn;
	std::string netDn(dn);

	if((tempScxANetDn.compare(netDn) == 0) || (tempScxBNetDn.compare(netDn) == 0) || (tempSwitchANetDn.compare(netDn) == 0) || (tempSwitchBNetDn.compare(netDn) == 0) )
		return true;
	else
		return false;
}
