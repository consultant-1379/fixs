/*
 * dst_oi.h
 *
 *  Created on: May 17, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_IMM_DST_OI_H_
#define ITHADM_CAA_INC_IMM_DST_OI_H_

#include "imm/base_oi.h"
#include "fixs_ith_programmacros.h"

#include <map>

namespace imm {

	class Dst_OI: public Base_OI {

	public:
		Dst_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName);

		virtual ~Dst_OI();

		virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

		virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

		virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

		virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);

		struct Dst: public Base_OI::Operation
		{
			operation::dstInfo info;

			bool completed;
			ImmAction action;

			Dst() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};

		};
	private:

		void resetOperationTable(const ACS_APGCC_CcbId&);

		/** @brief	Struct of IMM MoC elements
		 */
		typedef std::multimap<ACS_APGCC_CcbId, Dst> operationTable_t;

		/** @brief	m_routerOperationTable
		 *
		 *  Map of pending Router Operations
		 *
		 */
		operationTable_t m_dstOperationTable;
	};

} /* namespace imm */

#endif /* ITHADM_CAA_INC_IMM_DST_OI_H_ */
