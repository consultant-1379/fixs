/*
 * FIXS_CCH_BladePercistanceValue.cpp
 *
 *  Created on: Jun 4, 2012
 *      Author: eanform
 */

#include "FIXS_CCH_BladePercistanceValue.h"

namespace {
	const int BOARD_ACCESSIBLE = 3;
	const int BOARD_TURNED_ON = 1;
}

/*============================================================================
	ROUTINE: FIXS_CCH_BladePercistanceValue
 ============================================================================ */
FIXS_CCH_BladePercistanceValue::FIXS_CCH_BladePercistanceValue() :  acs_apgcc_objectimplementerinterface_V3(IMM_Util::IMPLEMENTER_BLADESWPERCISTANCE),
m_ImmClassName(IMM_Util::classBladeSwPersistance)
{
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
}

/*============================================================================
	ROUTINE: ~FIXS_CCH_BladePercistanceValue
 ============================================================================ */
FIXS_CCH_BladePercistanceValue::~FIXS_CCH_BladePercistanceValue()
{

	FIXS_CCH_logging = 0;
}

ACS_CC_ReturnType FIXS_CCH_BladePercistanceValue::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " class Name: " << className << endl;
	cout << " parent Name: " << parentname << endl;

	UNUSED(attr);
	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}

ACS_CC_ReturnType FIXS_CCH_BladePercistanceValue::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectDeleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;


	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}

ACS_CC_ReturnType FIXS_CCH_BladePercistanceValue::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectModifyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	UNUSED(attrMods);
	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}

ACS_CC_ReturnType FIXS_CCH_BladePercistanceValue::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << "COMPLETECallback " << endl;
	cout << endl;

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}

void FIXS_CCH_BladePercistanceValue::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbAbortCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << "ABORTCallback " << endl;
	cout << endl;
}

void FIXS_CCH_BladePercistanceValue::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbApplyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << "APPLYCallback " << endl;
	cout << endl;

}

ACS_CC_ReturnType FIXS_CCH_BladePercistanceValue::updateRuntime(const char* p_objName, const char** p_attrName)
{
	printf("------------------------------------------------------------\n");
	printf("                   updateRuntime called                     \n");
	printf("------------------------------------------------------------\n");

	cout<<"Object Name: "<<p_objName<<endl;
	cout<<"Attribute Name: "<<p_attrName<<endl;


	printf("------------------------------------------------------------\n");

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}


void FIXS_CCH_BladePercistanceValue::adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_InvocationType invocation,
		const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
		ACS_APGCC_AdminOperationParamType**paramList) {

	printf("------------------------------------------------------------\n");
	printf("                   adminOperationCallback called             \n");
	printf("------------------------------------------------------------\n");


	IMM_Util::BoardResetType type = IMM_Util::RESET_NORMAL;
	bool force = false;
	for (int i = 0; paramList[i] != 0; i++)
	{
		if (strcmp(paramList[i]->attrName, IMM_Util::PARAM_BLADESWPERSISTANCE_RESET_TYPE)==0)
		{
			type = *(reinterpret_cast<IMM_Util::BoardResetType *>(paramList[i]->attrValues));
		}
		
		if (strcmp(paramList[i]->attrName, IMM_Util::PARAM_BLADESWPERSISTANCE_FORCE)==0)
		{
			force = *(reinterpret_cast<bool *>(paramList[i]->attrValues));
		}
	}

	cout<<"oiHandle   : "<< oiHandle <<endl;
	cout<<"invocation : "<< invocation <<endl;
	cout<<"p_objName  : "<< p_objName <<endl;
	cout<<"operationId: "<< operationId <<endl;

	cout << " Implementing Admin Operation with ID = " << operationId << endl;

	std::string magAndSlot("");
	IMM_Util::getRdnValue(p_objName,magAndSlot);
	SaAisErrorT retVal = SA_AIS_OK;

	//############## ACTION 1 ----BLOCK

	if (operationId == 1)
	{
		cout << " ----------- ACTION 1 ----BLOCK-------------\n  " << endl;

		int res = boardBlock(magAndSlot, force);
		switch(res)
		{

		case EXECUTED:
			retVal = SA_AIS_OK;
			break;
		case BOARD_ALREADY_BLOCKED:
		{
					setAdminOperationError(BOARD_BLOCKED,oiHandle,invocation);
                                                        break;

		}
		case HARDWARE_NOT_PRESENT:
		{
					 setAdminOperationError(FAILED_HARDWARE_NOT_PRESENT,oiHandle,invocation);
			                                break;
		}
		case SNMP_FAILURE:
		{
					setAdminOperationError(FAILED_SNMP_FAILURE,oiHandle,invocation);
			                                break;
		}
		case FUNC_BOARD_NOTALLOWED:
		{
					setAdminOperationError(FAILED_FBN_NOT_ALLOWED,oiHandle,invocation);
			                                break;
		}
		case OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE:
		{

					setAdminOperationError(FAILURE_OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE,oiHandle,invocation);
                                                        break;
		}
		case CCH_ERR_IP_ADDRESS_CONVERSION:
                case CCH_ERR_DMX_MAGAZINE_NOT_FOUND:
                case CCH_ERR_DMX_TENANT_NOT_FOUND:
		case IRONSIDE_FAILURE:
		{
					
					setAdminOperationError(IRONSIDE_FAILURE,oiHandle,invocation);
                                                        break;
		}
		default:
			retVal = SA_AIS_ERR_FAILED_OPERATION;
			break;
		}
		

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : BLOCK ACTION",LOG_LEVEL_DEBUG);
		this->adminOperationResult( oiHandle , invocation, retVal );


	}

	//############## ACTION 2 ----DEBLOCK

	else if (operationId == 2)
	{
		cout << " ----------- ACTION 2 ----DEBLOCK-------------\n  " << endl;

		int res = boardDeblock(magAndSlot);
		switch(res)
		{
		case EXECUTED:
			retVal = SA_AIS_OK;
			break;
		case BOARD_ALREADY_DEBLOCKED:
		{
						setAdminOperationError(BOARD_DEBLOCKED,oiHandle,invocation);
			                        break;
		}
		case HARDWARE_NOT_PRESENT:
		{
						setAdminOperationError(FAILED_HARDWARE_NOT_PRESENT,oiHandle,invocation);
			                        break;
		}
		case SNMP_FAILURE:
		{
						 setAdminOperationError(FAILED_SNMP_FAILURE,oiHandle,invocation);
			                                break;
		}
		case FUNC_BOARD_NOTALLOWED:
		{
						 setAdminOperationError(FAILED_FBN_NOT_ALLOWED,oiHandle,invocation);
			                                break;
		}
		case CCH_ERR_IP_ADDRESS_CONVERSION:
                case CCH_ERR_DMX_MAGAZINE_NOT_FOUND:
                case CCH_ERR_DMX_TENANT_NOT_FOUND:
                case IRONSIDE_FAILURE:
                {
					
                                        	setAdminOperationError(IRONSIDE_FAILURE,oiHandle,invocation);
                                                        break;
                }
		default:
			retVal = SA_AIS_ERR_FAILED_OPERATION;
			break;
		}



		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : DEBLOCK ACTION",LOG_LEVEL_DEBUG);
		this->adminOperationResult( oiHandle , invocation, retVal );


	}


	//############## ACTION 3 ----RESET

	else if (operationId == 3)
	{
		std::cout << " ----------- ACTION 3 ----RESET-------------\n  " << std::endl;
		std::cout<<"Reset Type: "<< type << std::endl;


		int res = resetBoard(magAndSlot, type);
		switch(res)
		{
			case EXECUTED:

				retVal = SA_AIS_OK;
				this->adminOperationResult( oiHandle , invocation, retVal );
				break;

			case HARDWARE_NOT_PRESENT:
			 {
			                                setAdminOperationError(FAILED_HARDWARE_NOT_PRESENT,oiHandle,invocation);
			                                break;
			                              }

			case SNMP_FAILURE:
			  {
			                                setAdminOperationError(FAILED_SNMP_FAILURE,oiHandle,invocation);
			                                break;
			                              }

			case FUNC_BOARD_NOTALLOWED:
			  {
			                                setAdminOperationError(FAILED_FBN_NOT_ALLOWED,oiHandle,invocation);
			                                break;
			                              }

			case BOARD_ALREADY_BLOCKED:
			{
							 setAdminOperationError(BOARD_BLOCKED_NO_DEFAULT_RESET,oiHandle,invocation);
			                                break;

			}
			case OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE:
			{
						     setAdminOperationError(FAILURE_OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE,oiHandle,invocation);
                                                        break;
			}
			
			case CCH_ERR_IP_ADDRESS_CONVERSION:
			case CCH_ERR_DMX_MAGAZINE_NOT_FOUND:
			case CCH_ERR_DMX_TENANT_NOT_FOUND:
			case IRONSIDE_FAILURE:
			{
							setAdminOperationError(IRONSIDE_FAILURE,oiHandle,invocation);
                                                        break;
			}
			default:
				retVal = SA_AIS_ERR_FAILED_OPERATION;
				break;
		}
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : RESET ACTION",LOG_LEVEL_DEBUG);
		this->adminOperationResult( oiHandle , invocation, retVal );


	}

	//############## NO ACTION FOR THIS NUMBER

	else
	{
		retVal = SA_AIS_ERR_NO_OP;
		cout << " ..... NO ACTION FOR THIS NUMBER  \n "<< endl;
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : NO ACTION FOR THIS NUMBER ",LOG_LEVEL_ERROR);
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;
	}

	cout << "---------------------------END ACTION-------------------------\n" << endl;

}


