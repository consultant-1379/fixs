/*
 * FIXS_CCH_ApManagedSw.cpp
 *
 *  Created on: Apr 20, 2012
 *      Author: eanform
 */

#include "FIXS_CCH_ApManagedSw.h"

/*============================================================================
	ROUTINE: FIXS_CCH_ApManagedSw
 ============================================================================ */
FIXS_CCH_ApManagedSw::FIXS_CCH_ApManagedSw() :  acs_apgcc_objectimplementerinterface_V3(IMM_Util::IMPLEMENTER_APSWPACKAGE),
m_ImmClassName(IMM_Util::classApManagedSwPackage),
thirdParam(false)
{
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
	ApActionThread = new FIXS_CCH_ThreadActions();
}

/*============================================================================
	ROUTINE: ~FIXS_CCH_ApManagedSw
 ============================================================================ */
FIXS_CCH_ApManagedSw::~FIXS_CCH_ApManagedSw()
{
	if (ApActionThread)
	{
		delete (ApActionThread);
		ApActionThread = NULL;
	}

	FIXS_CCH_logging = 0;
}

ACS_CC_ReturnType FIXS_CCH_ApManagedSw::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
{
	DEBUG("---------------------------------------------------");
	DEBUG("          ObjectCreateCallback invocated           ");
	DEBUG("---------------------------------------------------");
	DEBUG(" ObjectImplementer: " << getImpName());

	DEBUG(" Imm Handle: " << oiHandle);
	DEBUG(" ccbId: " << ccbId);
	DEBUG(" class Name: " << className);
	DEBUG(" parent Name: " << parentname);

	ACS_CC_ReturnType result = ACS_CC_FAILURE;

	// check if RDN attribute
	for(size_t idx = 0; attr[idx] != NULL ; idx++)
	{
		if( ACE_OS::strcmp(IMM_Util::ATT_SWPACK_FBN, attr[idx]->attrName) == 0 )
		{
			int fbn = (*reinterpret_cast<int *>(attr[idx]->attrValues[0]));
			DEBUG(" FBN: "<< fbn);

			addObjectCcbId(ccbId,objName,createT,"",fbn);
			result = ACS_CC_SUCCESS;
		}
	}

	return result;
}

ACS_CC_ReturnType FIXS_CCH_ApManagedSw::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	DEBUG("---------------------------------------------------");
	DEBUG("          ObjectDeleteCallback invocated           ");
	DEBUG("---------------------------------------------------");
	DEBUG(" ObjectImplementer: " << getImpName());

	DEBUG(" Imm Handle: " << oiHandle);
	DEBUG(" ccbId: " << ccbId);
	DEBUG(" object Name: " << objName);


	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	std::string container("");
	IMM_Util::getRdnValue(objName,container);

	addObjectCcbId(ccbId,objName,deleteT,container);

	return result;
}

ACS_CC_ReturnType FIXS_CCH_ApManagedSw::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	DEBUG("---------------------------------------------------");
	DEBUG("          ObjectModifyCallback invocated           ");
	DEBUG("---------------------------------------------------");
	DEBUG(" ObjectImplementer: " << getImpName());
	DEBUG(" Imm Handle: " << oiHandle);
	DEBUG(" ccbId: " << ccbId);
	DEBUG(" object Name: " << objName);

	int i = 0;
	while( attrMods[i] )
	{
		if (strcmp(attrMods[i]->modAttr.attrName, IMM_Util::ATT_SWPACK_DEFAULT) == 0)
		{
			int valueToSet = *reinterpret_cast<int *>(attrMods[i]->modAttr.attrValues[0]);

			std::string container("");
			IMM_Util::getRdnValue(objName,container);


			//new default value
			if (valueToSet == 1)
			{
				addObjectCcbId(ccbId,objName,modifyT,container,0,IMM_Util::ATT_SWPACK_DEFAULT);
			}
		}

		i++;
	}


	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}

