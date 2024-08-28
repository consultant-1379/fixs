/*
 * FIXS_TREFC_ImmInterface.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: estevol
 */

#include "FIXS_TREFC_ImmInterface.h"
#include "FIXS_TREFC_Logger.h"

int ImmInterface::getAPZShelfAddress(uint32_t &address) {

	int retValue = 0;
	unsigned int magazine = 0;

	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			unsigned short sysId = ACS_CS_API_HWC_NS::SysType_AP + 1; //2001
			boardSearch->setSysId(sysId);
			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 0)
				{
					// trace: no APUB board found.it will never reach this place
					//printTRA("No APUB board found");
					retValue = INTERNALERR;
				}
				else
				{
					for (unsigned int i = 0; i < boardList.size(); i++)
					{
						BoardID boardId = boardList[i];
						returnValue = hwc->getMagazine (magazine, boardId);
						if (returnValue == ACS_CS_API_NS::Result_Success)
						{
							break; //APUB magazine found
						}
					}
				}
			}
			else
			{
				//trace - GetBoardIds failed, error from CS
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] GetBoardIds failed, error from CS: %u", __FUNCTION__,__LINE__, returnValue);
				//printTRA(tmpStr);
				std::cout << "DBG: " << tmpStr << std::endl;
				retValue = NO_REPLY_FROM_CS;
			}

			if(retValue == 0) {
				address=magazine;
			}
		}
		else
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS....", __LINE__);
			//printTRA(tmpStr);
			retValue = NO_REPLY_FROM_CS;
		}
		if (boardSearch)
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	}
	else
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS....", __LINE__);
		//printTRA(tmpStr);
		retValue = NO_REPLY_FROM_CS;
	}

	if (hwc) ACS_CS_API::deleteHWCInstance(hwc);
	return retValue;

}

int ImmInterface::getAPZShelfAddress(std::string &address) {
	uint32_t uint_address = 0;
	int res = 0;
	res = getAPZShelfAddress(uint_address);

	if (res == 0) {
		if (!ulongToStringMagazine(uint_address, address))
			res = 1;
//		else {
//			std::istringstream mag(address);
//			TREFC_Util::reverseDottedDecStr(mag);
//			address = mag.str();
//		}
	}
	return res;
}

bool ImmInterface::setServiceStatus(IMM_Util::TimeServiceStatus servStatus)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"IN setServiceStatus" <<std::endl; //mychange
	bool res = true;
	int sStatus = servStatus;
	ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterInt(IMM_Util::ATT_TIMEREFERENCE_STATUS,ATTR_INT32T,&sStatus,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::ATT_TIMEREFERENCE_OBJ,parToModify)== ACS_CC_FAILURE)
	{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"IN setServiceStatus IMM_Util::modify FAILED res=false" <<std::endl; //mychange
	 res = false;
	}
	delete [] parToModify.attrValues;
	return res;
}

//@
bool ImmInterface::setSwitchServiceRefType(IMM_Util::ExternalReferenceType SxServRefType)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"IN setSwitchServiceRefType" <<std::endl;
	bool res = true;
	int Sx_servRefType = SxServRefType;
	ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterInt(IMM_Util::ATT_SWITCHTIMESERVICE_REF_TYPE,ATTR_INT32T,&Sx_servRefType,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::ATT_SWITCHTIMESERVICE_DN,parToModify)== ACS_CC_FAILURE)
	{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"IN setSwitchServiceRefType IMM_Util::modify FAILED res=false" <<std::endl;
	 res = false;
	}
	delete [] parToModify.attrValues;
	return res;
}

bool ImmInterface::setSwitchServiceType(IMM_Util::TimeServiceType SxServType)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"IN setSwitchServiceType" <<std::endl;
	bool res = true;
	int Sx_servType = SxServType;
	ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterInt(IMM_Util::ATT_SWITCHTIMESERVICE_TYPE,ATTR_INT32T,&Sx_servType,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::ATT_SWITCHTIMESERVICE_DN,parToModify)== ACS_CC_FAILURE)
	{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"IN setSwitchServiceType IMM_Util::modify FAILED res=false" <<std::endl;
	 res = false;
	}
	delete [] parToModify.attrValues;
	return res;
}

/*bool ImmInterface::setSwitchA_TrefIP(string SxNwIP)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"IN setSwitchServiceType" <<std::endl;
	bool res = true;
	const char *Sx_NwIP = SxNwIP;
	ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterString(IMM_Util::ATT_SWITCHTIMEREFERENCE_IP,ATTR_STRINGT,&Sx_NwIP,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_NW_OBJ,parToModify)== ACS_CC_FAILURE)
	{
	std::cout << __FUNCTION__ << "@" << __LINE__ <<"IN setSwitchServiceType IMM_Util::modify FAILED res=false" <<std::endl;
	 res = false;
	}

	return res;
}*/

//@

//ACS_CC_ReturnType ImmInterface::setServiceType(IMM_Util::TimeServiceType servType) {
//
//	ACS_CC_ReturnType setResult;
//	int intType = servType;
//	ACS_CC_ImmParameter serviceType = IMM_Util::defineParameterInt(IMM_Util::ATT_TIMEREFERENCE_TYPE,ATTR_INT32T,&intType,1);
//	setResult = FIXS_TREFC_OI_TimeService::getInstance()->modifyRuntimeObj(IMM_Util::ATT_TIMEREFERENCE_OBJ,&serviceType);
//	if (setResult == ACS_CC_FAILURE)
//		cout << " FIXS_TREFC_TimeService modify " << IMM_Util::ATT_TIMEREFERENCE_TYPE<< " FAILED !!!"<< endl;
//	delete [] serviceType.attrValues;
//
//	return setResult;
//}

