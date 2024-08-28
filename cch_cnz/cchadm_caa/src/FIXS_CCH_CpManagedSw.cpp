/*
 * FIXS_CCH_CpManagedSw.cpp
 *
 *  Created on: Apr 20, 2012
 *      Author: eanform
 */

#include "FIXS_CCH_CpManagedSw.h"

/*============================================================================
	ROUTINE: FIXS_CCH_CpManagedSw
 ============================================================================ */
FIXS_CCH_CpManagedSw::FIXS_CCH_CpManagedSw() :  acs_apgcc_objectimplementerinterface_V3(IMM_Util::IMPLEMENTER_CPSWPACKAGE),
m_ImmClassName(IMM_Util::classCpManagedSwPackage)
{
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
	CpActionThread = new FIXS_CCH_ThreadActions();
}

/*============================================================================
	ROUTINE: ~FIXS_CCH_CpManagedSw
 ============================================================================ */
FIXS_CCH_CpManagedSw::~FIXS_CCH_CpManagedSw()
{
	if (CpActionThread)
	{
		delete (CpActionThread);
		CpActionThread = NULL;
	}

	FIXS_CCH_logging = 0;
}

ACS_CC_ReturnType FIXS_CCH_CpManagedSw::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " class Name: " << className << endl;
	cout << " parent Name: " << parentname << endl;

	ACS_CC_ReturnType result = ACS_CC_FAILURE;

	// check if RDN attribute
	for(size_t idx = 0; attr[idx] != NULL ; idx++)
	{
		if( ACE_OS::strcmp(IMM_Util::ATT_SWPACK_FBN, attr[idx]->attrName) == 0 )
		{
			int fbn = (*reinterpret_cast<int *>(attr[idx]->attrValues[0]));
			cout << __FUNCTION__<< " " << __LINE__ << " FBN: "<< fbn <<endl;

			addObjectCcbId(ccbId,objName,createT,"",fbn);
			result = ACS_CC_SUCCESS;
		}
	}

	return result;
}

ACS_CC_ReturnType FIXS_CCH_CpManagedSw::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectDeleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;


	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	std::string container("");
	IMM_Util::getRdnValue(objName,container);

	addObjectCcbId(ccbId,objName,deleteT,container);

	return result;
}

ACS_CC_ReturnType FIXS_CCH_CpManagedSw::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectModifyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	cout << "MODIFYCallback " << endl;
	cout << endl;

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
				addObjectCcbId(ccbId,objName,modifyT,container);
			}

		}

		i++;
	}


	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}

