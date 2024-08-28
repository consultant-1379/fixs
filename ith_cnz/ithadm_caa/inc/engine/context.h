/*
 * context.h
 *
 *  Created on: Mar 15, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_ENGINE_CONTEXT_H_
#define ITHADM_CAA_INC_ENGINE_CONTEXT_H_


#include <string>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "engine/aggregator.h"
#include "engine/bridge.h"
#include "engine/bridgeport.h"
#include "engine/ethernetport.h"
#include "engine/subnetvlan.h"
#include "engine/vlan.h"

#include "engine/router.h"
#include "engine/interfaceipv4.h"
#include "engine/address.h"
#include "engine/dst.h"
#include "engine/nexthop.h"
#include "engine/vrrpinterface.h"

#include "switch_interface/full_adapter.h"


namespace engine
{
	class VrrpSession;
	class BfdSessionIPv4;
	class BfdProfile;
	class AclEntryIpv4;
	class AclIpv4;

	enum
		{
			bridge_idx = 0,
			ethernetPort_idx,
			aggregator_idx,
			bridgePort_idx,
			vlan_idx,
			subnetVlan_idx,

			router_idx,
			interface_idx,
			address_idx,
			dst_idx,
			nextHop_idx,
			vrrpInterface_idx,
			vrrpSession_idx,
			bfdProfile_idx,
			bfdSession_idx,
			aclEntry_idx,
			aclIpv4_idx,

			//Keep as last element
			mutex_number
		};

	struct context_shared_mutex
	{
		context_shared_mutex(): exclusive(false), shared_count(0) {};
		~context_shared_mutex() {};

		boost::recursive_mutex state_change;
		boost::recursive_mutex lock;
		bool exclusive;
		uint16_t shared_count;

		bool try_lock_shared()
		{
			boost::recursive_mutex::scoped_lock lk(state_change);

			bool locked = lock.try_lock();
			if (locked)
			{
				shared_count++;
				lock.unlock();
			}

			return locked;
		}

		bool try_lock()
		{
			boost::recursive_mutex::scoped_lock lk(state_change);
			if (shared_count)
			{
				return false;
			}
			else
			{
				exclusive = lock.try_lock();
				return exclusive;
			}
		}

		bool is_locked() {return exclusive;}

		void unlock()
		{
			boost::recursive_mutex::scoped_lock lk(state_change);
			exclusive=false;
			lock.unlock();
		}

		void unlock_shared()
		{
			boost::recursive_mutex::scoped_lock lk(state_change);
			--shared_count;
			//lock.unlock();
		}
	};


	class Context : private boost::noncopyable
	{
		friend class ConfigurationManager;
		friend class ContextAccess;

	public:

		/// Constructor
		Context(const std::string& smxId);

		/// Destructor
		virtual ~Context();

		fixs_ith::ErrorConstants setBridge(boost::shared_ptr<Bridge>&);

		fixs_ith::ErrorConstants setBridgePort(boost::shared_ptr<BridgePort>&);

		fixs_ith::ErrorConstants setEthernetPort(boost::shared_ptr<EthernetPort>&);

		fixs_ith::ErrorConstants setAggregator(boost::shared_ptr<Aggregator>&);

		fixs_ith::ErrorConstants setVlan(boost::shared_ptr<Vlan>&);

		fixs_ith::ErrorConstants setSubnetVlan(boost::shared_ptr<SubnetVlan>&);

		fixs_ith::ErrorConstants setRouter(boost::shared_ptr<Router>&);

		fixs_ith::ErrorConstants setInterface(boost::shared_ptr<InterfaceIPv4>&);

		fixs_ith::ErrorConstants setRouterInterfaceAddress(boost::shared_ptr<Address>&);

		fixs_ith::ErrorConstants setVrrpSession(boost::shared_ptr<VrrpSession>& newVrrpSession);

		fixs_ith::ErrorConstants setDst(boost::shared_ptr<Dst>&);

		fixs_ith::ErrorConstants setNextHop(boost::shared_ptr<NextHop>&);

		fixs_ith::ErrorConstants setVrrpInterface(boost::shared_ptr<VrrpInterface>&);

		fixs_ith::ErrorConstants setVrrpInterfaceAddress(boost::shared_ptr<Address>&);

		fixs_ith::ErrorConstants setBfdProfile(boost::shared_ptr<BfdProfile>&);

		fixs_ith::ErrorConstants setBfdSessionIPv4(boost::shared_ptr<BfdSessionIPv4>& newBfdSession);

		fixs_ith::ErrorConstants setAclEntryIpv4(boost::shared_ptr<AclEntryIpv4>& newAclEntry);

		fixs_ith::ErrorConstants setAclIpV4(boost::shared_ptr<AclIpv4>& newAclIpV4);

		bool hasBridge(const fixs_ith::SwitchBoardPlaneConstants&);

		bool hasVlan(const fixs_ith::SwitchBoardPlaneConstants&, const uint16_t& );

		bool hasVlan(const fixs_ith::SwitchBoardPlaneConstants&, const std::string& );

		bool hasSubnetBasedVlan(const fixs_ith::SwitchBoardPlaneConstants&, const uint16_t& );

		bool hasSubnetBasedVlan(const fixs_ith::SwitchBoardPlaneConstants&, const std::string& );

		bool hasIpv4SubnetBasedVlan(const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& subnet);
		
		bool IsUntaggedPort (const fixs_ith::SwitchBoardPlaneConstants& plane, const std::string& portDN, bool loadRuntimeInfo= false); //TR HX22060

		boost::shared_ptr<Bridge> getBridgeByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&);

		boost::shared_ptr<BridgePort> getBridgePortByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&, bool loadRuntimeInfo = false);

		boost::shared_ptr<EthernetPort> getEthernetPortByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&, bool loadRuntimeInfo = false);

		boost::shared_ptr<Aggregator> getAggregatorByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&, bool loadRuntimeInfo = false);

		boost::shared_ptr<Vlan> getVlanByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&, bool loadRuntimeInfo = false);

		boost::shared_ptr<SubnetVlan> getSubnetVlanByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&, bool loadRuntimeInfo = false);

		boost::shared_ptr<Router> getRouterByName(const std::string&);

		boost::shared_ptr<InterfaceIPv4> getInterfaceByName(const std::string routerName, const std::string& name, uint16_t runtimeAttributeMask = 0);

		boost::shared_ptr<VrrpSession> getVrrpSessionByName(const std::string& routerName, const std::string& interfaceName, const std::string& vrrpSessionName);

		boost::shared_ptr<VrrpSession> getVrrpSessionByDN(const std::string& vrrpSessionDN);

		boost::shared_ptr<Address> getRouterInterfaceAddressByName(const std::string& routerName, const std::string interfaceName, const std::string& name);

		boost::shared_ptr<Address> getRouterInterfaceAddress(const std::string& routerName, const std::string interfaceName);

		boost::shared_ptr<Dst> getDstByName(const std::string routerName, const std::string& name);

		boost::shared_ptr<NextHop> getNextHopByName(const std::string& routerName, const std::string dstName, const std::string& name);

		boost::shared_ptr<VrrpInterface> getVrrpInterfaceByName(const std::string& name);

		boost::shared_ptr<VrrpInterface> getVrrpInterfaceById(const int32_t& vrId);

		boost::shared_ptr<Address> getVrrpInterfaceAddressByName(const std::string& vrrpInterfaceName, const std::string& name);

		boost::shared_ptr<Address> getVrrpInterfaceAddress(const std::string& vrrpInterfaceName);

		boost::shared_ptr<Vlan> getVlanById(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& vlanId, bool loadRuntimeInfo = false);

		boost::shared_ptr<SubnetVlan> getSubnetVlanById(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& vlanId, bool loadRuntimeInfo = false);

		boost::shared_ptr<BridgePort> getBridgePortByIndex(const fixs_ith::SwitchBoardPlaneConstants&, const uint16_t&, bool loadRuntimeInfo = false);

		boost::shared_ptr<EthernetPort> getEthernetPortByIndex(const fixs_ith::SwitchBoardPlaneConstants&, const uint16_t&, bool loadRuntimeInfo = false);

		boost::shared_ptr<Aggregator> getAggregatorByIndex(const fixs_ith::SwitchBoardPlaneConstants&, const uint16_t&, bool loadRuntimeInfo = false);

		std::set< boost::shared_ptr<Vlan> > getVlans(const fixs_ith::SwitchBoardPlaneConstants& plane);

		std::set< boost::shared_ptr<BridgePort> > getBridgePorts(const fixs_ith::SwitchBoardPlaneConstants& plane);

		std::set< boost::shared_ptr<Aggregator> > getAggregators(const fixs_ith::SwitchBoardPlaneConstants& plane);

		std::set< boost::shared_ptr<InterfaceIPv4> > getInterfaces();

		std::set< boost::shared_ptr<InterfaceIPv4> > getInterfaces(const std::string& routerName);

		std::set< boost::shared_ptr<Address> > getRouterInterfaceAddresses(const std::string& routerName);

		std::set< boost::shared_ptr<Dst> > getDsts(const std::string& routerName);

		std::set<std::string> getNextHopsAddressWithBFDEnabled(const std::string& routerName);

		std::set<std::string> getNextHopsAddress(const std::string& routerName);

		std::set< uint16_t > getBridgePortsIndexes(const fixs_ith::SwitchBoardPlaneConstants& plane);

		std::set< uint16_t > getEthernetPortsIndexes(const fixs_ith::SwitchBoardPlaneConstants& plane);

		std::set< boost::shared_ptr<VrrpSession> > getVrrpSessions(const std::string& vrrpInterfaceName);

		fixs_ith::ErrorConstants getVrrpSessionInfo(const std::string& vrrpSessionDN, operation::vrrpSessionInfo& data);

		boost::shared_ptr<BfdProfile> getBfdProfileByName(const std::string& routerName, const std::string& name);

		boost::shared_ptr<BfdSessionIPv4> getBfdSessionIPv4ByName(const std::string& routerName, const std::string& name);

		fixs_ith::ErrorConstants getBfdSessionIPv4Info(const std::string& bfdSessionDN, operation::bfdSessionInfo& data);

		std::set<std::string> getAddressOfBfdSessions(const std::string& routerName);

		std::set<std::string> getAddressOfInterfaceWithBFD(const std::string& routerName);

		std::string getNextHopAddress(const std::string& routerName, const std::string& dstName, const std::string& nextHopName);

		boost::shared_ptr<AclEntryIpv4> getAclEntryIpv4ByName(const std::string& routerName, const std::string& name);

		boost::shared_ptr<AclIpv4> getAclIpV4ByName(const std::string& routerName, const std::string& name);

		fixs_ith::ErrorConstants resetBridgeByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&);

		fixs_ith::ErrorConstants resetBridgePortByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&);

		fixs_ith::ErrorConstants resetEthernetPortByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&);

		fixs_ith::ErrorConstants resetAggregatorByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&);

		fixs_ith::ErrorConstants resetVlanByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&);

		fixs_ith::ErrorConstants resetSubnetVlanByName(const fixs_ith::SwitchBoardPlaneConstants&, const std::string&);

		fixs_ith::ErrorConstants resetRouterByName(const std::string&);

		fixs_ith::ErrorConstants resetInterfaceByName(const std::string& routerName, const std::string& name);

		fixs_ith::ErrorConstants resetVrrpSessionByName(const std::string& routerName, const std::string interfaceName, const std::string& vrrpSessionName);

		fixs_ith::ErrorConstants resetVrrpSessionByDN(const std::string& vrrpSessionDN);

		fixs_ith::ErrorConstants resetRouterInterfaceAddressByName(const std::string& routerName, const std::string interfaceName, const std::string& name);

		fixs_ith::ErrorConstants resetDstByName(const std::string& routerName, const std::string& name);

		fixs_ith::ErrorConstants resetNextHopByName(const std::string& routerName, const std::string dstName, const std::string& name);

		fixs_ith::ErrorConstants resetVrrpInterfaceByName(const std::string& name);

		fixs_ith::ErrorConstants resetVrrpInterfaceAddressByName(const std::string interfaceName, const std::string& name);

		fixs_ith::ErrorConstants resetBfdSessionIPv4ByName(const std::string& routerName, const std::string& name);

		fixs_ith::ErrorConstants resetBfdProfileByDN(const std::string& bfdProfileDN);

		fixs_ith::ErrorConstants resetBfdProfileByName(const std::string& routerName, const std::string& bfdProfileName);

		fixs_ith::ErrorConstants resetAclEntryByName(const std::string& routerName, const std::string& aclEntryName);

		fixs_ith::ErrorConstants resetAclIpV4ByName(const std::string& routerName, const std::string& aclIpV4Name);

		fixs_ith::ErrorConstants replaceBridge(boost::shared_ptr<Bridge>&);

		fixs_ith::ErrorConstants replaceBridgePort(boost::shared_ptr<BridgePort>&);

		fixs_ith::ErrorConstants replaceEthernetPort(boost::shared_ptr<EthernetPort>&);

		fixs_ith::ErrorConstants replaceAggregator(boost::shared_ptr<Aggregator>& );

		fixs_ith::ErrorConstants replaceVlan(boost::shared_ptr<Vlan>& );

		fixs_ith::ErrorConstants replaceSubnetVlan(boost::shared_ptr<SubnetVlan>& );

		fixs_ith::ErrorConstants replaceInterface(boost::shared_ptr<InterfaceIPv4>&, const operation::interfaceInfo& );

		fixs_ith::ErrorConstants replaceVrrpSession(boost::shared_ptr<VrrpSession>& newVrrpSession);

		fixs_ith::ErrorConstants replaceRouterInterfaceAddress(boost::shared_ptr<Address>&, const operation::addressInfo& );

		fixs_ith::ErrorConstants replaceDst(boost::shared_ptr<Dst>&, const operation::dstInfo& );

		fixs_ith::ErrorConstants replaceNextHop(boost::shared_ptr<NextHop>&, const operation::nextHopInfo& );

		fixs_ith::ErrorConstants replaceVrrpInterface(boost::shared_ptr<VrrpInterface>& , const operation::vrrpInterfaceInfo& );

		fixs_ith::ErrorConstants replaceVrrpInterfaceAddress(boost::shared_ptr<Address>&, const operation::addressInfo& );

		fixs_ith::ErrorConstants replaceBfdProfile(boost::shared_ptr<BfdProfile>&, const operation::bfdProfileInfo& );

		fixs_ith::ErrorConstants replaceBfdSessionIPv4(boost::shared_ptr<BfdSessionIPv4>& newBfdSession);

		fixs_ith::ErrorConstants replaceAclEntryIpv4(boost::shared_ptr<AclEntryIpv4>& newAclEntry, const operation::aclEntryInfo& );

		fixs_ith::ErrorConstants replaceAclIpv4(boost::shared_ptr<AclIpv4>& newAclIpv4);

		fixs_ith::ErrorConstants checkVrIdentityUniqueness(int32_t vrIdentity);

		fixs_ith::ErrorConstants checkAclEntryPriorityUniqueness(const std::string& routerName, const std::string& aclIpv4Name, const std::string& aclEntryName, const uint16_t priority);

		const bool checkRouterNextHopsAddress(const std::string& routerName, const std::string& address);

		const bool isAggregatorMember(const fixs_ith::SwitchBoardPlaneConstants&, const uint16_t& snmpIndex);

		const bool isAggregatedPort(const fixs_ith::SwitchBoardPlaneConstants& , const uint16_t& snmpIndex);

		const bool isEncapsulation(const std::string& encapsulationDN);

		const bool hasLoopback(const std::string& routerName);

		const bool hasDst(const std::string& routerName, const std::string& dst);

		const bool hasRouterInterfaceAddress(const std::string& routerName, const std::string& address);

		const bool hasConflictingRouterInterfaceAddress(const std::string& routerName, const std::string& address);

		bool hasVrrpSession(const std::string& routerName, const std::string& interfaceName);

		bool hasVrrpSessionOwner(const std::string& vrrpInterfaceName);

		bool hasPrioritizedVrrpSession(const std::string& vrrpInterfaceName);

		bool hasAddressNextHop(const std::string& routerName, const std::string& dstName, const std::string& nextHopName);

		// Clear all elements in the context
		void clear();

		bool isContextRemoveOngoing() const { return m_removeOngoing; };

 	private:

		// set context to be removed
		void setRemoveOngoing() { m_removeOngoing = true; };

		bool m_removeOngoing;

		std::string m_smxId;

		//
		typedef std::multimap<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Bridge> > bridgeMap_t;
		bridgeMap_t m_bridgeMap;

		//
		typedef std::multimap<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<EthernetPort> > ethernetPortMap_t;
		ethernetPortMap_t m_ethernetPortMap;

		//
		typedef std::multimap<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<BridgePort> > bridgePortMap_t;
		bridgePortMap_t m_bridgePortMap;

		//
		typedef std::multimap<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Aggregator> > aggregatorMap_t;
		aggregatorMap_t m_aggregatorMap;

		//
		typedef std::multimap<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<Vlan> > vlanMap_t;
		vlanMap_t m_vlanMap;

		//
		typedef std::multimap<fixs_ith::SwitchBoardPlaneConstants, boost::shared_ptr<SubnetVlan> > subnetVlanMap_t;
		subnetVlanMap_t m_subnetVlanMap;

		//
		typedef std::set<boost::shared_ptr<Router> > routerSet_t;
		routerSet_t m_routerSet;

		typedef std::multimap<std::string, boost::shared_ptr<InterfaceIPv4> > interfaceMap_t;
		interfaceMap_t m_interfaceMap;

		typedef std::multimap<std::string, boost::shared_ptr<VrrpSession> > vrrpSessions_t;
		vrrpSessions_t m_vrrpSessions;

		typedef std::multimap< std::string, boost::shared_ptr<Address> > addressMap_t;
		addressMap_t m_addressMap;

		typedef std::multimap<std::string, boost::shared_ptr<Dst> > dstMap_t;
		dstMap_t m_dstMap;

		typedef std::multimap<std::string, boost::shared_ptr<NextHop> > nextHopMap_t;
		nextHopMap_t m_nextHopMap;

		//
		typedef std::set<boost::shared_ptr<VrrpInterface> > vrrpInterfaceSet_t;
		vrrpInterfaceSet_t m_vrrpInterfaceSet;

		typedef std::multimap< std::string, boost::shared_ptr<BfdProfile> > bfdProfileMap_t;
		bfdProfileMap_t m_bfdProfileMap;

		typedef std::multimap< std::string, boost::shared_ptr<BfdSessionIPv4> > bfdSessions_t;
		bfdSessions_t m_bfdSessions;

		typedef std::multimap< std::string, boost::shared_ptr<AclEntryIpv4> > aclEntries_t;
		aclEntries_t m_aclEntries;

		typedef std::multimap< std::string, boost::shared_ptr<AclIpv4> > aclIpV4_t;
		aclIpV4_t m_aclIpV4;

		fixs_ith_switchInterfaceAdapter m_switchInterface;

		boost::recursive_mutex m_contextMutex[mutex_number];

		context_shared_mutex m_contextLock;


		bool try_lock() {return m_contextLock.try_lock();};

		bool try_lock_shared() {return m_contextLock.try_lock_shared();};

		void unlock() {m_contextLock.unlock();};

		void unlock_shared() {m_contextLock.unlock_shared();};

		void loadRuntimeInfo(boost::shared_ptr<BridgePort>& );

		void loadRuntimeInfo(boost::shared_ptr<EthernetPort>& );

		void loadRuntimeInfo(boost::shared_ptr<Aggregator>& );

		void loadRuntimeInfo(boost::shared_ptr<InterfaceIPv4>&, const uint16_t& runtimeAttributeMask );

		bool isMonitoredPortLink(const fixs_ith::SwitchBoardPlaneConstants&, const uint16_t& snmpIndex);

		bool isBackPlanePort(const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t if_index);

		bool isControlPort(const uint16_t if_index);

		bool isTransportPort(const uint16_t if_index);

	};

}


#endif /* ITHADM_CAA_INC_ENGINE_CONTEXT_H_ */
