/*
 * ConfigurationReloadHelper.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: eanform
 */

#include "engine/configurationReloadHelper.h"

#include "imm/imm.h"
#include "operation/operation.h"
#include "common/utility.h"

#include "fixs_ith_programconstants.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_logger.h"

#include "command/patch_cmd.h"

#include "acs_apgcc_omhandler.h"
#include <ACS_APGCC_Util.H>

#include <string>
#include <vector>

namespace engine
{
	ConfigurationReloadHelper::ConfigurationReloadHelper()
	: m_objectManager(),
	  m_commonClassNames(),
	  m_contextSpecificClassNames(),
	  m_switchBoardKey(),
	  m_loadAllObjects(true)
	{
		initializeClassNameMaps();
	}

	ConfigurationReloadHelper::ConfigurationReloadHelper(const std::string& switchBoardKey)
	: m_objectManager(),
	  m_commonClassNames(),
	  m_contextSpecificClassNames(),
	  m_switchBoardKey(switchBoardKey),
	  m_loadAllObjects(false)
	{
		initializeClassNameMaps();
	}

	ConfigurationReloadHelper::~ConfigurationReloadHelper()
	{

	}

	bool ConfigurationReloadHelper::reloadConfigurationFromIMM()
	{
		return ( m_switchBoardKey.empty() ? loadCommonObjs() : loadSwitchBoardObjs() );
	}

	int ConfigurationReloadHelper::loadConfigurationPatch(std::string& error_message)
	{
		struct stat buffer;
		std::string file_name = common::PATCH_FOLDER_NAME + "/" + m_switchBoardKey;

		if (stat (file_name.c_str(), &buffer) == 0)
		{
			smx_patch_cmd command(file_name.c_str());
			std::ostringstream error_stream;

			int ret_val = command.parse();
			if (ret_val != patch_cmd::NO_ERROR)
			{
				command.error_msg(ret_val, error_stream);
				error_message.assign(error_stream.str());

				return ret_val;
			}

			ret_val = command.execute();
			if (ret_val != patch_cmd::NO_ERROR)
			{
				command.error_msg(ret_val, error_stream);
				error_message.assign(error_stream.str());

				return ret_val;
			}

			return fixs_ith::ERR_NO_ERRORS;
		}

		return fixs_ith::ERR_NO_PATCH;

	}

