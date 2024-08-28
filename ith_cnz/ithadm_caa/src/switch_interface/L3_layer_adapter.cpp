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

#include <stdlib.h>
#include <limits.h>

#include <boost/algorithm/string/replace.hpp>

//#include "switch_interface/L3_layer_adapter.h"

#include "fixs_ith_workingset.h"
#include "fixs_ith_switchboardinfo.h"
#include "netconf/rdn_path.h"

#include "switch_interface/L3_layer_adapter.h"

namespace switch_interface
{

L3_layer_adapter::L3_layer_adapter(fixs_ith_sbdatamanager * sb_datamanager)
{
	// if switchboards data manager has not been provided, use the instance defined in the ITH working set
	_sb_datamanager = (sb_datamanager ? sb_datamanager: & fixs_ith::workingSet_t::instance()->get_sbdatamanager());
}


op_result L3_layer_adapter::create_router(const operation::routerInfo & routerInfo, bool useMerge)
{
	// get Id of the Router to be created on SMX
	std::string  routerId = common::utility::getRouterIdFromRouterName(routerInfo.name);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to CREATE the ROUTER '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
				 routerId.c_str(), routerInfo.moDN.c_str(), routerInfo.smxId.c_str(), useMerge ? "[using MERGE]": "");

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, routerInfo.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the ROUTER '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				routerId.c_str(), routerInfo.moDN.c_str(), routerInfo.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// we need to adjust IMM DN removing from ROUTER RDN the part <mag>_<slot>:
	// for example if the ROUTER RDN is "1.2.0.4_0:test" ---> the adjusted ROUTER RDN will be 'test'
	std::string adjusted_router_dn;
	_adjust_routerId_in_imm_dn(adjusted_router_dn, routerInfo.getDN());

	// now use NETCONF Manager to create the ROUTER
	int create_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().create_router(switchboard_info, adjusted_router_dn.c_str(), useMerge);
	if(create_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the ROUTER '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::create_router()' returned '%d']",
						routerId.c_str(), routerInfo.moDN.c_str(), routerInfo.smxId.c_str(), create_res);

		return map_netconfmanager_error(create_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully created the ROUTER '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			routerId.c_str(), routerInfo.moDN.c_str(), routerInfo.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::delete_router(const operation::routerInfo & routerInfo)
{
	// get Router Id to be deleted on SMX
	std::string  routerId = common::utility::getRouterIdFromRouterName(routerInfo.name);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to DELETE the ROUTER '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
				routerId.c_str(), routerInfo.moDN.c_str(), routerInfo.smxId.c_str());

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, routerInfo.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the ROUTER '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				routerId.c_str(), routerInfo.moDN.c_str(), routerInfo.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// we need to adjust IMM DN removing from ROUTER RDN the part <mag>_<slot>.
	// for example if the ROUTER RDN is "1.2.0.4_0:test" ---> the adjusted ROUTER RDN will be 'test'
	std::string adjusted_router_dn;
	_adjust_routerId_in_imm_dn(adjusted_router_dn, routerInfo.moDN);

	// now use NETCONF Manager to delete the ROUTER
	int delete_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().delete_router(switchboard_info, adjusted_router_dn.c_str());
	if(delete_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE ROUTER '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::delete_router()' returned '%d']",
						routerId.c_str(), routerInfo.moDN.c_str(), routerInfo.smxId.c_str(), delete_res);

		return map_netconfmanager_error(delete_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully deleted the ROUTER '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			routerId.c_str(), routerInfo.moDN.c_str(), routerInfo.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::create_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info, bool useMerge)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to CREATE the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
					 interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(),
					 useMerge ? "[using MERGE]": "");

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, interfaceIPv4Info.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// modify the ROUTER RDN in the IMM DN, removing the prefix '<mag>_<slot>:';
	// (example: substitute "routerId=1.2.0.4_0:oam" with "routerId=oam")
	std::string adjusted_interfaceIpV4_dn;
	_adjust_routerId_in_imm_dn(adjusted_interfaceIpV4_dn, interfaceIPv4Info.moDN);

	// now use NETCONF Manager to create the INTERFACE IPv4
	netconf::configdataList configdataList;
	_build_configdataList(configdataList, interfaceIPv4Info, true, OP_CREATE);
	_log_configdataList(configdataList);	// only for debug purposes !!
	int create_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().create_ipv4_interface(switchboard_info, adjusted_interfaceIpV4_dn.c_str(), configdataList, useMerge);
	if(create_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::create_ipv4_interface()' returned '%d']",
						interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(), create_res);

		return map_netconfmanager_error(create_res);
	}

	// attach acl
	if(interfaceIPv4Info.changeMask & imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE)
	{
		std::string aclNameOnSMX;
		_adjust_acl_name(interfaceIPv4Info.aclData.getAclName(), interfaceIPv4Info.aclData.getRouterName(), aclNameOnSMX);

		int acl_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().attach_acl_to_interface(switchboard_info,
																			interfaceIPv4Info.aclData.getRouterName(),
																			interfaceIPv4Info.aclData.getVlanId(),
																			aclNameOnSMX);

		if(acl_res != netconf::ERR_NO_ERROR)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to ATTACH ACL:<%s> to the INTERFACE IPv4<%s> on the SWITCH BOARD having key '%s'. "
					"[Call 'netconf::manager::attach_acl_to_interface()' returned '%d']",
					interfaceIPv4Info.aclData.getAclName(), interfaceIPv4Info.name.c_str(), interfaceIPv4Info.smxId.c_str(), acl_res);

			// delete the interface
			delete_interfaceIPv4(interfaceIPv4Info);

			return map_netconfmanager_error(acl_res);
		}
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully created the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::delete_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to DELETE the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str());

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, interfaceIPv4Info.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// modify the ROUTER RDN in the IMM DN, removing the prefix '<mag>_<slot>:';
	// (example: substitute "routerId=1.2.0.4_0:oam" with "routerId=oam")
	std::string adjusted_interfaceIpV4_dn;
	_adjust_routerId_in_imm_dn(adjusted_interfaceIpV4_dn, interfaceIPv4Info.moDN);

	// now use NETCONF Manager to delete the INTERFACE IPv4
	int delete_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().delete_ipv4_interface(switchboard_info, adjusted_interfaceIpV4_dn.c_str());
	if(delete_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::delete_ipv4_interface()' returned '%d']",
						interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(), delete_res);

		return map_netconfmanager_error(delete_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully deleted the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::modify_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info, bool useMerge)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to MODIFY the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
			interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(),
			useMerge ? "[using MERGE]": "");

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, interfaceIPv4Info.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to MODIFY the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// modify the ROUTER RDN in the IMM DN, removing the prefix '<mag>_<slot>:';
	// (example: substitute "routerId=1.2.0.4_0:oam" with "routerId=oam")
	std::string adjusted_interfaceIpV4_dn;
	_adjust_routerId_in_imm_dn(adjusted_interfaceIpV4_dn, interfaceIPv4Info.moDN);

	// now use NETCONF Manager to modify the INTERFACE IPv4
	netconf::configdataList configdataList;
	_build_configdataList(configdataList, interfaceIPv4Info, true, OP_MODIFY);
	_log_configdataList(configdataList);	// only for debug purposes !!
	int modify_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().modify_ipv4_interface(switchboard_info, adjusted_interfaceIpV4_dn.c_str(), configdataList, useMerge);
	if(modify_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to MODIFY the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::modify_ipv4_interface()' returned '%d']",
						interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(), modify_res);

		return map_netconfmanager_error(modify_res);
	}


	if(interfaceIPv4Info.changeMask & imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE)
	{
		int acl_res;
		std::string aclNameOnSMX;
		if( interfaceIPv4Info.isAclIngressEmpty() )
		{
			// remove ACL from interface
			acl_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().detach_acl_to_interface(switchboard_info,
							interfaceIPv4Info.aclData.getRouterName(),
							interfaceIPv4Info.aclData.getVlanId());
		}
		else
		{
			// Change ACL on Interface
			std::string aclNameOnSMX;
			_adjust_acl_name(interfaceIPv4Info.aclData.getAclName(), interfaceIPv4Info.aclData.getRouterName(), aclNameOnSMX);

			acl_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().attach_acl_to_interface(switchboard_info,
				interfaceIPv4Info.aclData.getRouterName(),
				interfaceIPv4Info.aclData.getVlanId(),
				aclNameOnSMX);
		}

		if(acl_res != netconf::ERR_NO_ERROR)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET ACL:<%s> to the INTERFACE IPv4<%s> on the SWITCH BOARD having key '%s'. "
					"[Call 'netconf::manager::attach_acl_to_interface()' returned '%d']",
					interfaceIPv4Info.aclData.getAclName(), interfaceIPv4Info.name.c_str(), interfaceIPv4Info.smxId.c_str(), acl_res);

			// TODO: undo previous applied changes????

			return map_netconfmanager_error(acl_res);
		}
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully modified the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::get_interfaceIPv4_info(operation::interfaceInfo & interfaceIPv4Info)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET info about the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
					interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str());

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, interfaceIPv4Info.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET info about the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	std::string adjusted_interfaceIpV4_dn;
	_adjust_routerId_in_imm_dn(adjusted_interfaceIpV4_dn, interfaceIPv4Info.getDN() );

	// now use NETCONF Manager to get info about the INTERFACE IPv4
	netconf::configdataList configdataList;
	_build_configdataList(configdataList, interfaceIPv4Info);
	_log_configdataList(configdataList);	// only for debug purposes !!

	int getinfo_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().get_ipv4_interface
			(configdataList, switchboard_info, adjusted_interfaceIpV4_dn.c_str());
	if(getinfo_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET info about the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::get_ipv4_interface()' returned '%d']",
						interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str(), getinfo_res);

		return map_netconfmanager_error(getinfo_res);
	}

	// fill output parameter
	_fill_from_configdataList(interfaceIPv4Info, configdataList);

	_log_interfaceInfo(interfaceIPv4Info);  // only for debug purposes !!

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT info about the INTERFACE IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			interfaceIPv4Info.name.c_str(), interfaceIPv4Info.moDN.c_str(), interfaceIPv4Info.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::create_addressIPv4(const operation::addressInfo & addressIPv4Info, bool useMerge)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to CREATE the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
					addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str(),
					useMerge ? "[using MERGE]": "");

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, addressIPv4Info.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_addressIpV4_dn;
	_adjust_routerId_in_imm_dn(adjusted_addressIpV4_dn, addressIPv4Info.getDN() );

	// now use NETCONF Manager to create the ADDRESS IPv4
	netconf::configdataList configdataList;
	_build_configdataList(configdataList, addressIPv4Info, false);
	_log_configdataList(configdataList);	// only for debug purposes !!
	int create_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().create_ipv4_address(switchboard_info, adjusted_addressIpV4_dn.c_str(), configdataList, useMerge);
	if(create_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::create_ipv4_address()' returned '%d']",
						addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str(), create_res);

		return map_netconfmanager_error(create_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully created the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::delete_addressIPv4(const operation::addressInfo & addressIPv4Info)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to DELETE the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
					addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str());

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, addressIPv4Info.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_addressIpV4_dn;
	_adjust_routerId_in_imm_dn(adjusted_addressIpV4_dn, addressIPv4Info.getDN() );

	// now use NETCONF Manager to delete the ADDRESS IPv4
	int delete_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().delete_ipv4_address(switchboard_info, adjusted_addressIpV4_dn.c_str());
	if(delete_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::delete_ipv4_address()' returned '%d']",
						addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str(), delete_res);

		return map_netconfmanager_error(delete_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully deleted the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::modify_addressIPv4(const operation::addressInfo & addressIPv4Info, bool useMerge)
{
	return _execute_operation<OP_MODIFY>("ADDRESS IPv4", & netconf::manager::modify_ipv4_address, addressIPv4Info, useMerge);
}

op_result L3_layer_adapter::get_addressIPv4(operation::addressInfo & addressIPv4Info)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET info about the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str());

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, addressIPv4Info.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET info about the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	std::string adjusted_addressIpV4_dn;
	_adjust_routerId_in_imm_dn(adjusted_addressIpV4_dn, addressIPv4Info.getDN() );

	// now use NETCONF Manager to get info about the INTERFACE IPv4
	netconf::configdataList configdataList;
	_build_configdataList(configdataList, addressIPv4Info);
	_log_configdataList(configdataList);	// only for debug purposes !!

	int getinfo_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().get_ipv4_address
			(configdataList, switchboard_info, adjusted_addressIpV4_dn.c_str());
	if(getinfo_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET info about the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::get_ipv4_address()' returned '%d']",
						addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str(), getinfo_res);

		return map_netconfmanager_error(getinfo_res);
	}

	// fill output parameter
	_fill_from_configdataList(addressIPv4Info, configdataList);

	_log_addressInfo(addressIPv4Info);  // only for debug purposes !!

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT info about the ADDRESS IPv4 '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			addressIPv4Info.name.c_str(), addressIPv4Info.moDN.c_str(), addressIPv4Info.smxId.c_str());

	return SBIA_OK;

}

