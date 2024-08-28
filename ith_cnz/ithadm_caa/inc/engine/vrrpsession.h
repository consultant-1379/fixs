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
 *  Created on: Jun 15, 2016
 *      Author: xvincon
 */

#ifndef ITHADM_CAA_INC_ENGINE_VRRPSESSION_H_
#define ITHADM_CAA_INC_ENGINE_VRRPSESSION_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

#include <string>

namespace engine
{
	class VrrpInterface;

	class VrrpSession
	{
		public:

			VrrpSession(const operation::vrrpSessionInfo& data);

			VrrpSession(const boost::shared_ptr<VrrpSession>& rhs);

			virtual ~VrrpSession();

			uint16_t modify(const operation::vrrpSessionInfo& newData);

			void setVrrpInterfaceInfo(const operation::vrrpInterfaceInfo& data);

			void resetChangeMask();

			const char* getSmxId() const {return m_data.getSmxId();};

			const char* getName() const {return m_data.getName(); };

			const char* getDN() const {return m_data.getDN(); };

			const char* getRouterName() const {return m_routerName.c_str();};

			const char* getInterfaceName() const {return m_interfaceName.c_str();};

			const char* getVrrpInterfaceName() const {return m_vrrpInterfaceName.c_str();};

			const operation::vrrpSessionInfo getData() const {return m_data; };

		private:

			operation::vrrpSessionInfo m_data;

			std::string m_routerName;

			std::string m_interfaceName;

			std::string m_vrrpInterfaceName;

	};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_VRRPSESSION_H_ */
