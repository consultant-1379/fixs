/*
 * bridgeport.h
 *
 *  Created on: Mar 15, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_BRIDGEPORT_H_
#define ITHADM_CAA_INC_ENGINE_BRIDGEPORT_H_

#include "operation/operation.h"
#include "boost/shared_ptr.hpp"



namespace engine {

class Aggregator;
class EthernetPort;

class BridgePort {
public:
	BridgePort(const operation::bridgePortInfo& data);

	BridgePort(const boost::shared_ptr<BridgePort>& rhs);

	virtual ~BridgePort();

	uint16_t getDefaultVlanId(){return m_data.getDefaultVlanId();};

	void setDefaultVlanId(const uint16_t& vlanId) {m_data.defaultVlanId = vlanId;};

	bool isAggregator() const { return m_aggregator != 0; };

	bool isEthernet() const { return m_ethernetPort != 0; };

	void setEthernetPort(boost::shared_ptr<EthernetPort>& ethernetPort) { m_ethernetPort = ethernetPort; m_aggregator.reset();};

	void setAggregator(boost::shared_ptr<Aggregator>& aggregator) { m_aggregator = aggregator; m_ethernetPort.reset(); };

	const fixs_ith::SwitchBoardPlaneConstants getPlane() const {return m_data.getPlane(); };

	const char* getSmxId() const {return m_data.getSmxId();};

	const char* getName() const {return m_data.getName(); };

	const char* getBridgePortDN() const {return m_data.getBridgePortDN();};

	const uint16_t getSnmpIndex() const;

	const uint16_t getChangeMask() const {return m_data.changeMask;};

	const imm::admin_State getAdminState() const {return m_data.getAdminState();};

	const imm::oper_State getOperState() const {return m_data.getOperState();};

	void setOperState(const imm::oper_State& state) {m_data.operState = state;};

	uint16_t modify(const operation::bridgePortInfo&);


private:
	operation::bridgePortInfo m_data;

	boost::shared_ptr<EthernetPort> m_ethernetPort;

	boost::shared_ptr<Aggregator> m_aggregator;

};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_BRIDGEPORT_H_ */