ACS_CC_ReturnType FIXS_CCH_CpManagedSw::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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
	bool valid = true;

	for (mapObjectCcb::iterator itr = objectCcbId.begin(); itr != objectCcbId.end() && valid; itr++)
	{
		FIXS_CCH_CpManagedSw::CcbIdMapData data = itr->second;

		if (data.m_opt == createT)
		{
			if ((data.m_fbn != IMM_Util::EPB1) && (data.m_fbn != IMM_Util::EVOET))
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw: IS NOT EPB1 or EVOET entry!!! ",LOG_LEVEL_ERROR);
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
			FIXS_CCH_SoftwareMap::getInstance()->setDefaultInMap(data.m_container,sFbn);

		}

		if (data.m_opt == deleteT)
		{
			std::string fbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(data.m_container);

			int sizePackageType = 0;

			//check if the package is marked as faulty
			if (FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(data.m_container))
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw: IS FAULTY PACKAGE !!! ",LOG_LEVEL_ERROR);
				this->setExitCode(DELETE_FAULTY_PACKAGE, IMM_Util::getErrorText(DELETE_FAULTY_PACKAGE));
				valid = false;
				break;
			}

			if (strcmp(fbn.c_str(), "EPB1" ) == 0)
			{
				if (!FIXS_CCH_SoftwareMap::getInstance()->isUsedPackage(data.m_container))
				{
					if (FIXS_CCH_SoftwareMap::getInstance()->isDefault(data.m_container))
					{
						//check if it is the last for this board type
						sizePackageType = FIXS_CCH_SoftwareMap::getInstance()->getEpb1Size();

						if (sizePackageType > 1)
						{
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw: DEFAULT PACKAGE!!! ",LOG_LEVEL_ERROR);
							this->setExitCode(DELETE_DEFAULT_PACKAGE, IMM_Util::getErrorText(DELETE_DEFAULT_PACKAGE));
							valid = false;
//							continue;
							break;
						}
					}
				}
				else
				{
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw: PACKAGE IN USE!!! ",LOG_LEVEL_ERROR);
					this->setExitCode(DELETE_USED_PACKAGE, IMM_Util::getErrorText(DELETE_USED_PACKAGE));
					valid = false;
//					continue;
					break;
				}

			}

			//EVOET
			else if (strcmp(fbn.c_str(), "EVOET" ) == 0)
			{
				if (!FIXS_CCH_SoftwareMap::getInstance()->isUsedPackage(data.m_container))
				{
					if (FIXS_CCH_SoftwareMap::getInstance()->isDefault(data.m_container))
					{
						//check if it is the last for this board type
						sizePackageType = FIXS_CCH_SoftwareMap::getInstance()->getEvoEtSize();

						if (sizePackageType > 1)
						{
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw: DEFAULT PACKAGE!!! ",LOG_LEVEL_ERROR);
							this->setExitCode(DELETE_DEFAULT_PACKAGE, IMM_Util::getErrorText(DELETE_DEFAULT_PACKAGE));
							valid = false;
//							continue;
							break;
						}
					}
				}
				else
				{
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw: PACKAGE IN USE!!! ",LOG_LEVEL_ERROR);
					this->setExitCode(DELETE_USED_PACKAGE, IMM_Util::getErrorText(DELETE_USED_PACKAGE));
					valid = false;
//					continue;
					break;
				}

			}
			else
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw: UNKNOWN FBN !!! ",LOG_LEVEL_ERROR);
				this->setExitCode(DELETE_INVALID_FBN, IMM_Util::getErrorText(DELETE_INVALID_FBN));
				valid = false;
//				continue;
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
			FIXS_CCH_CpManagedSw::CcbIdMapData data = itr->second;

			if (data.m_opt == deleteT)
			{
				std::string fbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(data.m_container);

				if (strcmp(fbn.c_str(), "EPB1" ) == 0)
				{
					FIXS_CCH_SoftwareMap::getInstance()->removeEpb1Entry(data.m_container);
				}
				else if (strcmp(fbn.c_str(), "EVOET" ) == 0)
				{
					FIXS_CCH_SoftwareMap::getInstance()->removeEvoEtEntry(data.m_container);
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
					snprintf(tmpStr, sizeof(tmpStr) - 1,"FIXS_CCH_CpManagedSw - Remove Folder Failed error[%d]",respFolder);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					this->setExitCode(1003,"FAILED_REMOVING_FOLDER");
				}
			}

			//remove from object ccbId map
			removeObjectCcbId(data.m_dn);
		}

		cleanCcbMap();

	}


	return result;
}

void FIXS_CCH_CpManagedSw::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

void FIXS_CCH_CpManagedSw::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
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

ACS_CC_ReturnType FIXS_CCH_CpManagedSw::updateRuntime(const char* p_objName, const char** p_attrName)
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