int FIXS_CCH_BladePercistanceValue::resetBoard (std::string dn_value, IMM_Util::BoardResetType type)
{
	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	std::string dn_blade("");
	cout << "dn_value: " << dn_value.c_str() << endl;

	//get mag and slot
	std::string sMag = ACS_APGCC::before(dn_value,"_");
	std::string sSlot =ACS_APGCC::after(dn_value,"_");

	//convert mag and slot
	CCH_Util::stringToUlongMagazine(sMag,umagazine);
	CCH_Util::stringToUshort(sSlot,uslot);

	cout << "mag: " << sMag.c_str() << "slot: " << sSlot.c_str()<< endl;

	if (FIXS_CCH_UpgradeManager::getInstance()->isScxData(umagazine,uslot))
	{
			return resetBoardSCXB(umagazine,uslot, type);
	}
	else if (FIXS_CCH_UpgradeManager::getInstance()->isCmxData(umagazine,uslot))
	{
			return resetBoardCMXB(umagazine,uslot, type);
	}
	else if (FIXS_CCH_UpgradeManager::getInstance()->isIptData(umagazine,uslot))
	{
		     return resetBoardIPTB(umagazine,uslot,type);
	}
	else if (FIXS_CCH_UpgradeManager::getInstance()->isIplbData(umagazine,uslot))
	{
		if((type == IMM_Util::RESET_NORMAL) ||(type == IMM_Util::RESET_NETWORK_BOOT))
		{
		    int resp = 0;
		    unsigned short islot=65;
                    IMM_Interface::getOtherBoardSlot(umagazine,uslot,islot);
		    resp = FIXS_CCH_UpgradeManager::getInstance()->checkIplbSupervision(umagazine, uslot, islot);
		    if(resp == 0) 
		    {
			return resetBoardIPLB(umagazine,uslot, type);
		    }
		    else
		    {
		      return resp;
		    }
		}
		else if((type == IMM_Util::RESET_FACTORY_DEFAULT))
		     return FUNC_BOARD_NOTALLOWED;
		else
		{
		    return resetBoardIPLB(umagazine,uslot, type);
		}
	}
	else if (FIXS_CCH_UpgradeManager::getInstance()->isSmxData(umagazine,uslot))
	{
		return resetBoardSMXB(umagazine,uslot, type);
	}
	else
	{
		return FUNC_BOARD_NOTALLOWED;
	}
	return 1;
}

