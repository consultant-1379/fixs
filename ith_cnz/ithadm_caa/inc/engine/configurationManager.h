/*
 * configurationManager.h
 *
 *  Created on: Mar 1, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_ENGINE__H_
#define ITHADM_CAA_INC_ENGINE__H_

#include "operation/operation.h"
#include "engine/context.h"

#include <map>

#include <boost/noncopyable.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace engine
{
	/**
	 * @class
	 *
	 * @brief Manage all defined data source.
	 *
	 */

	enum ContextLockMode
	{
		EXCLUSIVE_ACCESS = 0,
		SHARED_ACCESS
	};

	enum ContextAccessMode
	{
		GET_EXISTING = 0,
		CREATE_IF_NOT_FOUND
	};

	enum ContextAccessResult
	{
		CONTEXT_ACCESS_ACQUIRED = 0,
		CONTEXT_ACCESS_DENIED,
		CONTEXT_NOT_FOUND
	};

	typedef struct ContextAccess
	{
		friend class ConfigurationManager;
	public:
		/**
		 *	@brief	Get the context with specified creation and lock policy.
		 *
		 *	@param smxId: the switchBoard key.
		 *
		 *	@param accessMode: the access policy to the context: get existing or create new if not found.
		 *
		 *	@param lockMode: the lock policy on the context: exclusive or shared access.
		 *
		 */
		ContextAccess(const std::string& Id, ContextAccessMode accessM, ContextLockMode lockM);
		~ContextAccess();

		Context* getContext() {return context;};

		void destroyContext();

		void setRemovContextOngoing();

		ContextAccessResult getAccessResult() {return accessResult;};

	private:
		Context* context;
		ContextAccessMode accessMode;
		ContextLockMode lockMode;
		ContextAccessResult accessResult;
		std::string smxId;
	} contextAccess_t;

	class ConfigurationManager : private boost::noncopyable
	{
	 friend class ContextAccess;

	 public:

		/// Constructor.
		ConfigurationManager();

		/// Destructor.
		virtual ~ConfigurationManager();

		/**
		 *	@brief
		 */
		fixs_ith::ErrorConstants add(const operation::bridgeInfo& );

		fixs_ith::ErrorConstants add(const operation::bridgePortInfo& );

		fixs_ith::ErrorConstants add(const operation::ethernetPortInfo& );

		fixs_ith::ErrorConstants add(const operation::aggregatorInfo& );

		fixs_ith::ErrorConstants add(const operation::vlanInfo& );

		fixs_ith::ErrorConstants add(const operation::subnetVlanInfo& );

		fixs_ith::ErrorConstants add(const operation::routerInfo& );

		fixs_ith::ErrorConstants add(const operation::interfaceInfo& );

		fixs_ith::ErrorConstants add(const operation::addressInfo& );

		fixs_ith::ErrorConstants add(const operation::dstInfo& );

		fixs_ith::ErrorConstants add(const operation::nextHopInfo& );

		fixs_ith::ErrorConstants add(const operation::vrrpInterfaceInfo& );

		fixs_ith::ErrorConstants add(const operation::vrrpSessionInfo& data);

		fixs_ith::ErrorConstants add(const operation::bfdProfileInfo& data);

		fixs_ith::ErrorConstants add(const operation::bfdSessionInfo& data);

		fixs_ith::ErrorConstants add(const operation::aclEntryInfo& data);

		fixs_ith::ErrorConstants add(const operation::aclIpV4Info& data);


		fixs_ith::ErrorConstants change(const operation::bridgeInfo& );

		fixs_ith::ErrorConstants change(const operation::bridgePortInfo& );

		fixs_ith::ErrorConstants change(const operation::ethernetPortInfo& );

		fixs_ith::ErrorConstants change(const operation::aggregatorInfo& );

		fixs_ith::ErrorConstants change(const operation::vlanInfo& );

		fixs_ith::ErrorConstants change(const operation::subnetVlanInfo& );

		fixs_ith::ErrorConstants change(const operation::interfaceInfo& );

		fixs_ith::ErrorConstants change(const operation::addressInfo& );

		fixs_ith::ErrorConstants change(const operation::dstInfo& );

		fixs_ith::ErrorConstants change(const operation::nextHopInfo& );

		fixs_ith::ErrorConstants change(const operation::vrrpInterfaceInfo& );

		fixs_ith::ErrorConstants change(const operation::vrrpSessionInfo& data);

		fixs_ith::ErrorConstants change(const operation::bfdProfileInfo& data);

		fixs_ith::ErrorConstants change(const operation::aclEntryInfo& data);

		fixs_ith::ErrorConstants change(const operation::aclIpV4Info& data);


		fixs_ith::ErrorConstants remove(const operation::bridgeInfo& );

		fixs_ith::ErrorConstants remove(const operation::bridgePortInfo& );

		fixs_ith::ErrorConstants remove(const operation::ethernetPortInfo& );

		fixs_ith::ErrorConstants remove(const operation::aggregatorInfo& );

		fixs_ith::ErrorConstants remove(const operation::vlanInfo& );

		fixs_ith::ErrorConstants remove(const operation::subnetVlanInfo& );

		fixs_ith::ErrorConstants remove(const operation::routerInfo& );

		fixs_ith::ErrorConstants remove(const operation::interfaceInfo& );

		fixs_ith::ErrorConstants remove(const operation::addressInfo& );

		fixs_ith::ErrorConstants remove(const operation::dstInfo& );

		fixs_ith::ErrorConstants remove(const operation::nextHopInfo& );

		fixs_ith::ErrorConstants remove(const operation::vrrpInterfaceInfo& );

		fixs_ith::ErrorConstants remove(const operation::vrrpSessionInfo& data);

		fixs_ith::ErrorConstants remove(const operation::bfdProfileInfo& data);

		fixs_ith::ErrorConstants remove(const operation::bfdSessionInfo& data);

		fixs_ith::ErrorConstants remove(const operation::aclEntryInfo& data);

		fixs_ith::ErrorConstants remove(const operation::aclIpV4Info& data);


		fixs_ith::ErrorConstants getVrrpInterfaceDNbyVrrpSessionDN(const std::string& vrrpSessionDN, std::string& vrrpInterfaceDN);

		fixs_ith::ErrorConstants getVrrpSessionsOfVrrpInterface(const std::string& vrrpInstanceName, std::set<std::string>& vrrpSessions);

		fixs_ith::ErrorConstants getIPv4AddressOfVrrpInterface(const std::string& vrrpInstanceName, std::string& address);

		fixs_ith::ErrorConstants getIPv4AddressOfInterface(const operation::vrrpSessionInfo& data, std::string& address);

		fixs_ith::ErrorConstants getVrrpSessionInfo(const std::string& vrrpSessionDN, operation::vrrpSessionInfo& data);

		fixs_ith::ErrorConstants getBfdSessionInfo(const std::string& bfdSessionDN, operation::bfdSessionInfo& data);

		fixs_ith::ErrorConstants getInterfacesIPv4ReferencingBfdProfile(const std::string& smxId, const std::string& routerName, const std::string& bfdProfileName, std::set<std::string>& interfaceIPv4DNs);

		fixs_ith::ErrorConstants getInterfaceIPv4byAddress(const operation::addressInfo& address, operation::interfaceInfo& interface);

		fixs_ith::ErrorConstants getInterfacesIPv4ReferencingAclIpv4(const std::string& smxId, const std::string& routerName, const std::string& aclIpv4Name, std::set<std::string>& interfaceIPv4DNs);

		fixs_ith::ErrorConstants getEthPortAutoNegotiateValue(const std::string& smxId, const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& ifIndex, bool& isAutoNegotiateEnabled);

		bool hasNextHopAddress(const std::string& nextHopDn);

		bool isBfdProfileDNValid(const std::string& smxId, const std::string& interfaceDN, const std::string& bfdProfileDN);

		bool isAclIngressDNValid(const std::string& smxId, const std::string& interfaceDN, const std::string& aclIngresseDN);

		/**
		 *	@brief	Loads all defined MOs from IMM at startup
		 *
		 *	@return true on success otherwise false.
		 */
		fixs_ith::ErrorConstants loadConfiguration(const std::string & smxId);

		/**
		 *	@brief	Loads all defined MOs from IMM at SMX restart
		 *
		 *	@return true on success otherwise false.
		 */
		fixs_ith::ErrorConstants reloadConfiguration(const std::string & smxId);

		fixs_ith::ErrorConstants  createPatchDirectory();
		
		fixs_ith::ErrorConstants setAdminStateOfDelayedPorts(const std::string& smxId, imm::admin_State adminstate);

		fixs_ith::ErrorConstants setPimAdaptionOfPorts(const std::string& smxId); //TR_HZ29732

		bool addToReloadInProgressSet(const std::string& smxId);

		bool isReloadInProgress(const std::string& smxId);

		void removeFromReloadInProgressSet(const std::string& smxId);

		std::set<std::string> getContextKeys();

		bool isProtectedObject(const fixs_ith::switchboard_plane_t& plane, const std::string& objName);

		bool hasContext(const std::string& smxId);

		bool isRemoveContextOngoing(const std::string& smxId) const;

		void clear() {m_reloadInProgressSet.clear();};

		bool is_early_enabled_if(const fixs_ith::switchboard_plane_t & plane, const uint16_t  ifIndex);

		fixs_ith::ErrorConstants getSnmpIndexfromBridgePortDN(const std::string objectDN,  uint16_t & ifIndex);


		int getLogsFromCMX(const std::string& smxId,const char * source_trap_ip);

		int deleteOldSNMPLogs(const std::string& smxId);
                //GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.
                bool createVlanEntryInAxeequipment(const std::string vlanName,int vlanTag);
                ACS_CC_ValuesDefinitionType defineAttributeString(const char* attributeName, ACS_CC_AttrValueType type, const char *value, unsigned int numValue);
                ACS_CC_ValuesDefinitionType defineAttributeInt(const char* attributeName, ACS_CC_AttrValueType type, int *value, unsigned int numValue);
		bool replaceVlanIdInAxeequipment(std::string vlanName,const fixs_ith::switchboard_plane_t & plane,int vlanTag);
		bool modifyVlanTag(const char *object, ACS_CC_ImmParameter attributeVlanTag);
                ACS_CC_ImmParameter defineParameterInt(const char* attributeName, ACS_CC_AttrValueType type, int *value, unsigned int numValue);
                //GEP2_GEP5 to GEP7 Dynamic UPD VLAN END

	 private:

		//
		typedef std::map<std::string, Context*> smxMap_t;
		smxMap_t m_smxMap;

		typedef std::multimap<fixs_ith::switchboard_plane_t, std::string> mapOfProtectedObjs_t;

		// List of protected objects
		mapOfProtectedObjs_t m_protectedObjs;

		// MUTEX to access context Map
		boost::recursive_mutex m_contextMapMutex;

		boost::recursive_mutex m_reloadInProgressSetMutex;
		std::set<std::string> m_reloadInProgressSet;

		fixs_ith::ErrorConstants assignTaggedPorts(boost::shared_ptr<Vlan>&, Context*);

		fixs_ith::ErrorConstants assignUntaggedPorts(boost::shared_ptr<Vlan>&, Context*);

		fixs_ith::ErrorConstants assignEthernetPorts(boost::shared_ptr<Aggregator>&, Context*);

		bool getContext(contextAccess_t&, const std::string& smxId);

		void resetContext(const std::string& smxId);

	};
}

#endif /* ITHADM_CAA_INC_ENGINE__H_ */
