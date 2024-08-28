/*
 * brfPersistentDataOwner_oi.h
 *
 *  Created on: Sep 8, 2016
 *      Author: xassore
 */

#ifndef ITHADM_CAA_INC_IMM_BRFPERSISTDATAOWNER_OI_H_
#define ITHADM_CAA_INC_IMM_BRFPERSISTDATAOWNER_OI_H_

#include "imm/base_oi.h"

#define BRF_PARTICIPANT_REPORT_ACTION_PROGRESS 21
#define BRF_PARTICIPANT_REPORT_ACTION_RESULT 22

namespace imm
{

	class BrfPersistDataOwner_OI: public Base_OI
	{

		enum brf_admincallback {

			BRF_PERMIT_BACKUP = 0,
			BRF_PREPARE_BACKUP =1,
			BRF_COMMIT_BACKUP = 3,
			BRF_CANCEL_BACKUP = 4,

		};

		public:

			BrfPersistDataOwner_OI();

			BrfPersistDataOwner_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName);

			virtual ~BrfPersistDataOwner_OI();

			virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

			virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

			virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

			virtual ACS_CC_ReturnType updateRuntime(const char* p_objName, const char** p_attrName);

			virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName,
					ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);

			int ResponseToBrfc(unsigned long long, int, int);

			inline bool isBackupInProgress() {return m_isBackupInProgress;};

		private:
			const char * _adminOperCallbackOperName(ACS_APGCC_AdminOperationIdType operationId);
		private:

			bool m_isBackupInProgress ;

	};

}

#endif /* ITHADM_CAA_INC_IMM_BRFPERSISTDATAOWNER_OI_H_ */