int ImmInterface::getAPZSwitchMagazineSlot (unsigned char &shelf, bool &slot0, bool &slot25, bool isSMX)
{
	int retValue = 0;
	unsigned int magazine = 0;
	slot0 =false;
	slot25 = false;

	ACS_CS_API_NS::CS_API_Result returnValue;

	retValue = getAPZShelfAddress(magazine);
	ACS_CS_API_HWC * hwc = NULL;
	ACS_CS_API_BoardSearch * boardSearch = NULL;

	if(retValue == 0) {

		hwc = ACS_CS_API::createHWCInstance();

		if (hwc)
		{
			boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
			if (boardSearch)
			{
				boardSearch->reset();
				boardSearch->setMagazine(magazine);
				if(isSMX)
					boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SMXB);
				else
					boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SCXB);
				ACS_CS_API_IdList boardList1;
				ACS_CS_API_NS::CS_API_Result returnValue1 = hwc->getBoardIds(boardList1, boardSearch);
				if (returnValue1 == ACS_CS_API_NS::Result_Success)
				{
					if (boardList1.size() > 0 )
					{
						unsigned long hlmag = ntohl(magazine);
						unsigned char u_plug0 = (unsigned char)(hlmag >> 24);
						shelf = u_plug0 & 0x0F;

						for (unsigned int i = 0; i < boardList1.size(); i++)
						{
							BoardID boardId = boardList1[i];
							unsigned short slot = 0;
							returnValue = hwc->getSlot (slot, boardId);
							if (returnValue == ACS_CS_API_NS::Result_Success)
							{
								if(slot == 0)
									slot0 = true;
								else if(slot == 25)
									slot25 =true;
							}
							else
							{
								//serious fault in CS: No IP_EthA found for the SCXB board
								char tmpStr[512] = {0};
								snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No IP_EthA found for the SCXB/SMXB board", __LINE__);
								//printTRA(tmpStr);
								retValue = NO_REPLY_FROM_CS;
								break;
							}
						}
					}
					else
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Board List is empty", __FUNCTION__, __LINE__);
						//printTRA(tmpStr);
						retValue = INTERNALERR;
					}
				}
				else
				{
					//serious fault in CS: No magazine found for the SCXB board
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS: No magazine found for the SCXB/SMXB board", __LINE__);
					//printTRA(tmpStr);
					retValue = NO_REPLY_FROM_CS;
				}
			}
		}
		else
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS....", __LINE__);
			//printTRA(tmpStr);
			retValue = NO_REPLY_FROM_CS;
		}
		if (boardSearch)
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	}
	else
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[line:%d] Serious fault in CS....", __LINE__);
		//printTRA(tmpStr);
		retValue = NO_REPLY_FROM_CS;
	}

	if (hwc)
		ACS_CS_API::deleteHWCInstance(hwc);
	return retValue;

}
//@
bool ImmInterface::getAdminStatus(int &adminState) {

	return IMM_Util::getImmAttributeInt(IMM_Util::ATT_TIMEREFERENCE_OBJ,IMM_Util::ATT_TIMEREFERENCE_ADMIN_STATE,adminState);
}

bool ImmInterface::getNwTimeServerName(std::string &name) {

	return IMM_Util::getImmAttributeString(IMM_Util::ATT_TIMEREFERENCE_OBJ,IMM_Util::ATT_TIMEREFERENCE_NWTIMESRV_NAME,name);
}

//@
bool ImmInterface::getServiceStatus(int &status) {

	return IMM_Util::getImmAttributeInt(IMM_Util::ATT_TIMEREFERENCE_OBJ,IMM_Util::ATT_TIMEREFERENCE_STATUS,status);
}

bool ImmInterface::getServiceType(int &type, bool isSMX) {
	
	if(isSMX)
		return IMM_Util::getImmAttributeInt(IMM_Util::ATT_SWITCHTIMESERVICE_DN,IMM_Util::ATT_SWITCHTIMESERVICE_TYPE,type);
	else
		return IMM_Util::getImmAttributeInt(IMM_Util::ATT_SCXTIMESERVICE_DN,IMM_Util::ATT_SCXTIMESERVICE_TYPE,type);
		
}

bool ImmInterface::getReferenceType(int &type, bool isSMX) {

	if(isSMX) {
		return IMM_Util::getImmAttributeInt(IMM_Util::ATT_SWITCHTIMESERVICE_DN,IMM_Util::ATT_SWITCHTIMESERVICE_REF_TYPE,type);
	}
	else {
		return IMM_Util::getImmAttributeInt(IMM_Util::ATT_SCXTIMESERVICE_DN,IMM_Util::ATT_SCXTIMESERVICE_REF_TYPE,type);
	}
		
}

bool ImmInterface::getSCXAtrefIP(std::string &scxIP) {

	return IMM_Util::getImmAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_SCX_NW_OBJA,IMM_Util::ATT_SCXTIMEREFERENCE_IP,scxIP); //@newly added for SCX A
}

bool ImmInterface::getSCXAtrefNetMask(std::string &scxMask) {

	return IMM_Util::getImmAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_SCX_NW_OBJA,IMM_Util::ATT_SCXTIMEREFERENCE_MASK,scxMask); //@newly added for SCX A
}

bool ImmInterface::getSCXAtrefGatewayIP(std::string &scxGIP) {

	return IMM_Util::getImmAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_SCX_NW_OBJA,IMM_Util::ATT_SCXTIMEREFERENCE_GATEWAY,scxGIP); //@newly added for SCX A
}

bool ImmInterface::getSCXBtrefIP(std::string &scxIP) {

	return IMM_Util::getImmAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_SCX_NW_OBJB,IMM_Util::ATT_SCXTIMEREFERENCE_IP,scxIP); //@newly added for SCX A
}

