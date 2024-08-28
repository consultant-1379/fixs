/*
 * subnetvlan.h
 *
 *  Created on: Mar 16, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_SUBNETVLAN_H_
#define ITHADM_CAA_INC_ENGINE_SUBNETVLAN_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine {
class Vlan;

class SubnetVlan {
public:
	SubnetVlan(const operation::subnetVlanInfo& data);

	SubnetVlan(const boost::shared_ptr<SubnetVlan>& rhs);

	virtual ~SubnetVlan();

	const fixs_ith::SwitchBoardPlaneConstants getPlane() const {return m_data.getPlane(); };

	const char* getSmxId() const {return m_data.getSmxId();};

	const char* getName() const {return m_data.getName(); };

	const uint16_t getVlanId() const {return m_data.getSubnetVlanId();};

	const char* getIpSubnet() const {return m_data.getIpSubnet();};

	void setVlan(const boost::shared_ptr<Vlan>& vlan) {m_associatedVlan = vlan;};

	uint16_t modify(const operation::subnetVlanInfo&);

private:
	operation::subnetVlanInfo m_data;
	boost::shared_ptr<Vlan> m_associatedVlan;
};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_SUBNETVLAN_H_ */
