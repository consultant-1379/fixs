/*
 * brfPersistDataOwner.cpp
 *
 *  Created on: Sep 8, 2016
 *      Author: xassore
 */

#include "imm/brfPersistDataOwner_oi.h"
#include "imm/imm.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_programconstants.h"

#include <acs_apgcc_adminoperation.h>

// TO BE MOVED INTO BRF constants file
const std::string CLASS_BRF_DATA_OWNER = "BrfPersistentDataOwner";
const std::string BRF_DATA_OWNER_IMPLNAME= "IthBrfDataPersistOwnerOi";


#define BRF_SUCCESS 		0
#define BRF_ERROR_OTHERS	99

// END

namespace imm
{

	BrfPersistDataOwner_OI::BrfPersistDataOwner_OI()
	: Base_OI(CLASS_BRF_DATA_OWNER, BRF_DATA_OWNER_IMPLNAME),
	  m_isBackupInProgress(false)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BrfPersistDataOwner_OI created");
	}


	BrfPersistDataOwner_OI::BrfPersistDataOwner_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
						Base_OI(managedObjectClassName, objectImplementerName),
						m_isBackupInProgress(false)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BrfPersistDataOwner_OI created");
	}

	BrfPersistDataOwner_OI::~BrfPersistDataOwner_OI()
	{

	}



void BrfPersistDataOwner_OI:: adminOperationCallback(ACS_APGCC_OiHandle oiHandle,ACS_APGCC_InvocationType invocation,
		const char* p_objName, ACS_APGCC_AdminOperationIdType operationId, ACS_APGCC_AdminOperationParamType** paramList)
{
	UNUSED(p_objName);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRFC-class : adminOperationCallback");
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " %s callback received",_adminOperCallbackOperName(operationId));

	int retVal = 1;
	adminOperationResult( oiHandle , invocation, retVal );

	unsigned long long requestId = 0;
	int i = 0;
	while( paramList[i] )
	{
		switch ( paramList[i]->attrType )
		{
			// ONLY FOR requested parameter
			case ATTR_UINT64T:
				if(strcmp(paramList[i]->attrName,"requestId")==0)
				{
					requestId = *reinterpret_cast<unsigned long long *>(paramList[i]->attrValues);
				}
				break;

			default:
				break;
		}
		i++;
	}

	switch (operationId)
	{
		case   BRF_PERMIT_BACKUP:
			// TO DO a check?
			m_isBackupInProgress = true;
		break;

		case BRF_PREPARE_BACKUP:
			m_isBackupInProgress = true;

//			retVal = ResponseToBrfc (requestId, BRF_SUCCESS, BRF_PARTICIPANT_REPORT_ACTION_PROGRESS);
//			{
//				// TO DO anything ???
//			}

		break;

		case BRF_CANCEL_BACKUP:
		case BRF_COMMIT_BACKUP:
			m_isBackupInProgress = false;

//			retVal = ResponseToBrfc (requestId, BRF_SUCCESS, BRF_PARTICIPANT_REPORT_ACTION_PROGRESS);
//			{
//				// TO DO anything ???
//			}

		break;
	}

	retVal = ResponseToBrfc(requestId, BRF_SUCCESS, BRF_PARTICIPANT_REPORT_ACTION_RESULT);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " Response to BRFC sent [<retcode == %d>].",retVal);

}