bool ImmInterface::getSCXBtrefNetMask(std::string &scxMask) {

	return IMM_Util::getImmAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_SCX_NW_OBJB,IMM_Util::ATT_SCXTIMEREFERENCE_MASK,scxMask); //@newly added for SCX A
}

bool ImmInterface::getSCXBtrefGatewayIP(std::string &scxGIP) {

	return IMM_Util::getImmAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_SCX_NW_OBJB,IMM_Util::ATT_SCXTIMEREFERENCE_GATEWAY,scxGIP); //@newly added for SCX A
}

int ImmInterface::getTimeReferencesList(std::map<std::string,std::string> &tRefList, bool isSMX) {

	int refType=-1;
	if(getReferenceType(refType,isSMX)) {
		if(refType == IMM_Util::GPS_REFERENCE)
		{
			string ipAddress = "GPS DEVICE";
			string name = "-";
			tRefList.insert ( pair<string,string>(ipAddress, name) );
			return 0;
		}
	}
	else {
		cout << "Failed to get Time References Type " << std::endl;
		return 1;
	}

	std::vector<std::string> refList;
	if (IMM_Util::getClassObjectsList(IMM_Util::classTimeServer,refList)) {

		string ref_dn("");
		string rdn_ref("");
		string name("");
		string ipAddress("");


		//loop of instances
		for (unsigned i=0; i<refList.size(); i++)
		{
			ref_dn.clear();
			rdn_ref.clear();
			name.clear();
			ipAddress.clear();

			ref_dn = refList[i].c_str();
//			//check the instance
//			IMM_Util::getRdnAttribute(ref_dn,rdn_ref);
//
//			if (strcmp(rdn_ref.c_str(),IMM_Util::ATT_GPSDEVICE_RDN) == 0)
//			{
//				ipAddress = "GPS DEVICE";
//				name = "-";
//				tRefList.insert ( pair<string,string>(ipAddress, name) );
//			}
//			else
//			{
				//get IpAddress and name
				cout << "dn of ref: "<< ref_dn << endl;
				IMM_Util::getImmAttributeString(ref_dn,IMM_Util::ATT_TIMESERVER_IP,ipAddress);
				//IMM_Util::getImmAttributeString(ref_dn,IMM_Util::ATT_TIMESERVER_RDN,name);
				name = IMM_Util::getIdValueFromRdn(ref_dn);
				tRefList.insert ( pair<string,string>(ipAddress, name) );
			//}
		}
	} else {
		cout << "Failed to get Time References Configuration" << std::endl;
		return 1;
	}
	return 0;
}

//TODO: this attribute should be moved somewhere else or should be accessible via CS APIs
bool ImmInterface::getAptEthInPort(uint32_t magazine, unsigned short uslot, int &port_out) {

	vector<string> bladeList;
	bool result = IMM_Util::getClassObjectsList(IMM_Util::classOtherBlade,bladeList);
	std::string shelf_dn;
	std::string shelfName;

	int aptEthInPort = 31;

	if (bladeList.size() > 0) {
		IMM_Util::getDnParent(bladeList[0].c_str(),shelf_dn);

		string tempDn = shelf_dn.substr(shelf_dn.find_first_of(',')+1, string::npos);

		TREFC_Util::ulongToStringMagazine(magazine, shelfName);
//		std::istringstream mag(shelfName);
//		TREFC_Util::reverseDottedDecStr(mag);
//		shelfName = mag.str();

		//replace(shelfName.begin(), shelfName.end(), '.', '_' );
		cout << "shelf name : " << shelfName << endl;
		std::string othBlade_rdn(IMM_Util::ATT_OTHERBLADE_RDN);
		std::string shelf_rdn(IMM_Util::ATT_SHELF_RDN);
		std::string str_slot("");
		TREFC_Util::ushortToString(uslot, str_slot);
		tempDn = othBlade_rdn + "=" + str_slot + "," + shelf_rdn + "=" + shelfName + "," + tempDn;
		cout << "temp dn : " << tempDn << endl;
		result = IMM_Util::getImmAttributeInt(tempDn, IMM_Util::ATT_OTHERBLADE_APTETHPORT, aptEthInPort);
		cout << "aptEthInPort : " << aptEthInPort << endl;
	}

	port_out = aptEthInPort;
	return result;
}

bool ImmInterface::getAptEthInPort(std::string magazine, unsigned short uslot, int &port_out) {

	vector<string> bladeList;
	bool result = IMM_Util::getClassObjectsList(IMM_Util::classOtherBlade,bladeList);
	std::string shelf_dn;
	std::string shelfName = magazine;

	int aptEthInPort = 31;

	if (bladeList.size() > 0) {
		IMM_Util::getDnParent(bladeList[0].c_str(),shelf_dn);

		string tempDn = shelf_dn.substr(shelf_dn.find_first_of(',')+1, string::npos);

		//replace(shelfName.begin(), shelfName.end(), '.', '_' );
		cout << "shelf name : " << shelfName << endl;
		std::string othBlade_rdn(IMM_Util::ATT_OTHERBLADE_RDN);
		std::string shelf_rdn(IMM_Util::ATT_SHELF_RDN);
		std::string str_slot("");
		TREFC_Util::ushortToString(uslot, str_slot);
		tempDn = othBlade_rdn + "=" + str_slot + "," + shelf_rdn + "=" + shelfName + "," + tempDn;
		cout << "temp dn : " << tempDn << endl;
		result = IMM_Util::getImmAttributeInt(tempDn, IMM_Util::ATT_OTHERBLADE_APTETHPORT, aptEthInPort);
	}
	
	cout << "aptEthInPort ;;;: " << aptEthInPort << endl;
	port_out = aptEthInPort;
	return result;
}


