
//## begin module%4E3A4E1602A1.includes preserve=yes
#include "FIXS_CMXH_Manager.h"
#include "FIXS_CMXH_Util.h"
#include <iostream>
#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"
#include "ACS_TRA_trace.h"
#include "FIXS_CMXH_Logger.h"
//## end module%4E3A4E1602A1.includes

//## begin module%4E3A4E1602A1.additionalDeclarations preserve=yes
using namespace CMXH_Util;

ACE_Recursive_Thread_Mutex FIXS_CMXH_Manager::m_lock;          // TR_HX98360

FIXS_CMXH_Manager* FIXS_CMXH_Manager::m_instance = 0;


FIXS_CMXH_Manager::FIXS_CMXH_Manager()
: m_snmpManager(0),m_lastAssignedHandleId(-1)
{
	cmxVector.clear();
	for(int i=0; i < MAX_NO_OF_CMXB ; i++)
		snmpHandler[i] = NULL;

	_trace = new ACS_TRA_trace("CMX Manager");
	FIXS_CMXH_logging = FIXS_CMXH_Logger::getLogInstance();

}


FIXS_CMXH_Manager::~FIXS_CMXH_Manager()
{
	if (_trace){
		delete(_trace);
		_trace = NULL;
	}

	for(int i=0; i < MAX_NO_OF_CMXB ; i++)
	{
		if(snmpHandler[i] && (snmpHandler[i]->isRunningThread() == true))
		{
			snmpHandler[i]->stop();
			sleep(1);
		}
		if(snmpHandler[i])
			delete snmpHandler[i];
	}
	FIXS_CMXH_logging = 0;
}



//## Other Operations (implementation)
FIXS_CMXH_Manager * FIXS_CMXH_Manager::getInstance ()
{
	//Lock lock;
	if (m_instance == 0)
	{
		m_instance = new FIXS_CMXH_Manager();
	}
	return m_instance;
}

void FIXS_CMXH_Manager::finalize ()
{
	delete(m_instance);
	m_instance = NULL;
}


int FIXS_CMXH_Manager::initCMXManager ()
{
	std::cout << __FUNCTION__ << "@" << __LINE__ << " Begin" << std::endl;
	int res = 0;
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_Manager] Initializing CMXH Manager...",LOG_LEVEL_DEBUG);
	std::cout << __FUNCTION__ << "@" << __LINE__ << " calling <<< getAllCMXBoardAddress >>>" << std::endl;
	res = getAllCMXBoardAddress(cmxVector);
	if(res == 0)
	{
		int size = (int)cmxVector.size();
		if(size > 0)
		{
			for(int i=0;i<size;i++)
			{
				snmpHandler[i] = new FIXS_CMXH_SNMPHandler();
				std::cout<<"\n cmx vector size= " <<size<<" handler id= " <<i<<endl;
				snmpHandler[i]->open();
				cmxVector[i].index = i;
				m_handleIdSet.insert(i);
				m_lastAssignedHandleId = i;
				snmpHandler[i]->setCMXBAddress(cmxVector[i].IP_EthA,cmxVector[i].IP_EthB);
				snmpHandler[i]->setCMXSwVersion(cmxVector[i].sw_version);
				snmpHandler[i]->openCMXBPorts();
				snmpHandler[i]->configureBackPortsAsEdgePort();
				snmpHandler[i]->handleRSTP();
				snmpHandler[i]->performVlanSettings();
			}
		}
	}

	return res;
	//## end FIXS_CMXH_Manager::initCMXManager%4E40CF850227.body
	std::cout << __FUNCTION__ << "@" << __LINE__ << " END " << std::endl;
}

