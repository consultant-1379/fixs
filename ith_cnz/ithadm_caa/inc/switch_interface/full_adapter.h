//	********************************************************
//
//	 COPYRIGHT Ericsson 2016
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2016.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2016 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************

#ifndef ITHADM_CAA_INC_SWITCH_INTERFACE_FULL_ADAPTER_H_
#define ITHADM_CAA_INC_SWITCH_INTERFACE_FULL_ADAPTER_H_

#include "switch_interface/L2_layer_adapter.h"
#include "switch_interface/L3_layer_adapter.h"

class fixs_ith_sbdatamanager;

namespace switch_interface
{
	class full_adapter: public L2_layer_adapter, public L3_layer_adapter
	{
	public:
		explicit full_adapter(fixs_ith_sbdatamanager * sb_datamanager = 0):
			L2_layer_adapter(sb_datamanager),
			L3_layer_adapter(sb_datamanager) { }

		virtual ~full_adapter() { };

	private:
		full_adapter(const full_adapter &);
		full_adapter & operator = (const full_adapter &);
	};
};

typedef switch_interface::full_adapter fixs_ith_switchInterfaceAdapter;

#endif /* ITHADM_CAA_INC_SWITCH_INTERFACE_FULL_ADAPTER_H_ */
