/*
 * handler.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: eanform
 */
#include "imm/handler.h"
#include "imm/imm.h"

#include "fixs_ith_programmacros.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_logger.h"


namespace imm
{

	OI_Handler::OI_Handler()
	: 	m_oiHandler(),
		//L2
		m_bridgeImplementer(imm::moc_name::CLASS_BRIDGE, imm::oi_name::IMPL_BRIDGE),
		m_bridgePortImplementer(imm::moc_name::CLASS_BRIDGE_PORT, imm::oi_name::IMPL_BRIDGEPORT),
		m_ethernetPortImplementer(imm::moc_name::CLASS_ETH_PORT, imm::oi_name::IMPL_ETHERNETPORT),
		m_aggregatorImplementer(imm::moc_name::CLASS_AGGREGATOR, imm::oi_name::IMPL_AGGREGATOR),
		m_vlanImplementer(imm::moc_name::CLASS_VLAN, imm::oi_name::IMPL_VLAN),
		m_subnetVlanImplementer(imm::moc_name::CLASS_SUBNET_VLAN, imm::oi_name::IMPL_SUBNETVLAN),
		//L3
		m_routerImplementer(imm::moc_name::CLASS_ROUTER, imm::oi_name::IMPL_ROUTER),
		m_interfaceIPv4Implementer(imm::moc_name::CLASS_INTERFACE, imm::oi_name::IMPL_INTERFACE),
		m_addressImplementer(imm::moc_name::CLASS_ADDRESS, imm::oi_name::IMPL_ADDRESS),
		m_dstImplementer(imm::moc_name::CLASS_DST, imm::oi_name::IMPL_DST),
		m_nextHopImplementer(imm::moc_name::CLASS_NEXTHOP, imm::oi_name::IMPL_NEXTHOP),
		m_vrrpInterfaceImplementer(imm::moc_name::CLASS_VRRP_INTERFACE, imm::oi_name::IMPL_VRRPINTERFACE),
		m_vrrpSessionImplementer(),
		m_bfdSessionIPv4Implementer(),
		m_bfdProfileImplementer(),
		m_brfPersistDataOwnerImplementer(),
		m_aclEntryImplementer(),
		m_aclIpv4Implementer()
	{

	}

	OI_Handler::~OI_Handler()
	{

	}

	int OI_Handler::registerInImm(Base_OI &oi)
	{
		if ((oi.isImmRegistered() == false) && (ACS_CC_FAILURE == m_oiHandler.addClassImpl(&oi, oi.getIMMClassName())))
		{
			//Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot register %s. See ACS_APGCC.log", oi.getIMMClassName());
			return fixs_ith::ERR_IMM_ADD_OI;
		}
		else
		{
			oi.setImmRegistration();

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Registered Object Implementer for <%s>", oi.getIMMClassName());
			return fixs_ith::ERR_NO_ERRORS;
		}
	}

	int OI_Handler::registerAsBrfObjectImpl(Base_OI &oi)
	{
		if ((oi.isImmRegistered() == false) && (ACS_CC_FAILURE == m_oiHandler.addObjectImpl(&oi)))
		{
			//Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot register %s. See ACS_APGCC.log", oi.getIMMObjectName());
			return fixs_ith::ERR_IMM_ADD_OI;
		}
		else
		{
			oi.setImmRegistration();

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Registered Object Implementer for <%s>", oi.getIMMObjectName());
			return fixs_ith::ERR_NO_ERRORS;
		}
	}