//TODO: these objects should be moved somewhere else
int ImmInterface::getExternalIpAddressConfiguration (ExtIpSwitch &extIpSwitch0, ExtIpSwitch &extIpSwitch25, bool isSMX) {

	vector<string> externIpList;
	bool res=false;
	if(isSMX){
		res=IMM_Util::getClassObjectsList(IMM_Util::classNetworkConfigurationSwitch,externIpList);
	} else {
		res=IMM_Util::getClassObjectsList(IMM_Util::classNetworkConfiguration,externIpList);
	}
	if(!res)
	{
		cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error getClassIstance - NO ExternalIpAddress INSTANCES" << endl;
		return 1;
	}
	else
	{
		string extIp_dn("");
		string ip_A("");
		string netmask_A("");
		string defGateway_A("");
		string ip_B("");
		string netmask_B("");
		string defGateway_B("");

		string dn_parent("");
		string boardSide("");

		for(unsigned int i=0;i<externIpList.size();i++)
		{
			extIp_dn.clear();
			extIp_dn = externIpList[i].c_str();
			dn_parent.clear();
			bool fetchInfoA=false;
			bool fetchInfoB=false;
			//check if scxA or scxb
			IMM_Util::getDnParent(extIp_dn,dn_parent);
			boardSide = IMM_Util::getIdValueFromRdn(dn_parent);
			if(isSMX) {
				if (strcmp(boardSide.c_str(),IMM_Util::SWITCHTIMEREFERENCE_SWITCHA) ==0)
					fetchInfoA=true;
				else if (strcmp(boardSide.c_str(),IMM_Util::SWITCHTIMEREFERENCE_SWITCHB) ==0)
					fetchInfoB=true;
			} else {
				if (strcmp(boardSide.c_str(),IMM_Util::SCXTIMEREFERENCE_SCXA) == 0)
					fetchInfoA=true;
				else if (strcmp(boardSide.c_str(),IMM_Util::SCXTIMEREFERENCE_SCXB) == 0)
					fetchInfoB=true;
			}
			if(fetchInfoA)
			{

				IMM_Util::getImmAttributeString(extIp_dn,IMM_Util::ATT_NETWORKCONFIGURATION_IP,ip_A);
				IMM_Util::getImmAttributeString(extIp_dn,IMM_Util::ATT_NETWORKCONFIGURATION_NETMASK,netmask_A);
				IMM_Util::getImmAttributeString(extIp_dn,IMM_Util::ATT_NETWORKCONFIGURATION_GATEWAY,defGateway_A);

				extIpSwitch0.slot = 0;
				extIpSwitch0.extIp = ip_A;
				extIpSwitch0.defaultGateway = defGateway_A;
				extIpSwitch0.netmask = netmask_A;

			}
			else if (fetchInfoB)
			{

				IMM_Util::getImmAttributeString(extIp_dn,IMM_Util::ATT_NETWORKCONFIGURATION_IP,ip_B);
				IMM_Util::getImmAttributeString(extIp_dn,IMM_Util::ATT_NETWORKCONFIGURATION_NETMASK,netmask_B);
				IMM_Util::getImmAttributeString(extIp_dn,IMM_Util::ATT_NETWORKCONFIGURATION_GATEWAY,defGateway_B);

				extIpSwitch25.slot = 25;
				extIpSwitch25.extIp = ip_B;
				extIpSwitch25.defaultGateway = defGateway_B;
				extIpSwitch25.netmask = netmask_B;
			}
		}
	}
	return 0;
}


int ImmInterface::getSWITCHBoardAddress(vector<SWITCHAPTIPAddress> &switchAPTVector,vector<SWITCHAPZIPAddress> &switchAPZVector, bool isSMX)
{

	uint32_t ApzMagazine;

	int countAptEntry = 0;
	//ACS_CS_API_NS::CS_API_Result returnValue;

	//Search APZ magazine
	int retValue = getAPZShelfAddress(ApzMagazine);

	ACS_CS_API_HWC * hwc = NULL;
	ACS_CS_API_BoardSearch * boardSearch = NULL;

	if(retValue == 0) {

		hwc = ACS_CS_API::createHWCInstance();

		if (hwc)
		{
			boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
			if (boardSearch)
			{
				boardSearch->reset();
				//TODO: This FBN will be eventually added to CS APIs
				if(isSMX)
					boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SMXB);
				else
					boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_SCXB);
				ACS_CS_API_IdList boardList1;
				ACS_CS_API_NS::CS_API_Result returnValue1 = hwc->getBoardIds(boardList1, boardSearch);

				if (returnValue1 == ACS_CS_API_NS::Result_Success)
				{
					if (boardList1.size() > 0 )
					{
						for (unsigned int i = 0; i < boardList1.size(); i++)
						{
							BoardID boardId = boardList1[i];
							uint32_t u_ipA;
							uint32_t u_ipB;
							uint32_t magazine;

							string ipA("");
							string ipB("");

							unsigned short uslot = 0;
							int aptEthPort = 31;

							if (hwc->getSlot (uslot, boardId) != ACS_CS_API_NS::Result_Success)
								return NO_REPLY_FROM_CS;
							if (hwc->getIPEthA(u_ipA, boardId) != ACS_CS_API_NS::Result_Success)
								return NO_REPLY_FROM_CS;
							if (hwc->getIPEthB(u_ipB, boardId) != ACS_CS_API_NS::Result_Success)
								return NO_REPLY_FROM_CS;
							if (hwc->getMagazine(magazine, boardId) != ACS_CS_API_NS::Result_Success)
								return NO_REPLY_FROM_CS;

							if (!getAptEthInPort(magazine, uslot,aptEthPort) )
								aptEthPort = 31;
								//return INTERNALERR;

							ipA = ulongToStringIP(u_ipA);
							ipB = ulongToStringIP(u_ipB);

							if (magazine == ApzMagazine) {
								SWITCHAPZIPAddress switchobj;
								switchobj.IP_EthA = ipA;
								switchobj.IP_EthB = ipB;
								switchobj.slotPos = uslot;
								switchAPZVector.push_back(switchobj);
							} else {
								countAptEntry ++;
								SWITCHAPTIPAddress switchobj;
								switchobj.IP_EthA = ipA;
								switchobj.IP_EthB = ipB;
								switchobj.index = countAptEntry;
								switchobj.ethInPort = aptEthPort;
								switchAPTVector.push_back(switchobj);
								std::cout << " scx.ethInPor : " << aptEthPort <<std::endl;

							}

						}
					}
					else
					{
						retValue = INTERNALERR;
					}
				}
				else
				{
					retValue = NO_REPLY_FROM_CS;
				}
			}
		}
		else
		{
			retValue = NO_REPLY_FROM_CS;
		}
		if (boardSearch)
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
	}

	if (hwc)
		ACS_CS_API::deleteHWCInstance(hwc);
	return retValue;
}