ACS_CC_ReturnType FIXS_CCH_ApManagedSw::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	DEBUG("---------------------------------------------------");
	DEBUG("          CcbCompleteCallback invocated           ");
	DEBUG("---------------------------------------------------");
	DEBUG(" ObjectImplementer: " << getImpName());

	DEBUG(" Imm Handle: " << oiHandle);
	DEBUG(" ccbId: " << ccbId);

	DEBUG("COMPLETECallback ");

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	bool valid = true;


	for (mapObjectCcb::iterator itr = objectCcbId.begin(); itr != objectCcbId.end() && valid; itr++)
	{
		FIXS_CCH_ApManagedSw::CcbIdMapData data = itr->second;

		if (data.m_opt == createT)
		{
			if ((data.m_fbn != IMM_Util::SCXB) && (data.m_fbn != IMM_Util::IPLB) && (data.m_fbn != IMM_Util::IPTB) && (data.m_fbn != IMM_Util::CMXB) && (data.m_fbn != IMM_Util::SMXB) )
			{
				if(FIXS_CCH_logging) FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: IS NOT VALID FBN !!! ",LOG_LEVEL_ERROR);
				this->setExitCode(FAILED_FBN_NOT_ALLOWED, IMM_Util::getErrorText(FAILED_FBN_NOT_ALLOWED));
				valid = false;
				continue;
			}
		}

		if (data.m_opt == modifyT)
		{

			std::string sFbn("");
			sFbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(data.m_container);

			// 1) set as NOT DAFAULT all container for this fbn
			// 2) set as DAFAULT for this container
			// 3) update global string "default package"
			if( data.m_attrib == "isDefaultPackage")
			{
 				FIXS_CCH_SoftwareMap::getInstance()->setDefaultInMap(data.m_container,sFbn);
			}

			if((data.m_attrib == "upgradeType") && ((strcmp(sFbn.c_str(),"SCXB" )== 0) || (strcmp(sFbn.c_str(),"CMXB" )==0) || (strcmp(sFbn.c_str(),"SMXB" )==0)))
			{
				if(FIXS_CCH_logging) FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: IS NOT ALLOWED FOR THIS FBN !!! ",LOG_LEVEL_ERROR);
				this->setExitCode(FAILED_FBN_NOT_ALLOWED, IMM_Util::getErrorText(FAILED_FBN_NOT_ALLOWED));
				valid = false;
				continue;
			}


			if ((data.m_attrib == "upgradeType") && (strcmp(sFbn.c_str(),"IPLB" )== 0) && FIXS_CCH_UpgradeManager::getInstance()->isIplbLoadModuleUpgradeInProgress(data.m_container))
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: UPGRADE ONGOING!!! ",LOG_LEVEL_ERROR);
				this->setExitCode(MODIFY_UPGRADE_ONGOING, IMM_Util::getErrorText(MODIFY_UPGRADE_ONGOING));
				valid = false;
				break;
			}

			else if ((data.m_attrib == "upgradeType") && (strcmp(sFbn.c_str(),"IPTB" )== 0) && FIXS_CCH_UpgradeManager::getInstance()->isIptLoadModuleUpgradeInProgress(data.m_container))
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: UPGRADE ONGOING!!! ",LOG_LEVEL_ERROR);
				this->setExitCode(MODIFY_UPGRADE_ONGOING, IMM_Util::getErrorText(MODIFY_UPGRADE_ONGOING));
				valid = false;
				break;
			}

		}

		if (data.m_opt == deleteT)
		{
			std::string fbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(data.m_container);

			int sizePackageType = 0;
                        
			//check if the package is marked as faulty
			if (FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(data.m_container))
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: IS FAULTY PACKAGE !!! ",LOG_LEVEL_ERROR);
				this->setExitCode(DELETE_FAULTY_PACKAGE, IMM_Util::getErrorText(DELETE_FAULTY_PACKAGE));
				valid = false;
				break;
			}

			//SCXB
			if (strcmp(fbn.c_str(),"SCXB" ) == 0)
			{
				if (!FIXS_CCH_UpgradeManager::getInstance()->isScxSoftwareUpgradeInProgress())
				{

					if (!FIXS_CCH_SoftwareMap::getInstance()->isUsedPackage(data.m_container))
					{
						if (FIXS_CCH_SoftwareMap::getInstance()->isDefault(data.m_container))
						{
							//check if it is the last for this board type
							sizePackageType = FIXS_CCH_SoftwareMap::getInstance()->getScxSize();
							if (sizePackageType > 1)
							{
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: DEFAULT PACKAGE!!! ",LOG_LEVEL_ERROR);
								this->setExitCode(DELETE_DEFAULT_PACKAGE, IMM_Util::getErrorText(DELETE_DEFAULT_PACKAGE));
								valid = false;
								continue;
							}
						}
					}
					else
					{
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: PACKAGE IN USE!!! ",LOG_LEVEL_ERROR);
						this->setExitCode(DELETE_USED_PACKAGE, IMM_Util::getErrorText(DELETE_USED_PACKAGE));
						valid = false;
						break;
					}
				}
				else
				{
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: UPGRADE ONGOING!!! ",LOG_LEVEL_ERROR);
					this->setExitCode(DELETE_UPGRADE_ONGOING, IMM_Util::getErrorText(DELETE_UPGRADE_ONGOING));
					valid = false;
					break;
				}
			
                         }
                         
            //IPLB
			else if (strcmp(fbn.c_str(),"IPLB" ) == 0)
			{
				if(!FIXS_CCH_UpgradeManager::getInstance()->isIplbLoadModuleUpgradeInProgress(data.m_container))
				{
					if (!FIXS_CCH_SoftwareMap::getInstance()->isUsedPackage(data.m_container))
					{
						if (FIXS_CCH_SoftwareMap::getInstance()->isDefault(data.m_container))
						{
							//check if it is the last for this board type
							sizePackageType = FIXS_CCH_SoftwareMap::getInstance()->getIplbSize();
							if (sizePackageType > 1)
							{
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: DEFAULT PACKAGE!!! ",LOG_LEVEL_ERROR);
								this->setExitCode(DELETE_DEFAULT_PACKAGE, IMM_Util::getErrorText(DELETE_DEFAULT_PACKAGE));
								valid = false;
								continue;
							}
						}
					}
					else
					{
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: PACKAGE IN USE!!! ",LOG_LEVEL_ERROR);
						this->setExitCode(DELETE_USED_PACKAGE, IMM_Util::getErrorText(DELETE_USED_PACKAGE));
						valid = false;
						break;
					}
				}
				else
                                {
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: UPGRADE ONGOING!!! ",LOG_LEVEL_ERROR);
                                        this->setExitCode(DELETE_UPGRADE_ONGOING, IMM_Util::getErrorText(DELETE_UPGRADE_ONGOING));
                                        valid = false;
                                        break;
                                }
				
				
                        } 
			//CMXB
			else  if (strcmp(fbn.c_str(),"CMXB") == 0)
			{
				if (!FIXS_CCH_UpgradeManager::getInstance()->isCmxSoftwareUpgradeInProgress())
				{

					if (!FIXS_CCH_SoftwareMap::getInstance()->isUsedPackage(data.m_container))
					{
						if (FIXS_CCH_SoftwareMap::getInstance()->isDefault(data.m_container))
						{
							//check if it is the last for this board type
							sizePackageType = FIXS_CCH_SoftwareMap::getInstance()->getCmxSize();
							if (sizePackageType > 1)
							{
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: DEFAULT PACKAGE!!! ",LOG_LEVEL_ERROR);
								this->setExitCode(DELETE_DEFAULT_PACKAGE, IMM_Util::getErrorText(DELETE_DEFAULT_PACKAGE));
								valid = false;
								continue;
							}
						}
					}
					else
					{
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: PACKAGE IN USE!!! ",LOG_LEVEL_ERROR);
						this->setExitCode(DELETE_USED_PACKAGE, IMM_Util::getErrorText(DELETE_USED_PACKAGE));
						valid = false;
						break;
					}
				}
				else
				{
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: UPGRADE ONGOING!!! ",LOG_LEVEL_ERROR);
					this->setExitCode(DELETE_UPGRADE_ONGOING, IMM_Util::getErrorText(DELETE_UPGRADE_ONGOING));
					valid = false;
					break;
				}
			}

			//IPTB
			else if (strcmp(fbn.c_str(), "IPTB" ) == 0)
			{
				if (!FIXS_CCH_UpgradeManager::getInstance()->isIptLoadModuleUpgradeInProgress(data.m_container))
				{
					if (!FIXS_CCH_SoftwareMap::getInstance()->isUsedPackage(data.m_container))
					{
						if (FIXS_CCH_SoftwareMap::getInstance()->isDefault(data.m_container))
						{
							//check if it is the last for this board type
							sizePackageType = FIXS_CCH_SoftwareMap::getInstance()->getIptSize();
							if (sizePackageType > 1)
							{
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: DEFAULT PACKAGE!!! ",LOG_LEVEL_ERROR);
								this->setExitCode(DELETE_DEFAULT_PACKAGE, IMM_Util::getErrorText(DELETE_DEFAULT_PACKAGE));
								valid = false;
								continue;
							}
						}
					}
					else
					{
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: PACKAGE IN USE!!! ",LOG_LEVEL_ERROR);
						this->setExitCode(DELETE_USED_PACKAGE, IMM_Util::getErrorText(DELETE_USED_PACKAGE));
						valid = false;
						break;
					}

				}
				else
				{
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: UPGRADE ONGOING!!! ",LOG_LEVEL_ERROR);
					this->setExitCode(DELETE_UPGRADE_ONGOING, IMM_Util::getErrorText(DELETE_UPGRADE_ONGOING));
					valid = false;
					break;
				}
			}
			//SMXB
			else if (strcmp(fbn.c_str(),"SMXB" ) == 0)
			{
				if (!FIXS_CCH_UpgradeManager::getInstance()->isSmxSoftwareUpgradeInProgress())
				{
					if (!FIXS_CCH_SoftwareMap::getInstance()->isUsedPackage(data.m_container))
					{
						if (FIXS_CCH_SoftwareMap::getInstance()->isDefault(data.m_container))
						{
							//check if it is the last for this board type
							sizePackageType = FIXS_CCH_SoftwareMap::getInstance()->getSmxSize();
							if (sizePackageType > 1)
							{
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: DEFAULT PACKAGE!!! ",LOG_LEVEL_ERROR);
								this->setExitCode(DELETE_DEFAULT_PACKAGE, IMM_Util::getErrorText(DELETE_DEFAULT_PACKAGE));
								valid = false;
								continue;
							}
						}
					}
					else
					{
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: PACKAGE IN USE!!! ",LOG_LEVEL_ERROR);
						this->setExitCode(DELETE_USED_PACKAGE, IMM_Util::getErrorText(DELETE_USED_PACKAGE));
						valid = false;
						break;
					}
				}
				else
				{
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: UPGRADE ONGOING!!! ",LOG_LEVEL_ERROR);
					this->setExitCode(DELETE_UPGRADE_ONGOING, IMM_Util::getErrorText(DELETE_UPGRADE_ONGOING));
					valid = false;
					break;
				}			
            }
			else
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: UNKNOWN FBN !!! ",LOG_LEVEL_ERROR);
				this->setExitCode(DELETE_INVALID_FBN, IMM_Util::getErrorText(DELETE_INVALID_FBN));
				valid = false;
				break;
			}
		}

	}

	if (!valid)
	{
		cleanCcbMap();
		result = ACS_CC_FAILURE;
	}
	else
	{

		for (mapObjectCcb::iterator itr = objectCcbId.begin(); itr != objectCcbId.end() && valid; itr++)
		{
			FIXS_CCH_ApManagedSw::CcbIdMapData data = itr->second;

			if (data.m_opt == deleteT)
			{
				std::string fbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(data.m_container);
                                
                                
	                        DEBUG(" delete option2: " << fbn);

				if (strcmp(fbn.c_str(), "SCXB" ) == 0)
				{
					FIXS_CCH_SoftwareMap::getInstance()->removeScxEntry(data.m_container);
				}
				else if (strcmp(fbn.c_str(), "CMXB" ) == 0)
				{
					FIXS_CCH_SoftwareMap::getInstance()->removeCmxEntry(data.m_container);
				}
				else if (strcmp(fbn.c_str(), "IPTB" ) == 0)
				{
					FIXS_CCH_SoftwareMap::getInstance()->removeIptEntry(data.m_container);
				}
				else if (strcmp(fbn.c_str(),"IPLB" ) == 0)
				{
					FIXS_CCH_SoftwareMap::getInstance()->removeIplbEntry(data.m_container);
				}
				else if (strcmp(fbn.c_str(), "SMXB" ) == 0)
				{
					FIXS_CCH_SoftwareMap::getInstance()->removeSmxEntry(data.m_container);
				}
				else
				{
					DEBUG("INVALID FBN:" << fbn);
				}  

				//remove from DISK
				std::string pathParam("");
				IMM_Interface::getPathSwPackage(data.m_dn,pathParam);

				std::string pathFolder("");
				int iPos = pathParam.find_last_of(DIR_SEPARATOR);
				if(iPos >=0)
				{
					// remove filename
					pathFolder = pathParam.substr(0,iPos);
				}

				int respFolder = CCH_Util::removeFolder(pathFolder.c_str());
				if (respFolder != 0)
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,"FIXS_CCH_ApManagedSw - Remove Folder Failed error[%d]",respFolder);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					this->setExitCode(FAILED_REMOVE_FOLDER, IMM_Util::getErrorText(FAILED_REMOVE_FOLDER));
				}
			}

			//remove from object ccbId map
			removeObjectCcbId(data.m_dn);
		}

		cleanCcbMap();

	}

	return result;
}

