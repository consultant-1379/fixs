/*
 * vrrpInterface.h
 *
 *  Created on: Jun 1, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_VRRPINTERFACE_H_
#define ITHADM_CAA_INC_ENGINE_VRRPINTERFACE_H_

#include "operation/operation.h"
#include "common/utility.h"

#include "boost/shared_ptr.hpp"

namespace engine {

class VrrpInterface {
	public:
		VrrpInterface(const operation::vrrpInterfaceInfo& data);
		VrrpInterface(const boost::shared_ptr<VrrpInterface>& rhs);

		virtual ~VrrpInterface();

		const char* getName() const {return m_data.getName(); };

		const char* getDN() const {return m_data.getDN(); };

		const operation::vrrpInterfaceInfo getData() const {return m_data; };

		uint16_t modify(const operation::vrrpInterfaceInfo&);

		void setAddressIPv4(const std::string& address) { m_data.address_ipv4 = address; };

	private:
		operation::vrrpInterfaceInfo m_data;

	};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_VRRPINTERFACE_H_ */
