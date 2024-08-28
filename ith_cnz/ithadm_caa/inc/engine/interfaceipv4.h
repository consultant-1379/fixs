/*
 * interfaceipv4.h
 *
 *  Created on: May 9, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_INTERFACEIPV4_H_
#define ITHADM_CAA_INC_ENGINE_INTERFACEIPV4_H_

#include "operation/operation.h"
#include "common/utility.h"
#include "engine/vlan.h"

#include "boost/shared_ptr.hpp"

namespace engine {

	class InterfaceIPv4 {
	public:
		InterfaceIPv4(const operation::interfaceInfo& data);

		InterfaceIPv4(const boost::shared_ptr<InterfaceIPv4>& rhs);

		virtual ~InterfaceIPv4();

		const char* getSmxId() const {return m_data.getSmxId();};

		const char* getName() const {return m_data.getName(); };

		const char* getDN() const {return m_data.getDN(); };

		const char* getRouterName() const {return m_routerName.c_str();};

		const bool isLoopback() const { return m_data.getLoopback(); };

		const bool hasEncapsulation() const { return strlen(m_data.getEncapsulation()) > 0; }

		const char* getEncapsulationDN() const {return m_data.getEncapsulation(); };

		const boost::shared_ptr<Vlan> getEncapsulation() const {return m_encapsulation;};

		const uint16_t getEncaspulationVid() const {return (NULL != m_encapsulation)? m_encapsulation->getVlanId(): imm::vlanId_range::UNDEFINED; };

		void setEncapsulation(const boost::shared_ptr<Vlan>& encapsulation);

		imm::oper_State getOperationalState() const {return m_data.getOperationalState(); };

		const operation::interfaceInfo& getData() const {return m_data;};

		uint16_t modify(const operation::interfaceInfo&);

		void setRuntimeInfo(const operation::interfaceInfo&);

		bool isBfdEnabled();

		bool isAclDefined() const;

		void setAclInfo();

		const char* getAclName() const { return m_data.aclData.getAclName(); };

		uint16_t getVlanId() const { return m_data.aclData.getVlanId(); };

	private:
		operation::interfaceInfo m_data;

		std::string m_routerName;

		boost::shared_ptr<Vlan> m_encapsulation;
	};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_INTERFACEIPV4_H_ */