void FIXS_CCH_ApManagedSw::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	DEBUG("---------------------------------------------------");
	DEBUG("          CcbAbortCallback invocated           ");
	DEBUG("---------------------------------------------------");
	DEBUG(" ObjectImplementer: " << getImpName());

	DEBUG(" Imm Handle: " << oiHandle);
	DEBUG(" ccbId: " << ccbId);

	DEBUG("ABORTCallback ");
}

void FIXS_CCH_ApManagedSw::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	DEBUG("---------------------------------------------------");
	DEBUG("          CcbApplyCallback invocated           ");
	DEBUG("---------------------------------------------------");
	DEBUG(" ObjectImplementer: " << getImpName());

	DEBUG(" Imm Handle: " << oiHandle);
	DEBUG(" ccbId: " << ccbId);

	DEBUG("APPLYCallback ");

}

ACS_CC_ReturnType FIXS_CCH_ApManagedSw::updateRuntime(const char* p_objName, const char** p_attrName)
{
	DEBUG("------------------------------------------------------------");
	DEBUG("                   updateRuntime called                     ");
	DEBUG("------------------------------------------------------------");

//	cout<<"Object Name: "<<p_objName<<endl;
//	cout<<"Attribute Name: "<<p_attrName<<endl;


	DEBUG("------------------------------------------------------------");

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}


