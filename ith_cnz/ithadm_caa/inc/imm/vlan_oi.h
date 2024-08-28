/*
 * vlan_oi.h
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_IMM_VLAN_OI_H_
#define ITHADM_CAA_INC_IMM_VLAN_OI_H_

#include "imm/base_oi.h"
#include "fixs_ith_programmacros.h"

#include <map>

namespace imm
{

	/**
	 * @class Vlan_OI
	 *
	 * @brief
	 * Object Implementer of Vlan MO.
	 *
	 *
	 * @sa Base_OI
	 */
	class Vlan_OI : public Base_OI
	{
 	 public:

		///  Constructor.
		Vlan_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName);

		///  Destructor.
		virtual ~Vlan_OI();

		virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

		virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

		virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

		virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);

		/** @brief	Struct of IMM Vlan MoC elements
		 */
		struct Vlan: public Base_OI::Operation
		{
			operation::vlanInfo info;

			bool completed;
			ImmAction action;

			Vlan() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};

		};

 	 private:

		void resetOperationTable(const ACS_APGCC_CcbId&);

		typedef std::multimap<ACS_APGCC_CcbId, Vlan> operationTable_t;

		/** @brief	m_vlanOperationTable
		 *
		 *  Map of pending Vlan Operations
		 *
		 */
		operationTable_t m_vlanOperationTable;

	};

} /* namespace imm */

#endif /* ITHADM_CAA_INC_IMM_VLAN_OI_H_ */