void FIXS_CMXH_Manager::update (const ACS_CS_API_HWCTableChange& observer)
{
  //## begin FIXS_CMXH_Manager::update
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_Manager] update",LOG_LEVEL_DEBUG);
	std::cout<< "observer.dataSize = " << observer.dataSize << std::endl;
	//Lock lock;

        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);  // TR_HX98360

	//	unsigned short boardId = 0;
	//	unsigned long magazine = 0;
	//	unsigned short slot = 0;

	if (observer.dataSize > 0 )
	{
		switch(observer.hwcData[0].operationType)
		{
		case ACS_CS_API_TableChangeOperation::Add :
			std::cout << "DBG: ------- Add -------- \n" << std::endl;
			std::cout << "FBN: " << observer.hwcData[0].fbn << std::endl;
			if (observer.hwcData[0].fbn == 380) //later replace 380 with CMXB FBN with IMM_Util
			{
				int index = createHandleId();

				std::cout << "\n CMX new handle ID= " << index << endl;
				std::string ipA_String = CMXH_Util::ulongToStringIP(observer.hwcData[0].ipEthA);
				std::string ipB_String = CMXH_Util::ulongToStringIP(observer.hwcData[0].ipEthB);
				std::cout << "\n CMX new IPA= " << ipA_String << endl;
				std::cout << "\n CMX new IPB= " << ipB_String << endl;
				int sw_version=0;
				int retValue = getNewboardSwVersion (ipA_String,ipB_String,sw_version);
				std::cout << "\n sw_version for new board = "<< sw_version <<std::endl;
				if(retValue == 0)
				{
					CMXIPAddress tempVector;
					tempVector.IP_EthA = ipA_String;
					tempVector.IP_EthB = ipB_String;
					tempVector.index = index;
					tempVector.sw_version = sw_version;
					std::cout << "\n sw_version for new board from tempvector = "<< tempVector.sw_version <<std::endl;

					cmxVector.push_back(tempVector);
					m_handleIdSet.insert(index);
					if(!snmpHandler[index])
						snmpHandler[index] = new FIXS_CMXH_SNMPHandler();
					std::cout << "\n CMX handler start id= " << index << endl;
					snmpHandler[index]->open();
					snmpHandler[index]->setCMXBAddress(tempVector.IP_EthA ,tempVector.IP_EthB);
					snmpHandler[index]->setCMXSwVersion(tempVector.sw_version);
					snmpHandler[index]->openCMXBPorts();
					snmpHandler[index]->configureBackPortsAsEdgePort();
					snmpHandler[index]->handleRSTP();
					snmpHandler[index]->performVlanSettings();
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("New CMX added");
				}
				else
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Board update failed.. Unable to retrieve software version: %d", __LINE__, observer.hwcData[0].operationType);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr);
				}
			}

			break;

		case ACS_CS_API_TableChangeOperation::Delete : 
			std::cout << "-------- Delete --------\n" << std::endl;
			std::cout << "FBN: " << observer.hwcData[0].fbn << std::endl;
			if (observer.hwcData[0].fbn == 380)
				//later replace 380 with CMXB FBN with IMM_Util
			{
				std::string ipA_String = CMXH_Util::ulongToStringIP(observer.hwcData[0].ipEthA);
				for(int i= 0; i < (int) cmxVector.size();i++)
				{
					if(cmxVector[i].IP_EthA == ipA_String )
					{
						int j = cmxVector[i].index;
						snmpHandler[j]->stop();
						cmxVector.erase(cmxVector.begin()+i);
						m_handleIdSet.erase(j);
						//delete snmphandler for the deleted board when it is stopped
						for (int i = 0; i < 12; i++)
						{
							if (snmpHandler[j]->isRunningThread() == false)
							{
								delete snmpHandler[j];
								snmpHandler[j] = NULL;
								break;
							}
							else 
							{
								sleep(1);
							}
						}
						std::cout<<"\n CMX handler stop id= " <<j<<" vector size = "<<cmxVector.size()<<endl;
 
						break;
					}
				}
				if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(" CMX deleted");

			}
			break;

		default:
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] HWCTable update has been discarded. Operation type: %d", __LINE__, observer.hwcData[0].operationType);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr);
		}
		std::cout << "unknown!!)" << "\n";
		}
	}
	//## end FIXS_CMXH_Manager::update%4E5CB36A0132.body
}

