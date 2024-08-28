
#include "FIXS_TREFC_Util.h"
#include "FIXS_TREFC_ImmInterface.h"

using namespace std;
using namespace TREFC_Util;

namespace TREFC_Util
{
	static EnvironmentType Environment = UNKNOWN;
}

//ACS_TRA_trace traceUtil = ACS_TRA_DEF("utility", "C512");

unsigned short TREFC_Util::numOfChar(istringstream &istr, char searchChar)
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
   (void)istr.seekg(0);    //      of the stream
   return numChar;
}


bool TREFC_Util::checkFigures(istringstream &istr)
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


bool TREFC_Util::dottedDecToLong(istringstream &istr, unsigned long &value,
                              unsigned long lowerLimit1, unsigned long upperLimit1,
                              unsigned long lowerLimit2, unsigned long upperLimit2,
                              unsigned long lowerLimit3, unsigned long upperLimit3,
                              unsigned long lowerLimit4, unsigned long upperLimit4)
{
   istr.clear();     // return to the beginning
   (void)istr.seekg(0);    //      of the stream
   char c;
   string strAddr = "";
   unsigned short dotNum = 0;
   value = 0;

   do {
      c = istr.get();
      if (c == '.' || c == EOF)
      {
         dotNum++;
         istringstream converter(strAddr);

         if (! checkFigures(converter))   //checks that only figures (0-9) are used in the option arg.
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


bool TREFC_Util::checkIPAddresses(const char* arg, unsigned long &ipAddress)
{
      istringstream istrTemp(arg);

      if (numOfChar(istrTemp,'.') != 3)   // If not 3 dots, faulty format
         return false;

      if (! dottedDecToLong(istrTemp, ipAddress, 0, 255, 0, 255, 0, 255, 0, 255))
            return false;

   return true;
}


bool TREFC_Util::checkReferenceName(string &s)
{
	string temp = s;
	// check the length of string & 1st character is alphabet or not
	if(temp.length()>8 || (!isalpha(temp[0])) )
		return false;

	//check alphanumeric or not
	for(unsigned int i = 0; i < temp.length(); i++)
	{
		if(!isalnum(temp[i]))
			return false;

		temp[i] = toupper(s[i]);
	}
	s = temp;
	return true;
}


bool TREFC_Util::checkIfActiveNode()
{
   bool active = false;
//
//   HCLUSTER     clusterHandle = 0;
//   HGROUP groupHandle = 0;
//   const wchar_t CLUSTER_GROUP_NAME[] = L"Cluster Group";
//   DWORD size = 300;
//   wchar_t ownerNode[300];
//
//   // Open handle to cluster
//   clusterHandle = OpenCluster(NULL);
//
//   if (clusterHandle)
//   {
//      // Open cluster group
//      groupHandle = OpenClusterGroup( clusterHandle, (LPWSTR)CLUSTER_GROUP_NAME );
//
//      if (groupHandle != 0)
//      {
//         // Get group state
//         CLUSTER_GROUP_STATE groupState = GetClusterGroupState(groupHandle, ownerNode, &size);
//
//         // Check if group was found
//         if (groupState != ClusterGroupStateUnknown)
//         {
//            DWORD       namelen = 0;
//            LPWSTR nodeName = 0;
//
//            // Get name length for the computer's NETBIOS name
//            (void) GetComputerNameW(nodeName, &namelen);
//
//            if (namelen > 0)
//            {
//               nodeName = new WCHAR[namelen];
//
//               if (nodeName)
//               {
//                  // Get NETBIOS name for this computer
//                  (void) GetComputerNameW(nodeName, &namelen);
//
//                  if (wcsicmp( nodeName, ownerNode ) == 0)
//                  {
//                     // Node is active
//                     active = true;
//                  }
//                  else
//                  {
//                     // Node is passive
//                     active = false;
//                  }
//
//                  delete [] nodeName;
//               }
//            }
//         }
//
//         (void) CloseClusterGroup(groupHandle);
//      }
//
//      (void) CloseCluster(clusterHandle);
//   }
//   // Cluster not available on AP-DE. Servicetype always master on AP1
//   // for debug compilation
//#ifdef _DEBUG
//   active = true;
//#endif

   return active;
}


void TREFC_Util::sendAlarm (const int problem, const unsigned int severity, const std::string &cause, const std::string &data, const std::string &text)
{
	acs_aeh_evreport EvReporter;

	std::string *processName;
	processName = new string();

//	int pid = _getpid();
	int pid = getpid();
	ostringstream id;
	id << pid;

	processName->append(FIXS_TREFC_PROCESS_NAME);
	processName->append(":");
	processName->append(id.str());

//	if (_trace->ACS_TRA_ON())
//	{
//		char tmpStr[512] = {0};
//		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] problem = %d, cause = %s, data = %s, text = %s\n",__FUNCTION__, problem, cause.c_str(), data.c_str(), text.c_str());
//		_trace->ACS_TRA_event(1, tmpStr);
//	}

	string severityString("");

	switch(severity)
   	{
		case Severity_A1:
			severityString = "A1";
			break;
		case Severity_A2:
			severityString = "A2";
			break;
		case Severity_CEASING:
			severityString = "CEASING";
				break;
		case Severity_O1:
			severityString = "O1";
			break;
		case Severity_Event:
			severityString = "EVENT";
			break;
		default:
			delete(processName);
         	return;
   	}

    const bool manualCeaseAllowed = false;
    ACS_AEH_ReturnType rt = EvReporter.sendEventMessage(processName->c_str(),
                                                              problem,
                                                              severityString.c_str(),
                                                              cause.c_str(),
                                                              "APZ",
															  FIXS_TREFC_PROCESS_NAME,
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
//				if (ACS_TRA_ON(traceUtil))
//				{
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Event syntax error\n", __FUNCTION__);
//					ACS_TRA_event(&traceUtil, tmpStr);
//				}
			break;

	        case ACS_AEH_eventDeliveryFailure:
//				if (ACS_TRA_ON(traceUtil))
//				{
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Event delivery failure\n", __FUNCTION__);
//					ACS_TRA_event(&traceUtil, tmpStr);
//				}
        	break;

   	        default:
//				if (ACS_TRA_ON(traceUtil))
//				{
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Unknown event failure\n", __FUNCTION__);
//					ACS_TRA_event(&traceUtil, tmpStr);
//				}
        	break;
		}
	}

	if (processName) {
		delete processName;
		processName = NULL;
	}

}


std::string TREFC_Util::ulongToStringIP (unsigned long ipaddress)
{
	char tempValueStr[16] = {0};
	uint32_t tempValue = ipaddress; tempValue = ipaddress;
	snprintf(tempValueStr, sizeof(tempValueStr), "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    std::string IP = tempValueStr;
	return IP;
}

bool TREFC_Util::isSCX(int /*magazine*/, int /*slot*/, std::string /*ip*/)
{
//	int retValue = 0;
//	std::cout<<"DBG: isSCX() - Magazine : "<<magazine<<std::endl;
//	std::cout<<"DBG: isSCX() - Slot : "<<slot<<std::endl;
//	std::cout<<"DBG: isSCX() - IP : "<<ip<<std::endl;
//	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
//	if (hwc)
//	{
//		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
//		if (boardSearch)
//		{
//			ACS_CS_API_IdList boardList;
//			boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SCXB);
//		//	boardSearch->setSlot(slot);
//			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
//			if (returnValue == ACS_CS_API_NS::Result_Success)
//			{
//				if (boardList.size() == 0)
//				{
//					// trace: no APUB board found.it will never reach this place
//					std::cout<<"DBG: isSCX() - NO BOARD FOUND"<<std::endl;
//					if (ACS_TRA_ON(traceUtil))
//					{
//						char tmpStr[512] = {0};
//						_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No board found", __LINE__);
//						ACS_TRA_event(&traceUtil, tmpStr);
//					}
//					retValue = 1;
//				}
//				else
//				{
//					for(int i = 0; i < (int)boardList.size(); i++)
//					{
//							BoardID boardId = boardList[i];
//							std::string tempIP("");
//							unsigned long ipNum = 0;
//							//unsigned short fbn = 0;
//							retValue = hwc->getIPEthA(ipNum, boardId);
//							if (retValue == ACS_CS_API_NS::Result_Success)
//							{
//								tempIP = TREFC_Util::ulongToStringIP(ipNum);
//								std::cout<<"IpA string : "<<tempIP<<std::endl;
//								if (tempIP.compare(ip) == 0)
//								{
//									std::cout<<"DBG: isSCX() - SCX"<<std::endl;
//									//if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
//									retValue = 0;
//									break;
//								}
//								else
//								{
//									tempIP.clear();
//									ipNum = 0;
//									retValue = hwc->getIPEthB(ipNum, boardId);
//									if (retValue == ACS_CS_API_NS::Result_Success)
//									{
//										tempIP = TREFC_Util::ulongToStringIP(ipNum);
//										std::cout<<"IpB string : "<<tempIP<<std::endl;
//										if (tempIP.compare(ip) == 0)
//										{
//											std::cout<<"DBG: isSCX() - SCX"<<std::endl;
//											//if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
//											retValue = 0;
//											break;
//										}
//										else
//										{
//											std::cout<<"DBG: isSCX() - NOT SCX"<<std::endl;
//											retValue = 1;
//										}
//									}
//									else
//									{
//										std::cout<<"DBG: isSCX() - NOT SCX"<<std::endl;
//										retValue = 1;
//									}
//								}
//							}
//							else
//							{
//								std::cout<<"DBG: isSCX() - COULD NOT GET IP"<<std::endl;
//								retValue = 1;
//							}
//					}//for
//				}
//			}
//			else
//			{
//				if (ACS_TRA_ON(traceUtil))
//				{//trace - GetBoardIds failed, error from CS
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
//					if (ACS_TRA_ON(traceUtil)) ACS_TRA_event(&traceUtil, tmpStr);
//					std::cout << "DBG: " << tmpStr << std::endl;
//				}
//				std::cout<<"DBG: isSCX() - GETBOARDIDS FAILED"<<std::endl;
//				retValue = 1;
//			}
//		}
//		if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
//	}
//
//	if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
//	if(retValue == 0)
//		return true;
//	else
		return false;

}



void TREFC_Util::getHexTime(char* byte, unsigned int& size)
{
	/*time_t rawtime;
	struct tm * st;

	time ( &rawtime );

	st = gmtime ( &rawtime );

	unsigned short shortYear = st->tm_year + 1900;//.wYear;
	cout<<"tm_year: "<< st->tm_year<<"  yr to set: "<<shortYear<<endl;
	shortYear = htons(shortYear);

	int i = 0;
	memcpy(byte, &shortYear, sizeof(unsigned short));
	i += sizeof(unsigned short);

	int month = st->tm_mon + 1;
	byte[i++] = (char)month;

	byte[i++] = (char)st->tm_mday;
	byte[i++] = (char)st->tm_hour;
	byte[i++] = (char)st->tm_min;
	byte[i++] = (char)st->tm_sec;
	byte[i++] = 0;
	byte[i++] = 0;
	byte[i++] = 0;
	byte[i++] = 0;

	size = i;
*/

	time_t rawtime;
	struct tm * st;

	time ( &rawtime );

	st = gmtime ( &rawtime );

	unsigned short shortYear = st->tm_year + 1900;//.wYear;

	int month = st->tm_mon + 1;

	sprintf(byte, "%04X%02X%02X%02X%02X%02X00000000", shortYear, (char)month, (char)st->tm_mday, (char)st->tm_hour, (char)st->tm_min, (char)st->tm_sec);

	size=strlen(byte);
}


void TREFC_Util::sendEvent (const int /*problem*/, const std::string &/*cause*/, const std::string &/*data*/, const std::string &/*text*/)
{
//	ACS_AEH_EvReport EvReporter;
//
//	std::string *processName;
//	try{
//	processName = new std::string();
//	}
//	catch(...)
//	{
//		if (ACS_TRA_ON(traceUtil))
//		{//trace - new failed for processName
//			char tmpStr[512] = {0};
//			_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] New failed for processName, not raising event", __FUNCTION__,__LINE__);
//			if (ACS_TRA_ON(traceUtil)) ACS_TRA_event(&traceUtil, tmpStr);
//			std::cout << "DBG: " << tmpStr << std::endl;
//		}
//	}
//
//	int pid = _getpid();
//	std::ostringstream id;
//	id << pid;
//
//	processName->append(FIXS_TREFC_PROCESS_NAME);
//	processName->append(":");
//	processName->append(id.str());
//
//	if (ACS_TRA_ON(traceUtil))
//	{
//		char tmpStr[512] = {0};
//		_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] problem = %d, cause = %s, data = %s, text = %s\n",__FUNCTION__, problem, cause.c_str(), data.c_str(), text.c_str());
//		ACS_TRA_event(&traceUtil, tmpStr);
//	}
//
//    ACS_AEH_ReturnType rt = EvReporter.sendEventMessage(processName->c_str(),
//                                                              problem,
//                                                              "EVENT",
//                                                              cause.c_str(),
//                                                              "APZ",
//															  FIXS_TREFC_PROCESS_NAME,
//                                                              data.c_str(),
//                                                              text.c_str());
//
//	delete processName;
//	if (rt == ACS_AEH_error)
//    {
//		ACS_AEH_ErrorReturnType error = EvReporter.getError();
//	    switch (error)
//       	{
//			case ACS_AEH_syntaxError:
//				if (ACS_TRA_ON(traceUtil))
//				{
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Event syntax error\n", __FUNCTION__);
//					ACS_TRA_event(&traceUtil, tmpStr);
//				}
//			break;
//
//	        case ACS_AEH_eventDeliveryFailure:
//				if (ACS_TRA_ON(traceUtil))
//				{
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Event delivery failure\n", __FUNCTION__);
//					ACS_TRA_event(&traceUtil, tmpStr);
//				}
//        	break;
//
//   	        default:
//				if (ACS_TRA_ON(traceUtil))
//				{
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s] Unknown event failure\n", __FUNCTION__);
//					ACS_TRA_event(&traceUtil, tmpStr);
//				}
//        	break;
//		}
//	}

}


bool TREFC_Util::stringToUpper (std::string& stringToConvert)
{
	//Changes each element of the string to upper case
	std::string convertedString = stringToConvert;

	for(unsigned int i = 0; i < stringToConvert.length(); i++)
	{
		convertedString[i] = toupper(stringToConvert[i]);
	}

	stringToConvert = convertedString;    //return the converted string

	return true;
}


bool TREFC_Util::isApzSCXMagazine(std::string /*snmpIP*/, unsigned short &/*slot*/)
{
//	int retValue = 0;
//	unsigned long SCXIP = 0, APZMag = 0;
//
//	std::cout << "DBG: snmpIP: " << snmpIP<< std::endl;
//
//	istringstream istrSCXIP(snmpIP);
//
//    if (numOfChar(istrSCXIP,'.') != 3)   // If not 3 dots, faulty format
//       return false;
//	std::cout << "DBG: SCXIP 1: " << SCXIP<< std::endl;
//
//    if (! dottedDecToLong(istrSCXIP, SCXIP, 0, 255, 0, 255, 0, 255, 0, 255))
//       return false;
//
//	std::cout << "DBG: SCXIP 2: " << SCXIP<< std::endl;
//
//	//unsigned long compMag = magazine;
//	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
//	if (hwc)
//	{
//			ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
//			if (boardSearch)
//			{
//				ACS_CS_API_IdList boardList;
//				unsigned short sysId = ACS_CS_API_HWC_NS::SysType_AP + 1; //2001
//				boardSearch->setSysId(sysId);
//				ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
//				if (returnValue == ACS_CS_API_NS::Result_Success)
//				{
//					if (boardList.size() == 0)
//					{
//						std::cout<<"No board"<<std::endl;
//						if (ACS_TRA_ON(traceUtil))
//						{
//							char tmpStr[512] = {0};
//							_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No board found", __LINE__);
//							ACS_TRA_event(&traceUtil, tmpStr);
//						}
//						retValue = 1;
//					}
//					else
//					{
//						BoardID boardId = boardList[0];
//						retValue = hwc->getMagazine(APZMag,boardId);
//						if (returnValue != ACS_CS_API_NS::Result_Success)
//						{
//							retValue = 1;
//						}
//					}
//				}
//			}
//			else
//			{
//				if (ACS_TRA_ON(traceUtil))
//				{//trace - GetBoardIds failed, error from CS
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, retValue);
//					if (ACS_TRA_ON(traceUtil)) ACS_TRA_event(&traceUtil, tmpStr);
//					std::cout << "DBG: " << tmpStr << std::endl;
//				}
//				retValue = 1;
//			}
//
//			boardSearch->reset();
//
//
//			if (retValue ==0)
//			{
//				ACS_CS_API_IdList boardList;
//				boardSearch->setMagazine(APZMag);
//				boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SCXB);
//				ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
//				if (returnValue == ACS_CS_API_NS::Result_Success)
//				{
//					if (boardList.size() == 0)
//					{
//						std::cout<<"No board"<<std::endl;
//						if (ACS_TRA_ON(traceUtil))
//						{
//							char tmpStr[512] = {0};
//							_snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] No board found", __LINE__);
//							ACS_TRA_event(&traceUtil, tmpStr);
//						}
//						retValue = 1;
//					}
//					else
//					{
//						for(int i = 0; i < (int)boardList.size();i++)
//						{
//							BoardID boardId = boardList[i];
//							unsigned long ipA = 0,ipB = 0;
//							retValue = hwc->getIPEthA(ipA, boardId);
//							std::cout << "DBG: ipA : " << ipA<< std::endl;
//							if (retValue == ACS_CS_API_NS::Result_Success)
//							{
//								if (SCXIP == ipA)
//								{
//									std::cout<<"Magazine matches :  "<<std::endl;
//									hwc->getSlot(slot,boardId);
//									//if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
//									retValue = 0;
//									break;
//								}
//								else
//								{
//									retValue = hwc->getIPEthB(ipB, boardId);
//									std::cout << "DBG: ipB : " << ipB<< std::endl;
//									if (retValue == ACS_CS_API_NS::Result_Success)
//									{
//										if (SCXIP == ipB)
//										{
//											std::cout<<"Magazine matches :  "<<std::endl;
//											hwc->getSlot(slot,boardId);
//											//if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
//											retValue = 0;
//											break;
//										}
//										else
//										{
//											retValue = 1;
//										}
//									}
//									else
//									{
//										retValue = 1;
//									}
//								}
//							}
//							else
//							{
//								retValue = 1;
//							}
//						}
//					}
//				}
//			}
//			else
//			{
//				if (ACS_TRA_ON(traceUtil))
//				{//trace - GetBoardIds failed, error from CS
//					char tmpStr[512] = {0};
//					_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, retValue);
//					if (ACS_TRA_ON(traceUtil)) ACS_TRA_event(&traceUtil, tmpStr);
//					std::cout << "DBG: " << tmpStr << std::endl;
//				}
//				retValue = 1;
//			}
//		if (boardSearch) ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
//	}
//
//	if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
//	if(retValue == 0)
//		return true;
//	else
		return false;
}


//***************************************************************************
//This function is used to read the PHA parameters from the .par file
//***************************************************************************

int TREFC_Util::GetServiceParams(ServiceParams */*ptrParams*/)
{
//	ACS_PHA_Tables  CXCParams(CXC_NAME);
//	ACS_PHA_Parameter<int>	srvPort("FIXS_TREFCBIN_Port");
//
//	if (srvPort.get(CXCParams) != ACS_PHA_PARAM_RC_OK )
//    {
//		std::cout<<"DBG: FIXS_TREFC_Util - Getting the PHA parameters failed "<<srvPort.get(CXCParams)<<std::endl;
//		ErrorHandler(READ_PARAM_ERROR,0,SERVICE_PORT);
//		return ERROR;
//
//    }
//	ptrParams->sPort = srvPort.data();
//	std::cout<<"DBG: FIXS_TREFC_Util - Getting the PHA parameters success and port value is "<<ptrParams->sPort<<std::endl;
//	return SUCCESS;
	return 0;
}


//***************************************************************************
//This function is a error handler function which raises event in system event log
//***************************************************************************
void TREFC_Util::ErrorHandler (char */*Err*/ , int /*ErrCode*/ ,char */*paramName*/ )
{
//	char strErrData[100],strAPErrData[200],strTmp[10];
//	strErrData[0] = 0;
//	if(paramName != NULL)
//	{
//		strcpy(strAPErrData,Err);
//		strcat(strAPErrData," - ");
//		strcat(strAPErrData,paramName);
//	}
//	if(ErrCode != 0)
//	{
//		itoa(ErrCode,strTmp,10);
//		strcpy(strErrData,"Microsoft Error : ");
//		strcat(strErrData,strTmp);
//	}
//
//	sendAlarm (14275, Severity_Event, strAPErrData, strErrData, "");
}

//TREFC_Util::EnvironmentType TREFC_Util::getEnvironment(void)
//{
//
//	TREFC_Util::EnvironmentType env;
//
//	ImmInterface::getEnvironment(env);
//
//	return env;

//	bool resp = true;
//	int CBA_SCX = -1;
//	int SYSTEM_TYPE = -1;
//	bool isCBA = false;
//	bool multipleCPSystem = false;
//
//	//get attributes value
//	if (!IMM_Util::getImmAttributeInt(IMM_Util::DN_APZ,IMM_Util::ATT_APZ_SYSTEM,SYSTEM_TYPE)) resp = false;
//
//	if (!IMM_Util::getImmAttributeInt(IMM_Util::DN_APZ,IMM_Util::ATT_APZ_CBA,CBA_SCX)) resp = false;
//
//	if(resp)
//	{
//		if (SYSTEM_TYPE == IMM_Util::MULTI_CP_SYSTEM) multipleCPSystem = true;
//		if (CBA_SCX == IMM_Util::EGEM2_SCX_SA) isCBA = true;
//
//		if (multipleCPSystem && isCBA)
//		{
//		/*	if (ACS_TRA_ON(traceUtil))
//			{ //trace
//				char tmpStr[512] = {0};
//				snprintf(tmpStr, sizeof(tmpStr) - 1,
//					"[%s@%d] Multiple-CP,CBA Environment ",
//					__FUNCTION__, __LINE__);
//				ACS_TRA_event(&traceUtil, tmpStr);
//			}
//		*/	return MULTIPLECP_CBA;
//		}
//		else if (multipleCPSystem && (!isCBA))
//		{
//		/*	if (ACS_TRA_ON(traceUtil))
//			{ //trace
//				char tmpStr[512] = {0};
//				snprintf(tmpStr, sizeof(tmpStr) - 1,
//					"[%s@%d] Multiple-CP,Not CBA Environment ",
//					__FUNCTION__, __LINE__);
//				ACS_TRA_event(&traceUtil, tmpStr);
//			}
//		*/	return MULTIPLECP_NOTCBA;
//		}
//		else if((!multipleCPSystem) && isCBA)
//		{
//		/*	if (ACS_TRA_ON(traceUtil))
//			{ //trace
//				char tmpStr[512] = {0};
//				snprintf(tmpStr, sizeof(tmpStr) - 1,
//					"[%s@%d] Single-CP,CBA Environment ",
//					__FUNCTION__, __LINE__);
//				ACS_TRA_event(&traceUtil, tmpStr);
//			}
//		*/	return SINGLECP_CBA;
//		}
//		else
//		{
//		/*	if (ACS_TRA_ON(traceUtil))
//			{ //trace
//				char tmpStr[512] = {0};
//				snprintf(tmpStr, sizeof(tmpStr) - 1,
//					"[%s@%d] Single-CP,Not CBA not supported by IPTB",
//					__FUNCTION__, __LINE__);
//				ACS_TRA_event(&traceUtil, tmpStr);
//			}
//		*/	return SINGLECP_NOTCBA; // single-CP, NO CBA not supported by IPTB
//		}
//	}
//	else
//	{
//	/*	if (ACS_TRA_ON(traceUtil))
//		{ //trace
//			char tmpStr[512] = {0};
//			snprintf(tmpStr, sizeof(tmpStr) - 1,
//				"[%s@%d] Failed to get multiple CP system.GetLastError(): %u",
//				__FUNCTION__, __LINE__,GetLastError());
//			ACS_TRA_event(&traceUtil, tmpStr);
//		}
//	*/	return UNKNOWN;
//	}


//}

std::string TREFC_Util::GetAPHostName()
{
    char name[255] = {0};

	{
		if( gethostname( name, sizeof(name)) == 0)
		{
			//OK
		/*	if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] read hostname %s", __FUNCTION__, __LINE__, name);
				ACS_TRA_event(&traceUtil, tmpStr);
			}
		*/
		}
		else
		{
		/*	if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Failed to retrieve the hostname", __FUNCTION__, __LINE__);
				ACS_TRA_event(&traceUtil, tmpStr);
			}
		*/
		}
	}
    return name;
}

void TREFC_Util::getCurrentTime(std::string& dateStr, std::string& timeStr)
{
	time_t longTime;
	time(&longTime);  // Get the current time
	struct tm* today = localtime(&longTime); // Convert to local time

	// Retrieve time information
	char dateChar[256] = {0};
	strftime(dateChar, sizeof (dateChar) - 1, "%Y-%m-%d", today);

	char timeChar[256] = {0};
	strftime(timeChar, sizeof (timeChar) - 1, "%H:%M:%S ", today);

	dateStr = dateChar;
	timeStr = timeChar;
}



std::string TREFC_Util::getCurrentTimeInSeconds()
{
	long timeinseconds;
	int seconds;
	int minutes;
	int hours;
	std::string secInStr;

	time_t longTime;
	time(&longTime);  // Get the current time
	struct tm* today = localtime(&longTime); // Convert to local time

	seconds = today->tm_sec;
	minutes = today->tm_min;
	hours = today->tm_hour;

	timeinseconds = (hours*3600)+(minutes*60)+seconds;

	TREFC_Util::longToString(timeinseconds,secInStr);

	return secInStr;
}


bool TREFC_Util::stringToUlongMagazine (std::string value, unsigned long &uvalue)
{

	// convert value
	std::istringstream istr(value);
	reverseDottedDecStr(istr); //reverses the order of the dotted decimal string to match the mag addr format on the server
	if (numOfChar(istr,'.') != 3) // If not 3 dots, faulty format
	{
		return false;
	}
	if (! dottedDecToLong(istr, uvalue, 0, 15, 0, 0, 0, 15, 0, 15))
	{
		return false;
	}
	//std::cout <<"mag: "<< value.c_str()<< "umag: "<< uvalue<< std::endl;
	return true;

}

bool TREFC_Util::ulongToStringMagazine (uint32_t uvalue, std::string &value)
{

	char tempValueStr[16] = {0}; //size 16, "255.255.255.255" + null character
	uint32_t tempValue;
	tempValue =  ntohl(uvalue);  //changes the representation to little-endian
	snprintf(tempValueStr, sizeof(tempValueStr) - 1, "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    value = tempValueStr;

	return true;

}

bool TREFC_Util::stringToUshort (std::string value, unsigned short &uvalue)
{

	std::istringstream istr(value);
	if (! checkFigures(istr))  //checks that only figures (0-9) are used in the option arg.
	{
		return false; //Usage
	}

	if (! (istr >> uvalue))   //try to convert operand to value number
	{
		return false; //Usage
	}

	if (uvalue > 25)
	{
		return false; //Usage
	}

	return true;
}

bool TREFC_Util::ushortToString (unsigned short uvalue, std::string &value)
{

	char tmp[16] = {0};
	snprintf(tmp, sizeof(tmp) - 1, "%u", uvalue);
	value = tmp;

	return true;
}

bool TREFC_Util::longlongToString (LONGLONG uvalue, std::string &value)
{
	std::stringstream ss;
	ss << uvalue;
	ss >> value;

	return true;
}


bool TREFC_Util::longToString (long uvalue, std::string &value)
{

	char tmp[16] = {0};
	snprintf(tmp,  sizeof(tmp) - 1,"%ld", uvalue);
	value = tmp;

	return true;
}

bool TREFC_Util::stringToLong (std::string value, long &uvalue)
{
	std::istringstream istr(value);
	if (! checkFigures(istr))  //checks that only figures (0-9) are used in the option arg.
	{
		return false; //Usage
	}

	char temp[16]={0};
	int strlen = value.size();

	int i;
	for (i=0;i<=strlen;i++)
	{
		temp[i]=value[i];
	}
	uvalue = atol(temp);

	return true;
}

bool TREFC_Util::doubleToString (double dvalue, std::string &value)
{

	std::ostringstream sstream;
	sstream << dvalue;
	value = sstream.str();

	return true;
}


bool TREFC_Util::stringToInt (std::string value, int &ivalue)
{
	std::istringstream istr(value);
	if (! checkFigures(istr))  //checks that only figures (0-9) are used in the option arg.
	{
		return false; //Usage
	}
	if (! (istr >> ivalue)) //try to convert operand to value number
	{
		return false; //Usage
	}

	return true;
}


std::string TREFC_Util::intToString (int ivalue)
{
	std::stringstream ss_value("");
	ss_value << ivalue; //try to convert operand to value number
	return ss_value.str();
}


bool TREFC_Util::intToUshort (int value, unsigned short &ivalue)
{
	std::ostringstream oss;
	oss << value;

	std::string res = oss.str();

	if (stringToUshort(res, ivalue))return true;
	else return false;
}

void TREFC_Util::reverseDottedDecStr (std::istringstream &istr)
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

TREFC_Util::EnvironmentType TREFC_Util::getEnvironment() {

	if (TREFC_Util::Environment == TREFC_Util::UNKNOWN)
	{
		if (ImmInterface::getEnvironment(TREFC_Util::Environment) != 0)
			TREFC_Util::Environment = TREFC_Util::UNKNOWN;
	}

	return TREFC_Util::Environment;
}

string TREFC_Util::getErrrorText(int error) {
	string text = "";

	switch(error){
	case WRONG_IP_ADDRESS:						text = "Incorrect IP address"; 											       break;
	case INVALID_NETMASK:						text = "Incorrect netmask";
								  break;
	
	case INVALID_GATEWAY:						text = "Incorrect default gateway";
								  break;

	case NOT_ALLOWED:						text = "Cannot delete last NetworkTimeServer when operationalState is ENABLED"; 			       	       break;

	case TIME_REF_IP_ALREADY_EXIST:					text = "Network Time Server IP address already exists"; 							       break;

	case SERVICE_TYPE_CHANGE_NOT_ALLOWED:       			text = "Cannot modify serviceType when operationalState is ENABLED";    					       break;

	case REFERENCE_TYPE_CHANGE_NOT_ALLOWED:     			text = "Cannot modify referenceType when operationalState is ENABLED"; 					               break;

	case EXT_IP_REMOVE_NOT_ALLOWED:					text = "Cannot delete networkConfiguration when operationalState is ENABLED"; 			                       break;

	case ILLEGAL_COMMAND:						text = "Operation not allowed in this system configuration";						 	       break;

	case MODIFICATION_NOT_ALLOWED:					text = "Cannot modify IP address";						 		                       break;

	default:							text = "Error when executing (general fault)";
	}

	return text;
}
