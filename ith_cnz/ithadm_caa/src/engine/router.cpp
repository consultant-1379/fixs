/*
 * router.cpp
 *
 *  Created on: May 4, 2016
 *      Author: eanform
 */

#include "engine/router.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "common/utility.h"

namespace engine {

	Router::Router(const operation::routerInfo& data):
			m_data(data)
	{

	}

	Router::Router(const boost::shared_ptr<Router>& rhs):
			m_data(rhs->m_data)
	{

	}

	Router::~Router()
	{

	}

} /* namespace engine */


