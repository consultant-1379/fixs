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
 *  Created on: Jul 4, 2016
 *      Author: xvincon
 */

#include "operation/remove_bfd_sessions.h"
#include "engine/context.h"

#include "fixs_ith_programconstants.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

#include <acs_apgcc_omhandler.h>

#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace operation
{
	const int IMM_OBJECT_NOT_EXIST = -12;

	Remove_BFD_Sessions::Remove_BFD_Sessions()
	: OperationBase(REMOVE_BFD_SESSIONS),
	  m_routerDN(),
	  m_routerName()
	{

	}

	Remove_BFD_Sessions::~Remove_BFD_Sessions()
	{

	}

	void Remove_BFD_Sessions::setOperationDetails(const void* op_details)
	{
		std::string objectDN(reinterpret_cast<const char*>(op_details));
		common::utility::getRouterDn(objectDN, m_routerDN);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Schedule BFD sessions removing for Router DN<%s>", m_routerDN.c_str());
	}

	int Remove_BFD_Sessions::call()
	{
		int operationResult = fixs_ith::ERR_NO_ERRORS;

		std::string smxId(common::utility::getSmxIdFromRouterDN(m_routerDN));
		m_routerName.assign(common::utility::getIdValueFromRdn(m_routerDN));

		engine::contextAccess_t access(smxId, engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = access.getContext();

		if(context)
		{
			std::set<std::string> bfdSessions = context->getAddressOfBfdSessions(m_routerName);
			std::set<std::string> monitoredInterface = context->getAddressOfInterfaceWithBFD(m_routerName);
			std::set<std::string> bfdSessionToRemove;

			searchBfdSessionsToRemove(bfdSessions, monitoredInterface, bfdSessionToRemove);

			operationResult = removeFromIMM_BFD_Sessions(bfdSessionToRemove);
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Context for SMX Id:<%s>, Router DN:<%s>", smxId.c_str(), m_routerDN.c_str());
		}

		// set result to caller
		m_operationResult.setErrorCode(operationResult);
		setResultToCaller();

		return fixs_ith::ERR_NO_ERRORS;
	}

	void Remove_BFD_Sessions::searchBfdSessionsToRemove(const std::set<std::string>& bfdSessions, const std::set<std::string>& interfaceSubnets, std::set<std::string>& bfdSessionToRemove)
	{
		std::set<std::string>::const_iterator bfdElement;

		for( bfdElement = bfdSessions.begin(); bfdSessions.end() != bfdElement; ++bfdElement)
		{
			if( !matchWithInterfaceSubnet(*bfdElement, interfaceSubnets) )
			{
				bfdSessionToRemove.insert(*bfdElement);
			}
		}
	}

	bool Remove_BFD_Sessions::matchWithInterfaceSubnet(const std::string& bfdAddress, const std::set<std::string>& interfaceSubnets)
	{
		bool ipMatch = false;

		const uint32_t bfdSessionAddress = inet_network(bfdAddress.c_str());

		std::set<std::string>::const_iterator interfaceAddressIterator;

		for(interfaceAddressIterator = interfaceSubnets.begin(); interfaceSubnets.end() != interfaceAddressIterator; ++interfaceAddressIterator)
		{
			// split cidr notation to ip and network prefix
			std::string ipAddress(interfaceAddressIterator->substr(0, interfaceAddressIterator->find(common::utility::parserTag::slash)));
			std::stringstream ipNetworkPrefix(interfaceAddressIterator->substr(interfaceAddressIterator->find(common::utility::parserTag::slash) +1U) );

			uint32_t numberOfBits = 0U;
			ipNetworkPrefix >> numberOfBits;
			const uint32_t subnetMask = (~uint32_t(0) << ( common::MAX_NETWORK_PREFIX - numberOfBits));

			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Address:<%s> Network Prefix:<%u> on Router:<%s>",
					ipAddress.c_str(), numberOfBits, m_routerName.c_str());

			const uint32_t interfaceAddress = inet_network(ipAddress.c_str());

			if( (bfdSessionAddress & subnetMask) == (interfaceAddress & subnetMask) )
			{
				ipMatch = true;
				break;
			}
		}

		return ipMatch;
	}

	int Remove_BFD_Sessions::removeFromIMM_BFD_Sessions(const std::set<std::string>& ipAddresses)
	{
		int result = fixs_ith::ERR_NO_ERRORS;

		OmHandler om_handler;

		if(om_handler.Init() != ACS_CC_SUCCESS)
		{
			// ERROR initializing imm om handler
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Internal OM IMM handler init failure: imm last error:<%d>, imm last error text:<%s>",
					om_handler.getInternalLastError(), om_handler.getInternalLastErrorText());
			return fixs_ith::ERR_OM_HANDLER_INIT_FAILURE;
		}

		std::set<std::string>::const_iterator element;

		for(element = ipAddresses.begin(); ipAddresses.end() != element; ++element)
		{
			char bfdSessionDN[1024] = {0};

			snprintf(bfdSessionDN, sizeof(bfdSessionDN)-1, "%s=%s,%s",
					imm::bfdSession_attribute::RDN.c_str(), element->c_str(), m_routerDN.c_str() );

			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Deleting BFD Session:<%s> on Router:<%s>", bfdSessionDN, m_routerName.c_str());

			if( om_handler.deleteObject(bfdSessionDN) == ACS_CC_FAILURE )
			{
				int immErrorCode = om_handler.getInternalLastError();

				if( IMM_OBJECT_NOT_EXIST == immErrorCode )
				{
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BFD Session:<%s> on ROUTER:<%s> NOT EXIST",
							bfdSessionDN, m_routerName.c_str());
				}
				else
				{
					result = fixs_ith::ERR_IMM_DELETE_OBJ;
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Delete of BFD Session:<%s> on ROUTER:<%s> FAILED, error_code:<%d> errorText:<%s>",
							bfdSessionDN, m_routerName.c_str(), immErrorCode, om_handler.getInternalLastErrorText());
				}
			}
		}

		if(om_handler.Finalize() != ACS_CC_SUCCESS)
		{
			// ERROR finalizing imm internal om handler
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Internal OM IMM handler finalize failure: imm last error:<%d>, imm last error text:<%s>",
					om_handler.getInternalLastError(), om_handler.getInternalLastErrorText());
		}

		return result;
	}
} /* namespace operation */
