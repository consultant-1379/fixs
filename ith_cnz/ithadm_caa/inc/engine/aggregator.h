/*
 * aggregator.h
 *
 *  Created on: Mar 16, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_AGGREGATOR_H_
#define ITHADM_CAA_INC_ENGINE_AGGREGATOR_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine {

	class EthernetPort;

	class Aggregator {
	public:
		Aggregator(const operation::aggregatorInfo& data);

		Aggregator(const boost::shared_ptr<Aggregator>& rhs);

		virtual ~Aggregator();

		std::set<std::string> getAdminAggMemberDN() {return m_data.getAdminAggMember();};

		std::set<uint16_t> getAdminAggMemebrIndex();

		bool hasAdminAggMemberIndex(const uint16_t& index);

		void addEthernetPorts(std::set< boost::shared_ptr<EthernetPort> > ports) {m_ethernetPorts.insert(ports.begin(), ports.end());};

		void removeEthernetPorts(std::set< boost::shared_ptr<EthernetPort> > ports) {m_ethernetPorts.erase(ports.begin(), ports.end());};

		void setEthernetPorts(std::set< boost::shared_ptr<EthernetPort> > ports) {m_ethernetPorts = ports;};

		const fixs_ith::SwitchBoardPlaneConstants getPlane() const {return m_data.getPlane(); };

		const char* getSmxId() const {return m_data.getSmxId();};

		const char* getName() const {return m_data.getName(); };

		const uint16_t getSnmpIndex() const {return m_data.getSnmpIndex();};

		const imm::oper_State getOperState() const {return m_data.getOperState();};

		void setOperState(const imm::oper_State& state) {m_data.operState = state;};

		uint16_t modify(const operation::aggregatorInfo&);

		std::set<std::string> getOperAggMemberDN() {return m_data.getOperAggMember();};

		void setOperAggMember(const std::set<std::string> DNs);

		void setOperAggMember(const std::set<uint16_t> indexes);

		uint32_t getDataRate() const {return m_data.getDataRate();};

		void setDataRate(const uint32_t& dataRate) {m_data.dataRate = dataRate;};

	private:
		operation::aggregatorInfo m_data;

		std::set<boost::shared_ptr<EthernetPort> > m_ethernetPorts;
	};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_AGGREGATOR_H_ */
