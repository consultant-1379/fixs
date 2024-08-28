/*
 * FIXS_CCH_ImmInterface.cpp
 *
 *  Created on: Mar 28, 2012
 *      Author: eanform
 */

#include "FIXS_CCH_ImmInterface.h"
void IMM_Interface::getEnvironmentType(EnvironmentType &env)
{

	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue arch;
	bool multipleCPSystem = false;

	if ((ACS_CS_API_NetworkElement::isMultipleCPSystem(multipleCPSystem) == ACS_CS_API_NS::Result_Success) && (ACS_CS_API_NetworkElement::getNodeArchitecture(arch) == ACS_CS_API_NS::Result_Success))
	{
		switch(arch)
		{
		case ACS_CS_API_CommonBasedArchitecture::SCB:

			if (multipleCPSystem) env = MULTIPLECP_NOTCBA;
			else env = SINGLECP_NOTCBA;
			break;

		case ACS_CS_API_CommonBasedArchitecture::SCX:

			if (multipleCPSystem) env = MULTIPLECP_CBA;
			else env = SINGLECP_CBA;
			break;

		case ACS_CS_API_CommonBasedArchitecture::DMX:

			if (multipleCPSystem) env = MULTIPLECP_DMX;
			else env = SINGLECP_DMX;
			break;
		case ACS_CS_API_CommonBasedArchitecture::VIRTUALIZED:
   			env = VIRTUALIZED;
			break;
		case ACS_CS_API_CommonBasedArchitecture::SMX:
			if (multipleCPSystem) env = MULTIPLECP_SMX;
			else env = SINGLECP_SMX;
			break;
		default:
			env = UNKNOWN;
			break;
		}
	}
	else
	{
		env = UNKNOWN;
	}


}


int IMM_Interface::getContainersFromDisk ()
{
	int retValue = 0;
	std::vector<std::string> p_dnList;
	std::string dn_obj("");
	std::string rdn_obj("");
	int defaultPackage = 0;
	int faultyPackage = 0;
	IMM_Util::getChildrenObject(IMM_Util::RDN_BLADESWM, p_dnList);
	if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
	{
		std::vector<std::string> immCxpObject;
		getImmBladeSWObjects(p_dnList,immCxpObject);
		FIXS_CCH_DiskHandler::createMisallingnedSwObjects(p_dnList,immCxpObject);
	}
	//loop of istance
	for (unsigned i=0; i<p_dnList.size(); i++)
	{
		defaultPackage = 0;
		rdn_obj.clear();
		dn_obj.clear();
		dn_obj = p_dnList[i].c_str();

		//check if exist
		IMM_Util::getRdnAttribute(dn_obj,rdn_obj);
		if (( strcmp(rdn_obj.c_str(),IMM_Util::ATT_APSWPACK_RDN) == 0 ) || (strcmp(rdn_obj.c_str(),IMM_Util::ATT_CPSWPACK_RDN) == 0))
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data;

			//Object DN
			data.dname = dn_obj;

			//Package
			std::string package("");
			IMM_Util::getRdnValue(dn_obj,package);
			data.m_container = package;

			//is Used (comment)
			std::vector<std::string> usedValues;
			IMM_Util::getImmMultiAttributeString(dn_obj,IMM_Util::ATT_SWPACK_USED_BY,usedValues);
			if (usedValues.size() > 0)
			{
				if (strcmp(usedValues[0].c_str(),"")!=0)
				{
					data.m_isUsed = true;
					data.m_comment = IN_USE;
				}
				else
				{
					data.m_isUsed = false;
					data.m_comment = NOT_USED;
				}
			}
			else
			{
				data.m_isUsed = false;
				data.m_comment = NOT_USED;
			}

			//default Package
			IMM_Util::getImmAttributeInt(dn_obj,IMM_Util::ATT_SWPACK_DEFAULT,defaultPackage);

			if (defaultPackage == 1){
				data.m_isDefault = true;
			}
			else{
				data.m_isDefault = false;
			}

			//default Package
			IMM_Util::getImmAttributeInt(dn_obj,IMM_Util::ATT_SWPACK_FAULTY,faultyPackage);

			if (faultyPackage == 1){
				data.m_isFaulty = true;
			}
			else{
				data.m_isFaulty = false;
			}
			
			//if start-up due to a restore
			if (FIXS_CCH_DiskHandler::isStartedAfterRestore()) //temporarily condition changed for basic testing
			{
				//Software path
				std::string pathParam("");
				IMM_Util::getImmAttributeString(dn_obj,IMM_Util::ATT_SWPACK_PATH,pathParam);

				//check if software path exist
				struct stat st;
				if ( (stat(pathParam.c_str(),&st) !=0) && (ACS_APGCC::is_active_node()) )
				{
					EventHandle m_shutdownEvent=0;
					const int WAIT_TIMEOUT= -2;
					const int WAIT_OBJECT_0=0;
					const int Shutdown_Event_Index=0;
					const int Number_Of_Alarm_Events=1;
					DWORD Safe_Timeout = 1000;
					bool respValue=false;
					bool m_exit=false;
					int count=0;
					if(data.m_isUsed == true)
					{
						data.m_isFaulty = true;

						//set faulty in IMM
						setFaultyPackage(dn_obj,1);
						/*if(data.m_isDefault) {
							setDefaultPackage(dn_obj,0);
							data.m_isDefault=false;
						}*/
						FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects.push_back(package);
					}	
					else {				
						respValue=IMM_Util::deleteImmObject(dn_obj,ACS_APGCC_SUBTREE);
						if(respValue)
						{
							continue;
						}
						if (m_shutdownEvent == 0)
							m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

						EventHandle handleArr=m_shutdownEvent;
						while (!m_exit) // Set to true when the thread shall exit
						{
							if(count >= 60){
								m_exit=true;
								break;
							}
							//wait for 1s
							int returnValue = FIXS_CCH_Event::WaitForEvents(Number_Of_Alarm_Events, &handleArr, Safe_Timeout);
	
							switch (returnValue)
							{
								case WAIT_TIMEOUT:  // Time out
								{
									if(IMM_Util::deleteImmObject(dn_obj,ACS_APGCC_SUBTREE))
									{	
										m_exit = true;
									}
									else{
										count++;
									}
									break;
								}
								case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
								{
									m_exit = true;      // Terminate the thread
									break;
								}
								default:
								{
									count++;
									break;
								}
							} // End of switch
						}//while
						if (m_exit)
							continue;
					}
				}
				else{
					cout<<"package exist in disk-- getContainersFromDisk function"<<endl;
				}
			}
			//cxcNumber
			std::string cxcNumber("");
			IMM_Util::getImmAttributeString(dn_obj,IMM_Util::ATT_SWPACK_CXC,cxcNumber);
			data.m_product = cxcNumber;

			//forFBN
			int forFBN = 0;
			IMM_Util::getImmAttributeInt(dn_obj,IMM_Util::ATT_SWPACK_FBN,forFBN);


			if (forFBN == IMM_Util::SCXB)
			{
				data.m_fbn = SCXFBN;

				if (data.m_isDefault)
 				{
					 FIXS_CCH_SoftwareMap::getInstance()->setScxDefaultPackage(data.m_container);
				}
				
				FIXS_CCH_SoftwareMap::getInstance()->addScxEntry(data);
			}

			else if (forFBN == IMM_Util::IPTB)
			{
				data.m_fbn = IPTFBN;

				if (data.m_isDefault) FIXS_CCH_SoftwareMap::getInstance()->setIptDefaultPackage(data.m_container);
				FIXS_CCH_SoftwareMap::getInstance()->addIptEntry(data);
			}
                        
			else if (forFBN == IMM_Util::IPLB)
			{
				data.m_fbn = IPLBFBN;

				if (data.m_isDefault) FIXS_CCH_SoftwareMap::getInstance()->setIplbDefaultPackage(data.m_container);
				FIXS_CCH_SoftwareMap::getInstance()->addIplbEntry(data);
			}

			else if (forFBN == IMM_Util::EPB1)
			{
				data.m_fbn = EPB1FBN;

				if (data.m_isDefault) FIXS_CCH_SoftwareMap::getInstance()->setEpb1DefaultPackage(data.m_container);
				FIXS_CCH_SoftwareMap::getInstance()->addEpb1Entry(data);
			}

			else if (forFBN == IMM_Util::CMXB)
			{
				data.m_fbn = CMXFBN;

				if (data.m_isDefault) FIXS_CCH_SoftwareMap::getInstance()->setCmxDefaultPackage(data.m_container);
				FIXS_CCH_SoftwareMap::getInstance()->addCmxEntry(data);
			}

			else if (forFBN == IMM_Util::EVOET)
			{
				data.m_fbn = EVOETFBN;

				if (data.m_isDefault) FIXS_CCH_SoftwareMap::getInstance()->setEvoEtDefaultPackage(data.m_container);
				FIXS_CCH_SoftwareMap::getInstance()->addEvoEtEntry(data);
			}
			else if (forFBN == IMM_Util::SMXB)
			{
				data.m_fbn = SMXFBN;
				if (data.m_isDefault) FIXS_CCH_SoftwareMap::getInstance()->setSmxDefaultPackage(data.m_container);
				FIXS_CCH_SoftwareMap::getInstance()->addSmxEntry(data);
			}

		}//check rdn
	}
	return retValue;
}


bool IMM_Interface::setSwDataFromModel(std::string blade_dn, std::string dn, SWData &data)
 {
	bool retVal = true;

	IMM_Util::getImmAttributeString(data.dnBladePersistance,IMM_Util::ATT_BLADESWPERSISTANCE_CURRENTCXP,data.currentCxc);

	if (data.currentCxc.empty())
	{
		std::string currCxp("");
		IMM_Util::getImmAttributeString(blade_dn,IMM_Util::ATT_OTHERBLADE_CURRSW,currCxp);
		IMM_Util::getRdnValue(currCxp,data.currentCxp);

		data.currentCxc = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(data.currentCxp);
	}
	else
	{
		//packageCurr
		data.currentCxp = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(data.currentCxc);
	}

	//softwareReason
	int softwareReason;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_REASON,softwareReason))
	{
		cout << __FUNCTION__<< " ERROR: softwareReason FAILURE!!!\n";
		return false;
	}
	data.lastUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(softwareReason);

	//softwareResult
	int softwareResult;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_RESULT,softwareResult))
	{
		cout << __FUNCTION__<< " ERROR: softwareResult FAILURE!!!\n";
		return false;
	}
	data.lastUpgradeResult = static_cast<CCH_Util::RpUpgradeResult>(softwareResult);

	//softwareLastTime
	std::string softwareLastTime;
	if (!IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_SWINVENTORY_TIME,softwareLastTime))
	{
		cout << __FUNCTION__<< " ERROR: softwareLastTime FAILURE!!!\n";
		return false;
	}
	data.lastUpgradeTime = softwareLastTime;

	//softwareLastErrorCode
	int softwareLastErrorCode = 0;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_ERRORCODE,softwareLastErrorCode))
	{
		cout << __FUNCTION__<< " ERROR: softwareLastErrorCode FAILURE!!!\n";
		return false;
	}
	data.lastErrorCode = softwareLastErrorCode;

	//softwareExpect
	std::string softwareExpect("");
	if (IMM_Util::getImmAttributeString(data.dnBladePersistance,IMM_Util::ATT_BLADESWPERSISTANCE_EXPECTCXP,softwareExpect))
	data.expectedCxc = softwareExpect;

	//packageExpect
	data.expectedCxp = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(data.expectedCxc);

	//softwareState
	int softwareState = 0;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_STATE,softwareState))
	{
		cout << __FUNCTION__<< " ERROR: softwareState FAILURE!!!\n";
		return false;
	}
	int ivalue = softwareState;
	data.upgradeStatus = static_cast<CCH_Util::SwStatus>(ivalue);

	//forcedUpgrade
	int forcedUpgrade = 0;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_FORCEDUPGRADE,forcedUpgrade))
	{
		cout << __FUNCTION__<< " ERROR: forcedUpgrade FAILURE!!!\n";
	//	return false;
	}
	data.forcedUpgrade = forcedUpgrade;

	 //UpgradeType
        int upgradeType = 0;
        if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_UPGRADETYPE,upgradeType))
        {
                cout << __FUNCTION__<< " ERROR: upgradeType FAILURE!!\n";
          //      return false;
        }
        data.upgradeType = static_cast<CCH_Util::BoardUpgradeType>(upgradeType);

 return retVal;
 }
