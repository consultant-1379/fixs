/*
 * router.h
 *
 *  Created on: May 4, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_ENGINE_ROUTER_H_
#define ITHADM_CAA_INC_ENGINE_ROUTER_H_


#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine {

class Router {
public:
	Router(const operation::routerInfo& data);

	Router(const boost::shared_ptr<Router>& rhs);

	virtual ~Router();

	const char* getSmxId() const {return m_data.getSmxId();};

	const char* getName() const {return m_data.getName(); };

	const operation::routerInfo& getData() const { return m_data;};

private:
	operation::routerInfo m_data;

};

} /* namespace engine */



#endif /* ITHADM_CAA_INC_ENGINE_ROUTER_H_ */
