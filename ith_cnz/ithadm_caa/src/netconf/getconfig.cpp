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
 *  Created on: Jun 1, 2016
 *      Author: xvincon
 */

#include "netconf/getconfig.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

#include "acs_nclib_factory.h"


namespace netconf
{

	GetConfig::GetConfig(acs_nclib::Operation getType, const std::string& xmlFilter)
	: RequestBase()
	{
		m_rpcRequest = acs_nclib_factory::create_rpc(getType);

		if( m_rpcRequest  )
		{
			acs_nclib_filter* filter = acs_nclib_factory::create_filter(acs_nclib::FILTER_SUBTREE);

			if(filter)
			{
				filter->set_filter_content(xmlFilter.c_str());
				m_rpcRequest->add_filter(filter);

				// filter object can be delete
				acs_nclib_factory::dereference(filter);
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to create filter for operation:<%i>", getType);

				acs_nclib_factory::dereference(m_rpcRequest);
				m_rpcRequest = 0;
			}
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to create RPC GetConfig request");
		}
	}


} /* namespace netconf */
