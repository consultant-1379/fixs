/*
 * FIXS_CCH_BrmPersistentDataOwner.h
 *
 *  Created on: Jul 11, 2012
 *      Author: tcslali
 */

#ifndef FIXS_CCH_BRMPERSISTENTDATAOWNER_H_
#define FIXS_CCH_BRMPERSISTENTDATAOWNER_H_

#include "acs_apgcc_objectimplementerinterface_V3.h"
#include "acs_apgcc_paramhandling.h"
#include "ACS_CC_Types.h"
#include "acs_apgcc_adminoperationtypes.h"
#include "acs_apgcc_omhandler.h"
#include "acs_apgcc_adminoperation.h"
#include "ACS_APGCC_Util.H"

#include "ACS_TRA_Logging.h"

#include "FIXS_CCH_UpgradeManager.h"
#include "FIXS_CCH_DiskHandler.h"
#include <iostream>

#define BRM_SYSTEM_BACKUP_MANAGER_CREATE_BACKUP	0
#define BRM_SYSTEM_BACKUP_MANAGER_DELETE  	1
#define BRM_SYSTEM_BACKUP_RESTORE 3
#define BRM_SYSTEM_BACKUP_CONFIRM_RESTORE 4
#define BRM_SYSTEM_BACKUP_CANCEL_CURRENT_ACTION 5

#define BRF_VERSION_PRA_
#ifndef BRF_VERSION_PRA_

#define BRM_PARTICIPANT_PERMIT_BACKUP 6
#define BRM_PARTICIPANT_COMMIT_BACKUP 7
#define BRM_PARTICIPANT_CANCEL_BACKUP 8

#define BRM_PERSISTENT_DATA_OWNER__PREPARE_BACKUP 9

#define BRM_PERSISTENT_STORAGE_OWNER__CREATE_BACKUP 10
#define BRM_PERSISTENT_STORAGE_OWNER__PERMIT_DELETE 11
#define BRM_PERSISTENT_STORAGE_OWNER__PREPARE_DELETE 12
#define BRM_PERSISTENT_STORAGE_OWNER__COMMIT_DELETE 13
#define BRM_PERSISTENT_STORAGE_OWNER__CANCEL_DELETE 14
#define BRM_PERSISTENT_STORAGE_OWNER__PERMIT_RESTORE 15
#define BRM_PERSISTENT_STORAGE_OWNER__PREPARE_RESTORE 16
#define BRM_PERSISTENT_STORAGE_OWNER__COMMIT_RESTORE 17
#define BRM_PERSISTENT_STORAGE_OWNER__CANCEL_RESTORE 18

#define BRM_PARTICIPANT_CONTAINER__REPORT_ACTION_PROGRESS 19
#define BRM_PARTICIPANT_CONTAINER__REPORT_ACTION_RESULT 20

#else

#define BRM_PARTICIPANT_PERMIT_BACKUP 0
#define BRM_PARTICIPANT_COMMIT_BACKUP 3
#define BRM_PARTICIPANT_CANCEL_BACKUP 4

#define BRM_PERSISTENT_DATA_OWNER_PREPARE_BACKUP 1
#define BRM_PERSISTENT_STORAGE_OWNER_CREATE_BACKUP 2
#define BRM_PERSISTENT_STORAGE_OWNER_PERMIT_DELETE 5
#define BRM_PERSISTENT_STORAGE_OWNER_PREPARE_DELETE 6
#define BRM_PERSISTENT_STORAGE_OWNER_COMMIT_DELETE 7
#define BRM_PERSISTENT_STORAGE_OWNER_CANCEL_DELETE 8
#define BRM_PERSISTENT_STORAGE_OWNER_PERMIT_RESTORE 9
#define BRM_PERSISTENT_STORAGE_OWNER_PREPARE_RESTORE 10
#define BRM_PERSISTENT_STORAGE_OWNER_COMMIT_RESTORE 11
#define BRM_PERSISTENT_STORAGE_OWNER_CANCEL_RESTORE 12

#define BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_PROGRESS 21
#define BRM_PARTICIPANT_CONTAINER_REPORT_ACTION_RESULT 22

#endif

// Define status for backupType parameter

#define BRF_SUCCESS 	0
#define BRF_ERROR_DISKSPACE 1
#define BRF_ERROR_CONFLICT 	2
#define BRF_ERROR_OTHERS	99


class FIXS_CCH_BrmPersistentDataOwner : public acs_apgcc_objectimplementerinterface_V3{
public :
	FIXS_CCH_BrmPersistentDataOwner();
	//FIXS_CCH_BrmPersistentDataOwner(string p_impName );
	//FIXS_CCH_BrmPersistentDataOwner(string p_objName, string p_impName, ACS_APGCC_ScopeT p_scope );
		virtual ~FIXS_CCH_BrmPersistentDataOwner();
		ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);
		ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);
		ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);
		ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);
		void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);
		void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);
		ACS_CC_ReturnType updateRuntime(const char* p_nameObj, const char** p_nameAttr);
		//ACS_CC_ReturnType updateRuntime(const char* p_nameObj, const char* p_nameAttr);
		/*the callback*/
		void adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_InvocationType invocation,
		const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
			ACS_APGCC_AdminOperationParamType**paramList);
		int ResponseToBrfc(unsigned long long requestId,int brfStatus,int responseCode);
		const char* getIMMClassName() const {return m_ImmClassName.c_str();};


		private:

			std::string m_ImmClassName;

			ACS_TRA_Logging *FIXS_CCH_logging;
	};

#endif /* FIXS_CCH_BRMPERSISTENTDATAOWNER_H_ */
