/*
 * interfaceipv4.h
 *
 *  Created on: May 9, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_NEXTHOP_H_
#define ITHADM_CAA_INC_ENGINE_NEXTHOP_H_

#include "operation/operation.h"
#include "common/utility.h"

#include "boost/shared_ptr.hpp"

namespace engine {

	class NextHop {
	public:
		NextHop(const operation::nextHopInfo& data);

		NextHop(const boost::shared_ptr<NextHop>& rhs);

		virtual ~NextHop();

		const char* getSmxId() const {return m_data.getSmxId();};

		const char* getName() const {return m_data.getName(); };

		const char* getDN() const {return m_data.getDN(); };

		const char* getRouterName() const {return m_routerName.c_str();};

		const char* getDstName() const {return m_dstName.c_str();};

		const bool isDiscard() const { return m_data.getDiscard(); };

		bool hasAddress() const { return m_data.hasAddress(); }

		bool isBFDEnabled() const { return m_data.getBfdMonitoring(); };

		const char* getAddress() const {return m_data.getAddress(); };

		const operation::nextHopInfo& getData() const {return m_data;};

		uint16_t modify(const operation::nextHopInfo&);


	private:
		operation::nextHopInfo m_data;

		std::string m_routerName;

		std::string m_dstName;
	};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_NEXTHOP_H_ */
