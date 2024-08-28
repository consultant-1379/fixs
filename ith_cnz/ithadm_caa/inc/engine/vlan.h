/*
 * vlan.h
 *
 *  Created on: Mar 15, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_VLAN_H_
#define ITHADM_CAA_INC_ENGINE_VLAN_H_

#include "operation/operation.h"
#include "engine/bridgeport.h"
#include "boost/shared_ptr.hpp"

#include <set>
#include <vector>


namespace engine {

const uint32_t SystemBaseVlanId = 1;

class Vlan {
public:

	Vlan(const operation::vlanInfo& data);

	Vlan(const boost::shared_ptr<Vlan>& rhs);

	virtual ~Vlan();

	const fixs_ith::SwitchBoardPlaneConstants getPlane() const {return m_data.getPlane(); };

	const char* getSmxId() const {return m_data.getSmxId();};

	const char* getName() const {return m_data.getName(); };

	const uint16_t getVlanId() const {return m_data.getVlanId();};

	const std::set<std::string> getTaggedPortsDN() {return m_data.getVlanTaggedPorts();};

	const std::set<std::string> getUntaggedPortsDN() {return m_data.getVlanUntaggedPorts();};

	const char* getDN() const  { return m_data.getVlanDN(); };

	bool hasTaggedPortDN(const std::string& portDN) {return m_data.taggedBridgePorts.find(portDN) != m_data.taggedBridgePorts.end();};

	bool hasUntaggedPortDN(const std::string& portDN) {return m_data.untaggedBridgePorts.find(portDN) != m_data.untaggedBridgePorts.end();};

	void setTaggedBridgePorts(std::set< boost::shared_ptr<BridgePort> > ports);

	void setUntaggedBridgePorts(std::set< boost::shared_ptr<BridgePort> > ports);

	void addTaggedBridgePort(std::set< boost::shared_ptr<BridgePort> > ports);

	void addUntaggedBridgePort(std::set< boost::shared_ptr<BridgePort> > ports);

	void removeTaggedBridgePorts(std::set< boost::shared_ptr<BridgePort> > ports);

	void removeUntaggedBridgePorts(std::set< boost::shared_ptr<BridgePort> > ports);

	void replaceTaggedBridgePort(boost::shared_ptr<BridgePort> port);

	void replaceUntaggedBridgePort(boost::shared_ptr<BridgePort> port);

	std::set< boost::shared_ptr<BridgePort> > getTaggedBridgePorts() {return m_taggedBridgePorts;};

	std::set< boost::shared_ptr<BridgePort> > getUntaggedBridgePorts(){return m_untaggedBridgePorts;};

	std::set<uint16_t> getTaggedBridgePortsIndex();

	std::set<uint16_t>  getUntaggedBridgePortsIndex();

	uint16_t modify(const operation::vlanInfo&);

	const bool isReserved() const {return m_data.getReservedBy().size() > 0;};

	const std::set<std::string> getReservedBy() const {return m_data.getReservedBy();};

private:
	operation::vlanInfo m_data;
	std::set< boost::shared_ptr<BridgePort> > m_taggedBridgePorts;
	std::set< boost::shared_ptr<BridgePort> > m_untaggedBridgePorts;

	void addPortsDN(const std::set< boost::shared_ptr<BridgePort> >& ports, std::set< std::string >& portsDN);

	void removePortsDN(const std::set< boost::shared_ptr<BridgePort> >& ports, std::set< std::string >& portsDN);


};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_VLAN_H_ */
