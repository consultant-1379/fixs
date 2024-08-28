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

#ifndef NETCONF_EDITCONFIGWITHCOMMIT_H_
#define NETCONF_EDITCONFIGWITHCOMMIT_H_

#include "netconf/requestbase.h"

namespace netconf
{

	class EditConfigWithCommit: public RequestInterface
	{
		public:

			EditConfigWithCommit(RequestInterface* requestBase) : m_request(requestBase) { };

			virtual ~EditConfigWithCommit() { delete m_request;};

			virtual int handleRequest(acs_nclib_session* session, const int timeout);

		private:

			int commit(acs_nclib_session* session, const int timeout);

			int receiveCommitAnswer(acs_nclib_session* session, const int timeout);

			void handleCommitError(acs_nclib_rpc_reply* answer);

			RequestInterface* m_request;
	};

} /* namespace netconf */

#endif /* NETCONF_EDITCONFIGWITHCOMMIT_H_ */