int FIXS_CMXH_Manager::getNewboardSwVersion(std::string ipA_String,std::string ipB_String,int &sw_version)
{
	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();

	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(380);
			// later replace 380 with FBN CMXB
			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no CMXB board found.it will never reach this place
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No CMXB board found", __LINE__);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					retValue = -1;	//INTERNALERR
				}
				else
				{
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						uint32_t IP_EthA = 0;
						returnValue = hwc->getIPEthA (IP_EthA, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							std::string C_IP_EthA = CMXH_Util::ulongToStringIP(IP_EthA);
							std::cout << "\Eth_nipA_String = "<< C_IP_EthA <<std::endl;

							if (C_IP_EthA.compare(ipA_String) == 0)
							{
								uint32_t IP_EthB = 0;
								returnValue = hwc->getIPEthB (IP_EthB, boardId);
								if (returnValue == ACS_CS_API_NS::Result_Success)
								{
									std::string C_IP_EthB = CMXH_Util::ulongToStringIP(IP_EthB);
									std::cout << "\nEth_ipB_String = "<< C_IP_EthB <<std::endl;
									if (C_IP_EthB.compare(ipB_String) == 0)
									{
										uint16_t version;
										ACS_CS_API_NS::CS_API_Result returnValue_sw = hwc->getSoftwareVersionType(version,boardId);
										if (returnValue_sw == ACS_CS_API_NS::Result_Success)
										{
											sw_version = version;
											std::cout << "\n sw_version = "<< sw_version <<std::endl;
											retValue = 0;

										}
										else
										{
											char tmpStr[512] = {0};
											snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No software version found for cmxb", __LINE__);
											if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
											if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
											retValue = -1;//NO_REPLY_FROM_CS
											break;
										}
									}
								}
								else
								{
									//serious fault in CS: No IP_EthB found for the CMXB board
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the CMXB board", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
									if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
									retValue = -1;//NO_REPLY_FROM_CS
									break;
								}

							}

						}
						else
						{
							//serious fault in CS: No IP_EthA found for the CMXB board
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the CMXB board", __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
							retValue = -1; //NO_REPLY_FROM_CS
							break;
						}
					}
				}
			}
			else
			{
				//trace - GetBoardIds failed, error from CS
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				std::cout << "DBG: " << tmpStr << std::endl;
				retValue = -1;	//NO_REPLY_FROM_CS
			}
			if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS....", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			retValue = -1;
		}
	}
	else
	{

		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS.. Unable to create HWC instance....", __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		retValue = -1;

	}
	return retValue;
}

int FIXS_CMXH_Manager::createHandleId ()
{
	//## begin FIXS_CMXH_Manager::createHandleId%4E7ACEA40330.body preserve=yes
	// Assign handle identifier
	int handleId = m_lastAssignedHandleId + 1;	// Add 1 to the last assigned number

	// Check if number is unused and is within bounds
	if ( (m_lastAssignedHandleId >= (MAX_NO_OF_CMXB-1)) || (m_handleIdSet.find(handleId) != m_handleIdSet.end()) )
	{
		// Board id exist, we must search for one instead
		handleId = -1;

		// Loop through all assignment to find an unused one
		for (unsigned short tempId = 0; tempId < MAX_NO_OF_CMXB; tempId++)
		{
			if (m_handleIdSet.find(tempId) == m_handleIdSet.end())	// Boardid does not exist
			{
				handleId = tempId;								// Use this number
				break;
			}
		}
	}

	if (handleId >= 0)					// If handleid . 0, then an no unused number was found
	{
		// Save assignments in sets
		m_handleIdSet.insert(handleId);

		// Record the last assignments
		m_lastAssignedHandleId = handleId;
	}

	return handleId;
	//## end FIXS_CMXH_Manager::createHandleId%4E7ACEA40330.body
}

