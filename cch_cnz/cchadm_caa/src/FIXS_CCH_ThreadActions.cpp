/*
 * FIXS_CCH_ThreadActions.cpp
 *
 *  Created on: Apr 4, 2012
 *      Author: eanform
 */


#include "FIXS_CCH_ThreadActions.h"

using namespace std;


/*============================================================================
	ROUTINE: FIXS_CCH_ThreadActions
 ============================================================================ */
FIXS_CCH_ThreadActions::FIXS_CCH_ThreadActions()
:running(0),installing(0),container(""),countAction(0)
{
	// Initialize the svc state flag
	tExit = false;

	m_shutdownEvent = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

//	m_stopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);

	m_CmdEvent = FIXS_CCH_Event::CreateEvent(false,false,0);

	_trace = new ACS_TRA_trace("FIXS_CCH_ThreadActions");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
FIXS_CCH_ThreadActions::~FIXS_CCH_ThreadActions()
{
	delete (_trace);

//	FIXS_CCH_Event::CloseEvent(m_stopEvent);
//	m_stopEvent = 0;

	FIXS_CCH_Event::CloseEvent(m_CmdEvent);
	m_CmdEvent = 0;

	FIXS_CCH_logging = 0;
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_CCH_ThreadActions::open(void *args)
{
	int result;

	UNUSED(args);

	tExit = false;

	result = activate();

  	return result;
}

/*============================================================================
	ROUTINE: stop
 ============================================================================ */
int FIXS_CCH_ThreadActions::stop()
{
	tExit=true;

//	if (m_stopEvent)
//	{
//		int retries=0;
//
//		while (FIXS_CCH_Event::SetEvent(m_stopEvent) == 0)
//		{
//			if( ++retries > 5) break;
//			sleep(1);
//		}
//	}

	return 0;
}

/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_CCH_ThreadActions::svc()
{
	int result = 0;

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions : Thread started... ",LOG_LEVEL_DEBUG);

	running = true;

//	EventHandle handleArr[3] = {m_shutdownEvent, m_CmdEvent, m_stopEvent};
	EventHandle handleArr[2] = {m_shutdownEvent, m_CmdEvent};

	while(!tExit)
	{

		int eventIndex = FIXS_CCH_Event::WaitForEvents(2,handleArr,20000);
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " eventIndex: "<< eventIndex << endl;
		if (eventIndex == 0)
		{
			tExit = true;
		}
		else if (eventIndex == FIXS_CCH_Event_RC_ERROR)
		{
			tExit = true;
		}
		else if (eventIndex == 1)
		{
			handleCommandEvent();
		}
	}

	running = false;

//	cleanActionMap();

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions : Thread stopped... ",LOG_LEVEL_DEBUG);

	return result;
}


void FIXS_CCH_ThreadActions::handleCommandEvent ()
{

	for (mapObjectAction::iterator itr = objectAction.begin(); itr != objectAction.end(); itr++)
	{
		FIXS_CCH_ThreadActions::ActionMapData data = itr->second;

		if (data.m_actionT == prepareT)
		{
			prepare_action(data.m_magazine,data.m_slot,data.m_container,data.m_upgradeType,data.m_force);
		}
		else if (data.m_actionT == activateT)
		{
			activate_action(data.m_magazine,data.m_slot);
		}
		else if (data.m_actionT == cancelT)
		{
			cancel_action(data.m_magazine,data.m_slot);
		}
		else if (data.m_actionT == commitT)
		{
			commit_action(data.m_magazine,data.m_slot);
		}
		else if (data.m_actionT == insertT)
		{
			insert_action(data.m_path,data.m_fbn, data.m_errorCode);
		}
		else if (data.m_actionT == defaultT)
		{
			default_action();
		}
		else if (data.m_actionT == prepareEpb1T)
		{
			prepareEpb1_action(data.m_magazine,data.m_slot,data.m_container);
		}
		else if (data.m_actionT == prepareMultiT)
		{
			prepare_Shelf_action(data.m_magazine,data.m_container, data.m_fbn);
		}
		else if (data.m_actionT == prepareAllT)
		{
			prepare_all_action(data.m_container, data.m_fbn);
		}
		else if (data.m_actionT == prepareEvoEtT)
		{
			prepareEvoEt_action(data.m_magazine,data.m_slot,data.m_container);
		}

		//remove from object ccbId map
		removeObjectAction(data.m_id);
	}

	//stop thread
//	stop();

}


int FIXS_CCH_ThreadActions::prepare(std::string sMag, std::string sSlot, std::string package, int upgradeType, bool force){
	addObjectAction(CCH_Util::prepareT,sMag,sSlot,package,"",0,-1,upgradeType,force);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}

int FIXS_CCH_ThreadActions::activateSw(std::string sMag, std::string sSlot){
	addObjectAction(CCH_Util::activateT,sMag,sSlot,container);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}

int FIXS_CCH_ThreadActions::cancel(std::string sMag, std::string sSlot){
	addObjectAction(CCH_Util::cancelT,sMag,sSlot,container);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}

int FIXS_CCH_ThreadActions::commit(std::string sMag, std::string sSlot){
	addObjectAction(CCH_Util::commitT,sMag,sSlot,container);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}

int FIXS_CCH_ThreadActions::insert_software(std::string pathSw, int fbn, int errorCode)
{
	addObjectAction(CCH_Util::insertT,"","","",pathSw,fbn,errorCode);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}

int FIXS_CCH_ThreadActions::change_default_package()
{
	addObjectAction(CCH_Util::defaultT);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}

int FIXS_CCH_ThreadActions::prepareEpb1(std::string sMag, std::string sSlot, std::string package){
	addObjectAction(CCH_Util::prepareEpb1T,sMag,sSlot,package);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}

int FIXS_CCH_ThreadActions::prepare_Shelf(std::string sMag, std::string package, int fbn){
	addObjectAction(CCH_Util::prepareMultiT,sMag,"",package,"",fbn);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}

int FIXS_CCH_ThreadActions::prepare_all(std::string package, int fbn){
	addObjectAction(CCH_Util::prepareAllT,"","",package,"",fbn);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}


int FIXS_CCH_ThreadActions::insert_action(std::string pathSw, int fbn, int errorCode)
{
	//return Code 1= OK, 0= NOK
	int resp = 0;

	installing = true;

	{ //trace
		char tmpChar[512] = {0};
		snprintf(tmpChar, sizeof(tmpChar) - 1,"FIXS_CCH_ThreadActions: installing software package with path:[%s] and fbn:[%d] ",pathSw.c_str(), fbn);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_INFO);
	}

	respCode respInstall = EXECUTED;
	int stateSwStruct = IMM_Util::NOT_AVAILABLE;
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;

	//in positive case
	if (errorCode < 0)
	{
		////// IMM ////////////////////////////
		//set state RUNNING
		stateSwStruct = IMM_Util::RUNNING;
		IMM_Interface::setSwMStructState(stateSwStruct);
		IMM_Interface::setSwMStructResult(resultSwStruct);
		IMM_Interface::setSwMStructResultInfo(resultInfoSwStruct);
		///////////////////////////////////////

		//check if no backup ongoing
		if((FIXS_CCH_UpgradeManager::getInstance()->isBackupOngoing()) == false)
		{

			//IPNA and CPUB import package handling
			if ((fbn == IMM_Util::IPNA) || (fbn == IMM_Util::CPUB))
			{

				acs_apgcc_adminoperation admin;
				ACS_CC_ReturnType result = ACS_CC_FAILURE;

				ACS_APGCC_ContinuationIdType continuation = 0;
				ACS_APGCC_AdminOperationIdType operationId = 1;

				//attribute list
				vector<ACS_APGCC_AdminOperationParamType> paramVector;
				ACS_APGCC_AdminOperationParamType intPar;
				ACS_APGCC_AdminOperationParamType stringPar;

				//fbn
				char attFbn[]= "fbn";
				intPar.attrName = attFbn;
				intPar.attrType = ATTR_INT32T;
				intPar.attrValues = reinterpret_cast<void*>(&fbn);

				//file name
				char attFile[]= "fileName";
				stringPar.attrName = attFile;
				stringPar.attrType = ATTR_STRINGT ;
				char* stringValue = const_cast<char *>(pathSw.c_str());
				stringPar.attrValues = reinterpret_cast<void*>(stringValue);

				paramVector.push_back(intPar);
				paramVector.push_back(stringPar);

				int ret;

				admin.init();
				result = admin.adminOperationInvoke(IMM_Util::DN_CPRELATEDSWMANAGER, continuation, operationId, paramVector, &ret, 0);
				if (result != ACS_CC_SUCCESS){
					std::cout <<"ERROR ::admOp.adminOperationInvoke()FAILED"<< std::endl;
					std::cout <<"ERROR CODE = "<< admin.getInternalLastError()<< "ERROR MESSAGE = " << admin.getInternalLastErrorText()<<std::endl;
				}
				resp = (SaAisErrorT) ret;
				admin.finalize();

				//if no answer from the implementer
				if (admin.getInternalLastError()==((-1)*SA_AIS_ERR_NOT_EXIST))
				{
					////// IMM ////////////////////////////
					//set state
					stateSwStruct = IMM_Util::FINISHED;
					IMM_Interface::setSwMStructState(stateSwStruct);
					//set time
					IMM_Interface::setSwMStructTime();

					respInstall = INTERNALERR;
					//set Result and ResultInfo in IMM model
					CCH_Util::getResultImportAction(respInstall, resultSwStruct, resultInfoSwStruct);
					IMM_Interface::setSwMStructResult(resultSwStruct);
					IMM_Interface::setSwMStructResultInfo(resultInfoSwStruct);
					///////////////////////////////////////

					installing = false;
				}

				return resp;
			}


			//for join dn with rdn
			char * dn_swRep=(char*)malloc(200);
			memset(dn_swRep,0,200);

			char * container=(char*)malloc(100);
			memset(container,0,100);

			FIXS_CCH_Installer installer(pathSw);
			if (installer.initialize())
			{
				resp = installer.install(fbn);
				respInstall = static_cast<respCode> (resp);
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "return : "<< resp << std::endl;
				if (resp == 0)
				{
					std::string path("");
					int isDefault = 0;
					int swVerType = 0;
					std::string installedProduct = installer.getInstalledProduct();
					std::string installedPackage = installer.getInstalledPackage();
					std::string fileNamePackage = installer.getFileNameInPackage();


					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "[INSTALLED PACKAGE] = " << installedPackage.c_str() << std::endl;
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "[INSTALLED PRODUCT] = " << installedProduct.c_str() << std::endl;

					CCH_Util::EnvironmentType environment;
					environment = FIXS_CCH_DiskHandler::getEnvironment();

					if ((installer.getInstallationType() == FIXS_CCH_Installer::SCX_INSTALLATION) || (installer.getInstallationType() == FIXS_CCH_Installer::CMX_INSTALLATION) || (installer.getInstallationType() == FIXS_CCH_Installer::SMX_INSTALLATION) )
					{
						//################################### -- create SCX object begin
						sprintf(container,"%s=%s",IMM_Util::ATT_APSWPACK_RDN,installedPackage.c_str());
						sprintf(dn_swRep,"%s,%s",container,IMM_Util::RDN_BLADESWM);
						//################################### --create SCX object end

						if (fbn == IMM_Util::SCXB)
						{
							path = FIXS_CCH_DiskHandler::getScxFolder();
							isDefault = FIXS_CCH_SoftwareMap::getInstance()->isScxEmpty();
						}
						else if (fbn == IMM_Util::CMXB)
						{
							path = FIXS_CCH_DiskHandler::getCmxFolder();
							isDefault = FIXS_CCH_SoftwareMap::getInstance()->isCmxEmpty();
							swVerType =  installer.getCmxVersionType();
						}
						else if (fbn == IMM_Util::SMXB)
						{
							path = FIXS_CCH_DiskHandler::getSmxFolder();
							isDefault = FIXS_CCH_SoftwareMap::getInstance()->isSmxEmpty();							
						}
						//path file
						std::string swPath = path + installedPackage + "/" + fileNamePackage;

						//check on environment type - m_environment
						if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA) ||(environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
						{
							if (FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(installedPackage))
							{
								IMM_Interface::setFaultyPackage(dn_swRep,0);
								FIXS_CCH_SoftwareMap::getInstance()->setNotFaultyContainer(installedPackage);
							}
							else
							{
								int validValues = false;

								if (fbn == IMM_Util::SCXB)
								{
									//allowed
									if (FIXS_CCH_SoftwareMap::getInstance()->addScxEntry(installedPackage, installedProduct, dn_swRep) == false)
									{
										{ //trace
											char tmpChar[512] = {0};
											snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Problem during installation process. addScxEntry() failed. uninstalling",__FUNCTION__, __LINE__);
											std::cout << "DBG: " << tmpChar << std::endl;
											if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
										}

										installer.uninstall();
										resp = 1;
									}
									else
									{
										char tmpChar[512] = {0};
										snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] SCX installation ok", __FUNCTION__, __LINE__);
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpChar << std::endl;
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);

										validValues = true;
									}
								}

								if (fbn == IMM_Util::CMXB)
								{
									//allowed
									if (FIXS_CCH_SoftwareMap::getInstance()->addCmxEntry(installedPackage, installedProduct, dn_swRep) == false)
									{
										{ //trace
											char tmpChar[512] = {0};
											snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Problem during installation process. addCmxEntry() failed. uninstalling",__FUNCTION__, __LINE__);
											std::cout << "DBG: " << tmpChar << std::endl;
											if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
										}

										installer.uninstall();
										resp = 1;
									}
									else
									{
										char tmpChar[512] = {0};
										snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] CMX installation ok", __FUNCTION__, __LINE__);
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpChar << std::endl;
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);

										validValues = true;
									}
								}	
								if (fbn == IMM_Util::SMXB)
								{
									//allowed
									if (FIXS_CCH_SoftwareMap::getInstance()->addSmxEntry(installedPackage, installedProduct, dn_swRep) == false)
									{
										{ //trace
											char tmpChar[512] = {0};
											snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Problem during installation process. addSmxEntry() failed. uninstalling",__FUNCTION__, __LINE__);
											std::cout << "DBG: " << tmpChar << std::endl;
											if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
										}

										installer.uninstall();
										resp = 1;
									}
									else
									{
										char tmpChar[512] = {0};
										snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] SMX installation ok", __FUNCTION__, __LINE__);
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpChar << std::endl;
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);

										validValues = true;
									}
								}								
								if (validValues)
								{
									std::cout<< "\n ...Before create ap sw swPath= " << swPath.c_str() << std::endl;
									//create IMM Object
									if (!IMM_Interface::createApSwObject(container,isDefault,fbn,swPath,installedProduct, swVerType)) resp = 1;
									else
									{
										std::string id("");
										if (!IMM_Interface::createSwManagedActionResultObject(dn_swRep,installedPackage,id)) resp = 1;
										else
											IMM_Interface::setSwManagedActionReferenceStruct(dn_swRep,id);
									}
								}

							}
						}
						else
						{
							//not allowed, uninstall
							{ //trace
								char tmpChar[512] = {0};
								snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Problem during installation process. Cannot add SCX Load Module in this configuration: uninstalling",__FUNCTION__, __LINE__);
								std::cout << "DBG: " << tmpChar << std::endl;
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
							}

							installer.uninstall();
							resp = 1;
						}
					}
					else if (installer.getInstallationType() == FIXS_CCH_Installer::IPT_INSTALLATION)
					{
						//################################### -- create IPT object begin
						sprintf(container,"%s=%s",IMM_Util::ATT_APSWPACK_RDN,installedPackage.c_str());
						sprintf(dn_swRep,"%s,%s",container,IMM_Util::RDN_BLADESWM);
						//################################### --create IPT object end

						path = FIXS_CCH_DiskHandler::getIptFolder();
						isDefault = FIXS_CCH_SoftwareMap::getInstance()->isIptEmpty();
						std::string swPath = path + installedPackage + "/" + fileNamePackage;

						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IPT_INSTALLATION" << std::endl;
						//check on environment type - m_environment
						if ((environment == CCH_Util::MULTIPLECP_NOTCBA) || (environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA) ||
						(environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
						{
							if (FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(installedPackage))
							{
								IMM_Interface::setFaultyPackage(dn_swRep,0);
								FIXS_CCH_SoftwareMap::getInstance()->setNotFaultyContainer(installedPackage);
							}
							else
							{
								bool isAllowed = true;
								int iptSize = FIXS_CCH_SoftwareMap::getInstance()->getIptSize();
								std::cout << "DBG: No. of IPT LMs present = " << iptSize << std::endl;
								if(iptSize >= 4)
								{
									if((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
									{
										isAllowed = false;
									}

								}
								if(!isAllowed)
								{
									std::cout << "DBG: Too many IPT packages. Installation allowed for upto 4 packages only in CBA environment." <<std::endl;
									//respInstall = TOO_MANY_LOAD_MODULES
									respInstall = MAXIMUM_LOAD_MODULES_INSTALLED;
									installer.uninstall();
									resp = 1;
								}
								else
								{
									//allowed installation
									std::cout << "DBG: Installation allowed." <<std::endl;
									if (FIXS_CCH_SoftwareMap::getInstance()->addIptEntry(installedPackage, installedProduct , dn_swRep) == false)
									{
										{
											char tmpChar[512] = {0};
											snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] Problem during installation process. addIptEntry() failed.", __FUNCTION__, __LINE__);
											std::cout << "DBG: " << tmpChar << std::endl;
											if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
										}

										installer.uninstall();
										resp = 1;

									}

									else
									{
										//trace
										char tmpChar[512] = {0};
										snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] IPT installation ok", __FUNCTION__, __LINE__);
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpChar << std::endl;
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);

										//create IMM Object
										if (!IMM_Interface::createApSwObject(container,isDefault,fbn,swPath,installedProduct)) resp = 1;
										else
										{
											std::string id("");
											if (!IMM_Interface::createSwManagedActionResultObject(dn_swRep,installedPackage,id)) resp = 1;
											else
												IMM_Interface::setSwManagedActionReferenceStruct(dn_swRep,id);
										}
									}
								}
							}
						}
						else
						{
							//not allowed, uninstall
							{ //trace
								char tmpChar[512] = {0};
								snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Problem during installation process. Cannot add IPT Load Module in current node architecture: uninstalling",__FUNCTION__, __LINE__);
								std::cout << "DBG: " << tmpChar << std::endl;
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
							}

							installer.uninstall();
							resp = 1;

						}
					}

					else if (installer.getInstallationType() == FIXS_CCH_Installer::IPLB_INSTALLATION)
					{
						//################################### -- create IPLB object begin
						sprintf(container,"%s=%s",IMM_Util::ATT_APSWPACK_RDN,installedPackage.c_str());
						sprintf(dn_swRep,"%s,%s",container,IMM_Util::RDN_BLADESWM);
						//################################### --create IPLB object end

						path = FIXS_CCH_DiskHandler::getIplbFolder();
						isDefault = FIXS_CCH_SoftwareMap::getInstance()->isIplbEmpty();
						std::string swPath = path + installedPackage + "/" + fileNamePackage;

						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IPLB_INSTALLATION" << std::endl;
						//check on environment type - m_environment
						if ((environment == CCH_Util::MULTIPLECP_DMX))
						{
							if (FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(installedPackage))
							{
								IMM_Interface::setFaultyPackage(dn_swRep,0);
								FIXS_CCH_SoftwareMap::getInstance()->setNotFaultyContainer(installedPackage);
							}
							else
                                                        {
                                                                bool isAllowed = true;
                                                                int iplbSize = FIXS_CCH_SoftwareMap::getInstance()->getIplbSize();
                                                                std::cout << "DBG: No. of IPLB LMs present = " << iplbSize << std::endl;
                                                                if(iplbSize >= 6)
                                                                {
                                                                   isAllowed = false;
                                                                }
                                                                if(!isAllowed)
                                                                {
                                                                        std::cout << "DBG: Too many IPLB packages. Installation allowed for upto 4 packages only in CBA environment." <<std::endl;
                                                                        //respInstall = TOO_MANY_LOAD_MODULES
                                                                        respInstall = MAXIMUM_LOAD_MODULES_INSTALLED;
                                                                        installer.uninstall();
                                                                        resp = 1;
                                                                }
							else
							{

								//allowed installation
								if (FIXS_CCH_SoftwareMap::getInstance()->addIplbEntry(installedPackage, installedProduct , dn_swRep) == false)
								{
									{
										char tmpChar[512] = {0};
										snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] Problem during installation process. addIplbEntry() failed.", __FUNCTION__, __LINE__);
										std::cout << "DBG: " << tmpChar << std::endl;
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
									}

									installer.uninstall();
									resp = 1;

								}
								else
								{
									//trace
									char tmpChar[512] = {0};
									snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] IPLB installation ok", __FUNCTION__, __LINE__);
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpChar << std::endl;
									if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);

									//create IMM Object
									if (!IMM_Interface::createApSwObject(container,isDefault,fbn,swPath,installedProduct)) resp = 1;
									else
									{
										std::string id("");
										if (!IMM_Interface::createSwManagedActionResultObject(dn_swRep,installedPackage,id)) resp = 1;
										else
											IMM_Interface::setSwManagedActionReferenceStruct(dn_swRep,id);
									}
								}
							}
						}
					}
						else
						{
							//not allowed, uninstall
							{ //trace
								char tmpChar[512] = {0};
								snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Problem during installation process. Cannot add IPLB Load Module in current node architecture: uninstalling",__FUNCTION__, __LINE__);
								std::cout << "DBG: " << tmpChar << std::endl;
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
							}

							installer.uninstall();
							resp = 1;

						}
					}
					else if ((installer.getInstallationType() == FIXS_CCH_Installer::EPB1_INSTALLATION) || (installer.getInstallationType() == FIXS_CCH_Installer::EVOET_INSTALLATION))
					{
						//################################### -- create EPB1 object begin
						sprintf(container,"%s=%s",IMM_Util::ATT_CPSWPACK_RDN,installedPackage.c_str());
						sprintf(dn_swRep,"%s,%s",container,IMM_Util::RDN_BLADESWM);
						//################################### --create EPB1 object end

						if (fbn == IMM_Util::EPB1)
						{
							path = FIXS_CCH_DiskHandler::getEpb1Folder();
							isDefault = FIXS_CCH_SoftwareMap::getInstance()->isEpb1Empty();
						}
						else if (fbn == IMM_Util::EVOET)
						{
							path = FIXS_CCH_DiskHandler::getEvoEtFolder();
							isDefault = FIXS_CCH_SoftwareMap::getInstance()->isEvoEtEmpty();
						}

						//path
						std::string swPath = path + installedPackage + "/" + fileNamePackage;

						//check on environment type - m_environment
						if ((environment == CCH_Util::MULTIPLECP_NOTCBA) || (environment == CCH_Util::MULTIPLECP_CBA)
								|| (environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::SINGLECP_NOTCBA)
								||(environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
						{
							if (FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(installedPackage))
							{
								IMM_Interface::setFaultyPackage(dn_swRep,0);
								FIXS_CCH_SoftwareMap::getInstance()->setNotFaultyContainer(installedPackage);
							}
							else
							{
								int validValues = false;

								if (fbn == IMM_Util::EPB1)
								{
									//allowed
									if (FIXS_CCH_SoftwareMap::getInstance()->addEpb1Entry(installedPackage, installedProduct, dn_swRep) == false)
									{
										{ // trace - error
											char tmpChar[512] = {0};
											snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] Problem during installation process. addEpb1Entry() failed.", __FUNCTION__, __LINE__);
											std::cout << "DBG: " << tmpChar << std::endl;
											if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
										}

										installer.uninstall();
										resp = 1;
									}
									else
									{
										//trace - ok
										char tmpChar[512] = {0};
										snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] EPB1 installation ok", __FUNCTION__, __LINE__);
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpChar << std::endl;
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);

										validValues = true;
									}
								}
								else if (fbn == IMM_Util::EVOET)
								{
									//allowed
									if (FIXS_CCH_SoftwareMap::getInstance()->addEvoEtEntry(installedPackage, installedProduct, dn_swRep) == false)
									{
										{ // trace - error
											char tmpChar[512] = {0};
											snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] Problem during installation process. addEvoEtEntry() failed.", __FUNCTION__, __LINE__);
											std::cout << "DBG: " << tmpChar << std::endl;
											if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
										}

										installer.uninstall();
										resp = 1;
									}
									else
									{
										//trace - ok
										char tmpChar[512] = {0};
										snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] EVOET installation ok", __FUNCTION__, __LINE__);
										std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpChar << std::endl;
										if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);

										validValues = true;
									}
								}

								if (validValues)
								{
									//create IMM Object
									if (!IMM_Interface::createCpSwObject(container,isDefault,fbn,swPath,installedProduct)) resp = 1;
									else
									{
										std::string id("");
										if (!IMM_Interface::createSwManagedActionResultObject(dn_swRep,installedPackage,id)) resp = 1;
										else
											IMM_Interface::setSwManagedActionReferenceStruct(dn_swRep,id);
									}
								}
							}
						}
						else
						{
							//not allowed, uninstall
							{ //trace
								char tmpChar[512] = {0};
								snprintf(tmpChar, sizeof(tmpChar) - 1,"[%s@%d] Problem during installation process. Cannot add EPB1 Load Module in current node architecture:. uninstalling",__FUNCTION__, __LINE__);
								std::cout << "DBG: " << tmpChar << std::endl;
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
							}

							installer.uninstall();
							resp = 1;
						}
					}
					else
					{
						char tmpChar[512] = {0};
						snprintf(tmpChar, sizeof(tmpChar) - 1, "[%s@%d] Problem during installation process.Install failed", __FUNCTION__, __LINE__);
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpChar << std::endl;
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
					}


				}//if resp

				else
				{
					std::cout << __FUNCTION__ << __LINE__ << "Installation software package FAILED !!! \n"<< std::endl;
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: Installation software package FAILED !!!",LOG_LEVEL_ERROR);
					resp = 1;
				}


			}//if (installer.initialize())
			else
			{
				std::cout << __FUNCTION__ << __LINE__ << "Initiation of installation software package FAILED !!! \n"<< std::endl;
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: Initiation of installation software package FAILED !!! ",LOG_LEVEL_ERROR);
				respInstall = INTERNALERR;
				resp = 1;
			}

			//if something failed, delete object
			if (resp == 1)
			{


				//remove from map
				if (installer.getInstallationType() == FIXS_CCH_Installer::SCX_INSTALLATION) FIXS_CCH_SoftwareMap::getInstance()->removeScxEntry(container);
				else if (installer.getInstallationType() == FIXS_CCH_Installer::IPT_INSTALLATION) FIXS_CCH_SoftwareMap::getInstance()->removeIptEntry(container);
				else if (installer.getInstallationType() == FIXS_CCH_Installer::EPB1_INSTALLATION) FIXS_CCH_SoftwareMap::getInstance()->removeEpb1Entry(container);
				else if (installer.getInstallationType() == FIXS_CCH_Installer::CMX_INSTALLATION) FIXS_CCH_SoftwareMap::getInstance()->removeCmxEntry(container);
				else if (installer.getInstallationType() == FIXS_CCH_Installer::EVOET_INSTALLATION) FIXS_CCH_SoftwareMap::getInstance()->removeEvoEtEntry(container);
				else if (installer.getInstallationType() == FIXS_CCH_Installer::IPLB_INSTALLATION) FIXS_CCH_SoftwareMap::getInstance()->removeIplbEntry(container);
				else if (installer.getInstallationType() == FIXS_CCH_Installer::SMX_INSTALLATION) FIXS_CCH_SoftwareMap::getInstance()->removeSmxEntry(container);
				//remove from disk
				installer.uninstall();

				//remove from IMM
				IMM_Util::deleteImmObject(dn_swRep);
			}

			free(container);
			free(dn_swRep);

		}
		else
		{
			cout << " ..... NOT ALLOWED, BACKUP ONGOING  \n "<< endl;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladeSwManagement : NOT ALLOWED, BACKUP ONGOING ",LOG_LEVEL_ERROR);
			respInstall = NOT_ALLOWED_BACKUP_ONGOING;
		}
	}
	else
	{
		//in negative case
		respInstall = static_cast<respCode> (errorCode);
	}
	////// IMM ////////////////////////////
	//set state
	stateSwStruct = IMM_Util::FINISHED;
	IMM_Interface::setSwMStructState(stateSwStruct);
	//set time
	IMM_Interface::setSwMStructTime();

	//set Result and ResultInfo in IMM model
	CCH_Util::getResultImportAction(respInstall, resultSwStruct, resultInfoSwStruct);
	IMM_Interface::setSwMStructResult(resultSwStruct);
	IMM_Interface::setSwMStructResultInfo(resultInfoSwStruct);
	///////////////////////////////////////

	installing = false;


	return resp;

}