op_result L3_layer_adapter::create_Dst(const operation::dstInfo & dstInfo, bool useMerge)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to CREATE the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
						dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str(),
						useMerge ? "[using MERGE]": "");

	// retrieve switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, dstInfo.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, dstInfo.getDN() );

	// now use NETCONF Manager to create the DST
	netconf::configdataList configdataList;
	_build_configdataList(configdataList, dstInfo, false);
	_log_configdataList(configdataList);	// only for debug purposes !!
	int create_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().create_dst(switchboard_info, adjusted_dn.c_str(), configdataList, useMerge);
	if(create_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::create_dst()' returned '%d']",
						dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str(), create_res);

		return map_netconfmanager_error(create_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully created the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::delete_Dst(const operation::dstInfo & dstInfo)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to DELETE the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
						dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str());

	// retrieve switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, dstInfo.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, dstInfo.getDN() );

	// now use NETCONF Manager to delete the DST
	int op_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().delete_dst(switchboard_info, adjusted_dn.c_str());
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::delete_dst()' returned '%d']",
						dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str(), op_res);

		return map_netconfmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully deleted the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::modify_Dst(const operation::dstInfo & dstInfo, bool useMerge)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to MODIFY the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
			dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str(), useMerge ? "[using MERGE]": "");

	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, dstInfo.smxId);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to MODIFY the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// modify the ROUTER RDN in the IMM DN, removing the prefix '<mag>_<slot>:';
	// (example: substitute "routerId=1.2.0.4_0:oam" with "routerId=oam")
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, dstInfo.moDN);

	netconf::configdataList configdataList;
	_build_configdataList(configdataList, dstInfo);
	_log_configdataList(configdataList);	// only for debug purposes !!
	int op_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().modify_dst(switchboard_info, adjusted_dn.c_str(), configdataList, useMerge);
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to MODIFY the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::modify_dst()' returned '%d']",
						dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str(), op_res);

		return map_netconfmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully modified the DST '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			dstInfo.name.c_str(), dstInfo.moDN.c_str(), dstInfo.smxId.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::create_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo, bool useMerge)
{
	return _execute_operation<OP_CREATE>("BFD_PROFILE", & netconf::manager::create_bfd_profile, bfdProfileInfo, useMerge);
}

op_result L3_layer_adapter::delete_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo)
{
	return _execute_operation<OP_DELETE>("BFD_PROFILE", & netconf::manager::delete_bfd_profile, bfdProfileInfo);
}


op_result L3_layer_adapter::modify_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo, bool useMerge)
{
	return _execute_operation<OP_MODIFY>("BFD_PROFILE", & netconf::manager::modify_bfd_profile, bfdProfileInfo, useMerge);
}

op_result L3_layer_adapter::get_BfdSessionIPv4_info(operation::bfdSessionInfo & bfdSessionIPv4Info)
{
	return _execute_operation<OP_GET>("BFDSESSIONIPV4INFO", & netconf::manager::get_bfd_sessionInfo, bfdSessionIPv4Info);
}

op_result L3_layer_adapter::get_Dst(operation::dstInfo & dstInfo)
{
	return _execute_operation<OP_GET>("DST", & netconf::manager::get_dst, dstInfo);
}

op_result  L3_layer_adapter::create_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo, bool useMerge)
{
	if(useMerge)
	{
		// if the vrrp session is already defined on the SMX, we MUST order a MODIFY operation, not a CREATE, because
		// in the request towards SMX we cannot include restricted attributes of SMX class "Vrrpv3IPv4Session" (e.g: "owner")
		operation::vrrpSessionInfo local_vrrpSessionInfo(vrrpSessionInfo);
		op_result op_res = _execute_operation<OP_GET>("VRRP_V3_IPv4_SESSION", & netconf::manager::get_vrrp_session, local_vrrpSessionInfo);
		if(op_res ==  switch_interface::SBIA_OK)
		{
			return _execute_operation<OP_MODIFY>("VRRP_V3_IPv4_SESSION",
					& netconf::manager::modify_vrrp_session, vrrpSessionInfo, useMerge);
		}
	}

	return _execute_operation<OP_CREATE>("VRRP_V3_IPv4_SESSION",
			& netconf::manager::create_vrrp_session, vrrpSessionInfo, useMerge);
}

op_result  L3_layer_adapter::delete_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo)
{
	return _execute_operation<OP_DELETE>("VRRP_V3_IPv4_SESSION", & netconf::manager::delete_vrrp_session, vrrpSessionInfo);
}

op_result L3_layer_adapter::modify_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo, bool useMerge)
{
   return _execute_operation<OP_MODIFY>("VRRP_V3_IPv4_SESSION", & netconf::manager::modify_vrrp_session, vrrpSessionInfo, useMerge);
}
op_result  L3_layer_adapter::create_NextHopInfo(const operation::nextHopInfo & nextHopInfo, bool useMerge)
{
	return _execute_operation<OP_CREATE>("NEXTHOP", & netconf::manager::create_next_hop, nextHopInfo, useMerge);
}

op_result L3_layer_adapter::modify_NextHopInfo(const operation::nextHopInfo & nextHopInfo, bool useMerge)
{
	return _execute_operation<OP_MODIFY>("NEXTHOP", & netconf::manager::modify_next_hop, nextHopInfo, useMerge);
}

op_result L3_layer_adapter::delete_NextHopInfo(const operation::nextHopInfo & nextHopInfo)
{
	return _execute_operation<OP_DELETE>("NEXTHOP", (NETCONF_MANAGER_DELETE_FUNC) 0, nextHopInfo);
}

op_result  L3_layer_adapter::get_NextHopInfo(operation::nextHopInfo & nextHopInfo)
{
	return _execute_operation<OP_GET>("NEXTHOP", & netconf::manager::get_next_hop, nextHopInfo);
}


op_result L3_layer_adapter::get_vrrpv3Session_info(operation::vrrpSessionInfo & vrrpSessionInfo) {
	return _execute_operation<OP_GET>("VRRP_V3_IPv4_SESSION",
			& netconf::manager::get_vrrp_session, vrrpSessionInfo);
}


op_result L3_layer_adapter::enable_Acl(const std::string & switchboard_key)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to enable ACL handling on the SWITCH BOARD having key '%s'", switchboard_key.c_str());

	// Retrieve switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key.c_str());
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to enable ACL handling on the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']", switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// invoke NETCONF manager to know if ACL handling is already enabled
	bool acl_status = false;
	int op_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().get_acl_status(switchboard_info, acl_status);
	if((op_res == netconf::ERR_NO_ERROR) && (acl_status == true /*ENABLED*/))
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ACL handling is already enabled on the SWITCH BOARD having key '%s'", switchboard_key.c_str());
		return SBIA_OK;
	}

	// invoke NETCONF manager to enable ACL
	op_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().enable_acl(switchboard_info);
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to enable ACL handling on the SWITCH BOARD having key '%s'"
						"[Call 'netconf::enable_acl()' returned '%d']", switchboard_key.c_str(), op_res);
		return map_netconfmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully enabled ACL handling on the SWITCH BOARD having key '%s'", switchboard_key.c_str());

	return SBIA_OK;
}


op_result L3_layer_adapter::create_AclEntry(const operation::aclEntryInfo & aclEntryInfo, bool useMerge)
{
	return _execute_acl_create_or_modify_operation<OP_CREATE>("ACL_ENTRY", & netconf::manager::create_acl_entry, aclEntryInfo, useMerge);
}

op_result L3_layer_adapter::delete_AclEntry(const operation::aclEntryInfo & aclEntryInfo)
{
	return _execute_operation<OP_DELETE>("ACL_ENTRY", & netconf::manager::delete_acl_entry, aclEntryInfo);
}

op_result L3_layer_adapter::modify_AclEntry(const operation::aclEntryInfo & aclEntryInfo, bool useMerge)
{
	return _execute_acl_create_or_modify_operation<OP_MODIFY>("ACL_ENTRY", & netconf::manager::modify_acl_entry, aclEntryInfo, useMerge);
}

op_result L3_layer_adapter::delete_AclIpv4(const operation::aclIpV4Info & aclIpV4Info)
{
	return _execute_operation<OP_DELETE>("ACL_IPV4", & netconf::manager::delete_acl_ipv4, aclIpV4Info);
}

void L3_layer_adapter::_build_configdataList(netconf::configdataList & configdataList, const operation::addressInfo & addressInfo, bool use_changemask, op_type /*opType*/)
{
	configdataList.clear();

	// NAME handling
	netconf::configdataElement name_configElement(netconf::addressIpv4::attribute::KEY, addressInfo.getName());
	configdataList.push_back(name_configElement);

	// ADDRESS handling
	if(!use_changemask || (addressInfo.changeMask & imm::addressIpv4_attribute::ADDRESS_CHANGE))
	{
		netconf::configdataElement encapsulation_configElement(netconf::addressIpv4::attribute::ADDRESS, addressInfo.address);
		configdataList.push_back(encapsulation_configElement);
	}
}


void L3_layer_adapter::_build_configdataList(netconf::configdataList & configdataList, const operation::interfaceInfo & interfaceInfo, bool use_changemask, op_type opType)
{
	configdataList.clear();

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Interface:<%s> Change Mask:<%d>", interfaceInfo.getName(), interfaceInfo.changeMask);
	// NAME handling
	netconf::configdataElement name_configElement(netconf::interfaceIpv4::attribute::KEY, interfaceInfo.getName());
	configdataList.push_back(name_configElement);

	// ADMINSTATE handling
	if(!use_changemask || (interfaceInfo.changeMask & imm::interfaceIpv4_attribute::ADMIN_STATE_CHANGE))
	{
		netconf::configdataElement adminstate_configElement(netconf::interfaceIpv4::attribute::ADMIN_STATE, adapters_utils::if_adminState_descr(interfaceInfo.adminstate));
		configdataList.push_back(adminstate_configElement);
	}

	// BFDSTATICROUTES and BFDPROFILE handling
	if(interfaceInfo.bfdStaticRoutes == imm::ENABLED)
	{
		netconf::configdataElement bfdprofile_configElement(netconf::interfaceIpv4::attribute::BFD_PROFILE, interfaceInfo.getBfdProfile());
		configdataList.push_back(bfdprofile_configElement);
	}// BfdStaticRoutes has been changed to DISABLED set BfdProfile rdn to 'null'
	else if(!use_changemask ||
			((interfaceInfo.changeMask & imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES_CHANGE) &&
					opType != OP_CREATE)	 )
	{
		std::string interface_dn = interfaceInfo.getDN();
			std::string null_bfd_profile = build_null_bfd_profile_dn(interface_dn);
			netconf::configdataElement bfdprofile_configElement(netconf::interfaceIpv4::attribute::BFD_PROFILE, null_bfd_profile);
			configdataList.push_back(bfdprofile_configElement);
	}

	// ENCAPSULATION handling
	if(!use_changemask || (interfaceInfo.changeMask & imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE))
	{
		netconf::configdataElement encapsulation_configElement(netconf::interfaceIpv4::attribute::ENCAPSULATION, interfaceInfo.getEncapsulation());
		configdataList.push_back(encapsulation_configElement);

		char encapsulationVid_str[16] = {0};
		::sprintf(encapsulationVid_str, "%u",interfaceInfo.encapsulationVid);
		netconf::configdataElement encapsulationVid_configElement(netconf::interfaceIpv4::attribute::ENCAPSULATION_VID, encapsulationVid_str);
		configdataList.push_back(encapsulationVid_configElement);
	}

	// LOOPBACK handling
	if(!use_changemask || (interfaceInfo.changeMask & imm::interfaceIpv4_attribute::LOOPBACK_CHANGE))
	{
		if (interfaceInfo.loopback)
		{
			netconf::configdataElement loopback_configElement(netconf::interfaceIpv4::attribute::ENCAPSULATION, "loopback");
			configdataList.push_back(loopback_configElement);
		}
	}

	// MTU handling
    if(!use_changemask || (interfaceInfo.changeMask & imm::interfaceIpv4_attribute::MTU_CHANGE))
	{
		char mtu_str[16] = {0};
		::sprintf(mtu_str, "%u",interfaceInfo.getMTU());

		netconf::configdataElement mtu_configElement(netconf::interfaceIpv4::attribute::MTU, mtu_str);
		configdataList.push_back(mtu_configElement);
	}

    if(opType == OP_CREATE)
    {
    	// add MAC attribute set to "auto" (this attribute is not represented in IMM)
    	//netconf::configdataElement mac_configElement(netconf::interfaceIpv4::attribute::MAC, "auto");
    	//set MAC to 'system' as required by BSC
    	netconf::configdataElement mac_configElement(netconf::interfaceIpv4::attribute::MAC, "system");
    	configdataList.push_back(mac_configElement);
    }

    if(!use_changemask || (interfaceInfo.changeMask & imm::interfaceIpv4_attribute::OPER_STATE_CHANGE))
    {
    	netconf::configdataElement operState_configElement(netconf::interfaceIpv4::attribute::OPER_STATE, "");
    	configdataList.push_back(operState_configElement);
    }
}


