/*
 * FIXS_CCH_BladeSwManagement.cpp
 *
 *  Created on: Apr 4, 2012
 *      Author: eanform
 */

#include "FIXS_CCH_BladeSwManagement.h"

/*============================================================================
 ROUTINE: FIXS_CCH_BladeSwManagement
 ============================================================================ */
FIXS_CCH_BladeSwManagement::FIXS_CCH_BladeSwManagement() :
	acs_apgcc_objectimplementerinterface_V3(IMM_Util::IMPLEMENTER_BLADESWM),
			m_ImmClassName(IMM_Util::classBladeSwManagement)
{
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	IMM_Interface::createActionResultStruct();

	IMM_Interface::setBladeSwmReferenceStruct();
	importSw = new FIXS_CCH_ThreadActions();
}

/*============================================================================
 ROUTINE: ~FIXS_CCH_BladeSwManagement
 ============================================================================ */
FIXS_CCH_BladeSwManagement::~FIXS_CCH_BladeSwManagement()
{
	if (importSw)
	{
		delete (importSw);
		importSw = NULL;
	}

	FIXS_CCH_logging = 0;
}

ACS_CC_ReturnType FIXS_CCH_BladeSwManagement::create(
		ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId,
		const char *className, const char* parentname,
		ACS_APGCC_AttrValues **attr)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback invocated           " << endl;
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

