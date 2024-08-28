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

#include "netconf/requestbase.h"
#include "netconf/session.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

#include "acs_nclib_factory.h"
#include "acs_nclib_library.h"
#include "acs_nclib_session.h"

#include <sstream>

namespace netconf
{

	RequestBase::~RequestBase()
	{
		acs_nclib_factory::dereference(m_rpcRequest);
	}

	int RequestBase::handleRequest(acs_nclib_session* session, const int timeout)
	{
		int result = netconf::FAILED;

		if(m_rpcRequest)
		{
			int sendResult = session->send(m_rpcRequest);
			if( acs_nclib::ERR_NO_ERRORS == sendResult )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NETCONF REQUEST:<%u> SENT", m_rpcRequest->get_message_id());
				result = receiveAnswer(session, timeout);
			}
			else
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to send request:<%d> Id:<%d>, returned error:<%d>, last error<%d, %s>",
						m_rpcRequest->get_operation(),	m_rpcRequest->get_message_id(), sendResult, session->last_error_code(), session->last_error_text());
			}
		}

		return result;
	}

	int RequestBase::receiveAnswer(acs_nclib_session* session, const int timeout)
	{
		int result = netconf::FAILED;

		acs_nclib_message* answer = 0;

		int receiveResult = session->receive(answer, timeout);

		if(acs_nclib::ERR_NO_ERRORS == receiveResult)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Received answerId:<%u> for requestId:<%u>",
					answer->get_message_id(), m_rpcRequest->get_message_id());

			if( answer->get_message_id() == m_rpcRequest->get_message_id() )
			{
				std::string netconfRawAnswer;
				answer->dump(netconfRawAnswer);

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NETCONF FULL XML ANSWER:< %s > RECIEVED for REQUEST:<%u>", netconfRawAnswer.c_str(), m_rpcRequest->get_message_id());
				result = setXmlAnswer(answer);
			}
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to receive answer for requestId:<%d>, returned error:<%d>, last error<%d, %s>",
					m_rpcRequest->get_message_id(), receiveResult, session->last_error_code(), session->last_error_text());
		}

		acs_nclib_factory::dereference(answer);

		return result;
	}

	int RequestBase::setXmlAnswer(acs_nclib_message* xmlAnswer)
	{
		int result = netconf::FAILED;

		acs_nclib_rpc_reply* reply = dynamic_cast<acs_nclib_rpc_reply*>(xmlAnswer);

		if(reply)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NETCONF ANSWER TYPE:<%u>", reply->get_reply_type());

			if( (reply->get_reply_type() == acs_nclib::REPLY_DATA) ||
					(reply->get_reply_type() == acs_nclib::REPLY_OK) )
			{
				result = netconf::SUCCESS;
				m_xmlAnswer.assign(reply->get_data());
			}
			else
			{
				result = netconf::ERROR;
				handleAnswerError(reply);
			}
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to cast acs_nclib_message object to acs_nclib_rpc_reply object");
		}

		return result;
	}

	void RequestBase::handleAnswerError(acs_nclib_rpc_reply* answer)
	{
		if( (answer->get_reply_type() == acs_nclib::REPLY_DATA_WITH_ERRORS) || (answer->get_reply_type() == acs_nclib::REPLY_ERROR) )
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

			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Received error answer for requestId:<%u>, %s",
					answer->get_message_id(), errorMsg.str().c_str() );
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Received answer type:<%d> for requestId:<%u>",
					answer->get_message_id(), answer->get_reply_type());
		}
	}

} /* namespace netconf */
