/*
 * bridgePort_oi.h
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_IMM_BRIDGEPORT_OI_H_
#define ITHADM_CAA_INC_IMM_BRIDGEPORT_OI_H_

#include "imm/base_oi.h"
#include "fixs_ith_programmacros.h"

#include <map>

namespace imm
{

	/**
	 * @class BridgePort_OI
	 *
	 * @brief
	 * Object Implementer of BridgePort MO.
	 *
	 *
	 * @sa Base_OI
	 */
	class BridgePort_OI : public Base_OI
	{
 	 public:

		///  Constructor.
		BridgePort_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName);

		///  Destructor.
		virtual ~BridgePort_OI();

		virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

		virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

		virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

		virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);

		virtual ACS_CC_ReturnType updateRuntime(const char* p_objName, const char** p_attrName);

		struct BridgePort: public Base_OI::Operation
		{
			operation::bridgePortInfo info;

			bool completed;
			ImmAction action;

			BridgePort() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};

		};

 	 private:

		void resetOperationTable(const ACS_APGCC_CcbId&);

		ACS_CC_ReturnType checkTransactionAugmentation(const BridgePort& bridgePortObj, ACS_APGCC_OiHandle& oiHandle, ACS_APGCC_CcbId& ccbId);

		/** @brief	Struct of IMM MoC elements
		 */
		typedef std::multimap<ACS_APGCC_CcbId, BridgePort> operationTable_t;

		/** @brief	m_bridgePortOperationTable
		 *
		 *  Map of pending BridgePort Operations
		 *
		 */
		operationTable_t m_bridgePortOperationTable;

	};

} /* namespace imm */


#endif /* ITHADM_CAA_INC_IMM_BRIDGEPORT_OI_H_ */