bool IMM_Interface::setBoardSwDataFromModel(std::string blade_dn, std::string dn, BoardSWData &data)
{
	bool retVal = true;

	IMM_Util::getImmAttributeString(data.dnBladePersistance,IMM_Util::ATT_BLADESWPERSISTANCE_CURRENTCXP,data.currentCxc);

	if (data.currentCxc.empty())
	{
		std::string currCxp("");
		IMM_Util::getImmAttributeString(blade_dn,IMM_Util::ATT_OTHERBLADE_CURRSW,currCxp);
		IMM_Util::getRdnValue(currCxp,data.currentCxp);

		data.currentCxc = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(data.currentCxp);
	}
	else
	{
		//packageCurr
		data.currentCxp = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(data.currentCxc);
	}

	//softwareReason
	int softwareReason;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_REASON,softwareReason))
	{
		cout << __FUNCTION__<< " ERROR: softwareReason FAILURE!!!\n";
		return false;
	}
	data.lastUpgradeReason = static_cast<CCH_Util::RpUpgradeResultInfo>(softwareReason);

	//softwareResult
	int softwareResult;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_RESULT,softwareResult))
	{
		cout << __FUNCTION__<< " ERROR: softwareResult FAILURE!!!\n";
		return false;
	}
	data.lastUpgradeResult = static_cast<CCH_Util::RpUpgradeResult>(softwareResult);

	//softwareLastTime
	std::string softwareLastTime;
	if (!IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_SWINVENTORY_TIME,softwareLastTime))
	{
		cout << __FUNCTION__<< " ERROR: softwareLastTime FAILURE!!!\n";
		return false;
	}
	data.lastUpgradeTime = softwareLastTime;

	//softwareLastErrorCode
	int softwareLastErrorCode = 0;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_ERRORCODE,softwareLastErrorCode))
	{
		cout << __FUNCTION__<< " ERROR: softwareLastErrorCode FAILURE!!!\n";
		return false;
	}
	data.lastErrorCode = softwareLastErrorCode;

	//softwareExpect
	std::string softwareExpect("");
	if (IMM_Util::getImmAttributeString(data.dnBladePersistance,IMM_Util::ATT_BLADESWPERSISTANCE_EXPECTCXP,softwareExpect))
		data.expectedCxc = softwareExpect;

	//packageExpect
	data.expectedCxp = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(data.expectedCxc);

	//softwareState
	int softwareState = 0;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWINVENTORY_STATE,softwareState))
	{
		cout << __FUNCTION__<< " ERROR: softwareState FAILURE!!!\n";
		return false;
	}
	int ivalue = softwareState;
	data.upgradeStatus = static_cast<CCH_Util::LoadModuleUpgradeStatus>(ivalue);

	//data.lastUpgradeType = (m_fullUpradeType ? "IPT_FULL_UPG" : "IPT_NORM_UPG");
	//softwareReason
	/*int upgradeType=0;
	if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWPACK_UPGRADETYPE,upgradeType))
	{
		cout << __FUNCTION__<< " ERROR: upgradeType FAILURE!!!\n";
		return false;
	}
	data.lastUpgradeType = upgradeType;*/

	//data.previousState = m_previousState;
	//data.failureCode = m_failureCode;
	return retVal;
}

bool IMM_Interface::getOtherBladeCurrLoadModule(std::string dn, std::string &value)
{
	bool res = false;
	std::string dn_package("");
	if (IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_OTHERBLADE_CURRSW,dn_package)) res = true;
	IMM_Util::getRdnValue(dn_package,value);

	return res;
}


bool IMM_Interface::modifyAllInventoryAttributes(std::string dn, SWData &data)
{
	bool result = true;


	//STATE
	int state = data.swState;
	ACS_CC_ImmParameter SWstatus = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_STATE,ATTR_INT32T,&state,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWstatus);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_STATE<< " FAILED !!!"<< endl;
	delete [] SWstatus.attrValues;

	//RESULT
	int res = data.lastUpgradeResult;
	ACS_CC_ImmParameter SWresult = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_RESULT,ATTR_INT32T,&res,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWresult);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_RESULT<< " FAILED !!!"<< endl;
	delete [] SWresult.attrValues;

	//REASON
	int reason = data.lastUpgradeReason;
	ACS_CC_ImmParameter SWreason = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_REASON,ATTR_INT32T,&reason,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWreason);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_REASON<< " FAILED !!!"<< endl;
	delete [] SWreason.attrValues;

	//LAST TIME
	if (CCH_Util::STATE_IDLE == state && CCH_Util::SW_RP_OK == res)
	{
		std::string lTime = data.lastUpgradeDate + " " + data.lastUpgradeTime;
		ACS_CC_ImmParameter SWtime = IMM_Util::defineParameterString(IMM_Util::ATT_SWINVENTORY_TIME,ATTR_STRINGT,lTime.c_str(),1);
		result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWtime);
		if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_LASTTIME<< " FAILED !!!"<< endl;
		delete [] SWtime.attrValues;
	}

	//ERROR CODE
	int errorCode = data.lastErrorCode;
	ACS_CC_ImmParameter SWerror = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_ERRORCODE,ATTR_INT32T,&errorCode,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWerror);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_ERRORCODE<< " FAILED !!!"<< endl;
	delete [] SWerror.attrValues;

	//CURRENT
	ACS_CC_ImmParameter SWcurr = IMM_Util::defineParameterString(IMM_Util::ATT_BLADESWPERSISTANCE_CURRENTCXP,ATTR_STRINGT,data.currentCxc.c_str(),1);
	result = IMM_Util::modify_OM_ImmAttr(data.dnBladePersistance.c_str(),SWcurr);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_BLADESWPERSISTANCE_CURRENTCXP<< " FAILED !!!"<< endl;
	delete [] SWcurr.attrValues;

	//EXPECT
	ACS_CC_ImmParameter SWexp = IMM_Util::defineParameterString(IMM_Util::ATT_BLADESWPERSISTANCE_EXPECTCXP,ATTR_STRINGT,data.expectedCxc.c_str(),1);
	result = IMM_Util::modify_OM_ImmAttr(data.dnBladePersistance.c_str(),SWexp);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_BLADESWPERSISTANCE_EXPECTCXP<< " FAILED !!!"<< endl;
	delete [] SWexp.attrValues;
	
	//FORCEDUPGRADE
	int forcedUpgrade = data.forcedUpgrade;
	ACS_CC_ImmParameter SWforce = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_FORCEDUPGRADE,ATTR_INT32T,&forcedUpgrade,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWforce);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_FORCEDUPGRADE<< " FAILED !!!"<< endl;
	delete [] SWforce.attrValues;

	
	//UPGRADETYPE
	int upgType = data.upgradeType;
	ACS_CC_ImmParameter SWupgtype = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_UPGRADETYPE,ATTR_INT32T,&upgType,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWupgtype);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_UPGRADETYPE<< " FAILED !!!"<< endl;
	delete [] SWupgtype.attrValues;
	return result;
}
bool IMM_Interface::modifyAllLMInventoryAttributes(std::string dn, BoardSWData &data)
{
	bool result = true;

	//STATE
	int state = data.lmState;
	ACS_CC_ImmParameter LMstatus = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_STATE,ATTR_INT32T,&state,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),LMstatus);
	if (!result) cout << " FIXS_CCH_IptbUpgrade modify " << IMM_Util::ATT_SWINVENTORY_STATE<< " FAILED !!!"<< endl;
	delete [] LMstatus.attrValues;

	//RESULT
	int res = data.lastUpgradeResult;
	ACS_CC_ImmParameter LMresult = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_RESULT,ATTR_INT32T,&res,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),LMresult);
	if (!result) cout << " FIXS_CCH_IptbUpgrade modify " << IMM_Util::ATT_SWINVENTORY_RESULT<< " FAILED !!!"<< endl;
	delete [] LMresult.attrValues;

	//REASON
	int reason = data.lastUpgradeReason;
	ACS_CC_ImmParameter LMreason = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_REASON,ATTR_INT32T,&reason,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),LMreason);
	if (!result) cout << " FIXS_CCH_IptbUpgrade modify " << IMM_Util::ATT_SWINVENTORY_REASON<< " FAILED !!!"<< endl;
	delete [] LMreason.attrValues;

	//LAST TIME
	if (CCH_Util::STATE_IDLE == state && CCH_Util::SW_RP_OK == res)
	{
		std::string lTime = data.lastUpgradeDate + " " + data.lastUpgradeTime;
		ACS_CC_ImmParameter LMtime = IMM_Util::defineParameterString(IMM_Util::ATT_SWINVENTORY_TIME,ATTR_STRINGT,lTime.c_str(),1);
		result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),LMtime);
		if (!result) cout << " FIXS_CCH_IptbUpgrade modify " << IMM_Util::ATT_SWINVENTORY_LASTTIME<< " FAILED !!!"<< endl;
		delete [] LMtime.attrValues;
	}

	//ERROR CODE
	int errorCode = data.lastErrorCode;
	ACS_CC_ImmParameter LMerror = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_ERRORCODE,ATTR_INT32T,&errorCode,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),LMerror);
	if (!result) cout << " FIXS_CCH_IptbUpgrade modify " << IMM_Util::ATT_SWINVENTORY_ERRORCODE<< " FAILED !!!"<< endl;
	delete [] LMerror.attrValues;

	//CURRENT
	ACS_CC_ImmParameter LMcurr = IMM_Util::defineParameterString(IMM_Util::ATT_BLADESWPERSISTANCE_CURRENTCXP,ATTR_STRINGT,data.currentCxc.c_str(),1);
	result = IMM_Util::modify_OM_ImmAttr(data.dnBladePersistance.c_str(),LMcurr);
	if (!result) cout << " FIXS_CCH_IptbUpgrade modify " << IMM_Util::ATT_BLADESWPERSISTANCE_CURRENTCXP<< " FAILED !!!"<< endl;
	delete [] LMcurr.attrValues;

	//EXPECT
	ACS_CC_ImmParameter LMexp = IMM_Util::defineParameterString(IMM_Util::ATT_BLADESWPERSISTANCE_EXPECTCXP,ATTR_STRINGT,data.expectedCxc.c_str(),1);
	result = IMM_Util::modify_OM_ImmAttr(data.dnBladePersistance.c_str(),LMexp);
	if (!result) cout << " FIXS_CCH_IptbUpgrade modify " << IMM_Util::ATT_BLADESWPERSISTANCE_EXPECTCXP<< " FAILED !!!"<< endl;
	delete [] LMexp.attrValues;

	//UPGRADE TYPE
	/*int upgradeType = data.lastUpgradeType;
	ACS_CC_ImmParameter LMUpgres = IMM_Util::defineParameterInt(IMM_Util::ATT_SWPACK_UPGRADETYPE,ATTR_INT32T,&upgradeType,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),LMUpgres);
	if (!result) cout << " FIXS_CCH_IptbUpgrade modify " << IMM_Util::ATT_SWPACK_UPGRADETYPE<< " FAILED !!!"<< endl;
	delete [] LMUpgres.attrValues;*/

	
	//FORCEDUPGRADE
	int forcedUpgrade = data.forcedUpgrade;
	ACS_CC_ImmParameter SWforce = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_FORCEDUPGRADE,ATTR_INT32T,&forcedUpgrade,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWforce);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_FORCEDUPGRADE<< " FAILED !!!"<< endl;
	delete [] SWforce.attrValues;

	
	//UPGRADETYPE
	int upgType = data.upgradeType;
	ACS_CC_ImmParameter SWupgtype = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_UPGRADETYPE,ATTR_INT32T,&upgType,1);
	result = IMM_Util::modify_OM_ImmAttr(dn.c_str(),SWupgtype);
	if (!result) cout << " FIXS_CCH_ScxUpgrade modify " << IMM_Util::ATT_SWINVENTORY_UPGRADETYPE<< " FAILED !!!"<< endl;
	delete [] SWupgtype.attrValues;
	return result;
}
void IMM_Interface::defineSwInventoryObjectDN(unsigned long magazine, unsigned short slot, std::string &dn)
{
	dn = "";
	std::string sMag("");
	std::string sSlot("");

	CCH_Util::ulongToStringMagazine(magazine,sMag);
	CCH_Util::ushortToString(slot,sSlot);

	std::string id= sMag + "_" + sSlot;

	std::vector<std::string> rpList;
	IMM_Util::getClassObjectsList(IMM_Util::classReportProgress,rpList);

	for (unsigned i=0; i<rpList.size(); i++)
	{
		std::string rp_dn("");
		std::string rdn_value("");
		rp_dn = rpList[i].c_str();

		IMM_Util::getRdnValue(rp_dn,rdn_value);
		if ( strcmp(rdn_value.c_str(),id.c_str()) == 0 )
		{
			dn = rp_dn;
		}
	}

}

