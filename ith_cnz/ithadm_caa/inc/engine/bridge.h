/*
 * bridge.h
 *
 *  Created on: Mar 15, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_BRIDGE_H_
#define ITHADM_CAA_INC_ENGINE_BRIDGE_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine {

class Bridge {
public:
	Bridge(const operation::bridgeInfo& data);

	Bridge(const boost::shared_ptr<Bridge>& rhs);

	virtual ~Bridge();

	const fixs_ith::SwitchBoardPlaneConstants getPlane() const {return m_data.getPlane(); };

	const char* getSmxId() const {return m_data.getSmxId();};

	const char* getName() const {return m_data.getName(); };

	uint16_t modify(const operation::bridgeInfo&);


private:
	operation::bridgeInfo m_data;
};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_BRIDGE_H_ */