int FIXS_CCH_BladePercistanceValue::resetBoardIPTB (unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type)
{
	CCH_Util::EnvironmentType environment = FIXS_CCH_DiskHandler::getEnvironment();

	int slot = 0;
	std::string sSlot("");

	std::string Master_Ip("");
	unsigned short Master_slot = 0;

	std::string shelfMgrIpA("");
	std::string shelfMgrIpB("");
	std::string Passive_Ip("");

	std::string oidBladePower("");
	std::string oidBladeRestart("");
	std::string oidBladeReset("");

	int boardPresence = 0;
	int res = 0;
	int boardPower = 0;

	std::string ipA_to_reset("");
	std::string ipB_to_reset("");

	CCH_Util::ushortToString(uslot,sSlot);
	CCH_Util::stringToInt(sSlot, slot);

	//get IPT board IPs
	FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,uslot,ipA_to_reset,ipB_to_reset);

	std::cout << "env type : " << environment << std::endl;

	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA) || (environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX)) //TR_HW29936
	{
		//get Master board IP and slot
		FIXS_CCH_UpgradeManager::getInstance()->getMasterPassiveSCX(umagazine,Master_Ip,Passive_Ip,Master_slot);

		//get Master board IPs
		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,Master_slot,shelfMgrIpA,shelfMgrIpB);

		std::cout << __FUNCTION__ << "@" << __LINE__ << " Master IP adress: " << Master_Ip.c_str() << " Master Slot: "<< Master_slot << std::endl;


		FIXS_CCH_SNMPManager *snmpReader =NULL;
		snmpReader = new FIXS_CCH_SNMPManager(Master_Ip);

		if (!snmpReader)
			return SNMP_FAILURE;

		boardPresence=snmpReader->getBoardPresence(shelfMgrIpA,slot);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " ResultBoardPresence: " << boardPresence << std::endl;

		if(boardPresence != 3)
		{
			boardPresence=snmpReader->getBoardPresence(shelfMgrIpB,slot);

			if (boardPresence != 3)
			{
				delete snmpReader;
				snmpReader = NULL;
				return HARDWARE_NOT_PRESENT;
			}
			else
			{
				boardPower=snmpReader->getBoardPower(shelfMgrIpB,slot);
			}

		}
		else
		{
			boardPower=snmpReader->getBoardPower(shelfMgrIpA,slot);
		}
		std::cout << __FUNCTION__ << "@" << __LINE__ << " ResultBoardPower: " << boardPower << std::endl;

		//Check if the board is powered
		if(boardPower == 0) // The board is turned off
		{
			//case BOARD_BLOCKED_NO_DEFAULT_RESET
			//std::cout<<"Board cannot be reset to factory default as it is blocked"<<std::endl;
			delete snmpReader;
			snmpReader = NULL;
			return BOARD_ALREADY_BLOCKED;
		}
		//if the board is turned-on, give a cold restart
		else if (boardPower == 1)
		{

			if (type == IMM_Util::RESET_NETWORK_BOOT)
			{
				//Set the networkboot info here for SCXB environment

				std::string netBootSet = "FF 00 00 00 02 00 00 00";
				std::string slotString("");
				CCH_Util::ushortToString((unsigned short)uslot, slotString);

				if (!snmpReader->setGprDataValue(netBootSet,sSlot,"512"))
				{
					if(snmpReader)
					{
						delete snmpReader;
						snmpReader = NULL;
					}
					return SNMP_FAILURE;
				}
				//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Leaving after setting the networkboot info here for SCXB" <<std::endl;


				const char* val = "0";
				if( snmpReader->queryBoardPresencePower(2,oidBladeRestart)!= false )
				{
					std::string oidBladeRestart_slot = oidBladeRestart + ".0";
					res = snmpReader->snmpSet("NETMAN", ipA_to_reset.c_str(), oidBladeRestart_slot.c_str(), 'i', val);

					std::cout << __FUNCTION__ << "@" << __LINE__ << " OID_Cold_Restart: " << oidBladeRestart_slot.c_str() << std::endl;

					if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on ipA: " << oidBladeRestart_slot.c_str() << std::endl;
						res = snmpReader->snmpSet("NETMAN", ipB_to_reset.c_str(), oidBladeRestart_slot.c_str(), 'i', val);

						std::cout << __FUNCTION__ << "@" << __LINE__ << " OID_Cold_Restart: " << oidBladeRestart_slot.c_str() << std::endl;

					}

					snmpReader->queryBoardPresencePower(4,oidBladeReset);

					//std::string oidBladeReset_slot = oidBladeReset + ".0";

					char* oidBladeReset_slot= (char*)malloc(100);
					memset(oidBladeReset_slot,0,100);
					sprintf(oidBladeReset_slot,"%s.%d",oidBladeReset.c_str(),slot);

					res = snmpReader->snmpSet("NETMAN",shelfMgrIpA.c_str(), oidBladeReset_slot, 'i', val);
					if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
					{
						res = snmpReader->snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladeReset_slot, 'i', val);

					}

					free(oidBladeReset_slot);

				}

			}
			else if (type == IMM_Util::RESET_NORMAL)
			{
				//NORMAL RESET
				const char* val = "0";
				if( snmpReader->queryBoardPresencePower(2,oidBladeRestart)!= false )
				{
					std::string oidBladeRestart_slot = oidBladeRestart + ".0";
					res = snmpReader->snmpSet("NETMAN", ipA_to_reset.c_str(), oidBladeRestart_slot.c_str(), 'i', val);

					std::cout << __FUNCTION__ << "@" << __LINE__ << " OID_Cold_Restart: " << oidBladeRestart_slot.c_str() << std::endl;

					if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on ipA: " << oidBladeRestart_slot.c_str() << std::endl;
						res = snmpReader->snmpSet("NETMAN", ipB_to_reset.c_str(), oidBladeRestart_slot.c_str(), 'i', val);

						std::cout << __FUNCTION__ << "@" << __LINE__ << " OID_Cold_Restart: " << oidBladeRestart_slot.c_str() << std::endl;

					}

					snmpReader->queryBoardPresencePower(4,oidBladeReset);

					//std::string oidBladeReset_slot = oidBladeReset + ".0";

					char* oidBladeReset_slot= (char*)malloc(100);
					memset(oidBladeReset_slot,0,100);
					sprintf(oidBladeReset_slot,"%s.%d",oidBladeReset.c_str(),slot);

					res = snmpReader->snmpSet("NETMAN",shelfMgrIpA.c_str(), oidBladeReset_slot, 'i', val);
					if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
					{
						res = snmpReader->snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladeReset_slot, 'i', val);

					}

					free(oidBladeReset_slot);

				}
			}
			else 
			{
				delete snmpReader;
				snmpReader = NULL;
				return FUNC_BOARD_NOTALLOWED;
			}
		}
		else
		{
			delete snmpReader;
			snmpReader = NULL;
			return SNMP_FAILURE;
		}

		delete snmpReader;
		snmpReader = NULL;
		return EXECUTED;
	}
	else if (environment == CCH_Util::MULTIPLECP_NOTCBA)
	{
		//SCB-RP
		//get Master board IP and slot
		std::cout << "Not CBA " << std::endl;
		FIXS_CCH_UpgradeManager::getInstance()->getMasterPassiveSCB(umagazine,shelfMgrIpA,shelfMgrIpB,Master_slot);

		std::cout << __FUNCTION__ << "@" << __LINE__ << " Master IP adress: " << shelfMgrIpA.c_str() << " Master Slot: "<< Master_slot << std::endl;


		FIXS_CCH_SNMPManager *snmpReader =NULL;
		snmpReader = new FIXS_CCH_SNMPManager(shelfMgrIpA);

		if (!snmpReader)
			return SNMP_FAILURE;

		boardPower = -1;
		boardPower=snmpReader->getXshmcBoardPowerOff(shelfMgrIpA,slot);
		std::cout << __FUNCTION__ << "@" << __LINE__ << " ResultBoardPower: " << boardPower << std::endl;

		//Check if the board is powered
		if(boardPower == 1) // The board is turned off
		{
			//case BOARD_BLOCKED_NO_DEFAULT_RESET
			//std::cout<<"Board cannot be reset to factory default as it is blocked"<<std::endl;
			delete snmpReader;
			snmpReader = NULL;
			return BOARD_ALREADY_BLOCKED;
		}
		//if the board is turned-on, give a cold restart
		else if (boardPower == 0)
		{
			std::string oidXshmcBoardReset = "1.3.6.1.4.1.193.154.2.1.2.1.1.1.9";
			std::string oidXshmcGPRData = "1.3.6.1.4.1.193.154.2.1.2.2.1.1.2";

			if (type == IMM_Util::RESET_NETWORK_BOOT)
			{
				//Set the networkboot info here for SCB-RP environment
				std::string GPRDataValue = "";
				std::string mask = "FF000000";
				std::string bootTypeString = "02000000";

				if(!snmpReader->getXshmcGprDataValue(&GPRDataValue, sSlot, "128"))
				{
					if(snmpReader)
					{
						delete snmpReader;
						snmpReader = NULL;
					}
					return SNMP_FAILURE;
				}

				size_t lookHere = 0;
				size_t foundHere;

				while((foundHere = mask.find('F', lookHere)) != string::npos)
				{
					lookHere++;
					GPRDataValue[(int)foundHere] = bootTypeString[(int)foundHere];
				}

				if(!snmpReader->setXshmcGprDataValue(GPRDataValue, sSlot, "128"))
				{
					if(snmpReader)
					{
						delete snmpReader;
						snmpReader = NULL;
					}
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMP set XshmcGprDataValue successful, value = "<< GPRDataValue << std::endl;
					return SNMP_FAILURE;
				}

				//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Leaving after setting the networkboot info here for SCXB" <<std::endl;

				const char* val = "1";
				char* oidBladeReset_slot= (char*)malloc(100);
				memset(oidBladeReset_slot,0,100);
				sprintf(oidBladeReset_slot,"%s.%d",oidXshmcBoardReset.c_str(),slot);

				res = snmpReader->snmpSet("NETMAN",shelfMgrIpA.c_str(), oidBladeReset_slot, 'i', val);
				if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
				{
					res = snmpReader->snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladeReset_slot, 'i', val);
					if(res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
					{
						std:: cout << "Board reset FAILED at slot : " << slot << std::endl;
						free(oidBladeReset_slot);
						delete snmpReader;
						snmpReader = NULL;
						return FUNC_BOARD_NOTALLOWED;
					}
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMP set XshmcBoardReset successful " << std::endl;
				}
				std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMP set XshmcBoardReset successful " << std::endl;

				free(oidBladeReset_slot);
			}
			else if (type == IMM_Util::RESET_NORMAL)
			{
				//NORMAL RESET
				const char* val = "1";
				char* oidBladeReset_slot= (char*)malloc(100);
				memset(oidBladeReset_slot,0,100);
				sprintf(oidBladeReset_slot,"%s.%d",oidXshmcBoardReset.c_str(),slot);

				res = snmpReader->snmpSet("NETMAN",shelfMgrIpA.c_str(), oidBladeReset_slot, 'i', val);
				if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
				{
					res = snmpReader->snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladeReset_slot, 'i', val);
					if(res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
					{
						std:: cout << "Board reset FAILED at slot : " << slot << std::endl;
						free(oidBladeReset_slot);
						delete snmpReader;
						snmpReader = NULL;
						return FUNC_BOARD_NOTALLOWED;
					}
					std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMP set XshmcBoardReset successful " << std::endl;
				}
				std::cout << __FUNCTION__ << "@" << __LINE__ << " SNMP set XshmcBoardReset successful " << std::endl;
				free(oidBladeReset_slot);
			}
			else //if (type == IMM_Util::RESET_FACTORY_DEFAULT)
			{
				delete snmpReader;
				snmpReader = NULL;
				return FUNC_BOARD_NOTALLOWED;
			}
		}
		else
		{
			delete snmpReader;
			snmpReader = NULL;
			return SNMP_FAILURE;
		}

		delete snmpReader;
		snmpReader = NULL;
		return EXECUTED;

	}

	else return ILLEGAL_COMMAND;

}

int FIXS_CCH_BladePercistanceValue::resetBoardIPLB (unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type)
{
        
	std::string Master_Ip("");
	std::string Passive_Ip("");
	std::string shelfMgrIpA("");
	std::string shelfMgrIpB("");

	 uint32_t dmx_addr1[2];
        FIXS_CCH_UpgradeManager::getInstance()->getDmxAddress(dmx_addr1[0], dmx_addr1[1]);

	FIXS_CCH_IronsideManager *ironsideMgr =NULL;
	ironsideMgr = new FIXS_CCH_IronsideManager(dmx_addr1[0], dmx_addr1[1]);
	std::cout << "Resetting the IPLB board";
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : IPLB Board Block ",LOG_LEVEL_DEBUG);
	int reset_result = ironsideMgr->reset_board(umagazine, uslot, type);
	if(reset_result != 0)
	{
		std::cout << "Resetting board failed";
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : IPLB Board Reset failed ",LOG_LEVEL_ERROR);
		delete ironsideMgr;
		ironsideMgr = NULL;
		return reset_result ;
	}

	std::cout << "Resetting the IPLB board success";
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : IPLB Board Block Success ",LOG_LEVEL_DEBUG);
	delete ironsideMgr;
	ironsideMgr = NULL;
	return EXECUTED;
}


