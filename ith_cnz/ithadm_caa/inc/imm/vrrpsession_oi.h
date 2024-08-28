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
 *  Created on: Jun 10, 2016
 *      Author: xvincon
 */

#ifndef IMM_VRRPSESSION_OI_H_
#define IMM_VRRPSESSION_OI_H_

#include "imm/base_oi.h"

namespace imm
{

	class VrrpSession_OI: public Base_OI
	{
			friend class ConfigurationHelper;

		public:

			VrrpSession_OI();

			virtual ~VrrpSession_OI();

			virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

			virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

			virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

			virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

			virtual ACS_CC_ReturnType updateRuntime(const char* p_objName, const char** p_attrName);

		private:

			struct VrrpSession: public Base_OI::Operation
			{
				operation::vrrpSessionInfo info;

				bool completed;
				ImmAction action;

				VrrpSession() : Base_OI::Operation(), info(), completed(false), action(Nothing) {};

			};

			bool checkTransactionAugmentation(VrrpSession& vrrpSessionObj, ACS_APGCC_OiHandle& oiHandle, ACS_APGCC_CcbId& ccbId);

			bool getVrrpSessionsDN(const VrrpSession& vrrpSessionObj, std::set<std::string>& vrrpSessionsDN);

			bool checkAndAddVrrpSessionDN(const std::string& vrrpSessionDN, std::set<std::string>& vrrpSessionsDN);

			bool calculateVrrpSessionPriority(VrrpSession& vrrpSessionObj);

			bool compareIPv4Address(const std::string& interfaceAddress, const std::string& vrrpAddress);

			ACS_CC_ReturnType modifyReservedByAttribute(ACS_APGCC_CcbHandle ccbHandle, ACS_APGCC_AdminOwnerHandle adminOwnerHandler, const char* vrrpInterfaceDN, const std::set<std::string>& vrrpSessionsDN);

			ACS_CC_ReturnType modifyPriorityAttribute(ACS_APGCC_CcbHandle ccbHandle, ACS_APGCC_AdminOwnerHandle adminOwnerHandler, const char* vrrpSessionDN, uint32_t priorityValue);

			void resetOperationTable(const ACS_APGCC_CcbId&);

			/** @brief	Struct of IMM MoC elements
			 */
			typedef std::multimap<ACS_APGCC_CcbId, VrrpSession> operationTable_t;

			/** @brief	m_vrrpInterfaceOperationTable
			 *
			 *  Map of pending VrrpInterface Operations
			 *
			 */
			operationTable_t m_vrrpSessionOperationTable;

	};

} /* namespace imm */

#endif /* IMM_VRRPSESSION_OI_H_ */