void FIXS_CCH_CpManagedSw::adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_InvocationType invocation,
		const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
		ACS_APGCC_AdminOperationParamType**paramList) {

	printf("------------------------------------------------------------\n");
	printf("                   adminOperationCallback called             \n");
	printf("------------------------------------------------------------\n");

	cout<<"oiHandle   : "<< oiHandle <<endl;
	cout<<"invocation : "<< invocation <<endl;
	cout<<"p_objName  : "<< p_objName <<endl;
	cout<<"operationId: "<< operationId <<endl;

	cout << " Implementing Admin Operation with ID = " << operationId << endl;

	SaAisErrorT retVal;
	std::string magazine("");
	std::string slot("");
	std::string container("");
	std::string sFbn("");
	int retval = 0;
	IMM_Util::getRdnValue(p_objName,container);
	sFbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(container);

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
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw: IS FAULTY PACKAGE !!! ",LOG_LEVEL_ERROR);
		retVal = SA_AIS_ERR_FAILED_OPERATION;
		this->setExitCode(FAULTY_PACKAGE, IMM_Util::getErrorText(FAULTY_PACKAGE));
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;

	}
	CpActionThread->setContainerToUpgrade(container);


//############## ACTION 1 ----PREPARE ACTION

	if (operationId == 1)
	{
		cout << " ----------- ACTION 1 ----PREPARE-------------\n  " << endl;

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : PREPARE ACTION ",LOG_LEVEL_DEBUG);

		retval = checkMagAndSlotParameters(paramList,magazine,slot);

		if(retval == CCH_Util::PREPARE_SLOT)
		{
			retVal = SA_AIS_ERR_INVALID_PARAM;
			setAdminOperationError(FAILED_INVALID_PARAMETERS,oiHandle,invocation);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : PREPAREUPGRADE WITHOUT MAG  ACTION, FAILED ",LOG_LEVEL_ERROR);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}

		retVal = handlePrepareAction(retval,magazine,slot,sFbn,container);

		if(retVal != SA_AIS_OK)
		{
			//get Dn struct
			std::string dn_struct("");
			IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);
			//check for board presence
			if(CCH_Util::isBoardPresent(magazine,slot))
			{
				this->setExitCode(FAILED_FBN_NOT_ALLOWED, IMM_Util::getErrorText(FAILED_FBN_NOT_ALLOWED));
				IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_PACKAGE_NOT_APPLICABLE);
			}
			else
			{
				this->setExitCode(FAILED_HARDWARE_NOT_PRESENT, IMM_Util::getErrorText(FAILED_HARDWARE_NOT_PRESENT));
				IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_HARDWARE_NOT_PRESENT);
			}

			//set values
			IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
			IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
			IMM_Interface::setManagedSwStructTime(dn_struct);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else
		{
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}

	}