bool IMM_Interface::getNeighborScxIp(std::string dn, int slot, std::string &neighborSCXIPA, std::string &neighborSCXIPB)
 {
	bool res = false;
	std::string dn_shelf("");
	std::vector<std::string> boardList;

	IMM_Util::getDnParent(dn,dn_shelf);
	cout << " get Neighbor Scx Ip into object magazine: " << dn_shelf.c_str() << endl;
	IMM_Util::getChildrenObject(dn_shelf,boardList);

	for (unsigned j=0; j<boardList.size(); j++)
	{
		int found_slot = 0;
		if (slot == 0) found_slot = 25;

		std::string board_dn("");
		std::string rdn_board("");
		board_dn = boardList[j].c_str();

		//check if is an object of OtherBlade Class
		IMM_Util::getRdnAttribute(board_dn,rdn_board);
		if ( strcmp(rdn_board.c_str(),IMM_Util::ATT_OTHERBLADE_RDN) == 0 )
		{
			int islot = 0;
			std::string sSlot("");
			IMM_Util::getRdnValue(board_dn,sSlot);
			CCH_Util::stringToInt(sSlot,islot);
//			IMM_Util::getImmAttributeInt(board_dn,IMM_Util::ATT_OTHERBLADE_SLOT,islot);
			if (found_slot == islot)
			{
				IMM_Util::getImmAttributeString(board_dn,IMM_Util::ATT_OTHERBLADE_IPA,neighborSCXIPA);
				IMM_Util::getImmAttributeString(board_dn,IMM_Util::ATT_OTHERBLADE_IPB,neighborSCXIPB);

				res = true;
			}
		}
	}
	return res;
 }

bool IMM_Interface::getNeighborSmxIp(std::string dn, int slot, std::string &neighborSMXIPA, std::string &neighborSMXIPB)
 {
	bool res = false;
	std::string dn_shelf("");
	std::vector<std::string> boardList;

	IMM_Util::getDnParent(dn,dn_shelf);
	cout << " get Neighbor Smx Ip into object magazine: " << dn_shelf.c_str() << endl;
	IMM_Util::getChildrenObject(dn_shelf,boardList);

	for (unsigned j=0; j<boardList.size(); j++)
	{
		int found_slot = 0;
		if (slot == 0) found_slot = 25;

		std::string board_dn("");
		std::string rdn_board("");
		board_dn = boardList[j].c_str();

		//check if is an object of OtherBlade Class
		IMM_Util::getRdnAttribute(board_dn,rdn_board);
		if ( strcmp(rdn_board.c_str(),IMM_Util::ATT_OTHERBLADE_RDN) == 0 )
		{
			int islot = 0;
			std::string sSlot("");
			IMM_Util::getRdnValue(board_dn,sSlot);
			CCH_Util::stringToInt(sSlot,islot);
//			IMM_Util::getImmAttributeInt(board_dn,IMM_Util::ATT_OTHERBLADE_SLOT,islot);
			if (found_slot == islot)
			{
				IMM_Util::getImmAttributeString(board_dn,IMM_Util::ATT_OTHERBLADE_IPA,neighborSMXIPA);
				IMM_Util::getImmAttributeString(board_dn,IMM_Util::ATT_OTHERBLADE_IPB,neighborSMXIPB);

				res = true;
			}
		}
	}
	return res;
 }
 
bool IMM_Interface::readUpgradeStatus (unsigned long magazine, unsigned short slot, PFMData &data)
{
	bool result = false;
	std::string powerAttr("");
	std::string fanAttr("");
	std::string powerResult("");
	std::string fanResult("");
	std::string dn_pers("");

	dn_pers = getDnBladePersistance(magazine,slot);
	data.dn_pfm = dn_pers;

	if (slot == 0)
	{

		data.module = CCH_Util::LOWER;
		powerAttr = IMM_Util::ATT_BLADESWPERSISTANCE_POWER_LOWER_STATE;
		powerResult = IMM_Util::ATT_BLADESWPERSISTANCE_POWER_LOWER_RESULT;
		fanAttr = IMM_Util::ATT_BLADESWPERSISTANCE_FAN_LOWER_STATE;
		fanResult = IMM_Util::ATT_BLADESWPERSISTANCE_FAN_LOWER_RESULT;


	}
	else if (slot == 25)
	{
		data.module = CCH_Util::UPPER;
		powerAttr = IMM_Util::ATT_BLADESWPERSISTANCE_POWER_UPPER_STATE;
		powerResult = IMM_Util::ATT_BLADESWPERSISTANCE_POWER_UPPER_RESULT;
		fanAttr = IMM_Util::ATT_BLADESWPERSISTANCE_FAN_UPPER_STATE;
		fanResult = IMM_Util::ATT_BLADESWPERSISTANCE_FAN_UPPER_RESULT;
	}


	int status = -1;
	int state = -1;
	int resultStatus = -1;

	//get fan status
	if (IMM_Util::getImmAttributeInt(dn_pers,fanAttr,status))
	{
		state = -1;
		if (status == IMM_Util::FW_IDLE) state = 0;
		else if (status == IMM_Util::FW_ONGOING)
		{
			state = 1;
			data.upgradeStatus = static_cast<CCH_Util::PFMStatus>(state); //set upgradeStatus
		}

		data.upgradeFanStatus = static_cast<CCH_Util::PFMStatus>(state); //set upgradeStatus

		if (IMM_Util::getImmAttributeInt(dn_pers,fanResult,resultStatus))
		{
			data.upgradeFanResult = static_cast<CCH_Util::PFMResult>(resultStatus); //set upgraderESULTStatus
		}

		result = true;
	}

	//get power status
	if (IMM_Util::getImmAttributeInt(dn_pers,powerAttr,status))
	{
		state = -1;
		if (status == IMM_Util::FW_IDLE) state = 0;
		else if (status == IMM_Util::FW_ONGOING)
		{
			state = 1;
			data.upgradeStatus = static_cast<CCH_Util::PFMStatus>(state); //set upgradeStatus
		}

		data.upgradePowerStatus = static_cast<CCH_Util::PFMStatus>(state); //set upgradeStatus

		if (IMM_Util::getImmAttributeInt(dn_pers,powerResult,resultStatus))
		{
			data.upgradePowerResult = static_cast<CCH_Util::PFMResult>(resultStatus); //set upgraderESULTStatus
		}

		result = true;
	}



	return result;

}

bool IMM_Interface::changePFMState(std::string dn, std::string attribute, int status)
{
	bool res = true;

	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(attribute.c_str(),ATTR_INT32T,&status,1);
	if (!IMM_Util::modify_OM_ImmAttr(dn.c_str(),changeState)== ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;

	return res;
}

bool IMM_Interface::getOtherBladeHardwareInfoDn(std::string dn, std::string& otherBladeHWinfoDN)
{
	std::vector<std::string> oneChild; //actually, this vector has to contain one element only
	bool bRet;
	std::string rdn("");
	std::string dn_child("");

	bRet = false;

	bRet = IMM_Util::getChildrenObject(dn, oneChild);

	for (unsigned int i=0; i<oneChild.size();i++)
	{
		rdn.clear();
		dn_child.clear();

		dn_child = oneChild[i];
		IMM_Util::getRdnAttribute(dn_child,rdn);
		if (strcmp(rdn.c_str(),IMM_Util::ATT_OTHERBLADEHARDWAREINFO_RDN) == 0)
		{
			otherBladeHWinfoDN = dn_child;
			bRet = true;
			break;
		}
	}

	return bRet;
}

bool IMM_Interface::getIpmiState(std::string dn, int &status)
{
	bool res = true;
	IMM_Util::getImmAttributeInt(dn.c_str(),IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_STATE,status);
	return res;
}

bool IMM_Interface::changeIpmiState(std::string dn, int status)
{
	bool res = true;

	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_STATE,ATTR_INT32T,&status,1);
	if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),changeState)== ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;

//	ACS_CC_ImmParameter FWstatus = IMM_Util::defineParameterInt(IMM_Util::ATT_OTHERBLADEHARDWAREINFO_IPMI_STATUS,ATTR_INT32T,&status,1);
//	if(FIXS_CCH_OtherBladeHardwareInfo::getInstance()->modifyRuntimeObj(dn.c_str(),&FWstatus) == ACS_CC_FAILURE) res = false;
//	delete [] FWstatus.attrValues;

	return res;
}

bool IMM_Interface::changeIpmiResult(std::string dn, int result)
{
	bool res = true;

	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT,ATTR_INT32T,&result,1);
	if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),changeState)== ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;

//	ACS_CC_ImmParameter FWstatus = IMM_Util::defineParameterInt(IMM_Util::ATT_OTHERBLADEHARDWAREINFO_IPMI_STATUS,ATTR_INT32T,&status,1);
//	if(FIXS_CCH_OtherBladeHardwareInfo::getInstance()->modifyRuntimeObj(dn.c_str(),&FWstatus) == ACS_CC_FAILURE) res = false;
//	delete [] FWstatus.attrValues;

	return res;
}

bool IMM_Interface::setIpmiUpgradeTime(std::string dn)
{
	bool res = true;
	std::string Current_Date("");
	std::string Current_Time("");

	CCH_Util::getCurrentTime(Current_Date,Current_Time);
	std::string dateTime = Current_Date + " " + Current_Time;
	ACS_APGCC::trimEnd(dateTime);

	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterString(IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_TIME,ATTR_STRINGT,dateTime.c_str(),1);
	if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;

	return res;
}

void IMM_Interface::printSoftwareRepository()
{
	IMM_Util::printDebugAllObjectClass(IMM_Util::classApManagedSwPackage);
	IMM_Util::printDebugAllObjectClass(IMM_Util::classCpManagedSwPackage);
}


