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
 *  Created on: Jul 6, 2016
 *      Author: xvincon
 */

#include "operation/add_bfd_session_for_nexthop.h"
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
	const int IMM_OBJECT_ALREADY_EXIST = -14;

	Add_BFD_Session_For_NextHop::Add_BFD_Session_For_NextHop()
	: OperationBase(ADD_BFD_SESSION_FOR_NEXTHOP),
	  m_nextHopDN(),
	  m_routerName()
	{

	}

	Add_BFD_Session_For_NextHop::~Add_BFD_Session_For_NextHop()
	{

	}

	void Add_BFD_Session_For_NextHop::setOperationDetails(const void* op_details)
	{
		m_nextHopDN.assign(reinterpret_cast<const char*>(op_details));

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Schedule BFD session adding for NextHop DN<%s>", m_nextHopDN.c_str());
	}

	int Add_BFD_Session_For_NextHop::call()
	{
		int operationResult = fixs_ith::ERR_NO_ERRORS;

		std::string smxId(common::utility::getSmxIdFromNextHopDN(m_nextHopDN));

		m_routerName.assign(common::utility::getRouterNameFromNextHopDN(m_nextHopDN));
		m_nextHopName.assign(common::utility::getNextHopNameFromDN(m_nextHopDN));

		engine::contextAccess_t access(smxId, engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = access.getContext();

		if(context)
		{
			// get nexthop address IPv4
			std::string destName(  common::utility::getDstNameFromNextHopDN(m_nextHopDN) );
			std::string nextHopAddress = context->getNextHopAddress(m_routerName, destName, m_nextHopName);

			// get all bfd session address already created
			std::set<std::string> bfdSessions = context->getAddressOfBfdSessions(m_routerName);

			// check that the bfd session is not already created
			if( bfdSessions.find(nextHopAddress) == bfdSessions.end() )
			{
				// bfd session not found check if the nexthop address is monitored
				std::set<std::string> monitoredInterface = context->getAddressOfInterfaceWithBFD(m_routerName);

				if( isNextHopAddressMonitored(nextHopAddress, monitoredInterface) )
				{
					// create the bfd session for this nexthop address
					operationResult = addToIMM_BFD_Session(nextHopAddress);
				}
			}
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Context for SMX Id:<%s>, NextHop DN:<%s>", smxId.c_str(), m_nextHopDN.c_str());
		}

		// set result to caller
		m_operationResult.setErrorCode(operationResult);
		setResultToCaller();

		return fixs_ith::ERR_NO_ERRORS;
	}

	bool Add_BFD_Session_For_NextHop::isNextHopAddressMonitored(const std::string& address, const std::set<std::string>& interfaceSubnets)
	{
		bool ipMatch = false;

		const uint32_t nextHopAddress = inet_network(address.c_str());

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

			if( (nextHopAddress & subnetMask) == (interfaceAddress & subnetMask) )
			{
				ipMatch = true;
				FIXS_ITH_LOG(LOG_LEVEL_INFO, "Address:<%s> monitored by Interface with subnet:<%s> on Router:<%s>",
						address.c_str(),  interfaceAddressIterator->c_str(), m_routerName.c_str());

				break;
			}
		}

		return ipMatch;
	}

	int Add_BFD_Session_For_NextHop::addToIMM_BFD_Session(const std::string& address)
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

		std::string routerDN;
		common::utility::getRouterDn(m_nextHopDN, routerDN);

		std::vector<ACS_CC_ValuesDefinitionType> attrList;
		ACS_CC_ValuesDefinitionType attrRdn;

		char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
		strncpy(attrdn, imm::bfdSession_attribute::RDN.c_str(), IMM_ATTR_NAME_MAX_SIZE-1) ;
		attrRdn.attrName = attrdn;
		attrRdn.attrType = ATTR_STRINGT;
		attrRdn.attrValuesNum = 1;

		char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
		snprintf(rdnValue, IMM_ATTR_NAME_MAX_SIZE, "%s=%s", imm::bfdSession_attribute::RDN.c_str(), address.c_str()) ;
		void *attrRdnValue[1] = {reinterpret_cast<void*>(rdnValue)};
		attrRdn.attrValues = attrRdnValue;

		attrList.push_back(attrRdn);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating BFD Session:<%s> on Router:<%s>", rdnValue, routerDN.c_str());

		if( om_handler.createObject( imm::moc_name::CLASS_BFD_SESSION.c_str(), routerDN.c_str(), attrList ) == ACS_CC_FAILURE )
		{
			int immErrorCode = om_handler.getInternalLastError();

			if( IMM_OBJECT_ALREADY_EXIST == immErrorCode )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BFD Session:<%s> on ROUTER:<%s> ALREADY EXIST",
						rdnValue, routerDN.c_str());
			}
			else
			{
				result = fixs_ith::ERR_IMM_CREATE_OBJ;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Create of BFD Session:<%s> on ROUTER:<%s> FAILED, error_code:<%d> errorText:<%s>",
						rdnValue, routerDN.c_str(), immErrorCode, om_handler.getInternalLastErrorText());
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