int FIXS_CCH_BladePercistanceValue::resetBoardSCXB(unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type)
{

	int slot = 0;
	int otherslot = 0;
	std::string sSlot("");
	int boardPresence = 0;
	int res = 0;
	int boardPower = 0;
	std::string shelfparent("");
	std::string dn_otherblade("");
	std::string oidBladePower("");
	std::string oidBladeRestart("");
	std::string oidBladeReset("");
	std::vector<std::string> list;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " start ResetBoard method " << std::endl;

	std::string shelfMgrIpA("");
	std::string shelfMgrIpB("");
	std::string ip1_EthA("");
	std::string ip1_EthB("");
	std::string ip2_EthA("");
	std::string ip2_EthB("");

	std::string ipA_to_reset("");
	std::string ipB_to_reset("");

	CCH_Util::ushortToString(uslot,sSlot);
	CCH_Util::stringToInt(sSlot, slot);

	//Get IpA,IpB
	if (slot == 0)
	{
		otherslot= 25;

		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,uslot,ip1_EthA,ip1_EthB);
		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,otherslot,ip2_EthA,ip2_EthB);

		ipA_to_reset = ip1_EthA;
		ipB_to_reset = ip1_EthB;
	}
	else
	{
		otherslot= 0;
		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,uslot,ip2_EthA,ip2_EthB);
		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,otherslot,ip1_EthA,ip1_EthB);

		ipA_to_reset = ip2_EthA;
		ipB_to_reset = ip2_EthB;
	}


	std::cout << __FUNCTION__ << "@" << __LINE__ << " IP adress: " << ip1_EthA << std::endl;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " IP adress: " << ip2_EthA << std::endl;


	FIXS_CCH_SNMPManager *snmpReader =NULL;
	snmpReader = new FIXS_CCH_SNMPManager(ip1_EthA);

	if (!snmpReader)
		return SNMP_FAILURE;

	int resSnmp = 0;


	resSnmp = snmpReader->isMaster(ip1_EthA);

	if (resSnmp == 1)//master
	{
		shelfMgrIpA = ip1_EthA;
		shelfMgrIpB = ip1_EthB;
	}
	else if (resSnmp == 2)//passive
	{
		shelfMgrIpA = ip2_EthA;
		shelfMgrIpB = ip2_EthB;
	}
	else if (resSnmp == HARDWARE_NOT_PRESENT)
	{
		resSnmp = snmpReader->isMaster(ip2_EthB);

		if (resSnmp == 1)//master
		{
			shelfMgrIpA = ip2_EthA;
			shelfMgrIpB = ip2_EthB;
		}
		else if (resSnmp == 2)//passive
		{
			shelfMgrIpA = ip1_EthA;
			shelfMgrIpB = ip1_EthB;
		}
		else if (resSnmp == HARDWARE_NOT_PRESENT)
		{
			delete snmpReader;
			snmpReader = NULL;
			return SNMP_FAILURE;
		}
	}

	boardPresence=snmpReader->getBoardPresence(shelfMgrIpA,slot);


	std::cout << __FUNCTION__ << "@" << __LINE__ << " ResultBoardPresence: " << boardPresence << std::endl;


	if(boardPresence != 3)
	{
		boardPresence = snmpReader->getBoardPresence(shelfMgrIpB,slot);

		if (boardPresence != 3)
		{
			delete snmpReader;
			snmpReader = NULL;
			return HARDWARE_NOT_PRESENT;
		}
		else
		{
			boardPower=snmpReader->getBoardPower(shelfMgrIpB,slot);
			//break;
		}

	}
	else
	{
		boardPower=snmpReader->getBoardPower(shelfMgrIpA,slot);
	}
	//}


	std::cout << __FUNCTION__ << "@" << __LINE__ << " ResultBoardPower: " << boardPower << std::endl;
	//Check if the board is powered
	if(boardPower == 0) // The board is turned off
	{
		if (type == IMM_Util::RESET_FACTORY_DEFAULT)
		{
			//TODO
			//case BOARD_BLOCKED_NO_DEFAULT_RESET
			//std::cout<<"Board cannot be reset to factory default as it is blocked"<<std::endl;
			delete snmpReader;
			snmpReader = NULL;
			return BOARD_BLOCKED_NO_DEFAULT_RESET;
		}
		else if (type == IMM_Util::RESET_NORMAL)
		{
			//RESET_NORMAL
			//set the proper OID for blade power
			if(snmpReader->queryBoardPresencePower(1,oidBladePower)!=false)
			{
				const char* val = "1";
				//std::string oidBladePower_slot = oidBladePower + ".1";
				char* oidBladePower_slot= (char*)malloc(100);
				memset(oidBladePower_slot,0,100);
				sprintf(oidBladePower_slot,"%s.%d",oidBladePower.c_str(),slot);

				//Turn-on the board
				res = snmpReader->snmpSet("NETMAN", shelfMgrIpA.c_str(), oidBladePower_slot, 'i', val);
				if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on ipA: " << oidBladePower_slot << std::endl;
					res = snmpReader->snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladePower_slot, 'i', val);
					if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on IpB: " << oidBladePower_slot << std::endl;
						delete snmpReader;
						snmpReader = NULL;
						free(oidBladePower_slot);
						return SNMP_FAILURE;
					}
				}
				free(oidBladePower_slot);
			}
			else
			{
				delete snmpReader;
				snmpReader = NULL;
				return SNMP_FAILURE;
			}
		}
		else
		{
			delete snmpReader;
			snmpReader = NULL;
			return FUNC_BOARD_NOTALLOWED;
		}
	}
	//if the board is turned-on, give a cold restart
	else if (boardPower == 1)
	{
		if (type == IMM_Util::RESET_FACTORY_DEFAULT)
		{
			//Writing a non-zero value to this object will force the blade to factory default
			const char* val = "1";
			if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpReader->snmpSet("NETMAN", ipA_to_reset.c_str(), snmpReader->getBladeReturnToFactoryDefaultOid().c_str(), 'i', val))
			{
				if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpReader->snmpSet("NETMAN", ipB_to_reset.c_str(), snmpReader->getBladeReturnToFactoryDefaultOid().c_str(), 'i', val))
				{
					delete snmpReader;
					snmpReader = NULL;
					return SNMP_FAILURE;
				}
			}
		}
		else if (type == IMM_Util::RESET_NORMAL)
		{
			//RESET_NORMAL
			const char* val = "0";
			if( snmpReader->queryBoardPresencePower(2,oidBladeRestart)!= false )
			{
				std::string oidBladeRestart_slot = oidBladeRestart + ".0";
				res = snmpReader->snmpSet("NETMAN", ipA_to_reset.c_str(), oidBladeRestart_slot.c_str(), 'i', val);

				std::cout << __FUNCTION__ << "@" << __LINE__ << " OID_Cold_Restart: " << oidBladeRestart_slot.c_str() << std::endl;



				if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on ipA: " << oidBladeRestart_slot.c_str() << std::endl;
					res = snmpReader->snmpSet("NETMAN", ipB_to_reset.c_str(), oidBladeRestart_slot.c_str(), 'i', val);

					std::cout << __FUNCTION__ << "@" << __LINE__ << " OID_Cold_Restart: " << oidBladeRestart_slot.c_str() << std::endl;

					/*	if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on IpB: " << oidBladeRestart_slot.c_str() << std::endl;
					return SNMP_FAILURE;
				}*/
				}

				snmpReader->queryBoardPresencePower(4,oidBladeReset);

				//std::string oidBladeReset_slot = oidBladeReset + ".0";

				char* oidBladeReset_slot= (char*)malloc(100);
				memset(oidBladeReset_slot,0,100);
				sprintf(oidBladeReset_slot,"%s.%d",oidBladeReset.c_str(),slot);

				res = snmpReader->snmpSet("NETMAN",shelfMgrIpA.c_str(), oidBladeReset_slot, 'i', val);
				if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
				{
					res = snmpReader->snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladeReset_slot, 'i', val);

				}

				free(oidBladeReset_slot);

			}
		}
		else 
		{
			delete snmpReader;
			snmpReader = NULL;
			return FUNC_BOARD_NOTALLOWED;
		}

	}

	else
	{
//		if (traceObj->ACS_TRA_ON())
//		{
//			char tmpStr[512] = {0};
//			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] isMaster failed", __FUNCTION__, __LINE__);
//			traceObj->ACS_TRA_event(1,tmpStr);
//		}
		delete snmpReader;
		snmpReader = NULL;
		return SNMP_FAILURE;
	}

	delete snmpReader;
	snmpReader = NULL;
	return EXECUTED;
}