int ImmInterface::getEnvironment(EnvironmentType &env)
{
	int resp = 0;
	int  arch= -1;
	int SYSTEM_TYPE = -1;
	bool isCBA = false;
	bool multipleCPSystem = false;

	//get attributes value
	if (!IMM_Util::getImmAttributeInt(IMM_Util::DN_APZ,IMM_Util::ATT_APZ_SYSTEM,SYSTEM_TYPE))
		resp = 1;

	if (!IMM_Util::getImmAttributeInt(IMM_Util::DN_APZ,IMM_Util::ATT_APZ_CBA,arch))
		resp = 1;

	if(resp == 0)
	{
		if (SYSTEM_TYPE == IMM_Util::MULTI_CP_SYSTEM)
			multipleCPSystem = true;
		if (arch == IMM_Util::EGEM2_SCX_SA) 
			isCBA = true;

		if(arch == IMM_Util::EGEM2_SMX_SA)
		{
			env = SMX;
		}
		else
		{ 
			if (multipleCPSystem && isCBA)
			{
				env = MULTIPLECP_CBA;
			}
			else if (multipleCPSystem && (!isCBA))
			{
				env = MULTIPLECP_NOTCBA;
			}
			else if((!multipleCPSystem) && isCBA)
			{
				env = SINGLECP_CBA;
			}
			else
			{
				env = SINGLECP_NOTCBA; // single-CP, NO CBA not supported by IPTB
			}
		}
	}
	else
	{
	/*	if (ACS_TRA_ON(traceUtil))
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,
				"[%s@%d] Failed to get multiple CP system.GetLastError(): %u",
				__FUNCTION__, __LINE__,GetLastError());
			ACS_TRA_event(&traceUtil, tmpStr);
		}
	*/	env = UNKNOWN;
	}

	return resp;
}

//TODO: these two methods should be rearranged or removed when the externalIPaddress class will be moved
//int ImmInterface::getAPZShelfDN(std::string &shelfAPZ_dn)
//{
//	int res = 0;
//	vector<std::string> apBladeList,otherBladeList,shelfList;
//	IMM_Util::getClassObjectsList(IMM_Util::classApBlade,apBladeList);
//
//	std::string apBoard_dn("");
//	//string shelfAPZ_dn("");
//	bool found = false;
//
//	for (unsigned i=0; i<apBladeList.size(); i++)
//	{
//		apBoard_dn.clear();
//		shelfAPZ_dn.clear();
//		apBoard_dn = apBladeList[i].c_str();
//
//		//get system Number of Ap Board
//		int sysNum = 0;
//		IMM_Util::getImmAttributeInt(apBoard_dn,IMM_Util::ATT_APBLADE_SYSNUM,sysNum);
//
//		if (sysNum == 1) //AP1
//		{
//			//get DN magazine
//			IMM_Util::getDnParent(apBoard_dn,shelfAPZ_dn);
//			cout << "shelfAPZ_dn" << shelfAPZ_dn << endl;
//			found = true;
//			break;
//		}
//	}
//	if(found) {
//		res = 0;
//	}
//	else
//		res = -1;
//
//	return res;
//}
//
//bool ImmInterface::isAPZMagazine(std::string SCXBoard_dn) {
//
//	//std::string SCXBoard_dn(name);
//	std::string shelf_dn(""), APZshelf_dn("");
//
//	IMM_Util::getDnParent(SCXBoard_dn,shelf_dn);
//
//	if (getAPZShelfDN(APZshelf_dn) == 0) {
//		if (shelf_dn == APZshelf_dn)
//			return true;
//		else
//			return false;
//	} else {
//		cout << "Unable to find APZ Magazine" << std::endl;
//		return false;
//	}
//
//}