void L3_layer_adapter::_build_configdataList(netconf::configdataList & configdataList, const operation::dstInfo & dstInfo, bool use_changemask, op_type /*opType*/)
{
	configdataList.clear();

	// NAME handling
	netconf::configdataElement name_configElement(netconf::dst::attribute::KEY, dstInfo.getName());
	configdataList.push_back(name_configElement);

	// ADDRESS handling
	if(!use_changemask || (dstInfo.changeMask & imm::dst_attribute::DST_CHANGE))
	{
		netconf::configdataElement encapsulation_configElement(netconf::dst::attribute::DST, dstInfo.dst);
		configdataList.push_back(encapsulation_configElement);
	}
}


void L3_layer_adapter::_build_configdataList(netconf::configdataList & configdataList, const operation::nextHopInfo & nextHopInfo, bool use_changemask, op_type /*opType*/)
{
	configdataList.clear();

	// NAME handling
	netconf::configdataElement name_configElement(netconf::nextHop::attribute::KEY, nextHopInfo.getName());
	configdataList.push_back(name_configElement);

	// ADDRESS and DISCARD handling
	if(!use_changemask || (nextHopInfo.changeMask & (imm::nextHop_attribute::ADDRESS_CHANGE | imm::nextHop_attribute::DISCARD_CHANGE)))
	{
		if(nextHopInfo.discard)
		{
			netconf::configdataElement discard_configElement(netconf::nextHop::attribute::DISCARD, "true");
			configdataList.push_back(discard_configElement);
		}
		else
		{
			netconf::configdataElement address_configElement(netconf::nextHop::attribute::ADDRESS, nextHopInfo.getAddress());
			configdataList.push_back(address_configElement);
		}
	}

	// ADMIN DISTANCE handling
	if(!use_changemask || (nextHopInfo.changeMask & imm::nextHop_attribute::ADMIN_DISTANCE_CHANGE))
	{
		char adminDistance_str[16] = {0};
		::sprintf(adminDistance_str, "%d",nextHopInfo.getAdminDistance());

		netconf::configdataElement adminDistance_configElement(netconf::nextHop::attribute::ADMIN_DISTANCE, adminDistance_str);
		configdataList.push_back(adminDistance_configElement);
	}

	// BFDMONITORING handling
	if(!use_changemask || (nextHopInfo.changeMask & imm::nextHop_attribute::BFD_MONITORING_CHANGE))
	{
		netconf::configdataElement bfdMonitoring_configElement(netconf::nextHop::attribute::BFD_ENABLE, nextHopInfo.bfdMonitoring ? "true": "false");
		configdataList.push_back(bfdMonitoring_configElement);
	}
}

void L3_layer_adapter::_build_configdataList(netconf::configdataList & configdataList, const operation::bfdProfileInfo & bfdProfileInfo, bool use_changemask, op_type /*opType*/)
{
	configdataList.clear();

	// NAME handling
	netconf::configdataElement name_configElement(netconf::bfdProfile::attribute::KEY, bfdProfileInfo.getName());
	configdataList.push_back(name_configElement);

	// DETECTIONMULTIPLIER handling
	if(!use_changemask || (bfdProfileInfo.changeMask & imm::bfdProfile_attribute::DETECTION_MULTI_CHANGE))
	{
		char detection_multi_str[16] = {0};
		::sprintf(detection_multi_str, "%u",bfdProfileInfo.getDetectionMultiplier());
		netconf::configdataElement detection_multi_configElement(netconf::bfdProfile::attribute::DETECTION_MULTI, detection_multi_str);
		configdataList.push_back(detection_multi_configElement);
	}
	// INTERVALRXMINREQUIRED handling
	if(!use_changemask || (bfdProfileInfo.changeMask & imm::bfdProfile_attribute::INTERVAL_RX_MIN_CHANGE))
	{
		char interval_rx_str[16] = {0};
		::sprintf(interval_rx_str, "%u",bfdProfileInfo.getIntervalRx());
		netconf::configdataElement interval_rx_min_configElement(netconf::bfdProfile::attribute::INTERVAL_RX_MIN, interval_rx_str);
		configdataList.push_back(interval_rx_min_configElement);
	}
	// INTERVALTXMINDESIRED handling
	if(!use_changemask || (bfdProfileInfo.changeMask & imm::bfdProfile_attribute::INTERVAL_TX_MIN_CHANGE))
	{
		char interval_tx_str[16] = {0};
		::sprintf(interval_tx_str, "%u",bfdProfileInfo.getIntervalTx());
		netconf::configdataElement interval_tx_min_configElement(netconf::bfdProfile::attribute::INTERVAL_TX_MIN, interval_tx_str);
		configdataList.push_back(interval_tx_min_configElement);
	}


	// ADD the SMX mandatory attribute 'requiredMinEchoRxInterval' in configurationdataList
	// since this attribute doesn't exist on ITH model it is set 'temporarily' equal to INTERVALRXMINREQUIRED.
	if(!use_changemask || (bfdProfileInfo.changeMask & imm::bfdProfile_attribute::INTERVAL_RX_MIN_CHANGE))
	{
		char interval_echo_rx_str[16] = {0};
		::sprintf(interval_echo_rx_str, "%u",bfdProfileInfo.getIntervalRx());
		netconf::configdataElement interval_echo_rx_min_configElement(netconf::bfdProfile::attribute::INTERVAL_ECHO_RX_MIN, interval_echo_rx_str);
		configdataList.push_back(interval_echo_rx_min_configElement);
	}

}

void L3_layer_adapter::_build_configdataList(netconf::configdataList & configdataList, const operation::bfdSessionInfo & bfdSessionInfo, bool /*use_changemask*/, op_type /*opType*/)
{
	configdataList.clear();

	// NAME handling
	netconf::configdataElement name_configElement(netconf::bfdSessionInfo::attribute::KEY, bfdSessionInfo.getName());
	configdataList.push_back(name_configElement);

	// ADDRESS DST handling
	netconf::configdataElement addr_dst_configElement(netconf::bfdSessionInfo::attribute::PEER_ADDRESS, bfdSessionInfo.getAddrDst());
	configdataList.push_back(addr_dst_configElement);

	// ADDRESS SRC handling
	netconf::configdataElement addr_src_configElement(netconf::bfdSessionInfo::attribute::PEER_INTERFACE, bfdSessionInfo.getAddrSrc());
	configdataList.push_back(addr_src_configElement);

	// DETECTION TIME handling
	char detect_time_str[16] = {0};
	::sprintf(detect_time_str, "%u",bfdSessionInfo.getDetectionTime());
	netconf::configdataElement detect_time_configElement(netconf::bfdSessionInfo::attribute::DETECTION_TIME, detect_time_str);
	configdataList.push_back(detect_time_configElement);

	// DIAGNOSTIC CODE handling
	char diagn_code_str[16] = {0};
	::sprintf(diagn_code_str, "%u",bfdSessionInfo.getDiagCode());
	netconf::configdataElement diagn_code_configElement(netconf::bfdSessionInfo::attribute::DIAGNOSTIC_CODE, diagn_code_str);
	configdataList.push_back(diagn_code_configElement);

	// INTERVAL TX NEGOTIATED handling
	char interval_tx_str[16] = {0};
	::sprintf(interval_tx_str, "%u",bfdSessionInfo.getIntervalTx());
	netconf::configdataElement interval_tx_min_configElement(netconf::bfdSessionInfo::attribute::NEGOTIATED_INTERVAL, interval_tx_str);
	configdataList.push_back(interval_tx_min_configElement);

	// LOCAL DISCRIMINATOR handling
	char local_discr_str[16] = {0};
	::sprintf(local_discr_str, "%u",bfdSessionInfo.getLocalDiscr());
	netconf::configdataElement local_discr_configElement(netconf::bfdSessionInfo::attribute::MY_DISCRIMINATOR, local_discr_str);
	configdataList.push_back(local_discr_configElement);

	// REMOTE DISCRIMINATOR handling
	char remote_discr_str[16] = {0};
	::sprintf(remote_discr_str, "%u",bfdSessionInfo.getRemoteDiscr());
	netconf::configdataElement remote_discr_configElement(netconf::bfdSessionInfo::attribute::YOUR_DISCRIMINATOR, remote_discr_str);
	configdataList.push_back(remote_discr_configElement);

	// PORT SRC handling
	char port_src_str[16] = {0};
	::sprintf(port_src_str, "%u",bfdSessionInfo.getPortSrc());
	netconf::configdataElement port_src_configElement(netconf::bfdSessionInfo::attribute::UDP_PORT_SRC, port_src_str);
	configdataList.push_back(port_src_configElement);

	// SESSION STATE handling
	char state_str[16] = {0};
	::sprintf(state_str, "%u",bfdSessionInfo.getSessionState());
	netconf::configdataElement session_state_configElement(netconf::bfdSessionInfo::attribute::STATE, state_str);
	configdataList.push_back(session_state_configElement);
}