//############## ACTION 2 ----PREPARE Shelf ACTION
//prepare multiple slot in the shelf
	if (operationId == 2)
	{
		cout << " ----------- ACTION 2 ----PREPARE SHELF-------------\n  " << endl;

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : PREPARE SHELF ACTION ",LOG_LEVEL_DEBUG);
		retval = checkMagAndSlotParameters(paramList,magazine,slot);
		if(retval == CCH_Util::PREPARE_SLOT)
		{
			retVal = SA_AIS_ERR_INVALID_PARAM;
			setAdminOperationError(FAILED_INVALID_PARAMETERS,oiHandle,invocation);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : PREPAREUPGRADE WITHOUT MAG  ACTION, FAILED ",LOG_LEVEL_ERROR);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		retVal = handlePrepareAction(retval,magazine,slot,sFbn,container);
		if(retVal != SA_AIS_OK)
		{
			//get Dn struct
			std::string dn_struct("");
			IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);
			//check for board presence
			if(CCH_Util::isBoardPresent(magazine,slot))
			{
				this->setExitCode(FAILED_FBN_NOT_ALLOWED, IMM_Util::getErrorText(FAILED_FBN_NOT_ALLOWED));
				IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_PACKAGE_NOT_APPLICABLE);
			}
			else
			{
				this->setExitCode(FAILED_HARDWARE_NOT_PRESENT, IMM_Util::getErrorText(FAILED_HARDWARE_NOT_PRESENT));
				IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_HARDWARE_NOT_PRESENT);
			}

			//set values
			IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
			IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
			IMM_Interface::setManagedSwStructTime(dn_struct);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else
		{
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}

	}

	//############## ACTION 3 ----PREPARE ALL ACTION
	//prepare multiple slot in all the defined shelfs
	if (operationId == 3)
	{
		cout << " ----------- ACTION 3 ----PREPARE ALL-------------\n  " << endl;

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : PREPARE ALL ACTION ",LOG_LEVEL_DEBUG);

		if (strcmp(sFbn.c_str(),"EPB1") == 0)
		{
			if(!CpActionThread->isRunningThread())
				CpActionThread->open();

			CpActionThread->prepare_all(container, CCH_Util::EPB1);

			retVal = SA_AIS_OK;
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else if (strcmp(sFbn.c_str(),"EVOET") == 0)
		{
			if(!CpActionThread->isRunningThread())
				CpActionThread->open();

			CpActionThread->prepare_all(container, CCH_Util::EVOET);

			retVal = SA_AIS_OK;
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else
		{
			//get Dn struct
			std::string dn_struct("");
			IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);
			//check for board presence
			if(CCH_Util::isBoardPresent(magazine,slot))
			{
				this->setExitCode(FAILED_FBN_NOT_ALLOWED, IMM_Util::getErrorText(FAILED_FBN_NOT_ALLOWED));
				IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_PACKAGE_NOT_APPLICABLE);
			}
			else
			{

				this->setExitCode(FAILED_HARDWARE_NOT_PRESENT, IMM_Util::getErrorText(FAILED_HARDWARE_NOT_PRESENT));
				IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_HARDWARE_NOT_PRESENT);
			}

			//set values
			IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
			IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
			IMM_Interface::setManagedSwStructTime(dn_struct);


			retVal = SA_AIS_ERR_FAILED_OPERATION;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : PREPARING SHELF ACTION, FBN NOT ALLOWED ",LOG_LEVEL_ERROR);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}

	}

	//############## ACTION 4 ----SET AS DEFAULT

	if (operationId == 4)
	{
		cout << " ----------- ACTION 4 ----SET AS DEFAULT-------------\n  " << endl;

		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : SET AS DEFAULT ACTION ",LOG_LEVEL_DEBUG);

		if(!CpActionThread->isRunningThread())
			CpActionThread->open();

		CpActionThread->change_default_package();

		retVal = SA_AIS_OK;
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;
	}

	//############## ACTION 5 ----PREPAREUPGRADE
	//prepareUpgrade
	if (operationId == 5)
	{
		cout<<"operationid 5"<<endl;
		retval = checkMagAndSlotParameters(paramList,magazine,slot);
		if(retval == CCH_Util::PREPARE_SLOT)
		{
			retVal = SA_AIS_ERR_INVALID_PARAM;
			setAdminOperationError(FAILED_INVALID_PARAMETERS,oiHandle,invocation);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : PREPAREUPGRADE WITHOUT MAG  ACTION, FAILED ",LOG_LEVEL_ERROR);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		retVal = handlePrepareAction(retval,magazine,slot,sFbn,container);
		if(retVal != SA_AIS_OK)
		{
			//get Dn struct
			std::string dn_struct("");
			IMM_Interface::getSwManagedActionReferenceStruct(p_objName,dn_struct);
			//check for board presence
			if(CCH_Util::isBoardPresent(magazine,slot))
			{
				this->setExitCode(FAILED_FBN_NOT_ALLOWED, IMM_Util::getErrorText(FAILED_FBN_NOT_ALLOWED));
				IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_PACKAGE_NOT_APPLICABLE);
			}
			else
			{
				this->setExitCode(FAILED_HARDWARE_NOT_PRESENT, IMM_Util::getErrorText(FAILED_HARDWARE_NOT_PRESENT));
				IMM_Interface::setManagedSwStructResultInfo(dn_struct, IMM_Util::ASW_HARDWARE_NOT_PRESENT);
			}

			//set values
			IMM_Interface::setManagedSwStructState(dn_struct, IMM_Util::FINISHED);
			IMM_Interface::setManagedSwStructResult(dn_struct, IMM_Util::FAILURE);
			IMM_Interface::setManagedSwStructTime(dn_struct);
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}
		else
		{
			this->adminOperationResult( oiHandle , invocation, retVal );
			return;
		}

	}


	//############## NO ACTION FOR THIS NUMBER

	if (operationId > 5)
	{
		retVal = SA_AIS_ERR_NO_OP;
		cout << " ..... NO ACTION FOR THIS NUMBER  \n "<< endl;
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw : NO ACTION FOR THIS NUMBER ",LOG_LEVEL_ERROR);
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;
	}

	cout << "---------------------------END ACTION-------------------------\n" << endl;

}

