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

#ifndef ITHADM_CAA_INC_SWITCH_INTERFACE_L3_LAYER_H_
#define ITHADM_CAA_INC_SWITCH_INTERFACE_L3_LAYER_H_

#include "fixs_ith_programconstants.h"
#include "operation/operation.h"


namespace switch_interface
{

	class L3_layer
	{
	public:
		L3_layer() {};

		virtual ~L3_layer() {};

		virtual op_result create_router(const operation::routerInfo & routerInfo, bool useMerge = true) = 0;
		virtual op_result delete_router(const operation::routerInfo & routerInfo) = 0;

		virtual op_result create_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info, bool useMerge = true) = 0;
		virtual op_result delete_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info) = 0;
		virtual op_result modify_interfaceIPv4(const operation::interfaceInfo & interfaceIPv4Info, bool useMerge = true) = 0;
		virtual op_result get_interfaceIPv4_info(operation::interfaceInfo & interfaceIPv4Info) = 0;

		virtual op_result create_addressIPv4(const operation::addressInfo & addressIPv4Info, bool useMerge = true) = 0;
		virtual op_result delete_addressIPv4(const operation::addressInfo & addressIPv4Info) = 0;
		virtual op_result modify_addressIPv4(const operation::addressInfo & addressIPv4Info, bool useMerge = true) = 0;

		virtual op_result create_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo, bool useMerge = true) = 0;
		virtual op_result delete_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo) = 0;
		virtual op_result modify_BfdProfile(const operation::bfdProfileInfo & bfdProfileInfo, bool useMerge = true) = 0;

		virtual op_result get_BfdSessionIPv4_info(operation::bfdSessionInfo & bfdSessionIPv4Info) = 0;

		virtual op_result create_Dst(const operation::dstInfo & dstInfo, bool useMerge = true) = 0;
		virtual op_result delete_Dst(const operation::dstInfo & dstInfo) = 0;
		virtual op_result modify_Dst(const operation::dstInfo & dstInfo, bool useMerge = true) = 0;
		virtual op_result get_Dst(operation::dstInfo & dstInfo) = 0;

		virtual op_result create_NextHopInfo(const operation::nextHopInfo & nextHopInfo, bool useMerge = true) = 0;
		virtual op_result delete_NextHopInfo(const operation::nextHopInfo & nextHopInfo) = 0;
		virtual op_result modify_NextHopInfo(const operation::nextHopInfo & nextHopInfo, bool useMerge = true) = 0;

		virtual op_result create_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo, bool useMerge = true) = 0;
		virtual op_result delete_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo) = 0;
		virtual op_result modify_vrrpv3Session(const operation::vrrpSessionInfo & vrrpSessionInfo, bool useMerge = true) = 0;
		virtual op_result get_vrrpv3Session_info(operation::vrrpSessionInfo & vrrpSessionInfo) = 0;

		virtual op_result enable_Acl(const std::string & switchboard_key) = 0;

		virtual op_result create_AclEntry(const operation::aclEntryInfo & aclEntryInfo, bool useMerge = true) = 0;
		virtual	op_result delete_AclEntry(const operation::aclEntryInfo & aclEntryInfo) = 0;
		virtual	op_result modify_AclEntry(const operation::aclEntryInfo & aclEntryInfo, bool useMerge = true) = 0;

		virtual	op_result delete_AclIpv4(const operation::aclIpV4Info & aclIpV4Info) = 0;
	};

}

#endif /* ITHADM_CAA_INC_SWITCH_INTERFACE_L3_LAYER_H_ */
