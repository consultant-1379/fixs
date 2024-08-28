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

#ifndef NETCONF_SESSION_H_
#define NETCONF_SESSION_H_

#include <boost/noncopyable.hpp>

#include <string>

class acs_nclib_session;


namespace netconf
{
	namespace command
	{
		enum CommandType
		{
			CMD_GETCONFIG        = 0,
			CMD_EDITCONFIG       = 1,
			CMD_GET              = 2,
			CMD_ACTION			 = 3
		};
	}

	enum OperationResult
	{
		FAILED  = -1,
		SUCCESS =	0,
		ERROR = 1
	};

	class RequestInterface;

	/**
	 * @class Session
	 *
	 * @brief
	 * This class provides a communication helper towards a SMX board via the netconf protocol
	 */
	class Session : private boost::noncopyable
	{
		public:

			static void initialize();

			static void shutdown();

			Session();

			Session(const char* serverAddress);

			Session(const char* serverAddress, int timeout);

			virtual ~Session();

			int performOperation(const std::string& xmlRequest, command::CommandType operation);

			inline void setRemoteAddress(const char * (& serverAddress)) { m_serverAddress = serverAddress; };

			inline std::string getRemoteAddress() const { return m_serverAddress; };

			inline void setTimeout(const int& timeout) { m_timeout = timeout; };

			inline int getTimeout() const { return m_timeout; };

			inline std::string getXmlAnswer() const { return m_xmlAnswer; };

			inline std::string getErrorMessage() const { return m_errorMessage; };

			inline std::string getErrorTag() const { return m_errorTag; };

			inline int  get_error_code() const { return m_error_code; };

		private:

			int open();

			int close();

			RequestInterface* createNetconfRequest(const std::string& xmlRequest, command::CommandType operation);

			std::string m_serverAddress;

			int m_timeout;

			int m_error_code;

			std::string m_xmlAnswer;

			std::string m_errorMessage;

			std::string m_errorTag;

			acs_nclib_session* m_session;

	};
}
#endif /* NETCONF_SESSION_H_ */