int FIXS_CCH_ThreadActions::default_action()
{
	int res = EXECUTED;

	//check if no backup ongoing
	if((FIXS_CCH_UpgradeManager::getInstance()->isBackupOngoing()) == false)
	{
		if (!FIXS_CCH_SoftwareMap::getInstance()->isDefault(container))
		{
			std::string sFbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(container);

			cout << "\n sFbn: "<< sFbn.c_str()<< endl;

			//get old dn
			std::string default_container("");
			std::string dn_old_container("");
			std::string dn_new_container("");

			if (strcmp(sFbn.c_str(),"SCXB") == 0)	default_container = FIXS_CCH_SoftwareMap::getInstance()->getScxDefaultPackage();
			if (strcmp(sFbn.c_str(),"EPB1") == 0)	default_container = FIXS_CCH_SoftwareMap::getInstance()->getEpb1DefaultPackage();
			if (strcmp(sFbn.c_str(),"IPTB") == 0)	default_container = FIXS_CCH_SoftwareMap::getInstance()->getIptDefaultPackage();
			if (strcmp(sFbn.c_str(),"CMXB") == 0)	default_container = FIXS_CCH_SoftwareMap::getInstance()->getCmxDefaultPackage();
			if (strcmp(sFbn.c_str(),"EVOET") == 0)	default_container = FIXS_CCH_SoftwareMap::getInstance()->getEvoEtDefaultPackage();
			if (strcmp(sFbn.c_str(),"IPLB") == 0)	default_container = FIXS_CCH_SoftwareMap::getInstance()->getIplbDefaultPackage();
			if (strcmp(sFbn.c_str(),"SMXB") == 0)	default_container = FIXS_CCH_SoftwareMap::getInstance()->getSmxDefaultPackage();

			FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn_old_container,default_container);
			cout << " OLD ---> container: "<< default_container.c_str() << "\n dn: "<< dn_old_container.c_str() << endl;
			FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn_new_container,container);
			cout << " NEW ---> container: "<< container.c_str() << "\n dn: "<< dn_new_container.c_str() << endl;

			//set new default package
			if (IMM_Interface::setDefaultPackage(dn_new_container,1))
			{

			}

			//set old default package
			if (IMM_Interface::setDefaultPackage(dn_old_container,0))
			{

			}
		}
		else
		{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("Container is already Default !!!",LOG_LEVEL_DEBUG);
		}
	}
	else
	{
		cout << " ..... NOT ALLOWED, BACKUP ONGOING  \n "<< endl;
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_BladeSwManagement : NOT ALLOWED, BACKUP ONGOING ",LOG_LEVEL_ERROR);
		res = NOT_ALLOWED_BACKUP_ONGOING;
	}

	//set struct value
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;
	CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
	setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);

	return res;
}

