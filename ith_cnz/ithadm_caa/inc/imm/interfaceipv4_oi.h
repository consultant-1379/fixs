/*
 * Interfaceipv4_OI.h
 *
 *  Created on: May 6, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_IMM_Interfaceipv4_OI_H_
#define ITHADM_CAA_INC_IMM_Interfaceipv4_OI_H_

#include "imm/base_oi.h"
#include "fixs_ith_programmacros.h"

namespace imm {

	class Interfaceipv4_OI : public Base_OI {

	public:
		Interfaceipv4_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName);

		virtual ~Interfaceipv4_OI();

		virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

		virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

		virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

		virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);

		virtual ACS_CC_ReturnType updateRuntime(const char* p_objName, const char** p_attrName);

		struct InterfaceIPv4: public Base_OI::Operation
		{
			operation::interfaceInfo info;

			bool completed;
			ImmAction action;

			InterfaceIPv4() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};

		};

	private:

		void resetOperationTable(const ACS_APGCC_CcbId&);

		ACS_CC_ReturnType checkTransactionAugmentation(const InterfaceIPv4& interfaceObj, ACS_APGCC_OiHandle& oiHandle, ACS_APGCC_CcbId& ccbId);

		ACS_CC_ReturnType setVlanReservedByAttribute(const operation::interfaceInfo& interfaceObjInfo, ACS_APGCC_CcbHandle& ccbHandleVal, ACS_APGCC_AdminOwnerHandle& adminOwnerHandlerVal);

		ACS_CC_ReturnType resetVlanReservedByAttribute(const operation::interfaceInfo& interfaceObjInfo, ACS_APGCC_CcbHandle& ccbHandleVal, ACS_APGCC_AdminOwnerHandle& adminOwnerHandlerVal);

		bool getCurrentBfdProfile(const InterfaceIPv4& interfaceIPv4Obj, std::string& bfdProfile);

		bool getCurrentAcl(const InterfaceIPv4& interfaceIPv4Obj, std::string& aclDn);

		bool getInterfaceIPv4DNsReferencingBfdProfile(const std::string& bfdProfileDN, const InterfaceIPv4& interfaceIPv4Obj, std::set<std::string>& interfaceIPv4DNs);

		bool getInterfaceIPv4DNsReferencingAclIpv4(const std::string& aclIpv4DN, const InterfaceIPv4& interfaceIPv4Obj, std::set<std::string>& interfaceIPv4DNs);

		ACS_CC_ReturnType modifyReservedByAttribute(ACS_APGCC_CcbHandle ccbHandle, ACS_APGCC_AdminOwnerHandle adminOwnerHandler, const std::string& objectDN, const std::set<std::string>& interfaceIPv4DNs );

		/** @brief	Struct of IMM MoC elements
		 */
		typedef std::multimap<ACS_APGCC_CcbId, InterfaceIPv4> operationTable_t;

		/** @brief	m_routerOperationTable
		 *
		 *  Map of pending Router Operations
		 *
		 */
		operationTable_t m_interfaceIPv4OperationTable;

	};

} /* namespace imm */

#endif /* ITHADM_CAA_INC_IMM_Interfaceipv4_OI_H_ */
