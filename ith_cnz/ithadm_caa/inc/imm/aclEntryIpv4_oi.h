/*
 * dst_oi.h
 *
 *  Created on: Oct 13, 2016
 *      Author: xclaele
 */

#ifndef ITHADM_CAA_INC_IMM_ACL_ENTRY_OI_H_
#define ITHADM_CAA_INC_IMM_ACL_ENTRY_OI_H_

#include "imm/base_oi.h"
#include "fixs_ith_programmacros.h"

namespace imm {

	class AclEntry_OI: public Base_OI {

	public:
		AclEntry_OI();

		virtual ~AclEntry_OI();

		virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

		virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

		virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

		virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);

		struct AclEntry: public Base_OI::Operation
		{
			operation::aclEntryInfo info;

			bool completed;
			ImmAction action;

			AclEntry() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};

		};
	private:

		void resetOperationTable(const ACS_APGCC_CcbId&);

		/** @brief	Struct of IMM MoC elements
		 */
		typedef std::multimap<ACS_APGCC_CcbId, AclEntry> operationTable_t;

		/** @brief	m_routerOperationTable
		 *
		 *  Map of pending Router Operations
		 *
		 */
		operationTable_t m_aclEntryOperationTable;
	};

} /* namespace imm */

#endif /* ITHADM_CAA_INC_IMM_ACL_ENTRY_OI_H_ */