int FIXS_CCH_ThreadActions::prepare_action(std::string sMag, std::string sSlot, std::string cxp, int upgradeType, bool force)
{
	int res = 0;

	if (strcmp(container.c_str(),"") != 0)
	{
		//set struct value
		setManagedSwPackageStrunct(IMM_Util::RUNNING,IMM_Util::NOT_ACTION_AVAILABLE,IMM_Util::EXECUTED);

		//check and create report progress object
		res = IMM_Interface::checkReportProgressObject(sMag,sSlot,cxp,CCH_Util::prepareT);

		//if the object exists or it was created successful
		if (res == EXECUTED)
		{
			std::string dn_obj("");
			unsigned long umagazine = 0;
			unsigned short uslot = 0;

			//convert mag and slot
			CCH_Util::stringToUlongMagazine(sMag,umagazine);
			CCH_Util::stringToUshort(sSlot,uslot);

			//get product
			std::string product = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(cxp);

			//perform prepare
			res = FIXS_CCH_UpgradeManager::getInstance()->prepare(umagazine, uslot, product, upgradeType, force);
			if (res == EXECUTED)
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: PREPARE ACTION COMPLETED !!! ",LOG_LEVEL_DEBUG);
			}
			else
			{
				if (IMM_Interface::getReportProgressObject(sMag,sSlot,dn_obj))
				{
					std::string dateSw("");
					std::string timeSw("");
					int resultSw = CCH_Util::SW_RP_NOT_OK;
					int reasonSw = CCH_Util::SW_RP_PREPARE_FAILURE;
					CCH_Util::getCurrentTime(dateSw, timeSw);
					CCH_Util::trim(timeSw);

					//result
					ACS_CC_ImmParameter SWresult = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_RESULT,ATTR_INT32T,&resultSw,1);
					IMM_Util::modify_OM_ImmAttr(dn_obj.c_str(),SWresult);

					//reason
					ACS_CC_ImmParameter SWreason = IMM_Util::defineParameterInt(IMM_Util::ATT_SWINVENTORY_REASON,ATTR_INT32T,&reasonSw,1);
					IMM_Util::modify_OM_ImmAttr(dn_obj.c_str(),SWreason);

					//time
//					std::string lTime = dateSw + " " + timeSw;
//					ACS_CC_ImmParameter SWtime = IMM_Util::defineParameterString(IMM_Util::ATT_SWINVENTORY_TIME,ATTR_STRINGT,lTime.c_str(),1);
//					IMM_Util::modify_OM_ImmAttr(dn_obj.c_str(),SWtime);

					delete [] SWresult.attrValues;
					delete [] SWreason.attrValues;
//					delete [] SWtime.attrValues;
				}
			}
		}


		//set struct value
		int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
		int resultInfoSwStruct = IMM_Util::EXECUTED;
		CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
		setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);
	}
	return res;
}