void L3_layer_adapter::_build_aclConfigdataList(netconf::configdataList & aclAttributes,
		std::vector<netconf::configdataList> & aclConditions,
		netconf::configdataList & aclAction,
		const operation::aclEntryInfo & aclEntryInfo)
{
	aclAttributes.clear();
	aclConditions.clear();
	aclAction.clear();

	/***  Start Rule handling ***/
	// RULE NAME handling
	netconf::configdataElement rule_name_configElement(netconf::rule::attribute::KEY, aclEntryInfo.getName());
	aclAttributes.push_back(rule_name_configElement);

	// PRIORITY handling
	if ( aclEntryInfo.changeMask & imm::aclEntry_attribute::PRIORITY_CHANGE)
	{
		char priority_str[16] = {0};
		::sprintf(priority_str, "%u", aclEntryInfo.getPriority());
		netconf::configdataElement priority_configElement(netconf::rule::attribute::PRIORITY, priority_str);
		aclAttributes.push_back(priority_configElement);
	}

	// POLICY GROUP handling
	char policy_group_str[64] = {0};
	::sprintf(policy_group_str, "%s%s", "ManagedElement=1,Ers=1,HwProfile=1,PolicyGroup=", netconf::ITH_POLICYGROUP_ID);
	netconf::configdataElement policy_group_configElement(netconf::rule::attribute::POLICY_GROUP, policy_group_str);
	aclAttributes.push_back(policy_group_configElement);
	/*** End Rule handling ***/


	/*** Start Condition handling ***/
	netconf::configdataList aclCondition;

	// IF NAME handling
	netconf::configdataElement if_name_configElement(netconf::ruleIf::attribute::KEY, "1");
	aclCondition.push_back(if_name_configElement);

	// ADDRESS DST handling
	if (aclEntryInfo.changeMask & imm::aclEntry_attribute::ADDR_DST_CHANGE)
	{
		std::set<std::string> addrDstList = aclEntryInfo.getAddrDst();
		std::string ipv4_addr_dst;
		for (std::set<std::string>::const_iterator it = addrDstList.begin(); addrDstList.end() != it; ++it)
		{
			if(addrDstList.begin() != it)
				ipv4_addr_dst.append(",");
			ipv4_addr_dst.append(*it);
		}
		boost::algorithm::replace_all(ipv4_addr_dst, "..", "-");
		netconf::configdataElement ipv4_addr_dst_configElement(netconf::ruleIf::attribute::IPV4_ADDR_DST, ipv4_addr_dst);
		aclCondition.push_back(ipv4_addr_dst_configElement);
	}

	// ADDRESS SRC handling
	if (aclEntryInfo.changeMask & imm::aclEntry_attribute::ADDR_SRC_CHANGE)
	{
		std::set<std::string> addrSrcList = aclEntryInfo.getAddrSrc();
		std::string ipv4_addr_src;
		for (std::set<std::string>::const_iterator it = addrSrcList.begin(); addrSrcList.end() != it; ++it)
		{
			if(addrSrcList.begin() != it)
				ipv4_addr_src.append(",");
			ipv4_addr_src.append(*it);
		}
		boost::algorithm::replace_all(ipv4_addr_src, "..", "-");
		netconf::configdataElement ipv4_addr_src_configElement(netconf::ruleIf::attribute::IPV4_ADDR_SRC, ipv4_addr_src);
		aclCondition.push_back(ipv4_addr_src_configElement);
	}

	// DSCP handling
	if (aclEntryInfo.changeMask & imm::aclEntry_attribute::DSCP_CHANGE)
	{
		std::set<std::string> dscpList = aclEntryInfo.getDscp();
		std::string ip_dscp;
		for (std::set<std::string>::const_iterator it = dscpList.begin(); dscpList.end() != it; ++it)
		{
			if(dscpList.begin() != it)
				ip_dscp.append(",");
			ip_dscp.append(*it);
		}
		boost::algorithm::replace_all(ip_dscp, "..", "-");
		netconf::configdataElement ip_dscp_configElement(netconf::ruleIf::attribute::IP_DSCP, ip_dscp);
		aclCondition.push_back(ip_dscp_configElement);
	}

	// FRAGMENT TYPE handling
	if (aclEntryInfo.changeMask & imm::aclEntry_attribute::FRAGMENT_TYPE_CHANGE)
	{
		std::string ip_fragment_type_str;
		switch(aclEntryInfo.getFragmentType())
		{
			case 3:
				ip_fragment_type_str = "TRUE";
				break;
			case 4:
				ip_fragment_type_str = "FALSE_OR_FIRST";
				break;
			case 5:
				ip_fragment_type_str = "NOT_FIRST";
				break;
		}
		netconf::configdataElement fragment_type_configElement(netconf::ruleIf::attribute::IP_FRAG, ip_fragment_type_str);
		aclCondition.push_back(fragment_type_configElement);
	}

	// IP PROTOCOL handling
	if (aclEntryInfo.changeMask & imm::aclEntry_attribute::IP_PROTOCOL_CHANGE)
	{
		std::set<std::string> ipProtocolList = aclEntryInfo.getIpProtocol();
		std::string ip_protocol;
		for (std::set<std::string>::const_iterator it = ipProtocolList.begin(); ipProtocolList.end() != it; ++it)
		{
			if(ipProtocolList.begin() != it)
				ip_protocol.append(",");
			ip_protocol.append(*it);
		}
		boost::algorithm::replace_all(ip_protocol, "..", "-");
		netconf::configdataElement ip_protocol_configElement(netconf::ruleIf::attribute::IP_PROTOCOL, ip_protocol);
		aclCondition.push_back(ip_protocol_configElement);
	}

	// PORT DST handling
	if (aclEntryInfo.changeMask & imm::aclEntry_attribute::PORT_DST_CHANGE)
	{
		std::set<std::string> portDstlList = aclEntryInfo.getPortDst();
		std::string l4_port_dst;
		for (std::set<std::string>::const_iterator it = portDstlList.begin(); portDstlList.end() != it; ++it)
		{
			if(portDstlList.begin() != it)
				l4_port_dst.append(",");
			l4_port_dst.append(*it);
		}
		boost::algorithm::replace_all(l4_port_dst, "..", "-");
		netconf::configdataElement port_dst_configElement(netconf::ruleIf::attribute::L4_PORT_DST, l4_port_dst);
		aclCondition.push_back(port_dst_configElement);
	}

	// PORT SRC handling
	if (aclEntryInfo.changeMask & imm::aclEntry_attribute::PORT_SRC_CHANGE)
	{
		std::set<std::string> portSrclList = aclEntryInfo.getPortSrc();
		std::string l4_port_src;
		for (std::set<std::string>::const_iterator it = portSrclList.begin(); portSrclList.end() != it; ++it)
		{
			if(portSrclList.begin() != it)
				l4_port_src.append(",");
			l4_port_src.append(*it);
		}
		boost::algorithm::replace_all(l4_port_src, "..", "-");
		netconf::configdataElement port_src_configElement(netconf::ruleIf::attribute::L4_PORT_SRC, l4_port_src);
		aclCondition.push_back(port_src_configElement);
	}

	// IP VERSION handling
	netconf::configdataElement ip_version_configElement(netconf::ruleIf::attribute::IP_VER, "IPV4");
	aclCondition.push_back(ip_version_configElement);

	aclConditions.push_back(aclCondition);
	/*** End Condition handling ***/


	/*** Start Action handling ***/
	if (aclEntryInfo.changeMask & imm::aclEntry_attribute::ACTION_CHANGE)
	{
		// THEN NAME handling
		netconf::configdataElement then_name_configElement(netconf::ruleThen::attribute::KEY, "1");
		aclAction.push_back(then_name_configElement);

		// ACTION handling
		netconf::configdataElement drop_configElement(netconf::ruleThen::attribute::DROP, "false");
		netconf::configdataElement no_operation_configElement(netconf::ruleThen::attribute::NO_OPERATION, "false");

		bool permitAction = (imm::PERMIT == aclEntryInfo.getAclAction());
		drop_configElement.second			= permitAction ? "false": "true";
		no_operation_configElement.second	= permitAction ? "true"	: "false";

		aclAction.push_back(drop_configElement);
		aclAction.push_back(no_operation_configElement);
	}
	/*** End Action handling ***/
}

void L3_layer_adapter::_build_configdataList (
		netconf::configdataList & config_data,
		const operation::vrrpSessionInfo & info,
		bool use_changemask,
		op_type opType) {
	config_data.clear();
	config_data.reserve(9);

#if 0 /* MAPPED ATTRIBUTE for SMX */
	const char * KEY = "vrrpv3Ipv4SessionId";
	const char * ADMIN_STATE = "adminState";
	const char * OWNER = "owner";
	const char * PRIORITY = "priority";
	const char * VRRP_STATE = "vrrpState"; /* Read Only */
#endif

	// NAME handling
	config_data.push_back
		(netconf::configdataElement(netconf::vrrpv3IPv4Session::attribute::KEY, info.getName()));

	char buf [1024] = {0};

	if ((!use_changemask || (info.changeMask & imm::vrrpSession_attribute::ADMIN_STATE_CHANGE)) &&
			(info.getAdminState() != imm::EMPTY_ADMIN_STATE)) {
		snprintf(buf, sizeof(buf), "%s", adapters_utils::if_adminState_descr(info.getAdminState()));
		config_data.push_back
			(netconf::configdataElement(netconf::vrrpv3IPv4Session::attribute::ADMIN_STATE, buf));
	}

	// the 'owner' attribute of SMX class 'Vrrpv3IPv4Session' is RESTRICTED. So it MUST not be specified
	// in the request towards SMX for MODIFY operation
	if ( (opType != OP_MODIFY) && (!use_changemask || (info.changeMask & imm::vrrpSession_attribute::PRIO_SESSION_CHANGE)) ) {
		snprintf(buf, sizeof(buf), "%s",
				( (info.getPrioritizedSession() == imm::VRRP_TRUE) && (info.getPriority() == 255) ) ? "true" : "false");
		config_data.push_back
			(netconf::configdataElement(netconf::vrrpv3IPv4Session::attribute::OWNER, buf));
	}

	if (!use_changemask || (info.changeMask & imm::vrrpSession_attribute::PRIORITY_CHANGE)) {
		snprintf(buf, sizeof(buf), "%d", info.getPriority());
		config_data.push_back
			(netconf::configdataElement(netconf::vrrpv3IPv4Session::attribute::PRIORITY, buf));
	}

	// Get attributes from the vrrp interface object
	netconf::configdataList vrrp_if_config_data;
	_build_configdataList(vrrp_if_config_data, info.getVrrpInterfaceInfo(),use_changemask, opType);
	if (!vrrp_if_config_data.empty()) {
		config_data.insert(config_data.end(), vrrp_if_config_data.begin(), vrrp_if_config_data.end());
	}
}


void L3_layer_adapter::_build_configdataList (
		netconf::configdataList & config_data,
		const operation::vrrpInterfaceInfo & info,
		bool use_changemask,
		op_type opType) {
	config_data.clear();
	config_data.reserve(4);

	// NOTE: NAME handling: Here RDN name element is not handled because in SMX switch
	// do not exist any configuration element that represent a VRRPv3 interface in IMM

	char buf [1024] = {0};

#if 0 /* MAPPED ATTRIBUTE for SMX */
	const char * VR_ID = "vrId";
	const char * ADV_INTERVAL = "advertiseInterval";
	const char * PREEMPT_MODE = "preemptMode";
	const char * PREEMPT_HOLD_TIME = "preemptHoldTime";
#endif

	// the 'vrid' attribute of SMX class 'Vrrpv3IPv4Session' is RESTRICTED. So it MUST not be specified
	// in the request towards SMX for MODIFY operation
	if ( (opType != OP_MODIFY) && (!use_changemask || (info.changeMask & imm::vrrpInterface_attribute::IDENTITY_CHANGE)) ) {
		snprintf(buf, sizeof(buf), "%d", info.getVrIdentity());
		config_data.push_back
			(netconf::configdataElement(netconf::vrrpv3IPv4Session::attribute::VR_ID, buf));
	}

	if (!use_changemask || (info.changeMask & imm::vrrpInterface_attribute::ADV_INTERVAL_CHANGE)) {
		snprintf(buf, sizeof(buf), "%d", info.getAdvertiseInterval());
		config_data.push_back
			(netconf::configdataElement(netconf::vrrpv3IPv4Session::attribute::ADV_INTERVAL, buf));
	}

	if (!use_changemask || (info.changeMask & imm::vrrpInterface_attribute::PREEMPT_MODE_CHANGE)) {
		snprintf(buf, sizeof(buf), "%s", (info.getPreemptMode() ? "true" : "false"));
		config_data.push_back
			(netconf::configdataElement(netconf::vrrpv3IPv4Session::attribute::PREEMPT_MODE, buf));
	}

	if (!use_changemask || (info.changeMask & imm::vrrpInterface_attribute::PREEMPT_HOLD_TIME_CHANGE)) {
		snprintf(buf, sizeof(buf), "%d", info.getPreemptHoldTime());
		config_data.push_back
			(netconf::configdataElement(netconf::vrrpv3IPv4Session::attribute::PREEMPT_HOLD_TIME, buf));
	}
}


int L3_layer_adapter::_fill_from_configdataList(operation::interfaceInfo & interfaceInfo, const netconf::configdataList & cfgdataList)
{
    for(netconf::configdataList::const_iterator it = cfgdataList.begin(); it != cfgdataList.end(); ++it)
    {
    	const std::string & curr_attr_name = it->first;
    	const std::string & curr_attr_value = it->second;

    	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NAME = %s, VALUE = %s", curr_attr_name.c_str(), curr_attr_value.c_str() );

    	if(curr_attr_name.compare(netconf::interfaceIpv4::attribute::ADMIN_STATE) == 0)
    		interfaceInfo.adminstate = adapters_utils::if_adminState_from_descr_to_enum(curr_attr_value);
    	else if(curr_attr_name.compare(netconf::interfaceIpv4::attribute::BFD_PROFILE) == 0)
    		interfaceInfo.bfdProfile.assign(curr_attr_value);
    	else if(curr_attr_name.compare(netconf::interfaceIpv4::attribute::ENCAPSULATION) == 0)
    		interfaceInfo.encapsulation.assign(curr_attr_value);
    	else if(curr_attr_name.compare(netconf::interfaceIpv4::attribute::KEY) == 0)
    	    interfaceInfo.name.assign(curr_attr_value);
    	else if(curr_attr_name.compare(netconf::interfaceIpv4::attribute::MTU) == 0)
    	    interfaceInfo.mtu = ::strtoul(curr_attr_value.c_str(),0,10);
    	else if(curr_attr_name.compare(netconf::interfaceIpv4::attribute::OPER_STATE) == 0)
    		interfaceInfo.operationalState = adapters_utils::if_operState_from_descr_to_enum(curr_attr_value);
    	else { } // skip current attribute
    }

	return 0;
}

