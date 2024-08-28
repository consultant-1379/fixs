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
 *  Created on: Jul 1, 2016
 *      Author: xvincon
 */

#ifndef BFDSESSION_OI_H_
#define BFDSESSION_OI_H_

#include "imm/base_oi.h"

namespace imm
{

	class BfdSession_OI: public Base_OI
	{
			friend class ConfigurationHelper;

		public:

			BfdSession_OI();

			virtual ~BfdSession_OI();

			virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

			virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

			virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual ACS_CC_ReturnType updateRuntime(const char* p_objName, const char** p_attrName);

		private:

			struct BfdSession : public Base_OI::Operation
			{
				operation::bfdSessionInfo info;

				bool completed;
				ImmAction action;

				BfdSession() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};
			};

			void updateAttribute(const char* objectDN, const char* attributeName, const char* attributeValue);

			void updateAttribute(const char* objectDN, const char* attributeName, uint32_t attributeValue);

			void updateAttribute(const char* objectDN, const char* attributeName, int32_t attributeValue);

			void resetOperationTable(const ACS_APGCC_CcbId&);

			/** @brief	Struct of IMM MoC elements
			 */
			typedef std::multimap<ACS_APGCC_CcbId, BfdSession> operationTable_t;

			/** @brief	m_bfdSessionOperationTable
			 *
			 *  Map of pending BFD_Session Operations
			 *
			 */
			operationTable_t m_bfdSessionOperationTable;
	};

} /* namespace imm */

#endif /* BFDSESSION_OI_H_ */
