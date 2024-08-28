/*
 * configurationHelper.h
 *
 *  Created on: Feb 23, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_IMM_CONFIGURATIONHELPER_H_
#define ITHADM_CAA_INC_IMM_CONFIGURATIONHELPER_H_

#include "fixs_ith_programconstants.h"
#include "operation/operation.h"

#include "imm/bridge_oi.h"
#include "imm/bridgePort_oi.h"
#include "imm/aggregator_oi.h"
#include "imm/ethernetPort_oi.h"
#include "imm/vlan_oi.h"
#include "imm/subnetVlan_oi.h"
#include "imm/router_oi.h"
#include "imm/interfaceipv4_oi.h"
#include "imm/address_oi.h"
#include "imm/dst_oi.h"
#include "imm/nexthop_oi.h"
#include "imm/vrrpInterface_oi.h"
#include "imm/vrrpsession_oi.h"
#include "imm/bfdprofile_oi.h"
#include "imm/bfdsession_oi.h"
#include "imm/aclEntryIpv4_oi.h"
#include "imm/aclipv4_oi.h"

#include <ACS_CC_Types.h>
#include <ace/Singleton.h>
#include <ace/RW_Thread_Mutex.h>
#include <ACS_CC_Types.h>

#include <map>
#include <vector>
#include <boost/thread/recursive_mutex.hpp>

namespace imm
{
	class AclIpv4_OI;

	struct ConfigurationError
	{
		fixs_ith::ErrorConstants errorCode;
		std::string errorMessage;
	};

	enum MOtype_t
	{
		//L2
		BRIDGE_T = 0,
		BRIDGEPORT_T,
		ETHERNETPORT_T,
		AGGREGATOR_T,
		VLAN_T,
		SUBNETVLAN_T,
		//L3
		ROUTER_T,
		INTERFACEIPV4_T,
		ADDRESS_T,
		DST_T,
		NEXTHOP_T,
		VRRPINTERFACE_T,
		VRRPSESSION_T,
		BFDPROFILE_T,
		BFDSESSION_T,
		ACLENTRY_T,
		ACLIPV4_T
	};
	/**
	 * @class ConfigurationHelper
	 *
	 * @brief
	 *  This class helps to manage the data sink configuration
	 *
	 * @sa ACE_Singleton
	 */
	class ConfigurationHelper
	{
	 public:

		friend class ACE_Singleton<ConfigurationHelper, ACE_Recursive_Thread_Mutex>;

		void scheduleCreateOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, const void *moInfo);

		void scheduleModifyOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, const void *moInfo);

		void scheduleDeleteOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, const void *moInfo);

		fixs_ith::ErrorConstants executeMoOperation(const ACS_APGCC_CcbId& configID);

		std::string getErrorText(const fixs_ith::ErrorConstants errorcode);

		fixs_ith::ErrorConstants validate(const operation::aclEntryInfo& info, const uint16_t& changeMask);


	 private:

		typedef std::pair<MOtype_t, const void*> operation_t;

		typedef std::multimap<ACS_APGCC_CcbId, operation_t> mapOfOperation_t;

		///  Constructor.
		ConfigurationHelper();

		///  Destructor.
		virtual ~ConfigurationHelper();

		void cleanMoOperation(const ACS_APGCC_CcbId& configID);

		bool existMoOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType);

		unsigned int countMoOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, mapOfOperation_t&);

		unsigned int countExclusiveMoOperation(const ACS_APGCC_CcbId& configID, const MOtype_t& moType, mapOfOperation_t&);

		bool isMoOperationAllowed(const ACS_APGCC_CcbId& configID);


		fixs_ith::ErrorConstants validateAndExecuteMoOperation(const MOtype_t& moType, const void *moInfo);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const Bridge_OI::Bridge* );

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const BridgePort_OI::BridgePort* );

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const Aggregator_OI::Aggregator* );

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const EthernetPort_OI::EthernetPort* );

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const Vlan_OI::Vlan* );

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const SubnetVlan_OI::SubnetVlan* );

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const Router_OI::Router* );

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const Interfaceipv4_OI::InterfaceIPv4* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const Address_OI::Address* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const Dst_OI::Dst* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const NextHop_OI::NextHop* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const VrrpInterface_OI::VrrpInterface* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const VrrpSession_OI::VrrpSession* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const BfdProfile_OI::BfdProfile* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const BfdSession_OI::BfdSession* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const AclEntry_OI::AclEntry* object);

		fixs_ith::ErrorConstants verifyAndExecuteMoOperation(const AclIpv4_OI::AclIpv4* object);


		fixs_ith::ErrorConstants validate(const operation::bridgePortInfo& info, const uint16_t& changeMask);

		fixs_ith::ErrorConstants validate(const operation::vlanInfo&, const uint16_t& changeMask);

		fixs_ith::ErrorConstants validate(const operation::subnetVlanInfo&, const uint16_t& changeMask);

		fixs_ith::ErrorConstants validate(const operation::interfaceInfo&, const uint16_t& changeMask);

		fixs_ith::ErrorConstants validate(const operation::addressInfo&, const uint16_t& changeMask);

		fixs_ith::ErrorConstants validate(const operation::dstInfo&, const uint16_t& changeMask);

		fixs_ith::ErrorConstants validate(const operation::vrrpInterfaceInfo&, const uint16_t& changeMask);


		fixs_ith::ErrorConstants reset_adminstate_changeMask(operation::bridgePortInfo& info);

		fixs_ith::ErrorConstants findAndExecute(const std::vector<MOtype_t> & listOperations, const std::pair<mapOfOperation_t::iterator, mapOfOperation_t::iterator>& commandRange);

		/// internal map of create configurations
		mapOfOperation_t m_createOperations;

		/// internal map of modify configurations
		mapOfOperation_t m_modifyOperations;

		/// internal map of delete configurations
		mapOfOperation_t m_deleteOperations;

		// Set of operations that cannot be executed in the same transaction
		std::set<MOtype_t> m_exclusiveMoOperations;

		// List of ordered operations for create transaction
		std::vector<MOtype_t> m_createOrderedMoOperations;

		// List of ordered operations for modify transaction
		std::vector<MOtype_t> m_modifyOrderedMoOperations;

		// List of ordered operations for delete transaction
		std::vector<MOtype_t> m_deleteOrderedMoOperations;


		// Mutex to schedule and execute MO operations
		boost::recursive_mutex m_operationMutex;

	};

	typedef ACE_Singleton<ConfigurationHelper, ACE_Recursive_Thread_Mutex> configurationHelper_t;

} /* namespace */


#endif /* ITHADM_CAA_INC_IMM_CONFIGURATIONHELPER_H_ */