bool FIXS_CCH_ThreadActions::setManagedSwPackageStrunct(int state, int result, int resultInfo)
{
	bool res = true;

	std::string dn_parent("");
	FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn_parent, container);

	//get Dn struct
	std::string dn_struct("");
	IMM_Interface::getSwManagedActionReferenceStruct(dn_parent,dn_struct);

	if (state>=0)
		IMM_Interface::setManagedSwStructState(dn_struct, state);

	if (result>=0)
		IMM_Interface::setManagedSwStructResult(dn_struct, result);

	if (resultInfo>=0)
		IMM_Interface::setManagedSwStructResultInfo(dn_struct, resultInfo);

	IMM_Interface::setManagedSwStructTime(dn_struct);

	return res;
}

int FIXS_CCH_ThreadActions::activate_action(std::string sMag, std::string sSlot)
{
	int res = 0;

	//set struct value
	setManagedSwPackageStrunct(IMM_Util::RUNNING,IMM_Util::NOT_ACTION_AVAILABLE,IMM_Util::EXECUTED);

	//check and create report progress object
	res = IMM_Interface::checkReportProgressObject(sMag,sSlot,container, CCH_Util::activateT);
	if (res == EXECUTED)
	{
		unsigned long umagazine = 0;
		unsigned short uslot = 0;

		//convert mag and slot
		CCH_Util::stringToUlongMagazine(sMag,umagazine);
		CCH_Util::stringToUshort(sSlot,uslot);

		//perform prepare
		res = FIXS_CCH_UpgradeManager::getInstance()->activate(umagazine, uslot);

		std::cout << __FUNCTION__ << "@" << __LINE__ << " +++ACTIVATE result: "<< res <<std::endl;
		if (res == EXECUTED)
		{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: ACTIVATE ACTION COMPLETED !!! ",LOG_LEVEL_DEBUG);
		}

	}
	//set struct value
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;
	CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
	setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);

	return res;
}