bool IMM_Interface::getBoardDataInfo(const char* dn_blade, std::string* strMagazine, std::string* strSlot, std::string* boardMasterIP, std::string* boardPassiveIP, std::string* boardMasterSlot, bool* isPassiveDeg)
{
 	int iSlot = -1;
 	bool bOk = false;
 	FIXS_CCH_SNMPManager *snmpReader = NULL;
 	std::string shelfparent("");
 	std::string strEthA0("");
 	std::string strEthB0("");
 	std::string strEthA25("");
 	std::string strEthB25("");
 	std::vector<std::string> list;
 	std::stringstream ss_slot, ss_boardMasterSlot;
 	std::string strMagazine_tmp("");

 	// Init local vars
 	list.clear();
 	boardMasterIP->clear();
 	boardMasterSlot->clear();
 	strMagazine->clear();
 	strSlot->clear();
 	(*isPassiveDeg) = false;

	//Get Slot
	bOk = IMM_Util::getImmAttributeInt(dn_blade,IMM_Util::ATT_OTHERBLADE_SLOT,iSlot);
	ss_slot << iSlot;
	*strSlot = ss_slot.str();

 	// Check error status
 	if (bOk == true)
 	{
 		// Go to shelf
 		bOk = IMM_Util::getDnParent(dn_blade, shelfparent);
 		// Check error
 		if(bOk == true)
 		{
 			// Save the shelf
 			bOk = IMM_Util::getImmAttributeString(shelfparent, IMM_Util::ATT_SHELF_ADDRESS, strMagazine_tmp);
 			*strMagazine = strMagazine_tmp;
 		}
 		else
 		{
 			// Error to obtain OTHERBLADE!
 			std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain shelf from dn_blade:[" << dn_blade << "]." << std::endl;
 		}
 	}
 	// Check error status
 	if(bOk == true)
 	{
 		// Get the IPs of the board in the magazine shelfparent;
 		bOk = GetBoardIps(shelfparent,&strEthA0,&strEthB0,&strEthA25,&strEthB25);

 		// Check error
 		if(bOk == true)
 		{
 			// Print diagnostic
 			//std::cout << __FUNCTION__ << ": Found Board:" << std::endl;
 			//std::cout << "  - board in slot 0 : IPA[" << strEthA0.c_str() << "] ; IPB[" << strEthB0.c_str() << "]" << std::endl;
 			//std::cout << "  - board in slot 25: IPA[" << strEthA25.c_str() << "] ; IPB[" << strEthB25.c_str() << "]" << std::endl;
 		}
 		else
 			std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain board ip address[" << std::endl;
 	}
 	// Check error
 	if(bOk == true)
 	{
 		// search board master
 		snmpReader = new FIXS_CCH_SNMPManager(strEthA0);

 		int res = 0;
 		// Check if is master
 		res = snmpReader->isMaster(strEthA0);
		if(res == 1) //is master
		{
			// SCX0-EthA is master
			(*boardMasterIP) = strEthA0;
			(*boardPassiveIP) = strEthA25;
			ss_boardMasterSlot << 0;
		}
		else if( res == 2) //is passive
		{
			// SCX0-EthA is master
			(*boardMasterIP) = strEthA25;
			(*boardPassiveIP) = strEthA0;
			ss_boardMasterSlot << 25;
		}
		else if (res == HARDWARE_NOT_PRESENT)
		{
			// verify if scxb is master
			res = snmpReader->isMaster(strEthB25);
			if(res == 1) //is master
			{
				// SCX0-EthB is master
				(*boardMasterIP) = strEthB25;
				(*boardPassiveIP) = strEthB0;
				ss_boardMasterSlot << 25;
			}
			else if( res == 2 ) //is passive
			{
				(*boardMasterIP) = strEthB0;
				(*boardPassiveIP) = strEthB25;
				ss_boardMasterSlot << 0;
			}
			else if( res == HARDWARE_NOT_PRESENT )
			{
				bOk = false;
			}
		}
	}

 	*boardMasterSlot = ss_boardMasterSlot.str();

 	// Found board master ip. Verify if the slave board is passive degradate.
 	if((bOk == true) && (boardMasterIP->empty() == false))
 	{
 		//std::cout << "DBG: "<<__FUNCTION__ << "@" << __LINE__ << " ### Find master board." << std::endl;
 	}
 	else
 	{
 		// Error. Can't find master board
 		std::cout << "DBG: "<<__FUNCTION__ << "@" << __LINE__ << " ### ERROR can't find master board." << std::endl;
 		bOk = false;
 	}
 	// Release resource
 	if(snmpReader != NULL)
 	{
 		delete(snmpReader);
 		snmpReader = NULL;
 	}

 	// Exit method
 	return(bOk);
}

 bool IMM_Interface::GetBoardIps(const std::string &shelfparent, std::string *strEthA0, std::string *strEthB0, std::string *strEthA25, std::string *strEthB25)
{
  	 bool result = false;

	 bool bRet = false;
 	 bool bFoundScx0 = false;
 	 bool bExit = false;
 	 int iCount = 0;
 	 int iEnd = 0;
 	// int iSlot = -1;
 	 std::vector<std::string> vctBlades;
 	 std::string rdn("");
 	 int fbn = 0;

 	 //Get child list of shelf
 	 bRet = IMM_Util::getChildrenObject(shelfparent, vctBlades);

 	 // Check error
 	 if(bRet == true)
 	 {
 		 // Init end
 		 iEnd = (int)vctBlades.size();

 		 // cycle for child list
 		 while((iCount < iEnd) && (bRet == true) && (bExit == false))
 		 {
 			 // Get slot of the child
 			 rdn.clear();
			 /* We have to find otherBladeId only*/
			 bRet = IMM_Util::getRdnAttribute(vctBlades[iCount], rdn);

			 if(bRet == false) break; //exit condition of while loop

			 if(rdn.compare(IMM_Util::ATT_OTHERBLADE_RDN) != 0)
 			 {
 				 iCount++;
 				 continue; //not otherBladeId: check the next element
 			 }
			 //bRet = getOtherBladeSlot(vctBlades[iCount], iSlot);

			 bRet = IMM_Util::getImmAttributeInt(vctBlades[iCount],IMM_Util::ATT_OTHERBLADE_FBN,fbn);
//			 // Check error
 			 if(bRet == true)
 			 {
 				 // Verify if the slot is an SCX
 				 //if(iSlot == 0)
 				 if((fbn == IMM_Util::SCB_RP) || (fbn == IMM_Util::SCXB) || (fbn == IMM_Util::SMXB)) //TR_HW29936
 				 {
 					 // Found SCX 0. Save IPs.
 					std::string ipAattr = IMM_Util::ATT_OTHERBLADE_IPA;
 					if(bFoundScx0 == false)
 					{
 						bRet = IMM_Util::getImmAttributeString(vctBlades[iCount],ipAattr,*strEthA0);
 					}
 					else
 					{
 						bRet = IMM_Util::getImmAttributeString(vctBlades[iCount],ipAattr,*strEthA25);
 					}

 					// Check error
 					if (bRet == false)
 					{
 						std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain OtherBlade_IPA (SCX 0) from children list:[" << vctBlades[iCount].c_str() << "]." << std::endl;
 					}
 					else
 					{
 						// IP-B SCX 0
 						std::string ipBattr = IMM_Util::ATT_OTHERBLADE_IPB;
 						if(bFoundScx0 == false)
 						{
 							bRet = IMM_Util::getImmAttributeString(vctBlades[iCount],ipBattr,*strEthB0);
 						}
 						else
 						{
 							bRet = IMM_Util::getImmAttributeString(vctBlades[iCount],ipBattr,*strEthB25);
 						}
 						if (bRet == false)
 						{
 							std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain OtherBlade_IPB (SCX 0) from children list:[" << vctBlades[iCount].c_str() << "]." << std::endl;
 						}
 						else
 						{
 							// Set flag: found scx0.
 							if(bFoundScx0 == true)
 							{
 								// Set exit flag: two SCX has been found.
 								result = true;
 								bExit = true;
 							}
 							bFoundScx0 = true;
 						}
 					}
 				 }
// 				 else if(iSlot == 25)
// 				 {
// 					 // Fount SCX 25. Save IPs.
// 					std::string ipAattr = IMM_Util::ATT_OTHERBLADE_IPA;
// 					bRet = IMM_Util::getImmAttributeString(vctBlades[iCount],ipAattr,*strEthA25);
// 					// Check error
// 					if (bRet == false)
// 					{
// 						std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain OtherBlade_IPA (SCX 25) from children list:[" << vctBlades[iCount].c_str() << "]." << std::endl;
// 					}
// 					else
// 					{
// 						// IP-B SCX 25
// 						std::string ipBattr = IMM_Util::ATT_OTHERBLADE_IPB;
// 						bRet = IMM_Util::getImmAttributeString(vctBlades[iCount],ipBattr,*strEthB25);
// 						if (bRet == false)
// 						{
// 							std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain OtherBlade_IPB (SCX 25) from children list:[" << vctBlades[iCount].c_str() << "]." << std::endl;
// 						}
// 						else
// 						{
// 							if(bFoundScx0 == true)
// 							{
// 								// Set exit flag: two SCX has been found.
// 								result = true;
// 								bExit = true;
// 							}
// 						}
// 					}
// 				 }
 			 }
 			 else
 			 {
 				 // Error ATT_OTHERBLADE_SLOT
 				 std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain OtherBlade slot from children list:[" << vctBlades[iCount].c_str() << "]." << std::endl;
 			 }
 			 // Next blade
 			 ++iCount;
 		 }// end while
 	 }
 	 else
 	 {
 		 // Error to get child list
 		 std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain child list from shelf:[" << shelfparent.c_str() << "]." << std::endl;
 	 }
 	 // exit from method
 	 return result;

}


 bool IMM_Interface::GetScxbIps(std::string shelfparent, std::string &strEthA, std::string &strEthB, int slot)
 {
	 bool result = false;

	 bool check = false;

	 std::vector<std::string> vctBlades;
	 std::string rdn("");
	 std::string dn_blade("");

	 //Get child list of shelf
	 check = IMM_Util::getChildrenObject(shelfparent,vctBlades);

	 // Check error
	 if(check == true)
	 {
		 for (unsigned i=0; i<vctBlades.size(); i++)
		 {
			 // Get slot of the child
			 rdn.clear();
			 dn_blade.clear();

			 dn_blade = vctBlades[i].c_str();
			 /* We have to find otherBladeId only*/
			 IMM_Util::getRdnAttribute(dn_blade, rdn);

			 if(strcmp(rdn.c_str(),IMM_Util::ATT_OTHERBLADE_RDN) == 0)
			 {
				 int iSlot = -1;
				 IMM_Util::getImmAttributeInt(dn_blade,IMM_Util::ATT_OTHERBLADE_SLOT,iSlot);

				 if (iSlot == slot)
				 {
					 int fbn = 0;
					 IMM_Util::getImmAttributeInt(dn_blade,IMM_Util::ATT_OTHERBLADE_FBN,fbn);

					 if (fbn == IMM_Util::SCXB)
					 {
						 IMM_Util::getImmAttributeString(dn_blade,IMM_Util::ATT_OTHERBLADE_IPA,strEthA);
						 IMM_Util::getImmAttributeString(dn_blade,IMM_Util::ATT_OTHERBLADE_IPB,strEthB);

						 cout << "-------------- GetScxbIps - slot: "<< slot << " ipA: "<< strEthA.c_str() << " ipB: "<< strEthB.c_str() << endl;

						 result = true;
						 break;
					 }

				 }
			 }
		 }
	 }

	 return result;

 }

 bool IMM_Interface::getOtherBladeFBN(std::string dn, int &fbn)
 {
	 bool res = true;
	 if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_OTHERBLADE_FBN,fbn)) res = false;
	 return res;
 }

 bool IMM_Interface::getOtherBladeSlot(std::string dn, int &slot)
 {
	 bool res = true;
	 if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_OTHERBLADE_SLOT,slot)) res = false;
	 return res;
 }

 bool IMM_Interface::getOtherBladeIPs(std::string dn, std::string &ipA, std::string &ipB)
 {
	 bool res = true;
	 if (!IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_OTHERBLADE_IPA,ipA)) res = false;
	 if (!IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_OTHERBLADE_IPB,ipB)) res = false;
	 return res;
 }

 bool IMM_Interface::getShelfMagazine(std::string dn, std::string &magazine)
 {
	 bool res = true;
	 if (!IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_SHELF_ADDRESS,magazine)) res = false;
	 return res;
 }


 bool IMM_Interface::getDnMagazine (std::string magazine,std::string &dn)
 {
	 std::string shelf_dn("");
	 std::string mag("");
	 std::vector<std::string> shelfList;

	 //get Instances
	 IMM_Util::getClassObjectsList(IMM_Util::classShelf,shelfList);

	 //loop of instatnces
	 for (unsigned i=0; i<shelfList.size(); i++)
	 {
		 shelf_dn.clear();
		 shelf_dn = shelfList[i].c_str();

		 //get magazine of shelf
		 IMM_Util::getImmAttributeString(shelf_dn,IMM_Util::ATT_SHELF_ADDRESS,mag);

		 if (strcmp(mag.c_str(),magazine.c_str()) == 0)
		 {
			 dn = shelf_dn;
			 return true;
		 }

	 }

	 return false;
 }

 bool IMM_Interface::setDefaultPackage(std::string dn, int value)
 {
	 bool res = true;
	 ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterInt(IMM_Util::ATT_SWPACK_DEFAULT,ATTR_INT32T,&value,1);
	 if (!IMM_Util::modify_OM_ImmAttr(dn.c_str(),parToModify)) res = false;

	 return res;
 }

 bool IMM_Interface::getUsedPackageValue(std::string dn, std::vector<std::string> &value)
 {
	 bool res = true;
	 if (!IMM_Util::getImmMultiAttributeString(dn,IMM_Util::ATT_SWPACK_USED_BY,value)) res = false;
	 else CCH_Util::sortUsedVector(value);
	 return res;
 }


bool IMM_Interface::getReportProgressCurrSoftware(std::string dn, std::string &value)
{
	bool res = false;

	if (IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_SWINVENTORY_SWCURRENT,value)) res = true;

	return res;
}

bool IMM_Interface::createActionResultStruct()
{

	bool res = true;

	vector<ACS_CC_ValuesDefinitionType> AttrList;

	ACS_CC_ValuesDefinitionType RDN = IMM_Util::defineAttributeString(IMM_Util::ATT_BLADESWMACTION_RDN,ATTR_STRINGT,IMM_Util::RDN_BLADESWMACTION,1);


	int state = IMM_Util::NOT_AVAILABLE;
	int result = IMM_Util::NOT_AVAILABLE;
	int resultInfo = IMM_Util::EXECUTED;

	ACS_CC_ValuesDefinitionType STATE = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWMACTION_STATE,ATTR_INT32T,&state,1);
	ACS_CC_ValuesDefinitionType RESULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWMACTION_RESULT,ATTR_INT32T,&result,1);
	ACS_CC_ValuesDefinitionType RESULTINFO = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWMACTION_RESULTINFO,ATTR_INT32T,&resultInfo,1);


	AttrList.push_back(RDN);
	AttrList.push_back(STATE);
	AttrList.push_back(RESULT);
	AttrList.push_back(RESULTINFO);


	if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classBladeSwMAsyncActionResult,IMM_Util::RDN_BLADESWM,AttrList)) res = false;

	//free memory
	delete[] RDN.attrValues;
	RDN.attrValues = NULL;
	delete[] STATE.attrValues;
	STATE.attrValues = NULL;
	delete[] RESULT.attrValues;
	RESULT.attrValues = NULL;
	delete[] RESULTINFO.attrValues;
	RESULTINFO.attrValues = NULL;

	return res;

}

bool IMM_Interface::setBladeSwmReferenceStruct()
{
	bool res = true;
	ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterString(IMM_Util::ATT_BLADESWM_ACTIONPROGRESS,ATTR_NAMET,IMM_Util::DN_BLADESWMACTION,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::RDN_BLADESWM,parToModify)== ACS_CC_FAILURE) res = false;
	delete [] parToModify.attrValues;
	return res;
}

