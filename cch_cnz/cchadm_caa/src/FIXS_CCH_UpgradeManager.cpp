
#include "FIXS_CCH_UpgradeManager.h"
#include "ACS_CS_API.h"
namespace
{
ACE_Thread_Mutex mtx;

const int MAX_RETRIES_ON_FSM_STOP = 12;
}

namespace {

struct Lock
{
	Lock()
	{
		std::cout << " ##################### FIXS_CCH_UpgradeManager: Lock region while exception active " <<std::endl;
		mtx.acquire();
	};

	~Lock()
	{
		mtx.release();
		std::cout << " ##################### FIXS_CCH_UpgradeManager: Leave region while exception active " <<std::endl;
	};
};
}

#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif


// Class FIXS_CCH_UpgradeManager::FsmListData 

FIXS_CCH_UpgradeManager::FsmListData::FsmListData (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ipA, unsigned long ipB, std::string container, std::string dn, CCH_Util::FBN fbn,  FIXS_CCH_StateMachine *fsm)
: m_boardId(boardId),
  m_magazine(magazine),
  m_slot(slot),
  m_swPtr(0),
  m_IP_EthA(ipA),
  m_IP_EthB(ipB),
  m_container(container),
  m_pfmPtr(0),
  m_ipmiPtr(0),
  m_fwPtr(0),
  m_lmPtr(0),
  m_fbn(fbn),
  m_dn(dn)
{
	fsm = NULL;
}

void FIXS_CCH_UpgradeManager::printError(std::string mess,const char *func,int line)
{
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] %s",func,line,mess.c_str());
	printf("%s\n",tmpStr);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
}


std::string FIXS_CCH_UpgradeManager::FsmListData::getIPEthAString ()
{
	std::string IPA("");
	IPA = CCH_Util::ulongToStringIP(m_IP_EthA);

	return IPA;
}

std::string FIXS_CCH_UpgradeManager::FsmListData::getIPEthBString ()
{
	std::string IPB("");
	IPB = CCH_Util::ulongToStringIP(m_IP_EthB);

	return IPB;
}

std::string FIXS_CCH_UpgradeManager::FsmListData::getIMPIMasterIP ()
{
	return m_ipMasterSCX;
}

// Additional Declarations

// Class FIXS_CCH_UpgradeManager 

FIXS_CCH_UpgradeManager* FIXS_CCH_UpgradeManager::m_instance = 0;
std::list<FIXS_CCH_UpgradeManager::FsmListData>* FIXS_CCH_UpgradeManager::m_iptUpgradeList = 0;
std::list<FIXS_CCH_UpgradeManager::FsmListData>* FIXS_CCH_UpgradeManager::m_swUpgradeList = 0;


FIXS_CCH_UpgradeManager::FIXS_CCH_UpgradeManager() : m_initialized(false),m_upgradeResult("-"),m_upgradeReason("-"),trapSubnet(0),
		trapLoadReleaseComplete(false),trapLoadReleaseIp(""),trapIpmiUpgrade(false),trapIpmiUpgradeIp(""),trapColdStartWarmStart(false),trapColdStartWarmStartIp(""),trapPfmUpgrade(false),trapPfmUpgradeIp(""), m_backup(false)
{
	m_swUpgradeList = new swUpgradeList_t;
	m_iptUpgradeList = new swUpgradeList_t;
	m_alarmHandler = new FIXS_CCH_AlarmHandler();
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	//m_trapReceiver = new FIXS_CCH_SNMPManager();
	_trace = new ACS_TRA_trace("CCH UpgradeManager");

	environment = CCH_Util::UNKNOWN;

	trapFwUpgrade = false;

	dmxc_address[0]=0;
	dmxc_address[1]=0;
}

FIXS_CCH_UpgradeManager::~FIXS_CCH_UpgradeManager()
{

	cleanStateMachine();

	if (_trace){
		delete(_trace);
		_trace = NULL;
	}

	if(m_iptUpgradeList)
		delete(m_iptUpgradeList);
	if(m_alarmHandler)
	{
		m_alarmHandler->stop();
		delete m_alarmHandler;
	}

	FIXS_CCH_logging = 0;
}

FIXS_CCH_UpgradeManager* FIXS_CCH_UpgradeManager::getInstance ()
{
	//Lock lock;
	if (m_instance == 0)
	{
		m_instance = new FIXS_CCH_UpgradeManager();
	}

	return m_instance;
}

void FIXS_CCH_UpgradeManager::finalize ()
{
	delete(m_instance);
	m_instance = NULL;
}

int FIXS_CCH_UpgradeManager::initialize ()
{
	//Lock lock;
	int retValue = 0;

	if (m_initialized == false)
	{
		if (fetchBoardInfo() == 0)
		{
			if (startSWUpgradeFSMs() == 0)
			{
				if (startPFMUpgradeFSMs() == 0)
				{
					if (startIpmiUpgradeFSMs() == 0)
					{
						if (startFwUpgradeFSMs() == 0)
						{
							if (startIPTUpgradeFSMs() == 0)
							{

								//start Alarm handler thread
								if(m_alarmHandler)
									m_alarmHandler->open();
								//OK
							}
							else
							{
								printError("Cannot start IPT upgrade State Machines",__FUNCTION__, __LINE__);
								//retValue = -6;
							}
						}
						//						/*	if (startIPTUpgradeFSMs() == 0)
					}
					else
					{
						printError("Cannot start IPMI upgrade State Machines",__FUNCTION__, __LINE__);

					}
				}
				else
				{
					printError("Cannot start PFM upgrade State Machines",__FUNCTION__, __LINE__);

				}

			}

			else
			{	std::cout<<__LINE__<<std::endl;
			printError("Cannot start SW upgrade State Machines",__FUNCTION__, __LINE__);
			//retValue = -5;
			}

		}
		else
		{	std::cout << __LINE__ << std::endl;
		printError("Cannot fetch board Info",__FUNCTION__, __LINE__);
		//retValue = -3;
		}

		m_initialized = true;

	}
	//IMM_Interface::checkBrmPersistentDataOwnerObject();
	environment = FIXS_CCH_DiskHandler::getEnvironment();

	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
	{
		checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);
	}
	else if ((environment == CCH_Util::SINGLECP_DMX) || (environment == CCH_Util::MULTIPLECP_DMX))
	{
		if (dmxc_address[0] == 0 || dmxc_address[1] == 0)
		{
			ACS_CS_API_NetworkElement::getDmxcAddress(dmxc_address[0], dmxc_address[1]);
		}

		std::cout << "DBG: " << "getDmxAddress" << "@" << __LINE__ << "dmxc_address[0]:" << dmxc_address[0]
		                                                                                                 <<"dmxc_address[1]:" << dmxc_address[1]<<std::endl;
		//char tmpStr[512] = {0};
		//snprintf(tmpStr, sizeof(tmpStr) - 1, "getDmxAddress [dmxc_address[0]:%d,dmxc_address[1]:%d] ",dmxc_address[0],dmxc_address[1]);
		//if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

		checkIPLBswVr();	
	}
	else if ((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
	{
		checkSMXswVr();
	}
	return retValue;
}

void FIXS_CCH_UpgradeManager::getDmxAddress(uint32_t &dmxadd1, uint32_t &dmxadd2)
{
	if (dmxc_address[0] == 0 || dmxc_address[1] == 0)
	{
		ACS_CS_API_NetworkElement::getDmxcAddress(dmxc_address[0], dmxc_address[1]);
	}

	dmxadd1 = dmxc_address[0];
	dmxadd2 = dmxc_address[1];
}
int FIXS_CCH_UpgradeManager::fetchBoardInfo()
{
	int retValue = 0;

	environment = FIXS_CCH_DiskHandler::getEnvironment();
	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
	{
		//clean used packages
		if (retValue == 0) FIXS_CCH_SoftwareMap::getInstance()->cleanAllUsedContainers();

		retValue = fetchSCXInfo();
		retValue = fetchEPBInfo();
		retValue = fetchCMXInfo();
		retValue = fetchEVOETInfo();

		retValue = fetchIptInfo();
	}
	else if (environment == CCH_Util::MULTIPLECP_DMX)
	{
		if (retValue == 0) FIXS_CCH_SoftwareMap::getInstance()->cleanAllUsedContainers();


		retValue = fetchIPLBInfo();

	}
	else if ((environment == CCH_Util::MULTIPLECP_NOTCBA) || (environment == CCH_Util::SINGLECP_NOTCBA))
	{
		//clean used packages
		if (retValue == 0) FIXS_CCH_SoftwareMap::getInstance()->cleanAllUsedContainers();

		retValue = fetchIptInfo();
	}
	else if ((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
	{
		//clean used packages
		if (retValue == 0) FIXS_CCH_SoftwareMap::getInstance()->cleanAllUsedContainers();

		retValue = fetchSMXInfo();
		retValue = fetchEPBInfo();
		retValue = fetchEVOETInfo();
		retValue = fetchIptInfo();// need to check if its supported
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ENVIRONMENT NOT SUPPORTED !!!" << std::endl;
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : fetchBoardInfo() - ENVIRONMENT NOT SUPPORTED !!! ",LOG_LEVEL_WARN);
		retValue = 1;
	}
	if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
		removeUnusedFaultySwObjects();
	return retValue;


}

int FIXS_CCH_UpgradeManager::fetchSCXInfo ()
{
	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SCXB);

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no SCX board found. Any SCX hasn't been configured yet.
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No SCX board found", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : fetchSCXInfo() - NO SCX Board Found !!! ",LOG_LEVEL_WARN);
					retValue = 0;
				}
				else
				{
					// got some SCX board
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						uint32_t magazine = 0;
						returnValue = hwc->getMagazine (magazine, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							// magazine read
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0%x", __LINE__, magazine);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}

							unsigned short slot = 0;
							returnValue = hwc->getSlot (slot, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								// slot read
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								if (isScxData(magazine, slot) == false)
								{
									std::string container("");
									std::string containerValue("");
									std::string dn_blade("");

									if(IMM_Interface::getDnBoard(magazine,slot,dn_blade))
									{
										returnValue = hwc->getContainerPackage (container, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
											//										if (IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container))
										{
											uint32_t IP_EthA = 0;
											returnValue = hwc->getIPEthA (IP_EthA, boardId);
											if (returnValue == ACS_CS_API_NS::Result_Success)
											{
												uint32_t IP_EthB = 0;
												returnValue = hwc->getIPEthB (IP_EthB, boardId);
												if (returnValue == ACS_CS_API_NS::Result_Success)
												{
													containerValue=container;
													if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
													{
														std::string IPA("");
														std::string IPB("");
														std::string cxc_no("");
														std::string mag("");
														IPA = CCH_Util::ulongToStringIP(IP_EthA);
														IPB = CCH_Util::ulongToStringIP(IP_EthB);
														if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("entering after restore..calling checkCurrentVersionInBoard function ",LOG_LEVEL_WARN);
														bool ret = checkCurrentVersionInBoard (IPA, IPB,cxc_no);
														if(ret)
														{
															ACS_CS_API_SET_NS::CS_API_Set_Result result;
															std::stringstream ss;
															ss << slot;
															std::string _slot = ss.str();
															CCH_Util::ulongToStringMagazine(magazine,mag);
															std::string mag_slot = mag + "_" + _slot;
															std::cout << "-------mag_slot= " << mag_slot.c_str() << std::endl;
															std::string currCxc = ACS_APGCC::after(cxc_no,"SCX_");
															std::string currContainer = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(currCxc);
															
															if(!currContainer.empty())
															{
																containerValue=currContainer;
																result = ACS_CS_API_Set::setSoftwareVersion(containerValue, mag, slot);
																bool retryVal=false;
																if(result!=ACS_CS_API_SET_NS::Result_Success)
																{
																	retryVal=retrySetSoftwareVersion(containerValue, mag, slot);
																}
																else
																	retryVal=true;
																if(retryVal==false) { 
																	containerValue=container;
																}
															}
														}
													}
													addScxEntry(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue,dn_blade);
													retValue = 0;
												}
												else
												{
													//serious fault in CS: No IP_EthB found for the SCXB board
													{
														char tmpStr[512] = {0};
														snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the SCXB board", __LINE__);
														if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
													}

													retValue = -7;
													break;
												}
											}
											else
											{
												//serious fault in CS: No IP_EthA found for the SCXB board
												{
													char tmpStr[512] = {0};
													snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the SCXB board", __LINE__);
													if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
												}

												retValue = -6;
												break;
											}
										}
										else
										{
											//serious fault in CS: No package found for the SCXB board
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No package found for the SCXB board", __LINE__);
												if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
											}

											retValue = -5;
											break;
										}
									}
								}
								else
								{
									//data already exists
									retValue = 0;
									std::cout << "data already exists..." << std::endl;
								}
							}
							else
							{
								//serious fault in CS: No slot found for the SCXB board
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the SCXB board", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								retValue = -4;
								break;
							}

						}
						else
						{
							//serious fault in CS: No magazine found for the SCXB board
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the SCXB board", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}
							retValue = -3;
							break;
						}
					}
				}


				/////////////////////////////
			}
			else
			{
				{//trace - GetBoardIds failed, error from CS
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				retValue = -2;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance failed, error from CS", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}

			retValue = -1;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		}
		retValue = -1;

	}

	return retValue;
}

int FIXS_CCH_UpgradeManager::fetchEPBInfo ()
{

	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_EPB1);

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no EPB board found. Any EPB hasn't been configured yet.
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No EPB board found", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
					retValue = 0;
				}
				else
				{
					// got some SCX board
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						uint32_t magazine = 0;
						returnValue = hwc->getMagazine (magazine, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							// magazine read
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0%x", __LINE__, magazine);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}

							unsigned short slot = 0;
							returnValue = hwc->getSlot (slot, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								// slot read
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								if (isEpbBoard(magazine, slot) == false)
								{
									std::string container("");
									std::string containerValue("");
									std::string dn_blade("");

									if(IMM_Interface::getDnBoard(magazine,slot,dn_blade))
									{
										returnValue = hwc->getContainerPackage (container, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
											// 										if (IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container))
										{
											uint32_t IP_EthA = 0;
											returnValue = hwc->getIPEthA (IP_EthA, boardId);
											if (returnValue == ACS_CS_API_NS::Result_Success)
											{
												uint32_t IP_EthB = 0;
												returnValue = hwc->getIPEthB (IP_EthB, boardId);
												if (returnValue == ACS_CS_API_NS::Result_Success)
												{
													containerValue=container;
													if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
													{
														std::string IPA("");
														std::string IPB("");
														std::string cxc_no("");
														std::string mag("");
														IPA = CCH_Util::ulongToStringIP(IP_EthA);
														IPB = CCH_Util::ulongToStringIP(IP_EthB);
														if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("entering after restore..calling checkCurrentVersionInBoard function ",LOG_LEVEL_WARN);
														bool ret = checkCurrentVersionInBoard (IPA, IPB,cxc_no); 
														if(ret)
														{
															ACS_CS_API_SET_NS::CS_API_Set_Result result;
															std::stringstream ss;
															ss << slot;
															std::string _slot = ss.str();
															CCH_Util::ulongToStringMagazine(magazine,mag);
															std::string mag_slot = mag + "_" + _slot;
															std::cout << "-------mag_slot= " << mag_slot.c_str() << std::endl;
															std::string currCxc = ACS_APGCC::after(cxc_no,"_");
															std::string currContainer = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(currCxc);
															if(!currContainer.empty())
                                                                                                                        {
                                                                                                                                containerValue=currContainer;
                                                                                                                                result = ACS_CS_API_Set::setSoftwareVersion(containerValue, mag, slot);
                                                                                                                                bool retryVal=false;
                                                                                                                                if(result!=ACS_CS_API_SET_NS::Result_Success)
                                                                                                                                {
                                                                                                                                        retryVal=retrySetSoftwareVersion(containerValue, mag, slot);
                                                                                                                                }
                                                                                                                                else
                                                                                                                                        retryVal=true;
                                                                                                                                if(retryVal==false) {
                                                                                                                                //      addScxEntry(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue ,dn_blade);       
                                                                                                                                        containerValue=container;
                                                                                                                                }
                                                                                                                        }
                                                                                                                }
                                                                                                        }

														addEpb1Entry(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue,dn_blade);

													retValue = 0;
												}
												else
												{
													//serious fault in CS: No IP_EthB found for the EPB board
													{
														char tmpStr[512] = {0};
														snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the EPB board", __LINE__);
														if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
													}

													retValue = -7;
													break;
												}
											}
											else
											{
												//serious fault in CS: No IP_EthA found for the SCXB board
												{
													char tmpStr[512] = {0};
													snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the SCXB board", __LINE__);
													if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
												}

												retValue = -6;
												break;
											}
										}
										else
										{
											//serious fault in CS: No package found for the SCXB board
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No package found for the SCXB board", __LINE__);
												if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
											}

											retValue = -5;
											break;
										}
									}
								}
								else
								{
									//data already exists
									retValue = 0;
									std::cout << "data already exists..." << std::endl;
								}
							}
							else
							{
								//serious fault in CS: No slot found for the SCXB board
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the SCXB board", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								retValue = -4;
								break;
							}

						}
						else
						{
							//serious fault in CS: No magazine found for the SCXB board
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the IPLBB board", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}
							retValue = -3;
							break;
						}
					}
				}


				/////////////////////////////
			}
			else
			{
				{//trace - GetBoardIds failed, error from CS
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				retValue = -2;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance failed, error from CS", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}

			retValue = -1;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		}
		retValue = -1;

	}

	return retValue;

}

int FIXS_CCH_UpgradeManager::fetchIPLBInfo ()
{

	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_IPLB);

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no IPLB board found. Any IPLB hasn't been configured yet.
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No IPLB board found", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
					retValue = 0;
				}
				else
				{
					// got some IPLB  board
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						uint32_t magazine = 0;
						returnValue = hwc->getMagazine (magazine, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							// magazine read
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0%x", __LINE__, magazine);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}

							unsigned short slot = 0;
							returnValue = hwc->getSlot (slot, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								// slot read
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								if (isIplbData(magazine, slot) == false)
								{
									std::string container("");
									std::string containerValue("");
										
									std::string dn_blade("");

									if(IMM_Interface::getDnBoard(magazine,slot,dn_blade))
									{
										returnValue = hwc->getContainerPackage (container, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
											// 										if (IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container))
										{
											uint32_t IP_EthA = 0;
											returnValue = hwc->getIPEthA (IP_EthA, boardId);
											if (returnValue == ACS_CS_API_NS::Result_Success)
											{
												uint32_t IP_EthB = 0;
												returnValue = hwc->getIPEthB (IP_EthB, boardId);
												if (returnValue == ACS_CS_API_NS::Result_Success)
												{
													containerValue=container;
													if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
													{														
														std::string IPA("");
														std::string IPB("");
														std::string cxc_no("");
														std::string mag("");
														IPA = CCH_Util::ulongToStringIP(IP_EthA);
														IPB = CCH_Util::ulongToStringIP(IP_EthB);
														if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("entering after restore..calling checkCurrentVersionInBoard function ",LOG_LEVEL_WARN);
														bool ret = checkCurrentVersionInBoard (IPA, IPB,cxc_no); 
														if(ret)
														{
															ACS_CS_API_SET_NS::CS_API_Set_Result result;
															std::stringstream ss;
															ss << slot;
															std::string _slot = ss.str();
															CCH_Util::ulongToStringMagazine(magazine,mag);
															std::string mag_slot = mag + "_" + _slot;
															std::cout << "-------mag_slot= " << mag_slot.c_str() << std::endl;
															std::string currCxc = ACS_APGCC::after(cxc_no,"IPLB_");
															std::string currContainer = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(currCxc);
															if(!currContainer.empty())
                                                                                                                        {
                                                                                                                                containerValue=currContainer;
                                                                                                                                result = ACS_CS_API_Set::setSoftwareVersion(containerValue, mag, slot);
                                                                                                                                bool retryVal=false;
                                                                                                                                if(result!=ACS_CS_API_SET_NS::Result_Success)
                                                                                                                                {
                                                                                                                                        retryVal=retrySetSoftwareVersion(containerValue, mag, slot);
                                                                                                                                }
                                                                                                                                else
                                                                                                                                        retryVal=true;
                                                                                                                                if(retryVal==false) {
                                                                                                                                        containerValue=container;
                                                                                                                                }
                                                                                                                        }
                                                                                                                }
                                                                                                        }
														addIplbData(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue,dn_blade);
						
													retValue = 0;
												}
												else
												{
													//serious fault in CS: No IP_EthB found for the IPLB board
													{
														char tmpStr[512] = {0};
														snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the IPLB board", __LINE__);
														if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
													}

													retValue = -7;
													break;
												}
											}
											else
											{
												//serious fault in CS: No IP_EthA found for the IPLB board
												{
													char tmpStr[512] = {0};
													snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the IPLB board", __LINE__);
													if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
												}

												retValue = -6;
												break;
											}
										}
										else
										{
											//serious fault in CS: No package found for the IPLB board
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No package found for the IPLB board", __LINE__);
												if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
											}

											retValue = -5;
											break;
										}
									}
								}
								else
								{
									//data already exists
									retValue = 0;
									std::cout << "data already exists..." << std::endl;
								}
							}
							else
							{
								//serious fault in CS: No slot found for the IPLB board
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the IPLBB board", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								retValue = -4;
								break;
							}

						}
						else
						{
							//serious fault in CS: No magazine found for the IPLB board
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the IPLB board", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}
							retValue = -3;
							break;
						}
					}
				}


				/////////////////////////////
			}
			else
			{
				{//trace - GetBoardIds failed, error from CS
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				retValue = -2;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance failed, error from CS", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}

			retValue = -1;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		}
		retValue = -1;

	}

	return retValue;
}

int FIXS_CCH_UpgradeManager::fetchIptInfo ()
{
	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_IPTB);

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no IPTB board found. Any IPTB hasn't been configured yet.
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No IPTB board found", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
					retValue = 0;
				}
				else
				{
					// got some IPTB board
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						uint32_t magazine = 0;
						returnValue = hwc->getMagazine (magazine, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							// magazine read
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0%x", __LINE__, magazine);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}

							unsigned short slot = 0;
							returnValue = hwc->getSlot (slot, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								// slot read
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								if (isIptData(magazine, slot) == false)
								{
									std::string container("");
									std::string containerValue("");
									std::string dn_blade("");

									if(IMM_Interface::getDnBoard(magazine,slot,dn_blade))
									{
										returnValue = hwc->getContainerPackage (container, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
											//										 if (IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container))
										{
											uint32_t IP_EthA = 0;
											returnValue = hwc->getIPEthA (IP_EthA, boardId);
											if (returnValue == ACS_CS_API_NS::Result_Success)
											{
												uint32_t IP_EthB = 0;
												returnValue = hwc->getIPEthB (IP_EthB, boardId);
												if (returnValue == ACS_CS_API_NS::Result_Success)
												{
													containerValue=container;
													if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
													{														
														std::string IPA("");
														std::string IPB("");
														std::string cxc_no("");
														std::string mag("");
														IPA = CCH_Util::ulongToStringIP(IP_EthA);
														IPB = CCH_Util::ulongToStringIP(IP_EthB);
														if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("entering after restore..calling checkCurrentVersionInBoard function ",LOG_LEVEL_WARN);
														bool ret = checkCurrentVersionInBoard (IPA, IPB,cxc_no); 
														if(ret)
														{
															ACS_CS_API_SET_NS::CS_API_Set_Result result;
															std::stringstream ss;
															ss << slot;
															std::string _slot = ss.str();
															CCH_Util::ulongToStringMagazine(magazine,mag);
															std::string mag_slot = mag + "_" + _slot;
															std::cout << "-------mag_slot= " << mag_slot.c_str() << std::endl;
															std::string currCxc = ACS_APGCC::after(cxc_no,"IPT_");
															std::string currContainer = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(currCxc);
															if(!currContainer.empty())
                                                                                                                        {
                                                                                                                                containerValue=currContainer;
                                                                                                                                result = ACS_CS_API_Set::setSoftwareVersion(containerValue, mag, slot);
                                                                                                                                bool retryVal=false;
                                                                                                                                if(result!=ACS_CS_API_SET_NS::Result_Success)
                                                                                                                                {
                                                                                                                                        retryVal=retrySetSoftwareVersion(containerValue, mag, slot);
                                                                                                                                }
                                                                                                                                else
                                                                                                                                        retryVal=true;
                                                                                                                                if(retryVal==false) {
                                                                                                                                //      addScxEntry(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue ,dn_blade);       
                                                                                                                                        containerValue=container;
                                                                                                                                }
                                                                                                                        }
                                                                                                                }
                                                                                                        }
													addIptData(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue,dn_blade);
													retValue = 0;
												}
												else
												{
													//serious fault in CS: No IP_EthB found for the IPTB board
													{
														char tmpStr[512] = {0};
														snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the IPTB board", __LINE__);
														if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
													}

													retValue = -7;
													break;
												}
											}
											else
											{
												//serious fault in CS: No IP_EthA found for the IPTB board
												{
													char tmpStr[512] = {0};
													snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the IPTB board", __LINE__);
													if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
												}

												retValue = -6;
												break;
											}
										}
										else
										{
											//serious fault in CS: No package found for the IPTB board
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No package found for the IPTB board", __LINE__);
												if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
											}

											retValue = -5;
											break;
										}
									}
								}
								else
								{
									//data already exists
									retValue = 0;
									std::cout << "data already exists..." << std::endl;
								}
							}
							else
							{
								//serious fault in CS: No slot found for the IPTB board
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the IPTB board", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								retValue = -4;
								break;
							}

						}
						else
						{
							//serious fault in CS: No magazine found for the IPTB board
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the IPTB board", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}
							retValue = -3;
							break;
						}
					}
				}


				/////////////////////////////
			}
			else
			{
				{//trace - GetBoardIds failed, error from CS
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				retValue = -2;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance failed, error from CS", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}

			retValue = -1;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		}
		retValue = -1;

	}

	return retValue;
}
int FIXS_CCH_UpgradeManager::fetchSMXInfo ()
{
	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SMXB);

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no SMX board found. Any SMX hasn't been configured yet.
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No SMX board found", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
					retValue = 0;
				}
				else
				{
					// got some SMX board
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						uint32_t magazine = 0;
						returnValue = hwc->getMagazine (magazine, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							// magazine read
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0%x", __LINE__, magazine);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}

							unsigned short slot = 0;
							returnValue = hwc->getSlot (slot, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								// slot read
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								if (isSmxData(magazine, slot) == false)
								{
									std::string container("");
									std::string dn_blade("");
									std::string containerValue("");

									if(IMM_Interface::getDnBoard(magazine,slot,dn_blade))
									{
										returnValue = hwc->getContainerPackage (container, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
											//										if (IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container))
										{
											uint32_t IP_EthA = 0;
											returnValue = hwc->getIPEthA (IP_EthA, boardId);
											if (returnValue == ACS_CS_API_NS::Result_Success)
											{
												uint32_t IP_EthB = 0;
												returnValue = hwc->getIPEthB (IP_EthB, boardId);
												if (returnValue == ACS_CS_API_NS::Result_Success)
												{
													containerValue=container;
													if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
													{
														std::string IPA("");
														std::string IPB("");
														std::string cxc_no("");
														std::string mag("");
														IPA = CCH_Util::ulongToStringIP(IP_EthA);
														IPB = CCH_Util::ulongToStringIP(IP_EthB);
														if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("entering after restore..calling checkCurrentVersionInBoard function ",LOG_LEVEL_WARN);
														bool ret = checkCurrentVersionInBoard (IPA, IPB,cxc_no);				
														if(ret)
														{
															ACS_CS_API_SET_NS::CS_API_Set_Result result;
															std::stringstream ss;
															ss << slot;
															std::string _slot = ss.str();
															CCH_Util::ulongToStringMagazine(magazine,mag);
															std::string mag_slot = mag + "_" + _slot;
															std::cout << "-------mag_slot= " << mag_slot.c_str() << std::endl;
															std::string currCxc = ACS_APGCC::after(cxc_no,"SMX_");
															std::string currContainer = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(currCxc);
															if(!currContainer.empty())
                                                                                                                        {
                                                                                                                                containerValue=currContainer;
                                                                                                                                result = ACS_CS_API_Set::setSoftwareVersion(containerValue, mag, slot);
                                                                                                                                bool retryVal=false;
                                                                                                                                if(result!=ACS_CS_API_SET_NS::Result_Success)
                                                                                                                                {
                                                                                                                                        retryVal=retrySetSoftwareVersion(containerValue, mag, slot);
                                                                                                                                }
                                                                                                                                else
                                                                                                                                        retryVal=true;
                                                                                                                                if(retryVal==false) {
                                                                                                                                        containerValue=container;
                                                                                                                                }
                                                                                                                        }
                                                                                                                }
                                                                                                        }
														addSmxEntry(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue,dn_blade);
													retValue = 0;
												}
												else
												{
													//serious fault in CS: No IP_EthB found for the SMXB board
													{
														char tmpStr[512] = {0};
														snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the SMXB board", __LINE__);
														if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
													}

													retValue = -7;
													break;
												}
											}
											else
											{
												//serious fault in CS: No IP_EthA found for the SMXB board
												{
													char tmpStr[512] = {0};
													snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the SMXB board", __LINE__);
													if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
												}

												retValue = -6;
												break;
											}
										}
										else
										{
											//serious fault in CS: No package found for the SMXB board
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No package found for the SMXB board", __LINE__);
												if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
											}

											retValue = -5;
											break;
										}
									}
								}
								else
								{
									//data already exists
									retValue = 0;
									std::cout << "data already exists..." << std::endl;
								}
							}
							else
							{
								//serious fault in CS: No slot found for the SMXB board
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the SMXB board", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								retValue = -4;
								break;
							}

						}
						else
						{
							//serious fault in CS: No magazine found for the SMXB board
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the SMXB board", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}
							retValue = -3;
							break;
						}
					}
				}


				/////////////////////////////
			}
			else
			{
				{//trace - GetBoardIds failed, error from CS
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				retValue = -2;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance failed, error from CS", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}

			retValue = -1;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		}
		retValue = -1;

	}

	return retValue;
}




bool FIXS_CCH_UpgradeManager::isScxData (unsigned long magazine, unsigned short slot)
{
	Lock lock;

	bool result = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			if (it->m_fbn == CCH_Util::SCXB)
			{
				cout << __FUNCTION__<<  " " << __LINE__ << "  FOUND !!!" << endl;
				result = true;
				break;
			}
		}
	}

	return result;
}
bool FIXS_CCH_UpgradeManager::isSmxData (unsigned long magazine, unsigned short slot)
{
	Lock lock;

	bool result = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			if (it->m_fbn == CCH_Util::SMXB)
			{
				cout << __FUNCTION__<<  " " << __LINE__ << "  FOUND !!!" << endl;
				result = true;
				break;
			}
		}
	}

	return result;
}

bool FIXS_CCH_UpgradeManager::isIptData (unsigned long magazine, unsigned short slot)
{
	Lock lock;

	bool result = false;
	for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			if (it->m_fbn == CCH_Util::IPTB)
			{
				cout << __FUNCTION__<<  " " << __LINE__ << "  IPTB DETAILS FOUND !!!" << endl;
				result = true;
				break;
			}
		}
	}
	return result;

}

bool FIXS_CCH_UpgradeManager::isIplbData (unsigned long magazine, unsigned short slot)
{
	Lock lock;

	bool result = false;
	cout << "Checking for IPLB Data" << "magazine" << magazine << "slot" << slot;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			cout << "FBN with magazine match" << it->m_fbn  ;
			if (it->m_fbn == CCH_Util::IPLB)
			{
				result = true;
				break;
			}
		}
	}
	return result;

}

void FIXS_CCH_UpgradeManager::addScxEntry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm)
{
	Lock lock;

	//to verify
	UNUSED(fsm);

	FsmListData data(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn, CCH_Util::SCXB);
	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " added magazine: " << data.m_magazine << ", slot:" << data.m_slot << ", IP_EthA:" << data.getIPEthAString().c_str() << ", IP_EthB:" << data.getIPEthBString().c_str() << "DN: " << data.m_dn <<std::endl;
	m_swUpgradeList->push_back(data);


	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ipA[%s] ipB[%s] cxp[%s] ", __FUNCTION__,__LINE__, MagValue.c_str(), data.m_slot, data.getIPEthAString().c_str(),data.getIPEthBString().c_str(), data.m_container.c_str());
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);

	if (!IMM_Interface::createBladeSwPersistance(magazine,slot))
            {
		cout << "FIXS_CCH_UpgradeManager: addScxEntry , FAILED to create persistance object !!!"<< endl;
                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : addScxEntry , FAILED to create persistance object !!! ",LOG_LEVEL_WARN);
	    }

	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;
	if (!IMM_Interface::setUsedPackage(container,usedRef))
	{
		cout << "FIXS_CCH_UpgradeManager: addScxEntry , FAILED to set used package !!!"<< endl;
               if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : addScxEntry , FAILED to set used package !!! ",LOG_LEVEL_WARN);
}
}
void FIXS_CCH_UpgradeManager::addSmxEntry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm)
{
	Lock lock;

	//to verify
	UNUSED(fsm);

	FsmListData data(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn, CCH_Util::SMXB);
	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " added magazine: " << data.m_magazine << ", slot:" << data.m_slot << ", IP_EthA:" << data.getIPEthAString().c_str() << ", IP_EthB:" << data.getIPEthBString().c_str() << "DN: " << data.m_dn <<"FBN:"<<data.m_fbn<<std::endl;
	m_swUpgradeList->push_back(data);


	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ipA[%s] ipB[%s] cxp[%s] ", __FUNCTION__,__LINE__, MagValue.c_str(), data.m_slot, data.getIPEthAString().c_str(),data.getIPEthBString().c_str(), data.m_container.c_str());
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);

	if (!IMM_Interface::createBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: addSmxEntry , FAILED to create persistance object !!!"<< endl;

	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;
	if (!IMM_Interface::setUsedPackage(container,usedRef)){
		cout << "FIXS_CCH_UpgradeManager: addSmxEntry , FAILED to set used package !!!"<< endl;
	}
}

void FIXS_CCH_UpgradeManager::addIptData (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *boardSm)
{
	Lock lock;

	//to verify
	UNUSED(boardSm);

	FsmListData data(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn, CCH_Util::IPTB);

	m_iptUpgradeList->push_back(data);

	std::string stringValue;
	(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ipA[%s] ipB[%s] ", __FUNCTION__,__LINE__, stringValue.c_str(), data.m_slot, data.getIPEthAString().c_str(),data.getIPEthBString().c_str());
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::createBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: addIptData , FAILED to create persistance object !!!"<< endl;

	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = stringValue+"_"+ sSlot;
	if (!IMM_Interface::setUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: addIptData , FAILED to set used package !!!"<< endl;

}

void FIXS_CCH_UpgradeManager::addIplbData (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *boardSm)
{
	Lock lock;

	//to verify
	UNUSED(boardSm);

	FsmListData data(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn, CCH_Util::IPLB);

	m_swUpgradeList->push_back(data);


	std::string stringValue;
	(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ipA[%s] ipB[%s] ", __FUNCTION__,__LINE__, stringValue.c_str(), data.m_slot, data.getIPEthAString().c_str(),data.getIPEthBString().c_str());
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::createBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: addIplbData , FAILED to create persistance object !!!"<< endl;

	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = stringValue+"_"+ sSlot;
	if (!IMM_Interface::setUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: addIplbData , FAILED to set used package !!!"<< endl;

}


int FIXS_CCH_UpgradeManager::startSWUpgradeFSMs ()
{

	int result  = 0;

	std::string dn_object("");
	std::string dn_swInv("");
	std::string dn_swPers("");

	CCH_Util::SWData wdata;

	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (it->m_swPtr == 0)
		{
			if (it->m_fbn == CCH_Util::SCXB)
			{
				FIXS_CCH_ScxUpgrade *swup = new FIXS_CCH_ScxUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);

				if (swup)
				{
					CCH_Util::SWData swOut;
					dn_swInv.clear();
					dn_swPers.clear();
					dn_swInv = swup->getSwInvObjectDN();
					it->m_swPtr = swup;
					dn_swPers = swup->getSwPercistanceDN();
					swOut.dnBladePersistance = dn_swPers;

					ACS_APGCC_ImmObject object;

					if (IMM_Util::getObject(dn_swInv, &object))
					{
						//std::cout << __FUNCTION__<< " SetDataFromModel " << std::endl;
						if (IMM_Interface::setSwDataFromModel(it->m_dn, dn_swInv, swOut))
						{
							if (strcmp(it->m_container.c_str(),swOut.currentCxp.c_str())!=0)
							{
								it->m_container = swOut.currentCxp;
							}

							std::cout << "\n SET PARAMETERS FROM MODEL" << std::endl;
							swup->restoreStatus(swOut);

							if (( swOut.upgradeStatus != CCH_Util::SW_IDLE ) && (swOut.upgradeStatus != CCH_Util::SW_UNKNOWN) && (swOut.upgradeStatus != CCH_Util::SW_FAILED))
							{
								swup->open();
								std::cout << "\n UPGRADE ONGOING on board :\n"<< it->m_dn.c_str() << std::endl;
							}
							else
							{
								//std::cout << " STATE: IDLE" << std::endl;
							}
						}
						else
						{
							if (swup->initSwInventory())
							{
								swup->getOverallStatus(wdata);
								std::cout << "\n SET DEFAULT PARAMETERS" << std::endl;
							}
							else
							{
								std::cout << "\n ERROR SET DEFAULT PARAMETERS "<< std::endl;
							}
						}
					}
					else
					{
						std::cout << "\nIMM Error : Child Object NOT EXIST "<< std::endl;
                                                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : IMM Error: Child Object Not exist !!! ",LOG_LEVEL_WARN);
					}
				}
				else
				{
					__;
					printError("Memory Error\n",__FUNCTION__, __LINE__);
				}
			}
			else if (it->m_fbn == CCH_Util::CMXB)
			{
				FIXS_CCH_CmxUpgrade *swup = new FIXS_CCH_CmxUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);

				if (swup)
				{
					CCH_Util::SWData swOut;
					dn_swInv.clear();
					dn_swPers.clear();
					dn_swInv = swup->getSwInvObjectDN();
					it->m_swPtr = swup;
					dn_swPers = swup->getSwPercistanceDN();
					swOut.dnBladePersistance = dn_swPers;

					ACS_APGCC_ImmObject object;

					if (IMM_Util::getObject(dn_swInv, &object))
					{
						//std::cout << __FUNCTION__<< " SetDataFromModel " << std::endl;
						if (IMM_Interface::setSwDataFromModel(it->m_dn, dn_swInv, swOut))
						{
							if (strcmp(it->m_container.c_str(),swOut.currentCxp.c_str())!=0)
							{
								it->m_container = swOut.currentCxp;
							}

							std::cout << "\n SET PARAMETERS FROM MODEL" << std::endl;
							swup->restoreStatus(swOut);

							if (( swOut.upgradeStatus != CCH_Util::SW_IDLE ) && (swOut.upgradeStatus != CCH_Util::SW_UNKNOWN) && (swOut.upgradeStatus != CCH_Util::SW_FAILED))
							{
								swup->open();
								std::cout << "\n UPGRADE ONGOING on board :\n"<< it->m_dn.c_str() << std::endl;
							}
							else
							{
								//std::cout << " STATE: IDLE" << std::endl;
							}
						}
						else
						{
							if (swup->initSwInventory())
							{
								swup->getOverallStatus(wdata);
								std::cout << "\n SET DEFAULT PARAMETERS" << std::endl;
							}
							else
							{
								std::cout << "\n ERROR SET DEFAULT PARAMETERS "<< std::endl;
								//result = 1;
							}
						}

					}
					else
					{
						std::cout << "\nIMM Error : Child Object NOT EXIST "<< std::endl;
						//result = 1;
					}
				}
				else
				{
					__;
					printError("Memory Error\n",__FUNCTION__, __LINE__);
					//result = 1;
				}
			}
			else if (it->m_fbn == CCH_Util::IPLB)
			{
				std::string shelfparent("");

				IMM_Util::getDnParent(it->m_dn, shelfparent);

				FIXS_CCH_IplbUpgrade *swup = new FIXS_CCH_IplbUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);


				if (swup)
				{
					CCH_Util::SWData swOut;
					dn_swInv.clear();
					dn_swPers.clear();
					dn_swInv = swup->getSwInvObjectDN();
					it->m_swPtr = swup;
					dn_swPers = swup->getSwPercistanceDN();
					swOut.dnBladePersistance = dn_swPers;

					ACS_APGCC_ImmObject object;

					if (IMM_Util::getObject(dn_swInv, &object))
					{
						//std::cout << __FUNCTION__<< " SetDataFromModel " << std::endl;
						if (IMM_Interface::setSwDataFromModel(it->m_dn, dn_swInv, swOut))
						{
							if (strcmp(it->m_container.c_str(),swOut.currentCxp.c_str())!=0)
							{
								it->m_container = swOut.currentCxp;
							}

							std::cout << "\n SET PARAMETERS FROM MODEL" << std::endl;
							swup->restoreStatus(swOut);

							if (( swOut.upgradeStatus != CCH_Util::SW_IDLE ) && (swOut.upgradeStatus != CCH_Util::SW_UNKNOWN) && (swOut.upgradeStatus != CCH_Util::SW_FAILED))
							{
								swup->open();
								std::cout << "\n UPGRADE ONGOING on board :\n"<< it->m_dn.c_str() << std::endl;
							}
							else
							{
								//std::cout << " STATE: IDLE" << std::endl;
							}
						}
						else
						{
							if (swup->initSwInventory())
							{
								swup->getOverallStatus(wdata);
								std::cout << "\n SET DEFAULT PARAMETERS" << std::endl;
							}
							else
							{
								std::cout << "\n ERROR SET DEFAULT PARAMETERS "<< std::endl;
								//result = 1;
							}
						}

					}
					else
					{
						std::cout << "\nIMM Error : Child Object NOT EXIST "<< std::endl;
						//result = 1;
					}
				}
				else
				{
					__;
					printError("Memory Error\n",__FUNCTION__, __LINE__);
					//result = 1;
				}
			}
			else if (it->m_fbn == CCH_Util::SMXB)
			{
				FIXS_CCH_SmxUpgrade *swup = new FIXS_CCH_SmxUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);

				if (swup)
				{
					CCH_Util::SWData swOut;
					dn_swInv.clear();
					dn_swPers.clear();
					dn_swInv = swup->getSwInvObjectDN();
					it->m_swPtr = swup;
					dn_swPers = swup->getSwPercistanceDN();
					swOut.dnBladePersistance = dn_swPers;

					ACS_APGCC_ImmObject object;

					if (IMM_Util::getObject(dn_swInv, &object))
					{
						//std::cout << __FUNCTION__<< " SetDataFromModel " << std::endl;
						if (IMM_Interface::setSwDataFromModel(it->m_dn, dn_swInv, swOut))
						{
							if (strcmp(it->m_container.c_str(),swOut.currentCxp.c_str())!=0)
							{
								it->m_container = swOut.currentCxp;
							}

							std::cout << "\n SET PARAMETERS FROM MODEL" << std::endl;
							swup->restoreStatus(swOut);

							if (( swOut.upgradeStatus != CCH_Util::SW_IDLE ) && (swOut.upgradeStatus != CCH_Util::SW_UNKNOWN) && (swOut.upgradeStatus != CCH_Util::SW_FAILED))
							{
								swup->open();
								std::cout << "\n UPGRADE ONGOING on board :\n"<< it->m_dn.c_str() << std::endl;
							}
							else
							{
								//std::cout << " STATE: IDLE" << std::endl;
							}
						}
						else
						{
							if (swup->initSwInventory())
							{
								swup->getOverallStatus(wdata);
								std::cout << "\n SET DEFAULT PARAMETERS" << std::endl;
							}
							else
							{
								std::cout << "\n ERROR SET DEFAULT PARAMETERS "<< std::endl;
							}
						}
					}
					else
					{
						std::cout << "\nIMM Error : Child Object NOT EXIST "<< std::endl;
					}
				}
				else
				{
					__;
					printError("Memory Error\n",__FUNCTION__, __LINE__);
				}
			}
			
		}
	}

	return result;
}


int FIXS_CCH_UpgradeManager::prepare (unsigned long magazine, unsigned short slot, std::string product, int upgradeType , bool force)
{

	int result = INTERNALERR;

	if(isBackupOngoing() == true){
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : NOT ALLOWED, BACKUP ONGOING ",LOG_LEVEL_ERROR);
		return NOT_ALLOWED_BACKUP_ONGOING;
	}

	if(isScxData(magazine,slot))
	{
		return prepareScx(magazine,slot,product);
	}
	else if(isCmxData(magazine,slot))
	{
		return prepareCmx(magazine,slot,product);
	}
	else if(isIptData(magazine,slot))
	{
		return prepareIpt(magazine,slot, product,upgradeType);
	}
	else if(isEpbBoard(magazine,slot))
	{
		return prepareEpb1(magazine,slot, product);
	}
	else if (isIplbData(magazine,slot))
	{
		return prepareIplb(magazine,slot, product,upgradeType,force);
	}
	else if(isSmxData(magazine,slot))
	{
		return prepareSmx(magazine,slot,product);
	}
	else result = FUNC_BOARD_NOTALLOWED; // option not allowed for this fbn

	return result;
}


int FIXS_CCH_UpgradeManager::activate (unsigned long umagazine, unsigned short uslot)
{

	int result = INTERNALERR;

	if(isBackupOngoing() == true){
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : NOT ALLOWED, BACKUP ONGOING ",LOG_LEVEL_ERROR);
		return NOT_ALLOWED_BACKUP_ONGOING;
	}

	if (isScxData(umagazine, uslot))
	{
		return activateScx(umagazine, uslot);
	}
	else if (isCmxData(umagazine, uslot))
	{
		return activateCmx(umagazine, uslot);
	}
	else if (isIptData(umagazine, uslot))
	{
		return activateIpt(umagazine, uslot );
	}
	else if (isIplbData(umagazine, uslot))
	{
		return activateIplb(umagazine, uslot);
	}
	else if (isSmxData(umagazine, uslot))
	{
		return activateSmx(umagazine, uslot);
	}
	else
	{
		if (isEpbBoard(umagazine, uslot)) return FUNC_BOARD_NOTALLOWED;
	}

	return result;

}

int FIXS_CCH_UpgradeManager::cancel (unsigned long umagazine, unsigned short uslot)
{
	int result = INTERNALERR;

	if(isBackupOngoing() == true){
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : NOT ALLOWED, BACKUP ONGOING ",LOG_LEVEL_ERROR);
		return NOT_ALLOWED_BACKUP_ONGOING;
	}
	//check if hardware exists
	if (isScxData(umagazine, uslot))
	{
		return cancelScx(umagazine, uslot);
	}
	else if (isCmxData(umagazine, uslot))
	{
		return cancelCmx(umagazine, uslot);
	}
	else if (isIptData(umagazine, uslot))
	{
		return cancelIpt(umagazine, uslot);
	}
	else if (isIplbData(umagazine, uslot))
	{
		return cancelIplb(umagazine, uslot);
	}
	else if (isSmxData(umagazine, uslot))
	{
		return cancelSmx(umagazine, uslot);
	}
	else if (isEpbBoard(umagazine, uslot)) return FUNC_BOARD_NOTALLOWED;

	return result;

}


int FIXS_CCH_UpgradeManager::commit (unsigned long umagazine, unsigned short uslot)
{

	int result = INTERNALERR;

	if(isBackupOngoing() == true){
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : NOT ALLOWED, BACKUP ONGOING ",LOG_LEVEL_ERROR);
		return NOT_ALLOWED_BACKUP_ONGOING;
	}

	//check if hardware exists
	if (isScxData(umagazine, uslot))
	{
		return commitScx(umagazine,uslot);
	}
	else if (isCmxData(umagazine, uslot))
	{
		return commitCmx(umagazine,uslot);
	}
	else if (isIptData(umagazine, uslot))
	{
		return commitIpt(umagazine, uslot, false);
	}
	else if (isIplbData(umagazine, uslot))
	{
		return commitIplb(umagazine,uslot,false);
	}
	else if (isSmxData(umagazine, uslot))
	{
		return commitSmx(umagazine,uslot);
	}
	else
	{
		if (isEpbBoard(umagazine, uslot)) return FUNC_BOARD_NOTALLOWED;
	}
	return result;

}

unsigned short FIXS_CCH_UpgradeManager::numOfChar (std::istringstream &istr, char searchChar)
{


	istr.clear();        // return to the beginning
	(void)istr.seekg(0); //      of the stream
	char c;
	unsigned short numChar = 0;

	do {
		c = istr.peek();
		if (c == searchChar)
			numChar = numChar + 1;

		(void)istr.seekg((int)(istr.tellg()) + 1);
	} while (c != EOF);

	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    // of the stream
	return numChar;

}

void FIXS_CCH_UpgradeManager::reverseDottedDecStr (std::istringstream &istr)
{


	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    //      of the stream
	std::istringstream tempIstr;
	std::string tempString = "";
	char c;

	do
	{
		c = istr.get();      //get char from istringstream

		if (c == '.' || c == EOF)
		{
			(void)tempIstr.seekg(0);

			if (c == '.')
				tempIstr.str(c + tempString + tempIstr.str());
			else
				tempIstr.str(tempString + tempIstr.str());

			tempString = "";
		}
		else
			tempString = tempString + c;
	} while (c != EOF);

	istr.str(tempIstr.str());

	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    // of the stream

}

bool FIXS_CCH_UpgradeManager::dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4)
{


	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    //      of the stream
	char c;
	std::string strAddr = "";
	unsigned short dotNum = 0;
	value = 0;

	do {
		c = istr.get();
		if (c == '.' || c == EOF)
		{
			dotNum++;
			std::istringstream converter(strAddr);

			if (! this->checkFigures(converter))   //checks that only figures (0-9) are used in the option arg.
				return false;

			strAddr = "";
			unsigned long plug;
			if (! (converter >> plug)) //check if string is possible to convert to long
			{
				value = 0;
				return false;
			}

			else
			{
				if (dotNum == 1)
				{
					if (! (((plug >= lowerLimit1) && (plug <= upperLimit1)) || (plug == 255)))  //CR GESB
					{
						value = 0;
						return false;
					}
				}

				else if (dotNum == 2)
				{
					if (! ((plug >= lowerLimit2) && (plug <= upperLimit2)))
					{
						value = 0;
						return false;
					}
				}

				else if (dotNum == 3)
				{
					if (! (((plug >= lowerLimit3) && (plug <= upperLimit3)) || (plug == 255)))  //CR GESB
					{
						value = 0;
						return false;
					}
				}

				else if (dotNum == 4)
				{
					if (! (((plug >= lowerLimit4) && (plug <= upperLimit4)) || (plug == 255)))  //CR GESB
					{
						value = 0;
						return false;
					}
				}

				value = plug | value;

				if (dotNum != 4)     // if not last byte, shift 1 byte to the left
					value = value << 8;
			}
		}

		else
			strAddr = strAddr + c;
	} while (c != EOF);

	return true;

}

bool FIXS_CCH_UpgradeManager::checkFigures (std::istringstream &istr)
{
	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    //      of the stream
	char c = istr.get(); //get first char in the istringstream

	while (c != EOF)
	{
		if ((c < '0') || (c > '9'))
			return false;

		c = istr.get();
	}

	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    //      of the stream
	return true;

}

bool FIXS_CCH_UpgradeManager::getUlongMagazine (std::string magazine, unsigned long &umagazine)
{


	// convert magazine
	std::istringstream istr(magazine);
	reverseDottedDecStr(istr); //reverses the order of the dotted decimal string to match the mag addr format on the server
	if (numOfChar(istr,'.') != 3) // If not 3 dots, faulty format
	{

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] faulty format. magazine = %s ", __FUNCTION__, __LINE__, magazine.c_str());
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
		return false;
	}
	if (! dottedDecToLong(istr, umagazine, 0, 15, 0, 0, 0, 15, 0, 15))
	{

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] faulty format. magazine = %s ", __FUNCTION__, __LINE__, magazine.c_str());
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
		return false;
	}
	return true;

}

bool FIXS_CCH_UpgradeManager::getUshortSlot (std::string slot, unsigned short &uslot)
{

	istringstream istr(slot);
	if (! this->checkFigures(istr))  //checks that only figures (0-9) are used in the option arg.
	{

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] faulty format. slot = %s ", __FUNCTION__, __LINE__, slot.c_str());
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}

		return false; //Usage
	}

	if (! (istr >> uslot))   //try to convert operand to slot number
	{

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] faulty format. slot = %s ", __FUNCTION__, __LINE__, slot.c_str());
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}

		return false; //Usage
	}

	if ((uslot > 28) || (uslot == 27))
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] faulty value. slot = %s ", __FUNCTION__, __LINE__, slot.c_str());
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}

		return false; //Usage
	}
	return true;
}

void FIXS_CCH_UpgradeManager::loadReleaseCompleted (int loadResult, std::string snmpIP)
{
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] loadReleaseCompleted TRAP from IP \"%s\" with result \"%d\"", __LINE__, snmpIP.c_str(), loadResult);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
	}

	if (checkSubnetIpTrapReceiver(snmpIP, CCH_Util::LoadReleaseComplete,false)) return;

	Lock lock;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->getIPEthAString() == snmpIP) || (it->getIPEthBString() == snmpIP))
		{
			if (it->m_fbn == CCH_Util::SCXB)
			{
				FIXS_CCH_ScxUpgrade *stateMachine = 0;
				try
				{

					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "STATE MACHINE FOUND !!!" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "snmpIP: " << snmpIP.c_str() << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{


					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}

					stateMachine = 0;
				}

				if (stateMachine)
				{
					int res = -1;
					res = stateMachine->loadReleaseCompleted(loadResult);
				}

				break;
			}
			else if (it->m_fbn == CCH_Util::CMXB)
			{
				FIXS_CCH_CmxUpgrade *stateMachine = 0;
				try
				{

					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "STATE MACHINE FOUND !!!" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "snmpIP: " << snmpIP.c_str() << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{


					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_CmxUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}

					stateMachine = 0;
				}

				if (stateMachine)
				{
					int res = -1;
					res = stateMachine->loadReleaseCompleted(loadResult);
				}

				break;
			}
			else if (it->m_fbn == CCH_Util::SMXB)
			{
				FIXS_CCH_SmxUpgrade *stateMachine = 0;
				try
				{

					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "STATE MACHINE FOUND !!!" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "snmpIP: " << snmpIP.c_str() << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{


					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}

					stateMachine = 0;
				}

				if (stateMachine)
				{
					int res = -1;
					res = stateMachine->loadReleaseCompleted(loadResult);
				}

				break;
			}
			else
			{
				break;
			}
		}
	}

	//clean subnet trap check
	checkSubnetIpTrapReceiver(snmpIP,CCH_Util::LoadReleaseComplete,true);

}

void FIXS_CCH_UpgradeManager::coldStart (std::string snmpIP)
{
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] coldStart TRAP from IP \"%s\"", __LINE__, snmpIP.c_str());
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
	}

	//	if (checkSubnetIpTrapReceiver(snmpIP, CCH_Util::ColdStartWarmStart,false)) return;

	//	Lock lock;


	int result = 0;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->getIPEthAString() == snmpIP) || (it->getIPEthBString() == snmpIP))
		{
			if(it->m_fbn == CCH_Util::SCXB)
			{
				FIXS_CCH_ScxUpgrade *stateMachine = 0;

				try
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "STATE MACHINE FOUND !!!" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "snmpIP: " << snmpIP.c_str() << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{


					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}

					stateMachine = 0;
				}

				if (stateMachine)
				{
					result = stateMachine->coldStart();
				}
				else
				{
					//result = INTERNALERR;
				}
				break;
			}
			else if(it->m_fbn == CCH_Util::CMXB)
			{
				FIXS_CCH_CmxUpgrade *stateMachine = 0;

				try
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "STATE MACHINE FOUND !!!" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "snmpIP: " << snmpIP.c_str() << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{


					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_CmxUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}

					stateMachine = 0;
				}

				if (stateMachine)
				{
					result = stateMachine->coldStart();
				}
				else
				{
					//result = INTERNALERR;
				}
				break;
			}
			else if(it->m_fbn == CCH_Util::SMXB)
			{
				FIXS_CCH_SmxUpgrade *stateMachine = 0;

				try
				{
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "STATE MACHINE FOUND !!!" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "snmpIP: " << snmpIP.c_str() << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "***************************************************************" << std::endl;
					stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}

					stateMachine = 0;
				}

				if (stateMachine)
				{
					result = stateMachine->coldStart();
				}
				else
				{
					//result = INTERNALERR;
				}
				break;
			}
			else
			{
				break;
			}
		}
	}
	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
	{
		checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);
	}
	else if ((environment == CCH_Util::SINGLECP_DMX) || (environment == CCH_Util::MULTIPLECP_DMX))
	{
		checkIPLBswVr();
	}
	else if ((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
	{
		checkSMXswVr();
	}
	//clean subnet trap check
	//	checkSubnetIpTrapReceiver(snmpIP,CCH_Util::ColdStartWarmStart,true);

}


bool FIXS_CCH_UpgradeManager::checkOtherSlotStatus (unsigned long umagazine, unsigned short uslot)
{
	//check other slot SW upgrade status for CMX and SCX or SMX
	bool retValue = true;
	bool found = false;
	unsigned short OtherUslot;

	OtherUslot = 0;
	if (uslot == 0) OtherUslot = 25;
	else if (uslot == 25) OtherUslot = 0;
	else if (uslot == 26) OtherUslot = 28;
	else if (uslot == 28) OtherUslot = 26;

	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == OtherUslot))
			{
				found = true;
				if  (it->m_swPtr == 0) return true;

				if  (it->m_fbn == CCH_Util::SCXB)
				{

					FIXS_CCH_ScxUpgrade *stateMachine = 0;
					try
					{
						if  (it->m_fbn == CCH_Util::SCXB) stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);

					}
					catch(bad_cast)
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}
						stateMachine = 0;
					}
					if ((stateMachine) && (stateMachine->getStateId() != stateMachine->STATE_IDLE ) && (stateMachine->getStateId() != stateMachine->STATE_FAILED))
					{
						cout << "DBG: Check Other Slot Status : " << stateMachine->getStateName().c_str() << endl;
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}
						retValue = false;
					}
					break;
				}

				else if  (it->m_fbn == CCH_Util::CMXB)
				{
					FIXS_CCH_CmxUpgrade *stateMachine = 0;

					try
					{
						if  (it->m_fbn == CCH_Util::CMXB) stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);

					}
					catch(bad_cast)
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_CmxUpgrade", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}
						stateMachine = 0;
					}
					if ((stateMachine) && (stateMachine->getStateId() != stateMachine->STATE_IDLE ) && (stateMachine->getStateId() != stateMachine->STATE_FAILED))
					{
						cout << "DBG: Check Other Slot Status : " << stateMachine->getStateName().c_str() << endl;
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}
						retValue = false;
					}
					break;
				}
				else if  (it->m_fbn == CCH_Util::SMXB)
				{
					FIXS_CCH_SmxUpgrade *stateMachine = 0;
					try
					{
						if  (it->m_fbn == CCH_Util::SMXB) stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
					}
					catch(bad_cast)
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}
						stateMachine = 0;
					}
					if ((stateMachine) && (stateMachine->getStateId() != stateMachine->STATE_IDLE ) && (stateMachine->getStateId() != stateMachine->STATE_FAILED))
					{
						cout << "DBG: Check Other Slot Status : " << stateMachine->getStateName().c_str() << endl;
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}
						retValue = false;
					}
					break;
				}
				else
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
					break;
				}
			}
		}
	}

	return retValue;
}

bool FIXS_CCH_UpgradeManager::isScxSoftwareUpgradeInProgress ()
{

	//Lock lock;
	bool result = false;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (it->m_swPtr)
		{

			FIXS_CCH_ScxUpgrade *stateMachine = 0;
			try
			{

				stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				stateMachine = 0;
			}

			if (stateMachine)
			{
				int stateSw = stateMachine->getStateImm();
				if ((stateSw != IMM_Util::IDLE) && (stateSw != IMM_Util::UNKNOWN) && (stateSw != IMM_Util::FAILED))
				{
					std::cout << "DBG: AT LEAST ONE SW STATE MACHINE IS NOT IDLE" << std::endl;
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("AT LEAST ONE SW STATE MACHINE IS NOT IDLE",LOG_LEVEL_ERROR);
					result = true;
					break;
				}
			}

		}
	}

	return result;

}
bool FIXS_CCH_UpgradeManager::isIptLoadModuleUpgradeInProgress ()
{

	// Lock lock;
	bool result = false;
	for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{
		if (it->m_lmPtr)
		{
			FIXS_CCH_IptbUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
			}
			catch (bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_lmPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				stateMachine = 0;
			}

			if (stateMachine)
			{
				int stateLm = stateMachine->getStateImm();
				if ((stateLm != IMM_Util::IDLE) && (stateLm != IMM_Util::UNKNOWN) && (stateLm != IMM_Util::FAILED))
				{
					std::cout << "DBG: AT LEAST ONE IPT LM STATE MACHINE IS NOT IDLE" << std::endl;
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FIXS_CCH_UpgradeManager:IPTB UPGRADE ONGOING!!!", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
					result = true;
					break;
				}
			}
		}
	}

	return result;
}
bool FIXS_CCH_UpgradeManager::isIptLoadModuleUpgradeInProgress(std::string container)
{

	bool result = false;
	for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{
		if (it->m_lmPtr)
		{
			FIXS_CCH_IptbUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
			}
			catch (bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_lmPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				stateMachine = 0;
			}
			if (stateMachine)
			{
				cout << "stateMachine->getExpectedCXP() " << stateMachine->getExpectedCXP() << endl;
				if(stateMachine->getExpectedCXP()==container)
				{
					int stateLm = stateMachine->getStateImm();
					if ((stateLm != IMM_Util::IDLE) && (stateLm != IMM_Util::UNKNOWN) && (stateLm != IMM_Util::FAILED))
					{
						std::cout << "DBG: AT LEAST ONE IPLB LM STATE MACHINE IS NOT IDLE" << std::endl;
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FIXS_CCH_UpgradeManager:IPTB UPGRADE ONGOING!!!", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}
						result = true;
						break;
					}
					else
					{

						std::cout << "DBG: IPTB LM STATE MACHINE IS IDLE" << std::endl;
						result = false;
						break;
					}
				}
			}
		}
	}

	return result;

}
bool FIXS_CCH_UpgradeManager::isSmxSoftwareUpgradeInProgress ()
{

	//Lock lock;
	bool result = false;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (it->m_swPtr)
		{

			FIXS_CCH_SmxUpgrade *stateMachine = 0;
			try
			{

				stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				stateMachine = 0;
			}

			if (stateMachine)
			{
				int stateSw = stateMachine->getStateImm();
				if ((stateSw != IMM_Util::IDLE) && (stateSw != IMM_Util::UNKNOWN) && (stateSw != IMM_Util::FAILED))
				{
					std::cout << "DBG: AT LEAST ONE SW STATE MACHINE IS NOT IDLE" << std::endl;
					result = true;
					break;
				}
			}

		}
	}

	return result;

}

int FIXS_CCH_UpgradeManager::ipmiUpgradeStatus (std::string magazine, std::string slot, std::string slotMaster, std::string ipMasterSCX)
{
	UNUSED(slotMaster);
	UNUSED(ipMasterSCX);

	int result = HARDWARE_NOT_PRESENT; //HARDWARE_NOT_PRESENT

	FIXS_CCH_IpmiUpgrade *stateMachine = NULL;
	FIXS_CCH_IpmiUpgrade *stateMachineOther = NULL;//new
	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	unsigned short uMagSlot = 0;
	int mibStatusOnSCX = -1, tempmibStatusOnOtherSCX = -1;
	std::string tempSlot;

	std::string masterScxIp("");
	std::string passiveScxIp("");
	std::string slotStr("");
	int slotOngoing = -1;

	int slotOngoingOther = -1;


	// std::string sSlotStateMachine("");
	unsigned short uSlotStateMachine = 0;
	unsigned short uSlotStateMachineOther = 0;

	//int iSlot = atoi(slot.c_str());

	// convert magazine
	if (getUlongMagazine(magazine, umagazine) == false) return result;
	// convert slot
	if (getUshortSlot(slot, uslot) == false) return result;

	if ( (!isScxData(umagazine, uslot)) && (!isEpbBoard(umagazine,uslot)) && (!isCmxData(umagazine,uslot)) && (!isEvoEtData(umagazine,uslot)) )
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ERROR: these values don't match with board type data: " << umagazine << " " << uslot << std::endl;
		//return result;
		return FUNC_BOARD_NOTALLOWED;
	}

	if(getMasterPassiveSCX(umagazine, masterScxIp, passiveScxIp, uMagSlot) == false) return result;

	if(CCH_Util::ushortToString(uMagSlot, slotStr) == false) return result;

	// sSlotStateMachine = slotStr;
	uSlotStateMachine = uMagSlot;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
			" uSlotStateMachine " << uSlotStateMachine <<
			std::endl;


	// //find state machine threads for both slot 0 and 25
	std::string tmpIPA, tmpOtherIPA;
	std::string tmpIPB, tmpOtherIPB;
	if(uSlotStateMachine == 0)
		uSlotStateMachineOther = 25;
	else
		uSlotStateMachineOther = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
			" uSlotStateMachine = " << uSlotStateMachine <<
			" uSlotStateMachineOther = " << uSlotStateMachineOther <<
			std::endl;


	std::string dn_pers = IMM_Interface::getDnBladePersistance(umagazine, uslot);

	Lock lock;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		//if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		if ((it->m_magazine == umagazine))// && (it->m_slot == uslot))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
					" getIPEthAString " << it->getIPEthAString().c_str() <<
					" getIPEthBString " << it->getIPEthBString().c_str() <<
					" m_slot " << it->m_slot << std::endl;

			if(it->m_slot == uSlotStateMachine)
			{
				try
				{
					tmpIPA = it->getIPEthAString();
					tmpIPB = it->getIPEthBString();
					stateMachine = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
					if (stateMachine)
					{
						stateMachine->m_ipMaster = masterScxIp;
						stateMachine->m_slotMaster = slotStr;
						slotOngoing = stateMachine->m_slotUpgOnGoing;

						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << 	" ipmiMachineState status = " << stateMachine->getStateName().c_str() << std::endl;
					}

				}
				catch (bad_cast)
				{
					stateMachine = 0;
				}
				if (!stateMachine)
				{
					return 1; // Internal Error
				}
			}

			if(it->m_slot == uSlotStateMachineOther)
			{
				try
				{
					tmpOtherIPA = it->getIPEthAString();
					tmpOtherIPB = it->getIPEthBString();
					stateMachineOther = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
					if (stateMachineOther)
					{
						stateMachineOther->m_ipMaster = masterScxIp;
						stateMachineOther->m_slotMaster = slotStr;
						slotOngoingOther = stateMachineOther->m_slotUpgOnGoing;

						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << 	" StatemachineOther status="<<stateMachineOther->getStateName().c_str()<<"uslot:"<<uslot<<std::endl;
					}


				}
				catch (bad_cast)
				{
					stateMachineOther = 0;
				}
				if (!stateMachineOther)
				{
					return 1; // Internal Error
				}
			}
		}
	}




	//--------------------------------------------------------------
	// The Following code is used to do a "double check"
	// between State machine and MIB Values
	//	Example (use case):
	//		1) Positive case
	//		- MIB Value: ongoing
	//		- State machine is ONGOING
	//		-> OK
	//
	//		2) Negative case
	//		- MIB Value: ready
	//		- State Machine is ONGOING
	//		-> NOK, this is a mismatch, State machine will turn off
	//--------------------------------------------------------------

	//SNMP request to check the status of the IPMI on board
	//	int iSlotMaster = atoi(slotStr.c_str());

	if (stateMachine->getStateName() == "ONGOING")
	{
		if ((slotOngoing == (int)uslot))//||(slotOngoingOther == (int)uslot))
		{
			// State Machine ONGOING
			mibStatusOnSCX = stateMachine->checkIPMIStatus(tmpIPA, tmpIPB, slot);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
					" mibStatusOnSCX " << mibStatusOnSCX <<
					" tmpIPA " << tmpIPA.c_str() <<
					" tmpIPB " << tmpIPB.c_str() <<
					" slot " << slot << std::endl;
			if ( mibStatusOnSCX == -1)
			{
				result = SNMP_FAILURE;
			}
			else if (mibStatusOnSCX == 1)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IPMI ongoing and state machine ongoing " << std::endl;
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] IPMI ongoing and state machine ongoing" , __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
				}
				result = EXECUTED;
				m_ipmiResult = "ONGOING";
			}
			else
			{
				// The MIB Status is Idle, but the State Machines are ongoing , it's needed turn off it!
				//State machine is on-going, it's needed turn off it!
				stateMachine->setUpgradeResult(IMM_Util::FW_NOT_OK);
				stateMachine->turnOffStateMachine();
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " No IPMI ongoing but state machine is started... I'll stop it " << std::endl;
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No IPMI ongoing but state machine ongoing... I'll stop it" , __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
				}

				result = EXECUTED;
				//get upgrade result
				int upgRes = -1;
				std::string ipmiAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT;
				IMM_Util::getImmAttributeInt(dn_pers,ipmiAttr,upgRes);
				if(upgRes == IMM_Util::FW_OK)
				{
					m_ipmiResult = "OK";
				}
				else
				{
					m_ipmiResult = "NOT OK";
				}
				//get upgrade time
				std::string upgTime("");
				std::string ipmiTimeAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_TIME;
				IMM_Util::getImmAttributeString(dn_pers,ipmiTimeAttr,upgTime);
				m_ipmiUpgradeTime = "LAST UPGRADE TIME: " + upgTime;
			}

		}
		else
		{
			result = EXECUTED;
			//get upgrade result
			int upgRes = -1;
			std::string ipmiAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT;
			IMM_Util::getImmAttributeInt(dn_pers,ipmiAttr,upgRes);
			if(upgRes == IMM_Util::FW_OK)
			{
				m_ipmiResult = "OK";
			}
			else
			{
				m_ipmiResult = "NOT OK";
			}

			//get upgrade time
			std::string upgTime("");
			std::string ipmiTimeAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_TIME;
			IMM_Util::getImmAttributeString(dn_pers,ipmiTimeAttr,upgTime);
			m_ipmiUpgradeTime = "LAST UPGRADE TIME: " + upgTime;
		}
	}
	else if (stateMachineOther->getStateName() == "ONGOING")
	{

		if ((slotOngoingOther == (int)uslot))//||(slotOngoingOther == (int)uslot))
		{
			// State Machine ONGOING
			mibStatusOnSCX = stateMachineOther->checkIPMIStatus(tmpIPA, tmpIPB, slot);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
					" mibStatusOnSCX " << mibStatusOnSCX <<
					" tmpIPA " << tmpIPA.c_str() <<
					" tmpIPB " << tmpIPB.c_str() <<
					" slot " << slot << std::endl;
			if ( mibStatusOnSCX == -1)
			{
				result = SNMP_FAILURE;
			}
			else if (mibStatusOnSCX == 1)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IPMI ongoing and state machine ongoing " << std::endl;
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] IPMI ongoing and state machine ongoing" , __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
				}
				result = EXECUTED;
				m_ipmiResult = "ONGOING";
			}
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__;
				if ((uslot != 0) && (uslot != 25))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__;
					// The MIB Status is Idle, but the State Machines are ongoing , it's needed to turn off it!
					//State machine is on-going, it's needed turn off it!
					stateMachineOther->setUpgradeResult(IMM_Util::FW_NOT_OK);
					stateMachineOther->turnOffStateMachine();

					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " No IPMI ongoing but state machine is started... I'll stop it " << std::endl;
					if (_trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No IPMI ongoing but state machine is started..I'll stop it" , __LINE__);
						_trace->ACS_TRA_event(1, tmpStr);
					}

					result = EXECUTED;
					//get upgrade result
					int upgRes = -1;
					std::string ipmiAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT;
					IMM_Util::getImmAttributeInt(dn_pers,ipmiAttr,upgRes);
					if(upgRes == IMM_Util::FW_OK)
					{
						m_ipmiResult = "OK";
					}
					else
					{
						m_ipmiResult = "NOT OK";
					}
					//get upgrade time
					std::string upgTime("");
					std::string ipmiTimeAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_TIME;
					IMM_Util::getImmAttributeString(dn_pers,ipmiTimeAttr,upgTime);
					m_ipmiUpgradeTime = "LAST UPGRADE TIME: " + upgTime;

				}
				else
				{
					tempmibStatusOnOtherSCX = stateMachineOther->checkIPMIStatus(tmpOtherIPA, tmpOtherIPB, slot);
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
							" tempmibStatusOnOtherSCX " << tempmibStatusOnOtherSCX <<
							" tmpOtherIPA " << tmpOtherIPA.c_str() <<
							" tmpOtherIPB " << tmpOtherIPB.c_str() <<
							" slot " << slot << std::endl;
					if(tempmibStatusOnOtherSCX == 1)
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IPMI ongoing and state machine ongoing " << std::endl;
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] IPMI ongoing and state machine ongoing" , __LINE__);
							_trace->ACS_TRA_event(1, tmpStr);
						}
						result = EXECUTED;
						m_ipmiResult = "ONGOING";

					}
					else
					{

						// The MIB Status is Idle, but the State Machines are ongoing , it's needed turn off it!
						//State machine is on-going, it's needed turn off it!
						stateMachineOther->setUpgradeResult(IMM_Util::FW_NOT_OK);
						stateMachineOther->turnOffStateMachine();

						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " No IPMI ongoing but state machine is started... I'll stop it " << std::endl;
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No IPMI ongoing but state machine is started..I'll stop it" , __LINE__);
							_trace->ACS_TRA_event(1, tmpStr);
						}

						result = EXECUTED;
						//get upgrade result
						int upgRes = -1;
						std::string ipmiAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT;
						IMM_Util::getImmAttributeInt(dn_pers,ipmiAttr,upgRes);
						if(upgRes == IMM_Util::FW_OK)
						{
							m_ipmiResult = "OK";
						}
						else
						{
							m_ipmiResult = "NOT OK";
						}
						//get upgrade time
						std::string upgTime("");
						std::string ipmiTimeAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_TIME;
						IMM_Util::getImmAttributeString(dn_pers,ipmiTimeAttr,upgTime);
						m_ipmiUpgradeTime = "LAST UPGRADE TIME: " + upgTime;
					}
				}
			}

		}
		else
		{
			result = EXECUTED;
			//get upgrade result
			int upgRes = -1;
			std::string ipmiAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT;
			IMM_Util::getImmAttributeInt(dn_pers,ipmiAttr,upgRes);
			if(upgRes == IMM_Util::FW_OK)
			{
				m_ipmiResult = "OK";
			}
			else
			{
				m_ipmiResult = "NOT OK";
			}

			//get upgrade time
			std::string upgTime("");
			std::string ipmiTimeAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_TIME;
			IMM_Util::getImmAttributeString(dn_pers,ipmiTimeAttr,upgTime);
			m_ipmiUpgradeTime = "LAST UPGRADE TIME: " + upgTime;
		}
	}
	else
	{
		result = EXECUTED;
		//get upgrade result
		int upgRes = -1;
		std::string ipmiAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_RESULT;
		IMM_Util::getImmAttributeInt(dn_pers,ipmiAttr,upgRes);
		if(upgRes == IMM_Util::FW_OK)
		{
			m_ipmiResult = "OK";
		}
		else
		{
			m_ipmiResult = "NOT OK";
		}

		//get upgrade time
		std::string upgTime("");
		std::string ipmiTimeAttr = IMM_Util::ATT_BLADESWPERSISTANCE_IPMI_TIME;
		IMM_Util::getImmAttributeString(dn_pers,ipmiTimeAttr,upgTime);
		m_ipmiUpgradeTime = "LAST UPGRADE TIME: " + upgTime;
	}

	return result;
}



int FIXS_CCH_UpgradeManager::InitFtpPath(const std::string &strPathIn, const std::string &strShelf, const std::string &dtrSlot, std::string *strPathOut)
{
	int iRet;
	int iPos;
	bool bOk;
	std::string strFileName;
	std::string strShelfLoc;
	std::string strExt;
	std::string strTmp;
	// Initialize
	bOk = false;
	iRet = DIRECTORYERR;
	iPos = -1;
	strFileName.clear();
	strShelfLoc.clear();
	strExt.clear();
	strTmp.clear();
	// Copy shelf in locale
	strShelfLoc = strShelf;
	// Replace "." with "-"
	std::replace(strShelfLoc.begin(), strShelfLoc.end(), CHR_SHELF_SEPARATOR, CHR_IPMI_SHELFSEPARATOR);
	// Extract filename + extension from path
	iPos = strPathIn.find_last_of(CHR_IPMI_DIR_SEPARATOR);
	if(iPos >= 0){
		// Filename is part of input path. Extract it.
		strTmp = strPathIn.substr(iPos +1);
	}else{
		// Only filename is incluse in strPathIn.
		strTmp = strPathIn;
	}
	// Extract extension
	iPos = strTmp.find_first_of(CHR_IPMI_FILE_SEPARATOR);
	if(iPos >=0){
		// Extension is part of the file. Extract it
		strExt = strTmp.substr(iPos +1);
		// then remove from filename
		strTmp = strTmp.substr(0,iPos);
	}else{
		// Extension does not exist. Put a default extension
		strExt = STR_IPMI_DEFAULT_EXT;
	}
	// Create the name file string: [file name]_[shelf]_[slot].[ext]
	strFileName =  strTmp;
	strFileName +=  CHR_IPMI_SEPARATOR;
	strFileName += strShelfLoc;
	strFileName += CHR_IPMI_SEPARATOR;
	strFileName += dtrSlot;
	strFileName += CHR_IPMI_FILE_SEPARATOR;
	strFileName += strExt;
	// Create Ftp area Path
	strTmp  = STR_IPMI_FTP_AREA;
	strTmp += strFileName;

	std::cout << __FUNCTION__ << "@" << __LINE__ << " CopyFile:[" << strPathIn.c_str() << "],[" <<  strTmp.c_str() << "]"<< std::endl;

	// Copy file from strPathIn to ftp area with "strFileName" name.
	bOk = CCH_Util::CopyFile(strPathIn.c_str(), strTmp.c_str(), false, INT_FILE_CPYBUFFER);
	// Check copy
	if(bOk == true)
	{
		// The file has been copied.
		(*strPathOut) = strFileName;
		iRet = EXECUTED;
		std::cout << "File copied" << std::endl;
	}
	else
	{
		std::cout << "Copy file FAILED " << std::endl;
		iRet = INTERNALERR;
	}
	// Create tftp path from output

	// 192.168.169.33:/IPMI_FW/19003-CXC138942_F_ipmi_upg_g_3_9_110518.bin
	// Exit from method
	std::cout <<"*************************************************"<<std::endl;
	std::cout <<"DBG: FTP PATH is - " << strFileName << std::endl;
	std::cout <<"*************************************************"<<std::endl;
	return(iRet);
}


bool FIXS_CCH_UpgradeManager::checkOtherSlotPresence(std::string strBoardMip, std::string slot)
{
	bool res = true;
	unsigned short uslot = -1;
	std::string strEthA0("");
	std::string strEthB0("");
	std::string strEthA25("");
	std::string strEthB25("");

	getUshortSlot(slot, uslot);
	if ((uslot !=0) && (uslot !=25)) return res;

	FIXS_CCH_SNMPManager *snmpReader = NULL;
	snmpReader = new FIXS_CCH_SNMPManager(strBoardMip);
	cout << "strBoardMip: " <<strBoardMip.c_str() <<  endl;
	//check hardware
	int boardPresence = -1;
	if (uslot == 0)
	{
		boardPresence = snmpReader->getBoardPresence(strBoardMip,25);
		cout << "boardPresence : " << boardPresence << "slot: "<< uslot << "strEthA25: " <<strBoardMip.c_str() <<  endl;
		if (boardPresence != 3 ) res = false;
	}
	else if (uslot == 25)
	{
		boardPresence = snmpReader->getBoardPresence(strBoardMip,0);
		cout << "boardPresence : " << boardPresence << "slot: "<< uslot <<  endl;
		if (boardPresence != 3 ) res = false;
	}

	// Release resource
	if(snmpReader != NULL)
	{
		delete(snmpReader);
		snmpReader = NULL;
	}

	return res;
}

int FIXS_CCH_UpgradeManager::ipmiUpgradeStart (std::string magazine, std::string slot, std::string slotMaster, std::string  ipMasterSCX, std::string  ipPassiveSCX, std::string valueToSet)
{

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ipmiUpgrade requested on magazine \"%s\" and slot \"%s\"",__LINE__, magazine.c_str(), slot.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}

	UNUSED(ipPassiveSCX);
	UNUSED(slotMaster);

	int result = HARDWARE_NOT_PRESENT;

	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	unsigned short uMagSlot = 0;
	unsigned short uSlotStateMachine = 0;

	std::string container("");
	std::string product("");
	std::string sSlotStateMachine("");
	std::string masterScxIp("");
	std::string passiveScxIp("");
	std::string slotStr("");
	std::string dn_ipmi("");

	if(isBackupOngoing() == true) return NOT_ALLOWED_BACKUP_ONGOING;

	std::cout << "Path before mod : " << valueToSet << std::endl;

	if(!CCH_Util::PathFileExists(valueToSet.c_str()))
		return DIRECTORYERR;

	if((valueToSet.find(".mot") == string::npos) && (valueToSet.find(".bin") == string::npos))
		return INCORRECT_PACKAGE;

	// convert magazine
	// 	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	if (getUlongMagazine(magazine, umagazine) == false) return result;

	// 	// convert slot
	if (getUshortSlot(slot, uslot) == false) return result;

	//check if hardware exists
	if ( (!isScxData(umagazine, uslot)) && (!isEpbBoard(umagazine,uslot)) && (!isCmxData(umagazine, uslot)) && (!isEvoEtData(umagazine, uslot)) )
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ERROR: these values don't match with board type data: " << umagazine << " " << uslot << std::endl;
		//return result;
		return FUNC_BOARD_NOTALLOWED;
	}

	//get dn of the object to update
	dn_ipmi = IMM_Interface::getDnBladePersistance(umagazine,uslot);

	if(getMasterPassiveSCX(umagazine, masterScxIp, passiveScxIp, uMagSlot) == false) return result;

	//uMagSlot = Master SCX slot
	if(CCH_Util::ushortToString(uMagSlot, slotStr) == false) return result;

	std:: cout << "MASTER SCX : " << masterScxIp << std::endl;

	//Looking for another upgrade if is a shelf manager blade
	if(fwCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,26) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,28) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;
	sSlotStateMachine = slot;
	uSlotStateMachine = uslot;

	//check if other SCX is defined and present
	if (isScxData(umagazine, uslot))
	{
		unsigned short otherSlot = -1;
		bool found = false;

		if (uSlotStateMachine == 0) otherSlot = 25;
		else otherSlot = 0;

		//check if is defined
		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == otherSlot))
			{
				found = true;
				break;
			}
		}
		if (!found) return NEIGHBOR_SCX_NOT_PRESENT;

		//check if is present
		if (!checkOtherSlotPresence(masterScxIp,sSlotStateMachine)) return NEIGHBOR_SCX_NOT_PRESENT;
	}


	// Initialize path
	std::string strFileName("");
	std::string strFtpPath("");
	std::string nodeIP("");
	std::string hostName("");


	if (InitFtpPath(valueToSet,magazine,slot,&strFileName) == 0)
	{
		//get hostname
		hostName = CCH_Util::GetAPHostName();
		cout << "GetAPHostName: " << hostName.c_str() << endl;

		if( slotStr.compare("0") == 0 )
		{
			if (hostName.compare("SC-2-1") == 0) nodeIP = (string)SUB_IP1 + IP1;
			else if (hostName.compare("SC-2-2") == 0) nodeIP = (string)SUB_IP1 + IP2;
			else
			{
				nodeIP = (string)CLUSTER_IP1;
			}
		}

		else if( slotStr.compare("25") == 0 )
		{
			if (hostName.compare("SC-2-1") == 0)	nodeIP = (string)SUB_IP2 + IP1;
			else if (hostName.compare("SC-2-2") == 0) nodeIP = (string)SUB_IP2 + IP2;
			else
			{
				nodeIP = (string)CLUSTER_IP2;
			}
		}

		strFtpPath = nodeIP + ":/boot/" +strFileName;

		cout << "strFtpPath: " << strFtpPath.c_str()  << endl;


		{
			Lock lock;

			for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
			{
				if ((it->m_magazine == umagazine) && (it->m_slot == uMagSlot))
				{
					//Found a slot master, it will be started !
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "FOUND ! set ipMasterSCX: " << ipMasterSCX << std::endl;

					if(it->m_ipmiPtr == 0)
					{
						FIXS_CCH_IpmiUpgrade *ipmiUp = new (std::nothrow) FIXS_CCH_IpmiUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);
						it->m_ipmiPtr = ipmiUp;
					}

					FIXS_CCH_IpmiUpgrade *stateMachine = 0;
					try
					{
						//get state machine
						stateMachine = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
						stateMachine->m_ipMaster = masterScxIp;
						stateMachine->m_slotMaster = slotStr;
						stateMachine->m_slotUpgOnGoing = (int)uslot;
						stateMachine->m_ipPassive = passiveScxIp;
						stateMachine->m_tftpPath = valueToSet;
						stateMachine->setSwBladePersistanceDn(dn_ipmi);
					}
					catch (bad_cast)
					{

						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_ipmiPtr is not a FIXS_CCH_IpmiUpgrade", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						stateMachine = 0;
					}
					if (!stateMachine)
					{
						return INTERNALERR; // Internal Error
					}

					if(!stateMachine->isRunningThread())
					{
						std::cout<<"DBG: "<< __FUNCTION__ <<"@"<< __LINE__ <<" IPMI UPGRADE activating thread... "<< std::endl;
						stateMachine->open();
					}
					sleep(2);

					//start IPMI upgrade
					std::cout<< "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IPMI UPGRADE thread already activated... " << std::endl;
					//result = stateMachine->upgrade(valueToSet, slot, ipMasterSCX);					break;
					result = stateMachine->upgrade(strFtpPath, slot, masterScxIp);
					break;
				}
			} //for
		} //lock
	}
	else
		return INTERNALERR;
	return result;

}

void FIXS_CCH_UpgradeManager::ipmiUpgradeTrap (std::string subrackNumValue, std::string slotPosValue, std::string ipmiUpgradeResult, std::string snmpIP)
{
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] ipmiUpgradeTrap - subrackNumValue: \"%s\"; slotPosValue: \"%s\", ipmiUpgradeResult: \"%s\", snmpIP: \"%s\"", __LINE__, subrackNumValue.c_str(), slotPosValue.c_str(), ipmiUpgradeResult.c_str(), snmpIP.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< "ipmiUpgradeTrap subrackNumValue: " << subrackNumValue.c_str() <<
			" - slotPosValue: " << slotPosValue.c_str() <<
			" - ipmiUpgradeResult: " << ipmiUpgradeResult.c_str() <<
			" - ipmiUpgradeResult: " << snmpIP.c_str() << std::endl;

	unsigned long umagazine = 0;
	int trapRes = 0;
	int res = 0;
	int islot = -1;
	trapRes = atoi(ipmiUpgradeResult.c_str());
	islot = atoi(slotPosValue.c_str());

	FIXS_CCH_IpmiUpgrade *stateMachine = NULL;
	FIXS_CCH_IpmiUpgrade *stateMachine0 = NULL;
	FIXS_CCH_IpmiUpgrade *stateMachine25 = NULL;

	if (checkSubnetIpTrapReceiver(snmpIP, CCH_Util::IpmiUpgrade,false)) return;

	Lock lock;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->getIPEthAString() == snmpIP) || (it->getIPEthBString() == snmpIP))
		{
			umagazine = it->m_magazine;
			break;
		}
	}

	bool found = false;
	bool found0 = false;
	bool found25 = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == umagazine))
		{
			if (it->m_slot == 0)
			{
				stateMachine0 = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
				if (stateMachine0)
					found0 = true;
			}
			else if (it->m_slot == 25)
			{
				stateMachine25 = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
				if (stateMachine25)
					found25 = true;
			}
			else if (it->m_slot == islot )//EPB1
			{
				stateMachine = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
				if (stateMachine)
					found = true;
			}

			//if ( (found0 == true) && (found25 == true))
			if ( (stateMachine0 != NULL) && (stateMachine25 != NULL) && (stateMachine != NULL) )
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " found SM 0 and SM 25 " << std::endl;
				FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager - ipmiUpgradeTrap, NO IPMI SM FOUND !!",LOG_LEVEL_ERROR);
				break;
			}
		}
	}
	string ipmiMachineState = "";
	string ipmiMachineState0 = "";
	string ipmiMachineState25 = "";
	if (found0 == true)
	{
		ipmiMachineState0 = stateMachine0->getStateName();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " found SM 0 - State: "
				<< ipmiMachineState0.c_str() << std::endl;
	}

	if (found25 == true)
	{
		ipmiMachineState25 = stateMachine25->getStateName();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " found SM 25 - State: "
				<< ipmiMachineState25.c_str() << std::endl;
	}

	if (found == true)
	{
		ipmiMachineState = stateMachine->getStateName();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " found SM - State: "
				<< ipmiMachineState.c_str() << std::endl;
	}

	//--------------------------------------------------------------------
	// Managing both "State Machine" we will bypass the change of Master
	//  - In case of an IPMI upgrade on SCX 0 (Master)
	//  - the SCX 25 will take the mastership
	//	- On SCX 0 reboot, SCX 25 (Master) will send the TRAP
	//	The following code will bind the TRAP (got from SCX Master) with the ongoing SCX

	if (ipmiMachineState0 == "ONGOING")
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
				"ipmiMachineState0 ONGOING - trapRes: " << trapRes  << std::endl;
		res = stateMachine0->ipmiUpgradeResultTrap(trapRes);
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ipmiMachineState0 IDLE " << std::endl;
	}

	if (ipmiMachineState25 == "ONGOING")
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
				"ipmiMachineState25 ONGOING - trapRes: " << trapRes  << std::endl;
		res = stateMachine25->ipmiUpgradeResultTrap(trapRes);

	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ipmiMachineState25 IDLE " << std::endl;
	}

	if (ipmiMachineState == "ONGOING")
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
				"ipmiMachineState ONGOING - trapRes: " << trapRes  << std::endl;
		res = stateMachine->ipmiUpgradeResultTrap(trapRes);

	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ipmiMachineState IDLE " << std::endl;
	}

	//clean subnet trap check
	checkSubnetIpTrapReceiver(snmpIP,CCH_Util::IpmiUpgrade,true);

}


int FIXS_CCH_UpgradeManager::startPFMUpgradeFSMs ()
{
	int result  = 0;

	//check environment - EGEM2
	environment = FIXS_CCH_DiskHandler::getEnvironment();
	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA) || (environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
	{
		for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{

			if ((it->m_fbn == CCH_Util::SCXB)||(it->m_fbn == CCH_Util::SMXB))
			{
				if (it->m_pfmPtr == 0)
				{
					FIXS_CCH_PowerFanUpgrade *pfmUp = new (std::nothrow) FIXS_CCH_PowerFanUpgrade(it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);

					if (pfmUp)
					{
						it->m_pfmPtr = pfmUp;
						CCH_Util::PFMData data;

						if (IMM_Interface::readUpgradeStatus(it->m_magazine, it->m_slot, data))
						{
							std::cout << "DBG:   RESTORE PFM STATE MACHINE" << std::endl;

							pfmUp->setUpgradeFanResult(data.upgradeFanResult);
							pfmUp->setUpgradePowerResult(data.upgradePowerResult);

							if (data.upgradeFanStatus == CCH_Util::PFM_ONGOING)
							{
								data.type = CCH_Util::FAN;

								std::cout << "------------------------------------------------------------"<< std::endl;
								std::cout<<" 			PFM UPGRADE CHECK STATUS: 				  "<< std::endl;
								std::cout<<"  type: "<< data.type << " module: " << data.module << std::endl;
								std::cout<<"  FAN status: "<< data.upgradeFanStatus <<  std::endl;
								std::cout<<"  Dn: \n"<< data.dn_pfm.c_str() << std::endl;
								std::cout << "------------------------------------------------------------"<< std::endl;

								pfmUp->restoreStatus(data);

								std::cout << "------------------------------------------------------------"<< std::endl;
								std::cout<<" 			PFM UPGRADE : Activating thread... 				  "<< std::endl;
								std::cout << "------------------------------------------------------------"<< std::endl;

								pfmUp->open();

							}
							else if (data.upgradePowerStatus == CCH_Util::PFM_ONGOING)
							{
								data.type = CCH_Util::POWER;


								std::cout << "------------------------------------------------------------"<< std::endl;
								std::cout<<" 			PFM UPGRADE CHECK STATUS: 				  "<< std::endl;
								std::cout<<"  type: "<< data.type << " module: " << data.module << std::endl;
								std::cout<<"  POWER status: "<< data.upgradePowerStatus <<  std::endl;
								std::cout<<"  Dn: \n"<< data.dn_pfm.c_str() << std::endl;
								std::cout << "------------------------------------------------------------"<< std::endl;

								pfmUp->restoreStatus(data);

								std::cout << "------------------------------------------------------------"<< std::endl;
								std::cout<<" 			PFM UPGRADE : Activating thread... 				  "<< std::endl;
								std::cout << "------------------------------------------------------------"<< std::endl;

								pfmUp->open();
							}
						}
					}
					else
					{

						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory Error", __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						result = -1;
					}
				}
			} //if fbn
		}
	}
	return result;
}

int FIXS_CCH_UpgradeManager::pfmUpgrade (std::string completePathFwPar, std::string magazine, std::string pfmTypeIndex, std::string pfmInstanceIndex, std::string slot)
{
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] pfmUpgrade requested on magazine \"%s\" and pfmTypeIndex \"%s\", pfmInstanceIndex: \"%s\", slot : \"%s\"",
				__LINE__, magazine.c_str(), pfmTypeIndex.c_str(), pfmInstanceIndex.c_str(), slot.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
	}

	int result = HARDWARE_NOT_PRESENT; //HARDWARE_NOT_PRESENT

	if(isBackupOngoing() == true) return NOT_ALLOWED_BACKUP_ONGOING;

	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	// convert magazine
	if (getUlongMagazine(magazine, umagazine) == false) return result;

	// convert slot
	if (getUshortSlot(slot, uslot) == false) return result;

	if(fwCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	
	// Initialize path
	std::string strFileName("");
	std::string strFtpPath("");
	std::string nodeIP("");
	std::string hostName("");
	std::string shFilename("");
	std::string xmlFilename("");
	std::string tempXmlDestFolder("");
	
	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
	{
		if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;
		result = InitFtpPath(completePathFwPar,magazine,slot,&strFileName);	
		if(result!=0)
			return result;
	}
	else{
		result = unZipPackage(completePathFwPar,shFilename,xmlFilename,magazine,slot);	
		if(result==0){
		std::string tftpRoot = FIXS_CCH_DiskHandler::getTftpRootFolderDefault();
		std::string tempStr = "/";
		strFileName = tftpRoot + tempStr + shFilename;
		tempXmlDestFolder = tftpRoot + tempStr + xmlFilename;
		cout << "strFileName: " << strFileName.c_str()  << endl;
		cout << "tempXmlDestFolder: " << tempXmlDestFolder.c_str()  << endl;
		}
		else 
			return result;
	}
		
	
	//get hostname
	
		hostName = CCH_Util::GetAPHostName();
		cout << "GetAPHostName: " << hostName.c_str() << endl;

		if( slot.compare("0") == 0 )
		{
			if (hostName.compare("SC-2-1") == 0) nodeIP = (string)SUB_IP1 + IP1;
			else if (hostName.compare("SC-2-2") == 0) nodeIP = (string)SUB_IP1 + IP2;
			else
			{
				nodeIP = (string)CLUSTER_IP1;
			}
		}

		else if( slot.compare("25") == 0 )
		{
			if (hostName.compare("SC-2-1") == 0)	nodeIP = (string)SUB_IP2 + IP1;
			else if (hostName.compare("SC-2-2") == 0) nodeIP = (string)SUB_IP2 + IP2;
			else
			{
				nodeIP = (string)CLUSTER_IP2;
			}
		}

		if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
		{
			strFtpPath = nodeIP + ":/boot/" +strFileName;
		}
		else
			strFtpPath = nodeIP + ":" +strFileName;	

		cout << "strFtpPath: " << strFtpPath.c_str()  << endl;

		//call state machine method
		{
			Lock lock;

			FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
			FIXS_CCH_PowerFanUpgrade *otherStateMachine = 0;
			unsigned short otherSlot = (uslot == 0 ? 25 : 0);

			for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
			{
				if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
				{
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
					}
					catch (bad_cast)
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}
						stateMachine = 0;
					}
					break;
				}
			}

			for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
			{
				if ((it->m_magazine == umagazine) && (it->m_slot == otherSlot))
				{
					try
					{
						otherStateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
					}
					catch (bad_cast)
					{

						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}
						otherStateMachine = 0;
					}
					break;
				}
			}


			if (stateMachine && otherStateMachine)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++++++++SET SCANNING OFF ONTO THE OTHER STATE MACHINE++++++++++++"<< std::endl;
				if (otherStateMachine->setScanningOff() == false)
				{
					//delete the pfm package from tftp and nbi folder
					CCH_Util::deleteFile(strFileName.c_str());
					CCH_Util::deleteFile(tempXmlDestFolder.c_str());
					CCH_Util::deleteFile(completePathFwPar.c_str());
					return INTERNALERR;
				}
			}


			if (stateMachine)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++++++++THE UPGRADE ON THE STATE MACHINE OF THE SMX/SCX: "<< slot <<" IN STARTED+++++++++++"<< std::endl;
				stateMachine->open();

				result = stateMachine->upgrade(strFtpPath, pfmTypeIndex, pfmInstanceIndex, slot);
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++++++++THE UPGRADE SET ON THE STATE MACHINE OF THE SMX: "<< slot <<" HAS BEEN COMPLETED WHIT THIS RESULT:+++++++++++" << result << std::endl;
				if (result && otherStateMachine)
				{
					// failure
					otherStateMachine->setScanningOn();
				}

			}
			else
			{
				result = 14;
			}
		}//lock
		//delete the pfm package from tftp and nbi folder in case of failure cases
		if( result != EXECUTED )
		{
			CCH_Util::deleteFile(strFileName.c_str());
			CCH_Util::deleteFile(tempXmlDestFolder.c_str());
			CCH_Util::deleteFile(completePathFwPar.c_str());
		}
		else
			CCH_Util::deleteFile(tempXmlDestFolder.c_str());
	
	return result;
}

void FIXS_CCH_UpgradeManager::pfmUpgradeResult (int pfmUpgradeResult_, std::string snmpIP)
{

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] pfmUpgradeResult TRAP from snmpIP \"%s\" with result %d",__LINE__, snmpIP.c_str(), pfmUpgradeResult_);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}

	if (checkSubnetIpTrapReceiver(snmpIP, CCH_Util::PfmUpgrade,false)) return;

	Lock lock;

	FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
	FIXS_CCH_PowerFanUpgrade *otherStateMachine = 0;
	unsigned short otherSlot = 0;
	unsigned long magazine = 0;

	//state machine
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->getIPEthAString() == snmpIP) || (it->getIPEthBString() == snmpIP))
		{
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
			}
			catch (bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);

					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				stateMachine = 0;
			}
			otherSlot = (it->m_slot == 0 ? 25 : 0);
			magazine = it->m_magazine;
			break;
		}
	}

	//other state machine
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == otherSlot))
		{
			otherStateMachine = 0;
			try
			{
				otherStateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
			}
			catch (bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				otherStateMachine = 0;
			}
			break;
		}
	}

	if (stateMachine && otherStateMachine)
	{

		if (stateMachine->pfmFwUpgradeResultTrap(pfmUpgradeResult_) == 0)
		{
			std::cout<<"++++++++++++++++++++++++++UPGRADE OK++++++++++++++++++++++++++++++++++"<<std::endl;
			otherStateMachine->setScanningOn();

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PFM UPGRADE OK", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			}

		}
		else
		{
			std::cout<<"++++++++++++++++++++++++++UPGRADE FAILED++++++++++++++++++++++++++++++++++"<<std::endl;
			otherStateMachine->setScanningOn();

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] PFM UPGRADE NOT OK", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
		}
	}

	//clean subnet trap check
	checkSubnetIpTrapReceiver(snmpIP,CCH_Util::PfmUpgrade,true);

}

bool FIXS_CCH_UpgradeManager::pfmCheckOtherSlotStatus (unsigned long umagazine, unsigned short uslot)
{
	bool retValue = true;
	bool found = false;
	unsigned short OtherUslot = 0;

	if (uslot == 0)
		OtherUslot = 25;

	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == OtherUslot))
			{
				found = true;
				if  (it->m_pfmPtr == 0)
				{
					return true;
				}

				FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
				}
				catch(bad_cast)
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}
					stateMachine = 0;
				}


				if ((stateMachine) && (stateMachine->getStateName() != "IDLE"))
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
					retValue = false;
				}

				break;
			}
		}
	}
	return retValue;

}

int FIXS_CCH_UpgradeManager::pfmStatus (std::string magazine, std::string type, std::string inst)
{
	ACS_APGCC::toUpper(type);
	ACS_APGCC::toUpper(inst);
	unsigned long umagazine = 0;
	int result = 0;
	bool found = false;
	int environement = -1;
	std::string statusStr="";
	environment = FIXS_CCH_DiskHandler::getEnvironment();
	if((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
	{
		statusStr = "SW UPGRADE STATUS: ";
	}
	else
		statusStr = "FW UPGRADE STATUS: ";
	 
	std::string lowerPowerStat("");
	std::string lowerFanStat("");
	std::string upperPowerStat("");
	std::string upperFanStat("");
	std::string dn("");
	//	 if (getUlongMagazine(magazine, umagazine) == false) return result;
	CCH_Util::stringToUlongMagazine(magazine,umagazine);

	std::cout << "DBG:!! " << __FUNCTION__ << "@" << __LINE__ << " magazine string ==" << magazine.c_str() << " , umagazine == " << umagazine << std::endl;

	std::cout << "DBG:!! " << __FUNCTION__ << "@" << __LINE__ << " retrieve info about magazine and slot 0" << std::endl;
	//retrieve info about magazine and slot 0
	{
		unsigned short uslot = 0;
		//Lock lock;
		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{

			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				found = true;
				std::cout << "DBG:!! " << __FUNCTION__ << "@" << __LINE__ << ", SCX/SMX FOUND!!" << std::endl;
				FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
				}
				catch (bad_cast)
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}
				if (stateMachine == 0) return INTERNALERR;

				m_currentState = stateMachine->getStateName();

				std::cout << "Current State : " << m_currentState <<std::endl;

				if (m_currentState == "ONGOING")
				{
					std::string typeSlot0 = stateMachine->getType();
					if (typeSlot0 == "0")//FAN
					{
						lowerPowerStat = stateMachine->getPowerResultState();

						std::cout << "DBG:!! " << __FUNCTION__ << "@" << __LINE__;
						std::cout << " slot: " << it->m_slot << ", magazine: " << it->m_magazine << std::endl;
					
						if(type.empty())
						{
							m_lowerFan = statusStr + "ONGOING";
							m_lowerPower = statusStr + lowerPowerStat;
						}
						//Reusing same code as power and fan enum are same as TYPEA an TYPEB and since type parameter will be either modType or upgtype
						else if ((type == "POWER")||(type == "TYPEB"))
						{
							if(inst == "LOWER")
							{
								m_lowerPower = statusStr + lowerPowerStat;
							}
						}
						else if ((type == "FAN")||(type == "TYPEA"))
						{
							if(inst == "LOWER")
							{
								m_lowerFan = statusStr + "ONGOING";
							}
						}
						else return INCORRECTUSAGE;
					}
					else //POWER
					{
						lowerFanStat = stateMachine->getFanResultState();

						std::cout << "DBG:!! " << __FUNCTION__ << "@" << __LINE__;
						std::cout << " slot: " << it->m_slot << ", magazine: " << it->m_magazine << std::endl;
						if(type.empty())
						{
							m_lowerFan = statusStr + lowerFanStat;
							m_lowerPower = statusStr + "ONGOING";
						}
						else if ((type == "POWER")||(type == "TYPEB"))
						{
							if(inst == "LOWER")
							{
								m_lowerPower = statusStr + "ONGOING";
							}
						}
						else if ((type == "FAN")||(type == "TYPEA"))
						{
							if(inst == "LOWER")
							{
								m_lowerFan = statusStr + lowerFanStat;
							}
						}
						else return INCORRECTUSAGE;
					}
				}
				else //IDLE
				{

					lowerFanStat = stateMachine->getFanResultState();
					lowerPowerStat = stateMachine->getPowerResultState();

					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__;
					std::cout << " slot: " << it->m_slot << ", magazine: " << it->m_magazine << std::endl;
					
					if(type.empty())
					{
						m_lowerFan = statusStr + lowerFanStat;
						m_lowerPower = statusStr + lowerPowerStat;
					}
					else if ((type == "POWER")||(type == "TYPEB"))
					{
						if(inst == "LOWER")
						{
							m_lowerPower = statusStr + lowerPowerStat;
						}
					}
					else if ((type == "FAN")||(type == "TYPEA"))
					{
						if(inst == "LOWER")
						{
							m_lowerFan = statusStr + lowerFanStat;
						}
					}
					else return INCORRECTUSAGE;

				}
				break;
			}
		}
	}

	//retrieve info about magazine and slot 25
	std::cout << "DBG:!! " << __FUNCTION__ << "@" << __LINE__ << " retrieve info about magazine and slot 25" << std::endl;
	{
		unsigned short uslot = 25;
		//Lock lock;
		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				found = true;
				std::cout << "DBG:!! " << __FUNCTION__ << "@" << __LINE__ << ", SCX FOUND!!" << std::endl;
				FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
				}
				catch(bad_cast)
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);

					}
					stateMachine = 0;
				}
				if (stateMachine == 0) return INTERNALERR;

				m_currentState = stateMachine->getStateName();
				if (m_currentState == "ONGOING")
				{
					std::string typeSlot25 = stateMachine->getType();
					if (typeSlot25 == "0")
					{
						upperPowerStat = stateMachine->getPowerResultState();

						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__;
						std::cout << " slot: " << it->m_slot << ", magazine: " << it->m_magazine << std::endl;
						if(type.empty())
						{
							m_upperFan = statusStr + "ONGOING";
							m_upperPower = statusStr + upperPowerStat;
						}
						else if((type == "POWER")||(type == "TYPEB"))
						{
							if(inst == "UPPER")
							{
								m_upperPower = statusStr + upperPowerStat;
							}
						}
						else if ((type == "FAN")||(type == "TYPEA"))
						{
							if(inst == "UPPER")
							{
								m_upperFan = statusStr + "ONGOING";
							}
						}
						else return INCORRECTUSAGE;
					}
					else
					{
						upperFanStat = stateMachine->getFanResultState();

						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__;
						std::cout << " slot: " << it->m_slot << ", magazine: " << it->m_magazine << std::endl;
						if(type.empty())
						{
							m_upperFan = statusStr + upperFanStat;
							m_upperPower = statusStr + "ONGOING";
						}
						else if((type == "POWER")||(type == "TYPEB"))
						{
							if(inst == "UPPER")
							{
								m_upperPower = statusStr + "ONGOING";
							}
						}
						else if ((type == "FAN")||(type == "TYPEA"))
						{
							if(inst == "UPPER")
							{
								m_upperFan = statusStr + upperFanStat;
							}
						}
						else return INCORRECTUSAGE;
					}
				}
				else
				{
					upperFanStat = stateMachine->getFanResultState();
					upperPowerStat = stateMachine->getPowerResultState();

					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__;
					std::cout << " slot: " << it->m_slot << ", magazine: " << it->m_magazine << std::endl;
					if(type.empty())
					{
						m_upperFan = statusStr + upperFanStat;
						m_upperPower = statusStr + upperPowerStat;
					}
					else if((type == "POWER")||(type == "TYPEB"))
					{
						if(inst == "UPPER")
						{
							m_upperPower = statusStr + upperPowerStat;
						}
					}
					else if ((type == "FAN")||(type == "TYPEA"))
					{
						if(inst == "UPPER")
						{
							m_upperFan = statusStr + upperFanStat;
						}
					}
					else return INCORRECTUSAGE;
				}
				break;
			}
		}
	}
	if (!found) return HARDWARE_NOT_PRESENT;
	return result;

}

int FIXS_CCH_UpgradeManager::pfmDisplay (std::string magazine, std::string type, std::string inst)
{
	int result = HARDWARE_NOT_PRESENT;

	std::string lowerFanFwRevision("");
	std::string lowerPowerFwRevision("");
	std::string lowerModuleType("");
	std::string upperFanFwRevision("");
	std::string upperPowerFwRevision("");
	std::string upperModuleType("");
	std::string lowerTypeASwRevision("");
	std::string upperTypeASwRevision("");
	std::string lowerTypeBSwRevision("");
	std::string upperTypeBSwRevision("");
	int curSwType=-1;
	unsigned long umagazine = 0;
	bool isSmx=false;
	int environment=-1;

	bool found = false;
	if (getUlongMagazine(magazine, umagazine) == false) return result;

	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return WAIT_FOR_UPGRADE_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return WAIT_FOR_UPGRADE_ONGOING;
	environment = FIXS_CCH_DiskHandler::getEnvironment();
		
	if((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
	{
		isSmx=true;
	}
	//retrieve info about magazine and slot 0
	{
		//Lock lock;
		unsigned short uslot = 0;
		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				found = true;
				FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
				}
				catch (bad_cast)
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}
				if (stateMachine == 0) return INTERNALERR;
				ACS_APGCC::toUpper(type);
				ACS_APGCC::toUpper(inst);
				
				if(type.empty())
				{	
					if(isSmx)
					{
						lowerTypeASwRevision = stateMachine->getCurrentRunningPFMSoftware(CCH_Util::typeA);
						lowerTypeBSwRevision = stateMachine->getCurrentRunningPFMSoftware(CCH_Util::typeB);
						curSwType=stateMachine->getCurrentRunningPFMSoftwareType();
						if(curSwType==CCH_Util::UPG)
						{
							m_lowerTypeASwRevision = "Loaded SW Revision = " + lowerTypeASwRevision;
							m_lowerTypeBSwRevision = "Running SW Revision = " + lowerTypeBSwRevision;
						}
						else//BL
						{
							m_lowerTypeASwRevision = "Running SW Revision = " + lowerTypeASwRevision;
							m_lowerTypeBSwRevision = "Loaded SW Revision = " + lowerTypeBSwRevision;
						}
						lowerModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
						m_lowerModuleType = "SW module device type = " + lowerModuleType;
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_lowerModuleType = " << m_lowerModuleType << std::endl;										
					}
					else
					{
						lowerFanFwRevision = stateMachine->getCurrentRunningFirmware(CCH_Util::FAN);
						lowerPowerFwRevision = stateMachine->getCurrentRunningFirmware(CCH_Util::POWER);
						
						m_lowerFanFwRevision = "Running FW Revision = " + lowerFanFwRevision;
						m_lowerPowerFwRevision = "Running FW Revision = " + lowerPowerFwRevision;						
						lowerModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
						m_lowerModuleType = "FW module device type = " + lowerModuleType;
					}
					
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_lowerModuleType = " << m_lowerModuleType << std::endl;										
				}
				else if ((type == "POWER")|| (type == "TYPEB"))
				{
					if(inst == "LOWER")
					{
						if(isSmx)
						{							
							lowerTypeBSwRevision = stateMachine->getCurrentRunningPFMSoftware(CCH_Util::typeB);
							curSwType=stateMachine->getCurrentRunningPFMSoftwareType();
							if(curSwType==CCH_Util::UPG)
							{
								m_lowerTypeBSwRevision = "Running SW Revision = " + lowerTypeBSwRevision;
							}
							else
							{
								m_lowerTypeBSwRevision = "Loaded SW Revision = " + lowerTypeBSwRevision;
							}
							lowerModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
							m_lowerModuleType = "SW module device type = " + lowerModuleType;
						}
						else
						{
							lowerPowerFwRevision = stateMachine->getCurrentRunningFirmware(CCH_Util::POWER);
							m_lowerPowerFwRevision = "Running FW Revision = " + lowerPowerFwRevision;
							lowerModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
							m_lowerModuleType = "FW module device type = " + lowerModuleType;
						}
						
						
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_lowerModuleType = " << m_lowerModuleType << std::endl;																
					}
					else if (inst == "UPPER")
					{
						//in next segment
					}
					else return INCORRECTUSAGE;
				}
				else if ((type == "FAN")|| (type == "TYPEA"))
				{
					if(inst == "LOWER")
					{
						if(isSmx)
						{							
							lowerTypeASwRevision = stateMachine->getCurrentRunningPFMSoftware(CCH_Util::typeA);
							curSwType=stateMachine->getCurrentRunningPFMSoftwareType();
							if(curSwType==CCH_Util::UPG)
							{
								m_lowerTypeASwRevision = "Loaded SW Revision = " + lowerTypeASwRevision;
							}
							else
							{
								m_lowerTypeASwRevision = "Running SW Revision = " + lowerTypeASwRevision;
							}
							lowerModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
							m_lowerModuleType = "SW module device type = " + lowerModuleType;
							
						}
						else
						{
							lowerFanFwRevision = stateMachine->getCurrentRunningFirmware(CCH_Util::FAN);
							m_lowerFanFwRevision = "Running FW Revision = " + lowerFanFwRevision;
							lowerModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
							m_lowerModuleType = "FW module device type = " + lowerModuleType;
						}
							
							std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_lowerModuleType = " << m_lowerModuleType << std::endl;										
							
					}
					else if (inst == "UPPER")
					{
						//in next segment
					}
					else return INCORRECTUSAGE;
				}
				else return INCORRECTUSAGE;
			}
		}
	}
	
	//retrieve info about magazine and slot 25

	{
		//Lock lock;

		unsigned short uslot = 25;
		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				found = true;
				FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
				}
				catch (bad_cast)
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);

					}
					stateMachine = 0;
				}
				if (stateMachine == 0) return INTERNALERR;
				if(type.empty())
				{
					if(isSmx)
					{
						upperTypeASwRevision = stateMachine->getCurrentRunningPFMSoftware(CCH_Util::typeA);
						upperTypeBSwRevision = stateMachine->getCurrentRunningPFMSoftware(CCH_Util::typeB);
						curSwType=stateMachine->getCurrentRunningPFMSoftwareType();
						if(curSwType==CCH_Util::UPG)
						{
							m_upperTypeASwRevision = "Loaded SW Revision = " + upperTypeASwRevision;
							m_upperTypeBSwRevision = "Running SW Revision = " + upperTypeBSwRevision;
						}
						else
						{
							m_upperTypeASwRevision = "Running SW Revision = " + upperTypeASwRevision;
							m_upperTypeBSwRevision = "Loaded SW Revision = " + upperTypeBSwRevision;
						}
						upperModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
						m_upperModuleType = "SW module device type = " + upperModuleType;
					}
					else
					{
						upperFanFwRevision = stateMachine->getCurrentRunningFirmware(CCH_Util::FAN);
						upperPowerFwRevision = stateMachine->getCurrentRunningFirmware(CCH_Util::POWER);					

						m_upperFanFwRevision = "Running FW Revision = " + upperFanFwRevision;
						m_upperPowerFwRevision = "Running FW Revision = " + upperPowerFwRevision;
						upperModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
						m_upperModuleType = "FW module device type = " + upperModuleType;	
					}
					

					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_upperModuleType = " << m_upperModuleType << std::endl;
				
				}
				else if((type == "POWER") || (type == "TYPEB"))
				{
					if(inst == "LOWER")
					{
						// prev section
					}
					else if (inst == "UPPER")
					{
						if(isSmx)
						{							
							upperTypeBSwRevision = stateMachine->getCurrentRunningPFMSoftware(CCH_Util::typeB);
							curSwType=stateMachine->getCurrentRunningPFMSoftwareType();
							if(curSwType==CCH_Util::UPG)
							{
								m_upperTypeBSwRevision = "Running SW Revision = " + upperTypeBSwRevision;
							}
							else
							{
								m_upperTypeBSwRevision = "Loaded SW Revision = " + upperTypeBSwRevision;
							}
							upperModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
							m_upperModuleType = "SW module device type = " + upperModuleType;
						}
						else
						{
							upperPowerFwRevision = stateMachine->getCurrentRunningFirmware(CCH_Util::POWER);
							
							m_upperPowerFwRevision = "Running FW Revision = " + upperPowerFwRevision;							
							upperModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);
							m_upperModuleType = "FW module device type = " + upperModuleType;
						}
						
						
					}
					else return INCORRECTUSAGE;
				}

				else if((type == "FAN")|| (type == "TYPEA"))
					{
						if(inst == "LOWER")
						{
							// prev section
						}
						else if (inst == "UPPER")
						{
							if(isSmx)
							{							
								upperTypeASwRevision = stateMachine->getCurrentRunningPFMSoftware(CCH_Util::typeA);
								curSwType=stateMachine->getCurrentRunningPFMSoftwareType();
								if(curSwType==CCH_Util::UPG)
								{
									m_upperTypeASwRevision = "Loaded SW Revision = " + upperTypeASwRevision;
								}
								else
								{
									m_upperTypeASwRevision = "Running SW Revision = " + upperTypeASwRevision;
								}
								upperModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);							
								m_upperModuleType = "SW module device type = " + upperModuleType;
							}
							else
							{
								upperFanFwRevision = stateMachine->getCurrentRunningFirmware(CCH_Util::FAN);
								
								m_upperFanFwRevision = "Running FW Revision = " + upperFanFwRevision;
								upperModuleType = stateMachine->getCurrentModuleDeviceType(isSmx);							
								m_upperModuleType = "FW module device type = " + upperModuleType;
							}
							
						}
						else return INCORRECTUSAGE;
					}
					else return INCORRECTUSAGE;			
			}
		}
	}
	if (!found)		result = HARDWARE_NOT_PRESENT;
	if(isSmx)
	{
		std::cout <<"VALUES:"<<m_lowerTypeASwRevision<<" "<<m_lowerTypeBSwRevision<<" "<< m_upperTypeASwRevision<<" "<<m_upperTypeBSwRevision<<std::endl;
		if (lowerTypeASwRevision.empty() && lowerTypeBSwRevision.empty() && upperTypeASwRevision.empty() && upperTypeBSwRevision.empty())
		{
			result = SNMP_FAILURE;
		}
		else result = EXECUTED;
	}
	else{	
		std::cout <<"VALUES:"<<m_lowerFanFwRevision<<" "<<m_lowerPowerFwRevision<<" "<< m_upperFanFwRevision<<" "<<m_upperPowerFwRevision<<std::endl;
		if (lowerFanFwRevision.empty() && lowerPowerFwRevision.empty() && upperFanFwRevision.empty() && upperPowerFwRevision.empty())
		{
			result = SNMP_FAILURE;
		}
		else result = EXECUTED;
	}

	return result;
}


int FIXS_CCH_UpgradeManager::writePfmStatus (char* buffer, int bufferSize)
{
	
	char *tmpBuf = 0;
	int nItem = 10;
	int environment=-1;
	environment = FIXS_CCH_DiskHandler::getEnvironment();
	
	if((m_upperFan.empty()) || (m_upperPower.empty()) || (m_lowerFan.empty()) || (m_lowerPower.empty()))
		nItem = 4;
	int respSize = sizeof(nItem) + (ITEM_SIZE * nItem);
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	if (respSize > bufferSize)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
		return -1;
	}
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	memcpy(buffer, &nItem, sizeof(nItem)); // write number of items, in this case always 1
	tmpBuf = buffer + sizeof(nItem);


	//write headerstring
	snprintf(tmpBuf, ITEM_SIZE, "%-20s", " ");   //to be defined
	tmpBuf = tmpBuf + ITEM_SIZE;

	char row[ITEM_SIZE] = {0};
	
	if((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
	{
		snprintf(row, sizeof(row) - 1, "%-20s\n", "PFM SOFTWARE UPGRADE PROGRESS REPORT");
	}
	else
		snprintf(row, sizeof(row) - 1, "%-20s\n", "PFM FIRMWARE UPGRADE PROGRESS REPORT");
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
	snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	tmpBuf = tmpBuf + ITEM_SIZE;

	if(!m_upperFan.empty())
	{
		if((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
		{
			snprintf(row, sizeof(row) - 1, "%-20s", "UPPER TYPEA MODULE");
		}
		else{
			snprintf(row, sizeof(row) - 1, "%-20s", "UPPER FAN MODULE");
		}
		
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;


		snprintf(row, sizeof(row) - 1, "%-20s\n", m_upperFan.c_str());
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;
	}

	if(!m_upperPower.empty())
	{
		if((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
		{
			snprintf(row, sizeof(row) - 1, "%-20s", "UPPER TYPEB MODULE");
		}
		else{
			snprintf(row, sizeof(row) - 1, "%-20s", "UPPER POWER MODULE");
		}
		
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;


		snprintf(row, sizeof(row) - 1, "%-20s\n", m_upperPower.c_str());
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;
	}

	if(!m_lowerFan.empty())
	{
		if((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
		{
			snprintf(row, sizeof(row) - 1, "%-20s", "LOWER TYPEA MODULE");
		}
		else{
			snprintf(row, sizeof(row) - 1, "%-20s", "LOWER FAN MODULE");
		}
		
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;

		snprintf(row, sizeof(row) - 1, "%-20s\n", m_lowerFan.c_str());
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;
	}

	if(!m_lowerPower.empty())
	{
		if((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
		{
			snprintf(row, sizeof(row) - 1, "%-20s", "LOWER TYPEB MODULE");
		}
		else{
			snprintf(row, sizeof(row) - 1, "%-20s", "LOWER POWER MODULE");
		}
		
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;


		snprintf(row, sizeof(row) - 1, "%-20s\n", m_lowerPower.c_str());
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;
	}

	//variables clean
	m_upperFan = "" ;
	m_upperPower = "";
	m_lowerFan = "";
	m_lowerPower = "";

	return respSize;

}

int FIXS_CCH_UpgradeManager::writePfmDisplay (char* buffer, int bufferSize)
{
	char *tmpBuf = 0;
	int nItem = 13;
	if((m_upperFanFwRevision.empty()) || (m_upperPowerFwRevision.empty()) || (m_lowerFanFwRevision.empty()) || (m_lowerPowerFwRevision.empty()))
		nItem = 4;
	int respSize = sizeof(nItem) + (ITEM_SIZE * nItem);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	if (respSize > bufferSize)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
		return -1;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;

	//write Header
	memcpy(buffer, &nItem, sizeof(nItem)); // write number of items, in this case always 1

	// write Item 1 - //write headerstring
	tmpBuf = buffer + sizeof(nItem);
	snprintf(tmpBuf, ITEM_SIZE - 1, "%-20s", " ");   //to be defined
	char row[ITEM_SIZE] = {0};

	// write Item 2
	if(!m_upperFanFwRevision.empty())
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", "UPPER FAN MODULE");
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE - 1, "%s", row);

		// write Item 3
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", m_upperFanFwRevision.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 4
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_upperModuleType.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	}


	// write Item 5
	if(!m_upperPowerFwRevision.empty())
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", "UPPER POWER MODULE");
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);


		// write Item 6
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", m_upperPowerFwRevision.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 7
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_upperModuleType.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	}

	// write Item 8
	if(!m_lowerFanFwRevision.empty())
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", "LOWER FAN MODULE");
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 9
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", m_lowerFanFwRevision.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 10
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_lowerModuleType.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	}

	// write Item 11
	if(!m_lowerPowerFwRevision.empty())
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", "LOWER POWER MODULE");
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 12
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", m_lowerPowerFwRevision.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 13
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_lowerModuleType.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	}

	//variable clean
	m_upperFanFwRevision = "";
	m_upperPowerFwRevision = "";
	m_upperModuleType = "";
	m_lowerFanFwRevision = "";
	m_lowerPowerFwRevision = "";
	m_lowerModuleType = "";

	return respSize;

}
int FIXS_CCH_UpgradeManager::writePfmSwDisplay (char* buffer, int bufferSize)
{
	char *tmpBuf = 0;
	int nItem = 13;
	if((m_upperTypeASwRevision.empty()) || (m_upperTypeBSwRevision.empty()) || (m_lowerTypeASwRevision.empty()) || (m_lowerTypeBSwRevision.empty()))
		nItem = 4;
	int respSize = sizeof(nItem) + (ITEM_SIZE * nItem);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	if (respSize > bufferSize)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
		return -1;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;

	//write Header
	memcpy(buffer, &nItem, sizeof(nItem)); // write number of items, in this case always 1

	// write Item 1 - //write headerstring
	tmpBuf = buffer + sizeof(nItem);
	snprintf(tmpBuf, ITEM_SIZE - 1, "%-20s", " ");   //to be defined
	char row[ITEM_SIZE] = {0};

	// write Item 2

	if(!m_upperTypeASwRevision.empty())
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", "UPPER TYPEA MODULE");
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE - 1, "%s", row);

		// write Item 3
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", m_upperTypeASwRevision.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 4
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_upperModuleType.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	}


	// write Item 5
	if(!m_upperTypeBSwRevision.empty())
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", "UPPER TYPEB MODULE");
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);


		// write Item 6
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", m_upperTypeBSwRevision.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 7
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_upperModuleType.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	}

	// write Item 8
	if(!m_lowerTypeASwRevision.empty())
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", "LOWER TYPEA MODULE");
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 9
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", m_lowerTypeASwRevision.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 10
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_lowerModuleType.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	}

	// write Item 11
	if(!m_lowerTypeBSwRevision.empty())
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", "LOWER TYPEB MODULE");
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 12
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s", m_lowerTypeBSwRevision.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		// write Item 13
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_lowerModuleType.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	}
	//variable clean
	
	m_upperTypeASwRevision = "";
	m_upperTypeBSwRevision = "";
	m_upperModuleType = "";
	m_lowerTypeASwRevision = "";
	m_lowerTypeBSwRevision = "";
	m_lowerModuleType = "";

	return respSize;

}

void FIXS_CCH_UpgradeManager::setScanning (unsigned long magazine, unsigned short slot, bool on)
{

	FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
			}
			catch(bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				stateMachine = 0;
			}

			if (stateMachine == 0) return;

			if (on)
			{
				stateMachine->setScanningOn();
			}
			else
			{
				stateMachine->setScanningOff();
			}
			break;
		}
	}

}

int FIXS_CCH_UpgradeManager::startIpmiUpgradeFSMs ()
{
	int result = 0;
	bool isPassiveDeg = false;

	//check environment - EGEM2
	environment = FIXS_CCH_DiskHandler::getEnvironment();
	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
	{
		Lock lock;	//initialization
		FIXS_CCH_IpmiUpgrade *ipmiUp = NULL;

		for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if (it->m_fbn == CCH_Util::SCXB)
			{
				// Set state machine
				ipmiUp = reinterpret_cast<FIXS_CCH_IpmiUpgrade*>(it->m_ipmiPtr);
				//There is an ipmi upgrade for each magazine.
				if (ipmiUp == NULL)
				{
					ipmiUp = new (std::nothrow) FIXS_CCH_IpmiUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);

					if(ipmiUp)
					{
						it->m_ipmiPtr = ipmiUp;

						CCH_Util::IpmiData data;

						unsigned short uMagSlot = 0;
						std::string masterScxIp("");
						std::string passiveScxIp("");
						std::string sSlot("");

						if(getMasterPassiveSCX(it->m_magazine, masterScxIp, passiveScxIp, uMagSlot))
						{
							if (it->m_slot == uMagSlot)
							{
								if (IMM_Interface::readUpgradeStatus(it->m_magazine, it->m_slot, data))
								{
									data.ipMasterSCX = masterScxIp;
									data.ipPassiveSCX = passiveScxIp;
									data.path = "";
									CCH_Util::ushortToString(uMagSlot,sSlot);
									data.slotMasterSCX = sSlot;

									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ------ IPMI DATA ----------------------------------------------" << std::endl;
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " is one scx in pass. deg. state? " << isPassiveDeg << std::endl;
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " master scx ip address:          " << data.ipMasterSCX.c_str() << std::endl;
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " master scx slot:                " << data.slotMasterSCX.c_str() << std::endl;
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " passive scx ip address:         " << data.ipPassiveSCX.c_str() << std::endl;
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " slot:                  		 " << data.slotUpgOnGoing << std::endl;
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ipmi upgrade status:   		 " << data.upgradeStatus << std::endl;
									std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ---------------------------------------------------------------" << std::endl;

									//restore status
									ipmiUp->restoreStatus(data);

									if(data.upgradeStatus == CCH_Util::IPMI_ONGOING)
									{
										std::cout<<" IPMI UPGRADE activating thread... "<< std::endl;
										ipmiUp->open();
									}
								}
							}
							else
							{
								continue;
							}
						}
					}
					else
					{
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory Error", __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}
						result = -1;
					}
				}
			}
		}//for
	}

	return result;
}

void FIXS_CCH_UpgradeManager::update (const ACS_CS_API_HWCTableChange& observer)
{
	std::cout<< "observer.dataSize = " << observer.dataSize << std::endl;

	unsigned short boardId = 0;
	unsigned long magazine = 0;
	unsigned long reverse_mag = 0;
	unsigned short slot = 0;
	unsigned long ip_ethA = 0;
	unsigned long ip_ethB = 0;
	std::string container("");
	std::string tempExpectedCxp("");
	bool postOperation = false;

	if (observer.dataSize > 0)
	{

		for(int i=0; i< observer.dataSize; i++)
		{
			boardId = 0;
			magazine = 0;
			reverse_mag = 0;
			slot = 0;
			ip_ethA = 0;
			ip_ethB = 0;
			container.clear();
			cout<<"observer.hwcData[i].operationType"<<observer.hwcData[i].operationType<<endl;
			switch(observer.hwcData[i].operationType)
			{
			
			case ACS_CS_API_TableChangeOperation::Add:
				std::cout << "DBG: ------- Add -------- \n" << std::endl;
				std::cout << "FBN: " << observer.hwcData[i].fbn << std::endl;

				if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_SCXB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;
					ip_ethA = observer.hwcData[i].ipEthA;
					ip_ethB = observer.hwcData[i].ipEthB;
					//				container = observer.hwcData[i].softwarePackage; //TODO
					fetchBoardId(magazine,slot,boardId,ACS_CS_API_HWC_NS::FBN_SCXB);


					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue;
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Add SCXB board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
					}

					if (isScxData(magazine,slot) == false)
					{
						std::string dn_blade("");
						IMM_Interface::getDnBoard(magazine,slot,dn_blade);
						IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container);

						addScxEntry (boardId, magazine, slot, ip_ethA, ip_ethB, container, dn_blade);

						//update instances
						postOperation = true;
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Board already present in the list", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);//HT41332
				}
				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_EPB1)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;
					ip_ethA = observer.hwcData[i].ipEthA;
					ip_ethB = observer.hwcData[i].ipEthB;
					//				container = observer.hwcData[i].softwarePackage; //TODO
					fetchBoardId(magazine,slot,boardId,ACS_CS_API_HWC_NS::FBN_EPB1);

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue;
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Add EPB1 board at magazine %s and slot %u ", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					if (isEpbBoard(magazine,slot) == false)
					{
						std::string dn_blade("");
						IMM_Interface::getDnBoard(magazine,slot,dn_blade);
						IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container);
						addEpb1Entry(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn_blade);

						//update instances
						postOperation = true;

					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Board already present in the list", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
				}
				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_IPTB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;
					ip_ethA = observer.hwcData[i].ipEthA;
					ip_ethB = observer.hwcData[i].ipEthB;
					//				container = observer.hwcData[i].softwarePackage; //TODO
					fetchBoardId(magazine,slot,boardId,ACS_CS_API_HWC_NS::FBN_IPTB);

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue;
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Add IPTB board at magazine %s and slot %u ", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					if (isIptData (magazine,slot) == false)
					{
						std::string dn_blade("");
						IMM_Interface::getDnBoard(magazine,slot,dn_blade);
						IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container);
						addIptData (boardId, magazine, slot, ip_ethA, ip_ethB, container,dn_blade);

						//update instances
						postOperation = true;
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Board already present in the list", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
				}

				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_IPLB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;
					ip_ethA = observer.hwcData[i].ipEthA;
					ip_ethB = observer.hwcData[i].ipEthB;
					fetchBoardId(magazine,slot,boardId,IMM_Util::IPLB);

					{ // trace
						char tmpStr[512] = {0};
					std::string stringValue;
					(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Add IPLB board at magazine %s and slot %u ", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					if (isIplbData(magazine,slot) == false)
					{
						std::string dn_blade("");
						IMM_Interface::getDnBoard(magazine,slot,dn_blade);
						IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container);
						addIplbData (boardId, magazine, slot, ip_ethA, ip_ethB, container,dn_blade);

						//update instances
						postOperation = true;
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Board already present in the list", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}checkIPLBswVr();//HT41332
				}

				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_CMXB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;
					ip_ethA = observer.hwcData[i].ipEthA;
					ip_ethB = observer.hwcData[i].ipEthB;
					fetchBoardId(magazine,slot,boardId,IMM_Util::CMXB); //TODO

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue;
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Add CMXB board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					if (isCmxData(magazine,slot) == false)
					{
						std::string dn_blade("");
						IMM_Interface::getDnBoard(magazine,slot,dn_blade);
						IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container);

						addCmxEntry(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn_blade);

						//update instances
						postOperation = true;
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Board already present in the list", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);//HT41332
				}

				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_EvoET) //TODO
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;
					ip_ethA = observer.hwcData[i].ipEthA;
					ip_ethB = observer.hwcData[i].ipEthB;
					fetchBoardId(magazine,slot,boardId,IMM_Util::EVOET); //TODO

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue;
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Add EVOET board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					if (isCmxData(magazine,slot) == false)
					{
						std::string dn_blade("");
						IMM_Interface::getDnBoard(magazine,slot,dn_blade);
						IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container);

						addEvoEtEntry(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn_blade);

						//update instances
						postOperation = true;
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Board already present in the list", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
				}
				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_SMXB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;
					ip_ethA = observer.hwcData[i].ipEthA;
					ip_ethB = observer.hwcData[i].ipEthB;
					fetchBoardId(magazine,slot,boardId,ACS_CS_API_HWC_NS::FBN_SMXB);
					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue;
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Add SMXB board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					if (isSmxData(magazine,slot) == false)
					{
						std::string dn_blade("");
						IMM_Interface::getDnBoard(magazine,slot,dn_blade);
						IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container);

						addSmxEntry (boardId, magazine, slot, ip_ethA, ip_ethB, container, dn_blade);

						//update instances
						postOperation = true;
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Board already present in the list", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
				}				
				break;

			case ACS_CS_API_TableChangeOperation::Delete :
				std::cout << "-------- Delete --------\n" << std::endl;
				std::cout << "FBN: " << observer.hwcData[i].fbn << std::endl;

				if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_SCXB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue("");
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Delete SCXB board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Magazine : " << magazine << "\n" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Slot : " << slot << "\n" << std::endl;
					{
						bool found = false;
						Lock lock;

						swUpgradeList_t::iterator it;
						for (it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
						{
							if ((it->m_magazine == magazine) && (it->m_slot == slot))
							{
								found = true;

								FIXS_CCH_ScxUpgrade * stateMachineSW = 0;
								FIXS_CCH_IpmiUpgrade * stateMachineIPMI = 0;
								FIXS_CCH_PowerFanUpgrade * stateMachinePFM = 0;
								FIXS_CCH_FwUpgrade * stateMachineFW = 0;
								stateMachineSW = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
								stateMachineIPMI = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
								stateMachinePFM = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
								stateMachineFW = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);

								tempExpectedCxp = stateMachineSW->getExpectedCXP();

								if (stateMachineSW)
								{
									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineSW->isRunningThread() == false)
										{
											delete (it->m_swPtr);
											it->m_swPtr = NULL;
											break;
										}
										else
										{
											stateMachineSW->stop();
										}

									}
								}


								if (stateMachineIPMI)
								{
									//delete IPMI state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineIPMI->isRunningThread() == false)
										{
											delete (it->m_ipmiPtr);
											it->m_ipmiPtr = NULL;
											break;
										}
										else
										{
											stateMachineIPMI->stop();
										}
									}
								}

								if (stateMachinePFM)
								{
									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachinePFM->isRunningThread() == false)
										{
											delete (it->m_pfmPtr);
											it->m_pfmPtr = NULL;
											break;
										}
										else
										{
											stateMachinePFM->stop();
										}
									}
								}


								if (stateMachineFW)
								{
									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineFW->isRunningThread() == false)
										{
											delete (it->m_fwPtr);
											it->m_fwPtr = NULL;
											break;
										}
										else
										{
											stateMachineFW->stop();
										}
									}
								}
								break;
							}
						}

						if (found)
						{
							//remove Scx board
							removeScxEntry(magazine,slot, tempExpectedCxp);

							//update instances
							postOperation = true;

							//delete FSM data from the internal data structure
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: delete entry from list " << std::endl;
							m_swUpgradeList->erase(it);
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Done !" << std::endl;

						}
					}
					checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);

				}
				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_EPB1)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue("");
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Delete EPB1 board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Magazine : " << magazine << "\n" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Slot : " << slot << "\n" << std::endl;
					{
						bool found = false;
						Lock lock;

						swUpgradeList_t::iterator it;
						for (it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
						{
							if ((it->m_magazine == magazine) && (it->m_slot == slot))
							{
								found = true;
								break;
							}
						}

						if (found)
						{
							//remove Epb1 board
							removeEpb1Entry(magazine,slot);

							//update instances
							postOperation = true;

							//delete FSM data from the internal data structure
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: delete entry from list " << std::endl;
							m_swUpgradeList->erase(it);
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Done !" << std::endl;

						}
					}
				}
				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_IPTB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue;
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Delete IPTB board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Magazine : " << magazine << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Slot : " << slot << std::endl;
					{
						bool found = false;
						Lock lock;
						swUpgradeList_t::iterator it;
						for (it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
						{
							if ((it->m_magazine == magazine) && (it->m_slot == slot))
							{

								found = true;
								FIXS_CCH_IptbUpgrade * stateMachineLW = 0;
								stateMachineLW = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);

								if (stateMachineLW)
								{
									tempExpectedCxp = stateMachineLW->getExpectedCXP();

									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineLW->isRunningThread() == false)
										{
											delete (it->m_lmPtr);
											it->m_lmPtr = NULL;
											break;
										}
										else
										{
											stateMachineLW->stop();
										}

									}
								}

								break;
							}
						}

						if (found)
						{
							//remove Iptb board
							removeIptEntry(magazine,slot,tempExpectedCxp);

							//update instances
							postOperation = true;

							//delete FSM data from the internal data structure
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: delete entry from list " << std::endl;
							m_iptUpgradeList->erase(it);
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Done !" << std::endl;
						}
					}
				}
				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_IPLB)
				{

					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue;
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Delete IPLB board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Magazine : " << magazine << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Slot : " << slot << std::endl;
					bool found = false;
					{
						Lock lock;
						swUpgradeList_t::iterator it;
						for (it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
						{
							if ((it->m_magazine == magazine) && (it->m_slot == slot))
							{

								found = true;
								FIXS_CCH_IplbUpgrade * stateMachineLW = 0;
								stateMachineLW = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);

								if (stateMachineLW)
								{
									tempExpectedCxp = stateMachineLW->getExpectedCXP();

									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineLW->isRunningThread() == false)
										{
											delete (it->m_lmPtr);
											it->m_lmPtr = NULL;
											break;
										}
										else
										{
											stateMachineLW->stop();
										}

									}
								}
								break;
							}
						}
						if (found)
						{
							//remove Iptb board
							removeIplbEntry(magazine,slot,tempExpectedCxp);

							//update instances
							postOperation = true;

							//delete FSM data from the internal data structure
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: delete entry from list " << std::endl;
							m_swUpgradeList->erase(it);
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Done !" << std::endl;
						}
					}
					checkIPLBswVr();

				}

				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_CMXB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue("");
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Delete CMXB board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Magazine : " << magazine << "\n" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Slot : " << slot << "\n" << std::endl;
					{
						bool found = false;
						Lock lock;

						swUpgradeList_t::iterator it;
						for (it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
						{
							if ((it->m_magazine == magazine) && (it->m_slot == slot))
							{
								found = true;

								FIXS_CCH_CmxUpgrade * stateMachineSW = 0;
								FIXS_CCH_FwUpgrade * stateMachineFW = 0;
								stateMachineSW = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);
								stateMachineFW = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);
								tempExpectedCxp = stateMachineSW->getExpectedCXP();

								if (stateMachineSW)
								{
									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineSW->isRunningThread() == false)
										{
											delete (it->m_swPtr);
											it->m_swPtr = NULL;
											break;
										}
										else
										{
											stateMachineSW->stop();
										}

									}
								}

								if (stateMachineFW)
								{
									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineFW->isRunningThread() == false)
										{
											delete (it->m_fwPtr);
											it->m_fwPtr = NULL;
											break;
										}
										else
										{
											stateMachineFW->stop();
										}
									}
								}
								break;
							}
						}

						if (found)
						{
							//remove Cmx board
							removeCmxEntry(magazine,slot, tempExpectedCxp);

							//update instances
							postOperation = true;

							//delete FSM data from the internal data structure
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: delete entry from list " << std::endl;
							m_swUpgradeList->erase(it);
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Done !" << std::endl;
						}
					}
					checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);
				}

				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_EvoET)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue("");
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Delete EVOET board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Magazine : " << magazine << "\n" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Slot : " << slot << "\n" << std::endl;
					{
						bool found = false;
						Lock lock;

						swUpgradeList_t::iterator it;
						for (it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
						{
							if ((it->m_magazine == magazine) && (it->m_slot == slot))
							{
								found = true;
								break;
							}
						}

						if (found)
						{
							//remove EvoEt board
							removeEvoEtEntry(magazine,slot);

							//update instances
							postOperation = true;

							//delete FSM data from the internal data structure
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: delete entry from list " << std::endl;
							m_swUpgradeList->erase(it);
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Done !" << std::endl;

						}
					}
				}
				else if (observer.hwcData[i].fbn == ACS_CS_API_HWC_NS::FBN_SMXB)
				{
					reverse_mag = observer.hwcData[i].magazine;
					magazine = htonl(reverse_mag);
					slot = observer.hwcData[i].slot;

					{ // trace
						char tmpStr[512] = {0};
						std::string stringValue("");
						(void) CCH_Util::ulongToStringMagazine (magazine, stringValue);
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [HWC TABLE UPDATE] Delete SMXB board at magazine %s and slot %u", __FUNCTION__, __LINE__, stringValue.c_str(), slot);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Magazine : " << magazine << "\n" << std::endl;
					std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Slot : " << slot << "\n" << std::endl;
					{
						bool found = false;
						Lock lock;

						swUpgradeList_t::iterator it;
						for (it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
						{
							if ((it->m_magazine == magazine) && (it->m_slot == slot))
							{
								found = true;

								FIXS_CCH_SmxUpgrade * stateMachineSW = 0;								
								FIXS_CCH_PowerFanUpgrade * stateMachinePFM = 0;
								FIXS_CCH_FwUpgrade * stateMachineFW = 0;
								stateMachineSW = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);								
								stateMachinePFM = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
								stateMachineFW = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);

								tempExpectedCxp = stateMachineSW->getExpectedCXP();

								if (stateMachineSW)
								{
									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineSW->isRunningThread() == false)
										{
											delete (it->m_swPtr);
											it->m_swPtr = NULL;
											break;
										}
										else
										{
											stateMachineSW->stop();
										}

									}
								}							

								if (stateMachinePFM)
								{
									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachinePFM->isRunningThread() == false)
										{
											delete (it->m_pfmPtr);
											it->m_pfmPtr = NULL;
											break;
										}
										else
										{
											stateMachinePFM->stop();
										}
									}
								}


								if (stateMachineFW)
								{
									//delete SW state machine when it is stopped
									for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
									{
										if (stateMachineFW->isRunningThread() == false)
										{
											delete (it->m_fwPtr);
											it->m_fwPtr = NULL;
											break;
										}
										else
										{
											stateMachineFW->stop();
										}
									}
								}
								break;
							}
						}

						if (found)
						{
							//remove Smx board
							removeSmxEntry(magazine,slot, tempExpectedCxp);

							//update instances
							postOperation = true;

							//delete FSM data from the internal data structure
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: delete entry from list " << std::endl;
							m_swUpgradeList->erase(it);
							std::cout << __FUNCTION__ << "@" << __LINE__ << "DBG: Done !" << std::endl;

						}
					}
					checkSMXswVr();

				}
				break;

				//				case ACS_CS_API_TableChangeOperation::Change :
				//				std::cout << "-------- Change --------\n" << std::endl;

			default:
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] HWCTable update has been discarded. Operation type: %d", __LINE__, observer.hwcData[0].operationType);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

				continue;
			}
			}

		}

		if (postOperation)
		{
			if (startSWUpgradeFSMs() == 0)
			{
				if (startPFMUpgradeFSMs() == 0)
				{
					if (startFwUpgradeFSMs() == 0)
					{
						if (startIpmiUpgradeFSMs() == 0)
						{
							if (startIPTUpgradeFSMs() == 0)
							{
								//								if (m_initialized == false) m_initialized = true;
							}
							else
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Impossible to start IPTB upgrade State Machines", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
							}
						}
						else
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Impossible to start IPMI upgrade State Machines", __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

						}
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Impossible to start FW upgrade State Machines", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						//retValue = -5; TODO
					}
				}
				else
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Impossible to start PFM upgrade State Machines", __LINE__);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					//retValue = -5; TODO
				}
			}
			else
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Impossible to start SW upgrade State Machines", __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				//retValue = -5; TODO
			}
		}
	}
	environment = FIXS_CCH_DiskHandler::getEnvironment();

	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
	{
		checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB);
		
	}
	if ((environment == CCH_Util::SINGLECP_DMX) || (environment == CCH_Util::MULTIPLECP_DMX))
	{
		checkIPLBswVr();
	}
	if ((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
	{
		checkSMXswVr();
	}
}


int FIXS_CCH_UpgradeManager::fetchBoardId (unsigned long magazine, unsigned short slot, unsigned short &boardId, /*ACS_CS_API_HWC_NS::HWC_FBN_Identifier*/ int fbn)
{

	int retValue = -1;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(fbn);
			boardSearch->setMagazine(magazine);
			boardSearch->setSlot(slot);

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{

				if (boardList.size() == 0)
				{
					// trace: no board found. The board related hasn't been configured yet.
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No NEW %d board found", __LINE__, fbn);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("No NEW boards found",LOG_LEVEL_WARN);
					retValue = -1;
				}
				else
				{
					boardId = boardList[0];
					retValue = 0;
				}
			}
			else
			{
				// GetBoardIds failed, error from CS
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] GetBoardId failed, error from CS", __LINE__);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				retValue = -1;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] GetBoardIds failed, error from CS", __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}
			retValue = -1;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		}
		retValue = -1;
	}
	return retValue;

}

bool FIXS_CCH_UpgradeManager::ipmiCheckOtherSlotStatus (unsigned long umagazine)
{


	bool found = false;
	//	bool bError = false;
	bool bReturn = true;

	environment = FIXS_CCH_DiskHandler::getEnvironment();

	//CBA environment
	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine))
			{
				if  (it->m_ipmiPtr != 0)
				{
					FIXS_CCH_IpmiUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
					}
					catch (bad_cast)
					{

						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_ipmiPtr is not a FIXS_CCH_IpmiUpgrade", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}
						stateMachine = 0;
					}
					if (!stateMachine)
					{
						//return 1; // Internal Error
						std::cout<<"DBG: "<< __FUNCTION__ << "@" << __LINE__ << " ERROR: stateMachine is NULL" << std::endl;
						//bError = true;
						break;
					}

					//check upgrade ongoing
					if (stateMachine->getStateName() == "ONGOING")
					{
						//found a slot with IPMI ongoing
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}

						found = true;
						break;
					}
				}
			}
		}
	}
	//NOT CBA environment
	else
	{
		//check SCB-RP board
		return true;
	}

	if(found) bReturn = false;

	return bReturn;
}

void FIXS_CCH_UpgradeManager::swMaintenanceList (CCH_Util::RecOfMaintenance *&  listOfMaitenance, unsigned short &  boardListLen)
{

	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	std::string sectionName("");
	std::string upgradeStatus("");
	bool ongoing = false;

	int swState = 0;
	FIXS_CCH_ScxUpgrade *swUpg = NULL;
	FIXS_CCH_CmxUpgrade *swCmxUpg = NULL;
	FIXS_CCH_SmxUpgrade *swSmxUpg = NULL;
	std::string ipmiState("");
	FIXS_CCH_IpmiUpgrade *ipmiUpg = NULL;
	std::string pwfanState("");
	FIXS_CCH_PowerFanUpgrade *pwfanUpg = NULL;
	std::string fwState("");
	FIXS_CCH_FwUpgrade *fwUpg = NULL;

	std::list<CCH_Util::RecOfMaintenance> recList;
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			umagazine = it->m_magazine;
			uslot = it->m_slot;

			if ((uslot != 0) && (uslot != 25)&&(uslot != 26) && (uslot != 28))
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " ((uslot != 0) && (uslot != 25)&& (uslot != 26) && (uslot != 28)) slot: " << uslot << " ...continue..." << std::endl;
				continue;
			}

			unsigned char slot = (unsigned char)uslot;
			unsigned long hlmag = ntohl(umagazine);
			unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
			unsigned char mag = u_plug0 & 0x0F;
			std::cout << __FUNCTION__ << "@" << __LINE__ << " umagazine: " << umagazine << " mag: " << (int)mag << " slot: " << (int)slot << std::endl;

			if (it->m_swPtr != 0)
			{
				if (it->m_fbn == CCH_Util::SCXB)
				{
					swUpg = dynamic_cast<FIXS_CCH_ScxUpgrade *>(it->m_swPtr);
					swState = swUpg->getStateImm();
				}
				else if (it->m_fbn == CCH_Util::CMXB)
				{
					swCmxUpg = dynamic_cast<FIXS_CCH_CmxUpgrade *>(it->m_swPtr);
					swState = swCmxUpg->getStateImm();
				}
				else if (it->m_fbn == CCH_Util::SMXB)
				{
					swSmxUpg = dynamic_cast<FIXS_CCH_SmxUpgrade *>(it->m_swPtr);
					swState = swSmxUpg->getStateImm();
				}
			}
			if (it->m_ipmiPtr != 0)
			{
				ipmiUpg = dynamic_cast<FIXS_CCH_IpmiUpgrade *>(it->m_ipmiPtr);
				ipmiState = ipmiUpg->getStateName();
			}
			if (it->m_pfmPtr != 0)
			{
				pwfanUpg = dynamic_cast<FIXS_CCH_PowerFanUpgrade *>(it->m_pfmPtr);
				pwfanState = pwfanUpg->getStateName();
			}
			if (it->m_fwPtr != 0)
			{
				fwUpg = dynamic_cast<FIXS_CCH_FwUpgrade *>(it->m_fwPtr);
				fwState = fwUpg->getStateName();
			}


			if (it->m_swPtr != 0)
			{
				if (swState == IMM_Util::ACTIVATING || swState == IMM_Util::CANCELLING)
				{
					CCH_Util::RecOfMaintenance rec;
					rec.mag = mag;
					rec.slot = slot;
					rec.type = 0x21;
					recList.push_back(rec);

					ongoing = true;

					if (_trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "SW ACTIVATING or MANUAL_ROLLBACK_ONGOING in progress [mag: %d] and [slot: %d]", (int)mag, (int)slot);
						_trace->ACS_TRA_event(1,tmpStr);
					}
					std::cout << __FUNCTION__ << "@" << __LINE__ << "SW ACTIVATING or MANUAL_ROLLBACK_ONGOING in progress for mag = "<<(int)mag <<" and slot ="<<(int)slot<<std::endl;

				}
			}
			if (!ongoing)
			{
				if (it->m_ipmiPtr != 0)
				{
					if (ipmiState == "ONGOING")
					{
						if ((ipmiUpg->m_slotUpgOnGoing == 0) ||
								(ipmiUpg->m_slotUpgOnGoing == 25)||
								(ipmiUpg->m_slotUpgOnGoing == 26)||
								(ipmiUpg->m_slotUpgOnGoing == 28))
						{
							CCH_Util::RecOfMaintenance rec;
							rec.mag = mag;
							rec.slot = (unsigned char)(ipmiUpg->m_slotUpgOnGoing);
							rec.type = 0x20;
							recList.push_back(rec);

							ongoing = true;

							if (_trace->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "IPMI UPGRADE ONGOING [mag: %d] and [slot: %d]", (int)mag, ipmiUpg->m_slotUpgOnGoing);
								_trace->ACS_TRA_event(1,tmpStr);
							}
							std::cout << __FUNCTION__ << "@" << __LINE__ << "IPMI UPGRADE ONGOING for mag = "<<(int)mag <<" and slot = "<<(int)slot<<std::endl;
						}
						else
						{
							std::cout << __FUNCTION__ << "@" << __LINE__ << " Jump if not SCXB or CMXB !" << std::endl;
						}
					}
				}
			}//ipmi

			if (!ongoing)
			{
				if (it->m_pfmPtr != 0)
				{
					if (pwfanState == "ONGOING")
					{
						CCH_Util::RecOfMaintenance rec;
						rec.mag = mag;
						rec.slot = slot;

						std::string pwfType = pwfanUpg->getType();
						if (pwfType == "0")//"FAN")
						{
							rec.type = 0x11;
							if (_trace->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "FAN UPGRADE ONGOING [mag: %d] and [slot: %d]", (int)mag, (int)slot);
								_trace->ACS_TRA_event(1,tmpStr);
							}
						}
						else
						{
							rec.type = 0x10;
							if (_trace->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "POWER UPGRADE ONGOING [mag: %d] and [slot: %d]", (int)mag, slot);
								_trace->ACS_TRA_event(1,tmpStr);
							}
						}

						recList.push_back(rec);
					}
				}
			}//pfm

			if (!ongoing)
			{
				if (it->m_fwPtr != 0)
				{
					if (fwState == "ONGOING")
					{
						if ((fwUpg->m_slotUpgOnGoing == 0) ||	(fwUpg->m_slotUpgOnGoing == 25)||
								(fwUpg->m_slotUpgOnGoing == 26) ||	(fwUpg->m_slotUpgOnGoing == 28))
						{
							CCH_Util::RecOfMaintenance rec;
							rec.mag = mag;
							rec.slot = (unsigned char)(fwUpg->m_slotUpgOnGoing);
							rec.type = 0x21;
							recList.push_back(rec);

							ongoing = true;

							if (_trace->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "FW UPGRADE ONGOING [mag: %d] and [slot: %d]", (int)mag, fwUpg->m_slotUpgOnGoing);
								_trace->ACS_TRA_event(1,tmpStr);
							}
						}
						else
						{
							std::cout << __FUNCTION__ << "@" << __LINE__ << " Jump if not SCXB or CMXB !" << std::endl;
						}
					}
				}
			}//fw

			//reset for next board
			ongoing = false;
		}//for
	}

	boardListLen = recList.size();
	if (_trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "Maintenance board list size: %d", boardListLen);
		_trace->ACS_TRA_event(1,tmpStr);
	}
	if (boardListLen > 0)
	{
		std::cout<<"boardListLen"<<boardListLen<<endl;
		listOfMaitenance = new CCH_Util::RecOfMaintenance[boardListLen];
		int i = 0;
		for (std::list<CCH_Util::RecOfMaintenance>::iterator recIt = recList.begin(); recIt != recList.end(); recIt++)
		{
			listOfMaitenance[i++] = *recIt;
		}
	}
}



int FIXS_CCH_UpgradeManager::startIPTUpgradeFSMs()
{

	int result  = 0;
	std::string dn_object("");
	std::string dn_swInv("");
	std::string dn_swPers("");
	CCH_Util::BoardSWData wdata;

	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{
		if (it->m_lmPtr == 0)
		{
			std::string shelfparent("");
			std::string strEthA0("");
			std::string strEthB0("");
			std::string strEthA25("");
			std::string strEthB25("");

			bool bOk = IMM_Util::getDnParent(it->m_dn, shelfparent);
			std::cout << __FUNCTION__ << "@" << __LINE__ << " ### shelfparent:[" << shelfparent << "]." << std::endl;

			if(bOk == true)
			{
				// Get the IPs of the board in the magazine shelfparent;
				bOk = IMM_Interface::GetBoardIps(shelfparent,&strEthA0,&strEthB0,&strEthA25,&strEthB25);

				// Check error
				if(bOk == true)
				{
					// Print diagnostic
					std::cout << __FUNCTION__ << ": Found Board:" << std::endl;
					std::cout << "  - board in slot 0 : IPA[" << strEthA0.c_str() << "] ; IPB[" << strEthB0.c_str() << "]" << std::endl;
					std::cout << "  - board in slot 25: IPA[" << strEthA25.c_str() << "] ; IPB[" << strEthB25.c_str() << "]" << std::endl;
				}
				else
					std::cout << __FUNCTION__ << "@" << __LINE__ << " ### ERROR to obtain board ip address[" << std::endl;
			}

			if (it->m_fbn == CCH_Util::IPTB)
			{
				FIXS_CCH_IptbUpgrade *lmup = new FIXS_CCH_IptbUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, strEthA0, strEthB0, strEthA25, strEthB25, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);

				if (lmup)
				{
					CCH_Util::BoardSWData swOut;
					dn_swInv.clear();
					dn_swPers.clear();
					dn_swInv = lmup->getSwInvObjectDN();
					it->m_lmPtr = lmup;
					dn_swPers = lmup->getSwPercistanceDN();
					swOut.dnBladePersistance = dn_swPers;

					ACS_APGCC_ImmObject object;

					if (IMM_Util::getObject(dn_swInv, &object))
					{
						std::cout << __FUNCTION__<< " setBoardSwDataFromModel " << std::endl;
						if (IMM_Interface::setBoardSwDataFromModel(it->m_dn, dn_swInv, swOut))
						{
							if (strcmp(it->m_container.c_str(),swOut.currentCxp.c_str())!=0)
							{
								it->m_container = swOut.currentCxp;
							}

							std::cout << "\n SET PARAMETERS FROM MODEL" << std::endl;
							lmup->restoreStatus(swOut);

							if (( swOut.upgradeStatus != CCH_Util::STATE_IDLE ) && (swOut.upgradeStatus != CCH_Util::STATE_FAILED))
							{
								lmup->open();
								std::cout << "\n UPGRADE ONGOING on board :\n"<< it->m_dn.c_str() << std::endl;
							}
							else
							{
								//std::cout << " STATE: IDLE" << std::endl;
							}
						}
						else
						{
							if (lmup->initSwInventory())
							{
								lmup->getOverallStatus(wdata);
								std::cout << "\n SET DEFAULT PARAMETERS" << std::endl;
							}
							else
							{
								std::cout << "\n ERROR SET DEFAULT PARAMETERS "<< std::endl;
							}
						}
					}
					else
					{
						std::cout << "\nIMM Error : Child Object NOT EXIST "<< std::endl;
					}
				}
				else
				{
					__;
					printError("Memory Error\n",__FUNCTION__, __LINE__);
				}
			}
		}
	}
	return result;
}
/*
 *
 *
 *
   //## begin FIXS_CCH_UpgradeManager::startSWUpgradeFSMs%4C050BDB0068.body preserve=yes
        TraceInOut inOut(__FUNCTION__);__;
        Lock lock;
        int result  = 0;
        FIXS_CCH_FSMDataStorage *storage = FIXS_CCH_FSMDataStorage::getInstance();
        CCH_Util::SWData wdata;

        for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
        {
                if (it->m_swPtr == 0)
                {
                        FIXS_CCH_ScxUpgrade *swup = new (std::nothrow) FIXS_CCH_ScxUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString());
                        if (swup)
                        {
                                CCH_Util::SWData swOut;
                                if(storage->readUpgradeStatus(it->m_magazine, it->m_slot, swOut)) {
                                        swup->restoreStatus(swOut);
                                } else {
                                        swup->getOverallStatus(wdata);
                                        storage->saveUpgradeStatus(it->m_magazine,it->m_slot,wdata);
                                }

                                it->m_swPtr = swup;
                                if (FIXS_CCH_ThreadPool::start(it->m_swPtr) == false)
                                {
                                        if (ACS_TRA_ON(traceObj))
                                        {
                                                char tmpStr[512] = {0};
                                                _snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Error during State Machine Thread start", __LINE__);
                                                ACS_TRA_event(&traceObj, tmpStr);
                                        }
                                        result = -2;
                                        delete it->m_swPtr;
                                        it->m_swPtr = 0;
                                }
                        }
                        else
                        {
                                if (ACS_TRA_ON(traceObj))
                                {
                                        char tmpStr[512] = {0};
                                        _snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory Error", __LINE__);
                                        ACS_TRA_event(&traceObj, tmpStr);
                                }
                                result = -1;
                        }
                }
        }
        return result;







	__;
	Lock lock;
	int result  = 0;
	FIXS_CCH_FSMDataStorage *storage = FIXS_CCH_FSMDataStorage::getInstance();
	CCH_Util::BoardSWData wdata;

	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{
		if (it->m_swPtr == 0)
		{
			unsigned long slot0_ipA=0;
			unsigned long slot0_ipB=0;
			unsigned long slot25_ipA=0;
			unsigned long slot25_ipB=0;

			if ( fetchSwitchAddresses (it->m_magazine, slot0_ipA, slot0_ipB, slot25_ipA, slot25_ipB) == 0)
			{
				std::string slot0_ipA_String = CCH_Util::ulongToStringIP(slot0_ipA);
				std::string slot0_ipB_String = CCH_Util::ulongToStringIP(slot0_ipB);
				std::string slot25_ipA_String = CCH_Util::ulongToStringIP(slot25_ipA);
				std::string slot25_ipB_String = CCH_Util::ulongToStringIP(slot25_ipB);

				std::cout << "-----------------------------------" << std::endl;
				std::cout << "CREATING IPT FSM:" << std::endl;
				std::cout<<"DBG: "<<__FUNCTION__<<"@"<<__LINE__<<" Switch 0 ip_ethA: "<<slot0_ipA_String<<std::endl;
				std::cout<<"DBG: "<<__FUNCTION__<<"@"<<__LINE__<<" Switch 0 ip_ethB: "<<slot0_ipB_String<<std::endl;
				std::cout<<"DBG: "<<__FUNCTION__<<"@"<<__LINE__<<" Switch 25 ip_ethA: "<<slot25_ipA_String<<std::endl;
				std::cout<<"DBG: "<<__FUNCTION__<<"@"<<__LINE__<<" Switch 25 ip_ethB: "<<slot25_ipB_String<<std::endl;
				std::cout<<"DBG: "<<__FUNCTION__<<"@"<<__LINE__<<" it->m_container: "<< it->m_container <<std::endl;

				FIXS_CCH_IptbUpgrade *iptbUpgrade = new (std::nothrow) FIXS_CCH_IptbUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, slot0_ipA_String, slot0_ipB_String, slot25_ipA_String, slot25_ipB_String, it->getIPEthAString(), it->getIPEthBString());
				if (iptbUpgrade)
				{
					CCH_Util::BoardSWData swOut;
					if(storage->readUpgradeStatus(it->m_magazine, it->m_slot, swOut))
					{
						iptbUpgrade->restoreStatus(swOut);

						if ((iptbUpgrade->getStateId() != FIXS_CCH_IptbUpgrade::STATE_IDLE)
							&& (iptbUpgrade->getStateId() != FIXS_CCH_IptbUpgrade::STATE_FAILED))
						{
							if (FIXS_CCH_ThreadPool::start(iptbUpgrade)) //start thread
							{
								std::cout << __FUNCTION__ << "@" << __LINE__ << "*** START THREADPOOL IPTBUPGRADE ***" <<std::endl;
							}
							else
							{
								if (ACS_TRA_ON(traceObj))
								{
									char tmpStr[512] = {0};
									_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory Error", __LINE__);
									ACS_TRA_event(&traceObj, tmpStr);
								}
								result = -3;
							}
						}
					}
					else
					{
						iptbUpgrade->saveStatus();
					}
					it->m_swPtr = iptbUpgrade;
				}
				else
				{
					if (ACS_TRA_ON(traceObj))
					{
						char tmpStr[512] = {0};
						_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory Error", __LINE__);
						ACS_TRA_event(&traceObj, tmpStr);
					}
					result = -2;
				}
			}

			else
			{
				//error - trace
				if (ACS_TRA_ON(traceObj))
				{
					char tmpStr[512] = {0};
					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Impossible to fetch switch addresses", __LINE__);
					ACS_TRA_event(&traceObj, tmpStr);
				}
				result = -1;

			}



		}
	}
	return result;
 */

//}

int FIXS_CCH_UpgradeManager::fetchSwitchAddresses (unsigned long magazine, unsigned long &slot0_ipA, unsigned long &slot0_ipB, unsigned long &slot25_ipA, unsigned long &slot25_ipB)
{
	UNUSED(magazine);
	UNUSED(slot0_ipA);
	UNUSED(slot0_ipB);
	UNUSED(slot25_ipA);
	UNUSED(slot25_ipB);


	return 0;
	/*

	__;

	int retValue = 0;
	ACS_CS_API_BoardSearch * boardSearch = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			{ // fetch slot 0 addresses
				ACS_CS_API_IdList boardList;
				boardSearch->setMagazine(magazine);
				boardSearch->setSlot(0);
				ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					if (boardList.size() == 0)
					{
						// trace: no Switch has been configured in the slot 0
						if (ACS_TRA_ON(traceObj))
						{
							char tmpStr[512] = {0};
							_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No Switch configured in the magazine 0x%x slot 0", __LINE__, magazine);
							ACS_TRA_event(&traceObj, tmpStr);
						}
						retValue = 0;
					}
					else
					{
						for (unsigned int i = 0; i < boardList.size(); i++)
						{
							BoardID boardId = boardList[i];
							returnValue = hwc->getIPEthA (slot0_ipA, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								returnValue = hwc->getIPEthB (slot0_ipB, boardId);
								if (returnValue == ACS_CS_API_NS::Result_Success)
								{
									std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " Read both IP addresses" << std::endl;
								}
								else
								{
									//serious fault: IP_ethB switch not configured in CS on slot 0
									if (ACS_TRA_ON(traceObj))
									{
										char tmpStr[512] = {0};
										_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] serious fault: switch not configured in CS on slot 0 in magazine %x", __LINE__, magazine);
										ACS_TRA_event(&traceObj, tmpStr);
									}
									__
										retValue = -8;
									break;
								}
							}
							else
							{
								//serious fault: IP_ethA switch not configured in CS on slot 0
								if (ACS_TRA_ON(traceObj))
								{
									char tmpStr[512] = {0};
									_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the Switch board", __LINE__);
									ACS_TRA_event(&traceObj, tmpStr);
								}
								__
									retValue = -7;
								break;
							}
						}//for
					}//else
				}//if
				else
				{
					// GetBoardIds failed, error from CS
					if (ACS_TRA_ON(traceObj))
					{
						char tmpStr[512] = {0};
						_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] GetBoardIds failed, error from CS", __LINE__);
						ACS_TRA_event(&traceObj, tmpStr);
					}
					retValue = -6;
					__
				}
			} // fetch slot 0 addresses

			//reset for performing a new search
			boardSearch->reset();

			{ // fetch slot 25 addresses
				ACS_CS_API_IdList boardList;
				boardSearch->setMagazine(magazine);
				boardSearch->setSlot(25);
				ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					if (boardList.size() == 0)
					{
						// trace: no Switch has been configured in the slot 0
						if (ACS_TRA_ON(traceObj))
						{
							char tmpStr[512] = {0};
							_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No Switch configured in the magazine 0x%x slot 25", __LINE__, magazine);
							ACS_TRA_event(&traceObj, tmpStr);
						}
						retValue = 0;
					}
					else
					{
						for (unsigned int i = 0; i < boardList.size(); i++)
						{
							BoardID boardId = boardList[i];
							returnValue = hwc->getIPEthA (slot25_ipA, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								returnValue = hwc->getIPEthB (slot25_ipB, boardId);
								if (returnValue == ACS_CS_API_NS::Result_Success)
								{
									std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " Read both IP addresses" << std::endl;
								}
								else
								{
									//serious fault: IP_ethB switch not configured in CS on slot 25
									if (ACS_TRA_ON(traceObj))
									{
										char tmpStr[512] = {0};
										_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] serious fault: switch not configured in CS on slot 25 in magazine %x", __LINE__, magazine);
										ACS_TRA_event(&traceObj, tmpStr);
									}
									__
										retValue = -5;
									break;
								}
							}
							else
							{
								//serious fault: IP_ethA switch not configured in CS on slot 25
								if (ACS_TRA_ON(traceObj))
								{
									char tmpStr[512] = {0};
									_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the Switch board", __LINE__);
									ACS_TRA_event(&traceObj, tmpStr);
								}
								__
									retValue = -4;
								break;
							}
						}//for
					}//else
				}//if
				else
				{
					// GetBoardIds failed, error from CS
					if (ACS_TRA_ON(traceObj))
					{
						char tmpStr[512] = {0};
						_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] GetBoardIds failed, error from CS", __LINE__);
						ACS_TRA_event(&traceObj, tmpStr);
					}
					retValue = -3;
					__
				}
			} // fetch slot 25 addresses
			__
		}
		else
		{
			if (ACS_TRA_ON(traceObj))
			{
				char tmpStr[512] = {0};
				_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] GetBoardIds failed, error from CS", __LINE__);
				ACS_TRA_event(&traceObj, tmpStr);
			}
			retValue = -2;
			__
		}
		__
	}
	else
	{
		if (ACS_TRA_ON(traceObj))
		{
			char tmpStr[512] = {0};
			_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			ACS_TRA_event(&traceObj, tmpStr);
		}
		retValue = -1;
		__
	}

	if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
	__;
	return retValue;

	 */}


int FIXS_CCH_UpgradeManager::prepareScx (unsigned long umagazine, unsigned short uslot, std::string product)
{

	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false)	return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,26) == false)	return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,28) == false)	return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false)	return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false)	return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false)	return UPGRADE_ALREADY_ONGOING;
	if(fwCheckOtherSlotStatus(umagazine) == false)		return UPGRADE_ALREADY_ONGOING;


	std::string container = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(product);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] product: %s  -  container: %s ", __FUNCTION__, __LINE__,product.c_str(), container.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
	}



	//call state machine method
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				if (it->m_swPtr == 0 )
				{
					FIXS_CCH_ScxUpgrade *swup = new FIXS_CCH_ScxUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);
					std::cout << __FUNCTION__ <<"Initialize  FIXS_CCH_ScxUpgrade "<<__LINE__ <<std::endl;
					if (swup)
					{
						it->m_swPtr = swup;
					}
				}

				{
					FIXS_CCH_ScxUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
					}
					catch (bad_cast)
					{
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
							_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						stateMachine = 0;
					}

					if (stateMachine)
					{
						//result = stateMachine->prepare(product, container);

						if (!stateMachine->isRunningThread())
						{
							std::cout << "  Thread StateMachine is not running..........START " << std::endl;
							stateMachine->open();
						}
						else std::cout << "  Thread StateMachine is already running.........." << std::endl;

						result = stateMachine->prepare(container, product);
						if (( result != WRONG_OPTION_PREPARE) && (result != SW_ALREADY_ON_BOARD) && (result != 0))
						{
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("SCX UPGRADE : Stoppping thread...",LOG_LEVEL_WARN);
							stateMachine->stop();
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
						else if (result == EXECUTED) //OK
						{
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
					}
					else
						result = 1;//INTERNALERR;

					break;
				}

			}
		}
	}

	return result;

}

int FIXS_CCH_UpgradeManager::prepareIpt (unsigned long umagazine, std::string magazine, unsigned short uslot, std::string slot, std::string package, bool normalInstallationType)
{

	UNUSED(umagazine);
	UNUSED(magazine);
	UNUSED(uslot);
	UNUSED(slot);
	UNUSED(package);
	UNUSED(normalInstallationType);

	return 0;
	/*
	std::string container; //cxp
	std::string cxc = package; //cxc
	int result = HARDWARE_NOT_PRESENT;

	//what if there's an ongoing upgrade on SCX ?

	//if package == 0, load default package else check if it exists through SoftwareMap
	if (cxc.empty())
	{
		//retrieve default package from the map // CXP
		if (FIXS_CCH_SoftwareMap::getInstance()->getIptDefaultPackage(container) == false)
		{
			if (ACS_TRA_ON(traceObj))
			{
				char tmpStr[512] = {0};
				_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] no installation yet. prepare failed ", __FUNCTION__, __LINE__);
				ACS_TRA_event(&traceObj, tmpStr);
			}
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ", Error. prepare failed. container is empty" << std::endl;
			return PACKAGE_NOT_INSTALLED;//15 QUESTION: proper code for no installation yet
		}

		//get sw_product
		cxc = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(container);
		if (cxc.empty())
		{
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ", Error. TmpStr is empty" << std::endl;
			if (ACS_TRA_ON(traceObj))
			{
				char tmpStr[512] = {0};
				_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CXC is Empty ", __FUNCTION__, __LINE__);
				ACS_TRA_event(&traceObj, tmpStr);
			}

			return INTERNALERR;
		}
	}
	else
	{
		if (FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(cxc, CCH_Util::IPTB) == false)
		{
			if (ACS_TRA_ON(traceObj))
			{
				char tmpStr[512] = {0};
				_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] the package does not exist = %s ", __FUNCTION__, __LINE__, package.c_str());
				ACS_TRA_event(&traceObj, tmpStr);
			}
			return PACKAGE_NOT_INSTALLED;
		}
		else
		{
			container = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(cxc);
			if ( !(FIXS_CCH_SoftwareMap::getInstance()->getFBN(container) == "IPTB") )
			{
				if (ACS_TRA_ON(traceObj))
				{
					char tmpStr[512] = {0};
					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] the package is not an IPT appropriate one", __FUNCTION__, __LINE__);
					ACS_TRA_event(&traceObj, tmpStr);
				}
				return PACKAGE_NOT_APPLICABLE;
			}
		}

	}

	//call state machine method
	{
		Lock lock;
		for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++) 
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				FIXS_CCH_IptbUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
				}
				catch (bad_cast)
				{
					if (ACS_TRA_ON(traceObj))
					{
						char tmpStr[512] = {0};
						_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_lmPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
						ACS_TRA_event(&traceObj, tmpStr);
					}
					stateMachine = 0;
				}

				if (stateMachine)
				{
					if (stateMachine->getStateId() == FIXS_CCH_IptbUpgrade::STATE_IDLE || stateMachine->getStateId() == FIXS_CCH_IptbUpgrade::STATE_FAILED)
					{	
						//start thread
						if (FIXS_CCH_ThreadPool::start(stateMachine))
						{
							//wait for initialization end
							for (int retry = 0; ((stateMachine->isInitialized() == false) && (retry < 10)); retry++) 
							{
								Sleep(50);
							}
						}

					}
					result = stateMachine->prepare(container, cxc, normalInstallationType);
				}

				else
				{
					//not found
					result = INTERNALERR;
				}
				break;
			}
		}
	}
	return result;

	 */}

int FIXS_CCH_UpgradeManager::prepareIpt (unsigned long umagazine, unsigned short uslot, std::string product,int upgradeType)
{
	int result = 0;
	std::string dn("");	
	std::string container = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(product);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] product: %s  -  container: %s ", __FUNCTION__, __LINE__,product.c_str(), container.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}



	//call state machine method
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				if (it->m_lmPtr == 0 )
				{
					std::string shelfparent("");
					std::string strEthA0("");
					std::string strEthB0("");
					std::string strEthA25("");
					std::string strEthB25("");

					bool bOk = IMM_Util::getDnParent(it->m_dn, shelfparent);
					std::cout << __FUNCTION__ << "@" << __LINE__ << " ### shelfparent:[" << shelfparent << "]." << std::endl;
					if(bOk == true)
					{
						// Get the IPs of the board in the magazine shelfparent;
						bOk = IMM_Interface::GetBoardIps(shelfparent,&strEthA0,&strEthB0,&strEthA25,&strEthB25);

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

					FIXS_CCH_IptbUpgrade *lmup = new FIXS_CCH_IptbUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, strEthA0, strEthB0, strEthA25, strEthB25, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);
					std::cout << __FUNCTION__ <<"Initialize  FIXS_CCH_IptbUpgrade "<<__LINE__ <<std::endl;
					if (lmup)
					{
						it->m_lmPtr = lmup;
					}
				}

				{
					FIXS_CCH_IptbUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
					}
					catch (bad_cast)
					{
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_lmPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
							_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						stateMachine = 0;
					}

					if (stateMachine)
					{

						if (!stateMachine->isRunningThread())
						{
							std::cout << "  Thread StateMachine is not running..........START " << std::endl;
							stateMachine->open();
						} 
						else    
							std::cout << "  Thread StateMachine is already running.........." << std::endl;
						FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn,container);
						if (upgradeType == 0) //normal(partial)
						{
							result = stateMachine->prepare(container, product,upgradeType);
						}
						else if (upgradeType == 1) //initial (complete)
						{
							if (stateMachine->getStateId() == FIXS_CCH_IptbUpgrade::STATE_IDLE || stateMachine->getStateId() == FIXS_CCH_IptbUpgrade::STATE_FAILED)
							{
								//start thread
								if (stateMachine->isRunningThread())
								{
									//wait for initialization end
									for (int retry = 0; ((stateMachine->isInitialized() == false) && (retry < 10)); retry++)
									{
										usleep(50000);
									}
								}	                                           		
							}

							result = stateMachine->prepare(container, product,upgradeType);
						}
						if (( result != WRONG_OPTION_PREPARE) && (result != SW_ALREADY_ON_BOARD) && (result != 0))
						{
							if (FIXS_CCH_logging)  FIXS_CCH_logging->Write("IPT UPGRADE : Stoppping thread...",LOG_LEVEL_WARN);
							stateMachine->stop();
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
						else if (result == EXECUTED) //OK
						{
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}

						return result;
					}
					else
						result = 1;//INTERNALERR;
				}	         break;

			}
		}//for loop 
	}
	return result;
}
int FIXS_CCH_UpgradeManager::prepareSmx (unsigned long umagazine, unsigned short uslot, std::string product)
{

	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false)	return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false)	return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false)	return UPGRADE_ALREADY_ONGOING;
	if(fwCheckOtherSlotStatus(umagazine) == false)		return UPGRADE_ALREADY_ONGOING;


	std::string container = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(product);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] product: %s  -  container: %s ", __FUNCTION__, __LINE__,product.c_str(), container.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
	}



	//call state machine method
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				if (it->m_swPtr == 0 )
				{
					FIXS_CCH_SmxUpgrade *swup = new FIXS_CCH_SmxUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);
					std::cout << __FUNCTION__ <<"Initialize  FIXS_CCH_SmxUpgrade "<<__LINE__ <<std::endl;
					if (swup)
					{
						it->m_swPtr = swup;
					}
				}

				{
					FIXS_CCH_SmxUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
					}
					catch (bad_cast)
					{
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);
							_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						stateMachine = 0;
					}

					if (stateMachine)
					{
						//result = stateMachine->prepare(product, container);

						if (!stateMachine->isRunningThread())
						{
							std::cout << "  Thread StateMachine is not running..........START " << std::endl;
							stateMachine->open();
						}
						else std::cout << "  Thread StateMachine is already running.........." << std::endl;

						result = stateMachine->prepare(container, product);
						if (( result != WRONG_OPTION_PREPARE) && (result != SW_ALREADY_ON_BOARD) && (result != 0))
						{
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("SMX UPGRADE : Stoppping thread...",LOG_LEVEL_WARN);
							stateMachine->stop();
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
						else if (result == EXECUTED) //OK
						{
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
					}
					else
						result = 1;//INTERNALERR;

					break;
				}

			}
		}
	}

	return result;

}

int FIXS_CCH_UpgradeManager::activateScx (unsigned long umagazine, unsigned short uslot)
{

	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;

	// check if state machine exist. if not, create the FSM
	// else... do this:

	Lock lock;


	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{

		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			FIXS_CCH_ScxUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
				}

				stateMachine = 0;
			}
			if (stateMachine)
			{
				if (!stateMachine->isRunningThread())
				{
					std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					stateMachine->open();
				}
				else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				result = stateMachine->activateSW();

			}
			else
				result = 1;

			break;
		}

	}

	return result;


}

int FIXS_CCH_UpgradeManager::activateSmx (unsigned long umagazine, unsigned short uslot)
{

	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;

	// check if state machine exist. if not, create the FSM
	// else... do this:

	Lock lock;


	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{

		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			FIXS_CCH_SmxUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
				}

				stateMachine = 0;
			}
			if (stateMachine)
			{
				if (!stateMachine->isRunningThread())
				{
					std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					stateMachine->open();
				}
				else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				result = stateMachine->activateSW();

			}
			else
				result = 1;

			break;
		}

	}

	return result;


}

int FIXS_CCH_UpgradeManager::cancelScx (unsigned long umagazine, unsigned short uslot)
{
	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;

	FIXS_CCH_ScxUpgrade *stateMachine = 0;
	/*	FIXS_CCH_ScxUpgrade *stateMachine0 = 0;
	 	FIXS_CCH_ScxUpgrade *stateMachine25 = 0;

	 	int machinesFound = 0;
	 	std::string ip_0 = "";
	 	std::string ip_25 = "";

	 	{ // set neighbor addresses
	 		Lock lock;
	 		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	 		{
	 			if (it->m_magazine == umagazine)
	 			{
	 				try
	 				{
	 					if (it->m_slot == 0)
	 					{
	 						ip_0 = it->getIPEthAString();
	 						stateMachine0 = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
	 						machinesFound++;
	 					}
	 					else
	 					{
	 						ip_25 = it->getIPEthAString();
	 						stateMachine25 = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
	 						machinesFound++;
	 					}
	 				}
	 				catch (bad_cast)
	 				{
	 					{ // log
	 						char tmpStr[512] = {0};
	 						_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
	 						FIXS_CCH_Logger::createInstance()->log(tmpStr, FIXS_CCH_Logger::Warning);
	 						if (ACS_TRA_ON(traceObj)) ACS_TRA_event(&traceObj, tmpStr);
	 					}
	 					stateMachine0 = 0;
	 					stateMachine25 = 0;
	 				}

	 				if (machinesFound >= 2) break;

	 			}//magazine
	 		}//for

	 		//set ip address
	 		if (stateMachine0 && stateMachine25)
	 		{
	 			if (uslot == 0) stateMachine0->m_neighborSCXIP = ip_25;
	 			else stateMachine25->m_neighborSCXIP = ip_0;
	 		}
	 	}

	 	{ // issue cancel
	 */	Lock lock;

	 //	result = HARDWARE_NOT_PRESENT; // 14;
	 for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	 {

		 if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		 {
			 try
			 {
				 stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
			 }
			 catch (bad_cast)
			 {
				 { // log
					 char tmpStr[512] = {0};
					 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
					 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				 }
				 stateMachine = 0;
			 }

			 if (stateMachine)
			 {
				 if (!stateMachine->isRunningThread())
				 {
					 std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					 stateMachine->open();

				 }
				 else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				 result = stateMachine->cancel();
			 }
			 else
			 {
				 result = 1;
			 }
			 break;
		 }//slot mag
	 }//for

	 //}
	 return result;
}
int FIXS_CCH_UpgradeManager::cancelSmx (unsigned long umagazine, unsigned short uslot)
{
	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;

	FIXS_CCH_SmxUpgrade *stateMachine = 0;
	 //	result = HARDWARE_NOT_PRESENT; // 14;
	 for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	 {

		 if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		 {
			 try
			 {
				 stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
			 }
			 catch (bad_cast)
			 {
				 { // log
					 char tmpStr[512] = {0};
					 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);
					 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				 }
				 stateMachine = 0;
			 }

			 if (stateMachine)
			 {
				 if (!stateMachine->isRunningThread())
				 {
					 std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					 stateMachine->open();

				 }
				 else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				 result = stateMachine->cancel();
			 }
			 else
			 {
				 result = 1;
			 }
			 break;
		 }//slot mag
	 }//for

	 //}
	 return result;
}

int FIXS_CCH_UpgradeManager::commitScx (unsigned long umagazine, unsigned short uslot)
{

	int result = 0;
	//unsigned short otherSlot = 0;
	std::string container("");

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;

	// check if state machine exist
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{

			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				FIXS_CCH_ScxUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}

				if (stateMachine)
				{
					if (!stateMachine->isRunningThread())
					{
						std::cout << "  Thread StateMachine is not running..........START " << std::endl;
						stateMachine->open();

					}
					else std::cout << "  Thread StateMachine is already running.........." << std::endl;

					result = stateMachine->commit();

				}
				else
				{
					result = 1;
				}
				break;
			}

		}

	}

	return result;
}

int FIXS_CCH_UpgradeManager::commitSmx (unsigned long umagazine, unsigned short uslot)
{

	int result = 0;
	//unsigned short otherSlot = 0;
	std::string container("");

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;

	// check if state machine exist
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{

			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				FIXS_CCH_SmxUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}

				if (stateMachine)
				{
					if (!stateMachine->isRunningThread())
					{
						std::cout << "  Thread StateMachine is not running..........START " << std::endl;
						stateMachine->open();

					}
					else std::cout << "  Thread StateMachine is already running.........." << std::endl;

					result = stateMachine->commit();

				}
				else
				{
					result = 1;
				}
				break;
			}

		}

	}

	return result;
}

int FIXS_CCH_UpgradeManager::activateIpt (unsigned long umagazine, unsigned short uslot)
{
	int result = 0;
	//unsigned short otherSlot = 0;
	std::string container("");
	Lock lock;

	for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{
		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			FIXS_CCH_IptbUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
			}
			catch (bad_cast)
			{
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_lmPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
				}

				stateMachine = 0;
			}
			if (stateMachine)
			{
				if (!stateMachine->isRunningThread())
				{
					std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					stateMachine->open();
				}
				else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				result = stateMachine->activateSw();

			}
			else
				result = 1;

			break;
		}
	}
	return result;
}

int FIXS_CCH_UpgradeManager::commitIpt (unsigned long umagazine, unsigned short uslot, bool setAsDefaultPackage)
{
	int result = 0;
	//unsigned short otherSlot = 0;
	std::string container("");

	// check if state machine exist
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
		{

			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				FIXS_CCH_IptbUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
				}
				catch(bad_cast)
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}

				if (stateMachine)
				{
					if (!stateMachine->isRunningThread())
					{
						std::cout << "  Thread StateMachine is not running..........START " << std::endl;
						stateMachine->open();

					}
					else std::cout << "  Thread StateMachine is already running.........." << std::endl;

					result = stateMachine->commit(setAsDefaultPackage);

				}
				else
				{
					result = 1;
				}
				break;
			}
		}
	}
	return result;
}

int FIXS_CCH_UpgradeManager::swInfoFbn (int fbn, std::vector<CCH_Util::SWINFO_FBN> &currentSwList)
{

	UNUSED(fbn);
	UNUSED(currentSwList);


	return 0;
	/*



	int result = 14;
	bool found = false;
	std::string commentSW;

		Lock lock;
		CCH_Util::SWINFO_FBN swInfo;
		if ( fbn == CCH_Util::SCXB )
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
			for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
			{
				m_currentSw.clear();
				FIXS_CCH_ScxUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
				}
				catch (bad_cast)
				{
					if (ACS_TRA_ON(traceObj))
					{
						char tmpStr[512] = {0};
						_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
						ACS_TRA_event(&traceObj, tmpStr);
					}
					stateMachine = 0;
				}

				if (stateMachine == 0) return 1;

				m_currentState = stateMachine->getStateName();

				if (
					(m_currentState == "PREPARING") ||
					(m_currentState == "PREPARED") || (m_currentState == "FAILED") ||
					(m_currentState == "ACTIVATING") ||
					(m_currentState == "MANUAL_ROLLBACK_ONGOING") || (m_currentState == "MANUAL_ROLLBACK_ENDED") ||
					(m_currentState == "AUTOMATIC_ROLLBACK_ENDED")
					)
				{
					//std::string pkg = stateMachine->getCurrentCXP();
					//m_currentSw = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(pkg);
					m_currentSw = "-";
					commentSW = "UPGRADE ONGOING";
				}
				else if (m_currentState == "ACTIVATED")
				{
					std::string cxcNew = stateMachine->getExpectedCXC();
					m_currentSw = cxcNew;
					commentSW = "UPGRADE ONGOING";
				}
				else
				{
					// IDLE
					m_currentSw.clear();
					std::string currentSw;
					currentSw = stateMachine->getCurrentRunningSoftware();

					commentSW = " ";
					size_t scx_Index = currentSw.find_first_of("SCX_");
					cout<<"DBG: current software : "<< currentSw << endl;
					if ((scx_Index == std::string::npos) || (currentSw.size() < 4))
					{
						//return SNMP_FAILURE; // 13; // "SNMP Failure"
						m_currentSw = "-";
						commentSW = "BOARD UNREACHABLE";
					}

					if(m_currentSw.compare("-") != 0)
					{
						m_currentSw = currentSw.substr((scx_Index+4));

						//debugging
						std::string pkg = stateMachine->getCurrentCXP();
						std::string trueSw = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(pkg);

						if (ACS_TRA_ON(traceObj))
						{
							char tmpStr[512] = {0};
							_snprintf(tmpStr, sizeof(tmpStr) - 1,
								"[%s@%d] m_currentSw: %s  -  pkg: %s  -  trueSw: %s "
								, __FUNCTION__, __LINE__,
								m_currentSw.c_str(), pkg.c_str(), trueSw.c_str());
							ACS_TRA_event(&traceObj, tmpStr);
						}

						if (trueSw != m_currentSw)
						{
							std::cout << "WARNING!! MISMATCH FOUND!!!" << std::endl;
						}
						//debugging
					}
				}

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "CURRENTSW = " << m_currentSw << std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "commentSW = " << commentSW << std::endl;

				if (m_currentSw.empty()) return 1;

				swInfo.currentSW = m_currentSw;
				swInfo.magazine = it->m_magazine;
				swInfo.slot = it->m_slot;
				swInfo.commentSW = commentSW;

				if (ACS_TRA_ON(traceObj))
				{
					char tmpStr[512] = {0};
					_snprintf(tmpStr, sizeof(tmpStr) - 1, 
						"[%s@%d] m_currentSw: %s  -  commentSW: %s "
						, __FUNCTION__, __LINE__, 
						m_currentSw.c_str(), commentSW.c_str());
					ACS_TRA_event(&traceObj, tmpStr);
				}
				currentSwList.push_back(swInfo);

			}
		}

	//look for EPB boards
	if(fbn == CCH_Util::EPB1 || fbn == CCH_Util::EPB2)
	{
		ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
		if (hwc)
		{
			cout<<"DBG: Got hwc instance"<<endl;
			ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
			if (boardSearch)
			{
				ACS_CS_API_IdList boardList;
				if (fbn == CCH_Util::EPB1) fbn = ACS_CS_API_HWC_NS::FBN_EPB1;
				if (fbn == CCH_Util::EPB2) fbn = ACS_CS_API_HWC_NS::FBN_EPB2;
				boardSearch->setFBN(fbn);
				ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
				if (returnValue == ACS_CS_API_NS::Result_Success)
				{
					cout<<"DBG: Got board list"<<endl;
					if (boardList.size() == 0)
					{
						//No matching boards for that magazine.
						cout<<"DBG: No boards found in HWC table"<<endl;
						return HARDWARE_NOT_PRESENT;
					}
					else
					{
						for (unsigned int i = 0; i < boardList.size(); i++)
						{
							unsigned long mag = 0;
							unsigned short slot = 0;
							std::string currentSw("");
							BoardID boardId = boardList[i];
							if(hwc->getMagazine(mag, boardId) != ACS_CS_API_NS::Result_Success)
								return NO_REPLY_FROM_CS;
							if(hwc->getSlot(slot, boardId) != ACS_CS_API_NS::Result_Success)
								return NO_REPLY_FROM_CS;
							if(hwc->getContainerPackage(currentSw, boardId) != ACS_CS_API_NS::Result_Success)
								return NO_REPLY_FROM_CS;
							currentSw = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(currentSw);
							//get magazine, slot, currentsw and push into vector
							swInfo.magazine = mag;
							swInfo.slot = slot;
							swInfo.currentSW = currentSw;
							swInfo.commentSW = " ";
							currentSwList.push_back(swInfo);

							if (ACS_TRA_ON(traceObj))
							{
								char tmpStr[512] = {0};
								_snprintf(tmpStr, sizeof(tmpStr) - 1, 
									"[%s@%d] currentSw: %s "
									, __FUNCTION__, __LINE__, currentSw.c_str());
								ACS_TRA_event(&traceObj, tmpStr);
							}
						}
					}
				}
			}
		}
		else return NO_REPLY_FROM_CS;

	}

	// look for IPTB board
	if ( fbn == CCH_Util::IPTB )
	{
		if(fetchIptInfo()==0)
			for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ", IPTB FOUND!!" << std::endl;
				FIXS_CCH_IptbUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
				}
				catch (bad_cast)
				{
					if (ACS_TRA_ON(traceObj))
					{
						char tmpStr[512] = {0};
						_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
						ACS_TRA_event(&traceObj, tmpStr);
					}
					stateMachine = 0;
				}


				if (stateMachine == 0) return INTERNALERR; // "Internal Error"

				m_currentState = stateMachine->getStateName();

				m_upgradeDateTimeInfo = stateMachine->getLastUpgradeDate() + " " + stateMachine->getLastUpgradeTime(); //collect of last SWupgrade date and time
				m_upgradeResult = stateMachine->getLastUpgradeResult();
				m_upgradeReason = stateMachine->getLastUpgradeReason(); 
				std::string pkg = stateMachine->getCurrentCXP();
				m_currentSw = FIXS_CCH_SoftwareMap::getInstance()->getSoftwareProduct(pkg);
				m_newSw = stateMachine->getExpectedCXC();

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "CURRENTSTATE = " << m_currentState.c_str() << std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "CURRENTSW = " << m_currentSw.c_str() << std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "NEWSW = " << m_newSw.c_str() << std::endl; 

				if (ACS_TRA_ON(traceObj))
				{
					char tmpStr[512] = {0};
					_snprintf(tmpStr, sizeof(tmpStr) - 1, 
						"[%s@%d] CURRENTSTATE: %s  -  CURRENTSW: %s  -  NEWSW: %s "
						, __FUNCTION__, __LINE__, 
						m_currentState.c_str(), m_currentSw.c_str(), m_newSw.c_str());
					ACS_TRA_event(&traceObj, tmpStr);
				}

				if (m_currentSw.empty()) return 1;
				swInfo.currentSW = m_currentSw;					
				swInfo.magazine = it->m_magazine;
				swInfo.slot = it->m_slot;
				swInfo.commentSW = " ";
				currentSwList.push_back(swInfo);

			} //for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	} //if ( fbn == IPTB)

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;

	if (currentSwList.empty()) return 14;
	else 
	{
		sortlist(currentSwList);
		return 0;
	}

	 */}

int FIXS_CCH_UpgradeManager::writeSwInfoFbn (char* buffer, int bufferSize, int fbn, std::vector<CCH_Util::SWINFO_FBN> currentSwList)
{

	UNUSED(fbn);
	UNUSED(currentSwList);
	UNUSED(buffer);
	UNUSED(bufferSize);

	return 0;
	/*

	char *tmpBuf = 0;
	int nItem = currentSwList.size()+ 1 ;
	int respSize = sizeof(nItem) + (ITEM_SIZE * nItem);
	std::string magStr, slotStr;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	if (respSize > bufferSize)
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
		return -1;
	}
	memcpy(buffer, &nItem, sizeof(nItem)); // write number of items, in this case always 1
	tmpBuf = buffer + sizeof(nItem);

	//write headerstring
	_snprintf(tmpBuf, ITEM_SIZE, "%-10s %-10s %-10s %-20s %-15s", "MAG", "SLOT", "FBN", "LOAD MODULE", "COMMENT");
	char row[ITEM_SIZE] = {0};
		if ( fbn == CCH_Util::SCXB )
		m_boardName = "SCXB";	
	else if ( fbn == CCH_Util::EPB1 )
		m_boardName = "EPB1";	
	else if ( fbn == CCH_Util::EPB2 )
		m_boardName = "EPB2";	
	else if ( fbn == CCH_Util::IPTB )
		m_boardName = "IPTB";	
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;

	if (ACS_TRA_ON(traceObj))
	{
		char tmpStr[512] = {0};
		_snprintf(tmpStr, sizeof(tmpStr) - 1, 
			"[%s@%d] ---------------------- Print-out ----------------------"
			, __FUNCTION__, __LINE__ );
		ACS_TRA_event(&traceObj, tmpStr);
	}

	for (vector<CCH_Util::SWINFO_FBN>::iterator it = currentSwList.begin(); it != currentSwList.end(); it++) 
	{
		tmpBuf = tmpBuf + ITEM_SIZE;
		CCH_Util::ulongToStringMagazine((*it).magazine, magStr);
		CCH_Util::ushortToString((*it).slot, slotStr);
		_snprintf(row, sizeof(row) - 1, "%-10s %-10s %-10s %-20s %-15s", magStr.c_str(), slotStr.c_str(), m_boardName.c_str(), (*it).currentSW.c_str(), (*it).commentSW.c_str() );
		_snprintf(tmpBuf, ITEM_SIZE, "%s", row);

		if (ACS_TRA_ON(traceObj))
		{
			ACS_TRA_event(&traceObj, row);
		}
	}

	return respSize;

	 */}

int FIXS_CCH_UpgradeManager::prepareEpb1 (unsigned long umagazine, unsigned short uslot, std::string container, int typeOfPrepare)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ",PREPARE EPB1" << std::endl;

	int retVal = 0;

	if(isBackupOngoing() == true){
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : NOT ALLOWED, BACKUP ONGOING ",LOG_LEVEL_ERROR);
		return NOT_ALLOWED_BACKUP_ONGOING;
	}

	std::string mag("");
	ACS_CS_API_SET_NS::CS_API_Set_Result result;

	// For multiple slots in the same magazine
	if (typeOfPrepare == CCH_Util::MULTIPLE_BOARD)
	{
		cout<<"DBG: PrepareEpb1 - Setting version for multiple EPB1 of magazine"<<endl;

		//string mag
		CCH_Util::ulongToStringMagazine(umagazine,mag);

		bool found = false;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if (it->m_magazine == umagazine)
			{
				found = true;
				uslot = it->m_slot;
				std::string old_container = it->m_container;

				if (isEpbBoard(umagazine, uslot))
				{
					result = ACS_CS_API_Set::setSoftwareVersion(container, mag, uslot);

					switch (result)
					{
					case ACS_CS_API_SET_NS::Result_Success:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_Success" << std::endl;

						//update usedBy reference
						IMM_Interface::updateUsedPackage(umagazine,uslot,old_container,container);

						//update into the list
						it->m_container = container;

						break;

					default:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_FAIL" <<result<< std::endl;
						retVal = NO_REPLY_FROM_CS;
						break;
					}

					if (retVal == NO_REPLY_FROM_CS) return retVal;
				}
			}
		}

		if (!found) 
		{
			std::string mag,slot;
			CCH_Util::ulongToStringMagazine(umagazine,mag);
			CCH_Util::ushortToString(uslot,slot);
			if(CCH_Util::isBoardPresent(mag,slot))
				return PACKAGE_NOT_APPLICABLE;
			else
				return  HARDWARE_NOT_PRESENT;
		}

		//update into map and model
		//		FIXS_CCH_SoftwareMap::getInstance()->updateUsedEpb1Containers();
	}

	// For all boards
	else if (typeOfPrepare == CCH_Util::ALL_MAGAZINE_BOARD)
	{
		cout<<"DBG: PrepareEpb1 - Setting version for all EPB1"<<endl;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if (it->m_fbn == CCH_Util::EPB1)
			{
				umagazine = it->m_magazine;
				uslot = it->m_slot;
				std::string old_container = it->m_container;

				//convert ulong magazine to string
				CCH_Util::ulongToStringMagazine(umagazine,mag);

				result = ACS_CS_API_Set::setSoftwareVersion(container, mag, uslot);

				switch (result)
				{
				case ACS_CS_API_SET_NS::Result_Success:
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_Success" << std::endl;

					//update usedBy reference
					IMM_Interface::updateUsedPackage(umagazine,uslot,old_container,container);

					//update into the list
					it->m_container = container;

					break;

				default:
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_FAIL" <<result<< std::endl;
					retVal = NO_REPLY_FROM_CS;
					break;
				}

				if (retVal == NO_REPLY_FROM_CS) return retVal;
			}
		}

		//update into map and model
		//		FIXS_CCH_SoftwareMap::getInstance()->updateUsedEpb1Containers();
	}

	// For single board
	else if(typeOfPrepare == CCH_Util::SINGLE_BOARD)
	{
		cout<<"DBG: PrepareEpb1 - Setting version for a single EPB1"<<endl;

		bool found = false;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if (it->m_fbn == CCH_Util::EPB1)
			{
				if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
				{
					found = true;

					std::string old_container = it->m_container;

					//convert ulong magazine to string
					CCH_Util::ulongToStringMagazine(umagazine,mag);

					cout<<"DBG: PrepareEpb1 - Setting version for magazine = "<< mag.c_str() << " slot: " << uslot << endl;
					result = ACS_CS_API_Set::setSoftwareVersion(container, mag, uslot);

					switch (result)
					{
					case ACS_CS_API_SET_NS::Result_Success:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_Success" << std::endl;

						//update usedBy reference
						IMM_Interface::updateUsedPackage(umagazine,uslot,old_container,container);

						//update into the list
						it->m_container = container;

						retVal = 0;
						break;
					default:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_FAIL" <<result<< std::endl;
						retVal = NO_REPLY_FROM_CS;
						break;
					}

					break;
				}
			}
		}
		if (!found)
		{
			std::string mag,slot;
			CCH_Util::ulongToStringMagazine(umagazine,mag);
			CCH_Util::ushortToString(uslot,slot);
			if(CCH_Util::isBoardPresent(mag,slot))
				return PACKAGE_NOT_APPLICABLE;
			else
				return  HARDWARE_NOT_PRESENT;
		}

	}

	return retVal;
}

int FIXS_CCH_UpgradeManager::prepareEvoEt (unsigned long umagazine, unsigned short uslot, std::string container, int typeOfPrepare)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ",PREPARE EVOET" << std::endl;

	int retVal = 0;

	if(isBackupOngoing() == true){
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : NOT ALLOWED, BACKUP ONGOING ",LOG_LEVEL_ERROR);
		return NOT_ALLOWED_BACKUP_ONGOING;
	}

	std::string mag("");
	ACS_CS_API_SET_NS::CS_API_Set_Result result;

	// For multiple slots in the same magazine
	if (typeOfPrepare == CCH_Util::MULTIPLE_BOARD)
	{
		cout<<"DBG: prepareEvoEt - Setting version for multiple EVOET of magazine"<<endl;

		//string mag
		CCH_Util::ulongToStringMagazine(umagazine,mag);

		bool found = false;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if (it->m_magazine == umagazine)
			{
				found = true;
				uslot = it->m_slot;
				std::string old_container = it->m_container;

				if (isEvoEtData(umagazine, uslot))
				{
					result = ACS_CS_API_Set::setSoftwareVersion(container, mag, uslot);

					switch (result)
					{
					case ACS_CS_API_SET_NS::Result_Success:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_Success" << std::endl;

						//update usedBy reference
						IMM_Interface::updateUsedPackage(umagazine,uslot,old_container,container);

						//update into the list
						it->m_container = container;

						break;

					default:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_FAIL" <<result<< std::endl;
						retVal = NO_REPLY_FROM_CS;
						break;
					}

					if (retVal == NO_REPLY_FROM_CS) return retVal;
				}
			}
		}

		if (!found)
		{
			std::string mag,slot;
			CCH_Util::ulongToStringMagazine(umagazine,mag);
			CCH_Util::ushortToString(uslot,slot);
			if(CCH_Util::isBoardPresent(mag,slot))
				return PACKAGE_NOT_APPLICABLE;
			else return  HARDWARE_NOT_PRESENT;
		}

		//update into map and model
	}

	// For all boards
	else if (typeOfPrepare == CCH_Util::ALL_MAGAZINE_BOARD)
	{
		cout<<"DBG: PrepareEvoET - Setting version for all EvoET"<<endl;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if (it->m_fbn == CCH_Util::EVOET)
			{
				umagazine = it->m_magazine;
				uslot = it->m_slot;
				std::string old_container = it->m_container;

				//convert ulong magazine to string
				CCH_Util::ulongToStringMagazine(umagazine,mag);

				result = ACS_CS_API_Set::setSoftwareVersion(container, mag, uslot);

				switch (result)
				{
				case ACS_CS_API_SET_NS::Result_Success:
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_Success" << std::endl;

					//update usedBy reference
					IMM_Interface::updateUsedPackage(umagazine,uslot,old_container,container);

					//update into the list
					it->m_container = container;

					break;

				default:
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_FAIL" <<result<< std::endl;
					retVal = NO_REPLY_FROM_CS;
					break;
				}

				if (retVal == NO_REPLY_FROM_CS) return retVal;
			}
		}

		//update into map and model

	}

	// For single board
	else if(typeOfPrepare == CCH_Util::SINGLE_BOARD)
	{
		cout<<"DBG: PrepareEvoET - Setting version for a single EvoET"<<endl;

		bool found = false;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if (it->m_fbn == CCH_Util::EVOET)
			{
				if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
				{
					found = true;

					std::string old_container = it->m_container;

					//convert ulong magazine to string
					CCH_Util::ulongToStringMagazine(umagazine,mag);

					cout<<"DBG: PrepareEpb1 - Setting version for magazine = "<< mag.c_str() << " slot: " << uslot << endl;
					result = ACS_CS_API_Set::setSoftwareVersion(container, mag, uslot);

					switch (result)
					{
					case ACS_CS_API_SET_NS::Result_Success:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_Success" << std::endl;

						//update usedBy reference
						IMM_Interface::updateUsedPackage(umagazine,uslot,old_container,container);

						//update into the list
						it->m_container = container;

						retVal = 0;
						break;
					default:
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ACS_CS_API_SET_NS::Result_FAIL" <<result<< std::endl;
						retVal = NO_REPLY_FROM_CS;
						break;
					}

					break;
				}
			}
		}
		if (!found)
		{
			std::string mag,slot;
			CCH_Util::ulongToStringMagazine(umagazine,mag);
			CCH_Util::ushortToString(uslot,slot);
			if(CCH_Util::isBoardPresent(mag,slot))
				return PACKAGE_NOT_APPLICABLE;
			else
				return HARDWARE_NOT_PRESENT;
		}

	}

	return retVal;
}

bool FIXS_CCH_UpgradeManager::tokenize (const string &str, vector <unsigned short>& tokens, const string &delimiters)
{
	UNUSED(str);
	UNUSED(tokens);
	UNUSED(delimiters);


	return true;
	/*

	unsigned short uslot = 0;
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
{
    {
        // Found a token, add it to the vector.
		if(getUshortSlot(str.substr(lastPos, pos - lastPos), uslot)==false)
			return false;
        tokens.push_back(uslot);
       // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
	return true;

	 */}

void FIXS_CCH_UpgradeManager::sortlist (std::vector<CCH_Util::SWINFO_FBN> &currentSwList)
{

	UNUSED(currentSwList);

	/*
	 vector<CCH_Util::SWINFO_FBN> tempPrintOutVector;     //temp vector holding the printout structs
	vector<CCH_Util::SWINFO_FBN>::iterator tempIterator;
	int leastVal = 0;
	int loop = (int)currentSwList.size() - 1;
	unsigned long temp1,temp2;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;

	if (loop <= 0)  
	{	
		//vector only contains 1 board, i.e. list sorted
		std::cout << "DBG: "<<endl<<"vector contains only one element"<<endl;
		return ;
	}

	for (int i = 0; i < loop; i++)
	{
		for (int j = 0; j < (int)currentSwList.size() - 1; j++)
		{
			temp1 = ntohl (currentSwList.at(leastVal).magazine);
			temp2 = ntohl (currentSwList.at(j+1).magazine) ;
			if (temp1 > temp2)
				leastVal = j + 1;
			else if ( temp1 == temp2 )
			{
				if (currentSwList.at(leastVal).slot > currentSwList.at(j+1).slot)
					leastVal = j + 1;
			}
		}
		CCH_Util::SWINFO_FBN  stemp;
		stemp = currentSwList.at(leastVal);   //gets the board with the least mag and slot addr from the vector
		tempPrintOutVector.push_back(stemp);      //add the board with the least mag and slot addr last in the tempvector
		tempIterator = currentSwList.begin();
		tempIterator = tempIterator + leastVal;
		(void)currentSwList.erase(tempIterator);       //erases the board with the least mag and slot addr from the vector
		leastVal = 0;  //clear the leastVal in order to start with first vector element in next loop

		if (currentSwList.size() == 1)        //if there's only one board left in the vector
		{
			stemp = currentSwList.at(0); //gets the board from the vector
			tempPrintOutVector.push_back(stemp);         //add the board in the tempvector
			tempIterator = currentSwList.begin();
			(void)currentSwList.erase(tempIterator);          //erases the board from the vector  
		}
	}

	currentSwList.assign(tempPrintOutVector.begin(), tempPrintOutVector.end());  //assigns the tempVector to the original
   // ends the sorting process


	 */}

int FIXS_CCH_UpgradeManager::fetchBoardIdList (unsigned long mag, vector <unsigned short> &uSlotIntVec, vector <unsigned short> &boardIdList, int boardName, int type)
{

	UNUSED(mag);
	UNUSED(uSlotIntVec);
	UNUSED(boardIdList);
	UNUSED(boardName);
	UNUSED(type);

	return 0;
	/*

	unsigned short tempSlot = -1;
	unsigned short fbn;
	short oldFbn = -1;
	bool found = false;


	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		cout<<"DBG: fetchBoardIdList - Got hwc instance"<<endl;
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			if(type == 3)
			{
				boardSearch->setMagazine(mag);
			}
			else if(type == 1)
			{
				boardSearch->setFBN(boardName);
			}
			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				cout<<"DBG: fetchBoardIdList - Got board list"<<endl;
				if (boardList.size() == 0)
				{
					//No matching boards for that magazine.
					cout<<"DBG: fetchBoardIdList - No boards found in HWC table"<<endl;
					return HARDWARE_NOT_PRESENT;
				}
				else
				{
					//for (unsigned int i = 0; i < boardList.size(); i++)
					if(type == 3 )
					{
						for(vector<unsigned short>::iterator it = uSlotIntVec.begin(); it!=uSlotIntVec.end(); it++)
						{
							found = false;
							for (unsigned int i = 0; i < boardList.size(); i++)
							{
								BoardID boardId = boardList[i];
								if((hwc->getSlot(tempSlot, boardId)) != ACS_CS_API_NS::Result_Success) 
									return NO_REPLY_FROM_CS;
								if(tempSlot == *it)
									{
										found = true;
										unsigned long tempMag = 0;

										if((hwc->getFBN(fbn, boardId))!= ACS_CS_API_NS::Result_Success) 
											return NO_REPLY_FROM_CS;
										if(oldFbn == -1)
										{
											if (fbn != ACS_CS_API_HWC_NS::FBN_EPB1 && fbn != ACS_CS_API_HWC_NS::FBN_EPB2)
											{
												cout<<"DBG: FBN is not EPB1 or EPB2"<<endl;
												return FUNC_BOARD_NOTALLOWED;
											}
										oldFbn = fbn;
										if(fbn == ACS_CS_API_HWC_NS::FBN_EPB1) m_boardName = "EPB1";
										if(fbn == ACS_CS_API_HWC_NS::FBN_EPB2) m_boardName = "EPB2";
									}
									else if(fbn != oldFbn)
									{
										return SLOT_FBN_NOT_SAME;
									}
									boardIdList.push_back(boardId);
								}	
							}
							if (!found)
								return HARDWARE_NOT_PRESENT;
					}//for
					}
					else if(type == 1)
					{
						for (unsigned int i = 0; i < boardList.size(); i++)
						{
							BoardID boardId = boardList[i];
							boardIdList.push_back(boardId);
						}
					}
				}
			}
			else return NO_REPLY_FROM_CS;
		}
		else return NO_REPLY_FROM_CS;
	}
	else return NO_REPLY_FROM_CS;

	return 0;

	 */}

int FIXS_CCH_UpgradeManager::isEpbBoard (unsigned long magazine, unsigned short slot)
{
	Lock lock;
	bool result = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			if (it->m_fbn == CCH_Util::EPB1)
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

int FIXS_CCH_UpgradeManager::checkOccurrenceScxContainer (std::string container)
{
	int num = 0;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (strcmp(it->m_container.c_str(),container.c_str()) == 0) num++;
	}
	return num;
}
bool FIXS_CCH_UpgradeManager::checkAllSCXandCMXSwVer (unsigned short fbn1, unsigned short fbn2, int timeoutFlag)
{
	char traceChar[512] = {0};
	snprintf(traceChar, sizeof(traceChar) - 1,"checkAll SCX and CMX");
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);

	bool res = true;
	bool scxAlignment = true;
	bool cmxAlignment = true;
	std::string sCXCToCompareSCX;
	std::string sCXCToCompareCMX;

	if (m_swUpgradeList->size() <= 0)
	{
		return true;
	}

	if((fbn1 == CCH_Util::SCXB)&&(fbn2 != CCH_Util::SCXB) && (fbn1 != CCH_Util::CMXB) && (fbn2 != CCH_Util::CMXB))
	{
		return res;
	}

	//			 Lock lock;
	if((fbn1 == CCH_Util::SCXB)||(fbn2 == CCH_Util::SCXB))
	{
		bool foundScx = false;
		FIXS_CCH_ScxUpgrade *stateMachine = 0;
		swUpgradeList_t::iterator firstSM = m_swUpgradeList->begin();
		for (; firstSM != m_swUpgradeList->end(); firstSM++)
		{
			if (firstSM->m_fbn == CCH_Util::SCXB)
			{
				foundScx = true;
				break;
			}
		}

		if (foundScx)
		{
			//stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (firstSM->m_swPtr);
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (firstSM->m_swPtr);
			}
			catch (bad_cast)
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. Failed to check all SCXB software version", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				stateMachine = 0;
				res = false;
				scxAlignment = false;
			}

			if (stateMachine)
			{
				//sCXCToCompare = stateMachine->getCurrentRunningSoftware();
				sCXCToCompareSCX = stateMachine->getCurrentCXP();

				std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
				std::cout << "	  sCXCToCompareSCX: " << sCXCToCompareSCX.c_str() << std::endl;
				std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

				for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
				{	
					if (it->m_fbn == CCH_Util::SCXB)
					{
						FIXS_CCH_ScxUpgrade *stateMachine = 0;
						try
						{
							stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
						}
						catch (bad_cast)
						{
							if (_trace->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
								_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
							}

							stateMachine = 0;
							res = false;
							scxAlignment = false;
							break;
						}

						if (stateMachine)
						{

							//						 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
							//						 std::cout << std::endl
							//								 << " getCurrentCXC = " << stateMachine->getCurrentCXC() << std::endl
							//								 << " getCurrentCXP = " << stateMachine->getCurrentCXP() << std::endl
							//								 << " getExpectedCXC = " << stateMachine->getExpectedCXC() << std::endl
							//								 << " getExpectedCXC = " << stateMachine->getExpectedCXP() << std::endl
							//								 << " " << std::endl;
							//						 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

							std::string cxcTmp = stateMachine->getCurrentRunningSoftware();
							cxcTmp.clear();
							cxcTmp = stateMachine->getCurrentCXP();
							std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
							std::cout << " COMPARE With SCX SW: " << cxcTmp.c_str() << std::endl;
							std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1,
										"[%s@%d] cxcTmp: %s -  getCurrentRunningSoftware: %s "
										" -  getCurrentCXC: %s  -  getCurrentCXP: %s "
										" -  getExpectedCXC: %s  -  getExpectedCXC: %s "
										, __FUNCTION__, __LINE__,
										cxcTmp.c_str(),stateMachine->getCurrentRunningSoftware().c_str(),
										stateMachine->getCurrentCXC().c_str(), stateMachine->getCurrentCXP().c_str(),
										stateMachine->getExpectedCXC().c_str(), stateMachine->getExpectedCXP().c_str());
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
							}


							if (strcmp(cxcTmp.c_str(), sCXCToCompareSCX.c_str()) != 0)
							{
								std::cout << " ---------------------------------- " << std::endl;
								std::cout << "  DIFFERENCE FROM : " << sCXCToCompareSCX.c_str() << " TO : " << cxcTmp.c_str() << std::endl;
								std::cout << " ---------------------------------- " << std::endl;
								scxAlignment = false;
								break;
							}
							stateMachine = 0;

						} //if (stateMachine)
						else
						{
							scxAlignment = false;
							res = false;
							break;
						}
					}
				} //for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
			} //if (stateMachine)
		}
	}


	if((fbn1 == CCH_Util::CMXB)||(fbn2 == CCH_Util::CMXB))
	{
		FIXS_CCH_CmxUpgrade *stateMachine = 0;
		bool foundCmx = false;
		swUpgradeList_t::iterator firstSM = m_swUpgradeList->begin();
		for (; firstSM != m_swUpgradeList->end(); firstSM++)
		{
			if (firstSM->m_fbn == CCH_Util::CMXB)
			{
				foundCmx = true;
				break;
			}
		}
		if(foundCmx)
		{
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (firstSM->m_swPtr);
			}
			catch (bad_cast)
			{
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
					_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				stateMachine = 0;
				res = false;
				cmxAlignment = false;
			}
			if (stateMachine)
			{
				sCXCToCompareCMX = stateMachine->getCurrentCXP();

				std::cout << " sCXCToCompareCMX: " << sCXCToCompareCMX << std::endl;

				for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++) 
				{
					if (it->m_fbn == CCH_Util::CMXB)
					{
						try
						{
							stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);
						}
						catch (bad_cast)
						{
							if (_trace->ACS_TRA_ON())
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
								_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
							}
							stateMachine = 0;
							res = false;
							cmxAlignment = false;
							break;
						}
						if (stateMachine)
						{
							std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
							std::cout << std::endl
									<< " getCurrentCXC = " << stateMachine->getCurrentCXC() << std::endl
									<< " getCurrentCXP = " << stateMachine->getCurrentCXP() << std::endl
									<< " getExpectedCXC = " << stateMachine->getExpectedCXC() << std::endl
									<< " getExpectedCXC = " << stateMachine->getExpectedCXP() << std::endl
									<< " getCurrentRunningSoftware = " << stateMachine->getCurrentRunningSoftware()
									<< " " << std::endl;
							std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
							std::string cxcTmp = stateMachine->getCurrentCXP();

							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1,
									"[%s@%d] cxcTmp: %s -  getCurrentRunningSoftware: %s "
									" -  getCurrentCXC: %s  -  getCurrentCXP: %s "
									" -  getExpectedCXC: %s  -  getExpectedCXC: %s "
									, __FUNCTION__, __LINE__,
									cxcTmp.c_str(),stateMachine->getCurrentRunningSoftware().c_str(),
									stateMachine->getCurrentCXC().c_str(), stateMachine->getCurrentCXP().c_str(),
									stateMachine->getExpectedCXC().c_str(), stateMachine->getExpectedCXP().c_str());
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);

							if (strcmp(cxcTmp.c_str(), sCXCToCompareCMX.c_str()) != 0)
							{
								std::cout << " ---------------------------------- " << std::endl;
								std::cout << "  DIFFERENCE FROM : " << sCXCToCompareCMX.c_str() << " TO : " << cxcTmp.c_str() << std::endl;
								std::cout << " ---------------------------------- " << std::endl;
								cmxAlignment = false;
								break;
							}
							stateMachine = 0;
						}
						else //if (stateMachine)
						{
							cmxAlignment = false;
							res = false;
							break;
						}
					}
				} //for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++) 
			} //if (stateMachine)
		}
	}


	if ((scxAlignment) && (cmxAlignment))
	{
		std::cout << " ================================== " << std::endl;
		std::cout << " scxAlignment and cmxAlignment true ... cease the alarm " << __LINE__<<std::endl;
		std::cout << " ================================== " << std::endl;

		// stop all timer and alarm cease

		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Alarm OFF ", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		//if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		std::cout << " ================================== " << std::endl;
		std::cout << " scx and cmx Alignment OK ... alarm OFF " << __LINE__<<std::endl;
		std::cout << " ================================== " << std::endl;
		//stop all timers
		if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== true)
			m_alarmHandler->cancelEgem2L2SwitchAlarmTimer();

		if(m_alarmHandler->isCMXAlarmTimerOngoing()== true)
			m_alarmHandler->cancelCmxAlarmTimer();

		//cease all alarm
		int problemScx = 35000;
		unsigned int severityCeasingScx = CCH_Util::Severity_CEASING;
		std::string causeScx = "SWITCHING BOARD SW MISALIGNMENT";
		std::string dataScx = "SW Fault";
		std::string textScx = "NOT ALL SCX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
		sendAlarm (problemScx, severityCeasingScx, causeScx, dataScx, textScx);

		int problemCmx = 35001;
		unsigned int severityCeasingCmx = CCH_Util::Severity_CEASING;
		std::string causeCmx = "SWITCHING BOARD SW MISALIGNMENT";
		std::string dataCmx = "SW Fault";
		std::string textCmx = "NOT ALL CMX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
		sendAlarm (problemCmx, severityCeasingCmx, causeCmx, dataCmx, textCmx);

	}
	else if ((!scxAlignment) && (!cmxAlignment))
	{

		std::cout << " ================================== " << std::endl;
		std::cout << " scxAlignment and cmxAlignment false ... raise the alarm " <<__LINE__<< std::endl;
		std::cout << " ================================== " << std::endl;

		if(timeoutFlag == CCH_Util::CMX_ALARM_TIMEOUT) //only cmx timer timed out
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CMX timer timed out ", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);

			// start Scx timer
			if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== false)
				m_alarmHandler->startEgem2L2SwitchAlarmTimer();

			std::cout << " ================================== " << std::endl;
			std::cout << "raise cmx alarm............ " <<__LINE__<< std::endl;
			std::cout << " ================================== " << std::endl;
			int problem = 35001;
			//raise alarm
			unsigned int severity = CCH_Util::Severity_O1;
			std::string cause = "SWITCHING BOARD SW MISALIGNMENT";
			std::string data = "SW Fault";
			std::string text = "NOT ALL CMX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problem, severity, cause, data, text);
		}
		else if(timeoutFlag == CCH_Util::SCX_ALARM_TIMEOUT) // only scx timer timed out
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] SCX timer timed out ", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);

			// start Cmx timer
			if(m_alarmHandler->isCMXAlarmTimerOngoing()== false)
				m_alarmHandler->startCmxAlarmTimer();

			int problem = 35000;
			std::cout << " ================================== " << std::endl;
			std::cout << "raise scx alarm............ " << __LINE__<<std::endl;
			std::cout << " ================================== " << std::endl;
			//raise alarm

			unsigned int severity = CCH_Util::Severity_O1;
			std::string cause = "SWITCHING BOARD SW MISALIGNMENT";
			std::string data = "SW Fault";
			std::string text = "NOT ALL SCX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problem, severity, cause, data, text);

		}
		else  //timeoutFlag = 0 
		{
			std::cout << "start scx and cmx timer............ " << __LINE__<<std::endl;
			// start scx alarm timer
			if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== false)
				m_alarmHandler->startEgem2L2SwitchAlarmTimer();

			// start cmx alarm timer
			if(m_alarmHandler->isCMXAlarmTimerOngoing()== false)
				m_alarmHandler->startCmxAlarmTimer();
		}
	}	
	else if (scxAlignment)
	{
		if (timeoutFlag == CCH_Util::CMX_ALARM_TIMEOUT) //only cmx timer timed out
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CMX timer timed out ", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			int problemCmx = 35001;
			std::cout << " ================================== " << std::endl;
			std::cout << " cmxAlignment false ... raise cmx the alarm " <<__LINE__<< std::endl;
			std::cout << " ================================== " << std::endl;
			//raise alarm
			unsigned int severityCmx = CCH_Util::Severity_O1;
			std::string causeCmx = "SWITCHING BOARD SW MISALIGNMENT";
			std::string dataCmx = "SW Fault";
			std::string textCmx = "NOT ALL CMX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemCmx, severityCmx, causeCmx, dataCmx, textCmx);

			// Cancel Scx timer
			if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== false)
				m_alarmHandler->cancelEgem2L2SwitchAlarmTimer();

			//cease SCX alarm
			std::cout << " ================================== " << std::endl;
			std::cout << " Cease scx alarm.......... " << __LINE__<<std::endl;
			std::cout << " ================================== " << std::endl;
			int problemScx = 35000;
			unsigned int severityCeasingScx = CCH_Util::Severity_CEASING;
			std::string causeScx = "SWITCHING BOARD SW MISALIGNMENT";
			std::string dataScx = "SW Fault";
			std::string textScx = "NOT ALL SCX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemScx, severityCeasingScx, causeScx, dataScx, textScx);
		}
		else if (timeoutFlag == CCH_Util::SCX_ALARM_TIMEOUT) //scx timer timed out
		{

			//cease SCX alarm
			std::cout << " ================================== " << std::endl;
			std::cout << " Cease scx alarm.......... " <<__LINE__<< std::endl;
			std::cout << " ================================== " << std::endl;
			int problemScx = 35000;
			unsigned int severityCeasingScx = CCH_Util::Severity_CEASING;
			std::string cause = "SWITCHING BOARD SW MISALIGNMENT";
			std::string data = "SW Fault";
			std::string text = "NOT ALL SCX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemScx, severityCeasingScx, cause, data, text);

			// start cmx alarm timer
			if(m_alarmHandler->isCMXAlarmTimerOngoing()== false)
				m_alarmHandler->startCmxAlarmTimer();

		}
		else // NO_ALARM_TIMEOUT
		{
			//SCX alarm off and start CMX timer
			std::cout << " ================================== " << std::endl;
			std::cout << " scx Alignment OK ... ........ " << __LINE__<<std::endl;
			std::cout << " ================================== " << std::endl;

			//stop SCX alarm timer if ongoing
			if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== true)
			{
				m_alarmHandler->cancelEgem2L2SwitchAlarmTimer();
			}

			//cease SCX alarm
			std::cout << " ================================== " << std::endl;
			std::cout << " Cease scx alarm.......... " << __LINE__<<std::endl;
			std::cout << " ================================== " << std::endl;
			int problemScx = 35000;
			unsigned int severityCeasingScx = CCH_Util::Severity_CEASING;
			std::string cause = "SWITCHING BOARD SW MISALIGNMENT";
			std::string data = "SW Fault";
			std::string text = "NOT ALL SCX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemScx, severityCeasingScx, cause, data, text);

			// start cmx alarm timer
			if(m_alarmHandler->isCMXAlarmTimerOngoing()== false)
				m_alarmHandler->startCmxAlarmTimer();

		}

	}
	else if (cmxAlignment)
	{
		if (timeoutFlag == CCH_Util::CMX_ALARM_TIMEOUT) //only cmx timer timed out
		{		
			//Cease CMX alarm
			int problemCmx = 35001;
			unsigned int severityCeasingCmx = CCH_Util::Severity_CEASING;
			std::string cause = "SWITCHING BOARD SW MISALIGNMENT";
			std::string data = "SW Fault";
			std::string text = "NOT ALL CMX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemCmx, severityCeasingCmx, cause, data, text);

			std::cout<<"start Scx AlarmTimer"<<__LINE__<<endl;// start scx alarm timer
			if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== false)
				m_alarmHandler->startEgem2L2SwitchAlarmTimer();
		}
		else if (timeoutFlag == CCH_Util::SCX_ALARM_TIMEOUT) //only scx timer timed out
		{
			// stop cmx timer
			if(m_alarmHandler->isCMXAlarmTimerOngoing()== true)
				m_alarmHandler->cancelCmxAlarmTimer();

			//raise SCX alarm
			int problemScx = 35000;
			std::cout << " ================================== " << std::endl;
			std::cout << " scxAlignment false ... raise scx alarm " <<__LINE__<< std::endl;
			std::cout << " ================================== " << std::endl;
			//raise alarm
			unsigned int severityScx = CCH_Util::Severity_O1;
			std::string causeScx = "SWITCHING BOARD SW MISALIGNMENT";
			std::string dataScx = "SW Fault";
			std::string textScx = "NOT ALL SCX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemScx, severityScx, causeScx, dataScx, textScx);

			//cease CMX alarm
			int problemCmx = 35001;
			unsigned int severityCeasingCmx = CCH_Util::Severity_CEASING;
			std::string causeCmx = "SWITCHING BOARD SW MISALIGNMENT";
			std::string dataCmx = "SW Fault";
			std::string textCmx = "NOT ALL CMX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemCmx, severityCeasingCmx, causeCmx, dataCmx, textCmx);
		}
		else //NO_ALARM_TIMEOUT
		{
			//CMX alarm off and start SCX timer
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CMX alignment OK ", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			std::cout << " ================================== " << std::endl;
			std::cout << " cmx Alignment OK ..............  " <<__LINE__<< std::endl;
			std::cout << " ================================== " << std::endl;

			//stop CMX alarm timer if ongoing
			if(m_alarmHandler->isCMXAlarmTimerOngoing()== true)
			{
				m_alarmHandler->cancelCmxAlarmTimer();
			}

			//cease CMX alarm
			int problemCmx = 35001;
			unsigned int severityCeasingCmx = CCH_Util::Severity_CEASING;
			std::string causeCmx = "SWITCHING BOARD SW MISALIGNMENT";
			std::string dataCmx = "SW Fault";
			std::string textCmx = "NOT ALL CMX BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemCmx, severityCeasingCmx, causeCmx, dataCmx, textCmx);

			// start scx alarm timer
			if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== false)
				m_alarmHandler->startEgem2L2SwitchAlarmTimer();
		}
	}

	return res;
}
bool FIXS_CCH_UpgradeManager::checkIPLBswVr()
{
	bool res = true;
	char traceChar[512] = {0};
	snprintf(traceChar, sizeof(traceChar) - 1,"checkboth IPLB Boards");
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);



	bool IplbAlignment = true;


	std::string sCXCToCompareIPLB;

	if (m_swUpgradeList->size() <= 0)
	{
		return true;
	}
	bool foundIplb = false;
	FIXS_CCH_IplbUpgrade *stateMachine = 0;
	swUpgradeList_t::iterator firstSM = m_swUpgradeList->begin();
	for (; firstSM != m_swUpgradeList->end(); firstSM++)
	{
		if (firstSM->m_fbn == CCH_Util::IPLB)
		{
			foundIplb = true;
			break;
		}
	}

	if (foundIplb)
	{
		try
		{
			stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (firstSM->m_swPtr);
		}
		catch (bad_cast)
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. Failed to check all IPLB software version", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			stateMachine = 0;
			res = false;
			IplbAlignment = false;
		}

		if (stateMachine)
		{

			sCXCToCompareIPLB = stateMachine->getCurrentCXP();

			std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
			std::cout << "    sCXCToCompareIPLB: " << sCXCToCompareIPLB.c_str() << std::endl;
			std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

			for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
			{
				if (it->m_fbn == CCH_Util::IPLB)
				{
					FIXS_CCH_IplbUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);
					}
					catch (bad_cast)
					{
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IplbUpgrade", __FUNCTION__, __LINE__);

							_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						stateMachine = 0;
						res = false;
						IplbAlignment = false;
						break;
					}
					if (stateMachine)
					{

						//                                               std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
						//                                               std::cout << std::endl
						//                                                               << " getCurrentCXC = " << stateMachine->getCurrentCXC() << std::endl
						//                                                               << " getCurrentCXP = " << stateMachine->getCurrentCXP() << std::endl
						//                                                               << " getExpectedCXC = " << stateMachine->getExpectedCXC() << std::endl
						//                                                               << " getExpectedCXC = " << stateMachine->getExpectedCXP() << std::endl
						//                                                               << " " << std::endl;
						//                                               std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

						std::string cxcTmp = stateMachine->getCurrentCXP();
						std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
						std::cout << " COMPARE With IPLB SW: " << cxcTmp.c_str() << std::endl;
						std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;


						if (strcmp(cxcTmp.c_str(), sCXCToCompareIPLB.c_str()) != 0)
						{
							std::cout << " ---------------------------------- " << std::endl;
							std::cout << "  DIFFERENCE FROM : " << sCXCToCompareIPLB.c_str() << " TO : " << cxcTmp.c_str() << std::endl;
							std::cout << " ---------------------------------- " << std::endl;
							IplbAlignment = false;
							break;
						}
						stateMachine = 0;

					} //if (stateMachine)
					else
					{
						IplbAlignment = false;
						res = false;
						break;
					}
				}
			} //for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		} //if (stateMachine)
	}


	if (!IplbAlignment)
	{
		std::cout << " ================================== " << std::endl;
		std::cout << " iplbAlignment false ... raise the alarm " <<__LINE__<< std::endl;
		std::cout << " ================================== " << std::endl;



		std::cout << " ================================== " << std::endl;
		std::cout << "raise IPLB alarm............ " <<__LINE__<< std::endl;
		std::cout << " ================================== " << std::endl;
		int problem = 35002;

		//raise alarm
		unsigned int severity = CCH_Util::Severity_O1;
		std::string cause = "IPLB SW MISALIGNMENT";
		std::string data = "SW Fault";
		std::string text = "NOT ALL IPLB ARE RUNNING ON THE SAME SOFTWARE LEVEL";
		sendAlarm (problem, severity, cause, data, text);


	}
	else
	{
		std::cout << " ================================== " << std::endl;
		std::cout << " IPLBAlignment true ... cease the alarm " << __LINE__<<std::endl;
		std::cout << " ================================== " << std::endl;

		// alarm cease

		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Alarm OFF ", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		//if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		std::cout << " ================================== " << std::endl;
		std::cout << " IPLB Alignment OK ... alarm OFF " << __LINE__<<std::endl;
		std::cout << " ================================== " << std::endl;

		//cease all alarm
		int problemIplb = 35002;
		unsigned int severityCeasingIplb = CCH_Util::Severity_CEASING;
		std::string causeIplb = "IPLB SW MISALIGNMENT";
		std::string dataIplb = "SW Fault";
		std::string textIplb = "NOT ALL IPLB ARE RUNNING ON THE SAME SOFTWARE LEVEL";
		sendAlarm (problemIplb, severityCeasingIplb, causeIplb, dataIplb, textIplb);


	}

	return res;
}
bool FIXS_CCH_UpgradeManager::checkSMXswVr(int timeoutFlag)
{
	bool res = true;
	char traceChar[512] = {0};
	snprintf(traceChar, sizeof(traceChar) - 1,"checkboth SMX Boards");
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);

	bool smxAlignment = true;

	std::string sCXCToCompareSMX;

	if (m_swUpgradeList->size() <= 0)
	{
		return true;
	}
	bool foundSmx = false;
	FIXS_CCH_SmxUpgrade *stateMachine = 0;
	swUpgradeList_t::iterator firstSM = m_swUpgradeList->begin();
	for (; firstSM != m_swUpgradeList->end(); firstSM++)
	{
		if (firstSM->m_fbn == CCH_Util::SMXB)
		{
			foundSmx = true;
			break;
		}
	}

	if (foundSmx)
	{
		try
		{
			stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (firstSM->m_swPtr);
		}
		catch (bad_cast)
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. Failed to check all SMXB software version", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			stateMachine = 0;
			res = false;
			smxAlignment = false;
		}

		if (stateMachine)
		{

			sCXCToCompareSMX = stateMachine->getCurrentCXP();

			std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
			std::cout << "    sCXCToCompareSMX: " << sCXCToCompareSMX.c_str() << std::endl;
			std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

			for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
			{
				if (it->m_fbn == CCH_Util::SMXB)
				{
					FIXS_CCH_SmxUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
					}
					catch (bad_cast)
					{
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_SmxUpgrade", __FUNCTION__, __LINE__);

							_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						stateMachine = 0;
						res = false;
						smxAlignment = false;
						break;
					}
					if (stateMachine)
					{
						std::string cxcTmp = stateMachine->getCurrentCXP();
						std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
						std::cout << " COMPARE With SMX SW: " << cxcTmp.c_str() << std::endl;
						std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;


						if (strcmp(cxcTmp.c_str(), sCXCToCompareSMX.c_str()) != 0)
						{
							std::cout << " ---------------------------------- " << std::endl;
							std::cout << "  DIFFERENCE FROM : " << sCXCToCompareSMX.c_str() << " TO : " << cxcTmp.c_str() << std::endl;
							std::cout << " ---------------------------------- " << std::endl;
							smxAlignment = false;
							break;
						}
						stateMachine = 0;

					} //if (stateMachine)
					else
					{
						smxAlignment = false;
						res = false;
						break;
					}
				}
			} //for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		} //if (stateMachine)
	}
	if (smxAlignment)
	{
		if (timeoutFlag != CCH_Util::SMX_ALARM_TIMEOUT) //smx timer timed out
		{
			//stop SMX alarm timer if ongoing
			if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== true)
			{
				m_alarmHandler->cancelEgem2L2SwitchAlarmTimer();
			}
		}
			//cease SMX alarm
			std::cout << " ================================== " << std::endl;
			std::cout << " Cease smx alarm.......... " <<__LINE__<< std::endl;
			std::cout << " ================================== " << std::endl;
			int problemSmx = 35003;
			unsigned int severityCeasingSmx = CCH_Util::Severity_CEASING;
			std::string cause = "SWITCHING BOARD SW MISALIGNMENT";
			std::string data = "SW Fault";
			std::string text = "NOT ALL SWITCHING BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
			sendAlarm (problemSmx, severityCeasingSmx, cause, data, text);
	}	
	else {
		if(timeoutFlag == CCH_Util::SMX_ALARM_TIMEOUT) // smx timer timed out
		{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] SMX timer timed out ", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);

				int problem = 35003;
				std::cout << " ================================== " << std::endl;
				std::cout << "raise smx alarm............ " << __LINE__<<std::endl;
				std::cout << " ================================== " << std::endl;
				//raise alarm

				unsigned int severity = CCH_Util::Severity_O1;
				std::string cause = "SWITCHING BOARD SW MISALIGNMENT";
				std::string data = "SW Fault";
				std::string text = "NOT ALL SWITCHING BOARDS ARE RUNNING ON THE SAME SOFTWARE LEVEL";
				sendAlarm (problem, severity, cause, data, text);
		}
		else{				
				std::cout << "start smx  timer............ " << __LINE__<<std::endl;
				// start smx alarm timer
				if(m_alarmHandler->isEgem2L2SwitchAlarmTimerOngoing()== false)
					m_alarmHandler->startEgem2L2SwitchAlarmTimer();
		}
			
	}

	return res;

	}
void FIXS_CCH_UpgradeManager::sendAlarm (const int problem, const unsigned int severity, const std::string &cause, const std::string &data, const std::string &text)
{
	acs_aeh_evreport EvReporter;

	std::string *processName = NULL;
	processName = new std::string();

	int pid = getpid();
	std::ostringstream id;
	id << pid;

	processName->append(CCH_Util::FIXS_CCH_PROCESS_NAME);
	processName->append(":");
	processName->append(id.str());


	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] problem = %d, cause = %s, data = %s, text = %s\n",__FUNCTION__, problem, cause.c_str(), data.c_str(), text.c_str());
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
	}

	std::string severityString;

	switch(severity)
	{
	case CCH_Util::Severity_A1:
		severityString = "A1";
		break;
	case CCH_Util::Severity_A2:
		severityString = "A2";
		break;
	case CCH_Util::Severity_CEASING:
		severityString = "CEASING";
		break;
	case CCH_Util::Severity_O1:
		severityString = "O1";
		break;
	default:

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Invalid Alarm severity specified %d \n", __FUNCTION__, severity);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
	}
	delete(processName);
	return;
	}

	const bool manualCeaseAllowed = false;
	ACS_AEH_ReturnType rt = EvReporter.sendEventMessage(processName->c_str(),
			problem,
			severityString.c_str(),
			cause.c_str(),
			"APZ",
			CCH_Util::FIXS_CCH_PROCESS_NAME,
			data.c_str(),
			text.c_str(),
			manualCeaseAllowed );

	if (rt == ACS_AEH_error)
	{

		//ACS_AEH_ErrorReturnType error = EvReporter.getError();
		ACS_AEH_ErrorType error = EvReporter.getError();
		switch (error)
		{
		case ACS_AEH_syntaxError:
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Event syntax error\n", __FUNCTION__);
				_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			break;

		case ACS_AEH_eventDeliveryFailure:
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Event delivery failure\n", __FUNCTION__);
				_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			break;

		default:
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Unknown event failure\n", __FUNCTION__);
				_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			break;
		} //switch (error)
	} //if (rt == ACS_AEH_error)

	if (processName) {
		delete processName;
		processName = NULL;
	}

}


int FIXS_CCH_UpgradeManager::cancelIpt (unsigned long umagazine, unsigned short uslot)
{
	int result = 0;
	//unsigned short otherSlot = 0;
	FIXS_CCH_IptbUpgrade *stateMachine = 0;
	Lock lock;

	for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{

		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
			}
			catch (bad_cast)
			{
				{ // log
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_lmPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				stateMachine = 0;
			}

			if (stateMachine)
			{
				if (!stateMachine->isRunningThread())
				{
					std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					stateMachine->open();

				}
				else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				result = stateMachine->cancel();
			}
			else
			{
				result = 1;
			}
			break;
		}//slot mag
	}//for
	return result;
}

int FIXS_CCH_UpgradeManager::getSwListSize ()
{
	Lock lock;
	return m_swUpgradeList->size();
}

bool FIXS_CCH_UpgradeManager::removeScxEntry(unsigned long magazine, unsigned short slot, std::string & tempExpectedCxp)
{
	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);

	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ", __FUNCTION__,__LINE__, MagValue.c_str(), slot);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);

	if (!IMM_Interface::deleteBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: removeScxEntry , FAILED to delete persistance object !!!"<< endl;

	//get current load module
	std::string container = getCurrentProduct(magazine,slot);

	//create value to search and delete from used package
	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;

	if (!IMM_Interface::setNotUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeScxEntry , FAILED to set used package !!!"<< endl;

	// remove board entry from  expected Cxp package

	if (!IMM_Interface::setNotUsedPackage(tempExpectedCxp,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeScxEntry , FAILED to unset used package !!!"<< endl;


	IMM_Interface::removeReportProgressObject(MagValue,sSlot,container);

	return true;

}

bool FIXS_CCH_UpgradeManager::removeIptEntry(unsigned long magazine, unsigned short slot,std::string & tempExpectedCxp)
{
	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);

	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ", __FUNCTION__,__LINE__, MagValue.c_str(), slot);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::deleteBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: removeIptEntry , FAILED to delete persistance object !!!"<< endl;

	//get current load module
	std::string container = getCurrentProduct(magazine,slot);

	//create value to search and delete from used package
	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;

	if (!IMM_Interface::setNotUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeIptEntry , FAILED to set used package !!!"<< endl;
	// remove board entry from  expected Cxp package

	if (!IMM_Interface::setNotUsedPackage(tempExpectedCxp,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeIptEntry , FAILED to unset used package !!!"<< endl;


	IMM_Interface::removeReportProgressObject(MagValue,sSlot,container);

	return true;
}

bool FIXS_CCH_UpgradeManager::removeIplbEntry(unsigned long magazine, unsigned short slot, std::string & tempExpectedCxp)
{

	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);

	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ", __FUNCTION__,__LINE__, MagValue.c_str(), slot);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::deleteBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: removeIplbEntry , FAILED to delete persistance object !!!"<< endl;

	//get current load module
	std::string container = getCurrentProduct(magazine,slot);

	//create value to search and delete from used package
	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;

	if (!IMM_Interface::setNotUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeIplbEntry , FAILED to set used package !!!"<< endl;

	if (!IMM_Interface::setNotUsedPackage(tempExpectedCxp,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeIplbEntry , FAILED to unset used package !!!"<< endl;


	IMM_Interface::removeReportProgressObject(MagValue,sSlot,container);

	return true;

}

bool FIXS_CCH_UpgradeManager::removeSmxEntry(unsigned long magazine, unsigned short slot, std::string & tempExpectedCxp)
{
	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);

	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ", __FUNCTION__,__LINE__, MagValue.c_str(), slot);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);

	if (!IMM_Interface::deleteBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: removeSmxEntry , FAILED to delete persistance object !!!"<< endl;

	//get current load module
	std::string container = getCurrentProduct(magazine,slot);

	//create value to search and delete from used package
	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;

	if (!IMM_Interface::setNotUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeSmxEntry , FAILED to set used package !!!"<< endl;

	// remove board entry from  expected Cxp package

	if (!IMM_Interface::setNotUsedPackage(tempExpectedCxp,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeSmxEntry , FAILED to unset used package !!!"<< endl;


	IMM_Interface::removeReportProgressObject(MagValue,sSlot,container);

	return true;

}



void FIXS_CCH_UpgradeManager::cleanStateMachine()
{
	swUpgradeList_t::iterator it;

	for (it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		FIXS_CCH_ScxUpgrade * stateMachineSW = 0;
		FIXS_CCH_SmxUpgrade * stateMachineSMXSW = 0;
		FIXS_CCH_IpmiUpgrade * stateMachineIPMI = 0;
		FIXS_CCH_PowerFanUpgrade * stateMachinePFM = 0;
		FIXS_CCH_IplbUpgrade * stateMachineIplb = 0;
		FIXS_CCH_FwUpgrade * stateMachineFW = 0;
		stateMachineIplb = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);
		stateMachineSW = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
		stateMachineSMXSW = dynamic_cast<FIXS_CCH_SmxUpgrade *> (it->m_swPtr);
		stateMachineIPMI = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
		stateMachinePFM = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
		stateMachineFW = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);
		if (stateMachineSW)
		{
			//delete SW state machine when it is stopped
			for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
			{
				if (stateMachineSW->isRunningThread() == false)
				{
					delete (it->m_swPtr);
					stateMachineSW = NULL;
					break;
				}
				else
				{
					stateMachineSW->stop();
				}

			}
		}
		if (stateMachineSMXSW)
		{
			//delete SW state machine when it is stopped
			for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
			{
				if (stateMachineSMXSW->isRunningThread() == false)
				{
					delete (it->m_swPtr);
					stateMachineSMXSW = NULL;
					break;
				}
				else
				{
					stateMachineSMXSW->stop();
				}

			}
		}
		if (stateMachineIplb)
		{
			//delete SW state machine when it is stopped
			for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
			{
				if (stateMachineIplb->isRunningThread() == false)
				{
					delete (it->m_swPtr);
					stateMachineIplb = NULL;
					break;
				}
				else
				{
					if(FIXS_CCH_logging) FIXS_CCH_logging->Write("thread is stopped in Upgrade Manager",LOG_LEVEL_DEBUG);
					stateMachineIplb->stop();
				}

			}
		}


		if (stateMachineIPMI)
		{
			//delete IPMI state machine when it is stopped
			for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
			{
				if (stateMachineIPMI->isRunningThread() == false)
				{
					delete stateMachineIPMI;
					stateMachineIPMI = NULL;
					break;
				}
				else
				{
					stateMachineIPMI->stop();
				}
			}
		}



		if (stateMachinePFM)
		{
			//delete SW state machine when it is stopped
			for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
			{
				if (stateMachinePFM->isRunningThread() == false)
				{
					delete stateMachinePFM;
					stateMachinePFM = NULL;
					break;
				}
				else
				{
					stateMachinePFM->stop();
				}
			}
		}


		if (stateMachineFW)
		{
			//delete SW state machine when it is stopped
			for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
			{
				if (stateMachineFW->isRunningThread() == false)
				{
					delete stateMachineFW;
					stateMachineFW = NULL;
					break;
				}
				else
				{
					stateMachineFW->stop();
				}
			}
		}
	}
	for (it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{
		FIXS_CCH_IptbUpgrade * stateMachineLW = 0;
		stateMachineLW = dynamic_cast<FIXS_CCH_IptbUpgrade *> (it->m_lmPtr);
		if (stateMachineLW)
		{
			//delete SW state machine when it is stopped
			for (int i = 0; i < MAX_RETRIES_ON_FSM_STOP; i++)
			{
				if (stateMachineLW->isRunningThread() == false)
				{
					delete(it->m_lmPtr);
					stateMachineLW = NULL;
					break;
				}
				else
				{
					stateMachineLW->stop();
				}
			}
		}
	}

}


void FIXS_CCH_UpgradeManager::addEpb1Entry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm)
{
	Lock lock;

	//to verify
	UNUSED(fsm);

	FsmListData data(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn, CCH_Util::EPB1);
	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " added magazine: " << data.m_magazine << ", slot:" << data.m_slot << ", IP_EthA:" << data.getIPEthAString().c_str() << ", IP_EthB:" << data.getIPEthBString().c_str() << "DN: " << data.m_dn <<std::endl;
	m_swUpgradeList->push_back(data);


	std::string MagValue;
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ipA[%s] ipB[%s] ", __FUNCTION__,__LINE__, MagValue.c_str(), data.m_slot, data.getIPEthAString().c_str(),data.getIPEthBString().c_str());
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::createBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: addEpb1Entry , FAILED to create persistance object !!!"<< endl;

	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;
	if (!IMM_Interface::setUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: addEpb1Entry , FAILED to set used package !!!"<< endl;

}

int FIXS_CCH_UpgradeManager::getEpb1ListSize ()
{
	Lock lock;
	return m_swUpgradeList->size();
}

int FIXS_CCH_UpgradeManager::getIptbListSize ()
{
	Lock lock;
	return m_iptUpgradeList->size();
}

bool FIXS_CCH_UpgradeManager::removeEpb1Entry(unsigned long magazine, unsigned short slot)
{

	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);

	//get current load module
	std::string container = getCurrentProduct(magazine,slot);

	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ", __FUNCTION__,__LINE__, MagValue.c_str(), slot);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::deleteBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: removeEpb1Entry , FAILED to delete persistance object !!!"<< endl;


	//create value to search and delete from used package
	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;

	if (!IMM_Interface::setNotUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeEpb1Entry , FAILED to set used package !!!"<< endl;

	return true;

}

int FIXS_CCH_UpgradeManager::checkOccurrenceEpb1Container (std::string container)
{
	int num = 0;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (strcmp(it->m_container.c_str(),container.c_str()) == 0) num++;
	}
	return num;
}


//  bool FIXS_CCH_UpgradeManager::checkImmStatusInProgress (std::string dn, int nextState, std::string attribute)
//  {
//	  bool res = false;
//	  //check upgrade state
//	  int actual_state = -1;
//	  int retries = 0;
//
//	  IMM_Util::getImmAttributeInt(dn,attribute,actual_state);
//
//	  if (actual_state == nextState) return true;
//
//	  while (actual_state != nextState)
//	  {
//		  cout << "Check upgrade state in ( ### NOT YET IN NEXT STATUS !!! ### )....retries: " << retries << endl;
//
//		  {
//			  char tmpStr[512] = {0};
//			  snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] ### NOT YET IN NEXT STATUS !!! ### ", __FUNCTION__,__LINE__);
//			  if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
//			  if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
//		  }
//
//		  IMM_Util::getImmAttributeInt(dn,attribute,actual_state);
//		  if (actual_state == nextState) res = true;
//		  if( ++retries > 8) break;
//		  sleep(1);
//	  }
//
//	  return res;
//  }

bool FIXS_CCH_UpgradeManager::checkSubnetIpTrapReceiver(std::string snmpIP, CCH_Util::trapType trapId, bool reset)
{
	bool res = false;

	int iPos = 0;
	int fPos = 0;
	std::string node("");

	if (!reset)
	{
		iPos = snmpIP.find_last_of(CHR_SHELF_SEPARATOR);
		fPos = snmpIP.find_last_of("");
		if(iPos >= 0) node = snmpIP.substr(iPos+1,fPos);

		cout << "######################## checkSubnetIpTrapReceiver ################### " << endl;
		cout << " snmp Ip:  "<< snmpIP.c_str() << " node to check: " << node.c_str() << endl;
	}

	switch(trapId)
	{
	case CCH_Util::LoadReleaseComplete:
	{
		if (!reset)
		{
			if (trapLoadReleaseComplete)
			{
				if (strcmp(trapLoadReleaseIp.c_str(),node.c_str()) == 0)
				{
					res = true;
					break;
				}
			}

			trapLoadReleaseComplete = true;
			trapLoadReleaseIp = node;
			break;
		}
		else
		{
			trapLoadReleaseComplete = false;
			trapLoadReleaseIp = "";
			break;
		}
	}

	case CCH_Util::ColdStartWarmStart:
	{
		if (!reset)
		{
			if (trapColdStartWarmStart)
			{
				if (strcmp(trapColdStartWarmStartIp.c_str(),node.c_str()) == 0)
				{
					res = true;
					break;
				}
			}

			trapColdStartWarmStart = true;
			trapColdStartWarmStartIp = node;
			break;
		}
		else
		{
			trapColdStartWarmStart = false;
			trapColdStartWarmStartIp = "";
			cout<<"trapColdStartWarmStart=false"<<__LINE__<<endl;
			break;
		}
	}

	case CCH_Util::IpmiUpgrade:
	{
		if (!reset)
		{
			if (trapIpmiUpgrade)
			{
				if (strcmp(trapIpmiUpgradeIp.c_str(),node.c_str()) == 0)
				{
					res = true;
					break;
				}
			}

			trapIpmiUpgrade = true;
			trapIpmiUpgradeIp = node;
			break;
		}
		else
		{
			trapIpmiUpgrade = false;
			trapIpmiUpgradeIp = "";
			break;
		}
	}

	case CCH_Util::PfmUpgrade:
	{
		if (!reset)
		{
			if (trapPfmUpgrade)
			{
				if (strcmp(trapPfmUpgradeIp.c_str(),node.c_str()) == 0)
				{
					res = true;
					break;
				}
			}

			trapPfmUpgrade = true;
			trapPfmUpgradeIp = node;
			break;
		}
		else
		{
			trapPfmUpgrade = false;
			trapPfmUpgradeIp = "";
			break;
		}
	}

	case CCH_Util::FwUpgrade:
	{
		if (!reset)
		{
			if (trapFwUpgrade)
			{
				if (strcmp(trapFwUpgradeIp.c_str(),node.c_str()) == 0)
				{
					res = true;
					break;
				}
			}

			trapFwUpgrade = true;
			trapFwUpgradeIp = node;
			break;
		}
		else
		{
			trapFwUpgrade = false;
			trapFwUpgradeIp = "";
			break;
		}
	}
	default:
		break;
	}

	return res;
}


int FIXS_CCH_UpgradeManager::fwRemoteStatus (std::string magazine, std::string slot, CCH_Util::FwData &tempFWData, CCH_Util::fwRemoteData &newFWData)
{

	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "mag =" << magazine << std::endl;

	// convert magazine
	if (getUlongMagazine(magazine, umagazine) == false) return INTERNALERR;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "mag =" << umagazine << std::endl;
	// convert slot
	if (getUshortSlot(slot, uslot) == false) return INTERNALERR;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "uslot =" << uslot << std::endl;

	if (!isScxData(umagazine, uslot) && !isCmxData(umagazine, uslot) && !isSmxData(umagazine, uslot)) return HARDWARE_NOT_PRESENT;

	Lock lock;
	bool found = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			found = true;
			break;
		}
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	if (!found) return INTERNALERR; // Internal Error

	FIXS_CCH_FSMDataStorage *storage = FIXS_CCH_FSMDataStorage::getInstance();

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	tempFWData.upgradeStatus = CCH_Util::FW_IDLE;
	tempFWData.lastUpgradeDate = "-";
	tempFWData.lastUpgradeTime = "-";
	tempFWData.result = "OK";

	storage->readUpgradeStatus(umagazine, uslot, tempFWData);

	if((tempFWData.upgradeStatus == CCH_Util::FW_ONGOING)|| (tempFWData.upgradeStatus == CCH_Util::FW_UNDEFINED))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		bool res =false;

		//read new values
		res = storage->readUpgradeStatus(umagazine, uslot, newFWData);
	}

	return 0; //TODO
}

int FIXS_CCH_UpgradeManager::printFwRemoteStatus (char *buffer, int bufferSize, CCH_Util::FwData &tempFWData, CCH_Util::fwRemoteData &newFWData)
{
	UNUSED(bufferSize);
	char *tmpBuf = 0;
	int count =0;

	for(int j=0;j<MAX_NO_OF_VERSION_INDEX;j++)
	{
		if((strcmp(tempFWData.fwInfo.fwType[j].c_str(),"") != 0) || (strcmp(tempFWData.fwInfo.fwVersion[j].c_str(),"") != 0))
		{
			count++;
		}
		if((strcmp(newFWData.fwType[j].c_str(),"") != 0) || (strcmp(newFWData.fwVersion[j].c_str(),"") != 0))
		{
			count++;
		}
	}
	int nItem = 0;
	if(tempFWData.upgradeStatus == CCH_Util::FW_IDLE )
	{
		if(strcmp(tempFWData.result.c_str(),"OK") == 0)
			nItem = 7;
		else
			nItem = 8;
	}
	else
		nItem = 10 + count;
	int respSize = sizeof(nItem) + (ITEM_SIZE * nItem);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;

	memcpy(buffer, &nItem, sizeof(nItem)); // write number of items, in this case always 1
	tmpBuf = buffer + sizeof(nItem);
	//write headerstring
	snprintf(tmpBuf, ITEM_SIZE, "%-40s", "FIRMWARE UPGRADE PROGRESS REPORT");
	tmpBuf = tmpBuf + ITEM_SIZE;

	if((tempFWData.upgradeStatus == CCH_Util::FW_ONGOING) || (tempFWData.upgradeStatus == CCH_Util::FW_UNDEFINED))
	{
		/*********add blank line ****************/
		snprintf(tmpBuf, ITEM_SIZE, " ");
		tmpBuf = tmpBuf + ITEM_SIZE;

		snprintf(tmpBuf, ITEM_SIZE, "  %-25s", "OLD FW LOADED");
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(tmpBuf, ITEM_SIZE, "  %-32s %-32s", "FW TYPE","FW VERSION");
		tmpBuf = tmpBuf + ITEM_SIZE;

		for(int j=0;j<MAX_NO_OF_VERSION_INDEX;j++)
		{
			if((strcmp(tempFWData.fwInfo.fwType[j].c_str(),"") != 0) || (strcmp(tempFWData.fwInfo.fwVersion[j].c_str(),"") != 0))
			{
				snprintf(tmpBuf, ITEM_SIZE, "  %-32s %-32s", tempFWData.fwInfo.fwType[j].c_str(),tempFWData.fwInfo.fwVersion[j].c_str());
				tmpBuf = tmpBuf + ITEM_SIZE;
			}
		}

		/*********add blank line ****************/
		snprintf(tmpBuf, ITEM_SIZE, " ");
		tmpBuf = tmpBuf + ITEM_SIZE;

		snprintf(tmpBuf, ITEM_SIZE, "  %-25s", "NEW FW LOADED");
		tmpBuf = tmpBuf + ITEM_SIZE;
		snprintf(tmpBuf, ITEM_SIZE, "  %-32s %-32s", "FW TYPE","FW VERSION");
		tmpBuf = tmpBuf + ITEM_SIZE;

		for(int j=0;j<MAX_NO_OF_VERSION_INDEX;j++)
		{
			if((strcmp(newFWData.fwType[j].c_str(),"") != 0) || (strcmp(newFWData.fwVersion[j].c_str(),"") != 0))
			{
				snprintf(tmpBuf, ITEM_SIZE, "  %-32s %-32s", newFWData.fwType[j].c_str(),newFWData.fwVersion[j].c_str());
				tmpBuf = tmpBuf + ITEM_SIZE;
			}
		}
	}

	if(tempFWData.upgradeStatus == CCH_Util::FW_IDLE )
	{
		/*********add blank line ****************/
		snprintf(tmpBuf, ITEM_SIZE, " ");
		tmpBuf = tmpBuf + ITEM_SIZE;

		if((strcmp(tempFWData.lastUpgradeDate.c_str(),"-") == 0) && (strcmp(tempFWData.lastUpgradeTime.c_str(),"-") == 0))
			snprintf(tmpBuf, ITEM_SIZE, "%-19s%-11s", "LAST UPGRADE TIME:",tempFWData.lastUpgradeDate.c_str());
		else
			snprintf(tmpBuf, ITEM_SIZE, "%-19s%-11s%-10s", "LAST UPGRADE TIME:",tempFWData.lastUpgradeDate.c_str(),tempFWData.lastUpgradeTime.c_str());

		tmpBuf = tmpBuf + ITEM_SIZE;

		//  	snprintf(tmpBuf, ITEM_SIZE, "%-8s%-40s", "RESULT:",tempFWData.result.c_str());

		if(strcmp(tempFWData.result.c_str(),"OK") != 0)
		{
			snprintf(tmpBuf, ITEM_SIZE, "%-8s%-40s", "FAILURE REASON:",tempFWData.lastUpgradeReason.c_str());
			tmpBuf = tmpBuf + ITEM_SIZE;
		}
	}

	if(tempFWData.upgradeStatus == CCH_Util::FW_IDLE){
		snprintf(tmpBuf, ITEM_SIZE, "%-19s%-8s", "FW UPGRADE STATUS:",tempFWData.result.c_str());
		tmpBuf = tmpBuf + ITEM_SIZE;
	}
	else if(tempFWData.upgradeStatus == CCH_Util::FW_UNDEFINED){
		snprintf(tmpBuf, ITEM_SIZE, "%-19s%-8s", "FW UPGRADE STATUS:","UNDEFINED");
		tmpBuf = tmpBuf + ITEM_SIZE;
	}
	else
	{
		/*********add blank line ****************/
		snprintf(tmpBuf, ITEM_SIZE, " ");
		tmpBuf = tmpBuf + ITEM_SIZE;

		snprintf(tmpBuf, ITEM_SIZE, "%-19s%-8s", "FW UPGRADE STATUS:","ONGOING");
		tmpBuf = tmpBuf + ITEM_SIZE;
	}
	//  tmpBuf = tmpBuf + ITEM_SIZE;

	return respSize;
}

int FIXS_CCH_UpgradeManager::startFwUpgradeFSMs ()
{
	int result  = 0;

	//create hidden classes
	if (!IMM_Util::createClassFwStorage()) return 1;
	if (!IMM_Util::createClassFwStorageInfo()) return 1;

	FIXS_CCH_FSMDataStorage *storage = FIXS_CCH_FSMDataStorage::getInstance();
	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		//There is an fw upgrade for each magazine.
		if ((it->m_fwPtr == 0))
		{
			if ((it->m_fbn == CCH_Util::SCXB) || (it->m_fbn == CCH_Util::CMXB)|| (it->m_fbn == CCH_Util::SMXB))
			{
				FIXS_CCH_FwUpgrade *fwUp = new (std::nothrow) FIXS_CCH_FwUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);

				if (fwUp)
				{
					CCH_Util::FwData data;
					it->m_fwPtr = fwUp;
					if(storage->readUpgradeStatus(it->m_magazine, it->m_slot, data))
					{
						fwUp->restoreStatus(data);
					}

					if(data.upgradeStatus == CCH_Util::FW_ONGOING)
					{
						std::cout << "\n UPGRADE ONGOING on board :\n"<< it->m_dn.c_str() << std::endl;
						fwUp->open();
					}
				}
			}
		}
	}
	return result;
}


int FIXS_CCH_UpgradeManager::fwUpgradeStart (std::string magazine, std::string slot, std::string valueToSet)
{
	if (_trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] fwUpgradeStart requested on magazine \"%s\" and slot \"%s\", value to set: %s", __LINE__, magazine.c_str(), slot.c_str(),  valueToSet.c_str());
		_trace->ACS_TRA_event(1, tmpStr);
	}
	int result = INTERNALERR; //INTERNAL ERROR

	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	std::string container("");
	std::string product("");
	std::string sSlotStateMachine("");
	unsigned short uSlotStateMachine = 0;

	// convert magazine
	if (getUlongMagazine(magazine, umagazine) == false) return result;

	// convert slot
	if (getUshortSlot(slot, uslot) == false) return result;

	if(isBackupOngoing() == true) return NOT_ALLOWED_BACKUP_ONGOING;

	//Looking for another upgrade
	if(fwCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;//(static_cast<int>(FWUPG_ONGOING));//UPGRADE_ALREADY_ONGOING
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;//(static_cast<int>(IPMIUPG_ONGOING));//UPGRADE_ALREADY_ONGOING
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;//UPGRADE_ALREADY_ONGOING
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;//UPGRADE_ALREADY_ONGOING
	if(checkOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;//UPGRADE_ALREADY_ONGOING
	if(checkOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;//UPGRADE_ALREADY_ONGOING
	if(checkOtherSlotStatus(umagazine,26) == false) return UPGRADE_ALREADY_ONGOING;//UPGRADE_ALREADY_ONGOING
	if(checkOtherSlotStatus(umagazine,28) == false) return UPGRADE_ALREADY_ONGOING;//UPGRADE_ALREADY_ONGOING

	sSlotStateMachine = slot;
	uSlotStateMachine = uslot;


	// Initialize path
	std::string strFileName("");
	std::string strFtpPath("");
	std::string nodeIP("");
	std::string hostName("");
	std::string tmpIPA("");
	std::string tmpIPB("");

	{
		Lock lock;
		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{

				FIXS_CCH_FwUpgrade *stateMachine = 0;
				try
				{
					//get state machine
					tmpIPA = it->getIPEthAString();
					tmpIPB = it->getIPEthBString();
					stateMachine = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);
				}
				catch (bad_cast)
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"Exception!! " << std::endl;
					if (_trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_fwPtr is not a FIXS_CCH_FwUpgrade", __FUNCTION__, __LINE__);
						_trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}
				if (!stateMachine) return 1; // Internal Error

				if(!stateMachine->isRunningThread())
				{
					std::cout<<"DBG: "<< __FUNCTION__ <<"@"<< __LINE__ <<" REMOTE FW UPGRADE activating thread... "<< std::endl;
					stateMachine->open();
					sleep(2);
				}

				if(!stateMachine->getCPAuthorization())
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"CP authorization for FW Upgarde failed " << std::endl;
					return NOT_ALLOWED_BY_CP;
				}
				else
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"CP authorization for FW Upgarde successful " << std::endl;
				}

				//start FW upgrade
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"Calling StateMachine upgrade " << std::endl;
				stateMachine->setDateandTime(); // set upgrade date and time

				std::string shFilename("");
				std::string xmlFilename("");
				result = unZipPackage(valueToSet,shFilename,xmlFilename,magazine,slot,true);
				if(result != 0)
				{
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager - fwUpgradeStartsaveOld(): FAILED to unZipPackage...",LOG_LEVEL_ERROR);
					break;
				}
				else
				{
					CCH_Util::fwRemoteData versionInfo;
					CCH_Util::FwData mydata;
					std::string tftpRoot = FIXS_CCH_DiskHandler::getTftpRootFolderDefault();
					std::string tempStr = "/";
					std::string tempShFullPath = tftpRoot + tempStr + shFilename;
					std::string tempXmlDestFolder = tftpRoot + tempStr + xmlFilename;
					cout << "tempShFullPath: " << tempShFullPath.c_str()  << endl;
					cout << "tempXmlDestFolder: " << tempXmlDestFolder.c_str()  << endl;
					std::string dn_perc = IMM_Interface::getDnBladePersistance(umagazine,uslot);

					result = getFirmwareVersion(umagazine,uslot,tmpIPA,tmpIPB, versionInfo);
					if(result != 0)
					{
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager - fwUpgradeStartsaveOld(): FAILED to take OLD value from board...",LOG_LEVEL_ERROR);
						CCH_Util::deleteFile(tempShFullPath.c_str());
						CCH_Util::deleteFile(tempXmlDestFolder.c_str());
						break;
					}
					else
					{
						bool isSmx = false;
						if((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
							isSmx = true;
						if(isSmx)
						{
							int activeArea =-1;
							int result = getActiveFwAreaFromBoard(umagazine,slot,activeArea);
							if(result != 0)
							{
                                        			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager - getActiveFwAreaFromBoard(): FAILED...",LOG_LEVEL_ERROR);
                                        			break;
							}

							customizeFirmwareType(versionInfo,activeArea); 
						}
						if (!FIXS_CCH_FSMDataStorage::getInstance()->saveOldFwUpgradeStatus(dn_perc,versionInfo))
						{
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager - fwUpgradeStartsaveOld(): FAILED to store old info...",LOG_LEVEL_ERROR);
							std::string xmlFilename = ((isSmx)? "metadata.json" : "metadata.xml");
							std::string tempXmlDestFolder = tftpRoot + tempStr + xmlFilename ;
							CCH_Util::deleteFile(tempShFullPath.c_str());
							CCH_Util::deleteFile(tempXmlDestFolder.c_str());
							result = INTERNALERR;
							break;
						}
						else if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager - fwUpgradeStartsaveOld(): OLD DATA SUCCESSFUL STORED !!!",LOG_LEVEL_ERROR);
					}

					FIXS_CCH_FSMDataStorage::getInstance()->readUpgradeStatus(umagazine,uslot,mydata);
					if(FIXS_CCH_FSMDataStorage::getInstance()->saveUpgradeStatus(dn_perc, mydata) == false) result = INTERNALERR;
					if(FIXS_CCH_FSMDataStorage::getInstance()->saveFwShFileName(dn_perc, tempShFullPath)== false) result = INTERNALERR;
					if(FIXS_CCH_FSMDataStorage::getInstance()->saveFwXmlFileName(dn_perc, tempXmlDestFolder)== false) result = INTERNALERR;

					//get hostname
					hostName = CCH_Util::GetAPHostName();
					cout << "GetAPHostName: " << hostName.c_str() << endl;

					if( slot.compare("0") == 0 )
					{
						if (hostName.compare("SC-2-1") == 0) nodeIP = (string)SUB_IP1 + IP1;
						else if (hostName.compare("SC-2-2") == 0) nodeIP = (string)SUB_IP1 + IP2;
						else
						{
							nodeIP = (string)CLUSTER_IP1;
						}
					}

					else if( slot.compare("25") == 0 )
					{
						if (hostName.compare("SC-2-1") == 0)	nodeIP = (string)SUB_IP2 + IP1;
						else if (hostName.compare("SC-2-2") == 0) nodeIP = (string)SUB_IP2 + IP2;
						else
						{
							nodeIP = (string)CLUSTER_IP2;
						}
					}

					else
					{
						nodeIP = (string)CLUSTER_IP1;
					}

					strFtpPath = "tftp://" + nodeIP + "/" +shFilename;
					cout << "strFtpPath: " << strFtpPath.c_str()  << endl;

					if(result == 0)
					{
						result = stateMachine->upgrade(strFtpPath, slot, tmpIPA, tmpIPB);
					}
					//  					else
					//  					{
					//  						stateMachine->stop();
					//  						sleep(1);
					//  					}

					break;
				}
			}
		}
	}

	return result;
}



void FIXS_CCH_UpgradeManager::fwUpgradeTrap (int fwUpgradeResult, std::string snmpIP)
{

	//if (ACS_TRA_ON(traceObj))
	//{
	//	char tmpStr[512] = {0};
	//	_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] fwUpgradeTrap - subrackNumValue: \"%s\"; slotPosValue: \"%s\", fwUpgradeResult: \"%s\", snmpIP: \"%s\"", __LINE__, subrackNumValue.c_str(), slotPosValue.c_str(), fwUpgradeResult.c_str(), snmpIP.c_str());
	//	ACS_TRA_event(&traceObj, tmpStr);
	//}
	//cout << "fwUpgradeTrap subrackNumValue: " << subrackNumValue.c_str() <<
	//	" - slotPosValue: " << slotPosValue.c_str() <<
	//	" - fwUpgradeResult: " << fwUpgradeResult.c_str() <<
	//	" - fwUpgradeResult: " << snmpIP.c_str() << std::endl;

	unsigned long umagazine = 0;
	unsigned short uslot = 0;
	int trapRes = 0;
	int res = 0;

	trapRes = fwUpgradeResult;

	FIXS_CCH_FwUpgrade *stateMachine = NULL;
	//FIXS_CCH_FwUpgrade *stateMachine25 = NULL;

	if (checkSubnetIpTrapReceiver(snmpIP, CCH_Util::FwUpgrade,false)) return;

	Lock lock;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->getIPEthAString() == snmpIP) || (it->getIPEthBString() == snmpIP))
		{
			umagazine = it->m_magazine;
			uslot = it->m_slot;
			break;
		}
	}

	//bool found0 = false;
	//bool found25 = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == umagazine) && it->m_slot == uslot)
		{
			stateMachine = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);
			break;
		}
	}

	//if ((it->m_magazine == umagazine))
	//{
	//	if (it->m_slot == 0)
	//	{
	//		stateMachine0 = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);
	//		found0 = true;
	//	}
	//	else
	//	{
	//		stateMachine25 = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);
	//		found25 = true;
	//	}

	//	//if ( (found0 == true) && (found25 == true))
	//	if ( (stateMachine0 != 0) && (stateMachine25 != 0) )
	//	{
	//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " found SM 0 and SM 25 " << std::endl;
	//		break;
	//	}
	//}

	string fwMachineState = "";
	//string fwMachineState25 = "";

	if (stateMachine) {
		fwMachineState = stateMachine->getStateName();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " found SM 0 - State: "<< fwMachineState.c_str() << std::endl;

		//if (found25 == true)
		//{
		//	fwMachineState25 = stateMachine25->getStateName();
		//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " found SM 25 - State: "
		//		<< fwMachineState25.c_str() << std::endl;
		//}


		//	 if (fwMachineState == "ONGOING")
		//	 {
		//		 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<"fwMachineState ONGOING - trapRes: " << trapRes  << std::endl;
		res = stateMachine->fwUpgradeResultTrap(trapRes);
		//	 }
		//	 else
		//	 {
		//		 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "fwMachineState IDLE " << std::endl;
		//	 }

		//clean subnet trap check
		checkSubnetIpTrapReceiver(snmpIP,CCH_Util::FwUpgrade,true);
	}
}


bool FIXS_CCH_UpgradeManager::fwCheckOtherSlotStatus (unsigned long umagazine)
{

	bool found = false;

	{
		Lock lock;
		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine))
			{
				FIXS_CCH_FwUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);
				}
				catch (bad_cast)
				{
					if (_trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_fwPtr is not a FIXS_CCH_FwUpgrade", __FUNCTION__, __LINE__);
						_trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}

				if (stateMachine && (stateMachine->getStateName() == "ONGOING"))
				{
					//found a slot with IPMI ongoing
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("Found an FW ongoing magazine",LOG_LEVEL_DEBUG);
					found = true;
					break;
				}
			}
		}
	}

	return (found ? false : true);
}

int FIXS_CCH_UpgradeManager::unZipPackage (std::string &path, std::string &shFilename, std::string &xmlFilename, std::string mag, std::string slot, bool isRfu)
{
	int result = 0;
	std::string cmdUnzip("");
	std::string tftpRoot = FIXS_CCH_DiskHandler::getTftpRootFolderDefault();
	std::string archievePath = path.substr(0,(path.find_last_of("/")+1));
	std::string zipFileName = path.substr(path.find_last_of("/")+1);
//	std::string gzFileName = path.substr(path.find_last_of("/")+1);
	std::cout <<"archievePath = " << archievePath.c_str() << std::endl;
	std::cout <<"zipFileName = " << zipFileName.c_str()  << std::endl;
	std::cout <<"tftpRoot = " << tftpRoot.c_str()  << std::endl;
	std::string tempFolder = CCH_Util::TEMP_FWFOLDER + "/" +  zipFileName ;
	std::string gztempFolder = CCH_Util::TEMP_FWFOLDER + "/";

	if(CCH_Util::CheckExtention(tempFolder.c_str(),".zip"))
		cmdUnzip = "unzip \"" + tempFolder +"\" -d \"" + CCH_Util::TEMP_FWFOLDER + "\"";
	else if(CCH_Util::CheckExtention(tempFolder.c_str(),".tar"))
	{
		cmdUnzip="tar xfv \"" + tempFolder +"\" -C \"" + CCH_Util::TEMP_FWFOLDER + "\">/dev/null";
	}

	else if(CCH_Util::CheckExtention(tempFolder.c_str(),".tar.gz"))
	{
		cmdUnzip 	= "tar zxfv \"" + tempFolder +"\" -C \"" + CCH_Util::TEMP_FWFOLDER + "\">/dev/null";
	}
	else return DIRECTORYERR;

	//	else
	//		return INCORRECT_PACKAGE;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	if(!CCH_Util::PathFileExists(CCH_Util::TEMP_FWFOLDER.c_str()))
	{
		//CCH_Util::TEMP_FOLDER doesn't exist, create it...
		ACS_APGCC::create_directories(CCH_Util::TEMP_FWFOLDER.c_str(),ACCESSPERMS);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	}
	else
	{
		//TEMP_FOLDER already exists
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	}

	if(!CCH_Util::PathFileExists(archievePath.c_str()))
	{
		//error the directory doesn't exist
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] The archive path %s does not exist.", __FUNCTION__, __LINE__, archievePath.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << traceChar << std::endl;

		CCH_Util::removeFolder(CCH_Util::TEMP_FWFOLDER.c_str());

		return 1;
	}
	else
	{

		if(!CCH_Util::PathFileExists(path.c_str())){

			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

			CCH_Util::removeFolder(CCH_Util::TEMP_FWFOLDER.c_str());
			//			CCH_Util::deleteFile(path.c_str()); //sw_package

			//return 1;
			return DIRECTORYERR;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

		if (CCH_Util::CopyFile(path.c_str(), tempFolder.c_str(), false, 4096) == false )
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error during copy!" << std::endl;
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Cannot copy \"%s\" to \"%s\"", __FUNCTION__, __LINE__, path.c_str(), tempFolder.c_str());
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " : " << traceChar << std::endl;

			CCH_Util::removeFolder(CCH_Util::TEMP_FWFOLDER.c_str());
			//			CCH_Util::deleteFile(path.c_str()); //sw_package

			//Check for Disk Full
			if (errno == EDQUOT) return DISK_FULL;

			return 1;
		}
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	if ( (system(cmdUnzip.c_str())) == 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "unzipped successfully "<<std::endl;
		bool isSmx=false;
		if ((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
			isSmx = true;
		if(isRfu)
		{
			if(isSmx)
			{		
				//Check tar.gz in unzipped file
				cmdUnzip.assign("");
				std::string filetz;
				if(CCH_Util::findRfuDataFile(filetz,CCH_Util::TEMP_FWFOLDER))
				{
					gztempFolder = gztempFolder + filetz;
					cmdUnzip 	= "gunzip  \"" + gztempFolder +"\" >/dev/null";
				}
				else return DIRECTORYERR;
				if ( (system(cmdUnzip.c_str())) == 0)
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "gunzipped successfully "<<std::endl;
				else return DIRECTORYERR;
			}
		}
		/*******************/
		if(CCH_Util::findMetadataFile(shFilename,CCH_Util::TEMP_FWFOLDER,isSmx))
		{
			
			std::string tempShFile = shFilename;
			//rename file with magazine and slot
			if(!isSmx)
				CCH_Util::renameFilewithMagAndSlot(shFilename,mag,slot);

			std::string tempShSourceFolder = CCH_Util::TEMP_FWFOLDER + "/" +tempShFile;
			std::string destShFilename = tftpRoot + "/" + shFilename;

			std::cout <<"destShFilename = " << destShFilename.c_str()  << std::endl;

			if (CCH_Util::CopyFile(tempShSourceFolder.c_str(), destShFilename.c_str(), false, 4096) == false )
			{
				std::cout <<"DBG: failed to copy shFilename: " << shFilename.c_str()
											<< " to folder: "<< destShFilename.c_str()<<std::endl;

				//Check for Disk Full
				if (errno == EDQUOT) return DISK_FULL;


				result = 1;
			}

			CCH_Util::deleteFile(tempShSourceFolder.c_str());
			bool fileFound = false;
			if(isSmx)
			{
				if(isRfu)
					fileFound = CCH_Util::findJsonFile(xmlFilename,CCH_Util::TEMP_FWFOLDER);
				else
					fileFound = CCH_Util::findXmlFile(xmlFilename,CCH_Util::TEMP_FWFOLDER);
			}
			else
				fileFound = CCH_Util::findXmlFile(xmlFilename,CCH_Util::TEMP_FWFOLDER);
			if(fileFound)
			{
				std::string tempXmlFile = xmlFilename;
				//rename file with magazine and slot
				if(!isSmx)
				CCH_Util::renameFilewithMagAndSlot(xmlFilename,mag,slot);

				std::string xmlPathFilename = CCH_Util::TEMP_FWFOLDER + "/" + tempXmlFile;
				std::string destXmlFilename = tftpRoot + "/" + xmlFilename;

				std::cout <<"destXmlFilename = " << destXmlFilename.c_str()  << std::endl;

				if (CCH_Util::CopyFile(xmlPathFilename.c_str(), destXmlFilename.c_str(), false, 4096) == false )
				{
					std::cout <<"DBG: failed to copy xmlFilename: " << xmlFilename.c_str()
														<< " to dest: "<< destXmlFilename.c_str()<<std::endl;

					//Check for Disk Full
					if (errno == EDQUOT) return DISK_FULL;

					result = 1;
				}

				CCH_Util::deleteFile(xmlPathFilename.c_str());

			}
			else result = INCORRECT_PACKAGE;
		}
		else result = INCORRECT_PACKAGE;
	}
	else result = INCORRECT_PACKAGE;

	CCH_Util::removeFolder(CCH_Util::TEMP_FWFOLDER.c_str());

	//CCH_Util::deleteFile(path.c_str()); //sw_package

	return result;
}

int FIXS_CCH_UpgradeManager::getFirmwareVersion (unsigned long umagazine, unsigned short uslot, std::string IP_EthA_str, std::string IP_EthB_str, CCH_Util::fwRemoteData &versionInfo)
{
	UNUSED(umagazine);
	UNUSED(uslot);

	std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;

	FIXS_CCH_SNMPManager *snmpReader = new FIXS_CCH_SNMPManager(IP_EthA_str);

	int attributeValue = -1;

	/********************** Fill versionInfo by making SNMP get queries *************/
	bool infoFlag = false;
	for(unsigned int i =1; i<= MAX_NO_OF_VERSION_INDEX;i++)
	{
		attributeValue = -1;
		infoFlag = false;
		if(snmpReader->getFwmLoadedAttribute(i,&attributeValue))
		{
			if(attributeValue == 1) // 1 means currentSW
				infoFlag = true;
			else
				infoFlag = false;
		}
		else
		{
			std::string tmpIp = snmpReader->getIpAddress();
			if(tmpIp.compare(IP_EthA_str) == 0)
				snmpReader->setIpAddress(IP_EthB_str);
			else
				snmpReader->setIpAddress(IP_EthA_str);
			if(snmpReader->getFwmLoadedAttribute(i,&attributeValue))
			{
				if(attributeValue == 1) // 1 means currentSW
					infoFlag = true;
				else
					infoFlag = false;
			}
			else
			{
				/************SNMP FAILURE***********/
				delete snmpReader;
				snmpReader = NULL;
				return SNMP_FAILURE;
			}
		}

		if (infoFlag == true) // means currentSW
		{
			std::string fwmLoadedType("");
			std::string fwmLoadedVersion("");

			/**************** fetching Firmware Load type ********************/
			if(snmpReader->getFwmLoadedType(i,fwmLoadedType))
			{
				ACS_APGCC::trimEnd(fwmLoadedType);
				ACS_APGCC::toUpper(fwmLoadedType);
				cout << "fwmLoadedType: "<< fwmLoadedType.c_str() << endl;
				versionInfo.fwType[i-1] = fwmLoadedType;

			}
			else
			{
				std::string tmpIp = snmpReader->getIpAddress();
				if(tmpIp.compare(IP_EthA_str) == 0)
					snmpReader->setIpAddress(IP_EthB_str);
				else
					snmpReader->setIpAddress(IP_EthA_str);
				if(snmpReader->getFwmLoadedType(i,fwmLoadedType))
				{
					ACS_APGCC::trimEnd(fwmLoadedType);
					ACS_APGCC::toUpper(fwmLoadedType);
					cout << "fwmLoadedType: "<< fwmLoadedType.c_str() << endl;
					versionInfo.fwType[i-1] = fwmLoadedType;
				}
				else
				{
					/************SNMP FAILURE***********/
					delete snmpReader;
					snmpReader = NULL;
					return SNMP_FAILURE;
				}
			}


			/**************** fetch firmware loaded version **************/
			if(snmpReader->getFwmLoadedVersion(i, fwmLoadedVersion))
			{
				ACS_APGCC::trimEnd(fwmLoadedVersion);
				ACS_APGCC::toUpper(fwmLoadedVersion);
				cout << "fwmLoadedVersion: "<< fwmLoadedVersion.c_str() << endl;
				versionInfo.fwVersion[i-1] = fwmLoadedVersion ;
			}
			else
			{
				std::string tmpIp = snmpReader->getIpAddress();
				if(tmpIp.compare(IP_EthA_str) == 0)
					snmpReader->setIpAddress(IP_EthB_str);
				else
					snmpReader->setIpAddress(IP_EthA_str);
				if(snmpReader->getFwmLoadedVersion(i, fwmLoadedVersion))
				{
					ACS_APGCC::trimEnd(fwmLoadedVersion);
					ACS_APGCC::toUpper(fwmLoadedVersion);
					cout << "fwmLoadedVersion: "<< fwmLoadedVersion.c_str() << endl;
					versionInfo.fwVersion[i-1] = fwmLoadedVersion ;
				}
				else
				{
					/************SNMP FAILURE***********/
					delete snmpReader;
					snmpReader = NULL;
					return SNMP_FAILURE;
				}
			}
		}
		else  // means noAttribute
		{
			versionInfo.fwType[i-1] = "";
			versionInfo.fwVersion[i-1] = "";
		}
	}

	if(snmpReader)
		delete snmpReader;
	snmpReader = NULL;
	return EXECUTED;
}



bool FIXS_CCH_UpgradeManager::getMagAndSlotFromDn (std::string dn, unsigned long &magazine, unsigned short &slot)
{
	bool result = false;
	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (strcmp(it->m_dn.c_str(),dn.c_str()) == 0)
		{
			magazine = it->m_magazine;
			slot = it->m_slot;
			result = true;
			break;
		}
	}

	return result;
}


bool FIXS_CCH_UpgradeManager::isUsedPackage(std::string container)
{
	bool result = false;

	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (strcmp(it->m_container.c_str(),container.c_str()) == 0)
		{
			result = true;
			break;
		}
	}

	return result;

}

std::string FIXS_CCH_UpgradeManager::getCurrentProduct(unsigned long &magazine, unsigned short slot)
{
	bool found = false;

	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && it->m_slot == slot)
		{
			found = true;
			return it->m_container;
		}
	}
	//check in IPT board list
	if (!found)
	{
		for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
		{
			if ((it->m_magazine == magazine) && it->m_slot == slot)
			{
				found = true;
				return it->m_container;
			}
		}
	}

	return "";
}


int FIXS_CCH_UpgradeManager::fetchEgem2L2SwitchIP(unsigned long magazine, unsigned short slot, std::string &ip_A, std::string &ip_B)
{
	int result = -1;
	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			result = 0;
			//ip_A = it->m_IP_EthA;
			//ip_B = it->m_IP_EthB;

			ip_A = it->getIPEthAString();
			ip_B = it->getIPEthBString();


			cout << "ipA of slot " << slot << " : " << ip_A << endl;
			cout << "ipB of slot " << slot << " : " << ip_B << endl;
			break;
		}
	}

	return result;
}

bool FIXS_CCH_UpgradeManager::getMasterPassiveSCX(unsigned long magazine, std::string& masterIp, std::string& passiveIp, unsigned short& masterSlot)
{
	bool result = true;
	std::string slot0IpA(""), slot25IpA(""), slot0IpB(""), slot25IpB("");
	fetchEgem2L2SwitchIP(magazine, 0, slot0IpA, slot0IpB);
	fetchEgem2L2SwitchIP(magazine, 25, slot25IpA, slot25IpB);
	FIXS_CCH_SNMPManager* snmpReader = new (std::nothrow) FIXS_CCH_SNMPManager(slot0IpA);
	int snmpRes = snmpReader->isMaster(slot0IpA);
	if(snmpRes == 1) //master
	{
		masterIp = slot0IpA;
		passiveIp = slot25IpA;
		masterSlot = 0;
	}
	else if(snmpRes == 2) //passive
	{
		masterIp = slot25IpA;
		passiveIp = slot0IpA;
		masterSlot = 25;
	}
	else
	{
		snmpRes = snmpReader->isMaster(slot25IpB);
		if(snmpRes == 1) //master
		{
			masterIp = slot25IpB;
			passiveIp = slot0IpB;
			masterSlot = 25;
		}
		else if(snmpRes == 2) //passive
		{
			masterIp = slot0IpB;
			passiveIp = slot25IpB;
			masterSlot = 0;
		}
		else
			result = false;

	}
	if(snmpReader != NULL)
	{
		delete(snmpReader);
		snmpReader = NULL;
	}
	return result;
}

int FIXS_CCH_UpgradeManager::writeIpmiStatus (char* buffer, int bufferSize)
{

	char *tmpBuf = 0;
	int nItem = 4;
	if(m_ipmiResult.compare("ONGOING") == 0)
		nItem = 3;
	int respSize = sizeof(nItem) + (ITEM_SIZE * nItem);
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	if (respSize > bufferSize)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
		return -1;
	}
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	memcpy(buffer, &nItem, sizeof(nItem)); // write number of items, in this case always 1
	tmpBuf = buffer + sizeof(nItem);


	//write headerstring
	snprintf(tmpBuf, ITEM_SIZE, "%-20s", " ");   //to be defined
	tmpBuf = tmpBuf + ITEM_SIZE;

	char row[ITEM_SIZE] = {0};

	std::string outputStr("");
	outputStr = "IPMI FW UPGRADE STATUS: " + m_ipmiResult;

	snprintf(row, sizeof(row) - 1, "%-20s\n", "IPMI FIRMWARE UPGRADE PROGRESS REPORT");
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
	snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	tmpBuf = tmpBuf + ITEM_SIZE;


	if(m_ipmiResult.compare("ONGOING") == 0)
	{
		snprintf(row, sizeof(row) - 1, "%-20s\n", outputStr.c_str());
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;
	}

	else
	{
		snprintf(row, sizeof(row) - 1, "%-20s\n", m_ipmiUpgradeTime.c_str());
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;

		snprintf(row, sizeof(row) - 1, "%-20s\n", outputStr.c_str());
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
		snprintf(tmpBuf, ITEM_SIZE, "%s", row);
		tmpBuf = tmpBuf + ITEM_SIZE;
	}


	//variables clean
	m_ipmiResult = "" ;
	m_ipmiUpgradeTime = "";

	return respSize;

}

int FIXS_CCH_UpgradeManager::writeIpmiDisplay (char* buffer, int bufferSize)
{
	char *tmpBuf = 0;
	int nItem = 4;

	int respSize = sizeof(nItem) + (ITEM_SIZE * nItem);
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	if (respSize > bufferSize)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
		return -1;
	}
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<std::endl;
	memcpy(buffer, &nItem, sizeof(nItem)); // write number of items, in this case always 1
	tmpBuf = buffer + sizeof(nItem);


	//write headerstring
	snprintf(tmpBuf, ITEM_SIZE, "%-20s", " ");   //to be defined
	tmpBuf = tmpBuf + ITEM_SIZE;

	char row[ITEM_SIZE] = {0};

	snprintf(row, sizeof(row) - 1, "%-20s\n", "IPMI FW INFORMATION");
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
	snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	tmpBuf = tmpBuf + ITEM_SIZE;

	snprintf(row, sizeof(row) - 1, "%-20s", m_ipmiFwType.c_str());
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
	snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	tmpBuf = tmpBuf + ITEM_SIZE;

	snprintf(row, sizeof(row) - 1, "%-20s\n", m_ipmiFwRevision.c_str());
	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " item = " << row << std::endl;
	snprintf(tmpBuf, ITEM_SIZE, "%s", row);
	tmpBuf = tmpBuf + ITEM_SIZE;

	//variables clean
	m_ipmiFwType = "" ;
	m_ipmiFwRevision = "";

	return respSize;
}

int FIXS_CCH_UpgradeManager::ipmiDisplay (std::string magazine, std::string slot)
{
	int result = EXECUTED;

	std::string running("");
	std::string product("");
	std::string revision("");
	unsigned long umagazine = 0;
	unsigned short uslot = 0;

	std::string masterIp("");
	std::string passiveIp("");
	unsigned short masterSlot = 0;

	cout << "Magazine : " << magazine << endl;
	cout << "slot : " << uslot << endl;

	if (getUlongMagazine(magazine, umagazine) == false) return INCORRECT_MAGAZINE;
	if (getUshortSlot(slot, uslot) == false) return INCORRECT_SLOT;

	if ((!isScxData(umagazine,uslot)) && (!isEpbBoard(umagazine,uslot)) && (!isCmxData(umagazine,uslot)) && (!isEvoEtData(umagazine,uslot))) return FUNC_BOARD_NOTALLOWED;

	if(!getMasterPassiveSCX(umagazine, masterIp, passiveIp, masterSlot))
	{
		std::cout << "getMasterPassiveSCX failed" << std::endl;
		return SNMP_FAILURE;
	}

	if(ipmiCheckBoardStatus(umagazine, uslot, masterSlot) == false) return WAIT_FOR_UPGRADE_ONGOING;

	FIXS_CCH_SNMPManager* snmpReader = new (std::nothrow) FIXS_CCH_SNMPManager(masterIp);
	if(!snmpReader->getIPMIBoardData(slot, running, product, revision))
	{
		snmpReader->setIpAddress(passiveIp);
		if(!snmpReader->getIPMIBoardData(slot, running, product, revision))
		{
			std::cout << "get IPMI board data failed" << std::endl;

			if(snmpReader != NULL)
			{
				delete(snmpReader);
				snmpReader = NULL;
			}

			return SNMP_FAILURE;
		}
	}

	if(snmpReader != NULL)
	{
		delete(snmpReader);
		snmpReader = NULL;
	}


	std::cout << "RUNNING : " << running << std::endl;
	std::cout << "PRODUCT : " << product << std::endl;
	std::cout << "REVISION : " << revision << std::endl;
	//running (0=UPG 1=FB)
	if(running.compare("0") == 0)
		m_ipmiFwType = "RUNNING IPMI FW TYPE: UPG";
	else if(running.compare("1") == 0)
		m_ipmiFwType = "RUNNING IPMI FW TYPE: FB";

	m_ipmiFwRevision = "RUNNING IPMI FW VERSION: " + product + " " + revision;


	return result;

}

bool FIXS_CCH_UpgradeManager::getBoardIps (unsigned long magazine, unsigned short slot, std::string &ethA, std::string &ethB)
{
	Lock lock;

	bool result = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			cout << __FUNCTION__<<  " " << __LINE__ << "  FOUND !!!" << endl;
			ethA = it->getIPEthAString();
			ethB = it->getIPEthBString();
			result = true;
			break;
		}
	}

	//check in IPT board list
	if (!result)
	{
		for (swUpgradeList_t::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
		{
			if ((it->m_magazine == magazine) && (it->m_slot == slot))
			{
				cout << __FUNCTION__<<  " " << __LINE__ << "  FOUND !!!" << endl;
				ethA = it->getIPEthAString();
				ethB = it->getIPEthBString();
				result = true;
				break;
			}
		}
	}

	return result;

}

bool FIXS_CCH_UpgradeManager::ipmiCheckBoardStatus (unsigned long umagazine, unsigned short uslot, unsigned short uslotMaster)
{
	bool found = false;
	bool bReturn = true;

	environment = FIXS_CCH_DiskHandler::getEnvironment();

	//CBA environment
	if ((environment == CCH_Util::SINGLECP_CBA) || (environment == CCH_Util::MULTIPLECP_CBA))
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslotMaster))
			{
				if  (it->m_ipmiPtr != 0)
				{
					FIXS_CCH_IpmiUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
					}
					catch (bad_cast)
					{

						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_ipmiPtr is not a FIXS_CCH_IpmiUpgrade", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}
						stateMachine = 0;
					}
					if (!stateMachine)
					{
						//return 1; // Internal Error
						std::cout<<"DBG: "<< __FUNCTION__ << "@" << __LINE__ << " ERROR: stateMachine is NULL" << std::endl;
						//bError = true;
						break;
					}

					//check upgrade ongoing
					if (stateMachine->getStateName() == "ONGOING")
					{
						if (stateMachine->m_slotUpgOnGoing == (int)uslot)
						{
							//found a slot with IPMI ongoing
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
							}

							found = true;
							break;
						}
					}
				}
			}
			//}
		}
	}
	//NOT CBA environment
	else
	{
		//check SCB-RP board
		return true;
	}

	if(found) bReturn = false;

	return bReturn;
}

void FIXS_CCH_UpgradeManager::setBackupOngoing(bool backup)
{
	cout <<"Setting backup ongoing to: ";
	if (backup == true) cout <<"TRUE" <<endl;
	else cout << "FALSE" <<endl;
	m_backup = backup;
}

bool FIXS_CCH_UpgradeManager::isBackupOngoing()
{
	std::string str("");
	if(m_backup == true) str = "TRUE";
	else str = "FALSE";
	cout <<"Backup ongoing : "<<str<<endl;
	return m_backup;
}

bool FIXS_CCH_UpgradeManager::isAnyUpgradeOngoing()
{
	bool found = false;

	if(isScxSoftwareUpgradeInProgress())
		return true;

	if(isCmxSoftwareUpgradeInProgress())
		return true;

	if(isIptLoadModuleUpgradeInProgress())
		return true;

	if(isSmxSoftwareUpgradeInProgress())
		return true;
	//RFU check
	{
		Lock lock;
		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{

			FIXS_CCH_FwUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_FwUpgrade *> (it->m_fwPtr);
			}
			catch (bad_cast)
			{
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_fwPtr is not a FIXS_CCH_FwUpgrade", __FUNCTION__, __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
				}
				stateMachine = 0;
			}

			if (stateMachine && (stateMachine->getStateName() == "ONGOING"))
			{
				//found a slot with IPMI ongoing
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("Found an FW ongoing magazine",LOG_LEVEL_DEBUG);
				found = true;
				break;
			}
		}

	}
	if(found == true)
		return true;

	//PFM Check
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if  (it->m_pfmPtr != 0)
			{


				FIXS_CCH_PowerFanUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_PowerFanUpgrade *> (it->m_pfmPtr);
				}
				catch(bad_cast)
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_pfmPtr is not a FIXS_CCH_PowerFanUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}
					stateMachine = 0;
				}


				if ((stateMachine) && (stateMachine->getStateName() != "IDLE"))
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}
					found = true;
				}

				break;
			}
		}
	}

	if(found == true)
		return true;

	//IPMI Check
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{

			if  (it->m_ipmiPtr != 0)
			{
				FIXS_CCH_IpmiUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_IpmiUpgrade *> (it->m_ipmiPtr);
				}
				catch (bad_cast)
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_ipmiPtr is not a FIXS_CCH_IpmiUpgrade", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					}
					stateMachine = 0;
				}
				if (!stateMachine)
				{
					//return 1; // Internal Error
					std::cout<<"DBG: "<< __FUNCTION__ << "@" << __LINE__ << " ERROR: stateMachine is NULL" << std::endl;
					//bError = true;
					break;
				}

				//check upgrade ongoing
				if (stateMachine->getStateName() == "ONGOING")
				{
					//found a slot with IPMI ongoing
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Upgrade already ongoing ", __FUNCTION__, __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
						FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					}

					found = true;
					break;
				}
			}

		}
	}
	if(found == true)
		return true;
	else
		return false;
}//fn end

bool FIXS_CCH_UpgradeManager::setCurrentProduct(unsigned long magazine, unsigned short slot, std::string container)
{
	bool res = false;

	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && it->m_slot == slot)
		{
			it->m_container = container;
			res = true;
		}
	}

	return res;
}

bool FIXS_CCH_UpgradeManager::setCurrentIptProduct(unsigned long magazine, unsigned short slot, std::string container)
{
	bool res = false;

	for (std::list<FIXS_CCH_UpgradeManager::FsmListData>::iterator it = m_iptUpgradeList->begin(); it != m_iptUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && it->m_slot == slot)
		{
			it->m_container = container;
			res = true;
		}
	}

	return res;
}


bool FIXS_CCH_UpgradeManager::isCmxSoftwareUpgradeInProgress()
{

	//Lock lock;
	bool result = false;

	//	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	//	{
	//		if (it->m_swPtr)
	//		{
	//
	//			FIXS_CCH_ScxUpgrade *stateMachine = 0;
	//			try
	//			{
	//
	//				stateMachine = dynamic_cast<FIXS_CCH_ScxUpgrade *> (it->m_swPtr);
	//			}
	//			catch (bad_cast)
	//			{
	//
	//				{
	//					char tmpStr[512] = {0};
	//					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_ScxUpgrade", __FUNCTION__, __LINE__);
	//					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	//				}
	//				stateMachine = 0;
	//			}
	//
	//			if (stateMachine)
	//			{
	//				int stateSw = stateMachine->getStateImm();
	//				if ((stateSw != IMM_Util::IDLE) && (stateSw != IMM_Util::UNKNOWN) && (stateSw != IMM_Util::FAILED))
	//				{
	//					std::cout << "DBG: AT LEAST ONE SW STATE MACHINE IS NOT IDLE" << std::endl;
	//					result = true;
	//					break;
	//				}
	//			}
	//
	//		}
	//	}

	return result;

}

int FIXS_CCH_UpgradeManager::fetchCMXInfo ()
{
	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(IMM_Util::CMXB); //TODO

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no CMX board found. Any CMX hasn't been configured yet.
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No CMXB board found", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
					retValue = 0;
				}
				else
				{
					// got some CMX board
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						uint32_t magazine = 0;
						returnValue = hwc->getMagazine (magazine, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							// magazine read
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0%x", __LINE__, magazine);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}

							unsigned short slot = 0;
							returnValue = hwc->getSlot (slot, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								// slot read
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								if (isCmxData(magazine, slot) == false)
								{
									std::string container("");
									std::string containerValue("");
									std::string dn_blade("");

									if(IMM_Interface::getDnBoard(magazine,slot,dn_blade))
									{
										returnValue = hwc->getContainerPackage (container, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
											//										if (IMM_Interface::getOtherBladeCurrLoadModule(dn_blade,container))
										{
											uint32_t IP_EthA = 0;
											returnValue = hwc->getIPEthA (IP_EthA, boardId);
											if (returnValue == ACS_CS_API_NS::Result_Success)
											{
												uint32_t IP_EthB = 0;
												returnValue = hwc->getIPEthB (IP_EthB, boardId);
												if (returnValue == ACS_CS_API_NS::Result_Success)
												{
													containerValue=container;
													if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
													{														
														std::string IPA("");
														std::string IPB("");
														std::string cxc_no("");
														std::string mag("");
														IPA = CCH_Util::ulongToStringIP(IP_EthA);
														IPB = CCH_Util::ulongToStringIP(IP_EthB);
														if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("entering after restore..calling checkCurrentVersionInBoard function ",LOG_LEVEL_WARN);
														bool ret = checkCurrentVersionInBoard (IPA, IPB,cxc_no); 
														if(ret)
														{
															ACS_CS_API_SET_NS::CS_API_Set_Result result;
															std::stringstream ss;
															ss << slot;
															std::string _slot = ss.str();
															CCH_Util::ulongToStringMagazine(magazine,mag);
															std::string mag_slot = mag + "_" + _slot;
															std::cout << "-------mag_slot= " << mag_slot.c_str() << std::endl;
															std::string currCxc = ACS_APGCC::after(cxc_no,"CMX_");
															std::string currContainer = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(currCxc);
															if(!currContainer.empty())
                                                                                                                        {
                                                                                                                                containerValue=currContainer;
                                                                                                                                result = ACS_CS_API_Set::setSoftwareVersion(containerValue, mag, slot);
                                                                                                                                bool retryVal=false;
                                                                                                                                if(result!=ACS_CS_API_SET_NS::Result_Success)
                                                                                                                                {
                                                                                                                                        retryVal=retrySetSoftwareVersion(containerValue, mag, slot);
                                                                                                                                }
                                                                                                                                else
                                                                                                                                        retryVal=true;
                                                                                                                                if(retryVal==false) {
                                                                                                                                        containerValue=container;
                                                                                                                                }
                                                                                                                        }
                                                                                                                }
                                                                                                        }
													addCmxEntry(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue,dn_blade);
													retValue = 0;
												}
												else
												{
													//serious fault in CS: No IP_EthB found for the CMX board
													{
														char tmpStr[512] = {0};
														snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the CMX board", __LINE__);
														if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
													}

													retValue = -7;
													break;
												}
											}
											else
											{
												//serious fault in CS: No IP_EthA found for the CMX board
												{
													char tmpStr[512] = {0};
													snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the CMX board", __LINE__);
													if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
												}

												retValue = -6;
												break;
											}
										}
										else
										{
											//serious fault in CS: No package found for the CMX board
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No package found for the CMX board", __LINE__);
												if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
											}

											retValue = -5;
											break;
										}
									}
								}
								else
								{
									//data already exists
									retValue = 0;
									std::cout << "data already exists..." << std::endl;
								}
							}
							else
							{
								//serious fault in CS: No slot found for the CMX board
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the CMX board", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								retValue = -4;
								break;
							}

						}
						else
						{
							//serious fault in CS: No magazine found for the CMX board
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the CMX board", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}
							retValue = -3;
							break;
						}
					}
				}


				/////////////////////////////
			}
			else
			{
				{//trace - GetBoardIds failed, error from CS
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				retValue = -2;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance failed, error from CS", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}

			retValue = -1;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		}
		retValue = -1;

	}

	return retValue;

}

int FIXS_CCH_UpgradeManager::fetchEVOETInfo ()
{
	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(IMM_Util::EVOET); //TODO

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no EVOET board found. Any EVOET hasn't been configured yet.
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No EVOET board found", __LINE__);
						if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					}
					retValue = 0;
				}
				else
				{
					// got some EVOET board
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						uint32_t magazine = 0;
						returnValue = hwc->getMagazine (magazine, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							// magazine read
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] magazine read:0%x", __LINE__, magazine);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}

							unsigned short slot = 0;
							returnValue = hwc->getSlot (slot, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								// slot read
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] slot read:%u", __LINE__, slot);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								if (isEvoEtData(magazine, slot) == false)
								{
									std::string container("");
									std::string containerValue("");
									std::string dn_blade("");

									if(IMM_Interface::getDnBoard(magazine,slot,dn_blade))
									{
										returnValue = hwc->getContainerPackage (container, boardId);
										if (returnValue == ACS_CS_API_NS::Result_Success)
										{
											uint32_t IP_EthA = 0;
											returnValue = hwc->getIPEthA (IP_EthA, boardId);
											if (returnValue == ACS_CS_API_NS::Result_Success)
											{
												uint32_t IP_EthB = 0;
												returnValue = hwc->getIPEthB (IP_EthB, boardId);
												if (returnValue == ACS_CS_API_NS::Result_Success)
												{													
													containerValue=container;
													if(FIXS_CCH_DiskHandler::isStartedAfterRestore())
													{
														std::string IPA("");
														std::string IPB("");
														std::string cxc_no("");
														std::string mag("");
														IPA = CCH_Util::ulongToStringIP(IP_EthA);
														IPB = CCH_Util::ulongToStringIP(IP_EthB);
														if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("entering after restore..calling checkCurrentVersionInBoard function ",LOG_LEVEL_WARN);
														bool ret = checkCurrentVersionInBoard (IPA, IPB,cxc_no); 
														if(ret)
														{
															ACS_CS_API_SET_NS::CS_API_Set_Result result;
															std::stringstream ss;
															ss << slot;
															std::string _slot = ss.str();
															CCH_Util::ulongToStringMagazine(magazine,mag);
															std::string mag_slot = mag + "_" + _slot;
															std::cout << "-------mag_slot= " << mag_slot.c_str() << std::endl;
															std::string currCxc = ACS_APGCC::after(cxc_no,"_");
															std::string currContainer = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(currCxc);
															if(!currContainer.empty())
                                                                                                                        {
                                                                                                                                containerValue=currContainer;
                                                                                                                                result = ACS_CS_API_Set::setSoftwareVersion(containerValue, mag, slot);
                                                                                                                                bool retryVal=false;
                                                                                                                                if(result!=ACS_CS_API_SET_NS::Result_Success)
                                                                                                                                {
                                                                                                                                        retryVal=retrySetSoftwareVersion(containerValue, mag, slot);
                                                                                                                                }
                                                                                                                                else
                                                                                                                                        retryVal=true;
                                                                                                                                if(retryVal==false) {
                                                                                                                                        containerValue=container;
                                                                                                                                }
                                                                                                                        }
                                                                                                                }
                                                                                                        }
													addEvoEtEntry(boardId, magazine, slot, IP_EthA, IP_EthB, containerValue,dn_blade);
													retValue = 0;
												}
												else
												{
													//serious fault in CS: No IP_EthB found for the EVOET board
													{
														char tmpStr[512] = {0};
														snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the EVOET board", __LINE__);
														if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
													}

													retValue = -7;
													break;
												}
											}
											else
											{
												//serious fault in CS: No IP_EthA found for the EVOET board
												{
													char tmpStr[512] = {0};
													snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the EVOET board", __LINE__);
													if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
												}

												retValue = -6;
												break;
											}
										}
										else
										{
											//serious fault in CS: No package found for the EVOET board
											{
												char tmpStr[512] = {0};
												snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No package found for the EVOET board", __LINE__);
												if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
											}

											retValue = -5;
											break;
										}
									}
								}
								else
								{
									//data already exists
									retValue = 0;
									std::cout << "data already exists..." << std::endl;
								}
							}
							else
							{
								//serious fault in CS: No slot found for the EVOET board
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No slot found for the EVOET board", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								}
								retValue = -4;
								break;
							}

						}
						else
						{
							//serious fault in CS: No magazine found for the EVOET board
							{
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the EVOET board", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							}
							retValue = -3;
							break;
						}
					}
				}


				/////////////////////////////
			}
			else
			{
				{//trace - GetBoardIds failed, error from CS
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				retValue = -2;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] createBoardSearchInstance failed, error from CS", __FUNCTION__, __LINE__);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}

			retValue = -1;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Memory error", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		}
		retValue = -1;

	}

	return retValue;
}

bool FIXS_CCH_UpgradeManager::isCmxData (unsigned long magazine, unsigned short slot)
{
	Lock lock;

	bool result = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			if (it->m_fbn == CCH_Util::CMXB)
			{
				cout << __FUNCTION__<<  " " << __LINE__ << "  FOUND !!!" << endl;
				result = true;
				break;
			}
		}
	}

	return result;
}

bool FIXS_CCH_UpgradeManager::isEvoEtData (unsigned long magazine, unsigned short slot)
{
	Lock lock;

	bool result = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			if (it->m_fbn == CCH_Util::EVOET)
			{
				cout << __FUNCTION__<<  " " << __LINE__ << "  FOUND !!!" << endl;
				result = true;
				break;
			}
		}
	}

	return result;
}

void FIXS_CCH_UpgradeManager::addCmxEntry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm)
{
	Lock lock;

	//to verify
	UNUSED(fsm);

	FsmListData data(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn, CCH_Util::CMXB);
	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " added magazine: " << data.m_magazine << ", slot:" << data.m_slot << ", IP_EthA:" << data.getIPEthAString().c_str() << ", IP_EthB:" << data.getIPEthBString().c_str() << "DN: " << data.m_dn <<std::endl;
	m_swUpgradeList->push_back(data);


	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ipA[%s] ipB[%s] cxp[%s] ", __FUNCTION__,__LINE__, MagValue.c_str(), data.m_slot, data.getIPEthAString().c_str(),data.getIPEthBString().c_str(), data.m_container.c_str());
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::createBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: addCmxEntry , FAILED to create persistance object !!!"<< endl;

	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;
	if (!IMM_Interface::setUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: addCmxEntry , FAILED to set used package !!!"<< endl;
}

void FIXS_CCH_UpgradeManager::addEvoEtEntry (unsigned short boardId, unsigned long magazine, unsigned short slot, unsigned long ip_ethA, unsigned long ip_ethB, std::string container, std::string dn ,  FIXS_CCH_StateMachine *fsm)
{
	Lock lock;

	//to verify
	UNUSED(fsm);

	FsmListData data(boardId, magazine, slot, ip_ethA, ip_ethB, container, dn, CCH_Util::EVOET);
	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " added magazine: " << data.m_magazine << ", slot:" << data.m_slot << ", IP_EthA:" << data.getIPEthAString().c_str() << ", IP_EthB:" << data.getIPEthBString().c_str() << "DN: " << data.m_dn <<std::endl;
	m_swUpgradeList->push_back(data);


	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ipA[%s] ipB[%s] cxp[%s] ", __FUNCTION__,__LINE__, MagValue.c_str(), data.m_slot, data.getIPEthAString().c_str(),data.getIPEthBString().c_str(), data.m_container.c_str());
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::createBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: addEvoEtEntry , FAILED to create persistance object !!!"<< endl;

	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;
	if (!IMM_Interface::setUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: addEvoEtEntry , FAILED to set used package !!!"<< endl;
}

bool FIXS_CCH_UpgradeManager::removeCmxEntry(unsigned long magazine, unsigned short slot, std::string & tempExpectedCxp)
{
	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);

	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ", __FUNCTION__,__LINE__, MagValue.c_str(), slot);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::deleteBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: removeCmxEntry , FAILED to delete persistance object !!!"<< endl;

	//get current load module
	std::string container = getCurrentProduct(magazine,slot);

	//create value to search and delete from used package
	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;

	if (!IMM_Interface::setNotUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeCmxEntry , FAILED to set used package !!!"<< endl;
	// remove entry for the board from expected CXP package
	if (!IMM_Interface::setNotUsedPackage(tempExpectedCxp,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeCmxEntry , FAILED to set used package !!!"<< endl;

	IMM_Interface::removeReportProgressObject(MagValue,sSlot,container);

	return true;
}

bool FIXS_CCH_UpgradeManager::removeEvoEtEntry(unsigned long magazine, unsigned short slot)
{
	std::string MagValue("");
	(void) CCH_Util::ulongToStringMagazine (magazine, MagValue);

	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s:%d] magazine[%s] slot[%d] ", __FUNCTION__,__LINE__, MagValue.c_str(), slot);
	if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);

	if (!IMM_Interface::deleteBladeSwPersistance(magazine,slot))
		cout << "FIXS_CCH_UpgradeManager: removeEvoEtEntry , FAILED to delete persistance object !!!"<< endl;

	//get current load module
	std::string container = getCurrentProduct(magazine,slot);

	//create value to search and delete from used package
	std::string sSlot = CCH_Util::intToString(slot);
	std::string usedRef = MagValue+"_"+ sSlot;

	if (!IMM_Interface::setNotUsedPackage(container,usedRef))
		cout << "FIXS_CCH_UpgradeManager: removeEvoEtEntry , FAILED to set used package !!!"<< endl;

	IMM_Interface::removeReportProgressObject(MagValue,sSlot,container);

	return true;
}

int FIXS_CCH_UpgradeManager::prepareCmx (unsigned long umagazine, unsigned short uslot, std::string product)
{

	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,25) == false)	return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;
	if(fwCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;


	std::string container = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(product);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] product: %s  -  container: %s ", __FUNCTION__, __LINE__,product.c_str(), container.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}

	//call state machine method
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				if (it->m_swPtr == 0 )
				{
					FIXS_CCH_CmxUpgrade *swup = new FIXS_CCH_CmxUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container, it->getIPEthAString(), it->getIPEthBString(), it->m_dn);
					std::cout << __FUNCTION__ <<"Initialize  FIXS_CCH_CmxUpgrade "<<__LINE__ <<std::endl;
					if (swup)
					{
						it->m_swPtr = swup;
					}
				}

				{
					FIXS_CCH_CmxUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);
					}
					catch (bad_cast)
					{
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_CmxUpgrade", __FUNCTION__, __LINE__);
							_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						stateMachine = 0;
					}

					if (stateMachine)
					{
						//result = stateMachine->prepare(product, container);

						if (!stateMachine->isRunningThread())
						{
							std::cout << "  Thread StateMachine is not running..........START " << std::endl;
							stateMachine->open();
						}
						else std::cout << "  Thread StateMachine is already running.........." << std::endl;

						result = stateMachine->prepare(container, product);
						if (( result != WRONG_OPTION_PREPARE) && (result != SW_ALREADY_ON_BOARD) && (result != 0))
						{
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("CMX UPGRADE : Stoppping thread...",LOG_LEVEL_DEBUG);
							stateMachine->stop();
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
						else if (result == EXECUTED) //OK
						{
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
					}
					else
						result = 1;//INTERNALERR;

					break;
				}

			}
		}
	}

	return result;
}

int FIXS_CCH_UpgradeManager::activateCmx (unsigned long umagazine, unsigned short uslot)
{

	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;

	// check if state machine exist. if not, create the FSM
	// else... do this:

	Lock lock;


	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{

		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			FIXS_CCH_CmxUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_CmxUpgrade", __FUNCTION__, __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
				}

				stateMachine = 0;
			}
			if (stateMachine)
			{
				if (!stateMachine->isRunningThread())
				{
					std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					stateMachine->open();
				}
				else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				result = stateMachine->activateSW();

			}
			else
				result = 1;

			break;
		}

	}

	return result;
}

int FIXS_CCH_UpgradeManager::cancelCmx (unsigned long umagazine, unsigned short uslot)
{
	int result = 0;
	//unsigned short otherSlot = 0;

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;

	Lock lock;

	//	result = HARDWARE_NOT_PRESENT; // 14;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			FIXS_CCH_CmxUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{
				{ // log
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_CmxUpgrade", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr); 
				}
				stateMachine = 0;
			}

			if (stateMachine)
			{
				if (!stateMachine->isRunningThread())
				{
					std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					stateMachine->open();

				}
				else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				result = stateMachine->cancel();
			}
			else
			{
				result = 1;
			}
			break;
		}//slot mag
	}//for

	return result;
}

int FIXS_CCH_UpgradeManager::commitCmx (unsigned long umagazine, unsigned short uslot)
{

	int result = 0;
	//unsigned short otherSlot = 0;
	std::string container("");

	if(checkOtherSlotStatus(umagazine,uslot) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(checkOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,0) == false) return UPGRADE_ALREADY_ONGOING;
	if(pfmCheckOtherSlotStatus(umagazine,25) == false) return UPGRADE_ALREADY_ONGOING;
	if(ipmiCheckOtherSlotStatus(umagazine) == false) return UPGRADE_ALREADY_ONGOING;

	// check if state machine exist
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{

			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				FIXS_CCH_CmxUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_CmxUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_CmxUpgrade", __FUNCTION__, __LINE__);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}

				if (stateMachine)
				{
					if (!stateMachine->isRunningThread())
					{
						std::cout << "  Thread StateMachine is not running..........START " << std::endl;
						stateMachine->open();

					}
					else std::cout << "  Thread StateMachine is already running.........." << std::endl;

					result = stateMachine->commit();

				}
				else
				{
					result = 1;
				}
				break;
			}
		}
	}
	return result;
}

bool FIXS_CCH_UpgradeManager::getMasterPassiveSCB(unsigned long magazine, std::string& masterIpA, std::string& masterIpB, unsigned short& masterSlot)
{
	std::string dn_blade0("");
	std::string dn_blade25("");
	std::string ipAslot0("");
	std::string ipBslot0("");
	std::string ipAslot25("");
	std::string ipBslot25("");

	unsigned short slot = 0;

	if(IMM_Interface::getDnBoard(magazine,slot,dn_blade0))
	{
		if(!IMM_Interface::getOtherBladeIPs(dn_blade0, ipAslot0, ipBslot0))
		{
			std::cout << " Getting IPs failed for dn :  " << dn_blade0 << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << " Getting DN failed for  : " << magazine << std::endl;
		return false;
	}

	slot = 25;

	if(IMM_Interface::getDnBoard(magazine,slot,dn_blade25))
	{
		if(!IMM_Interface::getOtherBladeIPs(dn_blade25, ipAslot25, ipBslot25))
		{
			std::cout << " Getting IPs failed for dn : " << dn_blade25 << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << " Getting DN failed for  : " << magazine << std::endl;
		return false;
	}

	FIXS_CCH_SNMPManager* snmpReader = new (std::nothrow) FIXS_CCH_SNMPManager(ipAslot0);
	int snmpRes = snmpReader->isMasterSCB(ipAslot0, 0);
	if(snmpRes == 1) //master
	{
		masterIpA = ipAslot0;
		masterIpB = ipBslot0;
		masterSlot = 0;
	}
	else
	{
		int snmpRes = snmpReader->isMasterSCB(ipBslot0, 0);
		if(snmpRes == 1) //master
		{
			masterIpA = ipAslot0;
			masterIpB = ipBslot0;
			masterSlot = 0;
		}

		else
		{
			int snmpRes = snmpReader->isMasterSCB(ipAslot25, 25);
			if(snmpRes == 1) //master
			{
				masterIpA = ipAslot25;
				masterIpB = ipBslot25;
				masterSlot = 25;
			}
			else
			{
				int snmpRes = snmpReader->isMasterSCB(ipBslot25, 25);
				if(snmpRes == 1) //master
				{
					masterIpA = ipAslot25;
					masterIpB = ipBslot25;
					masterSlot = 25;
				}
				else
				{
					//All SNMP req fail
					{
						std::cout << " Getting master status failed" << std::endl;
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool FIXS_CCH_UpgradeManager::checkIplbUpgradeStatus (unsigned long magazine, unsigned short slot)
{
	Lock lock;

	bool result = false;
	cout << "Checking for IPLB Data" << "magazine" << magazine << "slot" << slot;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == magazine) && (it->m_slot == slot))
		{
			cout << "FBN with magazine match" << it->m_fbn  ;
			if (it->m_fbn == CCH_Util::IPLB )
			{
				FIXS_CCH_IplbUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IplbUpgrade",
								__FUNCTION__, __LINE__);

						if (_trace->ACS_TRA_ON())
							_trace->ACS_TRA_event(1, tmpStr);
						if(FIXS_CCH_logging)
							FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

					}
					stateMachine = 0;

				}

				if (stateMachine)
				{
					int stateLm = stateMachine->getStateImm();
					if ((stateLm != IMM_Util::IDLE) && (stateLm != IMM_Util::UNKNOWN) && (stateLm != IMM_Util::FAILED))
					{
						std::cout << "DBG: IPLB LM STATE MACHINE IS NOT IDLE ON SLOT =" << slot<<std::endl;
						result = true;
						break;
					}
				}
			}
		}
	}
	return result;

}

bool FIXS_CCH_UpgradeManager::isIplbLoadModuleUpgradeInProgress()
{

	//Lock lock;
	bool result = false;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (it->m_swPtr)
		{

			FIXS_CCH_IplbUpgrade *stateMachine = 0;
			try
			{

				stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IplbUpgrade", __FUNCTION__, __LINE__);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				stateMachine = 0;
			}

			if (stateMachine)
			{
				int stateSw = stateMachine->getStateImm();
				if ((stateSw != IMM_Util::IDLE) && (stateSw != IMM_Util::UNKNOWN) && (stateSw != IMM_Util::FAILED))
				{
					std::cout << "DBG: AT LEAST ONE SW STATE MACHINE IS NOT IDLE" << std::endl;
					result = true;
					break;
				}
			}

		}
	}

	return result;
}

int FIXS_CCH_UpgradeManager::prepareIplb (unsigned long umagazine, unsigned short uslot, std::string product, int upgradeType, bool force)
{
	int result = 0;
	std::string dn("");

	unsigned short islot=65;
	IMM_Interface::getOtherBoardSlot(umagazine,uslot,islot);

	std::cout << "prepareIplb::Checking if upgrade is already ongoing on other slot"<<std::endl;
	if(force!=1){
		if(checkIplbUpgradeStatus(umagazine,islot))
		{
			std::cout << "prepareIplb::Upgrade is already ongoing on SLOT "<<islot<<std::endl;
			return UPGRADE_ALREADY_ONGOING;
		}
		std::cout << "prepareIplb::Upgrade is not ongoing on SLOT "<<islot<<std::endl;
	}
	std::string container = FIXS_CCH_SoftwareMap::getInstance()->getContainerByProduct(product);
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] product: %s  -  container: %s ", __FUNCTION__, __LINE__,product.c_str(), container.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}

	//check for IPLB supervision
	if(force!=1)
	{
		int resp = 0;
		resp = checkIplbSupervision(umagazine, uslot, islot);
		if(resp != 0)
		{
			return resp;
		}
	}


	//call state machine method
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{
			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				if (it->m_swPtr == 0 )
				{
					FIXS_CCH_IplbUpgrade *swup = new FIXS_CCH_IplbUpgrade(it->m_boardId, it->m_magazine, it->m_slot, it->m_container,it->getIPEthAString(), it->getIPEthBString(), it->m_dn );
					std::cout << __FUNCTION__ <<"Initialize  FIXS_CCH_IplbUpgrade "<<__LINE__ <<std::endl;
					if (swup)
					{
						it->m_swPtr = swup;
						swup->setUpgradeType(upgradeType);
						swup->setForcedUpgrade(force);
					}

				}

				{
					FIXS_CCH_IplbUpgrade *stateMachine = 0;
					try
					{
						stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);

						(stateMachine)->setUpgradeType(upgradeType);
						(stateMachine)->setForcedUpgrade(force);
					}
					catch (bad_cast)
					{
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IplbUpgrade", __FUNCTION__, __LINE__);
							_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						}

						stateMachine = 0;
					}

					if (stateMachine)
					{

						if (!stateMachine->isRunningThread())
						{
							std::cout << "  Thread StateMachine is not running..........START " << std::endl;
							stateMachine->open();
						}
						else std::cout << "  Thread StateMachine is already running.........." << std::endl;
						FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn,container);
						std::cout << "upgradeType" << upgradeType << std::endl;

						if (upgradeType == 0) //normal(partial)
						{
							result = stateMachine->prepare(container, product,true);
						}
						else if (upgradeType == 1) //initial (complete)
						{
							result = stateMachine->prepare(container, product,false);
						}
						if (( result != WRONG_OPTION_PREPARE) && (result != 0))
						{
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("IPLB UPGRADE : Stoppping thread...",LOG_LEVEL_DEBUG);
							stateMachine->stop();
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
						else if (result == EXECUTED) //OK
						{
							std::cout << "------------------------------------------------------------------- " << std::endl;
						}
					}
					else
						result = 1;//INTERNALERR;
					break;
				}
			}
		}
	}
	return result;
}
int FIXS_CCH_UpgradeManager::activateIplb (unsigned long umagazine, unsigned short uslot)
{
	int result = 0;
	//unsigned short otherSlot = 0;
	std::string container("");
	Lock lock;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			FIXS_CCH_IplbUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IplbUpgrade", __FUNCTION__, __LINE__);
					_trace->ACS_TRA_event(1, tmpStr);
				}

				stateMachine = 0;
			}
			if (stateMachine)
			{
				if (!stateMachine->isRunningThread())
				{
					std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					stateMachine->open();
				}
				else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				result = stateMachine->activateSw();

			}
			else
				result = 1;

			break;
		}
	}
	return result;
}

int FIXS_CCH_UpgradeManager::commitIplb (unsigned long umagazine, unsigned short uslot, bool setAsDefaultPackage)
{
	int result = 0;
	//unsigned short otherSlot = 0;
	std::string container("");

	// check if state machine exist
	{
		Lock lock;

		for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
		{

			if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
			{
				FIXS_CCH_IplbUpgrade *stateMachine = 0;
				try
				{
					stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);
				}
				catch(bad_cast)
				{

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IptbUpgrade", __FUNCTION__, __LINE__);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
					}
					stateMachine = 0;
				}

				if (stateMachine)
				{
					if (!stateMachine->isRunningThread())
					{
						std::cout << "  Thread StateMachine is not running..........START " << std::endl;
						stateMachine->open();

					}
					else std::cout << "  Thread StateMachine is already running.........." << std::endl;

					result = stateMachine->commit(setAsDefaultPackage);

				}
				else
				{
					result = 1;
				}
				break;
			}
		}
	}
	return result;
}
int FIXS_CCH_UpgradeManager::cancelIplb (unsigned long umagazine, unsigned short uslot)
{
	int result = 0;
	//unsigned short otherSlot = 0;
	FIXS_CCH_IplbUpgrade *stateMachine = 0;
	Lock lock;

	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{

		if ((it->m_magazine == umagazine) && (it->m_slot == uslot))
		{
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{
				{ // log
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IplbUpgrade", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				}
				stateMachine = 0;
			}

			if (stateMachine)
			{
				if (!stateMachine->isRunningThread())
				{
					std::cout << "  Thread StateMachine is not running..........START " << std::endl;
					stateMachine->open();

				}
				else std::cout << "  Thread StateMachine is already running.........." << std::endl;

				result = stateMachine->cancel();
			}
			else
			{
				result = 1;
			}
			break;
		}//slot mag
	}//for
	return result;
}

bool FIXS_CCH_UpgradeManager::isIplbLoadModuleUpgradeInProgress(std::string container)
{

	bool result = false;
	for (swUpgradeList_t::iterator it = m_swUpgradeList->begin(); it != m_swUpgradeList->end(); it++)
	{
		if (it->m_swPtr)
		{
			FIXS_CCH_IplbUpgrade *stateMachine = 0;
			try
			{
				stateMachine = dynamic_cast<FIXS_CCH_IplbUpgrade *> (it->m_swPtr);
			}
			catch (bad_cast)
			{

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Caught bad_cast exception. it->m_swPtr is not a FIXS_CCH_IplbUpgrade", __FUNCTION__, __LINE__);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr); if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				stateMachine = 0;
			}
			if (stateMachine)
			{
				cout << "stateMachine->getUpgradedCXP()" << stateMachine->getUpgradedCXP() << endl;
				if(stateMachine->getUpgradedCXP()== container)
				{
					int stateLm = stateMachine->getStateImm();
					if ((stateLm != IMM_Util::IDLE) && (stateLm != IMM_Util::UNKNOWN) && (stateLm != IMM_Util::FAILED))
					{
						std::cout << "DBG: AT LEAST ONE IPLB LM STATE MACHINE IS NOT IDLE" << std::endl;
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] FIXS_CCH_UpgradeManager:IPTB UPGRADE ONGOING!!!", __FUNCTION__, __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						}
						result = true;
						break;
					}
				}
			}
		}
	}

	return result;

}
int FIXS_CCH_UpgradeManager::checkIplbSupervision(unsigned long umagazine,unsigned short uslot, unsigned short islot)
{
	FIXS_CCH_IronsideManager *ironsideManager = new FIXS_CCH_IronsideManager(dmxc_address[0],dmxc_address[1]);
	if(islot!=65)//get islot is success
	{
		cout<<"check of boardBlock on other slot"<<islot<<endl;

		if((ironsideManager->isBoardblocked(umagazine,islot))!=0)
		{
			cout<<"Other Iplb BoardBlocked"<<endl;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager : Other IPLB Board Blocked ",LOG_LEVEL_DEBUG);
			delete ironsideManager;
			return OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE;
		}
		std::string gprId7 = CCH_Util::intToString(263);
		std::string output="";
		std::string success="255.255.255.255.0.0.255.255";
		cout<<"check of Gprdata"<<endl;
		if(!ironsideManager->getGprData(output,umagazine,islot,gprId7))
		{
			cout<<"check of output compare:"<<output<<"succ:"<<success<<endl;
			if(output.compare(success)!=0)
			{
				cout<<"output compare failed"<<endl;
				char tmpChar[512] = {0};
				snprintf(tmpChar, sizeof(tmpChar) - 1,
						"GPR register 7 is set on slot %d",islot);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_DEBUG);
				delete ironsideManager;
				return OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE;
			}
		}
		else
		{
			cout<<"read of Ram reg fail"<<endl;
			char tmpChar[512] = {0};
			snprintf(tmpChar, sizeof(tmpChar) - 1,
					"getGprDataValue read of RAM register 7 is not ok on slot %d",islot);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpChar,LOG_LEVEL_ERROR);
			delete ironsideManager;
			return IRONSIDE_FAILURE;
		}
	}
	/*    //Check for current board block
    cout<<"check of Board Block on own slot"<<uslot<<endl;
    if(ironsideManager->isBoardblocked(umagazine,uslot)!=0)
    {
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_UpgradeManager: Current IPLB Board Blocked ",LOG_LEVEL_DEBUG);
        delete ironsideManager;
	return BOARD_ALREADY_BLOCKED;
    }*/
	delete ironsideManager;
	return false;
}
bool FIXS_CCH_UpgradeManager::checkCurrentVersionInBoard (string & IP_EthA, string & IP_EthB, string & cxc_no)
{

	FIXS_CCH_SNMPManager * m_snmpManager = NULL;

	bool result = false;
	int countFlag=0;
	string ipadd;

	m_snmpManager = new FIXS_CCH_SNMPManager(IP_EthA);

	do
	{
		if (m_snmpManager->getCurrentDefaultVersion(cxc_no) && (cxc_no.empty() == false))
		{
			//snmp success
			std::cout<< __FUNCTION__ << "@" << __LINE__ << ", SNMP value == " << cxc_no.c_str() << std::endl;
			result = true;
			break;
		}
		else if (countFlag == 0)
		{

			m_snmpManager->setIpAddress(IP_EthB);

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in getCurrentLoadedVersion(&cxc_no)-switching IP Address(%s) "
                                                , __FUNCTION__, __LINE__, m_snmpManager->getIpAddress().c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
			}
			countFlag++;
			result = false;
                }
		else
		{
			cout<< __FUNCTION__ << "@" << __LINE__ << " NO VALUE !!!!!!!! " << endl;

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error in getCurrentLoadedVersion(&cxc_no) "
				                                        , __FUNCTION__, __LINE__);
      				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
			}
			countFlag++;
			result = false;
		}
	}
	while(countFlag == 1);

	if (m_snmpManager != NULL)
	{
		delete(m_snmpManager);
		m_snmpManager = NULL;
	}
	return result;
}
bool FIXS_CCH_UpgradeManager::retrySetSoftwareVersion (std::string currContainer, std::string mag, unsigned short slot)
{
	EventHandle m_shutdownEvent=0;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;
	const int Shutdown_Event_Index=0;
        const int Number_Of_Alarm_Events=1;
                                        DWORD Safe_Timeout = 1000;
                                        bool m_exit=false;
                                        int count=0;
					bool valueSet=true;
					ACS_CS_API_SET_NS::CS_API_Set_Result result;
                                        if (m_shutdownEvent == 0)
                                                m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

                                        EventHandle handleArr=m_shutdownEvent;
                                        while (!m_exit) // Set to true when the thread shall exit
                                        {
                                                if(count >= 60){
                                                        m_exit=true;
							valueSet=false;
                                                        break;
                                                }
                                                //wait for 1s
                                                int returnValue = FIXS_CCH_Event::WaitForEvents(Number_Of_Alarm_Events, &handleArr, Safe_Timeout);
						switch (returnValue)
                                                {
                                                        case WAIT_TIMEOUT:  // Time out
                                                        {
								result=ACS_CS_API_Set::setSoftwareVersion(currContainer, mag, slot);
                                                                if(result==ACS_CS_API_SET_NS::Result_Success)
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
								valueSet=false;
                                                                break;
                                                        }
                                                        default:
                                                        {
                                                                count++;
                                                                break;
                                                        }
                                                } // End of switch
                                        }//while
	return valueSet;
}

void FIXS_CCH_UpgradeManager::removeUnusedFaultySwObjects()
{
	std::string dn ="";
	for(unsigned j=0; j<FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects.size(); j++)
	{
		if(!FIXS_CCH_SoftwareMap::getInstance()->isUsedPackage(FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str()))		
		{
			FIXS_CCH_SoftwareMap::getInstance()->getDnFromPackage(dn, FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
			if (IMM_Util::deleteImmObject(dn,ACS_APGCC_SUBTREE)) std::cout<<" OBJECT DELETED !!! "<<dn<<endl;

			std::string fbn = FIXS_CCH_SoftwareMap::getInstance()->getFBN(FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());

			if (strcmp(fbn.c_str(),"SCXB" ) == 0){

				if(FIXS_CCH_SoftwareMap::getInstance()->isDefault (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str())) {
					FIXS_CCH_SoftwareMap::getInstance()->removeScxEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
					FIXS_CCH_SoftwareMap::getInstance()->setFirstScxDefaultPackage();
				} else {
					FIXS_CCH_SoftwareMap::getInstance()->removeScxEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
				}
			}
			else if (strcmp(fbn.c_str(),"IPLB" ) == 0) {
				if(FIXS_CCH_SoftwareMap::getInstance()->isDefault (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str())) {
				FIXS_CCH_SoftwareMap::getInstance()->removeIplbEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
				FIXS_CCH_SoftwareMap::getInstance()->setFirstIplbDefaultPackage();
				} else {
					FIXS_CCH_SoftwareMap::getInstance()->removeIplbEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
				}	
			}
			else if (strcmp(fbn.c_str(),"CMXB") == 0) {
				if(FIXS_CCH_SoftwareMap::getInstance()->isDefault (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str())) {
					FIXS_CCH_SoftwareMap::getInstance()->removeCmxEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
					FIXS_CCH_SoftwareMap::getInstance()->setFirstCmxDefaultPackage();
				} else {
					FIXS_CCH_SoftwareMap::getInstance()->removeCmxEntry(FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
				}
			}
			else if (strcmp(fbn.c_str(), "IPTB" ) == 0) {
				if(FIXS_CCH_SoftwareMap::getInstance()->isDefault (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str())) {
					FIXS_CCH_SoftwareMap::getInstance()->removeIptEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
					FIXS_CCH_SoftwareMap::getInstance()->setFirstIptDefaultPackage();
				} else {
					FIXS_CCH_SoftwareMap::getInstance()->removeIptEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
				}
			}
			else if (strcmp(fbn.c_str(),"SMXB" ) == 0) {
				if(FIXS_CCH_SoftwareMap::getInstance()->isDefault (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str())) {
					FIXS_CCH_SoftwareMap::getInstance()->removeSmxEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
					FIXS_CCH_SoftwareMap::getInstance()->setFirstSmxDefaultPackage();
				} else {
					FIXS_CCH_SoftwareMap::getInstance()->removeSmxEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
				}
			}
			else if (strcmp(fbn.c_str(), "EPB1" ) == 0) {
				if(FIXS_CCH_SoftwareMap::getInstance()->isDefault (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str())) {
					FIXS_CCH_SoftwareMap::getInstance()->removeEpb1Entry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
					FIXS_CCH_SoftwareMap::getInstance()->setFirstEpb1DefaultPackage();
				} else {
					FIXS_CCH_SoftwareMap::getInstance()->removeEpb1Entry(FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
				}
			}
			else if (strcmp(fbn.c_str(), "EVOET" ) == 0) {
				if(FIXS_CCH_SoftwareMap::getInstance()->isDefault (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str())) {
					FIXS_CCH_SoftwareMap::getInstance()->removeEvoEtEntry (FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
					FIXS_CCH_SoftwareMap::getInstance()->setFirstEvoEtDefaultPackage();
				} else {
					FIXS_CCH_SoftwareMap::getInstance()->removeEvoEtEntry(FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
				}
			}
			/*std::vector<std::string>::iterator position = std::find(FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects.begin(), FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects.end(), FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects[j].c_str());
			if (position != FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects.end())
				FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects.erase(position);*/	
		}
	}
	FIXS_CCH_SoftwareMap::getInstance()->immFaultySwObjects.clear();

}

void FIXS_CCH_UpgradeManager::customizeFirmwareType(CCH_Util::fwRemoteData &info, int activeArea)
{
	const int NO_OF_CUSTOMIZE_TYPE_NAME = 3;
	bool isAreaA = false;
	int count = 0;
	std::cout << __FUNCTION__ << "@" << __LINE__ << "active area value is : " << activeArea << std::endl;
	if(activeArea == 0 )
	{
		isAreaA = true;
	}
	for(int j=0;j<MAX_NO_OF_VERSION_INDEX;j++)
	{
		if((info.fwType[j] != "") || (info.fwVersion[j] != ""))
		{
			CCH_Util::stringToUpper (info.fwType[j]);
			CCH_Util::stringToUpper (info.fwVersion[j]);

			if((info.fwType[j]).find("CXP_TYPE") != std::string::npos)
			{ 
				info.fwType[j] = (isAreaA) ? ("CXP_TYPE_A"): ("CXP_TYPE_B");
				count++;
			}
			else if((((info.fwType[j]).compare("IPMI_PARAMETERS_SENSOR_A") == 0 )) || ((info.fwType[j]).compare("IPMI_PARAMETERS_SENSOR_B") == 0 ))
			{
				info.fwType[j] = (isAreaA) ? ("IPMI_PARAMETERS_SENSOR_A") : ("IPMI_PARAMETERS_SENSOR_B");
				count++;
			}
			else if((((info.fwType[j]).compare("IPMI_PARAMETERS_PMB_A") == 0 )) || ((info.fwType[j]).compare("IPMI_PARAMETERS_PMB_B") == 0 ))
			{
				info.fwType[j] = (isAreaA) ? ("IPMI_PARAMETERS_PMB_A") : ("IPMI_PARAMETERS_PMB_B");
				count++;
			}
		}
		if(count == NO_OF_CUSTOMIZE_TYPE_NAME)
			break;
	}

}

int FIXS_CCH_UpgradeManager::getActiveFwAreaFromBoard (unsigned long umagazine, std::string slot,int &value) // value= 0 (A),1(B)
{
        std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;

        bool result = true;
        int masterSlot = -1;
        std::string slot0IpA(""), slot25IpA(""), slot0IpB(""), slot25IpB(""),masterIp("");
        fetchEgem2L2SwitchIP(umagazine, 0, slot0IpA, slot0IpB);
        fetchEgem2L2SwitchIP(umagazine, 25, slot25IpA, slot25IpB);
        FIXS_CCH_SNMPManager* snmpReader = new (std::nothrow) FIXS_CCH_SNMPManager(slot0IpA);
        int snmpRes = snmpReader->isMaster(slot0IpA);

        if(snmpRes == 1) //master
        {
                masterSlot = 0;
        }
        else if(snmpRes == 2) //passive
        {
                masterSlot = 25;
        }
        else
        {
                snmpRes = snmpReader->isMaster(slot25IpB);
                if(snmpRes == 1) //master
                {
                        masterSlot = 25;
                }
 		else if(snmpRes == 2) //passive
                {
                        masterSlot = 0;
                }
                else
                        result = false;

        }

        bool res=true;
        if(result)
        {
                if(masterSlot == 0)
                {
                        snmpReader->setIpAddress(slot0IpA);
                        res = snmpReader->getActiveArea(slot,value);
                        if(!res)
                        {
                                snmpReader->setIpAddress(slot0IpB);
                                res = snmpReader->getActiveArea(slot,value);
                        }
                }
                else if (masterSlot == 25)
                {
                        snmpReader->setIpAddress(slot25IpA);
                        res = snmpReader->getActiveArea(slot,value);
                        if(!res)
                        {
                                snmpReader->setIpAddress(slot25IpB);
                                res = snmpReader->getActiveArea(slot,value);
                        }
                }
  	}
        else
        {

                if(snmpReader != NULL)
                {
                        delete(snmpReader);
                        snmpReader = NULL;
                }
                return SNMP_FAILURE;
        }

        if(snmpReader != NULL)
        {
                delete(snmpReader);
                snmpReader = NULL;
        }

        if(res == true)
                return 0;
        else
                return SNMP_FAILURE;
}