int FIXS_CCH_ThreadActions::cancel_action(std::string sMag, std::string sSlot)
{
	int res = 0;

	//set struct value
	setManagedSwPackageStrunct(IMM_Util::RUNNING,IMM_Util::NOT_ACTION_AVAILABLE,IMM_Util::EXECUTED);

	//check and create report progress object
	res = IMM_Interface::checkReportProgressObject(sMag,sSlot,container, CCH_Util::cancelT);
	if (res == EXECUTED)
	{
		unsigned long umagazine = 0;
		unsigned short uslot = 0;

		//convert mag and slot
		CCH_Util::stringToUlongMagazine(sMag,umagazine);
		CCH_Util::stringToUshort(sSlot,uslot);

		//perform prepare
		res = FIXS_CCH_UpgradeManager::getInstance()->cancel(umagazine, uslot);
		if (res == EXECUTED)
		{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: CANCEL ACTION COMPLETED !!! ",LOG_LEVEL_DEBUG);
		}

	}
	//set struct value
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;
	CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
	setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);

	return res;
}

int FIXS_CCH_ThreadActions::commit_action(std::string sMag, std::string sSlot)
{
	int res = 0;

	//set struct value
	setManagedSwPackageStrunct(IMM_Util::RUNNING,IMM_Util::NOT_ACTION_AVAILABLE,IMM_Util::EXECUTED);

	//check and create report progress object
	res = IMM_Interface::checkReportProgressObject(sMag,sSlot,container, CCH_Util::commitT);
	if (res == EXECUTED)
	{
		unsigned long umagazine = 0;
		unsigned short uslot = 0;

		//convert mag and slot
		CCH_Util::stringToUlongMagazine(sMag,umagazine);
		CCH_Util::stringToUshort(sSlot,uslot);

		//perform prepare
		res = FIXS_CCH_UpgradeManager::getInstance()->commit(umagazine, uslot);
		if (res == EXECUTED)
		{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: COMMIT ACTION COMPLETED !!! ",LOG_LEVEL_DEBUG);
		}

	}
	//set struct value
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;
	CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
	setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);


	return res;
}