bool IMM_Interface::setSwManagedActionReferenceStruct(std::string dn, std::string value )
{
	bool res = true;
	std::string dn_ref = value + "," + dn;
	ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterString(IMM_Util::ATT_SWPACK_PROGRESS,ATTR_NAMET,dn_ref.c_str(),1);
	if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),parToModify)== ACS_CC_FAILURE) res = false;

	return res;
}

bool IMM_Interface::setSwMStructState(int status)
{
	bool res = true;
	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(IMM_Util::ATT_BLADESWMACTION_STATE,ATTR_INT32T,&status,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::DN_BLADESWMACTION,changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;

	return res;
}

bool IMM_Interface::setSwMStructResultInfo(int value)
{
	bool res = true;
	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(IMM_Util::ATT_BLADESWMACTION_RESULTINFO,ATTR_INT32T,&value,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::DN_BLADESWMACTION,changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;

	return res;
}

bool IMM_Interface::setSwMStructResult(int value)
{
	bool res = true;
	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(IMM_Util::ATT_BLADESWMACTION_RESULT,ATTR_INT32T,&value,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::DN_BLADESWMACTION,changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;

	return res;
}

bool IMM_Interface::setSwMStructTime()
{
	bool res = true;
	std::string Current_Date("");
	std::string Current_Time("");

	CCH_Util::getCurrentTime(Current_Date,Current_Time);
	std::string dateTime = Current_Date + " " + Current_Time;
	ACS_APGCC::trimEnd(dateTime);

	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterString(IMM_Util::ATT_BLADESWMACTION_TIME,ATTR_STRINGT,dateTime.c_str(),1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::DN_BLADESWMACTION,changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;

	return res;
}

bool IMM_Interface::getDnBoard (unsigned long uMagazine, unsigned short uSlot, std::string &dn)
{
	bool res = false;

	std::string shelf_dn("");
	std::string magazine("");
	std::string tempMag("");
	std::string sSlot("");

	CCH_Util::ulongToStringMagazine(uMagazine, magazine);
	CCH_Util::ushortToString(uSlot,sSlot);

	std::vector<std::string> shelfList,boardList;

	//get Instances
	IMM_Util::getClassObjectsList(IMM_Util::classShelf,shelfList);

	//loop of instatnces
	for (unsigned i=0; i<shelfList.size(); i++)
	{
		shelf_dn.clear();
		shelf_dn = shelfList[i].c_str();

		//get magazine of shelf
//		IMM_Util::getImmAttributeString(shelf_dn,IMM_Util::ATT_SHELF_ADDRESS,tempMag);
		IMM_Util::getRdnValue(shelf_dn,tempMag);

		if (strcmp(tempMag.c_str(),magazine.c_str()) == 0)
		{
			std::string board_dn("");
			std::string rdn_board("");
			IMM_Util::getChildrenObject(shelf_dn,boardList);

			for (unsigned j=0; j<boardList.size(); j++)
			{
				board_dn.clear();
				rdn_board.clear();
				board_dn = boardList[j].c_str();

				//check if is an object of OtherBlade Class
				IMM_Util::getRdnAttribute(board_dn,rdn_board);
				if ( strcmp(rdn_board.c_str(),IMM_Util::ATT_OTHERBLADE_RDN) == 0 )
				{
					std::string tempSlot("");
//					IMM_Util::getImmAttributeInt(board_dn,IMM_Util::ATT_OTHERBLADE_SLOT,tempSlot);
					IMM_Util::getRdnValue(board_dn,tempSlot);
					if (strcmp(tempSlot.c_str(),sSlot.c_str()) == 0)

					{
						dn = board_dn;
						res = true;
						break;
					}
				}
			}
		}
	}

	return res;
}

bool IMM_Interface::createApSwObject(std::string package,int isDefault,int forFBN,std::string swPath, std::string product, int swVerType)
{
	bool result = true;
	vector<ACS_CC_ValuesDefinitionType> AttrList;
//	int isFaulty = 0;
//	int upgType=0;

	ACS_CC_ValuesDefinitionType SwRepRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_APSWPACK_RDN,ATTR_STRINGT,package.c_str(),1);
	ACS_CC_ValuesDefinitionType SwRepDEFAULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWPACK_DEFAULT,ATTR_INT32T,&isDefault,1);
//	ACS_CC_ValuesDefinitionType SwRepFAULTY = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWPACK_FAULTY,ATTR_INT32T,&isFaulty,1);
	ACS_CC_ValuesDefinitionType SwRepFBN = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWPACK_FBN,ATTR_INT32T,&forFBN,1);
	ACS_CC_ValuesDefinitionType SwRepPATH = IMM_Util::defineAttributeString(IMM_Util::ATT_SWPACK_PATH,ATTR_STRINGT,swPath.c_str(),1);
	ACS_CC_ValuesDefinitionType SwRepCXC = IMM_Util::defineAttributeString(IMM_Util::ATT_SWPACK_CXC,ATTR_STRINGT,product.c_str(),1);
	ACS_CC_ValuesDefinitionType SwRepUSED = IMM_Util::defineAttributeString(IMM_Util::ATT_SWPACK_USED_BY,ATTR_STRINGT,"",1);
//	ACS_CC_ValuesDefinitionType SwRepPROGRESS = IMM_Util::defineAttributeString(IMM_Util::ATT_SWPACK_PROGRESS,ATTR_STRINGT,"",0);
	ACS_CC_ValuesDefinitionType SwVersionType = IMM_Util::defineAttributeInt(IMM_Util::ATT_SW_VERSION_TYPE,ATTR_INT32T,&swVerType,1);

	AttrList.push_back(SwRepRDN);
	AttrList.push_back(SwRepDEFAULT);
//	AttrList.push_back(SwRepFAULTY);
	AttrList.push_back(SwRepFBN);
	AttrList.push_back(SwRepPATH);
	AttrList.push_back(SwRepCXC);
	AttrList.push_back(SwRepUSED);
	AttrList.push_back(SwVersionType);
//	AttrList.push_back(SwRepPROGRESS);

	if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classApManagedSwPackage,IMM_Util::RDN_BLADESWM,AttrList)) result = false;

	//free memory
	delete[] SwRepRDN.attrValues;
	SwRepRDN.attrValues = NULL;
	delete[] SwRepDEFAULT.attrValues;
	SwRepDEFAULT.attrValues = NULL;
//	delete[] SwRepFAULTY.attrValues;
//	SwRepFAULTY.attrValues = NULL;
	delete[] SwRepUSED.attrValues;
	SwRepUSED.attrValues = NULL;
	delete[] SwRepFBN.attrValues;
	SwRepFBN.attrValues = NULL;
	delete[] SwRepPATH.attrValues;
	SwRepPATH.attrValues = NULL;
	delete[] SwRepCXC.attrValues;
	SwRepCXC.attrValues = NULL;
//	delete[] SwRepPROGRESS.attrValues;
//	SwRepPROGRESS.attrValues = NULL;
	delete[] SwVersionType.attrValues;
	SwVersionType.attrValues = NULL;

	return result;
}


bool IMM_Interface::createCpSwObject(std::string package, int isDefault,int forFBN, std::string swPath, std::string product)
{
	bool result = true;

	cout << "package: "<< package.c_str() << " isDefault: "<< isDefault <<" forFBN: "<< forFBN <<" swPath: "<< swPath.c_str() <<" product: "<< product.c_str() <<endl;
	vector<ACS_CC_ValuesDefinitionType> AttrList;
//	int isFaulty = 0;

	ACS_CC_ValuesDefinitionType SwRepRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_CPSWPACK_RDN,ATTR_STRINGT,package.c_str(),1);
	ACS_CC_ValuesDefinitionType SwRepDEFAULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWPACK_DEFAULT,ATTR_INT32T,&isDefault,1);
//	ACS_CC_ValuesDefinitionType SwRepFAULTY = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWPACK_FAULTY,ATTR_INT32T,&isFaulty,1);
	ACS_CC_ValuesDefinitionType SwRepFBN = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWPACK_FBN,ATTR_INT32T,&forFBN,1);
	ACS_CC_ValuesDefinitionType SwRepPATH = IMM_Util::defineAttributeString(IMM_Util::ATT_SWPACK_PATH,ATTR_STRINGT,swPath.c_str(),1);
	ACS_CC_ValuesDefinitionType SwRepCXC = IMM_Util::defineAttributeString(IMM_Util::ATT_SWPACK_CXC,ATTR_STRINGT,product.c_str(),1);
	ACS_CC_ValuesDefinitionType SwRepUSED = IMM_Util::defineAttributeString(IMM_Util::ATT_SWPACK_USED_BY,ATTR_STRINGT,"",1);

	AttrList.push_back(SwRepRDN);
	AttrList.push_back(SwRepDEFAULT);
//	AttrList.push_back(SwRepFAULTY);
	AttrList.push_back(SwRepFBN);
	AttrList.push_back(SwRepPATH);
	AttrList.push_back(SwRepCXC);
	AttrList.push_back(SwRepUSED);

	if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classCpManagedSwPackage,IMM_Util::RDN_BLADESWM,AttrList)) result = false;

	//free memory
	delete[] SwRepRDN.attrValues;
	SwRepRDN.attrValues = NULL;
	delete[] SwRepDEFAULT.attrValues;
	SwRepDEFAULT.attrValues = NULL;
//	delete[] SwRepFAULTY.attrValues;
//	SwRepFAULTY.attrValues = NULL;
	delete[] SwRepUSED.attrValues;
	SwRepUSED.attrValues = NULL;
	delete[] SwRepFBN.attrValues;
	SwRepFBN.attrValues = NULL;
	delete[] SwRepPATH.attrValues;
	SwRepPATH.attrValues = NULL;
	delete[] SwRepCXC.attrValues;
	SwRepCXC.attrValues = NULL;

	return result;
}

bool IMM_Interface::createBladeSwPersistance(unsigned long magazine, unsigned short slot)
{
	bool result = true;

	vector<ACS_CC_ValuesDefinitionType> AttrList;

	//convert magazine
	std::string sMagazine("");
	(void) CCH_Util::ulongToStringMagazine (magazine, sMagazine);

	std::string dn("");
	std::string cxp("");
	std::string cxc("");
	getDnBoard(magazine,slot,dn);
	getOtherBladeCurrLoadModule(dn,cxp);
	cxc = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(cxp);

	//convert slot
	std::string sSlot("");
	int iSlot = -1;
	CCH_Util::ushortToString(slot,sSlot);
	CCH_Util::stringToInt(sSlot,iSlot);

	std::string id = "bladeSwPersistanceValueId=" + sMagazine + "_" + sSlot;
	int status = IMM_Util::FW_IDLE;
	int resultState = IMM_Util::FW_OK;

	ACS_CC_ValuesDefinitionType RDN = IMM_Util::defineAttributeString(IMM_Util::ATT_BLADESWPERSISTANCE_RDN,ATTR_STRINGT,id.c_str(),1);
	ACS_CC_ValuesDefinitionType MAG = IMM_Util::defineAttributeString(IMM_Util::ATT_BLADESWPERSISTANCE_MAG,ATTR_STRINGT,sMagazine.c_str(),1);
	ACS_CC_ValuesDefinitionType SLOT = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_SLOT,ATTR_INT32T,&iSlot,1);
	ACS_CC_ValuesDefinitionType IPMI = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_STATE,ATTR_INT32T,&status,1);
	ACS_CC_ValuesDefinitionType IPMIRESULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT,ATTR_INT32T,&resultState,1);
	ACS_CC_ValuesDefinitionType IPMITIME = IMM_Util::defineAttributeString(IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_TIME,ATTR_STRINGT,"-",1);
	ACS_CC_ValuesDefinitionType POWER_UPPER = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_POWER_UPPER_STATE,ATTR_INT32T,&status,1);
	ACS_CC_ValuesDefinitionType POWER_UPPER_RESULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_POWER_UPPER_RESULT,ATTR_INT32T,&resultState,1);
	ACS_CC_ValuesDefinitionType POWER_LOWER = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_POWER_LOWER_STATE,ATTR_INT32T,&status,1);
	ACS_CC_ValuesDefinitionType POWER_LOWER_RESULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_POWER_LOWER_RESULT,ATTR_INT32T,&resultState,1);
	ACS_CC_ValuesDefinitionType FAN_UPPER = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_FAN_UPPER_STATE,ATTR_INT32T,&status,1);
	ACS_CC_ValuesDefinitionType FAN_UPPER_RESULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_FAN_UPPER_RESULT,ATTR_INT32T,&resultState,1);
	ACS_CC_ValuesDefinitionType FAN_LOWER = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_FAN_LOWER_STATE,ATTR_INT32T,&status,1);
	ACS_CC_ValuesDefinitionType FAN_LOWER_RESULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_BLADESWPERSISTANCE_FAN_LOWER_RESULT,ATTR_INT32T,&resultState,1);
	ACS_CC_ValuesDefinitionType CURRCXP = IMM_Util::defineAttributeString(IMM_Util::ATT_BLADESWPERSISTANCE_CURRENTCXP,ATTR_STRINGT,cxc.c_str(),1);
	ACS_CC_ValuesDefinitionType EXPCXP = IMM_Util::defineAttributeString(IMM_Util::ATT_BLADESWPERSISTANCE_EXPECTCXP,ATTR_STRINGT,"-",1);

	AttrList.push_back(RDN);
	AttrList.push_back(MAG);
	AttrList.push_back(SLOT);
	AttrList.push_back(IPMI);
	AttrList.push_back(IPMIRESULT);
	AttrList.push_back(IPMITIME);
	AttrList.push_back(POWER_UPPER);
	AttrList.push_back(POWER_UPPER_RESULT);
	AttrList.push_back(POWER_LOWER);
	AttrList.push_back(POWER_LOWER_RESULT);
	AttrList.push_back(FAN_UPPER);
	AttrList.push_back(FAN_UPPER_RESULT);
	AttrList.push_back(FAN_LOWER);
	AttrList.push_back(FAN_LOWER_RESULT);
	AttrList.push_back(CURRCXP);
	AttrList.push_back(EXPCXP);

	if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classBladeSwPersistance,IMM_Util::RDN_BLADESWM,AttrList)) result = false;

	//free memory
	delete[] RDN.attrValues;
	RDN.attrValues = NULL;
	delete[] MAG.attrValues;
	MAG.attrValues = NULL;
	delete[] SLOT.attrValues;
	SLOT.attrValues = NULL;
	delete[] IPMI.attrValues;
	IPMI.attrValues = NULL;
	delete[] IPMIRESULT.attrValues;
	IPMIRESULT.attrValues = NULL;
	delete[] IPMITIME.attrValues;
	IPMITIME.attrValues = NULL;
	delete[] POWER_UPPER.attrValues;
	POWER_UPPER.attrValues = NULL;
	delete[] POWER_UPPER_RESULT.attrValues;
	POWER_UPPER_RESULT.attrValues = NULL;
	delete[] POWER_LOWER.attrValues;
	POWER_LOWER.attrValues = NULL;
	delete[] POWER_LOWER_RESULT.attrValues;
	POWER_LOWER_RESULT.attrValues = NULL;
	delete[] FAN_UPPER.attrValues;
	FAN_UPPER.attrValues = NULL;
	delete[] FAN_UPPER_RESULT.attrValues;
	FAN_UPPER_RESULT.attrValues = NULL;
	delete[] FAN_LOWER.attrValues;
	FAN_LOWER.attrValues = NULL;
	delete[] FAN_LOWER_RESULT.attrValues;
	FAN_LOWER_RESULT.attrValues = NULL;
	delete[] CURRCXP.attrValues;
	CURRCXP.attrValues = NULL;
	delete[] EXPCXP.attrValues;
	EXPCXP.attrValues = NULL;

	return result;
}