int L3_layer_adapter::_fill_from_configdataList(operation::addressInfo & addressInfo, const netconf::configdataList & cfgdataList)
{
    for(netconf::configdataList::const_iterator it = cfgdataList.begin(); it != cfgdataList.end(); ++it)
    {
    	const std::string & curr_attr_name = it->first;
    	const std::string & curr_attr_value = it->second;

    	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NAME = %s, VALUE = %s", curr_attr_name.c_str(), curr_attr_value.c_str() );

    	if(curr_attr_name.compare(netconf::addressIpv4::attribute::ADDRESS) == 0)
    		addressInfo.address.assign(curr_attr_value);
    	else if(curr_attr_name.compare(netconf::addressIpv4::attribute::KEY) == 0)
    		addressInfo.name.assign(curr_attr_value);
    	else { } // skip current attribute
    }

	return 0;
}
int L3_layer_adapter::_fill_from_configdataList(operation::dstInfo & dstInfo, const netconf::configdataList & cfgdataList)
{
    for(netconf::configdataList::const_iterator it = cfgdataList.begin(); it != cfgdataList.end(); ++it)
    {
    	const std::string & curr_attr_name = it->first;
    	const std::string & curr_attr_value = it->second;

    	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NAME = %s, VALUE = %s", curr_attr_name.c_str(), curr_attr_value.c_str() );

    	if(curr_attr_name.compare(netconf::dst::attribute::DST) == 0)
    		dstInfo.dst.assign(curr_attr_value);
    	else if(curr_attr_name.compare(netconf::interfaceIpv4::attribute::KEY) == 0)
    		dstInfo.name.assign(curr_attr_value);
    	else { } // skip current attribute
    }

	return 0;
}

int L3_layer_adapter::_fill_from_configdataList(operation::bfdSessionInfo & bfdSession, const netconf::configdataList & cfgdataList)
{
   for(netconf::configdataList::const_iterator it = cfgdataList.begin(); it != cfgdataList.end(); ++it)
	{
		const std::string & curr_attr_name = it->first;
		const std::string & curr_attr_value = it->second;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NAME = %s, VALUE = %s", curr_attr_name.c_str(), curr_attr_value.c_str() );

		if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::KEY) == 0)
					bfdSession.name.assign(curr_attr_value);
		else  if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::DETECTION_TIME) == 0)
			bfdSession.detectionTime = ::strtoul(curr_attr_value.c_str(),0,10);
		else if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::DIAGNOSTIC_CODE) == 0)
			bfdSession.diagCode = adapters_utils::bfdSessionDiagnosticCode_from_descr_to_enum(curr_attr_value);
		else if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::MY_DISCRIMINATOR) == 0)
			bfdSession.localDiscriminator = ::strtoul(curr_attr_value.c_str(),0,10);
		else if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::NEGOTIATED_INTERVAL) == 0)
			bfdSession.interval_tx= ::strtoul(curr_attr_value.c_str(),0,10);
		else if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::PEER_ADDRESS) == 0)
			bfdSession.addrDst.assign(curr_attr_value);
		else if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::PEER_INTERFACE) == 0)
			bfdSession.addrSrc.assign(curr_attr_value);
		else if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::STATE) == 0)
			bfdSession.sessionState = adapters_utils::bfdSessionState_from_descr_to_enum(curr_attr_value);
		else if(curr_attr_name.compare(netconf::bfdSessionInfo::attribute::UDP_PORT_SRC) == 0)
			bfdSession.portSrc= ::strtoul(curr_attr_value.c_str(),0,10);
		else { } // skip current attribute
	}

	return 0;
}
int L3_layer_adapter::_fill_from_configdataList(operation::nextHopInfo & nextHopInfo, const netconf::configdataList & cfgdataList)
{
    for(netconf::configdataList::const_iterator it = cfgdataList.begin(); it != cfgdataList.end(); ++it)
    {
    	const std::string & curr_attr_name = it->first;
    	const std::string & curr_attr_value = it->second;

    	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NAME = %s, VALUE = %s", curr_attr_name.c_str(), curr_attr_value.c_str() );

    	if(curr_attr_name.compare(netconf::nextHop::attribute::REFERENCE) == 0)
    	{
    		char hop_dn_buf [8 * 1024] = {0};
    		::memccpy(hop_dn_buf, curr_attr_value.c_str(), 0, sizeof(hop_dn_buf));

    		netconf::rdn_path rdnp(hop_dn_buf);

    		int hop_dn_length = rdnp.split();
    		if (hop_dn_length <= 0)
    		{
    			// ERROR: HOP DN path malformed
    			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
    				"Bad or malformed NextHop dn: hop_dn == <%s>: hop_dn_length == <%d>", curr_attr_value.c_str(), hop_dn_length);
    		}
    		else
    		{
				std::string tmp_str = rdnp[hop_dn_length - 1].value();
				std::replace(tmp_str.begin(),tmp_str.end(),  '_', '.');
				nextHopInfo.address = tmp_str;
			}
       	}
		else if(curr_attr_name.compare(netconf::nextHop::attribute::ADMIN_DISTANCE) == 0)
			nextHopInfo.adminDistance = strtol(curr_attr_value.c_str(),0,10);
		else if(curr_attr_name.compare(netconf::nextHop::attribute::BFD_ENABLE) == 0)
			nextHopInfo.bfdMonitoring = (curr_attr_value == "true" ? true : false);
		else if(curr_attr_name.compare(netconf::nextHop::attribute::DISCARD) == 0)
			nextHopInfo.discard = (curr_attr_value == "true" ? true : false);
		else if(curr_attr_name.compare(netconf::interfaceIpv4::attribute::KEY) == 0)
			nextHopInfo.name.assign(curr_attr_value);
		else { } // skip current attribute
    }

	return 0;
}



int L3_layer_adapter::_fill_from_configdataList (
		operation::vrrpSessionInfo & session_info,
		const netconf::configdataList & cfg_data_list) {

	netconf::configdataList::const_iterator it;

	for (it = cfg_data_list.begin(); it != cfg_data_list.end(); ++it) {
  	const std::string & curr_attr_name = it->first;
  	const std::string & curr_attr_value = it->second;

  	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NAME = %s, VALUE = %s",
  			curr_attr_name.c_str(), curr_attr_value.c_str());

#if 0 /* MAPPED ATTRIBUTE from SMX */
	const char * KEY = "vrrpv3Ipv4SessionId";
	const char * ADMIN_STATE = "adminState";
	const char * OWNER = "owner";
	const char * PRIORITY = "priority";
	const char * VRRP_STATE = "vrrpState"; /* Read Only */
#endif

		if (curr_attr_name == netconf::vrrpv3IPv4Session::attribute::ADMIN_STATE) {
  		session_info.administrativeState =
  				adapters_utils::if_adminState_from_descr_to_enum(curr_attr_value);
  	} else if (curr_attr_name == netconf::vrrpv3IPv4Session::attribute::OWNER) {
  		session_info.prioritizedSession =
  				adapters_utils::if_vrrpvPrioSession_from_descr_to_enum(curr_attr_value);
  	} else if (curr_attr_name == netconf::vrrpv3IPv4Session::attribute::PRIORITY) {
  		session_info.priority = strtoul(curr_attr_value.c_str(), 0, 0);
  	} else if (curr_attr_name == netconf::vrrpv3IPv4Session::attribute::VRRP_STATE) {
  		session_info.vrrpState = adapters_utils::vrrpSessionState_from_descr_to_enum(curr_attr_value);
  	} else { /* SKIP */ }
  }

	_fill_from_configdataList(session_info.interfaceInfo, cfg_data_list);

	return 0;
}


int L3_layer_adapter::_fill_from_configdataList (
		operation::vrrpInterfaceInfo & interface_info,
		const netconf::configdataList & cfg_data_list) {

	netconf::configdataList::const_iterator it;

	for (it = cfg_data_list.begin(); it != cfg_data_list.end(); ++it) {
  	const std::string & curr_attr_name = it->first;
  	const std::string & curr_attr_value = it->second;

  	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NAME = %s, VALUE = %s",
  			curr_attr_name.c_str(), curr_attr_value.c_str());

#if 0 /* MAPPED ATTRIBUTE from SMX */
	const char * VR_ID = "vrId";
	const char * ADV_INTERVAL = "advertiseInterval";
	const char * PREEMPT_MODE = "preemptMode";
	const char * PREEMPT_HOLD_TIME = "preemptHoldTime";
#endif

		if (curr_attr_name == netconf::vrrpv3IPv4Session::attribute::PREEMPT_MODE) {
			interface_info.preemptMode = (strcasecmp(curr_attr_value.c_str(), "TRUE") == 0);
		} else {
			int32_t * field_ptr = 0;

			if (curr_attr_name == netconf::vrrpv3IPv4Session::attribute::VR_ID)
				field_ptr = &interface_info.vrIdentity;
			else if (curr_attr_name == netconf::vrrpv3IPv4Session::attribute::ADV_INTERVAL)
				field_ptr = &interface_info.advertiseInterval;
			else if (curr_attr_name == netconf::vrrpv3IPv4Session::attribute::PREEMPT_HOLD_TIME)
				field_ptr = &interface_info.preemptHoldTime;

			field_ptr && (*field_ptr = strtol(curr_attr_value.c_str(), 0, 0));
		}
  }

	return 0;
}


int L3_layer_adapter::test_operations()
{
	fixs_ith_switchInterfaceAdapter switchInterfaceAdapter;

	// DELETE an IPv4 ADDRESS
	const char ipv4addressDN_delete[] = "addressIPv4Id=primary,interfaceIPv4Id=if_test,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";

	operation::addressInfo addressInfo_delete;

	addressInfo_delete.smxId.assign("15.0.0.0_25");
	addressInfo_delete.name.assign("primary");
	addressInfo_delete.moDN.assign(ipv4addressDN_delete);

	switch_interface::op_result result = switchInterfaceAdapter.delete_addressIPv4(addressInfo_delete);

	// DELETE AN IPV4 INTERFACE
	const char ipv4InterfaceDN_delete[] = "interfaceIPv4Id=if_test,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";

	operation::interfaceInfo interfaceInfo_delete;
	interfaceInfo_delete.smxId.assign("15.0.0.0_25");
	interfaceInfo_delete.name.assign("if_test");
	interfaceInfo_delete.moDN.assign(ipv4InterfaceDN_delete);

	result = switchInterfaceAdapter.delete_interfaceIPv4(interfaceInfo_delete);


	// CREATE AN IPV4 INTERFACE
	const char ipv4InterfaceDN[] = "interfaceIPv4Id=if_test_2,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	netconf::configdataList ipv4Interf_configdata;

	operation::interfaceInfo interfaceInfo;
	interfaceInfo.smxId.assign("15.0.0.0_25");
	interfaceInfo.bfdStaticRoutes = imm::DISABLED;
	interfaceInfo.name.assign("if_test_2");
	interfaceInfo.moDN.assign(ipv4InterfaceDN);
	interfaceInfo.adminstate = imm::UNLOCKED;
	interfaceInfo.mtu = 1500;
	interfaceInfo.encapsulation = "vlanId=900,bridgeId=15.0.0.0_0_T,AxeInfrastructureTransportMinfrastructureTransportMId=1";

	result = switchInterfaceAdapter.create_interfaceIPv4(interfaceInfo);


	// CREATE AN IPV4 Address
	const char ipv4addressDN[] = "addressIPv4Id=address_test_2,interfaceIPv4Id=if_test_2,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";

	operation::addressInfo addressInfo;

	addressInfo.smxId.assign("15.0.0.0_25");
	addressInfo.moDN.assign(ipv4addressDN);
	addressInfo.name.assign("address_test_2");
	addressInfo.address.assign("10.33.24.131/28");

	result = switchInterfaceAdapter.create_addressIPv4(addressInfo);


	// MODIFY AN IPV4 Address
	addressInfo.smxId.assign("15.0.0.0_25");
	addressInfo.moDN.assign(ipv4addressDN);
	addressInfo.name.assign("address_test_2");
	addressInfo.address.assign("10.33.24.133/28");
	addressInfo.changeMask = imm::addressIpv4_attribute::ADDRESS_CHANGE;

	result = switchInterfaceAdapter.modify_addressIPv4(addressInfo);


	// MODIFY ADMIN STATE OF IPV4 INTERFACE
	interfaceInfo.smxId.assign("15.0.0.0_25");
	interfaceInfo.moDN.assign(ipv4InterfaceDN);
	interfaceInfo.adminstate = imm::LOCKED;
	interfaceInfo.changeMask = imm::interfaceIpv4_attribute::ADMIN_STATE_CHANGE;

	result =  switchInterfaceAdapter.modify_interfaceIPv4(interfaceInfo);


	// GET IPv4 Interface Info
	operation::interfaceInfo out_interfaceInfo;
	out_interfaceInfo.smxId.assign("15.0.0.0_25");
	out_interfaceInfo.name.assign("if_test_2");
	out_interfaceInfo.moDN.assign(ipv4InterfaceDN);
	out_interfaceInfo.changeMask = UINT16_MAX;	// we are interested to all info

	result =  switchInterfaceAdapter.get_interfaceIPv4_info(out_interfaceInfo);

	return result;
}