	bool ConfigurationReloadHelper::loadCommonObjs()
	{
		fixs_ith::ErrorConstants loadResult = fixs_ith::ERR_NO_ERRORS;

		ACS_CC_ReturnType omResult = ACS_CC_SUCCESS;

		omResult = m_objectManager.Init();
		if (omResult != ACS_CC_SUCCESS)
		{
			//Internal Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: Initialization of OmHandler FAILED!!!");
			loadResult = fixs_ith::ERR_GENERIC;
		}
		else
		{
			for (std::vector<std::string>::const_iterator it = m_commonClassNames.begin(); fixs_ith::ERR_NO_ERRORS == loadResult && it != m_commonClassNames.end(); ++it)
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Instances of :<%s> ", it->c_str());

				std::vector<std::string> objectDNCollection;

				imm::getClassInstances(&m_objectManager, *it, objectDNCollection);

				for(std::vector<std::string>::iterator objectDN = objectDNCollection.begin(); fixs_ith::ERR_NO_ERRORS == loadResult && objectDN != objectDNCollection.end(); ++objectDN)
				{

					if (imm::moc_name::CLASS_VRRP_INTERFACE.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading VrrpInterface object :<%s> ", objectDN->c_str());

						loadResult = loadVrrpInterface(*objectDN);
					}
					else if (imm::moc_name::CLASS_ADDRESS.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Address object :<%s> ", objectDN->c_str());
						//We shall load ONLY the addresses of VRRP Interfaces
						if (common::utility::getSmxIdFromAddressDN(*objectDN).empty())
						{
							loadResult = loadAddress(*objectDN);
						}
					}
				}
			}

			omResult = m_objectManager.Finalize();
			if (omResult != ACS_CC_SUCCESS)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: Finalization of OmHandler FAILED!!!");
			}
		}

		return fixs_ith::ERR_NO_ERRORS == loadResult;
	}

	bool ConfigurationReloadHelper::loadSwitchBoardObjs()
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ConfigurationReloadHelper::loadSwitchBoardObjs");
		if(!m_loadAllObjects && m_switchBoardKey.empty())
		{
			//Nothing to do
			return true;
		}

		fixs_ith::ErrorConstants loadResult = fixs_ith::ERR_NO_ERRORS;

		ACS_CC_ReturnType omResult = ACS_CC_SUCCESS;

		omResult = m_objectManager.Init();
		if (omResult != ACS_CC_SUCCESS)
		{
			//Internal Error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: Initialization of OmHandler FAILED!!!");
			loadResult = fixs_ith::ERR_GENERIC;
		}
		else
		{
			for (std::vector<std::string>::const_iterator it = m_contextSpecificClassNames.begin(); fixs_ith::ERR_NO_ERRORS == loadResult && it != m_contextSpecificClassNames.end(); ++it)
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Instances of :<%s> ", it->c_str());

				std::vector<std::string> objectDNCollection;
				if (m_loadAllObjects)
				{
					imm::getClassInstances(&m_objectManager, *it, objectDNCollection);
				}
				else
				{
					imm::getClassInstances(&m_objectManager, *it, m_switchBoardKey, objectDNCollection);
				}

				for(std::vector<std::string>::iterator objectDN = objectDNCollection.begin(); fixs_ith::ERR_NO_ERRORS == loadResult && objectDN != objectDNCollection.end(); ++objectDN)
				{

					if(imm::moc_name::CLASS_BRIDGE.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Bridge object :<%s> ", objectDN->c_str());

						loadResult = loadBridge(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_ETH_PORT.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading EthernetPort object :<%s> ", objectDN->c_str());

						loadResult = loadEthernetPort(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_AGGREGATOR.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Aggregator object :<%s> ", objectDN->c_str());

						loadResult = loadAggregator(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_BRIDGE_PORT.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading BridgePort object :<%s> ", objectDN->c_str());

						loadResult = loadBridgePort(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_VLAN.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Vlan object :<%s> ", objectDN->c_str());

						loadResult = loadVlan(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_SUBNET_VLAN.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading SubnetVlan object :<%s> ", objectDN->c_str());
						loadResult = loadSubnetVlan(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_ROUTER.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Router object :<%s> ", objectDN->c_str());

						loadResult = loadRouter(*objectDN);
						continue;

					}

					if(imm::moc_name::CLASS_INTERFACE.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Interface object :<%s> ", objectDN->c_str());

						loadResult = loadInterface(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_ADDRESS.compare(*it) == 0)
					{
						//Skip load of the addresses of VRRP Interfaces
						if( !common::utility::getSmxIdFromAddressDN(*objectDN).empty() )
						{
							FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Address object :<%s> ", objectDN->c_str());
							loadResult = loadAddress(*objectDN);
						}

						continue;
					}

					if(imm::moc_name::CLASS_DST.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading Dst object :<%s> ", objectDN->c_str());

						loadResult = loadDst(*objectDN);
                                                FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ConfigurationReloadHelper::loadSwitchBoardObjs:loadDst completed succesfully");
						continue;
					}

					if(imm::moc_name::CLASS_NEXTHOP.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading NextHop object :<%s> ", objectDN->c_str());

						loadResult = loadNextHop(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_VRRP_SESSION.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading VRRP Session object :<%s> ", objectDN->c_str());

						loadResult = loadVrrpSession(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_BFD_SESSION.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading BFD Session object :<%s> ", objectDN->c_str());

						loadResult = loadBfdSession(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_BFD_PROFILE.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading BFD Profile object :<%s> ", objectDN->c_str());

						loadResult = loadBfdProfile(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_ACL.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading ACL IPv4 object :<%s> ", objectDN->c_str());

						loadResult = loadAclIpv4(*objectDN);
						continue;
					}

					if(imm::moc_name::CLASS_ACL_ENTRY.compare(*it) == 0)
					{
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading ACL Entry IPv4 object :<%s> ", objectDN->c_str());

						loadResult = loadAclEntry(*objectDN);
						continue;
					}
				}
			}

			omResult = m_objectManager.Finalize();
			if (omResult != ACS_CC_SUCCESS)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: Finalization of OmHandler FAILED!!!");
			}
		}
               FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ConfigurationReloadHelper:End Result is succesful");
		return fixs_ith::ERR_NO_ERRORS == loadResult;

	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadBridge(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Bridge DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;


		ACS_APGCC_ImmAttribute addressAttribute;
		addressAttribute.attrName.assign(imm::bridge_attribute::ADDRESS);
		attributeList.push_back(&addressAttribute);


		ACS_APGCC_ImmAttribute subnetVlanRefAttribute;
		subnetVlanRefAttribute.attrName.assign(imm::bridge_attribute::SUBNET_VLAN_REF);
		attributeList.push_back(&subnetVlanRefAttribute);


		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::bridgeInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);
			dataMO.smxId = common::utility::getSmxIdFromBridgeDN(objectDN);
			dataMO.plane = common::utility::getPlaneFromBridgeDN(objectDN);

			if (0U != addressAttribute.attrValuesNum)
			{
				dataMO.bridgeAddress = (reinterpret_cast<char*>(addressAttribute.attrValues[0]));
			}

			dataMO.ipv4SubnetVlan.clear();
			for (uint32_t j = 0; j < subnetVlanRefAttribute.attrValuesNum; j++)
			{
				dataMO.ipv4SubnetVlan.insert(reinterpret_cast<char*>(subnetVlanRefAttribute.attrValues[j]));
			}


			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a Bridge\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    Plane: <%s>\n"
						"#####    Bridge: <%s>\n"
						"#####    ipv4SubnetVlan [size]:<%u>\n"
						"#####    address:<%s>\n",
						dataMO.getBridgeDN(), dataMO.getSmxId(), common::utility::planeToString(dataMO.getPlane()).c_str(), dataMO.getName(), dataMO.getSubnetVlanReference().size(), dataMO.getBridgeAddress());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add Bridge:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getBridgeDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadEthernetPort(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "EthernetPort DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;


		ACS_APGCC_ImmAttribute macAddressAttribute;
		macAddressAttribute.attrName.assign(imm::ethernetPort_attribute::MAC_ADDRESS);
		attributeList.push_back(&macAddressAttribute);

		ACS_APGCC_ImmAttribute snmpIndexAttribute;
		snmpIndexAttribute.attrName.assign(imm::ethernetPort_attribute::SNMP_INDEX);
		attributeList.push_back(&snmpIndexAttribute);

		ACS_APGCC_ImmAttribute maxFrameSizeAttribute;
		maxFrameSizeAttribute.attrName.assign(imm::ethernetPort_attribute::MAX_FRAME_SIZE);
		attributeList.push_back(&maxFrameSizeAttribute);

		ACS_APGCC_ImmAttribute autoNegotiateAttribute;
		autoNegotiateAttribute.attrName.assign(imm::ethernetPort_attribute::AUTO_NEGOTIATE);
		attributeList.push_back(&autoNegotiateAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::ethernetPortInfo dataMO;

		if(getResult == ACS_CC_SUCCESS )
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);
			dataMO.smxId = common::utility::getSmxIdFromEthernetPortDN(objectDN);
			dataMO.plane = common::utility::getPlaneFromEthernetPortDN(objectDN);

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "-----------------------------------------\n"
									"#####    DN: <%s>\n"
									"#####    SMX ID: <%s>\n"
									"#####    Plane: <%s>\n"
									"#####    ---------------------------------------",
									dataMO.getEthernetPortDN(), dataMO.getSmxId(), common::utility::planeToString(dataMO.getPlane()).c_str());

			if (0U != macAddressAttribute.attrValuesNum)
			{
				dataMO.macAddress = (reinterpret_cast<char*>(macAddressAttribute.attrValues[0]));
			}

			if (0U != snmpIndexAttribute.attrValuesNum)
			{
				dataMO.snmpIndex = *(reinterpret_cast<uint16_t*>(snmpIndexAttribute.attrValues[0]));
			}

			if (0U != maxFrameSizeAttribute.attrValuesNum)
			{
				dataMO.maxFrameSize = *(reinterpret_cast<uint32_t*>(maxFrameSizeAttribute.attrValues[0]));
			}

			if (0U != autoNegotiateAttribute.attrValuesNum)
			{
				int value = *reinterpret_cast<int32_t*>(autoNegotiateAttribute.attrValues[0]);
				dataMO.autoNegotiate = ( 0U == value ? imm::AN_LOCKED : imm::AN_UNLOCKED );
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a EthernetPort\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    Plane: <%s>\n"
						"#####    EthernetPort: <%s>\n"
						"#####    macAddress:<%s>\n"
						"#####    snmpIndex: <%u>\n"
						"#####    maxFrameSize: <%u>\n"
						"#####    autoNegotiate: <%d>\n",
						dataMO.getEthernetPortDN(), dataMO.getSmxId(), common::utility::planeToString(dataMO.getPlane()).c_str(),
						dataMO.getName(), dataMO.getMacAddress(), dataMO.getSnmpIndex(), dataMO.getMaxFrameSize(), dataMO.getAutoNegotiate());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add EthernetPort:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getEthernetPortDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadAggregator(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Aggregator DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;


		ACS_APGCC_ImmAttribute aggMemberAttribute;
		aggMemberAttribute.attrName.assign(imm::aggregator_attribute::AGG_MEMBER);
		attributeList.push_back(&aggMemberAttribute);

		ACS_APGCC_ImmAttribute snmpIndexAttribute;
		snmpIndexAttribute.attrName.assign(imm::aggregator_attribute::SNMP_INDEX);
		attributeList.push_back(&snmpIndexAttribute);


		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::aggregatorInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);
			dataMO.smxId = common::utility::getSmxIdFromAggregatorDN(objectDN);
			dataMO.plane = common::utility::getPlaneFromAggregatorDN(objectDN);

			dataMO.aggMember.clear();
			for (uint32_t j = 0; j < aggMemberAttribute.attrValuesNum; j++)
			{
				dataMO.aggMember.insert(reinterpret_cast<char*>(aggMemberAttribute.attrValues[j]));
			}

			if (0U != snmpIndexAttribute.attrValuesNum)
			{
				dataMO.snmpIndex = *(reinterpret_cast<uint16_t*>(snmpIndexAttribute.attrValues[0]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a Aggregator\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    Plane: <%s>\n"
						"#####    Aggregator: <%s>\n"
						"#####    dataRate: <%u>\n"
						"#####    aggMember [size]: <%u>\n"
						"#####    snmpIndex: <%u>\n",
						dataMO.getAggregatorDN(), dataMO.getSmxId(), common::utility::planeToString(dataMO.getPlane()).c_str(),dataMO.getName(), dataMO.getDataRate(), dataMO.getAdminAggMember().size(), dataMO.getSnmpIndex());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add Aggregator:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getAggregatorDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadBridgePort(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BridgePort DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute adminstateAttribute;
		adminstateAttribute.attrName.assign(imm::bridgePort_attribute::ADM_STATE);
		attributeList.push_back(&adminstateAttribute);

		ACS_APGCC_ImmAttribute defaultVlanIdAttribute;
		defaultVlanIdAttribute.attrName.assign(imm::bridgePort_attribute::DEFAULT_VLAN_ID);
		attributeList.push_back(&defaultVlanIdAttribute);

		ACS_APGCC_ImmAttribute portAddressAttribute;
		portAddressAttribute.attrName.assign(imm::bridgePort_attribute::PORT_ADDRESS);
		attributeList.push_back(&portAddressAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::bridgePortInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);

			std::string parentRDNvalue =  common::utility::getParentDN(dataMO.moDN);
			dataMO.smxId = common::utility::getSmxIdFromBridgeDN(parentRDNvalue);
			dataMO.plane = common::utility::getPlaneFromBridgeDN(parentRDNvalue);

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "-----------------------------------------\n"
					"#####    DN: <%s>\n"
					"#####    SMX ID: <%s>\n"
					"#####    Plane: <%s>\n"
					"#####    \n---------------------------------------",
					dataMO.getBridgePortDN(), dataMO.getSmxId(), common::utility::planeToString(dataMO.getPlane()).c_str());

			if(0U != defaultVlanIdAttribute.attrValuesNum)
			{
				dataMO.defaultVlanId = *(reinterpret_cast<uint32_t*>(defaultVlanIdAttribute.attrValues[0]));
				dataMO.changeMask |= imm::bridgePort_attribute::DEFAULT_VLAN_ID_CHANGE;
			}

			if(0U != portAddressAttribute.attrValuesNum)
			{
				dataMO.portAddress = reinterpret_cast<char*>(portAddressAttribute.attrValues[0]);
				dataMO.changeMask |= imm::bridgePort_attribute::PORT_ADDRESS_CHANGE;
			}

			if(0U != adminstateAttribute.attrValuesNum)
			{
				uint16_t IfIndex ;
				fixs_ith::ErrorConstants result = fixs_ith::workingSet_t::instance()->getConfigurationManager().getSnmpIndexfromBridgePortDN(objectDN, IfIndex);
				if(result) {
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Load BridgePort:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
							dataMO.getBridgePortDN(), result);
					return result;
				}
				// If IfIndex is an interface to be ENABLED as last step of SMX configuration set changeMask to skip operation towards SMX
				if (fixs_ith::workingSet_t::instance()->getConfigurationManager().is_early_enabled_if(dataMO.plane, IfIndex)){
					dataMO.changeMask |= imm::bridgePort_attribute::ADM_STATE_CHANGE;
				}
				dataMO.adminstate = *(reinterpret_cast<imm::admin_State *>(adminstateAttribute.attrValues[0]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a BridgePort\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    Plane: <%s>\n"
						"#####    BridgePort: <%s>\n"
						"#####    portAddress: <%s>\n"
						"#####    adminState: <%d>\n"
						"#####    defaultVlanId: <%u>\n"
						"#####    changeMask: <%u>\n",
						dataMO.getBridgePortDN(), dataMO.getSmxId(), common::utility::planeToString(dataMO.getPlane()).c_str(),dataMO.getName(), dataMO.getPortAddress(), dataMO.getBridgePortState(), dataMO.getDefaultVlanId(),
						dataMO.getChangeMask());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add BridgePort:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getBridgePortDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

fixs_ith::ErrorConstants ConfigurationReloadHelper::loadVlan(
		const std::string& objectDN) {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Vlan DN:<%s> ", objectDN.c_str());

	fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

	//List of attributes to get
	std::vector<ACS_APGCC_ImmAttribute*> attributeList;

	ACS_APGCC_ImmAttribute vlanIdAttribute;
	vlanIdAttribute.attrName.assign(imm::vlan_attribute::VLAN_ID);
	attributeList.push_back(&vlanIdAttribute);

	ACS_APGCC_ImmAttribute taggedPortsAttribute;
	taggedPortsAttribute.attrName.assign(imm::vlan_attribute::TAGGED_PORTS);
	attributeList.push_back(&taggedPortsAttribute);

	ACS_APGCC_ImmAttribute untaggedPortsAttribute;
	untaggedPortsAttribute.attrName.assign(imm::vlan_attribute::UNTAGGED_PORTS);
	attributeList.push_back(&untaggedPortsAttribute);

	ACS_APGCC_ImmAttribute reservedByAttribute;
	reservedByAttribute.attrName.assign(imm::vlan_attribute::RESERVED_BY);
	attributeList.push_back(&reservedByAttribute);

	ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(),
			attributeList);

	operation::vlanInfo dataMO;

	if (getResult == ACS_CC_SUCCESS) {
		dataMO.moDN.assign(objectDN);
		dataMO.name = common::utility::getIdValueFromRdn(objectDN);

		std::string parentRDNvalue = common::utility::getParentDN(dataMO.moDN);
		dataMO.smxId = common::utility::getSmxIdFromBridgeDN(parentRDNvalue);
		dataMO.plane = common::utility::getPlaneFromBridgeDN(parentRDNvalue);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"-----------------------------------------\n" "#####    DN: <%s>\n" "#####    SMX ID: <%s>\n" "#####    Plane: <%s>\n" "#####    \n---------------------------------------",
				dataMO.getVlanDN(), dataMO.getSmxId(),
				common::utility::planeToString(dataMO.getPlane()).c_str());

		dataMO.taggedBridgePorts.clear();
		for (uint32_t j = 0; j < taggedPortsAttribute.attrValuesNum; j++) {
			dataMO.taggedBridgePorts.insert(
					reinterpret_cast<char*>(taggedPortsAttribute.attrValues[j]));
		}

		for (uint32_t j = 0; j < untaggedPortsAttribute.attrValuesNum; j++) {
			dataMO.untaggedBridgePorts.insert(
					reinterpret_cast<char*>(untaggedPortsAttribute.attrValues[j]));
		}

		if (0U != vlanIdAttribute.attrValuesNum) {
			dataMO.vId =
					*(reinterpret_cast<uint16_t *>(vlanIdAttribute.attrValues[0]));
		}

		for (uint32_t j = 0; j < reservedByAttribute.attrValuesNum; j++) {
			dataMO.reservedBy.insert(
					reinterpret_cast<char *>(reservedByAttribute.attrValues[j]));
		}

		result =
				fixs_ith::workingSet_t::instance()->getConfigurationManager().add(
						dataMO);

		if (fixs_ith::ERR_NO_ERRORS == result) {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Added a Vlan\n" "#####    DN: <%s>\n" "#####    SMX ID: <%s>\n" "#####    Plane: <%s>\n" "#####    Vlan: <%s>\n" "#####    taggedPorts [size]: <%u>\n" "#####    untaggedPorts [size]: <%u>\n" "#####    vlan tag: <%u>\n",
					dataMO.getVlanDN(), dataMO.getSmxId(),
					common::utility::planeToString(dataMO.getPlane()).c_str(),
					dataMO.getName(), dataMO.getVlanTaggedPorts().size(),
					dataMO.getVlanUntaggedPorts().size(), dataMO.getVlanId());
		} else {
			// Error on adding data source
			// TR HY37073
			//GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Failed to add Vlan:<%s>, DN:<%s>: errorCode:<%d>",
					dataMO.getName(), dataMO.getVlanDN(), result);

			if ((common::utility::is_hwtype_gep5())
					&& (dataMO.getPlane() == fixs_ith::TRANSPORT_PLANE)) {
				if (dataMO.getName() == engine::updVlan::transport::left::NAME
						|| dataMO.getName()
								== engine::updVlan::transport::right::NAME
						|| dataMO.getName()
								== engine::updVlan::transport::cmxleft::NAME
						|| dataMO.getName()
								== engine::updVlan::transport::cmxright::NAME) {
					result = fixs_ith::ERR_NO_ERRORS;
					FIXS_ITH_LOG(LOG_LEVEL_WARN,
							"Ignore Error No need to add UPD Vlan:<%s>, DN:<%s>: plane:<%s>,errorCode:<%d>",
							dataMO.getName(), dataMO.getVlanDN(),
							common::utility::planeToString(dataMO.getPlane()).c_str(),
							result);

				}

			}
		}
		//GEP2_GEP5 to GEP7 Dynamic UPD VLAN END
	} else {
		// getAttribute error
		// OmHandler error
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
				objectDN.c_str(), m_objectManager.getInternalLastError(),
				m_objectManager.getInternalLastErrorText());
	}

	return result;
}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadSubnetVlan(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "SubnetVlan DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute vlanIdAttribute;
		vlanIdAttribute.attrName.assign(imm::subnetVlan_attribute::SUBNET_VID);
		attributeList.push_back(&vlanIdAttribute);

		ACS_APGCC_ImmAttribute ipSubnetAttribute;
		ipSubnetAttribute.attrName.assign(imm::subnetVlan_attribute::IP_SUBNET);
		attributeList.push_back(&ipSubnetAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::subnetVlanInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);

			std::string parentRDNvalue =  common::utility::getParentDN(dataMO.moDN);
			dataMO.smxId = common::utility::getSmxIdFromBridgeDN(parentRDNvalue);
			dataMO.plane = common::utility::getPlaneFromBridgeDN(parentRDNvalue);

			if (0U !=  ipSubnetAttribute.attrValuesNum)
			{
				dataMO.ipv4Subnet = reinterpret_cast<char*>(ipSubnetAttribute.attrValues[0]);
			}

			if(0U != vlanIdAttribute.attrValuesNum)
			{
				dataMO.vId = *(reinterpret_cast<uint16_t *>(vlanIdAttribute.attrValues[0]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a SubnetVlan\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    Plane: <%s>\n"
						"#####    SubnetVlan: <%s>\n"
						"#####    ipv4Subnet : <%s>\n"
						"#####    vlan tag: <%u>\n",
						dataMO.getSubnetVlanDN(), dataMO.getSmxId(), common::utility::planeToString(dataMO.getPlane()).c_str(),dataMO.getName(), dataMO.getIpSubnet(), dataMO.getSubnetVlanId());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add SubnetVlan:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getSubnetVlanDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadRouter(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Router DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		operation::routerInfo dataMO;

		dataMO.moDN.assign(objectDN);
		dataMO.name = common::utility::getIdValueFromRdn(objectDN);
		dataMO.smxId = common::utility::getSmxIdFromRouterDN(objectDN);

		result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

		if( fixs_ith::ERR_NO_ERRORS == result )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a Router\n"
					"#####    DN: <%s>\n"
					"#####    SMX ID: <%s>\n"
					"#####    Router: <%s>\n",
					dataMO.getDN(), dataMO.getSmxId(), dataMO.getName());
		}
		else
		{
			// Error on adding data source
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add Router:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
					dataMO.getDN(), result);
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadAclIpv4(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ACL Ipv4 DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute reservedByAttribute;
		reservedByAttribute.attrName.assign(imm::aclIpv4_attribute::RESERVED_BY);
		attributeList.push_back(&reservedByAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		if(getResult == ACS_CC_SUCCESS)
		{
			operation::aclIpV4Info dataMO;

			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);
			dataMO.smxId = common::utility::getSmxIdFromAclDN(objectDN);

			for (uint32_t j = 0; j < reservedByAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::aclIpv4_attribute::RESERVED_BY_CHANGE;
				dataMO.reservedBy.insert(reinterpret_cast<char *>(reservedByAttribute.attrValues[j]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a ACL IPv4\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    ACLIpv4: <%s>\n",
						dataMO.getDN(), dataMO.getSmxId(), dataMO.getName());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add ACL Ipv4:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadAclEntry(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ACL Entry Ipv4 DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute actionAttribute;
		actionAttribute.attrName.assign(imm::aclEntry_attribute::ACLACTION);
		attributeList.push_back(&actionAttribute);

		ACS_APGCC_ImmAttribute addrDstAttribute;
		addrDstAttribute.attrName.assign(imm::aclEntry_attribute::ADDR_DST);
		attributeList.push_back(&addrDstAttribute);

		ACS_APGCC_ImmAttribute addrSrcAttribute;
		addrSrcAttribute.attrName.assign(imm::aclEntry_attribute::ADDR_SRC);
		attributeList.push_back(&addrSrcAttribute);

		ACS_APGCC_ImmAttribute dscpAttribute;
		dscpAttribute.attrName.assign(imm::aclEntry_attribute::DSCP);
		attributeList.push_back(&dscpAttribute);

		ACS_APGCC_ImmAttribute fragmentTypeAttribute;
		fragmentTypeAttribute.attrName.assign(imm::aclEntry_attribute::FRAGMENT_TYPE);
		attributeList.push_back(&fragmentTypeAttribute);

		ACS_APGCC_ImmAttribute icmpTypeAttribute;
		icmpTypeAttribute.attrName.assign(imm::aclEntry_attribute::ICMP_TYPE);
		attributeList.push_back(&icmpTypeAttribute);

		ACS_APGCC_ImmAttribute ipProtocolAttribute;
		ipProtocolAttribute.attrName.assign(imm::aclEntry_attribute::IP_PROTOCOL);
		attributeList.push_back(&ipProtocolAttribute);

		ACS_APGCC_ImmAttribute portDstAttribute;
		portDstAttribute.attrName.assign(imm::aclEntry_attribute::PORT_DST);
		attributeList.push_back(&portDstAttribute);

		ACS_APGCC_ImmAttribute portSrcAttribute;
		portSrcAttribute.attrName.assign(imm::aclEntry_attribute::PORT_SRC);
		attributeList.push_back(&portSrcAttribute);

		ACS_APGCC_ImmAttribute priorityAttribute;
		priorityAttribute.attrName.assign(imm::aclEntry_attribute::PRIORITY);
		attributeList.push_back(&priorityAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		if(getResult == ACS_CC_SUCCESS)
		{
			operation::aclEntryInfo dataMO;

			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);
			dataMO.smxId = common::utility::getSmxIdFromAclEntryDN(objectDN);

			if(0U != actionAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::ACTION_CHANGE;
				dataMO.action = *(reinterpret_cast<imm::aclAction *>(actionAttribute.attrValues[0]));
			}

			for (uint32_t j = 0; j < addrDstAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::ADDR_DST_CHANGE;
				dataMO.addrDst.insert(reinterpret_cast<char *>(addrDstAttribute.attrValues[j]));
			}

			for (uint32_t j = 0; j < addrSrcAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::ADDR_SRC_CHANGE;
				dataMO.addrSrc.insert(reinterpret_cast<char *>(addrSrcAttribute.attrValues[j]));
			}

			for (uint32_t j = 0; j < dscpAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::DSCP_CHANGE;
				dataMO.dscp.insert(reinterpret_cast<char *>(dscpAttribute.attrValues[j]));
			}

			if (0U != fragmentTypeAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::FRAGMENT_TYPE_CHANGE;
				dataMO.fragmentType = *(reinterpret_cast<imm::aclFragmentType*>(fragmentTypeAttribute.attrValues[0]));
			}

			for (uint32_t j = 0; j < icmpTypeAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::ICMP_TYPE_CHANGE;
				dataMO.icmpType.insert(reinterpret_cast<char *>(icmpTypeAttribute.attrValues[j]));
			}

			for (uint32_t j = 0; j < ipProtocolAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::IP_PROTOCOL_CHANGE;
				dataMO.ipProtocol.insert(reinterpret_cast<char *>(ipProtocolAttribute.attrValues[j]));
			}

			for (uint32_t j = 0; j < portDstAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::PORT_DST_CHANGE;
				dataMO.portDst.insert(reinterpret_cast<char *>(portDstAttribute.attrValues[j]));
			}

			for (uint32_t j = 0; j < portSrcAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::PORT_SRC_CHANGE;
				dataMO.portSrc.insert(reinterpret_cast<char *>(portSrcAttribute.attrValues[j]));
			}

			if(0U != priorityAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::aclEntry_attribute::PRIORITY_CHANGE;
				dataMO.priority = *(reinterpret_cast<uint16_t *>(priorityAttribute.attrValues[0]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a ACL ENTRY\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    AclEntryIpv4: <%s>\n",
						dataMO.getDN(), dataMO.getSmxId(), dataMO.getName());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add AclEntryIpv4:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadInterface(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "InterfaceIPv4 DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute aclAttribute;
		aclAttribute.attrName.assign(imm::interfaceIpv4_attribute::ACL_IPV4);
		attributeList.push_back(&aclAttribute);

		ACS_APGCC_ImmAttribute mtuAttribute;
		mtuAttribute.attrName.assign(imm::interfaceIpv4_attribute::MTU);
		attributeList.push_back(&mtuAttribute);

		ACS_APGCC_ImmAttribute loopbackAttribute;
		loopbackAttribute.attrName.assign(imm::interfaceIpv4_attribute::LOOPBACK);
		attributeList.push_back(&loopbackAttribute);

		ACS_APGCC_ImmAttribute encapsulationAttribute;
		encapsulationAttribute.attrName.assign(imm::interfaceIpv4_attribute::ENCAPSULATION);
		attributeList.push_back(&encapsulationAttribute);

		ACS_APGCC_ImmAttribute bfdStaticRoutesAttribute;
		bfdStaticRoutesAttribute.attrName.assign(imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES);
		attributeList.push_back(&bfdStaticRoutesAttribute);

		ACS_APGCC_ImmAttribute bfdProfileAttribute;
		bfdProfileAttribute.attrName.assign(imm::interfaceIpv4_attribute::BFD_PROFILE);
		attributeList.push_back(&bfdProfileAttribute);

		ACS_APGCC_ImmAttribute adminStateAttribute;
		adminStateAttribute.attrName.assign(imm::interfaceIpv4_attribute::ADMIN_STATE);
		attributeList.push_back(&adminStateAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::interfaceInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);
			dataMO.smxId = common::utility::getSmxIdFromInterfaceDN(dataMO.moDN);

			if (0U != aclAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::interfaceIpv4_attribute::ACL_IPV4_CHANGE;
				dataMO.aclIngress.assign(reinterpret_cast<char *>(aclAttribute.attrValues[0]));
			}

			if (0U !=  loopbackAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::interfaceIpv4_attribute::LOOPBACK_CHANGE;
				dataMO.loopback = *(reinterpret_cast<bool*>(loopbackAttribute.attrValues[0]));
			}

			if(0U != mtuAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::interfaceIpv4_attribute::MTU_CHANGE;
				dataMO.mtu = *(reinterpret_cast<uint32_t *>(mtuAttribute.attrValues[0]));
			}

			if(0U != encapsulationAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::interfaceIpv4_attribute::ENCAPSULATION_CHANGE;
				dataMO.encapsulation.assign(reinterpret_cast<char*>(encapsulationAttribute.attrValues[0]));
			}

			if (0U != bfdStaticRoutesAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::interfaceIpv4_attribute::BFD_STATIC_ROUTES_CHANGE;
				dataMO.bfdStaticRoutes = *(reinterpret_cast<imm::oper_State*>(bfdStaticRoutesAttribute.attrValues[0]));
			}

			if (0U != bfdProfileAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::interfaceIpv4_attribute::BFD_PROFILE_CHANGE;
				dataMO.bfdProfile.assign(reinterpret_cast<char*>(bfdProfileAttribute.attrValues[0]));
			}

			if (0U != adminStateAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::interfaceIpv4_attribute::ADMIN_STATE_CHANGE;
				dataMO.adminstate = *(reinterpret_cast<imm::admin_State*>(adminStateAttribute.attrValues[0]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a InterfaceIPv4\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    InterfaceIPv4: <%s>\n"
						"#####    loopback : <%d>\n"
						"#####    mtu : <%d>\n"
						"#####    encapsulation: <%s>\n"
						"#####    bfdStaticRoutes: <%d>\n"
						"#####    bfdProfile: <%s>\n"
						"#####    adminstate: <%d>\n"
						"#####    aclIngress: <%s>\n",
						dataMO.getDN(), dataMO.getSmxId(), dataMO.getName(),
						dataMO.loopback, dataMO.mtu, dataMO.getEncapsulation(),
						dataMO.bfdStaticRoutes, dataMO.getBfdProfile(), dataMO.getAdmState(), dataMO.getAclIngress());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add InterfaceIPv4:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadAddress(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Address DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute addressAttribute;
		addressAttribute.attrName.assign(imm::addressIpv4_attribute::ADDRESS);
		attributeList.push_back(&addressAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::addressInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);

			dataMO.smxId = common::utility::getSmxIdFromAddressDN(dataMO.moDN);


			if(0U != addressAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::addressIpv4_attribute::ADDRESS_CHANGE;
				dataMO.address = reinterpret_cast<char *>(addressAttribute.attrValues[0]);
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a Address\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    Address ID: <%s>\n"
						"#####    address : <%s>\n",
						dataMO.getDN(), dataMO.getSmxId(), dataMO.getName(),
						dataMO.getAddress());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add Address:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadDst(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Dst DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute dstAttribute;
		dstAttribute.attrName.assign(imm::dst_attribute::DST);
		attributeList.push_back(&dstAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::dstInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);

			dataMO.smxId = common::utility::getSmxIdFromDstDN(dataMO.moDN);


			if(0U != dstAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::dst_attribute::DST_CHANGE;
				dataMO.dst.assign(reinterpret_cast<char *>(dstAttribute.attrValues[0]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a Dst\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    Dst ID: <%s>\n"
						"#####    dst : <%s>\n",
						dataMO.getDN(), dataMO.getSmxId(), dataMO.getName(),
						dataMO.getDst());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add Dst:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadNextHop(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "NextHop DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute addressAttribute;
		addressAttribute.attrName.assign(imm::nextHop_attribute::ADDRESS);
		attributeList.push_back(&addressAttribute);

		ACS_APGCC_ImmAttribute adminDistanceAttribute;
		adminDistanceAttribute.attrName.assign(imm::nextHop_attribute::ADMIN_DISTANCE);
		attributeList.push_back(&adminDistanceAttribute);

		ACS_APGCC_ImmAttribute bfdMonitoringAttribute;
		bfdMonitoringAttribute.attrName.assign(imm::nextHop_attribute::BFD_MONITORING);
		attributeList.push_back(&bfdMonitoringAttribute);

		ACS_APGCC_ImmAttribute discardAttribute;
		discardAttribute.attrName.assign(imm::nextHop_attribute::DISCARD);
		attributeList.push_back(&discardAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::nextHopInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);

			dataMO.smxId = common::utility::getSmxIdFromNextHopDN(dataMO.moDN);


			if(0U != addressAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::nextHop_attribute::ADDRESS_CHANGE;
				dataMO.address = reinterpret_cast<char *>(addressAttribute.attrValues[0]);
			}

			if(0U != adminDistanceAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::nextHop_attribute::ADMIN_DISTANCE_CHANGE;
				dataMO.adminDistance = *(reinterpret_cast<int16_t *>(adminDistanceAttribute.attrValues[0]));
			}

			if(0U != bfdMonitoringAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::nextHop_attribute::BFD_MONITORING_CHANGE;
				dataMO.bfdMonitoring = *(reinterpret_cast<bool *>(bfdMonitoringAttribute.attrValues[0]));
			}

			if(0U != discardAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::nextHop_attribute::DISCARD_CHANGE;
				dataMO.discard = *(reinterpret_cast<bool *>(discardAttribute.attrValues[0]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a NextHop\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    NextHop ID : <%s>\n"
						"#####    address : <%s>\n"
						"#####    adminDistance : <%d>\n"
						"#####    bfdMonitoring : <%d>\n"
						"#####    discard : <%d>\n",
						dataMO.getDN(), dataMO.getSmxId(), dataMO.getName(),
						dataMO.getAddress(), dataMO.getAdminDistance(), dataMO.getBfdMonitoring(), dataMO.getDiscard());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add NextHop:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadVrrpInterface(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpInterface DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute advertiseIntervalAttribute;
		advertiseIntervalAttribute.attrName.assign(imm::vrrpInterface_attribute::ADV_INTERVAL);
		attributeList.push_back(&advertiseIntervalAttribute);

		ACS_APGCC_ImmAttribute preemptHoldTimeAttribute;
		preemptHoldTimeAttribute.attrName.assign(imm::vrrpInterface_attribute::PREEMPT_HOLD_TIME);
		attributeList.push_back(&preemptHoldTimeAttribute);

		ACS_APGCC_ImmAttribute preemptModeAttribute;
		preemptModeAttribute.attrName.assign(imm::vrrpInterface_attribute::PREEMPT_MODE);
		attributeList.push_back(&preemptModeAttribute);

		ACS_APGCC_ImmAttribute reservedByAttribute;
		reservedByAttribute.attrName.assign(imm::vrrpInterface_attribute::RESERVED_BY);
		attributeList.push_back(&reservedByAttribute);

		ACS_APGCC_ImmAttribute vrIdentityAttribute;
		vrIdentityAttribute.attrName.assign(imm::vrrpInterface_attribute::IDENTITY);
		attributeList.push_back(&vrIdentityAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		operation::vrrpInterfaceInfo dataMO;

		if(getResult == ACS_CC_SUCCESS)
		{
			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getIdValueFromRdn(objectDN);

			//dataMO.smxId is intentionally empty. VRRP interface won't be included in any switch specific context

			if(0U != advertiseIntervalAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::vrrpInterface_attribute::ADV_INTERVAL_CHANGE;
				dataMO.advertiseInterval = *(reinterpret_cast<int32_t *>(advertiseIntervalAttribute.attrValues[0]));
			}

			if(0U != preemptHoldTimeAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::vrrpInterface_attribute::PREEMPT_HOLD_TIME_CHANGE;
				dataMO.preemptHoldTime = *(reinterpret_cast<int32_t *>(preemptHoldTimeAttribute.attrValues[0]));
			}

			if(0U != preemptModeAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::vrrpInterface_attribute::PREEMPT_MODE_CHANGE;
				dataMO.preemptMode = *(reinterpret_cast<bool *>(preemptModeAttribute.attrValues[0]));
			}

			for (uint32_t j = 0; j < reservedByAttribute.attrValuesNum; j++)
			{
				dataMO.changeMask |= imm::vrrpInterface_attribute::RESERVED_BY_CHANGE;
				dataMO.reservedBy.insert(reinterpret_cast<char *>(reservedByAttribute.attrValues[j]));
			}

			if(0U != vrIdentityAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::vrrpInterface_attribute::IDENTITY_CHANGE;
				dataMO.vrIdentity = *(reinterpret_cast<int32_t *>(vrIdentityAttribute.attrValues[0]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a VrrpInterface\n"
						"#####    DN: <%s>\n"
						"#####    VrrpInterface ID : <%s>\n"
						"#####    advertiseInterval : <%d>\n"
						"#####    preemptHoldTime : <%d>\n"
						"#####    preemptMode : <%d>\n"
						"#####    vrIdentity : <%d>\n",
						dataMO.getDN(), dataMO.getName(),
						dataMO.getAdvertiseInterval(), dataMO.getPreemptHoldTime(), dataMO.getPreemptMode(), dataMO.getVrIdentity());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add VrrpInterface:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadVrrpSession(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpSession DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute adminStateAttribute;
		adminStateAttribute.attrName.assign(imm::vrrpSession_attribute::ADMIN_STATE);
		attributeList.push_back(&adminStateAttribute);

		ACS_APGCC_ImmAttribute vrrpInterfaceRefAttribute;
		vrrpInterfaceRefAttribute.attrName.assign(imm::vrrpSession_attribute::VRRP_INTERFACE_REF);
		attributeList.push_back(&vrrpInterfaceRefAttribute);

		ACS_APGCC_ImmAttribute prioSessionAttribute;
		prioSessionAttribute.attrName.assign(imm::vrrpSession_attribute::PRIO_SESSION);
		attributeList.push_back(&prioSessionAttribute);

		ACS_APGCC_ImmAttribute priorityAttribute;
		priorityAttribute.attrName.assign(imm::vrrpSession_attribute::PRIORITY);
		attributeList.push_back(&priorityAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		if(ACS_CC_SUCCESS == getResult)
		{
			operation::vrrpSessionInfo dataMO;

			dataMO.moDN.assign(objectDN);
			dataMO.name = common::utility::getVrrpSessionNameFromDN(objectDN);
			dataMO.smxId = common::utility::getSmxIdFromVrrpv3SessionDN(dataMO.moDN);


			if(0U != adminStateAttribute.attrValuesNum)
			{
				// get Administrative State value
				int32_t value = *reinterpret_cast<int32_t*>(adminStateAttribute.attrValues[0]);
				dataMO.administrativeState = ( 0U == value ? imm::LOCKED : imm::UNLOCKED );
				dataMO.changeMask |= imm::vrrpSession_attribute::ADMIN_STATE_CHANGE;
			}

			if(0U != vrrpInterfaceRefAttribute.attrValuesNum)
			{
				// get vrrpInterface ref
				dataMO.vrrpv3InterfaceRef.assign( reinterpret_cast<char*>(vrrpInterfaceRefAttribute.attrValues[0]));
				dataMO.changeMask |= imm::vrrpSession_attribute::VRRP_INTERFACE_REF_CHANGE;
			}

			if(0U != prioSessionAttribute.attrValuesNum)
			{
				// get prioritizedSession value
				int32_t value = *reinterpret_cast<int32_t*>(prioSessionAttribute.attrValues[0]);
				dataMO.prioritizedSession = ( 0U == value ? imm::VRRP_TRUE : imm::VRRP_FALSE);
				dataMO.changeMask |= imm::vrrpSession_attribute::PRIO_SESSION_CHANGE;
			}

			if(0U != priorityAttribute.attrValuesNum)
			{
				// get priority value
				dataMO.priority = *reinterpret_cast<uint32_t*>(priorityAttribute.attrValues[0]);
				dataMO.changeMask |= imm::vrrpSession_attribute::PRIORITY_CHANGE;
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a VRRP Session\n"
						"#####    DN: <%s>\n"
						"#####    SMX ID: <%s>\n"
						"#####    VRRP Session ID : <%s>\n"
						"#####    Admin State : <%d>\n"
						"#####    Prior. Session : <%d>\n"
						"#####    Priority : <%d>\n",
						dataMO.getDN(), dataMO.getSmxId(), dataMO.getName(), dataMO.getAdminState(), dataMO.getPrioritizedSession(), dataMO.getPriority());
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add VRRP:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}
		}
		else
		{
			// getAttribute error
			// OmHandler error
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "OmHandler getAttribute(<%s>) failed!, errorCode:<%d> errorMsg:<%s>",
					objectDN.c_str(), m_objectManager.getInternalLastError(), m_objectManager.getInternalLastErrorText());
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadBfdSession(const std::string& objectDN)
	{
		operation::bfdSessionInfo dataMO;

		dataMO.moDN.assign(objectDN);
		dataMO.name.assign(common::utility::getIdValueFromRdn(objectDN));
		dataMO.smxId.assign( common::utility::getSmxIdFromBfdSessionDN(objectDN));

		fixs_ith::ErrorConstants result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

		if( fixs_ith::ERR_NO_ERRORS == result )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a BFD Session\n"
					"#####    Name: <%s>\n"
					"#####    DN: <%s>\n"
					"#####    SMX ID: <%s>\n",
					dataMO.getName(), dataMO.getDN(), dataMO.getSmxId());
		}
		else
		{
			// Error on adding data source
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add BFD:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
					dataMO.getDN(), result);
		}

		return result;
	}

	fixs_ith::ErrorConstants ConfigurationReloadHelper::loadBfdProfile(const std::string& objectDN)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "VrrpSession DN:<%s> ", objectDN.c_str());

		fixs_ith::ErrorConstants result = fixs_ith::ERR_GENERIC;

		//List of attributes to get
		std::vector<ACS_APGCC_ImmAttribute*> attributeList;

		ACS_APGCC_ImmAttribute detectionMultiAttribute;
		detectionMultiAttribute.attrName.assign(imm::bfdProfile_attribute::DETECTION_MULTI);
		attributeList.push_back(&detectionMultiAttribute);

		ACS_APGCC_ImmAttribute intervalTXMinAttribute;
		intervalTXMinAttribute.attrName.assign(imm::bfdProfile_attribute::INTERVAL_TX_MIN);
		attributeList.push_back(&intervalTXMinAttribute);

		ACS_APGCC_ImmAttribute intervalRXMinAttribute;
		intervalRXMinAttribute.attrName.assign(imm::bfdProfile_attribute::INTERVAL_RX_MIN);
		attributeList.push_back(&intervalRXMinAttribute);

		ACS_APGCC_ImmAttribute reservedByAttribute;
		reservedByAttribute.attrName.assign(imm::bfdProfile_attribute::RESERVED_BY);
		attributeList.push_back(&reservedByAttribute);

		ACS_CC_ReturnType getResult = m_objectManager.getAttribute(objectDN.c_str(), attributeList);

		if(getResult == ACS_CC_SUCCESS)
		{
			operation::bfdProfileInfo dataMO;
			dataMO.moDN.assign(objectDN);
			dataMO.name.assign(common::utility::getIdValueFromRdn(objectDN));
			dataMO.smxId.assign(common::utility::getSmxIdFromBfdProfileDN(objectDN));

			if(0U != detectionMultiAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::bfdProfile_attribute::DETECTION_MULTI_CHANGE;
				dataMO.detectionMultiplier = *(reinterpret_cast<uint32_t*>(detectionMultiAttribute.attrValues[0]));
			}

			if(0U != intervalTXMinAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::bfdProfile_attribute::INTERVAL_TX_MIN_CHANGE;
				dataMO.intervalTxMinDesired = *(reinterpret_cast<uint32_t*>(intervalTXMinAttribute.attrValues[0]));
				// convert ms to microsec
				dataMO.intervalTxMinDesired *= common::MS_TO_MICROSEC;
			}

			if(0U != intervalRXMinAttribute.attrValuesNum)
			{
				dataMO.changeMask |= imm::bfdProfile_attribute::INTERVAL_RX_MIN_CHANGE;
				dataMO.intervalRxMinRequired = *(reinterpret_cast<uint32_t*>(intervalRXMinAttribute.attrValues[0]));
				// convert ms to microsec
				dataMO.intervalRxMinRequired *= common::MS_TO_MICROSEC;
			}

			for(uint32_t index = 0U; index < reservedByAttribute.attrValuesNum; ++index)
			{
				dataMO.changeMask |= imm::bfdProfile_attribute::RESERVED_BY_CHANGE;
				dataMO.reservedBy.insert(reinterpret_cast<const char*>(reservedByAttribute.attrValues[index]));
			}

			result = fixs_ith::workingSet_t::instance()->getConfigurationManager().add(dataMO);

			if( fixs_ith::ERR_NO_ERRORS == result )
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Added a BFD Profile\n"
						"#####    DN: <%s>\n"
						"#####    BFD Profile ID : <%s>\n"
						"#####    detectionMultiplier : <%d>\n"
						"#####    intervalTxMinDesired : <%d>\n"
						"#####    intervalRxMinRequired : <%d>\n",
						dataMO.getDN(), dataMO.getName(), dataMO.getDetectionMultiplier(),
						dataMO.getIntervalTx(), dataMO.getIntervalRx() );
			}
			else
			{
				// Error on adding data source
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to add VrrpInterface:<%s>, DN:<%s>: errorCode:<%d>", dataMO.getName(),
						dataMO.getDN(), result);
			}

		}
		return result;
	}

	void ConfigurationReloadHelper::initializeClassNameMaps()
	{
		//Common classes
		m_commonClassNames.push_back(imm::moc_name::CLASS_VRRP_INTERFACE);
		m_commonClassNames.push_back(imm::moc_name::CLASS_ADDRESS);


		//Context Specific classes
		//L2 classes... Order MATTERS!!!
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_BRIDGE);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_ETH_PORT);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_AGGREGATOR);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_BRIDGE_PORT);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_VLAN);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_SUBNET_VLAN);

		//L3 classes... Order MATTERS!!!
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_ROUTER);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_BFD_PROFILE);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_BFD_SESSION);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_ACL);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_ACL_ENTRY);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_ROUTE_TABLE);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_DST);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_NEXTHOP);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_INTERFACE);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_ADDRESS);
		m_contextSpecificClassNames.push_back(imm::moc_name::CLASS_VRRP_SESSION);
	}

}//namespace