int FIXS_CMXH_Manager::getAllCMXBoardAddress (std::vector<CMXIPAddress> &cmxVector)
{
	//## begin FIXS_CMXH_Manager::getAllCMXBoardAddress
	int retValue = 0;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setFBN(380);
			// later replace 380 with FBN CMXB
			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no CMXB board found.it will never reach this place
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No CMXB board found", __LINE__);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);					
					retValue = 0;	//INTERNALERR
				}
				else
				{
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						//	unsigned long IP_EthA = 0;
						uint32_t IP_EthA = 0;
						returnValue = hwc->getIPEthA (IP_EthA, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							//	unsigned long IP_EthB = 0;
							uint32_t IP_EthB = 0;
							uint16_t version;
							returnValue = hwc->getIPEthB (IP_EthB, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)  
							{ 
								ACS_CS_API_NS::CS_API_Result returnValue_sw = hwc->getSoftwareVersionType(version,boardId);
								if (returnValue_sw == ACS_CS_API_NS::Result_Success)
								{
									ACS_CS_API_NS::CS_API_Result returnValue_sw = hwc->getSoftwareVersionType(version,boardId);
									if (returnValue_sw == ACS_CS_API_NS::Result_Success)
									{
										CMXIPAddress cmx;
										std::string ipA_String = CMXH_Util::ulongToStringIP(IP_EthA);
										std::string ipB_String = CMXH_Util::ulongToStringIP(IP_EthB);
										std::cout << "\nipA_String = "<< ipA_String.c_str() <<std::endl;
										std::cout << "\nipB_String = "<< ipB_String.c_str() <<std::endl;
										std::cout << "\n Version = " << version << std::endl;
										cmx.IP_EthA = ipA_String;
										cmx.IP_EthB = ipB_String;
										cmx.sw_version = version;
										cmxVector.push_back(cmx);
									}
									else
									{
										char tmpStr[512] = {0};
										snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No software version found for cmxb", __LINE__);
										if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
										if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
										retValue = -1;//NO_REPLY_FROM_CS
										break;
									}
								}
								else
								{
									char tmpStr[512] = {0};
									snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No software version found for cmxb", __LINE__);
									if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
									if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
									retValue = -1;//NO_REPLY_FROM_CS
									break;
								}
							}
							else
							{
								//serious fault in CS: No IP_EthB found for the CMXB board
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthB found for the CMXB board", __LINE__);
								if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
								if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
								retValue = -1;//NO_REPLY_FROM_CS
								break;
							}
						}
						else
						{
							//serious fault in CS: No IP_EthA found for the CMXB board
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the CMXB board", __LINE__);
							if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
							if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
							retValue = -1; //NO_REPLY_FROM_CS
							break;
						}
					}
				}
			}
			else
			{
				//trace - GetBoardIds failed, error from CS
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				std::cout << "DBG: " << tmpStr << std::endl;
				retValue = -1;	//NO_REPLY_FROM_CS
			}
			if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS....", __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			retValue = -1;
		}
		if (hwc) ACS_CS_API::deleteHWCInstance(hwc); 
	}
	return retValue;
	//## end FIXS_CMXH_Manager::getAllCMXBoardAddress
}

bool FIXS_CMXH_Manager::isCMXB (int magazine, int slot, std::string ip)
{
	std::vector<CMXIPAddress> cmxbTempVector;
	int res = 0;
	bool retValue=false;
	UNUSED(magazine);
	UNUSED(slot);

	res = getAllCMXBoardAddress(cmxbTempVector);
	if(res == 0)
	{
		for(int i = 0; i < (int)cmxbTempVector.size(); i++)
		{
			if (cmxbTempVector[i].IP_EthA.compare(ip) == 0 || cmxbTempVector[i].IP_EthB.compare(ip) == 0)
			{
				std::cout<<"DBG: isCMX() - true"<<std::endl;
				retValue = true;
				break;
			}
			else
			{
				std::cout<<"DBG: isCMX() - NOT CMX"<<std::endl;
			}
		}
	}
	cmxbTempVector.clear();

	return retValue;
}

