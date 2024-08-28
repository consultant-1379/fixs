/*
 * address.h
 *
 *  Created on: Oct 17, 2016
 *      Author: xclaele
 */

#ifndef ITHADM_CAA_INC_ENGINE_ACLENTRY_H_
#define ITHADM_CAA_INC_ENGINE_ACLENTRY_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine {

	class AclEntryIpv4
	{
		public:
			AclEntryIpv4(const operation::aclEntryInfo& data);

			AclEntryIpv4(const boost::shared_ptr<AclEntryIpv4>& rhs);

			virtual ~AclEntryIpv4();

			const char* getSmxId() const {return m_data.getSmxId();};

			const char* getName() const {return m_data.getName();};

			const char* getDN() const {return m_data.getDN();};

			const char* getRouterName() const {return m_routerName.c_str();};

			const char* getAclIpv4Name() const {return m_aclIpv4Name.c_str();};

			const operation::aclEntryInfo& getData() const {return m_data;};

			uint16_t modify(const operation::aclEntryInfo&);

		private:
			operation::aclEntryInfo m_data;

			std::string m_routerName;

			std::string m_aclIpv4Name;
	};

} /* namespace engine */



#endif /* ITHADM_CAA_INC_ENGINE_ACLENTRY_H_ */