	int OI_Handler::registerInReactor(Base_OI &oi)
	{
		errno = 0;
		if ( (oi.isReatorRegistered() == false) && (fixs_ith::workingSet_t::instance()->get_main_reactor().register_handler(oi.getSelObj(), &oi, ACE_Event_Handler::READ_MASK) < 0))
		{
			//Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'register_handler' failed: cannot register <%s> OI handler into the main reactor", oi.getIMMClassName());
			return fixs_ith::ERR_REACTOR_HANDLER_REGISTER_FAILURE;
		}
		else
		{

			oi.setReactorRegistration();

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "OI <%s> registered into main reactor", oi.getIMMClassName());
			return fixs_ith::ERR_NO_ERRORS;
		}
	}

	int OI_Handler::take_ownership()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");

		//--------------------------------------------------
		// Initialize IMM and set Implementers in the map
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ---- REGISTER IMPLEMENTERS ----");

		int result = fixs_ith::ERR_NO_ERRORS;

		//L2 objs
		result = registerInImm(m_bridgeImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_bridgePortImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_ethernetPortImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_aggregatorImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_vlanImplementer);				if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_subnetVlanImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;

		//L3 objs
		result = registerInImm(m_routerImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_interfaceIPv4Implementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_addressImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_dstImplementer);				if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_nextHopImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_vrrpInterfaceImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_vrrpSessionImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_bfdSessionIPv4Implementer);	if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_bfdProfileImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_aclEntryImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInImm(m_aclIpv4Implementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;

		//----------------------------------------------------------------------------------
		// register Implementers into the main reactor to receive IMM event notifications
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ---- REGISTER INTO REACTOR ----");

		//L2 objs
		result = registerInReactor(m_bridgeImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_bridgePortImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_ethernetPortImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_aggregatorImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_vlanImplementer);				if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_subnetVlanImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;

		//L3 objs
		result = registerInReactor(m_routerImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_interfaceIPv4Implementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_addressImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_dstImplementer);				if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_nextHopImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_vrrpInterfaceImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_vrrpSessionImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_bfdProfileImplementer);		if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_bfdSessionIPv4Implementer);	if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_aclEntryImplementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;
		result = registerInReactor(m_aclIpv4Implementer);			if(fixs_ith::ERR_NO_ERRORS != result) return result;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Out");
		return fixs_ith::ERR_NO_ERRORS;
	}

	int OI_Handler::registerAsBrfPartecipant()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ---- REGISTER AS BRF BACKUP PARTECIPANT ----");
		//# TO BE MOVED To elsewhere
		ACS_APGCC_ScopeT scope = ACS_APGCC_ONE;

		char brfObject_dn[IMM_DN_PATH_SIZE_MAX] = {0};
		::snprintf(brfObject_dn, IMM_DN_PATH_SIZE_MAX, "%s,%s", imm::brfPersistentDataOwner::BRF_OBJECT_NAME.c_str(),imm::brf_mom_dn::BRF_MOC_DN);
       // m_brfPersistDataOwnerImplementer.setIMMObjectName("brfPersistentDataOwnerId=ERIC-APG-FIXS-ITH,brfParticipantContainerId=1");
       // m_brfPersistDataOwnerImplementer.setObjName("brfPersistentDataOwnerId=ERIC-APG-FIXS-ITH,brfParticipantContainerId=1");
        m_brfPersistDataOwnerImplementer.setIMMObjectName(brfObject_dn);
        m_brfPersistDataOwnerImplementer.setObjName(brfObject_dn);
        m_brfPersistDataOwnerImplementer.setScope(scope);
		//#
        int result = fixs_ith::ERR_NO_ERRORS;
		result = registerAsBrfObjectImpl(m_brfPersistDataOwnerImplementer);
		if(fixs_ith::ERR_NO_ERRORS != result) return result;

		//----------------------------------------------------------------------------------
		// register BRF OBJECT Implementer into the main reactor to receive IMM event notifications
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ---- REGISTER BRF IMPLEMENTER INTO REACTOR ----");

		result = registerInReactor(m_brfPersistDataOwnerImplementer);
		if(fixs_ith::ERR_NO_ERRORS != result) return result;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Out");
		return fixs_ith::ERR_NO_ERRORS;
	}



	int OI_Handler::unregisterFromReactor(Base_OI &oi)
	{
		int result = fixs_ith::ERR_NO_ERRORS;

		if(oi.isReatorRegistered())
		{
			errno = 0;
			int reactorResult = fixs_ith::workingSet_t::instance()->get_main_reactor().remove_handler(oi.getSelObj(), ACE_Event_Handler::READ_MASK);
			if(reactorResult < 0)
			{
				// ERROR: Removing the event handler from the reactor
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
							"Call 'remove_handler' failed: trying to remove the OI of MOC:<%s> from the main reactor. "
							"Error:<%d>",  oi.getIMMClassName(),  errno);

				result = fixs_ith::ERR_REACTOR_HANDLER_REMOVE_FAILURE;
			}
			else
			{
				oi.unsetReactorRegistration();
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Handler of MOC:<%s> removed from the main reactor", oi.getIMMClassName());
			}
		}
		return result;
	}

	int OI_Handler::unregisterFromImm(Base_OI &oi)
	{
		int result = fixs_ith::ERR_NO_ERRORS;

		if( oi.isImmRegistered() && ( ACS_CC_FAILURE == m_oiHandler.removeClassImpl(&oi, oi.getIMMClassName()) ) )
		{
			//Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "removeClassImpl failed for MOC:<%s>.", oi.getIMMClassName());
			result = fixs_ith::ERR_IMM_REMOVE_OI;
		}
		else
		{
			oi.unsetImmRegistration();
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "removeClassImpl for MOC:<%s> DONE.", oi.getIMMClassName());
		}

		return result;
	}

	int OI_Handler::unregisterAsBrfObjectImpl(Base_OI &oi)
	{
		int result = fixs_ith::ERR_NO_ERRORS;

		if( oi.isImmRegistered() && ( ACS_CC_FAILURE == m_oiHandler.removeObjectImpl(&oi) ) )
		{
			//Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "removeBrfObjImpl failed for MO:<%s>.", oi.getIMMObjectName());
			result = fixs_ith::ERR_IMM_REMOVE_OI;
		}
		else
		{
			oi.unsetImmRegistration();
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "removeBrfObjImpl for MO:<%s> DONE.", oi.getIMMObjectName());
		}

		return result;
	}


	int OI_Handler::release_ownership()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");

		//--------------------------------------------------------------------------
		// Remove OIs from main reactor
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Remove implementers from main reactor");

		//L2 objs
		int bridgeReactorResult 		=	unregisterFromReactor(m_bridgeImplementer);
		int bridgePortReactorResult 	=	unregisterFromReactor(m_bridgePortImplementer);
		int ethernetPortReactorResult 	=	unregisterFromReactor(m_ethernetPortImplementer);
		int aggregatorReactorResult 	=	unregisterFromReactor(m_aggregatorImplementer);
		int vlanReactorResult 			=	unregisterFromReactor(m_vlanImplementer);
		int subnetVlanReactorResult 	=	unregisterFromReactor(m_subnetVlanImplementer);

		//L3 objs
		int routerReactorResult 		=	unregisterFromReactor(m_routerImplementer);
		int interfaceReactorResult 		=	unregisterFromReactor(m_interfaceIPv4Implementer);
		int addressReactorResult 		=	unregisterFromReactor(m_addressImplementer);
		int dstReactorResult 			=	unregisterFromReactor(m_dstImplementer);
		int nextHopReactorResult 		=	unregisterFromReactor(m_nextHopImplementer);
		int vrrpInterfaceReactorResult	=	unregisterFromReactor(m_vrrpInterfaceImplementer);
		int vrrpSessionReactorResult	=	unregisterFromReactor(m_vrrpSessionImplementer);
		int bfdProfileReactorResult		=	unregisterFromReactor(m_bfdProfileImplementer);
		int bfdSessionReactorResult		=	unregisterFromReactor(m_bfdSessionIPv4Implementer);
		int aclEntryReactorResult		=	unregisterFromReactor(m_aclEntryImplementer);
		int aclIpV4ReactorResult		=	unregisterFromReactor(m_aclIpv4Implementer);

		if (bridgeReactorResult || bridgePortReactorResult || ethernetPortReactorResult || aggregatorReactorResult || vlanReactorResult || subnetVlanReactorResult ||
				routerReactorResult || interfaceReactorResult || addressReactorResult || dstReactorResult || nextHopReactorResult ||
				vrrpInterfaceReactorResult || vrrpSessionReactorResult || bfdSessionReactorResult || bfdProfileReactorResult || aclEntryReactorResult || aclIpV4ReactorResult)
		{
			return fixs_ith::ERR_REACTOR_HANDLER_REMOVE_FAILURE;
		}
		else
		{
			//Success
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "All handlers have successfully been removed from the main reactor");
		}

		//------------------------------------------------------------------------
		// Remove OIs from IMM
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Remove implementers from IMM");

		//L2 objs
		int bridgeImmResult 		=	unregisterFromImm(m_bridgeImplementer);
		int bridgePortImmResult 	=	unregisterFromImm(m_bridgePortImplementer);
		int ethernetPortImmResult 	=	unregisterFromImm(m_ethernetPortImplementer);
		int aggregatorImmResult 	=	unregisterFromImm(m_aggregatorImplementer);
		int vlanImmResult 			=	unregisterFromImm(m_vlanImplementer);
		int subnetVlanImmResult 	=	unregisterFromImm(m_subnetVlanImplementer);

		//L3 objs
		int routerImmResult 		=	unregisterFromImm(m_routerImplementer);
		int interfaceImmResult 		=	unregisterFromImm(m_interfaceIPv4Implementer);
		int addressImmResult 		=	unregisterFromImm(m_addressImplementer);
		int dstImmResult 			=	unregisterFromImm(m_dstImplementer);
		int nextHopImmResult 		=	unregisterFromImm(m_nextHopImplementer);
		int vrrpInterfaceImmResult	=	unregisterFromImm(m_vrrpInterfaceImplementer);
		int vrrpSessionImmResult	=	unregisterFromImm(m_vrrpSessionImplementer);
		int bfdProfileImmResult		=	unregisterFromImm(m_bfdProfileImplementer);
		int bfdSessionImmResult		=	unregisterFromImm(m_bfdSessionIPv4Implementer);
		int aclEntrymmResult		=	unregisterFromImm(m_aclEntryImplementer);
		int aclIpv4Result		=	unregisterFromImm(m_aclIpv4Implementer);

		if (bridgeImmResult || bridgePortImmResult || ethernetPortImmResult || aggregatorImmResult || vlanImmResult || subnetVlanImmResult ||
				routerImmResult || interfaceImmResult || addressImmResult || dstImmResult || nextHopImmResult ||
				  vrrpInterfaceImmResult || vrrpSessionImmResult || bfdSessionImmResult || bfdProfileImmResult || aclEntrymmResult || aclIpv4Result)
		{
			//Error
			return fixs_ith::ERR_IMM_REMOVE_OI;
		}
		else
		{
			//Success
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "All handlers have successfully been removed from main reactor");
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Out");
		return fixs_ith::ERR_NO_ERRORS;
	}

	int OI_Handler::unregisterAsBrfPartecipant()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");

		//--------------------------------------------------------------------------
		// Remove OIs from main reactor
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Remove BRF Object implementer from main reactor");

		int op_result =	unregisterFromReactor(m_brfPersistDataOwnerImplementer);

		if (op_result) return fixs_ith::ERR_REACTOR_HANDLER_REMOVE_FAILURE;

		else  FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRF Object implementer handler has successfully been removed from the main reactor");


		//------------------------------------------------------------------------
		// Remove OI from IMM
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Remove BRF implementer from IMM");

		op_result =	unregisterAsBrfObjectImpl(m_brfPersistDataOwnerImplementer);

		if (op_result ) return fixs_ith::ERR_IMM_REMOVE_OI;

		else FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRF implementer has successfully been unregistered from IMM");

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Out");
		return fixs_ith::ERR_NO_ERRORS;
	}


	bool OI_Handler::isBackupOngoing()
	{
		bool backup_state = m_brfPersistDataOwnerImplementer.isBackupInProgress();

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "%s", (backup_state == true)? "Backup On going":"No backup on going");
		return backup_state;

	}

} /* namespace imm */