void FIXS_CCH_ApManagedSw::adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_InvocationType invocation,
		const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
		ACS_APGCC_AdminOperationParamType**paramList) {

	DEBUG("------------------------------------------------------------");
	DEBUG("                   adminOperationCallback called             ");
	DEBUG("------------------------------------------------------------");

	DEBUG("oiHandle   : "<< oiHandle);
	DEBUG("invocation : "<< invocation);
	DEBUG("p_objName  : "<< p_objName);
	DEBUG("operationId: "<< operationId);

	DEBUG(" Implementing Admin Operation with ID = " << operationId);

	SaAisErrorT retVal;
	IMM_Util::ActionSwResultInfo resultValue = IMM_Util::ASW_EXECUTED;
	std::string magazine("");
	std::string slot("");
	std::string sFbn("");
	std::string product("");
	IMM_Util::BoardUpgradeType upgradeType;
	bool force;

	std::string container("");
	IMM_Util::getRdnValue(p_objName,container);
	sFbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(container);
	product = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(container);

	//check if the package is marked as faulty
	if (FIXS_CCH_SoftwareMap::getInstance()->checkFaultyContainers(sFbn))
	{
		//get Dn struct
		std::string dn_struct("");
		IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);
		//set values
		IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
		IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
		IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_ONE_INSTALLED_PACKAGE_IS_FAULTY);
		IMM_Interface::setManagedSwStructTime(dn_struct);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw: IS FAULTY PACKAGE !!! ",LOG_LEVEL_ERROR);
		retVal = SA_AIS_ERR_FAILED_OPERATION;
		this->setExitCode(FAULTY_PACKAGE, IMM_Util::getErrorText(FAULTY_PACKAGE));
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;

	}

	ApActionThread->setContainerToUpgrade(container);

