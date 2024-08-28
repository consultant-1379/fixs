/*
 * bfdprofile.h
 *
 *  Created on: Jun 24, 2016
 *      Author: xclaele
 */

#ifndef ITHADM_CAA_INC_ENGINE_BFDPROFILE_H_
#define ITHADM_CAA_INC_ENGINE_BFDPROFILE_H_

#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine {

class BfdProfile {

	public:

		BfdProfile(const operation::bfdProfileInfo& data);

		BfdProfile(const boost::shared_ptr<BfdProfile>& rhs);

		virtual ~BfdProfile();

		const char* getName() const {return m_data.getName(); };

		const char* getDN() const {return m_data.getDN(); };

		const char* getRouterName() const {return m_routerName.c_str(); };

		const operation::bfdProfileInfo getData() const {return m_data; };

		uint16_t modify(const operation::bfdProfileInfo&);

	private:
		operation::bfdProfileInfo m_data;

		std::string m_routerName;

	};

} /* namespace engine */

#endif /* ITHADM_CAA_INC_ENGINE_BFDPROFILE_H_ */
