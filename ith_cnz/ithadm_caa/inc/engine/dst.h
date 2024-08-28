/*
 * dst.h
 *
 *  Created on: May 17, 2016
 *      Author: estevol
 */

#ifndef ITHADM_CAA_INC_ENGINE_DST_H_
#include "operation/operation.h"

#include "boost/shared_ptr.hpp"

namespace engine {

class Dst {
public:
	Dst(const operation::dstInfo& data);

	Dst(const boost::shared_ptr<Dst>& rhs);

	virtual ~Dst();

	const char* getSmxId() const {return m_data.getSmxId();};

	const char* getName() const {return m_data.getName(); };

	const char* getDst() const {return m_data.getDst(); };

	const char* getRouterName() const {return m_routerName.c_str(); };

	const operation::dstInfo& getData() const {return m_data; };

	uint16_t modify(const operation::dstInfo&);

private:
	operation::dstInfo m_data;

	std::string m_routerName;

};

} /* namespace engine */



#endif /* ITHADM_CAA_INC_ENGINE_DST_H_ */