int L3_layer_adapter::test_dst_operations()
{
	//const char DST_1_DN[] = "dstId=1,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	const char DST_2_DN[] = "dstId=zagor,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";

	switch_interface::op_result op_result = switch_interface::SBIA_OK;

	fixs_ith_switchInterfaceAdapter switchInterfaceAdapter;

	operation::dstInfo dstInfo;
	dstInfo.smxId.assign("15.0.0.0_25");
	dstInfo.moDN = DST_2_DN;
	dstInfo.name = "zagor";
	dstInfo.dst = "10.0.0.0/24";
	dstInfo.changeMask = UINT16_MAX;

	// delete DST
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  1) TESTING  'delete' DST operation (DstId == %s) ...", dstInfo.name.c_str());
	op_result = switchInterfaceAdapter.delete_Dst(dstInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  1) TESTING 'delete' DST operation (DstId == %s) ... op_result == %d", dstInfo.name.c_str(), op_result);

	// get DST
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  2) TESTING  'get' DST operation (DstId == %s) ...", dstInfo.name.c_str());
	op_result = switchInterfaceAdapter.get_Dst(dstInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  2) TESTING 'get' DST operation (DstId == %s) ... op_result == %d", dstInfo.name.c_str(), op_result);

	// create DST
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  3) TESTING  'create' DST operation (DstId == %s) ...", dstInfo.name.c_str());
	op_result = switchInterfaceAdapter.create_Dst(dstInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  3) TESTING 'create' DST operation (DstId == %s) ... op_result == %d", dstInfo.name.c_str(), op_result);

	// get DST
	const char DST_3_DN[] = "dstId=Internal,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	operation::dstInfo outDstInfo;
	outDstInfo.smxId.assign("15.0.0.0_25");
	outDstInfo.moDN = DST_3_DN;
	outDstInfo.name = "Internal";
	//outDstInfo.changeMask = imm::dst_attribute::DST_CHANGE;   // ATTENTION PROGRAM CRASH when not commented !!!

	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  4) TESTING  'get' DST operation (DstId == %s) ...", outDstInfo.name.c_str());
	op_result = switchInterfaceAdapter.get_Dst(outDstInfo);

	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  4) TESTING 'get' DST operation (DstId == %s) ... op_result == %d, got DST is '%s'",
			outDstInfo.name.c_str(), op_result, outDstInfo.dst.c_str());

	return 0;
}

int L3_layer_adapter::test_nexthop_operations()
{
	//const char DST_1_DN[] = "dstId=1,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	//const char DST_2_DN[] = "dstId=zagor,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	const char NEXTHOP_DN[] = "nextHopId=PLUTO_NEXTHOP,dstId=zagor,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";


	switch_interface::op_result op_result = switch_interface::SBIA_OK;

	fixs_ith_switchInterfaceAdapter switchInterfaceAdapter;

	operation::nextHopInfo nextHopInfo;
	nextHopInfo.smxId.assign("15.0.0.0_25");
	nextHopInfo.moDN = NEXTHOP_DN;
	nextHopInfo.name = "PLUTO_NEXTHOP";
	nextHopInfo.address = "10.33.23.70";
	nextHopInfo.adminDistance = 2;
	nextHopInfo.bfdMonitoring = true;
	nextHopInfo.changeMask = UINT16_MAX;

	// create NEXTHOP
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  1) TESTING  'create' NEXTHOP operation (nextHopId == %s) ...", nextHopInfo.name.c_str());
	op_result = switchInterfaceAdapter.create_NextHopInfo(nextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  1) TESTING 'create' NEXTHOP operation (nextHopId == %s) ... op_result == %d", nextHopInfo.name.c_str(), op_result);

	// modify NEXTHOP
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  2) TESTING  'modify' NEXTHOP operation (nextHopId == %s) ...", nextHopInfo.name.c_str());
	nextHopInfo.bfdMonitoring = false;
	nextHopInfo.adminDistance = 4;
	nextHopInfo.changeMask = imm::nextHop_attribute::ADMIN_DISTANCE_CHANGE | imm::nextHop_attribute::BFD_MONITORING_CHANGE;
	op_result = switchInterfaceAdapter.modify_NextHopInfo(nextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  2) TESTING 'modify' NEXTHOP operation (nextHopId == %s) ... op_result == %d", nextHopInfo.name.c_str(), op_result);

	// get NEXTHOP
	operation::nextHopInfo outNextHopInfo;
	outNextHopInfo.smxId.assign("15.0.0.0_25");
	outNextHopInfo.moDN = NEXTHOP_DN;
	outNextHopInfo.name = "PLUTO_NEXTHOP";
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  3) TESTING  'get' NEXTHOP operation (nextHopId == %s) ...", outNextHopInfo.name.c_str());
	op_result = switchInterfaceAdapter.get_NextHopInfo(outNextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  3) TESTING 'get' NEXTHOP operation (nextHopId == %s) ... op_result == %d",
			outNextHopInfo.name.c_str(), op_result);
	_log_nexthopInfo(outNextHopInfo);

	// create NEXTHOP
	nextHopInfo.smxId.assign("15.0.0.0_25");
	nextHopInfo.moDN = NEXTHOP_DN;
	nextHopInfo.name = "PLUTO_NEXTHOP";
	nextHopInfo.address = "10.33.23.70";
	nextHopInfo.adminDistance = 4;
	nextHopInfo.bfdMonitoring = false;
	nextHopInfo.changeMask = UINT16_MAX;
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  4) TESTING  'create' NEXTHOP operation (nextHopId == %s) ...", nextHopInfo.name.c_str());
	op_result = switchInterfaceAdapter.create_NextHopInfo(nextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  4) TESTING 'create' NEXTHOP operation (nextHopId == %s) ... op_result == %d", nextHopInfo.name.c_str(), op_result);

	// delete NEXTHOP
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  5) TESTING  'delete' NEXTHOP operation (nextHopId == %s) ...", nextHopInfo.name.c_str());
	op_result = switchInterfaceAdapter.delete_NextHopInfo(nextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  5) TESTING 'delete' NEXTHOP operation (nextHopId == %s) ... op_result == %d", nextHopInfo.name.c_str(), op_result);

	return 0;
}


int L3_layer_adapter::test_vrrp_operations () {
	const char VRRP_SESSION_DN [] =
			"vrrpv3SessionId=xnicmut_01,"
			"interfaceIPv4Id=test,"
			"routerId=15.0.0.0_25:testRouteras,"
			"AxeInfrastructureTransportMinfrastructureTransportMId=1";

	operation::vrrpSessionInfo vrrpSessionInfo;
	vrrpSessionInfo.smxId = "15.0.0.0_25";
	vrrpSessionInfo.moDN = VRRP_SESSION_DN;
	vrrpSessionInfo.name = "xnicmut_01";

	vrrpSessionInfo.administrativeState = imm::LOCKED;
	vrrpSessionInfo.prioritizedSession = imm::VRRP_TRUE;
	vrrpSessionInfo.priority = 128;

	vrrpSessionInfo.interfaceInfo.preemptMode = false;
	vrrpSessionInfo.interfaceInfo.vrIdentity = 0;
	vrrpSessionInfo.interfaceInfo.advertiseInterval = 3;
	vrrpSessionInfo.interfaceInfo.preemptHoldTime = 16;

	// create NEXTHOP
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
			">>>>>>>>>  1) TESTING 'create' VRRp3-SESSION operation (vrrpSessionInfo == %s) ...",
			vrrpSessionInfo.name.c_str());
	op_result test_result = create_vrrpv3Session(vrrpSessionInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG,
			"<<<<<<<<<  1) TESTING 'create' VRRp3-SESSION operation (vrrpSessionInfo == %s) ..."
			" test_result == %d", vrrpSessionInfo.name.c_str(), test_result);



#if 0
	//const char DST_1_DN[] = "dstId=1,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	//const char DST_2_DN[] = "dstId=zagor,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	const char NEXTHOP_DN[] = "nextHopId=PLUTO_NEXTHOP,dstId=zagor,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";

	switch_interface::op_result op_result = switch_interface::SBIA_OK;

	fixs_ith_switchInterfaceAdapter switchInterfaceAdapter;

	operation::nextHopInfo nextHopInfo;
	nextHopInfo.smxId.assign("15.0.0.0_25");
	nextHopInfo.moDN = NEXTHOP_DN;
	nextHopInfo.name = "PLUTO_NEXTHOP";
	nextHopInfo.address = "10.33.23.70";
	nextHopInfo.adminDistance = 2;
	nextHopInfo.bfdMonitoring = true;
	nextHopInfo.changeMask = UINT16_MAX;

	// create NEXTHOP
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  1) TESTING  'create' NEXTHOP operation (nextHopId == %s) ...", nextHopInfo.name.c_str());
	op_result = switchInterfaceAdapter.create_NextHopInfo(nextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  1) TESTING 'create' NEXTHOP operation (nextHopId == %s) ... op_result == %d", nextHopInfo.name.c_str(), op_result);

	// modify NEXTHOP
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  2) TESTING  'modify' NEXTHOP operation (nextHopId == %s) ...", nextHopInfo.name.c_str());
	nextHopInfo.bfdMonitoring = false;
	nextHopInfo.adminDistance = 4;
	nextHopInfo.changeMask = imm::nextHop_attribute::ADMIN_DISTANCE_CHANGE | imm::nextHop_attribute::BFD_MONITORING_CHANGE;
	op_result = switchInterfaceAdapter.modify_NextHopInfo(nextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  2) TESTING 'modify' NEXTHOP operation (nextHopId == %s) ... op_result == %d", nextHopInfo.name.c_str(), op_result);

	// get NEXTHOP
	operation::nextHopInfo outNextHopInfo;
	outNextHopInfo.smxId.assign("15.0.0.0_25");
	outNextHopInfo.moDN = NEXTHOP_DN;
	outNextHopInfo.name = "PLUTO_NEXTHOP";
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  3) TESTING  'get' NEXTHOP operation (nextHopId == %s) ...", outNextHopInfo.name.c_str());
	op_result = switchInterfaceAdapter.get_NextHopInfo(outNextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  3) TESTING 'get' NEXTHOP operation (nextHopId == %s) ... op_result == %d",
			outNextHopInfo.name.c_str(), op_result);

	// create NEXTHOP
	nextHopInfo.smxId.assign("15.0.0.0_25");
	nextHopInfo.moDN = NEXTHOP_DN;
	nextHopInfo.name = "PLUTO_NEXTHOP";
	nextHopInfo.address = "10.33.23.70";
	nextHopInfo.adminDistance = 4;
	nextHopInfo.bfdMonitoring = false;
	nextHopInfo.changeMask = UINT16_MAX;
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  4) TESTING  'create' NEXTHOP operation (nextHopId == %s) ...", nextHopInfo.name.c_str());
	op_result = switchInterfaceAdapter.create_NextHopInfo(nextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  4) TESTING 'create' NEXTHOP operation (nextHopId == %s) ... op_result == %d", nextHopInfo.name.c_str(), op_result);

	// delete NEXTHOP
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  5) TESTING  'delete' NEXTHOP operation (nextHopId == %s) ...", nextHopInfo.name.c_str());
	op_result = switchInterfaceAdapter.delete_NextHopInfo(nextHopInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  5) TESTING 'delete' NEXTHOP operation (nextHopId == %s) ... op_result == %d", nextHopInfo.name.c_str(), op_result);
#endif

	return 0;
}