int BrfPersistDataOwner_OI::ResponseToBrfc(unsigned long long requestId,int brfStatus,int responseCode)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"BRFC-class :  ResponseToBrfc");
	acs_apgcc_adminoperation admOp;
	std::vector<ACS_APGCC_AdminOperationParamType> vectorIN;
	ACS_APGCC_AdminOperationParamType firstElem;
	ACS_APGCC_AdminOperationParamType secondElem;

	if( responseCode  == BRF_PARTICIPANT_REPORT_ACTION_RESULT )
	{
		ACS_APGCC_AdminOperationParamType thirdElem;

		//create first Element of parameter list
		char attName1[]= "requestId";
		firstElem.attrName =attName1;
		firstElem.attrType=ATTR_UINT64T;
		// return requestId previously checked
		unsigned long long intValue1 = requestId;
		firstElem.attrValues=reinterpret_cast<void*>(&intValue1);

		// create second Element of parameter list
		char attName2[]= "resultCode";
		secondElem.attrName =attName2;
		secondElem.attrType=ATTR_INT32T ;
		int intValue2 = brfStatus; // BRF_SUCCESS
		secondElem.attrValues=reinterpret_cast<void*>(&intValue2);

		//create the third Element of parameter list
		char attName3[]= "message";
		thirdElem.attrName =attName3;
		thirdElem.attrType=ATTR_STRINGT ;
		char* strValue = const_cast<char*>("");
		void* valueStr[1]={ reinterpret_cast<void*>(strValue) };
		thirdElem.attrValues=valueStr;

		//insert into the vector
		vectorIN.push_back(firstElem);
		vectorIN.push_back(secondElem);
		vectorIN.push_back(thirdElem);
	}
	else if (responseCode == BRF_PARTICIPANT_REPORT_ACTION_PROGRESS)
	{
		//create first Element of parameter list
		char attName1[]= "requestId";
		firstElem.attrName =attName1;
		firstElem.attrType=ATTR_UINT64T;
		unsigned long long intValue1 = requestId;
		firstElem.attrValues=reinterpret_cast<void*>(&intValue1);

		// create second Element of parameter list
		char attName3[]= "progressCount";
		secondElem.attrName =attName3;
		secondElem.attrType=ATTR_UINT32T ;
		int intValue3 = 50; // 50 %
		secondElem.attrValues=reinterpret_cast<void*>(&intValue3);

		//insert into the vector
		vectorIN.push_back(firstElem);
		vectorIN.push_back(secondElem);
	}

	const char *dnObjName1 = "brfParticipantContainerId=1";
	long long int timeOutVal_30sec = 30*(1000000000LL);

	int res1 = admOp.init();

	if (res1 != ACS_CC_SUCCESS)
	{
		return -1;
	}

	int returnValue1 = 1;
	/*Wait at least one second before invoke administrative operations in order to synchronize whit BRFC*/
	sleep(1);
	/* Invoke operation  */
	int retry = 0;
	while (admOp.adminOperationInvoke(dnObjName1, 0 , responseCode , vectorIN , &returnValue1, timeOutVal_30sec) == ACS_CC_FAILURE)
	{
		std::string tempErrMsg ="ERROR CODE = ";
		tempErrMsg += admOp.getInternalLastError();
		tempErrMsg += " ERROR MESSAGE = " ;
		tempErrMsg+= admOp.getInternalLastErrorText();


		if (admOp.getInternalLastError()!=((-1)*SA_AIS_ERR_EXIST)) break;

		sleep(1);
		if (++retry > 60) return -1;
	}

	return 0;
}


ACS_CC_ReturnType BrfPersistDataOwner_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentName, ACS_APGCC_AttrValues **attr)
{
	(void)oiHandle;
	(void)ccbId;
	(void)className;
	(void)parentName;
	(void)attr;
	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType BrfPersistDataOwner_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	(void)oiHandle;
	(void)ccbId;
	(void)objName;
	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType BrfPersistDataOwner_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	(void)oiHandle;
	(void)ccbId;
	(void)objName;
	(void)attrMods;
	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType BrfPersistDataOwner_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	(void)oiHandle;
	(void)ccbId;

	return ACS_CC_SUCCESS;
}

void BrfPersistDataOwner_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	UNUSED (oiHandle);
	UNUSED (ccbId);
}

void BrfPersistDataOwner_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	(void)oiHandle;
	(void)ccbId;
}

ACS_CC_ReturnType BrfPersistDataOwner_OI::updateRuntime(const char* p_objName, const char** p_attrName)
{
	(void)p_objName;
	(void)p_attrName;
	return ACS_CC_SUCCESS;
}


const char * BrfPersistDataOwner_OI::_adminOperCallbackOperName(ACS_APGCC_AdminOperationIdType operationId)
{
	switch (operationId)
	{
		case   BRF_PERMIT_BACKUP: return "PERMIT_BACKUP";

		case BRF_COMMIT_BACKUP: return "COMMIT_BACKUP";

		case BRF_PREPARE_BACKUP: return "PREPARE_BACKUP";

		case BRF_CANCEL_BACKUP: return "CANCEL_BACKUP";

		default : return "UNKNOWN callback operation";
	}
}


}
