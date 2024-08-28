/*
 * VrrpInterface_OI.h
 *
 *  Created on: Jun 1, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_IMM_VrrpInterface_OI_H_
#define ITHADM_CAA_INC_IMM_VrrpInterface_OI_H_

#include "imm/base_oi.h"
#include "fixs_ith_programmacros.h"

namespace imm {

	class VrrpInterface_OI : public Base_OI {

	public:
		VrrpInterface_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName);

		virtual ~VrrpInterface_OI();

		virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

		virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

		virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

		virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		struct VrrpInterface: public Base_OI::Operation
		{
			operation::vrrpInterfaceInfo info;

			bool completed;
			ImmAction action;
			std::set<std::string> previousVrrpSessionsDN;

			VrrpInterface() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};

		};

	private:

		void resetOperationTable(const ACS_APGCC_CcbId&);

		/** @brief	Struct of IMM MoC elements
		 */
		typedef std::multimap<ACS_APGCC_CcbId, VrrpInterface> operationTable_t;

		/** @brief	m_vrrpInterfaceOperationTable
		 *
		 *  Map of pending VrrpInterface Operations
		 *
		 */
		operationTable_t m_vrrpInterfaceOperationTable;

	};

} /* namespace imm */

#endif /* ITHADM_CAA_INC_IMM_VrrpInterface_OI_H_ */