int FIXS_CCH_BladePercistanceValue::resetBoardSMXB(unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type)
{

	int slot = 0;
	int otherslot = 0;
	std::string sSlot("");
	int boardPresence = 0;
	int res = 0;
	int boardPower = 0;
	std::string shelfparent("");
	std::string dn_otherblade("");
	std::string oidBladePower("");
	std::string oidBladeRestart("");
	std::string oidBladeReset("");
	std::vector<std::string> list;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " start ResetBoard method " << std::endl;

	std::string shelfMgrIpA("");
	std::string shelfMgrIpB("");
	std::string ip1_EthA("");
	std::string ip1_EthB("");
	std::string ip2_EthA("");
	std::string ip2_EthB("");

	std::string ipA_to_reset("");
	std::string ipB_to_reset("");

	CCH_Util::ushortToString(uslot,sSlot);
	CCH_Util::stringToInt(sSlot, slot);

	//Get IpA,IpB
	if (slot == 0)
	{
		otherslot= 25;

		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,uslot,ip1_EthA,ip1_EthB);
		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,otherslot,ip2_EthA,ip2_EthB);

		ipA_to_reset = ip1_EthA;
		ipB_to_reset = ip1_EthB;
	}
	else
	{
		otherslot= 0;
		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,uslot,ip2_EthA,ip2_EthB);
		FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine,otherslot,ip1_EthA,ip1_EthB);

		ipA_to_reset = ip2_EthA;
		ipB_to_reset = ip2_EthB;
	}


	std::cout << __FUNCTION__ << "@" << __LINE__ << " IP adress: " << ip1_EthA << std::endl;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " IP adress: " << ip2_EthA << std::endl;


	FIXS_CCH_SNMPManager *snmpReader =NULL;
	snmpReader = new FIXS_CCH_SNMPManager(ip1_EthA);

	if (!snmpReader)
	{
		std::cout<<"Unable to create SNMP reader obj for ipp addr "<<ip1_EthA;
		return SNMP_FAILURE;
	}

	int resSnmp = 0;


	resSnmp = snmpReader->isMaster(ip1_EthA);

	if (resSnmp == 1)//master
	{
		shelfMgrIpA = ip1_EthA;
		shelfMgrIpB = ip1_EthB;
	}
	else if (resSnmp == 2)//passive
	{
		shelfMgrIpA = ip2_EthA;
		shelfMgrIpB = ip2_EthB;
	}
	else if (resSnmp == HARDWARE_NOT_PRESENT)
	{
		resSnmp = snmpReader->isMaster(ip2_EthB);

		if (resSnmp == 1)//master
		{
			shelfMgrIpA = ip2_EthA;
			shelfMgrIpB = ip2_EthB;
		}
		else if (resSnmp == 2)//passive
		{
			shelfMgrIpA = ip1_EthA;
			shelfMgrIpB = ip1_EthB;
		}
		else if (resSnmp == HARDWARE_NOT_PRESENT)
		{
			delete snmpReader;
			snmpReader = NULL;
			std::cout<<"Delete snmpReader because HARDWARE_NOT_PRESENT"<<endl;
			return SNMP_FAILURE;
		}
	}

	boardPresence=snmpReader->getBoardPresence(shelfMgrIpA,slot);


	std::cout << __FUNCTION__ << "@" << __LINE__ << " ResultBoardPresence: " << boardPresence << std::endl;


	if(boardPresence != 3)
	{
		boardPresence = snmpReader->getBoardPresence(shelfMgrIpB,slot);

		if (boardPresence != 3)
		{
			delete snmpReader;
			snmpReader = NULL;
			return HARDWARE_NOT_PRESENT;
		}
		else
		{
			boardPower=snmpReader->getBoardPower(shelfMgrIpB,slot);
			//break;
		}

	}
	else
	{
		boardPower=snmpReader->getBoardPower(shelfMgrIpA,slot);
	}
	//}


	std::cout << __FUNCTION__ << "@" << __LINE__ << " ResultBoardPower: " << boardPower << std::endl;
	//Check if the board is powered
	if(boardPower == 0) // The board is turned off
	{
		if (type == IMM_Util::RESET_FACTORY_DEFAULT)
		{
			//TODO
			//case BOARD_BLOCKED_NO_DEFAULT_RESET
			//std::cout<<"Board cannot be reset to factory default as it is blocked"<<std::endl;
			delete snmpReader;
			snmpReader = NULL;
			return BOARD_BLOCKED_NO_DEFAULT_RESET;
		}
		else if (type == IMM_Util::RESET_NORMAL)
		{
			//RESET_NORMAL
			//set the proper OID for blade power
			if(snmpReader->queryBoardPresencePower(1,oidBladePower)!=false)
			{
				const char* val = "1";
				//std::string oidBladePower_slot = oidBladePower + ".1";
				char* oidBladePower_slot= (char*)malloc(100);
				memset(oidBladePower_slot,0,100);
				sprintf(oidBladePower_slot,"%s.%d",oidBladePower.c_str(),slot);

				//Turn-on the board
				res = snmpReader->snmpSet("NETMAN", shelfMgrIpA.c_str(), oidBladePower_slot, 'i', val);
				if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on ipA: " << oidBladePower_slot << std::endl;
					res = snmpReader->snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladePower_slot, 'i', val);
					if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
					{
						std::cout << __FUNCTION__ << "@" << __LINE__ << " Res: " << res << " SET failed on IpB: " << oidBladePower_slot << std::endl;
						delete snmpReader;
						snmpReader = NULL;
						free(oidBladePower_slot);
						return SNMP_FAILURE;
					}
				}
				free(oidBladePower_slot);
			}
			else
			{
				delete snmpReader;
				snmpReader = NULL;
				return SNMP_FAILURE;
			}
		}
		else
		{
			delete snmpReader;
			snmpReader = NULL;
			return FUNC_BOARD_NOTALLOWED;
		}
	}
	//if the board is turned-on, give a cold restart
	else if (boardPower == 1)
	{
		if (type == IMM_Util::RESET_FACTORY_DEFAULT)
		{
			//Writing a non-zero value to this object will force the blade to factory default
			const char* val = "1";
			if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpReader->snmpSet("NETMAN", ipA_to_reset.c_str(), snmpReader->getBladeReturnToFactoryDefaultOid().c_str(), 'i', val))
			{
				if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpReader->snmpSet("NETMAN", ipB_to_reset.c_str(), snmpReader->getBladeReturnToFactoryDefaultOid().c_str(), 'i', val))
				{
					delete snmpReader;
					snmpReader = NULL;
					return SNMP_FAILURE;
				}
			}
		}
		else if (type == IMM_Util::RESET_NORMAL)
		{
			//RESET_NORMAL
			const char* val = "0";
			if( snmpReader->queryBoardPresencePower(4,oidBladeReset)!= false )
			{
				char* oidBladeReset_slot= (char*)malloc(100);
				memset(oidBladeReset_slot,0,100);
				sprintf(oidBladeReset_slot,"%s.%d",oidBladeReset.c_str(),slot);

				res = snmpReader->snmpSet("NETMAN",shelfMgrIpA.c_str(), oidBladeReset_slot, 'i', val);
				if (res != FIXS_CCH_SNMPManager::GETSET_SNMP_OK)
				{
					res = snmpReader->snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladeReset_slot, 'i', val);

				}

				free(oidBladeReset_slot);

			}
		}
		else
		{
			delete snmpReader;
			snmpReader = NULL;
			return FUNC_BOARD_NOTALLOWED;
		}

	}

	else
	{
//		if (traceObj->ACS_TRA_ON())
//		{
//			char tmpStr[512] = {0};
//			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] isMaster failed", __FUNCTION__, __LINE__);
//			traceObj->ACS_TRA_event(1,tmpStr);
//		}
		delete snmpReader;
		snmpReader = NULL;
		return SNMP_FAILURE;
	}

	delete snmpReader;
	snmpReader = NULL;
	return EXECUTED;
}

