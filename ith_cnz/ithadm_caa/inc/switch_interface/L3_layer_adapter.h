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

#ifndef ITHADM_CAA_INC_SWITCH_INTERFACE_L3_LAYER_ADAPTER_H_
#define ITHADM_CAA_INC_SWITCH_INTERFACE_L3_LAYER_ADAPTER_H_

#include "switch_interface/commonDefs.h"
#include "switch_interface/L3_layer.h"

#include "netconf/netconf.h"
#include "netconf/manager.h"
#include "fixs_ith_sbdatamanager.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"

#define FIXS_ITH_RETRIEVE_SWITCHBOARDSINFO_RETURN_ON_FAILURE(switchboard_info, sb_datamanager, operation, object_type, objectInfo, retval_on_failure)  { 	\
	fixs_ith_sbdatamanager::sbm_constant call_result = sb_datamanager->get_switchboard_info(switchboard_info, objectInfo.getSmxId()); 				\
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)   																							\
	{																																				\
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to %s the %s '%s' [IMM DN is '%s'] on the SWITCH BOARD having key '%s'"								\
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",															\
				operation, object_type, objectInfo.name.c_str(), objectInfo.moDN.c_str(), objectInfo.smxId.c_str(), call_result);					\
		return retval_on_failure;																													\
	}																																				\
 }

namespace switch_interface
{
	class L3_layer_adapter: public L3_layer
	{
	public:
		explicit L3_layer_adapter(fixs_ith_sbdatamanager * sb_datamanager = 0);

		virtual ~L3_layer_adapter() {}

		op_result create_router(const operation::routerInfo & routerInfo, bool useMerge = true);
		op_result delete_router(const operation::routerInfo & routerInfo);