void FIXS_CCH_CpManagedSw::addObjectCcbId(ACS_APGCC_CcbId ccbId, std::string dn, CCH_Util::operationCallBackType opt, std::string container, int fbn )
{
	mapObjectCcb::iterator it = objectCcbId.find(dn);
	if (it == objectCcbId.end())
	{
		FIXS_CCH_CpManagedSw::CcbIdMapData data;
		data.m_dn = dn;
		data.m_ccbId = ccbId;
		data.m_opt = opt;
		data.m_fbn = fbn;
		data.m_container = container;

		objectCcbId.insert(mapObjectCcb::value_type(data.m_dn, data));
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " new Object CcbId: \n" << data.m_dn << "\n ccbid: " << data.m_ccbId << "\n opt: " << data.m_opt << "\n fbn: " << data.m_fbn << "\n container: " << data.m_container <<std::endl;
	}
}


void FIXS_CCH_CpManagedSw::removeObjectCcbId(std::string dn)
{
	mapObjectCcb::iterator it = objectCcbId.find(dn);
	if (it != objectCcbId.end())
	{
		std::cout << " ...RemoveObject dn: " << dn.c_str()<< std::endl;
		objectCcbId.erase(it);
	}

}

void FIXS_CCH_CpManagedSw::cleanCcbMap()
{
	for (mapObjectCcb::iterator it = objectCcbId.begin(); it != objectCcbId.end(); it++)
	{
		objectCcbId.erase(it);
	}
}

int FIXS_CCH_CpManagedSw::checkMagAndSlotParameters(ACS_APGCC_AdminOperationParamType **paramList, std::string &mag, std::string &slot)
{
	bool slotPar = false, magPar = false;

	for (int i = 0; paramList[i] != 0; i++)
	{
		if (strcmp((char *)paramList[i]->attrName,"slot") == 0 )
		{
			slot = reinterpret_cast<char *>(paramList[i]->attrValues);
			if (slot == "ALL")
				slotPar = false;
			else
				slotPar = true;
		}
		if (strcmp((char *)paramList[i]->attrName,"shelf") == 0 )
		{
			mag = reinterpret_cast<char *>(paramList[i]->attrValues);
			if(mag =="ALL")
				magPar = false;
			else
				magPar = true;
		}
	}

	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	if(magPar && slotPar)
	{
		if (CCH_Util::stringToUlongMagazine(mag,umagazine))
		{
			if (CCH_Util::stringToUshort(slot,uslot))
			{
				if (FIXS_CCH_UpgradeManager::getInstance()->isEpbBoard(umagazine, uslot) || FIXS_CCH_UpgradeManager::getInstance()->isEvoEtData(umagazine, uslot))
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,"FIXS_CCH_CpManagedSw - Action Parameters: magazine [%s] and slot [%s]",mag.c_str(), slot.c_str());
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

					return CCH_Util::PREPARE_MAG_SLOT;
				}
				else{
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw - Action Parameters: FBN not allowed",LOG_LEVEL_ERROR);
					return false;
				}
			}
			else{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw - Action Parameters: invalid slot",LOG_LEVEL_ERROR);
				return false;
			}
		}
		else{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_CpManagedSw - Action Parameters: invalid magazine",LOG_LEVEL_ERROR);
			return false;
		}

	}	  
	else if (magPar && !slotPar)
	{
		unsigned long umagazine = 0;
		if (CCH_Util::stringToUlongMagazine(mag,umagazine))
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,"FIXS_CCH_CpManagedSw - Action Parameters: magazine [%s]",mag.c_str());
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

			return CCH_Util::PREPARE_SHELF;

		}
		else
		{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ApManagedSw - Action Parameters: invalid magazine",LOG_LEVEL_ERROR);
			return false;
		}
	}
	else if(slotPar && !magPar)
	{
		return CCH_Util::PREPARE_SLOT;
	}	
	else if(!slotPar && !magPar)
	{
		return CCH_Util::PREPARE_ALL;
	}
	else
	{
		return false;
	}
}


