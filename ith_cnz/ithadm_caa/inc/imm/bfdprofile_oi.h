/*
 *
 * COPYRIGHT Ericsson 2016
 *	All rights reserved.
 *
 *	The Copyright to the computer program(s) herein
 *	is the property of Ericsson 2016.
 *	The program(s) may be used and/or copied only with
 *	the written permission from Ericsson 2016 or in
 *	accordance with the terms and conditions stipulated in
 *	the agreement/contract under which the program(s) have
 *	been supplied.
 *
 *
 *  Created on: Jun 24, 2016
 *      Author: xclaele
 */

#ifndef ITHADM_CAA_INC_IMM_BfdProfile_OI_H_
#define ITHADM_CAA_INC_IMM_BfdProfile_OI_H_

#include "imm/base_oi.h"
#include "fixs_ith_programmacros.h"

namespace imm {

	class BfdProfile_OI : public Base_OI {

	public:
		BfdProfile_OI();

		virtual ~BfdProfile_OI();

		virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

		virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

		virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

		virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

		struct BfdProfile: public Base_OI::Operation
		{
			operation::bfdProfileInfo info;

			bool completed;
			ImmAction action;

			BfdProfile() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};

		};

	private:

		void resetOperationTable(const ACS_APGCC_CcbId&);

		/** @brief	Struct of IMM MoC elements
		 */
		typedef std::multimap<ACS_APGCC_CcbId, BfdProfile> operationTable_t;

		/** @brief	m_bfdProfileOperationTable
		 *
		 *  Map of pending BfdProfile Operations
		 *
		 */
		operationTable_t m_bfdProfileOperationTable;

	};

} /* namespace imm */

#endif /* ITHADM_CAA_INC_IMM_BfdProfile_OI_H_ */
