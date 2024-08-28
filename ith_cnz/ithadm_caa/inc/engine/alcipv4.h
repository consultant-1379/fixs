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
 *  Created on: Oct 21, 2016
 *      Author: xvincon
 */

#ifndef ITHADM_CAA_INC_ENGINE_ALCIPV4_H_
#define ITHADM_CAA_INC_ENGINE_ALCIPV4_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine
{

	class AclIpv4
	{
		public:

			AclIpv4(const operation::aclIpV4Info& data);

			AclIpv4(const boost::shared_ptr<AclIpv4>& rhs);

			virtual ~AclIpv4();

			const char* getSmxId() const {return m_data.getSmxId();};

			const char* getName() const {return m_data.getName();};

			const char* getDN() const {return m_data.getDN();};

			const char* getRouterName() const {return m_routerName.c_str();};

			const operation::aclIpV4Info& getData() const {return m_data;};

			uint16_t modify(const operation::aclIpV4Info&);

		private:

			operation::aclIpV4Info m_data;

			std::string m_routerName;
	};

} /* namespace imm */

#endif /* ITHADM_CAA_INC_ENGINE_ALCIPV4_H_ */
