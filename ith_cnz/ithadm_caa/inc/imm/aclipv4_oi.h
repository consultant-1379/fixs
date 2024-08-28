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
 *  Created on: Oct 18, 2016
 *      Author: xvincon
 */

#ifndef ACLIPV4_OI_H_
#define ACLIPV4_OI_H_

#include "imm/base_oi.h"

namespace imm
{

	class AclIpv4_OI: public Base_OI
	{
		public:

			AclIpv4_OI();

			virtual ~AclIpv4_OI();

			virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

			virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

			virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

			virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			struct AclIpv4 : public Base_OI::Operation
			{
				operation::aclIpV4Info info;

				bool completed;
				ImmAction action;

				AclIpv4() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};
			};

		private:

			void resetOperationTable(const ACS_APGCC_CcbId&);

			/** @brief	Struct of IMM MoC elements
			 */
			typedef std::multimap<ACS_APGCC_CcbId, AclIpv4> operationTable_t;

			/** @brief	m_aclIpv4OperationTable
			 *
			 *  Map of pending AclIpV4 Operations
			 *
			 */
			operationTable_t m_aclIpv4OperationTable;
	};

} /* namespace imm */

#endif /* ACLIPV4_OI_H_ */