bool IMM_Interface::deleteBrmPersistentDataOwnerObject()
{
	bool result = true;
	std::string dn(IMM_Util::DN_BRMPERSISTENTDATAOWNER);
	if (!IMM_Util::deleteImmObject(dn))
		result = false;
	return result;
}


bool IMM_Interface::deleteBladeSwPersistance(unsigned long magazine, unsigned short slot)
{
	bool result = true;

	std::string dn("");
	std::string rdn_obj("");

	//convert magazine
	std::string sMagazine("");
	(void) CCH_Util::ulongToStringMagazine (magazine, sMagazine);

	//convert slot
	std::string sSlot("");
	CCH_Util::ushortToString(slot,sSlot);

	std::string id = "bladeSwPersistanceValueId=" + sMagazine + "_" + sSlot;

	std::vector<std::string> persList;
	std::vector<std::string> persListChild;

	//get Instances
	IMM_Util::getClassObjectsList(IMM_Util::classBladeSwPersistance,persList);

	//loop of instances
	for (unsigned i=0; i<persList.size(); i++)
	{
		dn.clear();
		rdn_obj.clear();
		dn = persList[i].c_str();

		//check if exist
		IMM_Util::getRdnObject(dn,rdn_obj);
		if ( strcmp(rdn_obj.c_str(),id.c_str()) == 0 )
		{
			cout << "IMM_Interface: deleteBladeSwPersistance , try to delete object: "<< dn.c_str() << endl;

			IMM_Util::getChildrenObject(dn,persListChild);
			//loop of instances
			for (unsigned j=0; j<persListChild.size(); j++)
			{
				IMM_Util::deleteImmObject(persListChild[j]);
			}

			if (!IMM_Util::deleteImmObject(dn)) result = false;
			break;
		}
	}


	return result;
}

bool IMM_Interface::getPathSwPackage(std::string dn, std::string &value)
{
	bool res = false;

	if (IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_SWPACK_PATH,value)) res = true;

	return res;
}

bool IMM_Interface::createSwManagedActionResultObject(std::string parent, std::string package, std::string &id)
{
	bool res = true;

	vector<ACS_CC_ValuesDefinitionType> AttrList;

	id = "id=" + package;
	int state = IMM_Util::NOT_AVAILABLE;
	int result = IMM_Util::NOT_AVAILABLE;
	int resultInfo = IMM_Util::EXECUTED;

	ACS_CC_ValuesDefinitionType RDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWMACTION_RDN,ATTR_STRINGT,id.c_str(),1);
	ACS_CC_ValuesDefinitionType STATE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWMACTION_STATE,ATTR_INT32T,&state,1);
	ACS_CC_ValuesDefinitionType RESULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWMACTION_RESULT,ATTR_INT32T,&result,1);
	ACS_CC_ValuesDefinitionType RESULTINFO = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWMACTION_RESULTINFO,ATTR_INT32T,&resultInfo,1);
	ACS_CC_ValuesDefinitionType TIME = IMM_Util::defineAttributeString(IMM_Util::ATT_SWMACTION_TIME,ATTR_STRINGT,"-",1);

	AttrList.push_back(RDN);
	AttrList.push_back(STATE);
	AttrList.push_back(RESULT);
	AttrList.push_back(RESULTINFO);
	AttrList.push_back(TIME);


	if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classManagedSwPackageActionResult,parent.c_str(),AttrList)) res = false;

	//free memory
	delete[] RDN.attrValues;
	RDN.attrValues = NULL;
	delete[] STATE.attrValues;
	STATE.attrValues = NULL;
	delete[] RESULT.attrValues;
	RESULT.attrValues = NULL;
	delete[] RESULTINFO.attrValues;
	RESULTINFO.attrValues = NULL;
	delete[] TIME.attrValues;
	TIME.attrValues = NULL;

	return res;
}


bool IMM_Interface::setUsedPackage(std::string container, std::string value)
{
	bool res = true;
	std::string dn("");
//	std::string container = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(package);
	FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn,container);

	cout << "----------- setUsedPackage \n dn: "<< dn.c_str() << "\n container: "<< container.c_str() <<" value to set: "<< value.c_str() << endl;
	//get old value
	std::vector<std::string> oldUsedValue;
	std::vector<std::string> newUsedValue;
	IMM_Interface::getUsedPackageValue(dn,oldUsedValue);

	bool found = false;
	for (unsigned int i=0; i<oldUsedValue.size();i++)
	{
		if (strcmp(oldUsedValue[i].c_str(),"") != 0)
		{
			if (strcmp(oldUsedValue[i].c_str(),value.c_str()) != 0)
			{
				newUsedValue.push_back(oldUsedValue[i]);
				cout << "vect["<<i<<"] : "<< oldUsedValue[i].c_str() <<endl;
			}
			else
			{
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		//add new element
		newUsedValue.push_back(value);
		//sort vector
		CCH_Util::sortUsedVector(newUsedValue);

		ACS_CC_ImmParameter parToModify = IMM_Util::defineMultiParameterString(IMM_Util::ATT_SWPACK_USED_BY,ATTR_STRINGT,newUsedValue,newUsedValue.size());
		if (!IMM_Util::modify_OM_ImmAttr(dn.c_str(),parToModify)) res = false;
		delete [] parToModify.attrValues;

		//set internal map
		FIXS_CCH_SoftwareMap::getInstance()->setUsedContainer(container);
	}
	return res;
}

bool IMM_Interface::setNotUsedPackage(std::string container, std::string value)
{
	bool res = true;
	std::string dn("");
	FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn,container);

	cout << "----------- setNotUsedPackage \n dn: "<< dn.c_str() << "\n container: "<< container.c_str() <<" value to set: "<< value.c_str() << endl;

	//get old value
	std::vector<std::string> oldUsedValue;
	std::vector<std::string> newUsedValue;
	IMM_Interface::getUsedPackageValue(dn,oldUsedValue);


	for (unsigned int i=0; i<oldUsedValue.size();i++)
	{
		if (strcmp(oldUsedValue[i].c_str(),"") != 0)
		{
			if (strcmp(oldUsedValue[i].c_str(),value.c_str()) != 0)
			{
				newUsedValue.push_back(oldUsedValue[i]);
				cout << "vect["<<i<<"] : "<< oldUsedValue[i].c_str() <<endl;
			}
		}
	}
	if (newUsedValue.size() == 0)
	{
		newUsedValue.push_back("");
		FIXS_CCH_SoftwareMap::getInstance()->setNotUsedContainer(container);
	}

//	if (found)
//	{
		ACS_CC_ImmParameter parToModify = IMM_Util::defineMultiParameterString(IMM_Util::ATT_SWPACK_USED_BY,ATTR_STRINGT,newUsedValue,newUsedValue.size());
		if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),parToModify)== ACS_CC_FAILURE) res = false;
		delete [] parToModify.attrValues;

//	}
	return res;
}

bool IMM_Interface::setEmptyUsedPackage(std::string package)
{
	bool res = true;
	std::string dn("");
	std::string container = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(package);
	FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn,container);

	cout << "----------- setEmptyUsedPackage \n dn: "<< dn.c_str() << "\n container: "<< container.c_str() << endl;

	//get old value
	std::vector<std::string> oldUsedValue;
	oldUsedValue.push_back("");

	ACS_CC_ImmParameter parToModify = IMM_Util::defineMultiParameterString(IMM_Util::ATT_SWPACK_USED_BY,ATTR_STRINGT,oldUsedValue,oldUsedValue.size());
	if (!IMM_Util::modify_OM_ImmAttr(dn.c_str(),parToModify)) 
	{
		res = false;
		if (FIXS_CCH_DiskHandler::isStartedAfterRestore())
		{
			EventHandle m_shutdownEvent=0;
			const int WAIT_TIMEOUT= -2;
			const int WAIT_OBJECT_0=0;
			const int Shutdown_Event_Index=0;
			const int Number_Of_Alarm_Events=1;
			DWORD Safe_Timeout = 1000;
			bool m_exit=false;
			int count=0;
			if (m_shutdownEvent == 0)
				m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
			EventHandle handleArr=m_shutdownEvent;
			while (!m_exit) // Set to true when the thread shall exit
			{
				if(count >= 60){
					m_exit=true;
					break;
				}
				//wait for 1s
				int returnValue = FIXS_CCH_Event::WaitForEvents(Number_Of_Alarm_Events, &handleArr, Safe_Timeout);
				switch (returnValue)
				{
					case WAIT_TIMEOUT:  // Time out
					{
						if(IMM_Util::modify_OM_ImmAttr(dn.c_str(),parToModify))
						{
							res=true;
							m_exit = true;
						}
						else{
							res=false;
							count++;
						}
						break;
					}
					case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
					{
						m_exit = true;      // Terminate the thread
						break;
					}
					default:
					{
						count++;
						break;
					}
				} // End of switch
			}//while
		}
	}
	delete [] parToModify.attrValues;

	//set internal map
	FIXS_CCH_SoftwareMap::getInstance()->setNotUsedContainer(container);

	return res;
}

std::string IMM_Interface::getDnBladePersistance(unsigned long magazine, unsigned short slot)
{
	std::string dn_out("");
	std::string sMagazine("");
	std::string sSlot("");

	CCH_Util::ulongToStringMagazine(magazine,sMagazine);
	CCH_Util::ushortToString(slot,sSlot);

	std::string id = "bladeSwPersistanceValueId=" + sMagazine + "_" + sSlot;

	std::vector<std::string> persList;

	//get Instances
	IMM_Util::getClassObjectsList(IMM_Util::classBladeSwPersistance,persList);

	std::string dn_obj("");
	std::string rdn_obj("");

	//loop of instances
	for (unsigned i=0; i<persList.size(); i++)
	{
		dn_obj.clear();
		rdn_obj.clear();
		dn_obj = persList[i].c_str();

		//check if exist
		IMM_Util::getRdnObject(dn_obj,rdn_obj);
		if ( strcmp(rdn_obj.c_str(),id.c_str()) == 0 )
		{
			dn_out = dn_obj;
			break;
		}
	}

	return dn_out;
}