bool ImmInterface::createSwitchTimeServiceClasses(bool isSMX)
{
	bool res = true;

	int defValue = IMM_Util::NOT_CONFIGURED;
        ACS_APGCC_ImmObject paramListA;
                ACS_APGCC_ImmObject paramListB;

	if(isSMX)
	{
		vector<ACS_CC_ValuesDefinitionType> AttrList;
		ACS_CC_ValuesDefinitionType RDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMESERVICE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMESERVICE_OBJ,1);
		ACS_CC_ValuesDefinitionType SERVTYPE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWITCHTIMESERVICE_TYPE,ATTR_INT32T,&defValue,1);
		ACS_CC_ValuesDefinitionType REFTYPE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWITCHTIMESERVICE_REF_TYPE,ATTR_INT32T,&defValue,1);
		AttrList.push_back(RDN);
		AttrList.push_back(SERVTYPE);
		AttrList.push_back(REFTYPE); 

		if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeService,IMM_Util::ATT_TIMEREFERENCE_OBJ,AttrList)) 
		{
                         int k = IMM_Util::getError();
                               char tmpStr[512] = {0};
                               snprintf(tmpStr, sizeof(tmpStr) - 1, "error createImmObject_NO_OI..Error id :%d",k);
                               FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);

                        if(k == -14)
                        {
                        if((IMM_Util::getObject(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_RDN,&paramListA)) == false)
                        {
                              FIXS_TREFC_Logger::getLogInstance()->Write("There is no getObject of switchA so creating",LOG_LEVEL_DEBUG);
                               //SWITCHA
                        vector<ACS_CC_ValuesDefinitionType> AttrListSwitchA;
                        ACS_CC_ValuesDefinitionType SWITCHARDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_RDN,1);
                        AttrListSwitchA.push_back(SWITCHARDN);

                        IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeReference,IMM_Util::ATT_SWITCHTIMESERVICE_DN,AttrListSwitchA);

                         delete[] SWITCHARDN.attrValues;
                                SWITCHARDN.attrValues = NULL;


                        }
                         if((IMM_Util::getObject(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_RDN,&paramListB)) == false)
                        {
                             FIXS_TREFC_Logger::getLogInstance()->Write("There is no getObject of switchB so creating",LOG_LEVEL_DEBUG);
                                    //SWITCHB
                        vector<ACS_CC_ValuesDefinitionType> AttrListSwitchB;
                        ACS_CC_ValuesDefinitionType SWITCHBRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_RDN,1);
                        AttrListSwitchB.push_back(SWITCHBRDN);

                           IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeReference,IMM_Util::ATT_SWITCHTIMESERVICE_DN,AttrListSwitchB);
                                  //all the obejcts are already present can exit;

                            delete[] SWITCHBRDN.attrValues;
                            SWITCHBRDN.attrValues = NULL;
                        }
                        }
                        else
          		 res = false;

		}
		else
		{
			//SWITCHA
			vector<ACS_CC_ValuesDefinitionType> AttrListSwitchA;
			ACS_CC_ValuesDefinitionType SWITCHARDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_RDN,1);
			AttrListSwitchA.push_back(SWITCHARDN);

			//SWITCHB
			vector<ACS_CC_ValuesDefinitionType> AttrListSwitchB;
			ACS_CC_ValuesDefinitionType SWITCHBRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_RDN,1);
			AttrListSwitchB.push_back(SWITCHBRDN);

			IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeReference,IMM_Util::ATT_SWITCHTIMESERVICE_DN,AttrListSwitchA);
			IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeReference,IMM_Util::ATT_SWITCHTIMESERVICE_DN,AttrListSwitchB);

			//free memory
			delete[] SWITCHARDN.attrValues;
			SWITCHARDN.attrValues = NULL;

			delete[] SWITCHBRDN.attrValues;
			SWITCHBRDN.attrValues = NULL;

		}

		//free memory
		delete[] RDN.attrValues;
		RDN.attrValues = NULL;
		delete[] SERVTYPE.attrValues;
		SERVTYPE.attrValues = NULL;
		delete[] REFTYPE.attrValues;
		REFTYPE.attrValues = NULL;
	}
	else
	{
			vector<ACS_CC_ValuesDefinitionType> AttrList;
			ACS_CC_ValuesDefinitionType RDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMESERVICE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMESERVICE_OBJ,1);
			ACS_CC_ValuesDefinitionType SERVTYPE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWITCHTIMESERVICE_TYPE,ATTR_INT32T,&defValue,1);
			ACS_CC_ValuesDefinitionType REFTYPE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SWITCHTIMESERVICE_REF_TYPE,ATTR_INT32T,&defValue,1);
			AttrList.push_back(RDN);
			AttrList.push_back(SERVTYPE);
			AttrList.push_back(REFTYPE);

			if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeService,IMM_Util::ATT_TIMEREFERENCE_OBJ,AttrList)) 
			{
                         int k = IMM_Util::getError();
                               char tmpStr[512] = {0};
                               snprintf(tmpStr, sizeof(tmpStr) - 1, "error createImmObject_NO_OI..Error id :%d",k);
                               FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);

                        if(k == -14)
                        {
                        if((IMM_Util::getObject(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_RDN,&paramListA)) == false)
                        {
                               FIXS_TREFC_Logger::getLogInstance()->Write("There is no getObject of switchA so creating",LOG_LEVEL_DEBUG);
                               //SWITCHA
                        vector<ACS_CC_ValuesDefinitionType> AttrListSwitchA;
                        ACS_CC_ValuesDefinitionType SWITCHARDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_RDN,1);
                        AttrListSwitchA.push_back(SWITCHARDN);

                        IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeReference,IMM_Util::ATT_SWITCHTIMESERVICE_DN,AttrListSwitchA);

                               delete[] SWITCHARDN.attrValues;
                                SWITCHARDN.attrValues = NULL;


                        }
                         if((IMM_Util::getObject(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_RDN,&paramListB)) == false)
                        {
                                  FIXS_TREFC_Logger::getLogInstance()->Write("There is no getObject of switchB so creating",LOG_LEVEL_DEBUG);            
			//SWITCHB
                        vector<ACS_CC_ValuesDefinitionType> AttrListSwitchB;
                        ACS_CC_ValuesDefinitionType SWITCHBRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_RDN,1);
                        AttrListSwitchB.push_back(SWITCHBRDN);

                           IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeReference,IMM_Util::ATT_SWITCHTIMESERVICE_DN,AttrListSwitchB);
                                  //all the obejcts are already present can exit;

                            delete[] SWITCHBRDN.attrValues;
                                SWITCHBRDN.attrValues = NULL;

                        }
                        }
                        else
                          res = false;

			}
			else
			{
				//SWITCHA
				vector<ACS_CC_ValuesDefinitionType> AttrListSwitchA;
				ACS_CC_ValuesDefinitionType SWITCHARDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_RDN,1);
				AttrListSwitchA.push_back(SWITCHARDN);

				//SWITCHB
				vector<ACS_CC_ValuesDefinitionType> AttrListSwitchB;
				ACS_CC_ValuesDefinitionType SWITCHBRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SWITCHTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_RDN,1);
				AttrListSwitchB.push_back(SWITCHBRDN);

				IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeReference,IMM_Util::ATT_SWITCHTIMESERVICE_DN,AttrListSwitchA);
				IMM_Util::createImmObject_NO_OI(IMM_Util::classSwitchTimeReference,IMM_Util::ATT_SWITCHTIMESERVICE_DN,AttrListSwitchB);

				//free memory
				delete[] SWITCHARDN.attrValues;
				SWITCHARDN.attrValues = NULL;

				delete[] SWITCHBRDN.attrValues;
				SWITCHBRDN.attrValues = NULL;

			}

			//free memory
			delete[] RDN.attrValues;
			RDN.attrValues = NULL;
			delete[] SERVTYPE.attrValues;
			SERVTYPE.attrValues = NULL;
			delete[] REFTYPE.attrValues;
			REFTYPE.attrValues = NULL;

		if (res != false)
		{  //create ScxTimeService and ScxTimeReference objects

			vector<ACS_CC_ValuesDefinitionType> AttrList;
			ACS_CC_ValuesDefinitionType RDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SCXTIMESERVICE_RDN,ATTR_STRINGT,IMM_Util::ATT_SCXTIMESERVICE_OBJ,1);
			ACS_CC_ValuesDefinitionType SERVTYPE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SCXTIMESERVICE_TYPE,ATTR_INT32T,&defValue,1);
			ACS_CC_ValuesDefinitionType REFTYPE = IMM_Util::defineAttributeInt(IMM_Util::ATT_SCXTIMESERVICE_REF_TYPE,ATTR_INT32T,&defValue,1);
			AttrList.push_back(RDN);
			AttrList.push_back(SERVTYPE);
			AttrList.push_back(REFTYPE);

			if (!IMM_Util::createImmObject_NO_OI(IMM_Util::classScxTimeService,IMM_Util::ATT_TIMEREFERENCE_OBJ,AttrList)) 
			{ 
                         int k = IMM_Util::getError();
                               char tmpStr[512] = {0};
                               snprintf(tmpStr, sizeof(tmpStr) - 1, "error createImmObject_NO_OI..Error id :%d",k);
                               FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_WARN);

                        if(k == -14)
                        {
                        if((IMM_Util::getObject(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_RDN,&paramListA)) == false)
                        { 
                            FIXS_TREFC_Logger::getLogInstance()->Write("There is no getObject of switchA so creating",LOG_LEVEL_DEBUG);

                                vector<ACS_CC_ValuesDefinitionType> AttrListScxA;
                                ACS_CC_ValuesDefinitionType SCXARDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_RDN,1);
                                AttrListScxA.push_back(SCXARDN);

                              IMM_Util::createImmObject_NO_OI(IMM_Util::classScxTimeReference,IMM_Util::ATT_SCXTIMESERVICE_DN,AttrListScxA);

                                delete[] SCXARDN.attrValues;
                                SCXARDN.attrValues = NULL;


                        }
                         if((IMM_Util::getObject(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_RDN,&paramListB)) == false)
                        {
                          FIXS_TREFC_Logger::getLogInstance()->Write("There is no getObject of switchB so creating",LOG_LEVEL_DEBUG);
                          //SCXB
                                vector<ACS_CC_ValuesDefinitionType> AttrListScxB;
                                ACS_CC_ValuesDefinitionType SCXBRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_RDN,1);
                                AttrListScxB.push_back(SCXBRDN);

                               IMM_Util::createImmObject_NO_OI(IMM_Util::classScxTimeReference,IMM_Util::ATT_SCXTIMESERVICE_DN,AttrListScxB);

				 delete[] SCXBRDN.attrValues;
                                SCXBRDN.attrValues = NULL;          
                        }
                        }
                        else
				res = false;
			
                        }
			else
			{
				//SCXA
				vector<ACS_CC_ValuesDefinitionType> AttrListScxA;
				ACS_CC_ValuesDefinitionType SCXARDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_RDN,1);
				AttrListScxA.push_back(SCXARDN);

				//SCXB
				vector<ACS_CC_ValuesDefinitionType> AttrListScxB;
				ACS_CC_ValuesDefinitionType SCXBRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_SCXTIMEREFERENCE_RDN,ATTR_STRINGT,IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_RDN,1);
				AttrListScxB.push_back(SCXBRDN);

				IMM_Util::createImmObject_NO_OI(IMM_Util::classScxTimeReference,IMM_Util::ATT_SCXTIMESERVICE_DN,AttrListScxA);
				IMM_Util::createImmObject_NO_OI(IMM_Util::classScxTimeReference,IMM_Util::ATT_SCXTIMESERVICE_DN,AttrListScxB);

				//free memory
				delete[] SCXARDN.attrValues;
				SCXARDN.attrValues = NULL;

				delete[] SCXBRDN.attrValues;
				SCXBRDN.attrValues = NULL;

			}

			//free memory
			delete[] RDN.attrValues;
			RDN.attrValues = NULL;
			delete[] SERVTYPE.attrValues;
			SERVTYPE.attrValues = NULL;
			delete[] REFTYPE.attrValues;
			REFTYPE.attrValues = NULL;
		}

	}

	return res;
}
bool ImmInterface::configureNetworkConfiguration(int slot,std::string Ip,std::string Netmask,std::string Gateway)
{
	bool res=true;
	vector<ACS_CC_ValuesDefinitionType> AttrList;
	const char * ip= Ip.c_str();
	const char * netmask=Netmask.c_str();
	const char * gateway=Gateway.c_str();

	
	
	ACS_CC_ValuesDefinitionType NETWORKCONFIGRDN = IMM_Util::defineAttributeString(IMM_Util::ATT_NETWORKCONFIGURATION_RDN,ATTR_STRINGT,IMM_Util::ATT_NETWORKCONFIGURATION_OBJ,1);
	ACS_CC_ValuesDefinitionType IPADDRESS = IMM_Util::defineAttributeString(IMM_Util::ATT_NETWORKCONFIGURATION_IP,ATTR_STRINGT,ip,1);
	ACS_CC_ValuesDefinitionType GATEWAY = IMM_Util::defineAttributeString(IMM_Util::ATT_NETWORKCONFIGURATION_GATEWAY,ATTR_STRINGT,gateway,1);
	ACS_CC_ValuesDefinitionType NETMASK = IMM_Util::defineAttributeString(IMM_Util::ATT_NETWORKCONFIGURATION_NETMASK,ATTR_STRINGT,netmask,1);

	AttrList.push_back(NETWORKCONFIGRDN);
	AttrList.push_back(IPADDRESS);
	AttrList.push_back(GATEWAY);
	AttrList.push_back(NETMASK);
	
	if(slot==0) {
		std::cout<<"create the object"<<std::endl;
		if(!IMM_Util::createImmObject_NO_OI(IMM_Util::classNetworkConfigurationSwitch,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ,AttrList)) res=false;
		else {
			ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterString(IMM_Util::ATT_SCXTIMEREFERENCE_REF_STRUCT,ATTR_NAMET,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_NW_OBJ,1);
		        if (!IMM_Util::modify_OM_ImmAttr(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ,parToModify)) res = false;
		}
	}
	else {
		if(!IMM_Util::createImmObject_NO_OI(IMM_Util::classNetworkConfigurationSwitch,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ,AttrList)) res=false;
		else {
                        ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterString(IMM_Util::ATT_SCXTIMEREFERENCE_REF_STRUCT,ATTR_NAMET,IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_NW_OBJ,1);
                        if (!IMM_Util::modify_OM_ImmAttr(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ,parToModify)) res = false;
                }

	}

	delete[] NETWORKCONFIGRDN.attrValues;
	NETWORKCONFIGRDN.attrValues = NULL;

	delete[] IPADDRESS.attrValues;
	NETWORKCONFIGRDN.attrValues = NULL;

	delete[] GATEWAY.attrValues;
	NETWORKCONFIGRDN.attrValues = NULL;

	delete[] NETMASK.attrValues;
	NETWORKCONFIGRDN.attrValues = NULL;
	return res;
}
bool ImmInterface::deleteSwitchTimeServiceClasses()
{
	bool res  = IMM_Util::deleteImmObject(IMM_Util::ATT_SCXTIMESERVICE_DN, ACS_APGCC_SUBTREE);
	bool res1 = IMM_Util::deleteImmObject(IMM_Util::ATT_SWITCHTIMESERVICE_DN, ACS_APGCC_SUBTREE);

	if((res == false) || (res1 == false))
		return false;
	else
		return true;
}

