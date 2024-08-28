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

#include "netconf/editconfig.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

#include "acs_nclib_factory.h"
#include "acs_nclib_session.h"

namespace netconf
{

	EditConfig::EditConfig(const std::string& xmlData)
	: RequestBase()
	{
		m_rpcRequest = acs_nclib_factory::create_rpc(acs_nclib::OP_EDITCONFIG, acs_nclib::DATASTORE_RUNNING);

		if(m_rpcRequest)
		{
			m_rpcRequest->set_config(xmlData.c_str());
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to create EditConfig request");
		}
	}


} /* namespace netconf */