void FIXS_CCH_ThreadActions::addObjectAction(CCH_Util::upgradeActionType actType, std::string sMag, std::string sSlot, std::string package, std::string path, int fbn, int errorCode, int upgradeType, bool force)
{

	FIXS_CCH_ThreadActions::ActionMapData data;

	data.m_id = countAction++;
	data.m_magazine = sMag;
	data.m_slot = sSlot;
	data.m_actionT = actType;
	data.m_container = package;
	data.m_path = path;
	data.m_fbn = fbn;
	data.m_errorCode = errorCode;
	data.m_upgradeType = upgradeType;
	data.m_force  = force;

	objectAction.insert(mapObjectAction::value_type(data.m_id, data));
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " new Object Action: \n" << data.m_id
			<< "\n magazine: " << data.m_magazine.c_str() << "\n slot: " << data.m_slot.c_str()
			<< "\n container: " << data.m_container.c_str() << "\n path: " << data.m_path.c_str() << "\n fbn: " << data.m_fbn << "\n upgradeType" << data.m_upgradeType << "\n force" << data.m_force << std::endl;
}


void FIXS_CCH_ThreadActions::removeObjectAction(int id)
{
	mapObjectAction::iterator it = objectAction.find(id);
	if (it != objectAction.end())
	{
		std::cout << " ...RemoveObject id: " << id << std::endl;
		objectAction.erase(it);
	}

}