bool ImmInterface::setServiceAdmState(IMM_Util::TimeServiceAdm servStatus)
{
	bool res = true;
	int sStatus = servStatus;
	ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterInt(IMM_Util::ATT_TIMEREFERENCE_ADMIN_STATE,ATTR_INT32T,&sStatus,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::ATT_TIMEREFERENCE_OBJ,parToModify)== ACS_CC_FAILURE) res = false;
	delete [] parToModify.attrValues;
	return res;
}

bool ImmInterface::setServiceResultState(IMM_Util::ChangeAdmStateResultType result)
{
	bool res = true;
	int iResult = result;
	ACS_CC_ImmParameter parToModify = IMM_Util::defineParameterInt(IMM_Util::ATT_TIMEREFERENCE_RESULT_STATE,ATTR_INT32T,&iResult,1);
	if (IMM_Util::modify_OM_ImmAttr(IMM_Util::ATT_TIMEREFERENCE_OBJ,parToModify)== ACS_CC_FAILURE) res = false;
	delete [] parToModify.attrValues;
	return res;
}

int ImmInterface::getTimeReferencesConfigList(std::map<std::string,std::string> &tRefList) {

	std::vector<std::string> refList;
	if (IMM_Util::getClassObjectsList(IMM_Util::ATT_SCXTIMEREFERENCE_SCX_NW_OBJA,refList)) {

		string ref_dn("");
		string rdn_ref("");
		string name("");
		string ipAddress("");


		//loop of instances
		for (unsigned i=0; i<refList.size(); i++)
		{
			ref_dn.clear();
			rdn_ref.clear();
			name.clear();
			ipAddress.clear();

			ref_dn = refList[i].c_str();
				cout << "dn of ref: "<< ref_dn << endl;
				IMM_Util::getImmAttributeString(ref_dn,IMM_Util::ATT_SCXTIMEREFERENCE_IP,ipAddress);
				name = IMM_Util::getIdValueFromRdn(ref_dn);
				tRefList.insert ( pair<string,string>(ipAddress, name) );
		}
	} else {
		cout << "Failed to get Time References Configuration" << std::endl;
		return 1;
	}
	return 0;
}