int L3_layer_adapter::test_bfd_operations()
{
	//const char DST_1_DN[] = "dstId=1,routeTableIPv4StaticId=1,routerId=15.0.0.0_0:PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	const char BFD_PROFILE_DN[] = "bfdProfileId=BFD_PROFILE_2,routerId=15.0.0.0_0:PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";

	switch_interface::op_result op_result = switch_interface::SBIA_OK;

	fixs_ith_switchInterfaceAdapter switchInterfaceAdapter;

	operation::bfdProfileInfo bfdProfileInfo;
	bfdProfileInfo.smxId.assign("15.0.0.0_0");
	bfdProfileInfo.moDN = BFD_PROFILE_DN;
	bfdProfileInfo.name = "BFD_PROFILE_2";
	bfdProfileInfo.detectionMultiplier = 10;
	bfdProfileInfo.intervalRxMinRequired = 1000;
	bfdProfileInfo.intervalTxMinDesired = 1000;
	bfdProfileInfo.changeMask = UINT16_MAX;

//	// delete BFD_PROFILE
//	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  1) TESTING  'delete' BFD_PROFILE operation (bfdProfileId == %s) ...", bfdProfileInfo.name.c_str());
//	op_result = switchInterfaceAdapter.delete_BfdProfile(bfdProfileInfo);
//	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  1) TESTING 'delete' BFD_PROFILE operation (bfdProfileId == %s) ... op_result == %d", bfdProfileInfo.name.c_str(), op_result);

	// create BFD_PROFILE
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  1) TESTING  'create' BFD_PROFILE operation (bfdProfileId == %s) ...", bfdProfileInfo.name.c_str());
	op_result = switchInterfaceAdapter.create_BfdProfile(bfdProfileInfo);
	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  1) TESTING 'create' BFD_PROFILE operation (bfdProfileId == %s) ... op_result == %d", bfdProfileInfo.name.c_str(), op_result);

	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  2) TESTING 'set' BFD_PROFILE attribute of IPV4 Interface");
	// MODIFY ADMIN STATE OF IPV4 INTERFACE
	const char ipv4InterfaceDN[] = "interfaceIPv4Id=if_pluto,routerId=15.0.0.0_0:PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	netconf::configdataList ipv4Interf_configdata;

	operation::interfaceInfo interfaceInfo;
	interfaceInfo.smxId.assign("15.0.0.0_0");
	interfaceInfo.moDN.assign(ipv4InterfaceDN);
	interfaceInfo.name.assign("if_pluto");
	interfaceInfo.bfdStaticRoutes = imm::DISABLED;
	interfaceInfo.bfdProfile.assign(BFD_PROFILE_DN);
	interfaceInfo.changeMask = imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE | imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES_CHANGE;

	op_result =  switchInterfaceAdapter.modify_interfaceIPv4(interfaceInfo);

	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  2) TESTING 'get' BFDSESSIONINFO ");
	// get BFD SESSION
	const char BFD_SESSION_DN[] = "bfdSessionIPv4Id=10.33.24.131,routerId=15.0.0.0_0:PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	operation::bfdSessionInfo bfdSessionInfo;
	bfdSessionInfo.smxId.assign("15.0.0.0_0");
	bfdSessionInfo.moDN = BFD_SESSION_DN;
	bfdSessionInfo.name = "10.33.24.131";

	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, ">>>>>>>>>  4) TESTING  'get' BFDSESSIONINFO operation (bfdSessionIPv4Id == %s) ...", bfdSessionInfo.name.c_str());
	op_result = switchInterfaceAdapter.get_BfdSessionIPv4_info(bfdSessionInfo);

	FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "<<<<<<<<<  4) TESTING 'get' BFDSESSIONINFO  (bfdSessionIPv4Id == %s) ... op_result == %d",
			bfdSessionInfo.name.c_str(), op_result);

	return 0;
}



int L3_layer_adapter::_adjust_routerId_in_imm_dn(std::string & adjusted_dn, const std::string& objectDN)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Before router adjust DN:<%s>", objectDN.c_str());

	adjusted_dn.clear(); // make the output string empty

	// get imm router name from a MO DN,
	// e.g.: objDN: "interfaceIPv4Id=if_test,routerId=1.2.0.4_0:oam,AxeInfrastructureTransportMinfrastructureTransportMId=1"
	//		 will be "1.2.0.4_0:oam"
	std::string immRouterName;
	common::utility::getRDNValue(objectDN, imm::router_attribute::RDN.c_str(), immRouterName);

	// get SMX Router name, e.g: "oam"
	std::string smxRouterName = common::utility::getRouterIdFromRouterName(immRouterName);

	// build the pattern to be replaced (it will be something like "routerId=1.2.0.4_0:oam";
	// we assume that in the input string 'imm_dn' no spaces are present between "routerId" and the character '=')
	std::string pattern(imm::router_attribute::RDN);
	pattern.append(1,'=').append(immRouterName);

	// build the string to replace the pattern
	std::string subst_str(imm::router_attribute::RDN);
	subst_str.append(1,'=').append(smxRouterName);

	// build the output string replacing all occurrences of string 'pattern' with the string 'subst_str'
	adjusted_dn.assign(objectDN);
	boost::algorithm::replace_all(adjusted_dn, pattern, subst_str);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "After router adjust DN:<%s>", adjusted_dn.c_str());

	return 0;
}

void L3_layer_adapter::_adjust_acl_name(const char* aclName, const char* routerName, std::string& aclNameOnSMX)
{
	// ACL name on SMX is routerName_ACLIPv4Name
	aclNameOnSMX.clear();

	aclNameOnSMX.assign(routerName);
	aclNameOnSMX.push_back(common::utility::parserTag::underscore);
	aclNameOnSMX.append(aclName);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "After ACL name adjust <%s>", aclNameOnSMX.c_str());

}

op_result L3_layer_adapter::map_netconfmanager_error(int netconfmanager_error, op_type op_type)
{
	op_result sbia_error = SBIA_ERROR;

	switch(netconfmanager_error)
	{
		case netconf::ERR_NO_ERROR:
			sbia_error = SBIA_OK;
			break;
		case netconf::ERR_OBJ_ALREADY_EXISTS:
			sbia_error = SBIA_OBJ_ALREADY_EXIST;
			break;
		case netconf::ERR_DATA_MISSING:
			sbia_error = (op_type == OP_CREATE ? SBIA_OP_DATA_MISSING: SBIA_OBJ_NOT_FOUND);
			break;
		default:
			sbia_error = SBIA_ERROR;
			break;
	}

	return sbia_error;
}


void L3_layer_adapter::_log_configdataList(const netconf::configdataList & cfgdataList)
{
	int i = 1;
	std::string log_str;
	char mess[1024]={0};
	log_str.append("\n->>>> LOGGING CONFIGDATA LIST:\n");
	for(netconf::configdataList::const_iterator it = cfgdataList.begin(); it != cfgdataList.end(); ++it)
	{
	   	const std::string & curr_attr_name = it->first;
	   	const std::string & curr_attr_value = it->second;

	   	mess[0] = '\0';
	   ::snprintf(mess,1024, "ENTRY #%d : [%s=%s] \n\n", i++, curr_attr_name.c_str(), curr_attr_value.c_str());
	   log_str.append(mess);
	}

   	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "%s", log_str.c_str());
}


void L3_layer_adapter::_log_interfaceInfo(const operation::interfaceInfo & interfaceInfo)
{
	char log_str[4096]={0};
	::snprintf(log_str,4096,
			"\n->>>> LOGGING INTERFACE IPv4 INFO:\n"
            "NAME = %s\n"
   	        "ADMIN_STATE = %d\n"
   	        "BFD_PROFILE = %s\n"
   	        "ENCAPSULATION = %s\n"
   	        "MTU = %d\n"
   	        "OPERATIONAL_STATE = %d\n\n",
			interfaceInfo.name.c_str(),
   	        interfaceInfo.adminstate,
   	        interfaceInfo.bfdProfile.c_str(),
   	        interfaceInfo.encapsulation.c_str(),
   	        interfaceInfo.mtu,
   	        interfaceInfo.operationalState);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "%s", log_str);
}

void L3_layer_adapter::_log_addressInfo(const operation::addressInfo & addressInfo)
{
	char log_str[4096]={0};
	::snprintf(log_str,4096,
			"\n->>>> LOGGING ADDRESS IPv4 INFO:\n"
            "NAME = %s\n"
   	        "ADDRESS = %s\n",
   	        addressInfo.name.c_str(),
			addressInfo.address.c_str()
   	        );

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "%s", log_str);
}

void L3_layer_adapter::_log_nexthopInfo(const operation::nextHopInfo & nextHopInfo)
{
	char log_str[4096]={0};
		::snprintf(log_str,4096,
				"\n->>>> LOGGING NEXTHOP INFO:\n"
	            "NAME = %s\n"
	   	        "ADDRESS = %s\n"
	   	        "ADMIN DISTANCE = %d\n"
	   	        "BFD MONITORING = %s\n"
	   	        "DISCARD = %s\n\n",
				nextHopInfo.name.c_str(),
				nextHopInfo.address.c_str(),
				nextHopInfo.adminDistance,
				(nextHopInfo.bfdMonitoring ? "true": "false"),
				(nextHopInfo.discard ? "true": "false"));

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "%s", log_str);
}

const char * L3_layer_adapter::_op_type_descr(op_type op_type)
{
	switch(op_type)
	{
	case OP_CREATE:
		return "CREATE";
	case OP_DELETE:
		return "DELETE";
	case OP_GET:
		return "GET";
	case OP_MODIFY:
		return "MODIFY";
	default:
		return "UNKNOWN";
	}
}


template<L3_layer_adapter::op_type OP_TYPE, typename OBJECT_INFO>
op_result L3_layer_adapter::_execute_operation(const char * object_type_descr, NETCONF_MANAGER_CREATE_OR_MODIFY_1_FUNC op_func, const OBJECT_INFO & object_info, bool use_merge)
{
	const char * op_type_descr = _op_type_descr(OP_TYPE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
						op_type_descr, object_type_descr,  object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
						use_merge ? "[using MERGE]": "");

	fixs_ith_switchboardinfo switchboard_info;
	FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, _sb_datamanager, op_type_descr, object_type_descr, object_info, switch_interface::SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE);

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, object_info.getDN() );

	netconf::configdataList configdataList;
	_build_configdataList(configdataList, object_info, true, OP_TYPE);
	_log_configdataList(configdataList);	// only for debug purposes !!

	// now use NETCONF Manager to execute the requested operation
	int op_res = ( (fixs_ith::workingSet_t::instance()->get_netconfmanager()).*(op_func))(switchboard_info, adjusted_dn.c_str(), configdataList, use_merge);
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::%s_%s()' returned '%d']",
						op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
						op_type_descr, object_type_descr, op_res);

		return map_netconfmanager_error(op_res, OP_TYPE);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully executed the operation '%s %s' '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	return SBIA_OK;
}


template <>
op_result L3_layer_adapter::_execute_operation
<L3_layer_adapter::OP_CREATE, operation::vrrpSessionInfo> (
		const char * object_type_descr,
		NETCONF_MANAGER_CREATE_OR_MODIFY_1_FUNC op_func,
		const operation::vrrpSessionInfo & object_info,
		bool use_merge) {
	const char * op_type_descr = _op_type_descr(OP_CREATE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Request to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
			op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(),
			object_info.smxId.c_str(), use_merge ? "[using MERGE]" : "");

	fixs_ith_switchboardinfo switchboard_info;
	FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, _sb_datamanager,
			op_type_descr, object_type_descr, object_info,
			switch_interface::SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE);

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, object_info.getDN() );

	netconf::configdataList configdataList;
	_build_configdataList(configdataList, object_info, true, OP_CREATE);
	_log_configdataList(configdataList);	// only for debug purposes !!

	// now use NETCONF Manager to execute the requested operation
	int op_res =
			((fixs_ith::workingSet_t::instance()->get_netconfmanager()).*(op_func))
				(switchboard_info, adjusted_dn.c_str(), configdataList, use_merge);

	if (op_res != netconf::ERR_NO_ERROR) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
				"[Call 'netconf::manager::%s_%s()' returned '%d']",
				op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(),
				object_info.smxId.c_str(), op_type_descr, object_type_descr, op_res);

		return map_netconfmanager_error(op_res, OP_CREATE);
	}

	// Now a ProtVirtIPv4Address configuration element should be created
	char vrrp_ipv4_address_dn [8 * 1024] = {0};
	netconf::configdataList vrrp_ipv4_address_properties;

	// Build the SMX DN used to create the ProtVirtIPv4Address element
	snprintf(vrrp_ipv4_address_dn, sizeof(vrrp_ipv4_address_dn), "%s=1,%s",
			//netconf::protVirtIPv4Address::CLASS_NAME, adjusted_dn.c_str());
			netconf::protVirtIPv4Address::attribute::KEY, adjusted_dn.c_str());

	// Add the address property
	size_t slash_pos = object_info.getVrrpInterfaceInfo().getAddressIPv4().find('/');
	vrrp_ipv4_address_properties.push_back(
			netconf::configdataElement(netconf::protVirtIPv4Address::attribute::ADDRESS,
					object_info.getVrrpInterfaceInfo().getAddressIPv4().substr(0, slash_pos))
	);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Creating the %s config element on the SWITCH BOARD having key '%s': "
			"vrrp_ipv4_address_dn == <%s>: use_merge == <%d>",
			netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(),
			vrrp_ipv4_address_dn, use_merge);

	op_res = (fixs_ith::workingSet_t::instance()->get_netconfmanager()).create_vrrp_IPv4_address
			(switchboard_info, vrrp_ipv4_address_dn, vrrp_ipv4_address_properties, use_merge);

	if (op_res != netconf::ERR_NO_ERROR) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"%s config element creation failed: SWITCH BOARD <%s>: "
				"[Call 'netconf::manager::create_vrrp_IPv4_address()' returned <%d>]",
				netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(), op_res);

		// Do rollback on vrrp session
		(fixs_ith::workingSet_t::instance()->get_netconfmanager()).delete_vrrp_session
				(switchboard_info, adjusted_dn.c_str());

		return map_netconfmanager_error(op_res, OP_CREATE);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Successfully executed the operation '%s %s' '%s' [IMM DN is '%s'] on the "
			"SWITCH BOARD having key '%s'", op_type_descr, object_type_descr, object_info.name.c_str(),
			object_info.moDN.c_str(), object_info.smxId.c_str());

	return SBIA_OK;
}