		op_result create_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info, bool useMerge = true);
		op_result delete_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info);
		op_result modify_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info, bool useMerge = true);
		op_result get_interfaceIPv4_info(operation::interfaceInfo & interfaceIPv4Info);

		op_result create_addressIPv4(const operation::addressInfo & addressIPv4Info, bool useMerge = true);
		op_result delete_addressIPv4(const operation::addressInfo & addressIPv4Info);
		op_result modify_addressIPv4(const operation::addressInfo & addressIPv4Info, bool useMerge = true);
		op_result get_addressIPv4(operation::addressInfo & addressIPv4Info);


		op_result create_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo, bool useMerge = true);
		op_result delete_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo);
		op_result modify_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo, bool useMerge = true);

		op_result get_BfdSessionIPv4_info(operation::bfdSessionInfo & bfdSessionIPv4Info) ;

		op_result create_Dst(const operation::dstInfo & dstInfo, bool useMerge = true);
		op_result delete_Dst(const operation::dstInfo & dstInfo);
		op_result modify_Dst(const operation::dstInfo & dstInfo, bool useMerge = true);
		op_result get_Dst(operation::dstInfo & dstInfo);

		op_result create_NextHopInfo(const operation::nextHopInfo & nextHopInfo, bool useMerge = true);
		op_result delete_NextHopInfo(const operation::nextHopInfo & nextHopInfo);
		op_result modify_NextHopInfo(const operation::nextHopInfo & nextHopInfo, bool useMerge = true);
		op_result get_NextHopInfo(operation::nextHopInfo & nextHopInfo);

		op_result create_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo, bool useMerge = true);
		op_result delete_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo);
		op_result modify_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo, bool useMerge = true);
		op_result get_vrrpv3Session_info(operation::vrrpSessionInfo & vrrpSessionInfo);

		op_result enable_Acl(const std::string & switchboard_key);

		op_result create_AclEntry(const operation::aclEntryInfo & aclEntryInfo, bool useMerge = true);
		op_result delete_AclEntry(const operation::aclEntryInfo & aclEntryInfo);
		op_result modify_AclEntry(const operation::aclEntryInfo & aclEntryInfo, bool useMerge = true);

		op_result delete_AclIpv4(const operation::aclIpV4Info & aclIpV4Info);

		// TODO: remove. Only for debug purposes
		int test_operations();
		int test_dst_operations();
		int test_nexthop_operations();
		int test_vrrp_operations ();
		int test_bfd_operations();

	private:

		enum op_type { OP_CREATE, OP_MODIFY, OP_GET, OP_DELETE, OP_UNDEFINED };

		typedef int (netconf::manager::* NETCONF_MANAGER_CREATE_OR_MODIFY_1_FUNC) (const fixs_ith_switchboardinfo & , const char * , const netconf::configdataList &,  bool);
		typedef int (netconf::manager::* NETCONF_MANAGER_DELETE_FUNC) (const fixs_ith_switchboardinfo & , const char *);
		typedef int (netconf::manager::* NETCONF_MANAGER_GET_FUNC) (netconf::configdataList &, const fixs_ith_switchboardinfo & , const char *);
		typedef int (netconf::manager::* NETCONF_MANAGER_CREATE_OR_MODIFY_2_FUNC) (const fixs_ith_switchboardinfo & , const char * , netconf::configdataList &,  bool);
		typedef int (netconf::manager::* NETCONF_MANAGER_CREATE_OR_MODIFY_ACL_FUNC) (const fixs_ith_switchboardinfo & , const char * , const netconf::configdataList & , const std::vector<netconf::configdataList> & , const netconf::configdataList & ,  bool);

	private:

		L3_layer_adapter(const L3_layer_adapter &);
		L3_layer_adapter & operator=(const L3_layer_adapter &);

		template<op_type OP_TYPE, typename OBJECT_INFO>
		op_result _execute_operation(const char * object_type_descr, NETCONF_MANAGER_CREATE_OR_MODIFY_1_FUNC op_func, const OBJECT_INFO & object_info, bool use_merge = true);

		template<op_type OP_TYPE, typename OBJECT_INFO>
		op_result _execute_operation(const char * object_type_descr, NETCONF_MANAGER_DELETE_FUNC op_func, const OBJECT_INFO & object_info);

		template<op_type OP_TYPE, typename OBJECT_INFO>
		op_result _execute_operation(const char * object_type_descr, NETCONF_MANAGER_GET_FUNC op_func, OBJECT_INFO & object_info);

		template<op_type OP_TYPE, typename OBJECT_INFO>
		op_result _execute_operation(const char * object_type_descr, NETCONF_MANAGER_CREATE_OR_MODIFY_2_FUNC op_func, const OBJECT_INFO & object_info, bool use_merge = true);

		template<op_type OP_TYPE>
		op_result _execute_acl_create_or_modify_operation (
				const char * object_type_descr,
				NETCONF_MANAGER_CREATE_OR_MODIFY_ACL_FUNC op_func,
				const operation::aclEntryInfo & object_info,
				bool use_merge = true);

		static void _build_configdataList(netconf::configdataList & cfgdataList, const operation::interfaceInfo & interfaceInfo, bool use_changemask = true, op_type opType = OP_CREATE);
		static void _build_configdataList(netconf::configdataList & cfgdataList, const operation::addressInfo & addressInfo, bool use_changemask = true, op_type opType = OP_CREATE);
		static void _build_configdataList(netconf::configdataList & cfgdataList, const operation::dstInfo & dstInfo, bool use_changemask = true, op_type opType = OP_CREATE);
		static void _build_configdataList(netconf::configdataList & cfgdataList, const operation::nextHopInfo & nextHopInfo, bool use_changemask = true, op_type opType = OP_CREATE);
		void _build_configdataList(
				netconf::configdataList & config_data,
				const operation::vrrpSessionInfo & info,
				bool use_changemask = true,
				op_type opType = OP_CREATE);

		void _build_configdataList(
				netconf::configdataList & config_data,
				const operation::vrrpInterfaceInfo & info,
				bool use_changemask = true,
				op_type op_type = OP_CREATE);

		void _build_configdataList(netconf::configdataList & cfgdataList, const operation::bfdProfileInfo & bfdProfileInfo, bool use_changemask = true, op_type opType = OP_CREATE);
		void _build_configdataList(netconf::configdataList & configdataList, const operation::bfdSessionInfo & bfdSessionInfo, bool use_changemask= true, op_type opType = OP_CREATE);

		void _build_aclConfigdataList(netconf::configdataList & aclAttributes,
				std::vector<netconf::configdataList> & aclConditions,
				netconf::configdataList & aclAction,
				const operation::aclEntryInfo & aclEntryInfo);

		//TODO : only for test purposes
		static void _log_configdataList(const netconf::configdataList & cfgdataList);
		static void _log_interfaceInfo(const operation::interfaceInfo & interfaceInfo);
		static void _log_nexthopInfo(const operation::nextHopInfo & nextHopInfo);
		static void _log_addressInfo(const operation::addressInfo & addressInfo);

		static int _fill_from_configdataList(operation::interfaceInfo & interfaceInfo, const netconf::configdataList & cfgdataList);
		static int _fill_from_configdataList(operation::dstInfo & dstInfo, const netconf::configdataList & cfgdataList);
		static int _fill_from_configdataList(operation::nextHopInfo & nextHopInfo, const netconf::configdataList & cfgdataList);
		static int _fill_from_configdataList (
				operation::vrrpSessionInfo & session_info,
				const netconf::configdataList & cfg_data_list);
		static int _fill_from_configdataList (
				operation::vrrpInterfaceInfo & interface_info,
				const netconf::configdataList & cfg_data_list);
		int _fill_from_configdataList(operation::addressInfo & addressInfo, const netconf::configdataList & cfgdataList);
		int _fill_from_configdataList(operation::bfdSessionInfo & bfdSession, const netconf::configdataList & cfgdataList);

		int _adjust_routerId_in_imm_dn(std::string & adjusted_dn, const std::string& objectDN);

		void _adjust_acl_name(const char* aclName, const char* routerName, std::string& aclNameOnSMX);

		static const char * _op_type_descr(op_type op_type);

		static op_result map_netconfmanager_error(int netconfmanager_error, op_type op_type = OP_UNDEFINED);

		static std::string build_null_bfd_profile_dn(const std::string & dn);
		fixs_ith_sbdatamanager * _sb_datamanager;
	};

	template <>
	op_result L3_layer_adapter::_execute_operation
	<L3_layer_adapter::OP_CREATE, operation::vrrpSessionInfo> (
			const char * object_type_descr,
			NETCONF_MANAGER_CREATE_OR_MODIFY_1_FUNC op_func,
			const operation::vrrpSessionInfo & object_info,
			bool use_merge);

	template <>
	op_result L3_layer_adapter::_execute_operation
	<L3_layer_adapter::OP_DELETE, operation::vrrpSessionInfo> (
			const char * object_type_descr,
			NETCONF_MANAGER_DELETE_FUNC op_func,
			const operation::vrrpSessionInfo & object_info);

	template<>
	op_result  L3_layer_adapter::_execute_operation<L3_layer_adapter::OP_DELETE, operation::nextHopInfo>(const char * object_type_descr, NETCONF_MANAGER_DELETE_FUNC op_func, const operation::nextHopInfo & object_info);
}

#endif /* ITHADM_CAA_INC_SWITCH_INTERFACE_L3_LAYER_ADAPTER_H_ */