int FIXS_CCH_BladePercistanceValue::resetBoardCMXB (unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ::::::::::::::::::::::::::::::::::::::::::::::::::: " << std::endl;

	//Get IP Addresses of CMX board
	std::string cmxEthA, cmxEthB;
	if (!FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine, uslot, cmxEthA, cmxEthB))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " HARDWARE_NOT_PRESENT" << std::endl;
		return HARDWARE_NOT_PRESENT;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " CMX EthA:" << cmxEthA << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " CMX EthB:" << cmxEthB << std::endl;

	//Get IP Addresses of Shelf Manager
	std::string shelfMgrIpA, shelfMgrIpB;
	if (!discoverShelfManager(umagazine, shelfMgrIpA, shelfMgrIpB))
	{
		//TODO Trace
		std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " ERROR" << std::endl;
		return SNMP_FAILURE;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " SCX EthA:" << shelfMgrIpA << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " SCX EthB:" << shelfMgrIpB << std::endl;

	//Check for board presence
	if (!isBoardAccessible(shelfMgrIpA, shelfMgrIpB, uslot))
	{
		//TODO Trace
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ERROR" << std::endl;
		return SNMP_FAILURE;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Board is ACCESSIBLE!!" << std::endl;

	//Get IP addresses for CMX board
	FIXS_CCH_SNMPManager snmpManager(cmxEthA);

	if (isBoardTurnedOn(shelfMgrIpA, shelfMgrIpB, uslot))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Board is TURNED ON!!" << std::endl;
		//Board Turned On
		if (type == IMM_Util::RESET_FACTORY_DEFAULT)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " RESET_FACTORY_DEFAULT!!" << std::endl;
			//Writing a non-zero value to this object will force the blade to factory default
			const char* val = "1";
			if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpManager.snmpSet("NETMAN", cmxEthA.c_str(), snmpManager.getBladeReturnToFactoryDefaultOid().c_str(), 'i', val))
			{
				if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpManager.snmpSet("NETMAN", cmxEthB.c_str(), snmpManager.getBladeReturnToFactoryDefaultOid().c_str(), 'i', val))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ERROR" << std::endl;
					return SNMP_FAILURE;
				}
			}
		}
		else if (type == IMM_Util::RESET_NORMAL)
		{
			//RESET_NORMAL
			std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " RESET_NORMAL!!" << std::endl;

			//Set Next Restart Type in SYSMGR-MIB
			const char* val = "0"; //Cold Restart
			std::string oidBladeRestart, oidBladeReset;
			if(snmpManager.queryBoardPresencePower(2, oidBladeRestart))
			{
				std::string oidBladeRestart_slot = oidBladeRestart + ".0";
				if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpManager.snmpSet("NETMAN", cmxEthA.c_str(), oidBladeRestart_slot.c_str(), 'i', val))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Error: CANNOT set Next Restart Type!" << std::endl;
					(void) snmpManager.snmpSet("NETMAN", cmxEthB.c_str(), oidBladeRestart_slot.c_str(), 'i', val);
				}

				//Shelf MIB is only supported by SCX. The reset request goes through SCX then.
				snmpManager.queryBoardPresencePower(4, oidBladeReset);
				char oidBladeReset_slot[256] = {0};
				snprintf(oidBladeReset_slot, sizeof(oidBladeReset_slot) - 1, "%s.%u", oidBladeReset.c_str(), uslot);
				if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpManager.snmpSet("NETMAN", shelfMgrIpA.c_str(), oidBladeReset_slot, 'i', val))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Error: CANNOT set Blade Reset!" << std::endl;
					if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpManager.snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladeReset_slot, 'i', val))
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Error: CANNOT set Blade Reset!" << std::endl;
						return SNMP_FAILURE;
					}
				}
			}
		}
		else
                        {
                                return FUNC_BOARD_NOTALLOWED;
                        }

	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Board is TURNED OFF!!" << std::endl;
		//Board Turned Off
		if (type == IMM_Util::RESET_FACTORY_DEFAULT)
		{
			//TODO
			//case BOARD_BLOCKED_NO_DEFAULT_RESET
			//std::cout<<"Board cannot be reset to factory default as it is blocked"<<std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " RESET_FACTORY_DEFAULT!!" << std::endl;
			return BOARD_BLOCKED_NO_DEFAULT_RESET;
		}
		else
		{
			//RESET_NORMAL
			std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " RESET_NORMAL!!" << std::endl;
			//set the proper OID for blade power
			std::string oidBladePower;
			if(snmpManager.queryBoardPresencePower(1, oidBladePower))
			{
				const char* val = "1";
				char oidBladePower_slot[256] = {0};
				snprintf(oidBladePower_slot, sizeof(oidBladePower_slot) - 1, "%s.%u", oidBladePower.c_str(), uslot);

				//Turn-on the board
				if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpManager.snmpSet("NETMAN", shelfMgrIpA.c_str(), oidBladePower_slot, 'i', val))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Error: CANNOT set Blade Power!" << std::endl;
					if (FIXS_CCH_SNMPManager::GETSET_SNMP_OK != snmpManager.snmpSet("NETMAN", shelfMgrIpB.c_str(), oidBladePower_slot, 'i', val))
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Error: CANNOT set Blade Power!" << std::endl;
						return SNMP_FAILURE;
					}
				}
			}
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __FILE__ << ":" << __LINE__ << " Internal Error!" << std::endl;
				return SNMP_FAILURE;
			}
		}
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " RESET DONE!!" << std::endl;
	return EXECUTED;
}

bool FIXS_CCH_BladePercistanceValue::discoverShelfManager(unsigned long umagazine, std::string &shelfMgrIpA, std::string &shelfMgrIpB)
{
	std::string scx0EthA, scx0EthB, scx25EthA, scx25EthB;
	FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine, 0, scx0EthA, scx0EthB);
	FIXS_CCH_UpgradeManager::getInstance()->getBoardIps(umagazine, 25, scx25EthA, scx25EthB);

	FIXS_CCH_SNMPManager snmpManager(scx0EthA);

	int resSnmp = snmpManager.isMaster(scx0EthA);
	if (1 == resSnmp)//master
	{
		shelfMgrIpA = scx0EthA;
		shelfMgrIpB = scx0EthB;
	}
	else if (2 == resSnmp)//passive
	{
		shelfMgrIpA = scx25EthA;
		shelfMgrIpB = scx25EthB;
	}
	else if (HARDWARE_NOT_PRESENT == resSnmp)
	{
		resSnmp = snmpManager.isMaster(scx25EthB);

		if (1 == resSnmp)//master
		{
			shelfMgrIpA = scx25EthA;
			shelfMgrIpB = scx25EthB;
		}
		else if (2 == resSnmp)//passive
		{
			shelfMgrIpA = scx0EthA;
			shelfMgrIpB = scx0EthB;
		}
		else if (HARDWARE_NOT_PRESENT == resSnmp)
		{
			return false;
		}
	}
	return true;
}

bool FIXS_CCH_BladePercistanceValue::isBoardAccessible(std::string & shelfMgrIpA, std::string & shelfMgrIpB, unsigned short & uslot)
{
	bool isPresent = true;
	FIXS_CCH_SNMPManager snmpManager(shelfMgrIpA);
	int boardPresence = snmpManager.getBoardPresence(shelfMgrIpA, uslot);
	if(BOARD_ACCESSIBLE != boardPresence)
	{
		boardPresence = snmpManager.getBoardPresence(shelfMgrIpB, uslot);
		if (BOARD_ACCESSIBLE != boardPresence)
		{
			isPresent = false;
		}
	}
	return isPresent;
}

bool FIXS_CCH_BladePercistanceValue::isBoardTurnedOn(std::string & shelfMgrIpA, std::string & shelfMgrIpB, unsigned short & uslot)
{
	bool isTurnedOn = true;
	FIXS_CCH_SNMPManager snmpManager(shelfMgrIpA);
	int boardPower = snmpManager.getBoardPower(shelfMgrIpA, uslot);
	if(BOARD_TURNED_ON != boardPower) // 1 stands for Turned On
	{
		boardPower = snmpManager.getBoardPower(shelfMgrIpB, uslot);
		if (BOARD_TURNED_ON != boardPower)
		{
			isTurnedOn = false;
		}
	}
	return isTurnedOn;
}


