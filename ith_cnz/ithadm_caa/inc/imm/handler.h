/*
 * handler.h
 *
 *  Created on: Feb 22, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_IMM_HANDLER_H_
#define ITHADM_CAA_INC_IMM_HANDLER_H_

#include <boost/noncopyable.hpp>
#include <acs_apgcc_oihandler_V3.h>

#include "imm/bridge_oi.h"
#include "imm/bridgePort_oi.h"
#include "imm/ethernetPort_oi.h"
#include "imm/aggregator_oi.h"
#include "imm/vlan_oi.h"
#include "imm/subnetVlan_oi.h"
#include "imm/router_oi.h"
#include "imm/interfaceipv4_oi.h"
#include "imm/address_oi.h"
#include "imm/dst_oi.h"
#include "imm/nexthop_oi.h"
#include "imm/vrrpInterface_oi.h"
#include "imm/vrrpsession_oi.h"
#include "imm/bfdsession_oi.h"
#include "imm/bfdprofile_oi.h"
#include "imm/brfPersistDataOwner_oi.h"
#include "imm/aclEntryIpv4_oi.h"
#include "imm/aclipv4_oi.h"


namespace imm
{
	/**
	 * @class OI_Handler
	 *
	 * @brief
	 * Manages both initialization and release of IMM related objects.
	 *
	 * @sa Base_OI
	 */
	class OI_Handler : private boost::noncopyable
	{
 	public:

		/// Constructor.
		OI_Handler();

		/// Destructor.
		virtual ~OI_Handler();

		/**
		 * @brief take_ownership performs the initialization of IMM objects
		 * @return int: 0 no errors
		 * @exception	none
		 */
		int take_ownership();

		/**
		 * @brief release_ownership releases IMM objects
		 * @return int: 0 no errors
		 * @exception	none
		 */
		int release_ownership();

		/**
		 * @brief registerAsBrfPartecipant register ITH as BRF Backup Partecipant
		 * in order to receive callbacks
		 * @return int: 0 no errors
		 * @exception	none
		 */
		int registerAsBrfPartecipant();

		/**
		 * @brief unregisterAsBrfPartecipant register ITH as BRF Backup Partecipant
		 * @return int: 0 no errors
		 * @exception	none
		 */
		int unregisterAsBrfPartecipant();

		bool isBackupOngoing();


 	private:
		int registerInImm(Base_OI &oi);
		int registerAsBrfObjectImpl(Base_OI &oi);
		int registerInReactor(Base_OI &oi);

		int unregisterFromImm(Base_OI &oi);
		int unregisterAsBrfObjectImpl(Base_OI &oi);
		int unregisterFromReactor(Base_OI &oi);

 	private:
		/* @brief m_oiHandler handles all Object Implementers */
		acs_apgcc_oihandler_V3 m_oiHandler;

		/* @brief Object Implementer of MOs */
		Bridge_OI		m_bridgeImplementer;
		BridgePort_OI	m_bridgePortImplementer;
		EthernetPort_OI	m_ethernetPortImplementer;
		Aggregator_OI	m_aggregatorImplementer;
		Vlan_OI			m_vlanImplementer;
		SubnetVlan_OI	m_subnetVlanImplementer;
		Router_OI		m_routerImplementer;
		Interfaceipv4_OI m_interfaceIPv4Implementer;
		Address_OI 		m_addressImplementer;
		Dst_OI 			m_dstImplementer;
		NextHop_OI		m_nextHopImplementer;
		VrrpInterface_OI m_vrrpInterfaceImplementer;
		VrrpSession_OI m_vrrpSessionImplementer;
		BfdSession_OI m_bfdSessionIPv4Implementer;
		BfdProfile_OI m_bfdProfileImplementer;
		// Implementer for BRF backup participant
		BrfPersistDataOwner_OI m_brfPersistDataOwnerImplementer;
		AclEntry_OI m_aclEntryImplementer;
		AclIpv4_OI m_aclIpv4Implementer;

	};

} /* namespace imm */



#endif /* ITHADM_CAA_INC_IMM_HANDLER_H_ */
