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
 *  Created on: Jun 30, 2016
 *      Author: xvincon
 */

#include "operation/add_bfd_sessions.h"
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

	Add_BFD_Sessions::Add_BFD_Sessions()
	: OperationBase(ADD_BFD_SESSIONS),
	  m_interfaceIPv4DN(),
	  m_routerName()
	{

	}

	Add_BFD_Sessions::~Add_BFD_Sessions()
	{

	}

	void Add_BFD_Sessions::setOperationDetails(const void* op_details)
	{
		const char* interfaceDN = reinterpret_cast<const char*>(op_details);

		m_interfaceIPv4DN.assign(interfaceDN);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Schedule BFD sessions adding for Interface DN<%s>", m_interfaceIPv4DN.c_str());
	}

	int Add_BFD_Sessions::call()
	{
		int operationResult = fixs_ith::ERR_NO_ERRORS;

		std::string smxId(common::utility::getSmxIdFromInterfaceDN(m_interfaceIPv4DN));

		m_routerName.assign(common::utility::getRouterNameFromInterfaceDN(m_interfaceIPv4DN));

		engine::contextAccess_t access(smxId, engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = access.getContext();

		if(context)
		{
			std::string interfaceAddress;
			uint32_t subnetMask;

			if( getInterfaceIpAddress(context, interfaceAddress, subnetMask) )
			{
				const uint32_t interfaceMask = (inet_network(interfaceAddress.c_str()) & subnetMask);
				std::set<std::string> nextHopsAddress = context->getNextHopsAddressWithBFDEnabled(m_routerName);

				FIXS_ITH_LOG(LOG_LEVEL_INFO, "There are <%zu> nextHop addresses", nextHopsAddress.size());

				std::set<std::string>::const_iterator element;
				std::set<std::string> addressOnTheSameSubnet;

				for(element = nextHopsAddress.begin(); nextHopsAddress.end() != element; ++element)
				{
					const uint32_t nextHopAddressMask = (inet_network(element->c_str()) & subnetMask);

					if(interfaceMask ==  nextHopAddressMask)
					{
						//both address are in the same subnet
						addressOnTheSameSubnet.insert(*element);
						FIXS_ITH_LOG(LOG_LEVEL_INFO, "IP:<%s> on the same subnet of IP interface", element->c_str() );
					}
				}

				operationResult = create_BFD_Session(addressOnTheSameSubnet);
			}
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Context for SMX Id:<%s>, InterfaceIPv4 DN:<%s>", smxId.c_str(), m_interfaceIPv4DN.c_str());
		}

		// set result to caller
		m_operationResult.setErrorCode(operationResult);
		setResultToCaller();

		return fixs_ith::ERR_NO_ERRORS;
	}

	bool Add_BFD_Sessions::getInterfaceIpAddress(engine::Context* context, std::string& ipAddress, uint32_t& subnetMask)
	{
		bool result = true;

		std::string interfaceName(common::utility::getIdValueFromRdn(m_interfaceIPv4DN));

		boost::shared_ptr<engine::Address> interfaceAddress = context->getRouterInterfaceAddress(m_routerName, interfaceName);

		if(interfaceAddress)
		{
			std::string address(interfaceAddress->getAddress());

			ipAddress.assign(address.substr(0, address.find(common::utility::parserTag::slash)));
			std::stringstream ipNetworkPrefix(address.substr(address.find(common::utility::parserTag::slash) +1U) );
			uint32_t numberOfBits = 0U;
			ipNetworkPrefix >> numberOfBits;
			subnetMask = (~uint32_t(0) << ( common::MAX_NETWORK_PREFIX - numberOfBits));

			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Address:<%s> Network Prefix:<%u> for Interface:<%s> on Router:<%s>",
					 ipAddress.c_str(), numberOfBits, interfaceName.c_str(), m_routerName.c_str());
		}
		else
		{
			result = false;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Address not found for Interface:<%s> on Router:<%s>",
					interfaceName.c_str(), m_routerName.c_str());
		}

		return result;
	}

	int Add_BFD_Sessions::create_BFD_Session(const std::set<std::string>& ipAddresses)
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
		std::string routerDN;

		common::utility::getRouterDn(m_interfaceIPv4DN, routerDN);

		for(element = ipAddresses.begin(); ipAddresses.end() != element; ++element)
		{
			std::vector<ACS_CC_ValuesDefinitionType> attrList;
			ACS_CC_ValuesDefinitionType attrRdn;

			char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
			strncpy(attrdn, imm::bfdSession_attribute::RDN.c_str(), IMM_ATTR_NAME_MAX_SIZE-1) ;
			attrRdn.attrName = attrdn;
			attrRdn.attrType = ATTR_STRINGT;
			attrRdn.attrValuesNum = 1;

			char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
			snprintf(rdnValue, IMM_ATTR_NAME_MAX_SIZE-1, "%s=%s", imm::bfdSession_attribute::RDN.c_str(), element->c_str()) ;
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