SaAisErrorT FIXS_CCH_CpManagedSw::handlePrepareAction(int actionType, std::string magazine,std::string slot, std::string sFbn,std::string container)
{
	SaAisErrorT retVal;
	switch(actionType)
	{
	case 0:
	{
		retVal = SA_AIS_ERR_INVALID_PARAM;
		return retVal;
	}
	case 1:
	{
		if (strcmp(sFbn.c_str(),"EPB1") == 0)
		{
			if(!CpActionThread->isRunningThread())
				CpActionThread->open();

			CpActionThread->prepareEpb1(magazine,slot,container);

			retVal = SA_AIS_OK;
			return retVal;
		}
		else if (strcmp(sFbn.c_str(),"EVOET") == 0)
		{
			if(!CpActionThread->isRunningThread())
				CpActionThread->open();

			CpActionThread->prepareEvoEt(magazine,slot,container);

			retVal = SA_AIS_OK;
			return retVal;
		}
		else{
			retVal = SA_AIS_ERR_FAILED_OPERATION;
			return retVal;
		}
		break;
	}
	case 2:
	{
		if (strcmp(sFbn.c_str(),"EPB1") == 0)
		{
			if(!CpActionThread->isRunningThread())
				CpActionThread->open();

			CpActionThread->prepare_Shelf(magazine,container, CCH_Util::EPB1);

			retVal = SA_AIS_OK;
			return retVal;
		}
		else if (strcmp(sFbn.c_str(),"EVOET") == 0)
		{
			if(!CpActionThread->isRunningThread())
				CpActionThread->open();

			CpActionThread->prepare_Shelf(magazine,container, CCH_Util::EVOET);

			retVal = SA_AIS_OK;
			return retVal;
		}
		else{
			retVal = SA_AIS_ERR_FAILED_OPERATION;
			return retVal;
		}
		break;
	}
	case 3:
	{
		if (strcmp(sFbn.c_str(),"EPB1") == 0)
		{
			if(!CpActionThread->isRunningThread())
				CpActionThread->open();

			CpActionThread->prepare_all(container, CCH_Util::EPB1);

			retVal = SA_AIS_OK;
			return retVal;

		}
		else if (strcmp(sFbn.c_str(),"EVOET") == 0)
		{
			if(!CpActionThread->isRunningThread())
				CpActionThread->open();

			CpActionThread->prepare_all(container, CCH_Util::EVOET);

			retVal = SA_AIS_OK;
			return retVal;
		}
		else{
			retVal = SA_AIS_ERR_FAILED_OPERATION;
			return retVal;
		}
		break;
	}
	case 4:
	{
		retVal = SA_AIS_ERR_INVALID_PARAM;
		return retVal;

		break;
	}
	default:
		retVal = SA_AIS_OK;
		return retVal;
		break;
	}
}
void FIXS_CCH_CpManagedSw::setAdminOperationError(int errorCode,ACS_APGCC_OiHandle oiHandle,ACS_APGCC_InvocationType invocation)
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

