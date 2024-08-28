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

#include "netconf/editconfigwithcommit.h"
#include "netconf/session.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

#include "acs_nclib_factory.h"
#include "acs_nclib_session.h"

#include <sstream>

namespace netconf
{

	int EditConfigWithCommit::handleRequest(acs_nclib_session* session, const int timeout)
	{
		int result = netconf::FAILED;

		if(m_request)
		{
			result = m_request->handleRequest(session, timeout);

			if( netconf::SUCCESS == result)
			{
				result = commit(session, timeout);
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Edit config failed before commit!");
				m_request->getErrorMessage(m_errorMessage);
				m_request->getErrorTag(m_errorTag);
			}
		}

		return result;
	}

	int EditConfigWithCommit::commit(acs_nclib_session* session, const int timeout)
	{
		int result = netconf::FAILED;

		acs_nclib_rpc* closeSessionRequest = acs_nclib_factory::create_rpc(acs_nclib::OP_CLOSESESSION);

		if(closeSessionRequest)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Send close session request to commit");

			int sentResult = session->send(closeSessionRequest);

			if(acs_nclib::ERR_NO_ERRORS == sentResult)
			{
				result = receiveCommitAnswer(session, timeout);
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to send close session request, returned error:<%d>, last error<%d, %s>",
						sentResult, session->last_error_code(), session->last_error_text());
			}

			acs_nclib_factory::dereference(closeSessionRequest);
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to create rpc close session request");
		}

		return result;
	}

	int EditConfigWithCommit::receiveCommitAnswer(acs_nclib_session* session, const int timeout)
	{
		int result = netconf::FAILED;

		acs_nclib_message* answer = 0;

		int recv_result = session->receive(answer, timeout);

		if(recv_result == acs_nclib::ERR_NO_ERRORS && answer->get_msg_type() == acs_nclib::RPC_REPLY_MSG)
		{
			acs_nclib_rpc_reply* reply = dynamic_cast<acs_nclib_rpc_reply*>(answer);

			if(reply && reply->get_reply_type() == acs_nclib::REPLY_OK)
			{
				result = netconf::SUCCESS;
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request committed");
			}
			else
			{
				result = netconf::ERROR;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Error on commit");
				handleCommitError(reply);
			}
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to receive answer on commit request, recv_result = <%d>", recv_result);
		}

		acs_nclib_factory::dereference(answer);

		return result;
	}

	void EditConfigWithCommit::handleCommitError(acs_nclib_rpc_reply* answer)
	{
		if( answer &&
				( (answer->get_reply_type() == acs_nclib::REPLY_DATA_WITH_ERRORS) || (answer->get_reply_type() == acs_nclib::REPLY_ERROR)) )
		{
			std::vector<acs_nclib_rpc_error*> errors;
			std::stringstream errorMsg;
			std::vector<acs_nclib_rpc_error*>::const_iterator errorIt;

			answer->get_error_list(errors);

			for(errorIt = errors.begin(); errorIt != errors.end(); ++errorIt)
			{
				errorMsg << "Error: [ Type:" << (*errorIt)->get_error_type() << " , Tag: " << (*errorIt)->get_error_tag() << " , Msg: " << (*errorIt)->get_error_message() << " ]\n";
				m_errorMessage.append((*errorIt)->get_error_message());
				m_errorTag.append((*errorIt)->get_error_tag());
			}

			acs_nclib_factory::dereference(errors);

			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Request Id:<%u> commit failed, %s",
					answer->get_message_id(), errorMsg.str().c_str() );
		}
	}


} /* namespace netconf */