void FIXS_CMXH_Manager::retrySettingAtCMXStart (std::string ip)
{
	//## begin FIXS_CMXH_Manager::retrySettingAtCMXStart%4E9681980041.body preserve=yes
	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_Manager] retrySettingAtCMXStart !!! (ip:%s)",ip.c_str()); // TR_HX74057
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN); //TR_HX74057

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": IN" <<std::endl;
	for(int i= 0; i < (int) cmxVector.size();i++)
	{
		if((cmxVector[i].IP_EthA == ip) || (cmxVector[i].IP_EthB == ip))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << ": Found entry in CMX vector" <<std::endl;
			int j = cmxVector[i].index;
			if(snmpHandler[j])
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << ": Setting retry for CMX" <<std::endl;
				snmpHandler[j]->setCMXBAddress(cmxVector[i].IP_EthA,cmxVector[i].IP_EthB);
				int sw_version;
				int retValue = getNewboardSwVersion (cmxVector[i].IP_EthA, cmxVector[i].IP_EthB,sw_version);
			if(retValue == 0)
				{
					snmpHandler[j]->setCMXSwVersion(sw_version);
					std::cout << "DBG: " << __FUNCTION__ << "@" << ": Software Version" << sw_version <<std::endl;
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_CMXH_Manager] Software version(%d) : ip(%s) ", sw_version, ip.c_str());  //TR_HX7405
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN); //TR_HX74057
					snmpHandler[j]->setTrap();
				}
				else
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Unable to retrieve software version on board: %u", __FUNCTION__,__LINE__, retValue);
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
					if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write(tmpStr,LOG_LEVEL_WARN); //TR_HX74057
					std::cout << "DBG: " << tmpStr << std::endl;
				}
			}
			break;
		}
	}
	//## end FIXS_CMXH_Manager::retrySettingAtCMXStart%4E9681980041.body
}

void FIXS_CMXH_Manager::HandleRSTPforCMX4(std::string ip)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_Manager] HandleRSTPforCMX4");

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": IN" <<std::endl;
	for(int i= 0; i < (int) cmxVector.size();i++)
	{
		if((cmxVector[i].IP_EthA == ip) || (cmxVector[i].IP_EthB == ip))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << ": Found entry in CMX vector" <<std::endl;
			int j = cmxVector[i].index;
			if(snmpHandler[j])
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << ": RSTP handling for CMX 4.0" <<std::endl;
				snmpHandler[j]->setCMXBAddress(cmxVector[i].IP_EthA,cmxVector[i].IP_EthB);
				snmpHandler[j]->handleRSTP();	
			}
			break;
		}
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": OUT" <<std::endl;
}

void FIXS_CMXH_Manager::deleteLogsInCMX (std::string &ip)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_Manager] deleteLogsInCMX");
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": IN" <<std::endl;
	for(int i= 0; i < (int) cmxVector.size();i++)
	{
		if((cmxVector[i].IP_EthA == ip) || (cmxVector[i].IP_EthB == ip))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << ": Found entry in CMX vector" <<std::endl;
			int j = cmxVector[i].index;
			if(snmpHandler[j])
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << ": About to call deleteOldLogsSNMP()" <<std::endl;
				std::cout << "DBG: " << __FUNCTION__ << "@" << "....IP_EthA = " << cmxVector[i].IP_EthA.c_str() << std::endl;
				snmpHandler[j]->setCMXBAddress(cmxVector[i].IP_EthA,cmxVector[i].IP_EthB);
				snmpHandler[j]->deleteOldLogsSNMP();
			}
			break;
		}
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ": OUT" <<std::endl;
}