void FIXS_CCH_ThreadActions::cleanActionMap()
{
	for (mapObjectAction::iterator it = objectAction.begin(); it != objectAction.end(); it++)
	{
		objectAction.erase(it);
	}

	countAction = 0;
}

void FIXS_CCH_ThreadActions::prepareEpb1_action(std::string sMag, std::string sSlot, std::string package)
{
	int res = 0;

	//set struct value
	setManagedSwPackageStrunct(IMM_Util::RUNNING,IMM_Util::NOT_ACTION_AVAILABLE,IMM_Util::EXECUTED);

	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	//convert mag
	CCH_Util::stringToUlongMagazine(sMag,umagazine);
	CCH_Util::stringToUshort(sSlot,uslot);

	res = FIXS_CCH_UpgradeManager::getInstance()->prepareEpb1(umagazine,uslot,package,CCH_Util::SINGLE_BOARD);
	if (res == EXECUTED)
	{
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: PREPARE EPB1 ACTION COMPLETED !!! ",LOG_LEVEL_DEBUG);
	}

	//set struct value
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;
	CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
	setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);
}

void FIXS_CCH_ThreadActions::prepare_Shelf_action(std::string sMag, std::string package, int fbn)
{
	int res = 0;

	//set struct value
	setManagedSwPackageStrunct(IMM_Util::RUNNING,IMM_Util::NOT_ACTION_AVAILABLE,IMM_Util::EXECUTED);

	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	//convert mag
	CCH_Util::stringToUlongMagazine(sMag,umagazine);

	if (fbn == CCH_Util::EPB1)
	{
		res = FIXS_CCH_UpgradeManager::getInstance()->prepareEpb1(umagazine,uslot,package,CCH_Util::MULTIPLE_BOARD);
	}

	if (fbn == CCH_Util::EVOET)
	{
		res = FIXS_CCH_UpgradeManager::getInstance()->prepareEvoEt(umagazine,uslot,package,CCH_Util::MULTIPLE_BOARD);
	}

	if (res == EXECUTED)
	{
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: PREPARE (Shelf) ACTION COMPLETED !!! ",LOG_LEVEL_DEBUG);
	}

	//set struct value
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;
	CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
	setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);

}

