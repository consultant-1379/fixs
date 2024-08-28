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
 *  Created on: Jul 1, 2016
 *      Author: xvincon
 */

#ifndef BFDSESSIONIPV4_H_
#define BFDSESSIONIPV4_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

#include <string>
#include <ctime>

namespace engine
{

	class BfdSessionIPv4
	{
		public:

			BfdSessionIPv4(const operation::bfdSessionInfo& data);

			BfdSessionIPv4(const boost::shared_ptr<BfdSessionIPv4>& rhs);

			virtual ~BfdSessionIPv4();

			const char* getSmxId() const {return m_data.getSmxId();};

			const char* getName() const {return m_data.getName(); };

			const char* getAddress() const {return m_data.getName(); };

			const char* getDN() const {return m_data.getDN(); };

			const char* getRouterName() const {return m_routerName.c_str();};

			const operation::bfdSessionInfo getData() const {return m_data; };

			void refreshAttribute(const operation::bfdSessionInfo& newData);

			bool isOutOfDate() const;


		private:

			operation::bfdSessionInfo m_data;

			std::string m_routerName;

			std::time_t m_lastUpdate;
	};

} /* namespace imm */

#endif /* BFDSESSIONIPV4_H_ */
