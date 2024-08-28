/*
 * address.h
 *
 *  Created on: May 17, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_ADDRESS_H_
#define ITHADM_CAA_INC_ENGINE_ADDRESS_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine {

	class Address
	{
		public:
			Address(const operation::addressInfo& data);

			Address(const boost::shared_ptr<Address>& rhs);

			virtual ~Address();

			const char* getSmxId() const {return m_data.getSmxId();};

			const char* getName() const {return m_data.getName(); };

			const char* getAddress() const {return m_data.getAddress(); };

			const char* getRouterName() const {return m_routerName.c_str(); };

			const char* getInterfaceName() const {return m_interfaceName.c_str(); };

			const bool isVRRPInterfaceAddress() const {return m_data.isVRRPInterfaceAddress(); };

			const operation::addressInfo& getData() const {return m_data; };

			uint16_t modify(const operation::addressInfo&);

		private:
			operation::addressInfo m_data;

			std::string m_routerName;

			std::string m_interfaceName;

	};

} /* namespace engine */



#endif /* ITHADM_CAA_INC_ENGINE_ADDRESS_H_ */
