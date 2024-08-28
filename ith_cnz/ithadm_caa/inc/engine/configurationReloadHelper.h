/*
 * configurationReloadHelper.h
 *
 *  Created on: Mar 1, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_ENGINE_CONFIGURATIONRELOADHELPER_H_
#define ITHADM_CAA_INC_ENGINE_CONFIGURATIONRELOADHELPER_H_

#include <boost/noncopyable.hpp>
#include <string>
#include <acs_apgcc_omhandler.h>
#include "fixs_ith_programconstants.h"

class OmHandler;

namespace engine
{

	/**
	 * @class DataReloadHelper
	 *
	 * @brief
	 *  This class helps to manage the data reload from IMM at startup
	 *
	 *
	*/
	class ConfigurationReloadHelper : private boost::noncopyable
	{
	 public:

		/// Constructor

		//Default constructor will load
		ConfigurationReloadHelper();
		ConfigurationReloadHelper(const std::string&);

		/// Destructor
		virtual ~ConfigurationReloadHelper();

		/**
		 *	@brief	Loads all defined data from IMM at startup
		 *
		 *	@return true on success otherwise false.
		 */
		bool reloadConfigurationFromIMM();

		int loadConfigurationPatch(std::string& error_message);

	 private:

		/**
		 *	@brief	Loads the common object(s), which are not part of one specific context.
		 *
		 *	@return true on success otherwise false.
		 */
		bool loadCommonObjs();
		/**
		 *	@brief	Loads all L2/L3 MOs from IMM
		 *
		 *	@return true on success otherwise false.
		 */
		bool loadSwitchBoardObjs();

		fixs_ith::ErrorConstants loadBridge(const std::string& objectDN);

		fixs_ith::ErrorConstants loadEthernetPort(const std::string& objectDN);

		fixs_ith::ErrorConstants loadAggregator(const std::string& objectDN);

		fixs_ith::ErrorConstants loadBridgePort(const std::string& objectDN);

		fixs_ith::ErrorConstants loadVlan(const std::string& objectDN);

		fixs_ith::ErrorConstants loadSubnetVlan(const std::string& objectDN);

		fixs_ith::ErrorConstants loadRouter(const std::string& objectDN);

		fixs_ith::ErrorConstants loadAclIpv4(const std::string& objectDN);

		fixs_ith::ErrorConstants loadAclEntry(const std::string& objectDN);

		fixs_ith::ErrorConstants loadInterface(const std::string& objectDN);

		fixs_ith::ErrorConstants loadAddress(const std::string& objectDN);

		fixs_ith::ErrorConstants loadDst(const std::string& objectDN);

		fixs_ith::ErrorConstants loadNextHop(const std::string& objectDN);

		fixs_ith::ErrorConstants loadVrrpInterface(const std::string& objectDN);

		fixs_ith::ErrorConstants loadVrrpSession(const std::string& objectDN);

		fixs_ith::ErrorConstants loadBfdSession(const std::string& objectDN);

		fixs_ith::ErrorConstants loadBfdProfile(const std::string& objectDN);


		void initializeClassNameMaps();

		// OM handler
		OmHandler m_objectManager;

		std::vector<std::string> m_commonClassNames;
		std::vector<std::string> m_contextSpecificClassNames;

		std::string m_switchBoardKey;

		bool m_loadAllObjects;
	};

} /* namespace engine */


#endif /* ITHADM_CAA_INC_ENGINE_CONFIGURATIONRELOADHELPER_H_ */