int FIXS_CMXH_Manager::getLogsFromCMX (string &ipNumber)
{
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("[FIXS_CMXH_Manager] getLogsFromCMX");
	std::cout << "\n getLogsFromCMX call for IP = " << ipNumber.c_str() <<  std::endl;
//	size_t oid_name_size = 0;
//	unsigned timeout_ms = 2000;

	std::string tftpRoot = "/data/apz/data";
	std::string tftpPathCMX = "/boot/cmx/cmx_logs/cmx_";
	std::string clusterIP = "192.168.169.";
	std::string tmp(ipNumber);
	std::cout << "tmp " << tmp.c_str() << std::endl;
	int nodeNum = CMXH_Util::getNode();
	if (clusterIP.compare(tmp.substr(0,12)) == 0 ) {
		char c[16];
		sprintf(c, "%d", nodeNum);
		clusterIP += std::string(c);
	} else {
		clusterIP = "192.168.170.";
		char c[16];
		sprintf(c, "%d", nodeNum);
		clusterIP += std::string(c);
	}
	std::cout << "\n DBG: clusterIP = " << clusterIP.c_str() << std::endl;
	int createRes;
	createRes = CMXH_Util::createDir(tftpRoot + "/boot/cmx");
	std::cout << "\n DBG: createRes = " << createRes << std::endl;

	createRes = CMXH_Util::createDir(tftpRoot + "/boot/cmx/cmx_logs");
	std::cout << "\n DBG: createRes = " << createRes << std::endl;
	
	//replace . with _
	//-- Remove this code if not necessary -- BEGIN
	std::string ipFiltered(ipNumber);
	if (ipFiltered.compare("")!=0){
		size_t foundDot;
		foundDot = ipFiltered.find_first_of(".");
		while (foundDot != string::npos){
			ipFiltered[foundDot] = '_';
			foundDot = ipFiltered.find_first_of(".",foundDot+1 );
		}
	}
	//-- Remove this code if not necessary -- END
	std::cout << "\n DBG: ipFiltered: " << ipFiltered.c_str() << std::endl;
	std::string logPath("");
	logPath = tftpRoot + tftpPathCMX + ipFiltered;
//	sprintf (logPath, "%s%s%s",tftpRoot.c_str(), tftpPathCMX.c_str(),ipFiltered.c_str());

	std::cout << "logPath = " << logPath.c_str()<< std::endl;

	createRes = CMXH_Util::createDir(logPath);
	std::cout << "\n DBG: createRes = " << createRes << std::endl;

	std::string snmplogPath("");
	snmplogPath = tftpPathCMX + ipFiltered;
//	sprintf (snmplogPath, "%s%s", tftpPathCMX.c_str(),ipFiltered.c_str());
	std::cout << "\n DBG: snmplogPath = " << snmplogPath.c_str() << std::endl;

	bool call_result = true;
	FIXS_CMXH_SNMPManager* manager = new FIXS_CMXH_SNMPManager(ipNumber);
	
	do 
	{
		call_result = manager->setContTransferSrvIP(clusterIP);
		if(!call_result)
		{
			std::cout << "\n DBG: setContTransferSrv SNMP set request failed";
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("setContTransferSrvIP SNMP set request failed",LOG_LEVEL_ERROR);
			break;
		}
	
		call_result = true;
		//oidContTransferSrvPath
		call_result = manager->setContTransferSrvPath(snmplogPath);
		if(!call_result)
		{
			std::cout << "\n DBG: setContTransferSrvPath SNMP set request failed";
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("setContTransferSrvPath SNMP set request failed",LOG_LEVEL_ERROR);
			break;
		}

		call_result = true;
		int intValue = 1;
		//oidContNotificationsCtrl
		call_result = manager->setContNotificationsCtrl(intValue);
		if(!call_result)
		{
			std::cout << "\n DBG: setContNotificationsCtrl SNMP set request failed";
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("setContNotificationsCtrl SNMP set request failed",LOG_LEVEL_ERROR);
			break;
		}

		call_result = true;
		//oidContAutomaticTransfer
		call_result = manager->setContAutomaticTransfer(intValue);
		if(!call_result)
		{
			std::cout << "\n DBG: setContAutomaticTransferCtrl SNMP set request failed";
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("setContAutomaticTransferCtrl SNMP set request failed",LOG_LEVEL_ERROR);
			break;
		}

		call_result = true;
		intValue = 0;
		//oidContFileEncryption
		call_result = manager->setContFileEncryption(intValue);
		if(!call_result)
		{
			std::cout << "\n DBG: setContFileEncryption SNMP set request failed";
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("setContFileEncryption SNMP set request failed",LOG_LEVEL_ERROR);
			break;
		}

		call_result = true;
		intValue = 1;
                //oidCreateContFile
                call_result = manager->setCreateContFile(intValue);
                if(!call_result)
                {
                        std::cout << "\n DBG: setCreateContFile SNMP set request failed";
						if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("setCreateContFile SNMP set request failed",LOG_LEVEL_ERROR);
                        break;
                }

	}while(0);

	if(manager) delete manager;
        return 0;
}

//}
//	size_t oid_name_size = 0;
//	unsigned timeout_ms = 2000;
	
//	sprintf (logPath, "%s%s%s",tftpRoot.c_str(), tftpPathCMX.c_str(),ipFiltered.c_str());
//	sprintf (snmplogPath, "%s%s", tftpPathCMX.c_str(),ipFiltered.c_str());
	
//## end FIXS_CMXH_Manager%4E3A4E1602A1.declarations

//## begin module%4E3A4E1602A1.epilog preserve=yes
//## end module%4E3A4E1602A1.epilog
