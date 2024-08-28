//	********************************************************
//
//	 COPYRIGHT Ericsson 2016
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2016.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2016 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************
#include "netconf/session.h"
#include "netconf/editconfig.h"
#include "netconf/editconfigwithcommit.h"
#include "netconf/getconfig.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

#include "acs_nclib_session.h"
#include "acs_nclib_rpc.h"
#include "acs_nclib_library.h"

#include <sstream>

namespace netconf
{
	namespace
	{
		const int DEFAULT_TIMEOUT_MS = 5000;

		const int DEFAULT_SMX_PORT = 22;
		const char DEFAULT_SMX_USER[] = "advanced";
		const char DEFAULT_SMX_PWD[]  = "ett,30";

	};

	void Session::initialize()
	{
		// init libssh2 for ssh layer
		acs_nclib_library::init();
	}

	void Session::shutdown()
	{
		acs_nclib_library::exit();
	}


	Session::Session()
	: m_serverAddress(),
	  m_timeout(DEFAULT_TIMEOUT_MS),
	  m_error_code(0),
	  m_xmlAnswer(),
	  m_errorMessage(),
	  m_errorTag(),
	  m_session(0)
	{


	}

	Session::Session(const char* serverAddress)
	: m_serverAddress(serverAddress),
	  m_timeout(DEFAULT_TIMEOUT_MS),
	  m_error_code(0),
	  m_xmlAnswer(),
	  m_errorMessage(),
	  m_errorTag(),
	  m_session(0)
	{


	}

	Session::Session(const char* serverAddress, int timeout)
	: m_serverAddress(serverAddress),
	  m_timeout(timeout),
	  m_error_code(0),
	  m_xmlAnswer(),
	  m_errorMessage(),
	  m_errorTag(),
	  m_session(0)
	{


	}

	Session::~Session()
	{
		delete m_session;
	}

	int Session::performOperation(const std::string& xmlRequest, command::CommandType operation)
	{
		m_xmlAnswer.clear();
		m_errorMessage.clear();
		m_errorTag.clear();
		m_error_code = 0;

		int result = open();

		if( netconf::SUCCESS == result )
		{
			result = netconf::FAILED;

			RequestInterface* operationRequest = createNetconfRequest(xmlRequest, operation);

			if(operationRequest)
			{
				result = operationRequest->handleRequest(m_session, m_timeout);

				if(( netconf::SUCCESS == result ))
				{
					operationRequest->getAnswer(m_xmlAnswer);
				}
				else
				{
					operationRequest->getErrorMessage(m_errorMessage);
					operationRequest->getErrorTag(m_errorTag);
				}

				delete operationRequest;
			}

			close();
		}

		return result;
	}

	int Session::open()
	{
		int result = netconf::SUCCESS;

		if( 0 != m_session) close();

		m_session = new (std::nothrow) acs_nclib_session(acs_nclib::SSH);

		if(m_session)
		{
			m_session->setPassword(DEFAULT_SMX_PWD);

			int openResult = m_session->open(m_serverAddress.c_str(), DEFAULT_SMX_PORT, acs_nclib::USER_AUTH_PASSWORD, DEFAULT_SMX_USER);

			if(acs_nclib::ERR_NO_ERRORS !=  openResult)
			{
				result = netconf::FAILED;
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to open Netconf Session, error: <%i, %s>", m_session->last_error_code(), m_session->last_error_text() );

				m_error_code= m_session->last_error_code();

				delete m_session;
				m_session = 0;
			}
		}
		else
		{
			result = netconf::FAILED;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to create Session Object");
		}

		return result;
	}

	int Session::close()
	{
		int result = netconf::SUCCESS;

		if( 0 != m_session)
		{
			result = m_session->close();

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Close netconf session return:<%d>", result);

			delete m_session;
			m_session = 0;
		}

		return result;
	}

	RequestInterface* Session::createNetconfRequest(const std::string& xmlRequest, command::CommandType operation)
	{
		RequestInterface* request = 0;

		switch(operation)
		{
			case command::CMD_GETCONFIG:
			{
				request = new (std::nothrow) GetConfig(acs_nclib::OP_GETCONFIG, xmlRequest);
			}
			break;

			case command::CMD_GET:
			{
				request = new (std::nothrow) GetConfig(acs_nclib::OP_GET, xmlRequest);
			}
			break;

			case command::CMD_EDITCONFIG:
			{
				request = new (std::nothrow) EditConfigWithCommit(new (std::nothrow) EditConfig(xmlRequest));
			}
			break;

			default:
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Un-Know Operation Request:<%i>", operation);
			}
		}

		if(!request)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to create operation:<%i> request", operation);
		}

		return request;
	}
}