bool IMM_Interface::createReportProgressObject(std::string parent, std::string magazine, std::string slot, std::string &id)
{
	bool res = true;

	vector<ACS_CC_ValuesDefinitionType> AttrList;

	id = "reportProgressId=" + magazine + "_" + slot;
	int state = IMM_Util::IDLE;
	int result = IMM_Util::SW_OK;
	int resultInfo = IMM_Util::EXECUTED;
	int errorCode = 0;
	int upgradeType = IMM_Util::NORMAL_UPGRADE ;
	int force = 0;


	ACS_CC_ValuesDefinitionType RDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWINVENTORY_RDN,ATTR_STRINGT,id.c_str(),1);
	ACS_CC_ValuesDefinitionType STATE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWINVENTORY_STATE,ATTR_INT32T,&state,1);
	ACS_CC_ValuesDefinitionType RESULT = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWINVENTORY_RESULT,ATTR_INT32T,&result,1);
	ACS_CC_ValuesDefinitionType RESULTINFO = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWINVENTORY_REASON,ATTR_INT32T,&resultInfo,1);
	ACS_CC_ValuesDefinitionType TIME = IMM_Util::defineAttributeString(IMM_Util::ATT_SWINVENTORY_TIME,ATTR_STRINGT,"-",1);
	ACS_CC_ValuesDefinitionType ERRORCODE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWINVENTORY_ERRORCODE,ATTR_INT32T,&errorCode,1);
	
	ACS_CC_ValuesDefinitionType UPGRADETYPE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWINVENTORY_UPGRADETYPE,ATTR_INT32T,&upgradeType,1);

	ACS_CC_ValuesDefinitionType FORCEDUPGRADE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWINVENTORY_FORCEDUPGRADE,ATTR_INT32T,&force,1);

	AttrList.push_back(RDN);
	AttrList.push_back(STATE);
	AttrList.push_back(RESULT);
	AttrList.push_back(RESULTINFO);
	AttrList.push_back(TIME);
	AttrList.push_back(ERRORCODE);
	AttrList.push_back(UPGRADETYPE);
	AttrList.push_back(FORCEDUPGRADE);

	if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classReportProgress,parent.c_str(),AttrList)) res = false;

	//free memory
	delete[] RDN.attrValues;
	RDN.attrValues = NULL;
	delete[] STATE.attrValues;
	STATE.attrValues = NULL;
	delete[] RESULT.attrValues;
	RESULT.attrValues = NULL;
	delete[] RESULTINFO.attrValues;
	RESULTINFO.attrValues = NULL;
	delete[] TIME.attrValues;
	TIME.attrValues = NULL;
	delete[] ERRORCODE.attrValues;
	ERRORCODE.attrValues = NULL;
	delete[] UPGRADETYPE.attrValues;
	UPGRADETYPE.attrValues = NULL;
	delete[] FORCEDUPGRADE.attrValues;
	FORCEDUPGRADE.attrValues = NULL;
	return res;
}

bool IMM_Interface::setManagedSwStructState(std::string dn, int status)
{
	bool res = true;
	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(IMM_Util::ATT_SWMACTION_STATE,ATTR_INT32T,&status,1);
	if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " +++IMM RES VALUE: "<< res <<std::endl;
	return res;
}

bool IMM_Interface::setManagedSwStructResultInfo(std::string dn, int value)
{
	bool res = true;
	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(IMM_Util::ATT_SWMACTION_RESULTINFO,ATTR_INT32T,&value,1);
	if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " +++IMM RES VALUE: "<< res <<std::endl;
	return res;
}

bool IMM_Interface::setManagedSwStructResult(std::string dn, int value)
{
	bool res = true;
	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterInt(IMM_Util::ATT_SWMACTION_RESULT,ATTR_INT32T,&value,1);
	if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " +++IMM RES VALUE: "<< res <<std::endl;
	return res;
}

bool IMM_Interface::setManagedSwStructTime(std::string dn)
{
	bool res = true;
	std::string Current_Date("");
	std::string Current_Time("");

	CCH_Util::getCurrentTime(Current_Date,Current_Time);
	std::string dateTime = Current_Date + " " + Current_Time;
	ACS_APGCC::trimEnd(dateTime);

	ACS_CC_ImmParameter changeState = IMM_Util::defineParameterString(IMM_Util::ATT_SWMACTION_TIME,ATTR_STRINGT,dateTime.c_str(),1);
	if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),changeState) == ACS_CC_FAILURE) res = false;
	delete [] changeState.attrValues;
	std::cout << __FUNCTION__ << "@" << __LINE__ << " +++IMM RES VALUE: "<< res <<std::endl;
	return res;
}

bool IMM_Interface::getSwManagedActionReferenceStruct(std::string dn, std::string &dn_struct)
{
	bool res = true;

	if (IMM_Util::getImmAttributeString(dn,IMM_Util::ATT_SWPACK_PROGRESS,dn_struct)) res = true;

	return res;
}

int IMM_Interface::checkReportProgressObject(std::string mag, std::string slot, std::string container, CCH_Util::upgradeActionType actType)
{
	int res = EXECUTED;

	bool found = false;
	std::vector<std::string> reportList;
	std::string dn_parent("");
	FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn_parent, container);

	//check if exist an other object of the same hardware
	std::string rdn_rp = "reportProgressId=" + mag + "_" + slot;

	//get Instances
	IMM_Util::getClassObjectsList(IMM_Util::classReportProgress,reportList);

	std::string dn_obj("");
	std::string rdn_obj("");

	cout << __FUNCTION__ << " " << __LINE__<< "check old object !!! " << endl;

	//loop of instances
	for (unsigned i=0; i<reportList.size(); i++)
	{
		dn_obj.clear();
		rdn_obj.clear();
		dn_obj = reportList[i].c_str();

		//check if exist
		IMM_Util::getRdnObject(dn_obj,rdn_obj);
		if ( strcmp(rdn_obj.c_str(),rdn_rp.c_str()) == 0 )
		{
			found = true;
			cout << __FUNCTION__ << " " << __LINE__<< "old object found !!! " << endl;
			break;
		}
	}

	if (found)
	{
		int state = -1;
		std::string dn_parent_found("");
		IMM_Util::getDnParent(dn_obj,dn_parent_found);

		if (IMM_Util::getImmAttributeInt(dn_obj,IMM_Util::ATT_SWINVENTORY_STATE,state))
		{

			switch(actType)
			{

			case CCH_Util::prepareT:
			{
				if ((state != SW_IDLE))
				{
					if (( state != SW_FAILED))
					{
						cout << __FUNCTION__ << " " << __LINE__<< " WRONG_OPTION_PREPARE !!! " << endl;
						return WRONG_OPTION_PREPARE;
					}
					else
					{
						//check if the upgrade is requested for the same container
						//is useful to maintain the lastUpgradeCompleted attribute value
						if (dn_parent_found.compare(dn_parent))
						{
							cout << __FUNCTION__ << " " << __LINE__<< "deleteImmObject: " << dn_obj << endl;
							IMM_Util::deleteImmObject(dn_obj);
						}
					}
				}
				else
				{
					//check if the upgrade is requested for the same container
					//is useful to maintain the lastUpgradeCompleted attribute value
					if (dn_parent_found.compare(dn_parent))
					{
						cout << __FUNCTION__ << " " << __LINE__<< "deleteImmObject: " << dn_obj << endl;
						IMM_Util::deleteImmObject(dn_obj);
					}
				}

				break;
			}

			case CCH_Util::activateT:

				if (state != SW_IDLE && dn_parent_found.compare(dn_parent))
				return WRONG_OPTION_ACTIVATE;


			case CCH_Util::cancelT:

				if (state != SW_IDLE && dn_parent_found.compare(dn_parent))
				return WRONG_OPTION_CANCEL;

			case CCH_Util::commitT:

				if (state != SW_IDLE && dn_parent_found.compare(dn_parent))
				return WRONG_OPTION_COMMIT;

			default:
				break;
			}
		}
	}

	if (CCH_Util::prepareT == actType)
	{
		//check if the object already exist
		std::string obj_rp = rdn_rp + "," + dn_parent;
		ACS_APGCC_ImmObject object;
		if (!IMM_Util::getObject(obj_rp, &object))
		{
			std::string rdn_object("");
			if (!IMM_Interface::createReportProgressObject(dn_parent,mag,slot,rdn_object))
			{
				res = INTERNALERR;
			}
		}
	}

	return res;
}


bool IMM_Interface::readIpmiStatus (unsigned long magazine, unsigned short slot, int &upgradeStatus, int &upgradeResult, std::string &dn)
{
	bool result = false;
	std::string ipmiAttr("");
	std::string dn_pers("");
	int status = -1;
	int state = -1;
	int upgRes = -1;

	dn_pers = getDnBladePersistance(magazine,slot);

	ipmiAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_STATE;

	dn = dn_pers;

	//get ipmi status
	if (IMM_Util::getImmAttributeInt(dn_pers,ipmiAttr,status))
	{
		state = -1;
		if (status == IMM_Util::FW_IDLE) state = 0;
		else if (status == IMM_Util::FW_ONGOING)
		{
			state = 1;
		}

		upgradeStatus = state;
		if(IMM_Util::getImmAttributeInt(dn_pers, IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT, upgRes))
		{
			if(upgRes == IMM_Util::FW_OK) upgradeResult = CCH_Util::IPMI_OK;
			else upgradeResult = CCH_Util::IPMI_NOT_OK;
			result = true;
		}
	}

	return result;
}


bool IMM_Interface::readUpgradeStatus (unsigned long magazine, unsigned short slot, IpmiData &data)
{
	UNUSED(slot);
	bool result = false;

	std::string dn_pers("");
	std::string magValue("");
	std::string persMag("");
	std::vector<std::string> persList;
	int status = -1;
	int state = -1;
	int slotOngoing = -1;

	//convert ulong to string magazine
	CCH_Util::ulongToStringMagazine(magazine,magValue);

	//get list of objects
	IMM_Util::getClassObjectsList(IMM_Util::classBladeSwPersistance,persList);

	//loop of instances
	for (unsigned i=0; i<persList.size(); i++)
	{
		dn_pers.clear();
		persMag.clear();
		dn_pers = persList[i].c_str();

		if (IMM_Util::getImmAttributeString(dn_pers,IMM_Util::ATT_BLADESWPERSISTANCE_MAG,persMag))
		{
			if (strcmp(persMag.c_str(),magValue.c_str()) == 0)
			{
				if (IMM_Util::getImmAttributeInt(dn_pers,IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_STATE,status))
				{
					if (status == IMM_Util::FW_ONGOING)
					{
						state = CCH_Util::IPMI_ONGOING;

						data.dn_ipmi = dn_pers;

						data.upgradeStatus = static_cast<CCH_Util::IpmiStatus>(state);

						if (IMM_Util::getImmAttributeInt(dn_pers,IMM_Util::ATT_BLADESWPERSISTANCE_SLOT,slotOngoing))
						{
							data.slotUpgOnGoing = slotOngoing;

//							if(IMM_Util::getImmAttributeInt(dn_pers, IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT, upgRes))
//							{
//								data.upgradeResult = static_cast<CCH_Util::IpmiResult>(upgRes);
								result = true;
								break;
//							}
						}
					}
				}
			}
		}
	}

	return result;
}

bool IMM_Interface::updateUsedPackage(unsigned long magazine, unsigned short slot, std::string old_container, std::string new_container)
{
	bool result = false;
	std::string sSlot("");
	std::string sMag("");

	CCH_Util::ushortToString(slot, sSlot);
	CCH_Util::ulongToStringMagazine(magazine,sMag);
	std::string usedRef = sMag+"_"+ sSlot;

	cout << " updateUsedPackage usedRef: " << usedRef.c_str() << " old_container: " << old_container.c_str() << " new_container: " << new_container.c_str() << endl;

	//old used reference
	IMM_Interface::setNotUsedPackage(old_container,usedRef);

	//new used reference
	IMM_Interface::setUsedPackage(new_container,usedRef);
	return result;
}

