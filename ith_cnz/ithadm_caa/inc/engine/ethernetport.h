/*
 * erthernetport.h
 *
 *  Created on: Mar 16, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_ETHERNETPORT_H_
#define ITHADM_CAA_INC_ENGINE_ETHERNETPORT_H_

#include "operation/operation.h"
#include "fixs_ith_snmpconstants.h"

#include <boost/make_shared.hpp>

namespace engine {

class EthernetPort {
public:
	EthernetPort(const operation::ethernetPortInfo& data);

	EthernetPort(const boost::shared_ptr<EthernetPort>& rhs);

	virtual ~EthernetPort();

	const fixs_ith::SwitchBoardPlaneConstants getPlane() const {return m_data.getPlane(); };

	const char* getSmxId() const {return m_data.getSmxId();};

	const char* getName() const {return m_data.getName(); };

	const uint16_t getSnmpIndex() const {return m_data.getSnmpIndex();};

	bool isSFPport() const {return (m_data.getSnmpIndex() >= ITH_SNMP_COMMUNITY_PIM_PORT_START && m_data.getSnmpIndex() <= ITH_SNMP_COMMUNITY_PIM_PORT_END);};

	const char* getDN() const { return m_data.getEthernetPortDN();};

	const uint32_t getMaxFrameSize() const {return m_data.getMaxFrameSize();};

	const int32_t getOperOperatingMode() const {return m_data.getOperatingMode();};

	imm::EthAutoNegotiate getAutoNegotiate() const { return m_data.getAutoNegotiate(); };

	imm::EthAutoNegotiationState getAutoNegotiationState() const { return m_data.getAutoNegotiationState(); };

	bool isAutoNegotiateEnabled() const { return (imm::AN_UNLOCKED == m_data.getAutoNegotiate()); };

	bool isAutoNegotiationSupported(std::vector<bool> & if_mau_type) const;

	uint16_t modify(const operation::ethernetPortInfo&);

	void setOperOperatingMode(const imm::EthOperOperatingMode& operMode) {m_data.operatingMode = operMode;};

	void setAutoNegotiationState(const imm::EthAutoNegotiationState& autoNegotiationState) {m_data.autoNegotiationState = autoNegotiationState;};

private:
	operation::ethernetPortInfo m_data;

	static const std::vector<fixs_ith_snmp::IfMauTypeListBitsConstants> not_auto_negotiable_mau_types;
};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_ETHERNETPORT_H_ */