//############## ACTION 1 ----PREPARE

	if (operationId == 1)
	{
		DEBUG(" ----------- ACTION 1 ----PREPARE ------------- ");

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : PREPARE ",LOG_LEVEL_DEBUG);

		resultValue = checkAllParameters(paramList,magazine,slot,upgradeType,force,product);
		if(IMM_Util::ASW_EXECUTED == resultValue)
		{
			if(!ApActionThread->isRunningThread())
				ApActionThread->open();

			ApActionThread->prepare(magazine,slot,container,upgradeType,force);

			retVal = SA_AIS_OK;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : PREPARING ACTION",LOG_LEVEL_DEBUG);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else
		{
			//get Dn struct
			std::string dn_struct("");
			IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);

			//set values
			IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
			IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
			IMM_Interface::setManagedSwStructResultInfo(dn_struct, resultValue);
			IMM_Interface::setManagedSwStructTime(dn_struct);


			retVal = SA_AIS_ERR_INVALID_PARAM;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : PREPARING ACTION, INVALID PARAMETERS ",LOG_LEVEL_ERROR);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
	}

//############## ACTION 2 ----ACTIVATE

	if (operationId == 2)
	{
		DEBUG(" ----------- ACTION 2 ----ACTIVATE ------------- ");

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : ACTIVATE ",LOG_LEVEL_DEBUG);

		resultValue = checkMagAndSlotParameters(paramList,magazine,slot,product);
		if(IMM_Util::ASW_EXECUTED == resultValue)
		{
			if(!ApActionThread->isRunningThread())
							ApActionThread->open();

			ApActionThread->activateSw(magazine,slot);
			retVal = SA_AIS_OK;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : ACTIVATE ACTION",LOG_LEVEL_DEBUG);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else
		{

			//get Dn struct
			std::string dn_struct("");
			IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);


			IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
			IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
			IMM_Interface::setManagedSwStructResultInfo(dn_struct, resultValue);
			IMM_Interface::setManagedSwStructTime(dn_struct);

			retVal = SA_AIS_ERR_INVALID_PARAM;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : ACTIVATE ACTION, INVALID PARAMETERS ",LOG_LEVEL_ERROR);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}


	}

//############## ACTION 3 ----CANCEL

	if (operationId == 3)
	{
		DEBUG(" ----------- ACTION 3 ---- CANCEL ------------- ");

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : CANCEL ",LOG_LEVEL_DEBUG);

		resultValue = checkMagAndSlotParameters(paramList,magazine,slot,product);
		if(IMM_Util::ASW_EXECUTED == resultValue)
		{
			if(!ApActionThread->isRunningThread())
				ApActionThread->open();

			ApActionThread->cancel(magazine,slot);
			retVal = SA_AIS_OK;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : CANCEL ACTION",LOG_LEVEL_DEBUG);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else
		{

			//get Dn struct
			std::string dn_struct("");
			IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);

			IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
			IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
			IMM_Interface::setManagedSwStructResultInfo(dn_struct, resultValue);
			IMM_Interface::setManagedSwStructTime(dn_struct);


			retVal = SA_AIS_ERR_INVALID_PARAM;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : CANCEL ACTION, INVALID PARAMETERS ",LOG_LEVEL_ERROR);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
	}

//############## ACTION 4 ----COMMIT

	if (operationId == 4)
	{
		DEBUG(" ----------- ACTION 4 ---- COMMIT ------------- ");

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : COMMIT ",LOG_LEVEL_DEBUG);

		resultValue = checkMagAndSlotParameters(paramList,magazine,slot,product);
		if(IMM_Util::ASW_EXECUTED == resultValue)
		{
			if(!ApActionThread->isRunningThread())
							ApActionThread->open();

			ApActionThread->commit(magazine,slot);
			retVal = SA_AIS_OK;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : COMMIT ACTION",LOG_LEVEL_DEBUG);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else
		{

			//get Dn struct
			std::string dn_struct("");
			IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);

			IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
			IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
			IMM_Interface::setManagedSwStructResultInfo(dn_struct, resultValue);
			IMM_Interface::setManagedSwStructTime(dn_struct);

			retVal = SA_AIS_ERR_INVALID_PARAM;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : COMMIT ACTION, INVALID PARAMETERS ",LOG_LEVEL_ERROR);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}


	}

//############## ACTION 5 ----SET AS DEFAULT

	if (operationId == 5)
	{
		DEBUG(" ----------- ACTION 5 ----SET AS DEFAULT------------- ");

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : SET AS DEFAULT ACTION ",LOG_LEVEL_DEBUG);

		if(!ApActionThread->isRunningThread())
			ApActionThread->open();

		ApActionThread->change_default_package();

		retVal = SA_AIS_OK;
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;
	}

//############## NO ACTION FOR THIS NUMBER

	if (operationId > 5)
	{
		retVal = SA_AIS_ERR_NO_OP;
		DEBUG(" ..... NO ACTION FOR THIS NUMBER  ");
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw : NO ACTION FOR THIS NUMBER ",LOG_LEVEL_ERROR);
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;
	}

	DEBUG("---------------------------END ACTION-------------------------");

}

void FIXS_CCH_ApManagedSw::addObjectCcbId(ACS_APGCC_CcbId ccbId, std::string dn, CCH_Util::operationCallBackType opt, std::string container, int fbn ,std::string attrib)
{
	mapObjectCcb::iterator it = objectCcbId.find(dn);
	if (it == objectCcbId.end())
	{
		FIXS_CCH_ApManagedSw::CcbIdMapData data;
		data.m_dn = dn;
		data.m_ccbId = ccbId;
		data.m_opt = opt;
		data.m_fbn = fbn;
		data.m_container = container;
		data.m_attrib = attrib;

		objectCcbId.insert(mapObjectCcb::value_type(data.m_dn, data));
		DEBUG(" new Object CcbId: " << data.m_dn);
		DEBUG(" ccbId: " << data.m_ccbId);
		DEBUG(" opt: " << data.m_opt);
		DEBUG(" fbn: " << data.m_fbn);
		DEBUG(" upgType: " << data.m_attrib);
		DEBUG(" container: " << data.m_container);
//		cout<<" new Object CcbId: \n" << data.m_dn << "\n ccbid: " << data.m_ccbId << "\n opt: " << data.m_opt << "\n fbn: " << data.m_fbn << "\n upgType: "<< data.m_attrib << "\n container: " << data.m_container <<std::endl;
	}
}


void FIXS_CCH_ApManagedSw::removeObjectCcbId(std::string dn)
{
	mapObjectCcb::iterator it = objectCcbId.find(dn);
	if (it != objectCcbId.end())
	{
		DEBUG(" ...RemoveObject dn: " << dn.c_str());
		objectCcbId.erase(it);
	}

}

void FIXS_CCH_ApManagedSw::cleanCcbMap()
{
	for (mapObjectCcb::iterator it = objectCcbId.begin(); it != objectCcbId.end(); it++)
	{
		objectCcbId.erase(it);
	}
}

IMM_Util::ActionSwResultInfo FIXS_CCH_ApManagedSw::checkMagAndSlotParameters(ACS_APGCC_AdminOperationParamType **paramList, std::string &mag, std::string &slot,  std::string product)
{

	//         std::cout << " ...paramList[0] slot: " << paramList[0]->attrName<< std::endl;
	//         std::cout << " ...paramList[1] shelf: " << paramList[1]->attrName<< std::endl;


	bool slotPar = false, magPar = false;
	for (int i = 0; paramList[i] != 0; i++)
	{
		if (strcmp((char *)paramList[i]->attrName,"slot") == 0 )
		{
			slot = reinterpret_cast<char *>(paramList[i]->attrValues);
			slotPar = true;
		}
		if (strcmp((char *)paramList[i]->attrName,"shelf") == 0 )
		{
			mag = reinterpret_cast<char *>(paramList[i]->attrValues);
			magPar = true;
		}
	}
	
	DEBUG(" mag " << mag.c_str()<< " slot " << slot.c_str() << "product: " << product.c_str() );

	if(slotPar && magPar)
	{
		unsigned long umagazine = 0;
		unsigned short uslot = 0;

		if (CCH_Util::stringToUlongMagazine(mag,umagazine))
		{
			if (CCH_Util::stringToUshort(slot,uslot))
			{
				/*if(((uslot == 0)||(uslot == 25)||(uslot == 26)||(uslot == 28)) && (thirdParam == true))
				{
					thirdParam=false;
					return IMM_Util::ASW_INVALID_PARAM;
				}
				else*/ if(CCH_Util::isBoardPresent(mag,slot))
				{
					if ( (FIXS_CCH_UpgradeManager::getInstance()->isScxData(umagazine, uslot) && FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(product, CCH_Util::SCXB))
							|| (FIXS_CCH_UpgradeManager::getInstance()->isCmxData(umagazine, uslot) && FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(product, CCH_Util::CMXB))
							|| (FIXS_CCH_UpgradeManager::getInstance()->isIplbData(umagazine, uslot) && FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(product, CCH_Util::IPLB))
							|| (FIXS_CCH_UpgradeManager::getInstance()->isIptData(umagazine, uslot) && FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(product, CCH_Util::IPTB))
							|| (FIXS_CCH_UpgradeManager::getInstance()->isSmxData(umagazine, uslot) && FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(product, CCH_Util::SMXB))
					)
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1,"FIXS_CCH_ApManagedSw - Action Parameters: magazine [%s] and slot [%s]",mag.c_str(), slot.c_str());
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

						return IMM_Util::ASW_EXECUTED;
					}
					else return IMM_Util::ASW_PACKAGE_NOT_APPLICABLE;
				}
				else return IMM_Util::ASW_HARDWARE_NOT_PRESENT;
			}
			else
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw - Action Parameters: invalid slot",LOG_LEVEL_ERROR);
			}
		}
		else
		{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw - Action Parameters: invalid magazine",LOG_LEVEL_ERROR);
		}
	}
	else return IMM_Util::ASW_INVALID_PARAM;

	return IMM_Util::ASW_HARDWARE_NOT_PRESENT;
}

IMM_Util::ActionSwResultInfo  FIXS_CCH_ApManagedSw::checkAllParameters(ACS_APGCC_AdminOperationParamType **paramList, std::string &mag, std::string &slot, IMM_Util::BoardUpgradeType  &upgradeType, bool &force, std::string product)
{
	//To check if default values are automatically taken from MOM or we have to assign
	force = false;
	upgradeType = IMM_Util::NORMAL_UPGRADE;

	for (int i = 0; paramList[i] != 0; i++)
	{
		if (strcmp((char *)paramList[i]->attrName,"force") == 0 )
		{
			force = *(reinterpret_cast<bool *>(paramList[i]->attrValues));
			thirdParam=true;
			DEBUG("force="<<force);
		}
		if (strcmp((char *)paramList[i]->attrName,"upgradeType") == 0 )
		{
			upgradeType = *(reinterpret_cast<IMM_Util::BoardUpgradeType *>(paramList[i]->attrValues));
			DEBUG("UPGRADETYPE="<<upgradeType);
			thirdParam=true;
		}
	}	

	return checkMagAndSlotParameters(paramList,mag,slot,product);
}