int FIXS_CCH_BladePercistanceValue::boardBlock (std::string dn_value, bool  force)
{
	unsigned long umagazine = 0;
	unsigned short uslot = 0;

        std::string dn_blade("");

	cout << "dn_value: " << dn_value.c_str() << endl;

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : Board Block ",LOG_LEVEL_DEBUG);

	//get mag and slot
	std::string sMag = ACS_APGCC::before(dn_value,"_");
	std::string sSlot =ACS_APGCC::after(dn_value,"_");

	//convert mag and slot
	CCH_Util::stringToUlongMagazine(sMag,umagazine);
	CCH_Util::stringToUshort(sSlot,uslot);

	cout << "mag: " << sMag.c_str() << "slot: " << sSlot.c_str()<< endl;

	if (FIXS_CCH_UpgradeManager::getInstance()->isIptData(umagazine,uslot))
	{
		return blockIPTB(umagazine,uslot);
	}
	else if (FIXS_CCH_UpgradeManager::getInstance()->isIplbData(umagazine,uslot))
	{
	   if(force !=1)
           {
	       unsigned short islot=65;
	       IMM_Interface::getOtherBoardSlot(umagazine,uslot,islot);

	       int resp = 0;
               resp = FIXS_CCH_UpgradeManager::getInstance()->checkIplbSupervision(umagazine, uslot, islot);
	       if(resp == 0)	
               {
               	    return blockIPLB(umagazine,uslot);
	       }
               else
               {
                    return resp;
               }
           }
	   else
	   {
		return blockIPLB(umagazine,uslot);
	   }
	}
	else
	{
		cout << " Function Board not allowed " << endl;
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : Board Block failed: Function Board not allowed",LOG_LEVEL_ERROR);
		return FUNC_BOARD_NOTALLOWED;
	}
}

int FIXS_CCH_BladePercistanceValue::boardDeblock (std::string dn_value)
{
        unsigned long umagazine = 0;
        unsigned short uslot = 0;
        cout << "dn_value: " << dn_value.c_str() << endl;

        //get mag and slot
        std::string sMag = ACS_APGCC::before(dn_value,"_");
        std::string sSlot =ACS_APGCC::after(dn_value,"_");

        //convert mag and slot
        CCH_Util::stringToUlongMagazine(sMag,umagazine);
        CCH_Util::stringToUshort(sSlot,uslot);

        cout << "mag: " << sMag.c_str() << "slot: " << sSlot.c_str()<< endl;

        if (FIXS_CCH_UpgradeManager::getInstance()->isIptData(umagazine,uslot))
        {
        	return deblockIPTB(umagazine,uslot);
        }
        else if (FIXS_CCH_UpgradeManager::getInstance()->isIplbData(umagazine,uslot))
        {
        	return deblockIPLB(umagazine,uslot);
        }
        else
        {
                cout << " Function Board not allowed " << endl;
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : Board DeBlock failed: Function Board not allowed",LOG_LEVEL_ERROR);
                return FUNC_BOARD_NOTALLOWED;
        }
}

int FIXS_CCH_BladePercistanceValue::blockIPTB (unsigned long umagazine, unsigned short uslot)
{
	int retValue = 0;
	std::string Master_Ip("");
	std::string Passive_Ip("");
	unsigned short Master_slot = 0;
	std::string shelfMgrIpA("");
	std::string shelfMgrIpB("");

	CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
	if( env == MULTIPLECP_NOTCBA )
	{
		// Non-CBA environment
		cout<<" DBG: "<<" Blocking the IPT board in Non-CBA environment......"<<endl;

		if(!FIXS_CCH_UpgradeManager::getInstance()->getMasterPassiveSCB(umagazine,shelfMgrIpA,shelfMgrIpB,Master_slot))
		{
			cout<<" DBG: "<<" getMasterPassiveSCB() returns false... "<<endl;
			return INTERNALERR;
		}
		std::cout << __FUNCTION__ << "@" << __LINE__ << " Master IP adress: " << shelfMgrIpA.c_str() << " Master Slot: "<< Master_slot << std::endl;

		FIXS_CCH_SNMPManager *snmpReader =NULL;
		snmpReader = new FIXS_CCH_SNMPManager(shelfMgrIpA);

		if (!snmpReader)
			return SNMP_FAILURE;

		int pwrOffVal = 0;
		//Verify if the board is already blocked or not
		if(!snmpReader->getXshmcBoardPwrOff(uslot,&pwrOffVal))
		{
			snmpReader->setIpAddress(shelfMgrIpB);
			if(!snmpReader->getXshmcBoardPwrOff(uslot,&pwrOffVal))
			{
				cout << "DBG: " << "getXshmcBoardPwrOff failed" << std::endl;
				delete snmpReader;
				snmpReader = NULL;
				return SNMP_FAILURE;
			}
		}

		cout << "DBG: " << "getXshmcBoardPwrOff success" << std::endl;
		if(pwrOffVal == 1)
		{
			cout<<" DBG: "<<"IPT board is already blocked......"<<endl;
			retValue = BOARD_ALREADY_BLOCKED;
		}
		else
		{
			//Set the OID to make the board blocked
			if(snmpReader->setXshmcBoardPwrOff(1,uslot)== true)
			{
				//turn mia led on
				cout << "DBG: " << "setXshmcBoardPwrOff success" << std::endl;
				for (int retry = 0; (retry < 5) && (snmpReader->setXshmcBoardYellowLed(1,uslot) == false); retry++)
				{
					sleep(100);
				}
				retValue = EXECUTED;
			}
			else
			{
				cout << "DBG: " << "setXshmcBoardPwrOff failed" << std::endl;
				retValue = SNMP_FAILURE;
			}
		}
		if(snmpReader)
		{
			delete snmpReader;
			snmpReader = NULL;
		}
	}
	else if( (env == SINGLECP_CBA) || (env == MULTIPLECP_CBA) || (env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX)) //TR_HW29936
	{
		// CBA environment
		cout<<" DBG: "<<" Blocking the IPT board in CBA environment......"<<endl;

		//get Master board IP and slot
		if(!FIXS_CCH_UpgradeManager::getInstance()->getMasterPassiveSCX(umagazine,Master_Ip,Passive_Ip,Master_slot))
		{
			cout<<" DBG: "<<" getMasterPassiveSCX() returns false... "<<endl;
			return INTERNALERR;
		}

		FIXS_CCH_SNMPManager *snmpReader =NULL;
		snmpReader = new FIXS_CCH_SNMPManager(Master_Ip);

		if (!snmpReader)
			return SNMP_FAILURE;

		int brdPresence = -1;
		for(int retry = 0; retry < 10; retry++)
		{
			snmpReader->getBladePresence(uslot,&brdPresence );
			if(brdPresence == 3)
			{
				//std::cout << __FUNCTION__ << "@" << __LINE__  << std::endl;
				break;
			}
		}
		if(brdPresence != 3)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__  << std::endl;
			delete snmpReader;
			snmpReader = NULL;
			return HARDWARE_NOT_PRESENT;
		}

		int pwrValue =-1;
		int boardPwr = snmpReader->getBladePwr(uslot, &pwrValue);
		if(boardPwr == 0)
		{
			cout << "DBG: " << "getBladePwr() returns zero" << std::endl;
			delete snmpReader;
			snmpReader = NULL;
			return SNMP_FAILURE;
		}
		else
		{
			if(pwrValue == 0) //Board is OFF
			{
				cout<<" DBG: "<<"IPT board is already blocked......"<<endl;
				retValue = BOARD_ALREADY_BLOCKED;
			}
			else
			{
				//Set the OID to make the board blocked
				cout << "DBG: "<<" lets set the board block"<<endl;
				bool status = snmpReader->setBladePwr(0,uslot);
				if (status == false)
				{
					cout << "DBG: " << "setBladePwr() failed" << std::endl;
					delete snmpReader;
					snmpReader = NULL;
					return SNMP_FAILURE;
				}
				retValue = EXECUTED;
			}
		}
		if(snmpReader)
		{
			delete snmpReader;
			snmpReader = NULL;
		}
	}
	else
	{
		return ILLEGAL_COMMAND;
	}

	return retValue;
}

