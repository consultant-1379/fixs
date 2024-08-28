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

#ifndef NETCONF_REQUESTBASE_H_
#define NETCONF_REQUESTBASE_H_

#include "acs_nclib_rpc.h"

class acs_nclib_session;
class acs_nclib_rpc;
class acs_nclib_message;
class acs_nclib_rpc_reply;

namespace netconf
{

	class RequestInterface
	{
		public:

			RequestInterface() {};

			virtual ~RequestInterface() {};

			virtual int handleRequest(acs_nclib_session* session, const int timeout) = 0;

			virtual void getAnswer(std::string& answer) { answer.assign(m_xmlAnswer); };

			virtual void getErrorMessage(std::string& errorMessage) { errorMessage.assign(m_errorMessage); };

			virtual void getErrorTag(std::string& errorTag) { errorTag.assign(m_errorTag); };

		protected:

			std::string m_xmlAnswer;

			std::string m_errorMessage;

			std::string m_errorTag;

	};

	class RequestBase : public RequestInterface
	{
		public:

			RequestBase() : m_rpcRequest(0) {};

			virtual ~RequestBase();

			virtual int handleRequest(acs_nclib_session* session, const int timeout);

		protected:

			int receiveAnswer(acs_nclib_session* session, const int timeout);

			int setXmlAnswer(acs_nclib_message* answer);

			void handleAnswerError(acs_nclib_rpc_reply* answer);

			acs_nclib_rpc* m_rpcRequest;

	};



} /* namespace netconf */

#endif /* NETCONF_REQUESTBASE_H_ */