void FIXS_CCH_ThreadActions::prepare_all_action(std::string package, int fbn)
{
	int res = 0;

	//set struct value
	setManagedSwPackageStrunct(IMM_Util::RUNNING,IMM_Util::NOT_ACTION_AVAILABLE,IMM_Util::EXECUTED);

	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	if (fbn == CCH_Util::EPB1)
	{
		res = FIXS_CCH_UpgradeManager::getInstance()->prepareEpb1(umagazine,uslot,package,CCH_Util::ALL_MAGAZINE_BOARD);
	}

	if (fbn == CCH_Util::EVOET)
	{
		res = FIXS_CCH_UpgradeManager::getInstance()->prepareEvoEt(umagazine,uslot,package,CCH_Util::ALL_MAGAZINE_BOARD);
	}

	if (res == EXECUTED)
	{
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions:  PREPARE (All) ACTION COMPLETED !!! ",LOG_LEVEL_DEBUG);
	}

	//set struct value
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;
	CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
	setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);
}

bool FIXS_CCH_ThreadActions::isInstallationOngoing()
{
	bool res = false;

		int result = IMM_Util::NOT_ACTION_AVAILABLE;
		IMM_Interface::getSwMStructState(result);
		std::cout << "DBG: " << __FUNCTION__<< " State:"<< result << std::endl;
		if (result == IMM_Util::RUNNING) res = true;

	return res;
}

int FIXS_CCH_ThreadActions::prepareEvoEt(std::string sMag, std::string sSlot, std::string package){
	addObjectAction(CCH_Util::prepareEvoEtT,sMag,sSlot,package);
	FIXS_CCH_Event::SetEvent(m_CmdEvent);
	return 0;
}


void FIXS_CCH_ThreadActions::prepareEvoEt_action(std::string sMag, std::string sSlot, std::string package)
{
	int res = 0;

	//set struct value
	setManagedSwPackageStrunct(IMM_Util::RUNNING,IMM_Util::NOT_ACTION_AVAILABLE,IMM_Util::EXECUTED);

	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	//convert mag
	CCH_Util::stringToUlongMagazine(sMag,umagazine);
	CCH_Util::stringToUshort(sSlot,uslot);

	res = FIXS_CCH_UpgradeManager::getInstance()->prepareEvoEt(umagazine,uslot,package, CCH_Util::SINGLE_BOARD);
	if (res == EXECUTED)
	{
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_ThreadActions: PREPARE EVOET ACTION COMPLETED !!! ",LOG_LEVEL_DEBUG);
	}

	//set struct value
	int resultSwStruct = IMM_Util::NOT_ACTION_AVAILABLE;
	int resultInfoSwStruct = IMM_Util::EXECUTED;
	CCH_Util::getResultUpgradeAction(res, resultSwStruct, resultInfoSwStruct);
	setManagedSwPackageStrunct(IMM_Util::FINISHED,resultSwStruct,resultInfoSwStruct);
}