int FIXS_CCH_BladePercistanceValue::blockIPLB(unsigned long umagazine, unsigned short uslot)
{
	std::string Master_Ip("");
	std::string Passive_Ip("");
	std::string shelfMgrIpA("");
	std::string shelfMgrIpB("");

	uint32_t dmx_addr1[2];
        FIXS_CCH_UpgradeManager::getInstance()->getDmxAddress(dmx_addr1[0], dmx_addr1[1]);
	FIXS_CCH_IronsideManager *ironsideMgr =NULL;
	ironsideMgr = new FIXS_CCH_IronsideManager(dmx_addr1[0], dmx_addr1[1]);
	std::cout << "Blocking the IPLB board";
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : IPLB Board Block ",LOG_LEVEL_DEBUG);
	int set_mia_led_result = ironsideMgr->block_board(umagazine, uslot);
	if(set_mia_led_result != 0)
	{
		std::cout << "blocking board failed";
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : IPLB Board Block failed ",LOG_LEVEL_ERROR);
		delete ironsideMgr;
		ironsideMgr = NULL;
		return set_mia_led_result;
	}

	std::cout << "Block the IPLB board success";
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : IPLB Board Block Success ",LOG_LEVEL_DEBUG);
	delete ironsideMgr;
	ironsideMgr = NULL;
	return EXECUTED;

}

int FIXS_CCH_BladePercistanceValue::deblockIPTB (unsigned long umagazine, unsigned short uslot)
{
	int retValue = 0;
	std::string Master_Ip("");
	std::string Passive_Ip("");
	unsigned short Master_slot = 0;
	std::string shelfMgrIpA("");
	std::string shelfMgrIpB("");

	CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();
	if( env == MULTIPLECP_NOTCBA )
	{
		// Non-CBA environment
		cout<<" DBG: "<<" De-blocking the IPT board in Non-CBA environment......"<<endl;
		if(!FIXS_CCH_UpgradeManager::getInstance()->getMasterPassiveSCB(umagazine,shelfMgrIpA,shelfMgrIpB,Master_slot))
		{
			cout<<" DBG: "<<" getMasterPassiveSCB() returns false... "<<endl;
			return INTERNALERR;
		}
		FIXS_CCH_SNMPManager *snmpReader =NULL;
		snmpReader = new FIXS_CCH_SNMPManager(shelfMgrIpA);

		if (!snmpReader)
			return SNMP_FAILURE;

		std::cout << __FUNCTION__ << "@" << __LINE__ << " Master IP adress: " << shelfMgrIpA.c_str() << " Master Slot: "<< Master_slot << std::endl;

		int pwrOnVal = 0;

		//Verify if the board is actually blocked or not
		if(!snmpReader->getXshmcBoardPwrOn(uslot,&pwrOnVal))
		{
			snmpReader->setIpAddress(shelfMgrIpB);
			if(!snmpReader->getXshmcBoardPwrOn(uslot,&pwrOnVal))
			{
				cout << "DBG: " << "getXshmcBoardPwrOff failed" << std::endl;
				delete snmpReader;
				snmpReader = NULL;
				return SNMP_FAILURE;
			}
		}

		std::cout << "DBG: " << "getXshmcBoardPwrOff success" << std::endl;
		if(pwrOnVal == 1)
		{
			std::cout << "DBG: " << "Board Already Deblocked" << std::endl;
			retValue = BOARD_ALREADY_DEBLOCKED;
		}
		else
		{
			//Set the OID to deblock the board
			if(snmpReader->setXshmcBoardPwrOn(1,uslot)== true)
			{
				//turn off mia led
				for (int retry = 0; (retry < 5) && (snmpReader->setXshmcBoardYellowLed(0,uslot) == false); retry++)
				{
					sleep(100);
				}
				std::cout << "DBG: " << "setXshmcBoardPwrOn success" << std::endl;
				retValue = EXECUTED;
			}
			else
			{
				std::cout << "DBG: " << "setXshmcBoardPwrOn failed" << std::endl;
				retValue = INTERNALERR;
			}
		}

		if(snmpReader)
		{
			delete snmpReader;
			snmpReader = NULL;
		}
	}
	else if( (env == SINGLECP_CBA) || (env == MULTIPLECP_CBA) || (env == CCH_Util::SINGLECP_SMX) || (env == CCH_Util::MULTIPLECP_SMX)) //TR_HW29936
	{
		// CBA environment
		cout<<" DBG: "<<" De-blocking the IPT board in CBA environment......"<<endl;
		if(!FIXS_CCH_UpgradeManager::getInstance()->getMasterPassiveSCX(umagazine,Master_Ip,Passive_Ip,Master_slot))
		{
			cout<<" DBG: "<<" getMasterPassiveSCX() returns false... "<<endl;
			return INTERNALERR;
		}

		FIXS_CCH_SNMPManager *snmpReader =NULL;
		snmpReader = new FIXS_CCH_SNMPManager(Master_Ip);

		if (!snmpReader)
			return SNMP_FAILURE;

		int brdPresence = -1;
		for(int retry = 0; retry < 10; retry++)
		{
			snmpReader->getBladePresence(uslot,&brdPresence );
			if(brdPresence == 3)
			{
				break;
			}
		}
		if(brdPresence != 3)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__  << std::endl;
			delete snmpReader;
			snmpReader = NULL;
			return HARDWARE_NOT_PRESENT;
		}
		int pwrValue =-1;
		int boardPwr = snmpReader->getBladePwr(uslot, &pwrValue);
		if(boardPwr == 0)
		{
			delete snmpReader;
			snmpReader = NULL;
			return SNMP_FAILURE;
		}
		else
		{
			if(pwrValue == 1) //Board is On
			{
				cout << "DBG: " << "IPT Board already Deblocked" << std::endl;
				retValue = BOARD_ALREADY_DEBLOCKED;
			}
			else
			{
				//Set the OID to deblock the board
				bool status = snmpReader->setBladePwr(1,uslot);
				if (status == false)
				{
					delete snmpReader;
					snmpReader = NULL;
					return SNMP_FAILURE;
				}
				retValue = EXECUTED;
			}
		}
		if(snmpReader)
		{
			delete snmpReader;
			snmpReader = NULL;
		}
	}
	else
	{
		return ILLEGAL_COMMAND;
	}
	return retValue;
}

int FIXS_CCH_BladePercistanceValue::deblockIPLB(unsigned long umagazine, unsigned short uslot)
{
        std::string Master_Ip("");
        std::string Passive_Ip("");
        std::string shelfMgrIpA("");
        std::string shelfMgrIpB("");

	uint32_t dmx_addr1[2];
        FIXS_CCH_UpgradeManager::getInstance()->getDmxAddress(dmx_addr1[0], dmx_addr1[1]);

        FIXS_CCH_IronsideManager *ironsideMgr =NULL;
        ironsideMgr = new FIXS_CCH_IronsideManager(dmx_addr1[0], dmx_addr1[1]);
        std::cout << "Blocking the IPLB board";
        int set_mia_led_result = ironsideMgr->dblock_board(umagazine, uslot);
        if(set_mia_led_result != 0)
        {
        	std::cout << "Deblocking board failed";
        	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : IPLB Board DeBlock failed ",LOG_LEVEL_ERROR);
        	delete ironsideMgr;
        	ironsideMgr = NULL;
        	return set_mia_led_result;
        }

        std::cout << "DeBlock the IPLB board success";
        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladePercistanceValue : IPLB Board DeBlock Success ",LOG_LEVEL_DEBUG);
        delete ironsideMgr;
        ironsideMgr = NULL;
        return EXECUTED;
}

void FIXS_CCH_BladePercistanceValue::setAdminOperationError(int errorCode,ACS_APGCC_OiHandle oiHandle,ACS_APGCC_InvocationType invocation)
{
        std::vector<ACS_APGCC_AdminOperationParamType> resultOut;
        ACS_APGCC_AdminOperationParamType errorTextElem;
        char attrNameText[]= "errorText";

        errorTextElem.attrName = attrNameText;
        errorTextElem.attrType = ATTR_STRINGT;

        string errText("@ComNbi@");
        errText.append(IMM_Util::getErrorText(errorCode));
        errorTextElem.attrType = ATTR_STRINGT;
        char* errValue = const_cast<char*>(errText.c_str());
        errorTextElem.attrValues=reinterpret_cast<void*>(errValue);

        resultOut.push_back(errorTextElem);

        //setExitCode(errorCode,errText);

        ACS_CC_ReturnType rc = adminOperationResult(oiHandle, invocation,0, resultOut);
        cout <<__FUNCTION__<<"@"<<__LINE__<<" Return Value = "<<rc<< endl;
        return;
}