int IMM_Interface::removeReportProgressObject(std::string mag, std::string slot, std::string container)
{
	int res = EXECUTED;

	bool found = false;
	std::vector<std::string> reportList;
	std::string dn_parent("");
	FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn_parent, container);

	//check if exist an other object of the same hardware
	std::string rdn_rp = "reportProgressId=" + mag + "_" + slot;

	//get Instances
	IMM_Util::getClassObjectsList(IMM_Util::classReportProgress,reportList);

	std::string dn_obj("");
	std::string rdn_obj("");

	cout << __FUNCTION__ << " " << __LINE__<< "check old object !!! " << endl;

	//loop of instances
	for (unsigned i=0; i<reportList.size(); i++)
	{
		dn_obj.clear();
		rdn_obj.clear();
		dn_obj = reportList[i].c_str();

		//check if exist
		IMM_Util::getRdnObject(dn_obj,rdn_obj);
		if ( strcmp(rdn_obj.c_str(),rdn_rp.c_str()) == 0 )
		{
			found = true;
			cout << __FUNCTION__ << " " << __LINE__<< "old object found !!! " << endl;
			break;
		}
	}

	if (found)
	{

		cout << __FUNCTION__ << " " << __LINE__<< "deleteImmObject: " << dn_obj << endl;
		IMM_Util::deleteImmObject(dn_obj);


	}

	return res;
}

bool IMM_Interface::getReportProgressObject(std::string mag, std::string slot, std::string &dn_rp)
{

	bool found = false;
	std::vector<std::string> reportList;

	//check if exist an other object of the same hardware
	std::string rdn_rp = "reportProgressId=" + mag + "_" + slot;

	//get Instances
	IMM_Util::getClassObjectsList(IMM_Util::classReportProgress,reportList);

	std::string dn_obj("");
	std::string rdn_obj("");

	cout << __FUNCTION__ << " " << __LINE__<< "check object !!! " << endl;

	//loop of instances
	for (unsigned i=0; i<reportList.size(); i++)
	{
		dn_obj.clear();
		rdn_obj.clear();
		dn_obj = reportList[i].c_str();

		//check if exist
		IMM_Util::getRdnObject(dn_obj,rdn_obj);
		if ( strcmp(rdn_obj.c_str(),rdn_rp.c_str()) == 0 )
		{
			found = true;
			dn_rp = dn_obj;
			cout << __FUNCTION__ << " " << __LINE__<< "object found !!! " << endl;
			break;
		}
	}

	return found;

}

bool IMM_Interface::setFaultyPackage(std::string dn, int value)
 {
	 bool res = true;

	 ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterInt(IMM_Util::ATT_SWPACK_FAULTY,ATTR_INT32T,&value,1);
	 if (IMM_Util::modify_OM_ImmAttr(dn.c_str(),parToModify)== ACS_CC_FAILURE) res = false;

	 return res;
 }

int IMM_Interface::checkBrmPersistentDataOwnerObject()
{
	int res = EXECUTED;

	//check if exist an other object of the same hardware
	std::string dn_brm = IMM_Util::DN_BRMPERSISTENTDATAOWNER;

	ACS_APGCC_ImmObject object;

	if (IMM_Util::getObject(dn_brm, &object))
	{
		cout <<"FOUND"<<endl;
	}
	else
	{
		cout << "No BrmPersistentDataOwnerObject object exists, create!" << endl;
		if(IMM_Interface::createBrmPersistentDataOwnerObject() != 0)
		{
			cout << "ERROR: Could not create BrmPersistentDataOwnerObject object!" <<endl;
			res = INTERNALERR;
		}
	}

	return res;
}

int IMM_Interface::createBrmPersistentDataOwnerObject()
{

	vector<ACS_CC_ValuesDefinitionType> AttrList;

	int backupType = 1;
//	int rebootAfterRestore = 0;
	int res = -1;

	ACS_CC_ValuesDefinitionType RDN = IMM_Util::defineAttributeString(IMM_Util::ATT_BRMPERSISTENTDATAOWNER_RDN,ATTR_STRINGT,IMM_Util::RDN_BRMPERSISTENTDATAOWNER,1);
	ACS_CC_ValuesDefinitionType VERSION = IMM_Util::defineAttributeString(IMM_Util::ATT_BRMPERSISTENTDATAOWNER_VERSION,ATTR_STRINGT,const_cast<char*>("1.0"),1);
	ACS_CC_ValuesDefinitionType BACKUPTYPE = IMM_Util::defineAttributeInt(IMM_Util::ATT_BRMPERSISTENTDATAOWNER_BACKUPTYPE,ATTR_INT32T,&backupType,1);
//	ACS_CC_ValuesDefinitionType REBOOTAFTERRESTORE = IMM_Util::defineAttributeInt(IMM_Util::ATT_BRMPERSISTENTDATAOWNER_REBOOTAFTERRESTORE,ATTR_INT32T,&rebootAfterRestore,1);

	AttrList.push_back(RDN);
	AttrList.push_back(VERSION);
	AttrList.push_back(BACKUPTYPE);
//	AttrList.push_back(REBOOTAFTERRESTORE);


	if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classBrmPersistentDataOwner,IMM_Util::RDN_BRMPARTICIPANTCONTAINER,AttrList))
	{
		cout << "BrfPersistentDataOwner Object Creation FAILURE for " << IMM_Util::RDN_BRMPERSISTENTDATAOWNER << endl;
		res = 1;
	}
	else
	{
		cout << "BrfPersistentDataOwner Object Creation Successful for " << IMM_Util::RDN_BRMPERSISTENTDATAOWNER << endl;
		res = 0;
	}

	//free memory
	delete[] RDN.attrValues;
	RDN.attrValues = NULL;
	delete[] VERSION.attrValues;
	VERSION.attrValues = NULL;
	delete[] BACKUPTYPE.attrValues;
	BACKUPTYPE.attrValues = NULL;
//	delete[] REBOOTAFTERRESTORE.attrValues;
//	REBOOTAFTERRESTORE.attrValues = NULL;

	return res;
}

bool IMM_Interface::getSwMStructState(int &value)
{
	bool res = true;
	if (!IMM_Util::getImmAttributeInt(IMM_Util::DN_BLADESWMACTION,IMM_Util::ATT_BLADESWMACTION_STATE,value)) res = false;
	return res;
}
bool IMM_Interface::getUpgTypeValue(std::string dn, int &value)
{
        bool res = true;
        if (!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_SWPACK_UPGRADETYPE,value)) res = false;
        return res;
}
bool IMM_Interface::getOtherBladeSysNum(std::string dn, int &sysnum)
{
        cout<<"getOtherBladeSysNum IN"<<endl;
        bool res = true;
        if(!IMM_Util::getImmAttributeInt(dn,IMM_Util::ATT_OTHERBLADE_SYSNUM,sysnum)) res = false;
        return res;
}

bool IMM_Interface::getOtherBoardInCluster(unsigned long uMagazine,unsigned short uSlot,int sysnum,unsigned short &iSlot)
{
	std::cout <<"getOtherBoardInCluster:: IN"<<std::endl;
	bool res = false;

	std::string shelf_dn("");
	std::string magazine("");
	std::string tempMag("");
	std::string sSlot("");

	CCH_Util::ulongToStringMagazine(uMagazine, magazine);
	CCH_Util::ushortToString(uSlot,sSlot);
	std::cout <<"getOtherBoardInCluster:: sSlot ===>"<<sSlot<<std::endl;

	std::cout <<"getOtherBoardInCluster:: magazine ===>"<<magazine<<std::endl;

	std::vector<std::string> shelfList,boardList;

	//get Instances
	IMM_Util::getClassObjectsList(IMM_Util::classShelf,shelfList);
	for (unsigned i=0; i<shelfList.size(); i++)
	{
		shelf_dn.clear();
		shelf_dn = shelfList[i].c_str();

		std::cout <<"getOtherBoardInCluster:: shelf_dn ===>"<<shelf_dn<<std::endl;

		//get magazine of shelf
		IMM_Util::getRdnValue(shelf_dn,tempMag);
		std::cout <<"getOtherBoardInCluster:: tempMag ===>"<<tempMag<<std::endl;
		if (strcmp(tempMag.c_str(),magazine.c_str()) == 0)
		{
			std::string board_dn("");
			std::string rdn_board("");
			IMM_Util::getChildrenObject(shelf_dn,boardList);
			for (unsigned j=0; j<boardList.size(); j++)
			{
				board_dn.clear();
				rdn_board.clear();
				board_dn = boardList[j].c_str();
				std::cout <<"getOtherBoardInCluster:: board_dn ===>"<<board_dn<<std::endl;

				//check if is an object of OtherBlade Class
				IMM_Util::getRdnAttribute(board_dn,rdn_board);
				std::cout <<"getOtherBoardInCluster:: rdn_board ===>"<<rdn_board<<std::endl;
				if ( strcmp(rdn_board.c_str(),IMM_Util::ATT_OTHERBLADE_RDN) == 0 )
				{
					std::string tempSlot("");
					IMM_Util::getRdnValue(board_dn,tempSlot);
					std::cout <<"getOtherBoardInCluster:: tempSlot ===>"<<tempSlot<<std::endl;
					int fbn = 0;
					if (getOtherBladeFBN(board_dn,fbn))
					{
						if(fbn == IMM_Util::IPLB)
						{
							int dn_sysnum = 0;
							if (getOtherBladeSysNum(board_dn,dn_sysnum))
							{
								std::cout<<"getOtherBoardInCluster:: dn_sysnum ==>"<<dn_sysnum<<std::endl;
								if(dn_sysnum == sysnum && (strcmp(tempSlot.c_str(),sSlot.c_str()) != 0))
								{
									int oSlot;
									CCH_Util::stringToInt(tempSlot,oSlot);
									iSlot = oSlot;
									res=true;
									return res;
								}

							}
						}
					}
				}
			}
		}
	}

	std::cout <<"getOtherBoardInCluster OUT"<<std::endl;
	return res;

}

void IMM_Interface::getEnvironmentTypeDirect(EnvironmentType &env)
{
	int systemType, shelfArch;

    //SINGLE-CP=0, MULTIPLE-CP=1
    if(!IMM_Util::getImmAttributeInt(IMM_Util::DN_APZ, IMM_Util::ATT_APZ_SYSTEM, systemType))
    {
    	std::cout << "DBG: Failed to get systemType" << std::endl;
    	env = UNKNOWN;
    	return;
    }
    else std::cout << "DBG: systemType = " << systemType << std::endl;

    //SCB = 0, SCX = 1, DMX = 2
    if(!IMM_Util::getImmAttributeInt(IMM_Util::DN_APZ, IMM_Util::ATT_APZ_CBA, shelfArch))
    {
    	std::cout << "DBG: Failed to get apgshelfArchitecture" << std::endl;
    	env = UNKNOWN;
    	return;
    }
    else std::cout << "DBG: shelfArch = " << shelfArch << std::endl;

    switch(shelfArch)
    {
    case 0: //SCB

    	if (systemType == 1) env = MULTIPLECP_NOTCBA;
    	else env = SINGLECP_NOTCBA;
    	break;

    case 1: //SCX

    	if (systemType == 1) env = MULTIPLECP_CBA;
    	else env = SINGLECP_CBA;
    	break;

    case 2: //DMX

    	if (systemType == 1) env = MULTIPLECP_DMX;
    	else env = SINGLECP_DMX;
    	break;

    default:
    	env = UNKNOWN;
    	break;
    }
    std::cout << "DBG: env type = " << env << std::endl;
}
bool IMM_Interface::getOtherBoardSlot(unsigned long umagazine,unsigned short uslot , unsigned short &islot)
{
    std::string dn_blade("");
    if(IMM_Interface::getDnBoard(umagazine,uslot,dn_blade))
    {
       int dn_sysnum =0;
       std::cout<<"dn_blade ==>"<<dn_blade.c_str()<<std::endl;
       if (IMM_Interface::getOtherBladeSysNum(dn_blade,dn_sysnum))
       {
          std::cout<<" dn_sysnum ==>"<<dn_sysnum<<std::endl;
          if(IMM_Interface::getOtherBoardInCluster(umagazine,uslot,dn_sysnum,islot))
          {
             return true;
          }

        }
    }
    islot=65;//getOtherBoardSlotFailed
    return false;
}

void IMM_Interface::getImmBladeSWObjects(std::vector<std::string> p_dnList, std::vector<std::string> & immCxpObject)
{
	std::string rdn_obj("");
	std::string dn_obj("");
	for (unsigned i=0; i<p_dnList.size(); i++)
	{
		dn_obj.clear();
		rdn_obj.clear();
		dn_obj = p_dnList[i].c_str();
		//check if exist
		IMM_Util::getRdnAttribute(dn_obj,rdn_obj);
		if (( strcmp(rdn_obj.c_str(),IMM_Util::ATT_APSWPACK_RDN) == 0 ) || (strcmp(rdn_obj.c_str(),IMM_Util::ATT_CPSWPACK_RDN) == 0))
		{
			int iPos = dn_obj.find_first_of(",");
			std::string cxpVersion = dn_obj.substr(0,iPos);
			iPos = cxpVersion.find_first_of("=");
			cxpVersion = cxpVersion.substr(iPos+1);
			std::cout << " CXP version: " << cxpVersion.c_str() << std::endl;
			immCxpObject.push_back(cxpVersion);
		}
	}
}