ACS_CC_ReturnType FIXS_CCH_BladeSwManagement::deleted(
		ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
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

ACS_CC_ReturnType FIXS_CCH_BladeSwManagement::modify(
		ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId,
		const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectModifyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	int i = 0;
	while (attrMods[i])
	{
		cout << " attrName: " << attrMods[i]->modAttr.attrName << endl;
		i++;
	}

	cout << "MODIFYCallback " << endl;
	cout << endl;

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}

ACS_CC_ReturnType FIXS_CCH_BladeSwManagement::complete(
		ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invocated            " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle       : " << oiHandle << endl;
	cout << " ccbId            : " << ccbId << endl;

	cout << " COMPLETECallback " << endl;
	cout << endl;

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;

	return result;
}

void FIXS_CCH_BladeSwManagement::abort(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "            CcbAbortCallback invocated             " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId     : " << ccbId << endl;

	cout << "ABORTCallback " << endl;
	cout << endl;
}

void FIXS_CCH_BladeSwManagement::apply(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "            CcbApplyCallback invocated             " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << "APPLYCallback " << endl;
	cout << endl;

}

ACS_CC_ReturnType FIXS_CCH_BladeSwManagement::updateRuntime(
		const char* p_objName, const char** p_attrName)
{
	cout << "------------------------------------------------------------"
			<< endl;
	cout << "                   updateRuntime called                     "
			<< endl;
	cout << "------------------------------------------------------------"
			<< endl;

	cout << "Object Name   : " << p_objName << endl;
	cout << "Attribute Name: " << p_attrName << endl;

	cout << "------------------------------------------------------------"
			<< endl;

	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	return result;
}

void FIXS_CCH_BladeSwManagement::adminOperationCallback(
		ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation,
		const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,
		ACS_APGCC_AdminOperationParamType**paramList)
{

	cout << "------------------------------------------------------------"
			<< endl;
	cout << "                   adminOperationCallback called            "
			<< endl;
	cout << "------------------------------------------------------------"
			<< endl;

	cout << "oiHandle   : " << oiHandle << endl;
	cout << "invocation : " << invocation << endl;
	cout << "p_objName  : " << p_objName << endl;
	cout << "operationId: " << operationId << endl;

	cout << " Implementing Admin Operation with ID = " << operationId << endl;

	switch (operationId)
	{
	case IMM_Util::IMPORT_PACKAGE:
		cout << "------------ IMPORT PACKAGE -------------" << endl;
		importSwPackage(oiHandle, invocation, paramList);
		break;
	default:
		//retVal = SA_AIS_ERR_NO_OP;
		cout << " ..... INVALID ACTION: " << operationId << endl;
		if (FIXS_CCH_logging)
			FIXS_CCH_logging->Write(
					"FIXS_CCH_BladeSwManagement: NO ACTION FOR THIS NUMBER ",
					LOG_LEVEL_ERROR);
		this->adminOperationResult(oiHandle, invocation, SA_AIS_ERR_NO_OP);
		break;
	}

	cout << "---------------------------END ACTION-------------------------"
			<< endl;
}

void FIXS_CCH_BladeSwManagement::importSwPackage(ACS_APGCC_OiHandle oiHandle,
		ACS_APGCC_InvocationType invocation,
		ACS_APGCC_AdminOperationParamType**paramList)
{
	if (FIXS_CCH_logging)
		FIXS_CCH_logging->Write("FIXS_CCH_BladeSwManagement : IMPORT PACKAGE ",
				LOG_LEVEL_DEBUG);

	std::string pathParameter("");
	int fbn = 0;
	int errorCode = 0;

	SaAisErrorT checkParRetVal = checkParams(paramList, fbn, pathParameter,
			errorCode);

	//start thread in case is not running
	if (!importSw->isRunningThread())
	{
		importSw->open();
	}

	if (checkParRetVal == SA_AIS_OK)
	{
		cout << " ..... checkParameters success !!!" << endl;

		if (!importSw->isInstallationOngoing())
		{
			importSw->insert_software(pathParameter, fbn);

			if (FIXS_CCH_logging)
				FIXS_CCH_logging->Write(
						"FIXS_CCH_BladeSwManagement : IMPORTING PACKAGE",
						LOG_LEVEL_DEBUG);
			this->adminOperationResult(oiHandle, invocation, checkParRetVal);
		}
		else
		{
			if (FIXS_CCH_logging)
				FIXS_CCH_logging->Write(
						"FIXS_CCH_BladeSwManagement : IMPORTING ACTION ALREADY RUNNING",
						LOG_LEVEL_DEBUG);
			//Return Error Message to IMM
			setAdminOperationError(FAILED_IMPORT_ACTION_ALREADY_RUNNING,
					oiHandle, invocation);
		}
	}
	else
	{
		cout << " ..... checkParameters fails errorCode: " << errorCode << endl;
		if (FIXS_CCH_logging)
			FIXS_CCH_logging->Write(
					"FIXS_CCH_BladeSwManagement : IMPORTING ACTION FAILED - CHECK PARAMETER",
					LOG_LEVEL_ERROR);
		cout << IMM_Util::getErrorText(errorCode);

		//Return Error Message to IMM
		setAdminOperationError(errorCode, oiHandle, invocation);
		importSw->insert_software("", -1, errorCode);
	}
}

SaAisErrorT FIXS_CCH_BladeSwManagement::checkParams(
		ACS_APGCC_AdminOperationParamType **paramList, int& fbn,
		std::string& path, int& error)
{
	SaAisErrorT result = SA_AIS_OK;
	std::string pathParameter("");
	CCH_Util::EnvironmentType env = FIXS_CCH_DiskHandler::getEnvironment();

	if (strcmp((char *) paramList[1]->attrName, "fbn") == 0)
	{
		fbn = *reinterpret_cast<int *> (paramList[1]->attrValues);

		cout << __FUNCTION__ << "fbn: " << fbn << endl;
		CCH_Util::APGtype m_APGnum = CCH_Util::getAPGtype();

		if (fbn == IMM_Util::SCXB)
		{
			if ((env == SINGLECP_CBA) || (env == MULTIPLECP_CBA))
			{
				pathParameter = FIXS_CCH_DiskHandler::getNbiSwPath(
						CCH_Util::swPackageScxb);
			}
			else
			{
				error = FAILED_INVALID_CONFIGURATION;
				return SA_AIS_ERR_NOT_SUPPORTED;
			}
		}
		else
			if (fbn == IMM_Util::EPB1)
			{

				if ((env == SINGLECP_CBA) || (env == MULTIPLECP_CBA) || (env == SINGLECP_SMX) || (env == MULTIPLECP_SMX))
				{
					pathParameter = FIXS_CCH_DiskHandler::getNbiSwPath(
							CCH_Util::swPackageEpb1);
				}
				else
				{
					error = FAILED_INVALID_CONFIGURATION;
					return SA_AIS_ERR_NOT_SUPPORTED;
				}
			}
			else
				if (fbn == IMM_Util::IPTB)
				{

					if ((env == SINGLECP_CBA) || (env == MULTIPLECP_CBA)
							|| (env == MULTIPLECP_NOTCBA) || (env == SINGLECP_SMX) || (env == MULTIPLECP_SMX))
					{
						pathParameter = FIXS_CCH_DiskHandler::getNbiSwPath(
								CCH_Util::swPackageIptb);

						if (m_APGnum == CCH_Util::AP2)
						{
							error = NOTALLOWED_AP2;
							return SA_AIS_ERR_NOT_SUPPORTED;
						}
					}
					else
					{
						error = FAILED_INVALID_CONFIGURATION;
						return SA_AIS_ERR_NOT_SUPPORTED;
					}
				}

				else
					if (fbn == IMM_Util::IPLB)
					{

						if (env == MULTIPLECP_DMX)
						{
							pathParameter = FIXS_CCH_DiskHandler::getNbiSwPath(
									CCH_Util::swPackageIplb);
							if (m_APGnum == CCH_Util::AP2)
							{
								error = NOTALLOWED_AP2;
								return SA_AIS_ERR_NOT_SUPPORTED;
							}
						}
						else
						{
							error = FAILED_INVALID_CONFIGURATION;
							return SA_AIS_ERR_NOT_SUPPORTED;
						}
					}
					else
						if (fbn == IMM_Util::IPNA)
						{
							if (env == SINGLECP_NOTCBA)
							{
								pathParameter
										= FIXS_CCH_DiskHandler::getNbiSwPath(
												CCH_Util::swPackageCp);
							}
							else
							{
								error = FAILED_INVALID_CONFIGURATION;
								return SA_AIS_ERR_NOT_SUPPORTED;
							}
						}
						else
							if (fbn == IMM_Util::CPUB)
							{
								pathParameter
										= FIXS_CCH_DiskHandler::getNbiSwPath(
												CCH_Util::swPackageCp);
							}
							else
								if (fbn == IMM_Util::CMXB)
								{

									if ((env == SINGLECP_CBA) || (env
											== MULTIPLECP_CBA))
									{
										pathParameter
												= FIXS_CCH_DiskHandler::getNbiSwPath(
														CCH_Util::swPackageCmxb);
									}
									else
									{
										error = FAILED_INVALID_CONFIGURATION;
										return SA_AIS_ERR_NOT_SUPPORTED;
									}
								}
								else
									if (fbn == IMM_Util::EVOET)
									{

										if ((env == SINGLECP_CBA) || (env
												== MULTIPLECP_CBA) || (env == SINGLECP_SMX) || (env == MULTIPLECP_SMX))
										{
											pathParameter
													= FIXS_CCH_DiskHandler::getNbiSwPath(
															CCH_Util::swPackageEvoEt);
										}
										else
										{
											error
													= FAILED_INVALID_CONFIGURATION;
											return SA_AIS_ERR_NOT_SUPPORTED;
										}
									}
									else if (fbn == IMM_Util::SMXB)
									{
										if ((env == SINGLECP_SMX) || (env == MULTIPLECP_SMX))										
										{
											pathParameter = FIXS_CCH_DiskHandler::getNbiSwPath(
											CCH_Util::swPackageSmxb);
										}
										else
										{
											error = FAILED_INVALID_CONFIGURATION;
											return SA_AIS_ERR_NOT_SUPPORTED;
										}									
									}
									else
										pathParameter
												= FIXS_CCH_DiskHandler::getNbiSwPath(
														CCH_Util::swPackage);

		//complete path
		pathParameter = pathParameter + "/"
				+ reinterpret_cast<char *> (paramList[0]->attrValues);
		std::cout << "path parameter is : " << pathParameter << endl;

		if (strcmp((char *) paramList[0]->attrName, "fileName") == 0)
		{
			cout << __FUNCTION__ << " " << __LINE__ << endl;
			if (CCH_Util::CheckExtention(pathParameter.c_str(),
					string(".zip").c_str()) || CCH_Util::CheckExtention(
					pathParameter.c_str(), string(".tar").c_str())
					|| CCH_Util::CheckExtention(pathParameter.c_str(), string(
							".tar.gz").c_str()))
			{
				cout << "FIXS_CCH_OI_SwRepository " << __LINE__
						<< " is ZIP or TAR or TAR.GZ" << endl;

				if (CCH_Util::PathFileExists(pathParameter.c_str()))
				{
					cout << __FUNCTION__ << "Check OK " << __LINE__ << endl;
					path = pathParameter;

					result = SA_AIS_OK;
				}
				else
				{
					cout << "PATH NOT EXIST " << endl;
					if (FIXS_CCH_logging)
						FIXS_CCH_logging->Write(
								"FIXS_CCH_BladeSwManagement : PATH NOT EXIST",
								LOG_LEVEL_ERROR);
					error = FAILED_PATH_NOT_EXIST;
					result = SA_AIS_ERR_NO_RESOURCES;
				}
			}
			else
			{
				cout << "WRONG EXTENSION " << endl;
				if (FIXS_CCH_logging)
					FIXS_CCH_logging->Write(
							"FIXS_CCH_BladeSwManagement : WRONG EXTENSION",
							LOG_LEVEL_ERROR);
				error = FAILED_WRONG_EXTENSION;
				result = SA_AIS_ERR_BAD_OPERATION;
			}
		}
		else
		{
			cout << "INVALID PARAMETER" << endl;
			if (FIXS_CCH_logging)
				FIXS_CCH_logging->Write(
						"FIXS_CCH_BladeSwManagement : INVALID PARAMETER",
						LOG_LEVEL_ERROR);
			error = FAILED_INVALID_FILENAME;
			result = SA_AIS_ERR_INVALID_PARAM;
		}
	}
	else
	{
		cout << "INVALID PARAMETER" << endl;
		if (FIXS_CCH_logging)
			FIXS_CCH_logging->Write(
					"FIXS_CCH_BladeSwManagement : INVALID PARAMETER",
					LOG_LEVEL_ERROR);
		error = FAILED_INVALID_FBN;
		result = SA_AIS_ERR_INVALID_PARAM;
	}

	return result;
}

void FIXS_CCH_BladeSwManagement::setAdminOperationError(int errorCode,
		ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation)
{

	cout << "FIXS_CCH_BladeSwManagement::setAdminOperationError IN" << endl;
	cout << __FUNCTION__ << "@" << __LINE__ << "errorCode: " << errorCode
			<< endl;

	std::vector<ACS_APGCC_AdminOperationParamType> resultOut;
	ACS_APGCC_AdminOperationParamType errorTextElem;
	char attrNameText[] = "errorText";

	errorTextElem.attrName = attrNameText;
	errorTextElem.attrType = ATTR_STRINGT;

	string errText("@ComNbi@");
	errText.append(IMM_Util::getErrorText(errorCode));
	errorTextElem.attrType = ATTR_STRINGT;
	char* errValue = const_cast<char*> (errText.c_str());
	errorTextElem.attrValues = reinterpret_cast<void*> (errValue);

	resultOut.push_back(errorTextElem);
	//setExitCode(errorCode,errText);

	ACS_CC_ReturnType rc = adminOperationResult(oiHandle, invocation, 0,
			resultOut);
	cout << __FUNCTION__ << "@" << __LINE__ << " Return Value: " << rc << endl;
	return;
}