template<L3_layer_adapter::op_type OP_TYPE, typename OBJECT_INFO>
op_result L3_layer_adapter::_execute_operation(const char * object_type_descr, NETCONF_MANAGER_DELETE_FUNC op_func, const OBJECT_INFO & object_info)
{
	const char * op_type_descr = _op_type_descr(OP_TYPE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
						op_type_descr, object_type_descr,  object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	fixs_ith_switchboardinfo switchboard_info;
	FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, _sb_datamanager, op_type_descr, object_type_descr, object_info, switch_interface::SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE);

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, object_info.getDN() );

	// now use NETCONF Manager to execute the requested operation
	int op_res = ((fixs_ith::workingSet_t::instance()->get_netconfmanager()).*(op_func))(switchboard_info, adjusted_dn.c_str());
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::%s_%s()' returned '%d']",
						op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
						op_type_descr, object_type_descr, op_res);

		return map_netconfmanager_error(op_res, OP_TYPE);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully executed the operation '%s %s' '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	return SBIA_OK;
}


template <>
op_result L3_layer_adapter::_execute_operation
<L3_layer_adapter::OP_DELETE, operation::vrrpSessionInfo> (
		const char * object_type_descr,
		NETCONF_MANAGER_DELETE_FUNC op_func,
		const operation::vrrpSessionInfo & object_info) {
	const char * op_type_descr = _op_type_descr(OP_DELETE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Request to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			op_type_descr, object_type_descr,  object_info.name.c_str(), object_info.moDN.c_str(),
			object_info.smxId.c_str());

	fixs_ith_switchboardinfo switchboard_info;
	FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, _sb_datamanager, op_type_descr,
			object_type_descr, object_info, switch_interface::SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE);

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, object_info.getDN());

	// The ProtVirtIPv4Address configuration element should be deleted
	char vrrp_ipv4_address_dn [8 * 1024] = {0};

	// Build the SMX DN used to delete the ProtVirtIPv4Address element
	std::string protVirtIPv4AddressId = "1";
	snprintf(vrrp_ipv4_address_dn, sizeof(vrrp_ipv4_address_dn), "%s=%s,%s",
			netconf::protVirtIPv4Address::attribute::KEY, protVirtIPv4AddressId.c_str(), adjusted_dn.c_str());

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Getting info on the %s config element on the SWITCH BOARD having key '%s': "
			"vrrp_ipv4_address_dn == <%s>",
			netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(), vrrp_ipv4_address_dn);

	netconf::configdataList configdataList;
	configdataList.push_back(netconf::configdataElement(netconf::protVirtIPv4Address::attribute::KEY, protVirtIPv4AddressId));

	int op_res = (fixs_ith::workingSet_t::instance()->get_netconfmanager()).get_vrrp_IPv4_address
			(configdataList, switchboard_info, vrrp_ipv4_address_dn);

	if (op_res != netconf::ERR_NO_ERROR) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Failure on getting info about %s config element: SWITCH BOARD <%s>: "
				"vrrp_ipv4_address_dn == <%s>: [Call 'netconf::manager::get_vrrp_IPv4_address()' returned <%d>]",
				netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(),
				vrrp_ipv4_address_dn, op_res);

		return map_netconfmanager_error(op_res, OP_DELETE);
	}

	netconf::configdataList::const_iterator address_it = configdataList.begin();
	while ((address_it != configdataList.end()) &&
				 (address_it->first != netconf::protVirtIPv4Address::attribute::ADDRESS)) {
		address_it++;
	}

	if (address_it == configdataList.end()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Failure on getting info about %s config element: SWITCH BOARD <%s>: "
				"vrrp_ipv4_address_dn == <%s>: Address attribute not found",
				netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(),
				vrrp_ipv4_address_dn);

		return map_netconfmanager_error(netconf::ERR_DATA_MISSING, OP_DELETE);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Deleting the %s config element on the SWITCH BOARD having key '%s': "
			"vrrp_ipv4_address_dn == <%s>",
			netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(), vrrp_ipv4_address_dn);

	op_res = (fixs_ith::workingSet_t::instance()->get_netconfmanager()).delete_vrrp_IPv4_address
			(switchboard_info, vrrp_ipv4_address_dn);

	if (op_res != netconf::ERR_NO_ERROR) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"%s config element deletion failed: SWITCH BOARD <%s>: "
				"[Call 'netconf::manager::create_vrrp_IPv4_address()' returned <%d>]",
				netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(), op_res);

		return map_netconfmanager_error(op_res, OP_DELETE);
	}

	// now use NETCONF Manager to execute the requested operation
	op_res =
			((fixs_ith::workingSet_t::instance()->get_netconfmanager()).*(op_func))
				(switchboard_info, adjusted_dn.c_str());
	if (op_res != netconf::ERR_NO_ERROR) {
		int op_res_save = op_res;

		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
				"[Call 'netconf::manager::%s_%s()' returned '%d']",
				op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(),
				object_info.smxId.c_str(), op_type_descr, object_type_descr, op_res);

		// Rollback: recreate the ProtVirtIPv4Address element
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"[ROLLBACK] Re-creating the %s config element on the SWITCH BOARD having key '%s': "
				"vrrp_ipv4_address_dn == <%s>",
				netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(),
				vrrp_ipv4_address_dn);

		op_res = (fixs_ith::workingSet_t::instance()->get_netconfmanager()).create_vrrp_IPv4_address
				(switchboard_info, vrrp_ipv4_address_dn, configdataList, true);

		if (op_res != netconf::ERR_NO_ERROR) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"[ROLLBACK] %s config element creation failed: SWITCH BOARD <%s>: "
					"[Call 'netconf::manager::create_vrrp_IPv4_address()' returned <%d>]",
					netconf::protVirtIPv4Address::CLASS_NAME, object_info.smxId.c_str(), op_res);

			return SBIA_OBJECT_CORRUPTED;
		}

		return map_netconfmanager_error(op_res_save, OP_DELETE);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully executed the operation '%s %s' '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	return SBIA_OK;
}


template<L3_layer_adapter::op_type OP_TYPE, typename OBJECT_INFO>
op_result L3_layer_adapter::_execute_operation(const char * object_type_descr, NETCONF_MANAGER_GET_FUNC op_func, OBJECT_INFO & object_info)
{
	const char * op_type_descr = _op_type_descr(OP_TYPE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
						op_type_descr, object_type_descr,  object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	fixs_ith_switchboardinfo switchboard_info;
	FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, _sb_datamanager, op_type_descr, object_type_descr, object_info, switch_interface::SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE);

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, object_info.getDN() );

	netconf::configdataList configdataList;
	_build_configdataList(configdataList, object_info, true, OP_TYPE);
	_log_configdataList(configdataList);	// only for debug purposes !!

	// now use NETCONF Manager to execute the requested operation
	int op_res = ((fixs_ith::workingSet_t::instance()->get_netconfmanager()).*(op_func))( configdataList, switchboard_info, adjusted_dn.c_str());
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::%s_%s()' returned '%d']",
						op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
						op_type_descr, object_type_descr, op_res);

		return map_netconfmanager_error(op_res, OP_TYPE);
	}

	// fill output parameter
	_fill_from_configdataList(object_info, configdataList);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully executed the operation '%s %s' '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	return SBIA_OK;
}


template<L3_layer_adapter::op_type OP_TYPE, typename OBJECT_INFO>
op_result L3_layer_adapter::_execute_operation(const char * object_type_descr, NETCONF_MANAGER_CREATE_OR_MODIFY_2_FUNC op_func, const OBJECT_INFO & object_info, bool use_merge)
{
	const char * op_type_descr = _op_type_descr(OP_TYPE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
						op_type_descr, object_type_descr,  object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
						use_merge ? "[using MERGE]": "");

	fixs_ith_switchboardinfo switchboard_info;
	FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, _sb_datamanager, op_type_descr, object_type_descr, object_info, switch_interface::SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE);

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, object_info.getDN() );

	netconf::configdataList configdataList;
	_build_configdataList(configdataList, object_info, true, OP_TYPE);
	_log_configdataList(configdataList);	// only for debug purposes !!

	// now use NETCONF Manager to execute the requested operation
	int op_res = ( (fixs_ith::workingSet_t::instance()->get_netconfmanager()).*(op_func))(switchboard_info, adjusted_dn.c_str(), configdataList, use_merge);
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::%s_%s()' returned '%d']",
						op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
						op_type_descr, object_type_descr, op_res);

		return map_netconfmanager_error(op_res, OP_TYPE);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully executed the operation '%s %s' '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	return SBIA_OK;
}

template<L3_layer_adapter::op_type OP_TYPE>
op_result L3_layer_adapter::_execute_acl_create_or_modify_operation (
		const char * object_type_descr,
		NETCONF_MANAGER_CREATE_OR_MODIFY_ACL_FUNC op_func,
		const operation::aclEntryInfo & object_info,
		bool use_merge)
{
	const char * op_type_descr = _op_type_descr(OP_TYPE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s' %s",
			op_type_descr, object_type_descr,  object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
			use_merge ? "[using MERGE]": "");

	fixs_ith_switchboardinfo switchboard_info;
	FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, _sb_datamanager, op_type_descr, object_type_descr, object_info, switch_interface::SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE);

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, object_info.getDN() );

	netconf::configdataList aclAttributes;
	std::vector<netconf::configdataList> aclConditions;
	netconf::configdataList aclAction;

	_build_aclConfigdataList(aclAttributes, aclConditions, aclAction, object_info);

	/*** only for debug purpose ***/
	_log_configdataList(aclAttributes);
	for (std::vector<netconf::configdataList>::iterator it = aclConditions.begin(); it != aclConditions.end(); ++it)
	{
		_log_configdataList(*it);
	}
	_log_configdataList(aclAction);
	/************ end *************/

	// now use NETCONF Manager to execute the requested operation
	int op_res = ( (fixs_ith::workingSet_t::instance()->get_netconfmanager()).*(op_func))(switchboard_info, adjusted_dn.c_str(), aclAttributes, aclConditions, aclAction, use_merge);
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
				"[Call 'netconf::manager::%s_%s()' returned '%d']",
				op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
				op_type_descr, object_type_descr, op_res);

		return map_netconfmanager_error(op_res, OP_TYPE);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully executed the operation '%s %s' '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	return SBIA_OK;
}

template<>
op_result  L3_layer_adapter::_execute_operation<L3_layer_adapter::OP_DELETE, operation::nextHopInfo>
(const char * object_type_descr, NETCONF_MANAGER_DELETE_FUNC /*op_func*/, const operation::nextHopInfo & object_info)
{
	const char * op_type_descr = _op_type_descr(OP_DELETE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
						op_type_descr, object_type_descr,  object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	fixs_ith_switchboardinfo switchboard_info;
	FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, _sb_datamanager, op_type_descr, object_type_descr, object_info, switch_interface::SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE);

	// adjust DN changing the ROUTER RDN value
	std::string adjusted_dn;
	_adjust_routerId_in_imm_dn(adjusted_dn, object_info.getDN() );

	// now use NETCONF Manager to execute the requested operation
	int op_res = fixs_ith::workingSet_t::instance()->get_netconfmanager().delete_next_hop( switchboard_info, adjusted_dn.c_str(), object_info.getAddress());
	if(op_res != netconf::ERR_NO_ERROR)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'. "
						"[Call 'netconf::manager::%s_%s()' returned '%d']",
						op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str(),
						op_type_descr, object_type_descr, op_res);

		return map_netconfmanager_error(op_res, OP_DELETE);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully executed the operation '%s %s' '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'",
			op_type_descr, object_type_descr, object_info.name.c_str(), object_info.moDN.c_str(), object_info.smxId.c_str());

	return SBIA_OK;
}



std::string  L3_layer_adapter::build_null_bfd_profile_dn(const std::string & dn)
{
	std::string parent_dn = common::utility::getParentDN(dn);

	string rdnId = imm::bfdProfile_attribute::RDN + "=null,";

	string bfd_profile_dn = rdnId + parent_dn;

	return bfd_profile_dn;
}


}
