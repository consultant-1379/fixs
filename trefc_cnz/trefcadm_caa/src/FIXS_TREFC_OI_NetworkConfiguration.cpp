/*
 * FIXS_TREFC_OI_NetworkConfiguration.cpp
 *
 *  Created on: Jan 3, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_OI_NetworkConfiguration.h"
#include "FIXS_TREFC_Logger.h"


FIXS_TREFC_OI_NetworkConfiguration* FIXS_TREFC_OI_NetworkConfiguration::m_instance = 0;

FIXS_TREFC_OI_NetworkConfiguration* FIXS_TREFC_OI_NetworkConfiguration::getInstance ()
{
	//Lock lock;
	if (m_instance == 0)
	{
		m_instance = new FIXS_TREFC_OI_NetworkConfiguration();
	}
	return m_instance;

}

void FIXS_TREFC_OI_NetworkConfiguration::finalize () {
	if (m_instance) {
		delete(m_instance);
		m_instance = NULL;
	}
}



/*============================================================================
	ROUTINE: FIXS_TREFC_OI_NetworkConfiguration
 ============================================================================ */
FIXS_TREFC_OI_NetworkConfiguration::FIXS_TREFC_OI_NetworkConfiguration() :  acs_apgcc_objectimplementerinterface_V2(IMM_Util::IMPLEMENTER_NETWORKCONFIGURATION_SWITCH),
m_ImmClassName(IMM_Util::classNetworkConfigurationSwitch),m_lock()
{
	_trace = new ACS_TRA_trace("FIXS_TREFC_OI_NetworkConfiguration");
	//	FIXS_TREFC_OI_NetworkConfigurationTrace = new ACS_TRA_trace("FIXS_TREFC_OI_NetworkConfiguration","C400");
}

/*============================================================================
	ROUTINE: ~FIXS_TREFC_OI_NetworkConfiguration
 ============================================================================ */
FIXS_TREFC_OI_NetworkConfiguration::~FIXS_TREFC_OI_NetworkConfiguration()
{
	if (_trace)
		delete(_trace);
}


ACS_CC_ReturnType FIXS_TREFC_OI_NetworkConfiguration::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " class Name: " << className << endl;
	cout << " parent Name: " << parentname << endl;
	//UNUSED(attr);
	ACS_CC_ReturnType result = ACS_CC_FAILURE;

	string netmask("");
	string ipAddress("");
	string gateway("");
	int slot = -1, switchSlot = -1;

	if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ, parentname)) 
	{
		slot = 0;
	}
	else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ, parentname))
	{
		slot = 25;
	}
	else if(0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ, parentname))
		switchSlot = 0;
	else if(0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ, parentname))
		switchSlot =25;
	else
		return result;

	if(( 0 == ACE_OS::strcmp(m_ImmClassName.c_str(), className ) ) || (0 == ACE_OS::strcmp(IMM_Util::classNetworkConfiguration, className ) ))
	{
		for(size_t idx = 0; attr[idx] != NULL ; idx++)
		{
			if( 0 == ACE_OS::strcmp(IMM_Util::ATT_NETWORKCONFIGURATION_IP, attr[idx]->attrName) )
			{
				ipAddress = reinterpret_cast<char *>(*attr[idx]->attrValues);
			}
			else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_NETWORKCONFIGURATION_NETMASK, attr[idx]->attrName) )
			{
				netmask = reinterpret_cast<char *>(*attr[idx]->attrValues);
			}
			else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_NETWORKCONFIGURATION_GATEWAY, attr[idx]->attrName) )
			{
				gateway = reinterpret_cast<char *>(*attr[idx]->attrValues);
			}
		}

	}

	//Check if is a valid IP address
	unsigned long int ip = 0;
	if (TREFC_Util::checkIPAddresses(ipAddress.c_str(),ip) == false) {
		cout << "Invalid IP address" << std::endl;
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Network Configuration creation rejected: Invalid IP address = %s", ipAddress.c_str());

		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
		int err = TREFC_Util::WRONG_IP_ADDRESS;
		this->setExitCode(err,TREFC_Util::getErrrorText(err));
		return ACS_CC_FAILURE;
	}


	//Check if is a valid NetMask
	ip = 0;
	if (TREFC_Util::checkIPAddresses(netmask.c_str(),ip) == false) {
		cout << "Invalid IP address" << std::endl;
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Network Configuration creation rejected: Invalid Netmask = %s", netmask.c_str());

		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
		int err = TREFC_Util::INVALID_NETMASK;
		this->setExitCode(err,TREFC_Util::getErrrorText(err));
		return ACS_CC_FAILURE;
	}

	//Check if is a valid Gateway
	ip = 0;
	if (TREFC_Util::checkIPAddresses(gateway.c_str(),ip) == false) {
		cout << "Invalid IP address" << std::endl;
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Network Configuration creation rejected: Invalid Gateway address = %s", gateway.c_str());

		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
		int err = TREFC_Util::INVALID_GATEWAY;
		this->setExitCode(err,TREFC_Util::getErrrorText(err));

		return ACS_CC_FAILURE;
	}

	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	//Add element to transaction


	ExtIpSwitch net;
	net.extIp = ipAddress;
	net.netmask = netmask;
	net.defaultGateway = gateway;
	if(environment==TREFC_Util::SMX)
		net.slot = switchSlot;
	else
		net.slot = slot;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_NetworkConfiguration] Creating Network Configuration %d:"
				"\nIP Address	= %s"
				"\nGateway		= %s"
				"\nNetmask		= %s", slot, ipAddress.c_str(), gateway.c_str(), netmask.c_str());
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
	}
	
//	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	
	if(environment!=TREFC_Util::SMX)// only in SCX environment
	{
		if((slot != -1) || (switchSlot != -1))
		{
			ACS_APGCC_CcbHandle ccbHandleVal;
			ACS_APGCC_AdminOwnerHandle adminOwnwrHandlerVal;
			// The vector of attributes
			vector<ACS_CC_ValuesDefinitionType> AttrList;

			ACS_CC_ReturnType retVal = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnwrHandlerVal);
			if (retVal != ACS_CC_SUCCESS)
			{
				cout << " getCcbAugmentationInitialize::ACS_CC_FAILURE " << endl;
				return ACS_CC_FAILURE;
			}
			else
			{
				cout << " getCcbAugmentationInitialize::ACS_CC_SUCCESS " << endl;
			}

			std::string tempRdn;
			if(slot==0)
			{
				std::string tempRdn0(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ);
				tempRdn = tempRdn0;
			}
			else if(slot == 25)
			{
				std::string tempRdn25(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ);
				tempRdn = tempRdn25;	
			}
			else if(switchSlot==0)
			{
					std::string tempSwitchRdn0(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ);
					tempRdn = tempSwitchRdn0;
			}
			else if(switchSlot == 25)
			{
					std::string tempSwitchRdn25(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ);
					tempRdn = tempSwitchRdn25;
			}

			ACS_CC_ValuesDefinitionType attrRdn; 
			char attrdn[]= "id";
			attrRdn.attrName = attrdn;
			attrRdn.attrType = ATTR_STRINGT;
			attrRdn.attrValuesNum = 1;
			char* rdnValue = const_cast<char *>("id=networkConfiguration_0");
			attrRdn.attrValues = new void*[1];
			attrRdn.attrValues[0] = reinterpret_cast<void*>(rdnValue); 

			ACS_CC_ValuesDefinitionType ipExtParam; 
			char * tempExtIp =const_cast<char *>(net.extIp.c_str());
			ipExtParam.attrName = const_cast<char *>(IMM_Util::ATT_NETWORKCONFIGURATION_IP);
			ipExtParam.attrType = ATTR_STRINGT;
			ipExtParam.attrValuesNum = 1;
			ipExtParam.attrValues = new void*[ipExtParam.attrValuesNum];
			ipExtParam.attrValues[0] = reinterpret_cast<void *>(tempExtIp); 

			ACS_CC_ValuesDefinitionType netmaskExtParam; 
			char * tempNetmask =const_cast<char *>(net.netmask.c_str());
					netmaskExtParam.attrName = const_cast<char *>(IMM_Util::ATT_NETWORKCONFIGURATION_NETMASK);
					netmaskExtParam.attrType = ATTR_STRINGT;
					netmaskExtParam.attrValuesNum = 1;
					netmaskExtParam.attrValues = new void*[netmaskExtParam.attrValuesNum];
					netmaskExtParam.attrValues[0] = reinterpret_cast<void *>(tempNetmask); 

			ACS_CC_ValuesDefinitionType gatewayExtParam; 
			char * tempGateway =const_cast<char *>(net.defaultGateway.c_str());
					gatewayExtParam.attrName = const_cast<char *>(IMM_Util::ATT_NETWORKCONFIGURATION_GATEWAY);
					gatewayExtParam.attrType = ATTR_STRINGT;
					gatewayExtParam.attrValuesNum = 1;
					gatewayExtParam.attrValues = new void*[gatewayExtParam.attrValuesNum];
					gatewayExtParam.attrValues[0] = reinterpret_cast<void *>(tempGateway);  

			AttrList.push_back(attrRdn);
			AttrList.push_back(ipExtParam);
			AttrList.push_back(netmaskExtParam);
			AttrList.push_back(gatewayExtParam);		
			
			char * class_name =NULL;
			if(slot != -1)
				class_name = const_cast<char *>(IMM_Util::classNetworkConfigurationSwitch);
			else 
			{
				class_name = const_cast<char *>(IMM_Util::classNetworkConfiguration);
			}
		
			cout << " createObjectAugmentCcb::before calling " << class_name<<endl;
			cout << " createObjectAugmentCcb::before calling " << tempRdn.c_str()<<endl;
			retVal = createObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, class_name, tempRdn.c_str(), AttrList);
					if (retVal != ACS_CC_SUCCESS){
							cout << " createObjectAugmentCcb::ACS_CC_FAILURE " << retVal << endl;
				delete [] attrRdn.attrValues;
				delete [] ipExtParam.attrValues;
				delete [] netmaskExtParam.attrValues;
				delete [] gatewayExtParam.attrValues;
				AttrList.clear();
				return ACS_CC_FAILURE;
			}
			else 
			{
				//prepare to apply ccb
				retVal = applyAugmentCcb (ccbHandleVal);
				if (retVal != ACS_CC_SUCCESS){
					cout << " applyAugmentCcb::ACS_CC_FAILURE " << endl;
					delete [] attrRdn.attrValues;
					delete [] ipExtParam.attrValues;
					delete [] netmaskExtParam.attrValues;
					delete [] gatewayExtParam.attrValues;
					AttrList.clear();
					return ACS_CC_FAILURE;
				}else{
					cout << " applyAugmentCcb::ACS_CC_SUCCESS " << endl;
				}
			}
			delete [] attrRdn.attrValues;
			delete [] ipExtParam.attrValues;
			delete [] netmaskExtParam.attrValues;
			delete [] gatewayExtParam.attrValues;
			AttrList.clear();
					
		}

	}
	addToTransaction(ccbId, net, IMM_Util::CREATE);

	result = ACS_CC_SUCCESS;

	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_NetworkConfiguration::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectDeleteCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	ACS_CC_ReturnType result = ACS_CC_FAILURE;
	int slot = -1,switchSlot=-1;

	std::string parentname("");
	IMM_Util::getDnParent(objName, parentname);
	std::string dnActuallyToDelete;
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	

	if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ, parentname.c_str()) )
	{
		std::string tempdn(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ);
		tempdn = "id=networkConfiguration_0," + tempdn;
		dnActuallyToDelete = tempdn;
		slot = 0;
	}
	else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ, parentname.c_str()) )
	{
		std::string tempdn(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ);
		tempdn = "id=networkConfiguration_0," + tempdn;
                dnActuallyToDelete = tempdn;
		slot = 25;
	}
	else if(0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ, parentname.c_str()))
	{
		std::string tempdn(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ);
		tempdn = "id=networkConfiguration_0," + tempdn;
                dnActuallyToDelete = tempdn;
                switchSlot = 0;
	}
        else if(0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ, parentname.c_str()))
	{
		std::string tempdn(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ);
		tempdn = "id=networkConfiguration_0," + tempdn;
                dnActuallyToDelete = tempdn;
                switchSlot =25;
	}
        else
		return result;

	
	
	int tempslot= -1;
	if(slot != -1)
		tempslot=slot;
	else
		tempslot= switchSlot;

	
	if(environment!=TREFC_Util::SMX)// only in SCX environment
	{
		ACS_APGCC_CcbHandle ccbHandleVal;
		ACS_APGCC_AdminOwnerHandle adminOwnwrHandlerVal;
		ACS_CC_ReturnType retVal = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnwrHandlerVal );
		if (retVal != ACS_CC_SUCCESS){
			cout << " getCcbAugmentationInitialize::ACS_CC_FAILURE " << endl;
			return ACS_CC_FAILURE;
		}else{
			cout << " getCcbAugmentationInitialize_SUCCESS" << endl;
			cout << "dnActuallyToDelete =  " << dnActuallyToDelete.c_str() << endl;
			retVal = deleteObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, dnActuallyToDelete.c_str() );
			if (retVal != ACS_CC_SUCCESS){
				cout << " deleteObjectAugmentCcb::ACS_CC_FAILURE  error code: " << getInternalLastError()<< "getInternalLastErrorText:"<<getInternalLastErrorText()<<endl;

			}else {
				/*prepare to apply ccb*/
				cout << "before  applyAugmentCcb_SUCCESS" << endl;
				retVal = applyAugmentCcb (ccbHandleVal);
				if (retVal != ACS_CC_SUCCESS){
					cout << " deleteObjectAugmentCcb::applyAugmentCcb::ACS_CC_FAILURE " << endl;
					return ACS_CC_FAILURE;
				}else{
					cout << " deleteObjectAugmentCcb::applyAugmentCcb::ACS_CC_SUCCESS " << endl;
				}
			}
		}

	}
	//Add element to transaction
	ExtIpSwitch temp;
	temp.extIp.clear();
	temp.netmask.clear();
	temp.defaultGateway.clear();
	temp.slot = tempslot;

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_NetworkConfiguration] Removing Network Configuration %d."
				, slot);
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
	}

	addToTransaction(ccbId, temp, IMM_Util::DELETE);

	result = ACS_CC_SUCCESS;

	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_NetworkConfiguration::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectModifyCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	cout << " MODIFYCallback " << endl;
	cout << endl;


	ACS_CC_ReturnType result = ACS_CC_FAILURE;
	bool modified = false;

	ExtIpSwitch modExtIp;

	std::string parentname("");
	IMM_Util::getDnParent(objName, parentname);
	int slot = -1, switchSlot = -1;

	if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ, parentname.c_str()) )
	{
		FIXS_TREFC_Manager::getInstance()->getExtIpAddrConfiguration(modExtIp, 0);
		slot=0;
	}
	else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ, parentname.c_str()) )
	{
		FIXS_TREFC_Manager::getInstance()->getExtIpAddrConfiguration(modExtIp, 25);
		slot=25;
	}
	else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ, parentname.c_str()) )
	{
		FIXS_TREFC_Manager::getInstance()->getExtIpAddrConfiguration(modExtIp, 0);
		switchSlot=0;
	}
	else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ, parentname.c_str()) )
	{
		FIXS_TREFC_Manager::getInstance()->getExtIpAddrConfiguration(modExtIp, 25);
		switchSlot=25;
	}
	else
		return result;

	std::string tempSwitchObj;

	if(slot == 0)
	{
		std::string tempScx0(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ);
		tempScx0 = "id=networkConfiguration_0," + tempScx0;
		tempSwitchObj = tempScx0;
	}
	else if(slot == 25)
	{
		std::string tempScx25(IMM_Util::ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ);
		tempScx25 = "id=networkConfiguration_0," + tempScx25;
		tempSwitchObj = tempScx25;
	}
	else if(switchSlot == 0)
	{
        std::string tempSwitch0(IMM_Util::ATT_SCXTIMEREFERENCE_SCXA_OBJ);
		tempSwitch0 = "id=networkConfiguration_0," + tempSwitch0;
        tempSwitchObj = tempSwitch0;
    }
    else if(switchSlot == 25)
    {
        std::string tempSwitch25(IMM_Util::ATT_SCXTIMEREFERENCE_SCXB_OBJ);
		tempSwitch25 = "id=networkConfiguration_0," + tempSwitch25;
        tempSwitchObj = tempSwitch25;
    }

	cout << " tempSwitchObj = " << tempSwitchObj.c_str() << endl;


	for(size_t idx = 0; attrMods[idx] != NULL ; idx++)
	{
		if( ACS_APGCC_ATTR_VALUES_REPLACE == attrMods[idx]->modType )
		{
			if( 0 == ACE_OS::strcmp(IMM_Util::ATT_NETWORKCONFIGURATION_IP, attrMods[idx]->modAttr.attrName) )
			{
				modExtIp.extIp = reinterpret_cast<char *>(*attrMods[idx]->modAttr.attrValues);
				modified = true;
			}
			else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_NETWORKCONFIGURATION_NETMASK, attrMods[idx]->modAttr.attrName) )
			{
				modExtIp.netmask = reinterpret_cast<char *>(*attrMods[idx]->modAttr.attrValues);
				modified = true;
			}
			else if( 0 == ACE_OS::strcmp(IMM_Util::ATT_NETWORKCONFIGURATION_GATEWAY, attrMods[idx]->modAttr.attrName) )
			{
				modExtIp.defaultGateway = reinterpret_cast<char *>(*attrMods[idx]->modAttr.attrValues);
				modified = true;
			}
		}
	}

	//Add element to transaction
	if (modified) {
		{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_NetworkConfiguration] Modifying Network Configuration %d:"
				"\nIP Address	= %s"
				"\nGateway		= %s"
				"\nNetmask		= %s", modExtIp.slot, modExtIp.extIp.c_str(), modExtIp.defaultGateway.c_str(), modExtIp.netmask.c_str());
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
		}

		//Check if is a valid IP address
		unsigned long int ip = 0;
		if (TREFC_Util::checkIPAddresses(modExtIp.extIp.c_str(),ip) == false) {
			cout << "Invalid IP address" << std::endl;
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Network Configuration creation rejected: Invalid IP address = %s", modExtIp.extIp.c_str());

			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
			int err = TREFC_Util::WRONG_IP_ADDRESS;
			this->setExitCode(err,TREFC_Util::getErrrorText(err));
			return ACS_CC_FAILURE;
		}


		//Check if is a valid NetMask
		ip = 0;
		if (TREFC_Util::checkIPAddresses(modExtIp.netmask.c_str(),ip) == false) {
			cout << "Invalid IP address" << std::endl;
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Network Configuration creation rejected: Invalid Netmask = %s", modExtIp.netmask.c_str());

			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
			int err = TREFC_Util::INVALID_NETMASK;
			this->setExitCode(err,TREFC_Util::getErrrorText(err));
			return ACS_CC_FAILURE;
		}

		//Check if is a valid Gateway
		ip = 0;
		if (TREFC_Util::checkIPAddresses(modExtIp.defaultGateway.c_str(),ip) == false) {
			cout << "Invalid IP address" << std::endl;
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_OI_TimeServer] Network Configuration creation rejected: Invalid Gateway address = %s", modExtIp.defaultGateway.c_str());

			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr, LOG_LEVEL_ERROR);
			int err = TREFC_Util::INVALID_GATEWAY;
			this->setExitCode(err,TREFC_Util::getErrrorText(err));
			return ACS_CC_FAILURE;
		}

	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();
	
	if(environment!=TREFC_Util::SMX)// only in SCX environment
	{
		ACS_APGCC_CcbHandle ccbHandleVal;
		ACS_APGCC_AdminOwnerHandle adminOwnwrHandlerVal;
		/*The vector of attributes*/
		vector<ACS_CC_ImmParameter> AttrList;

		ACS_CC_ReturnType retVal = getCcbAugmentationInitialize(oiHandle, ccbId, &ccbHandleVal, &adminOwnwrHandlerVal);
		if (retVal != ACS_CC_SUCCESS)
		{
			cout << " getCcbAugmentationInitialize::ACS_CC_FAILURE " << endl;
			return ACS_CC_FAILURE;
		}
		else
		{
			cout << " getCcbAugmentationInitialize::ACS_CC_SUCCESS " << endl;
		}

		char * tempExtIp =const_cast<char *>(modExtIp.extIp.c_str());
		ACS_CC_ImmParameter ipExtParam = IMM_Util::defineParameterString(IMM_Util::ATT_NETWORKCONFIGURATION_IP,ATTR_STRINGT,tempExtIp,1);;

		char * tempNetmask =const_cast<char *>(modExtIp.netmask.c_str());
		ACS_CC_ImmParameter netmaskExtParam= IMM_Util::defineParameterString(IMM_Util::ATT_NETWORKCONFIGURATION_NETMASK,ATTR_STRINGT,tempNetmask,1);

		char * tempGateway =const_cast<char *>(modExtIp.defaultGateway.c_str());
		ACS_CC_ImmParameter gatewayExtParam= IMM_Util::defineParameterString(IMM_Util::ATT_NETWORKCONFIGURATION_GATEWAY,ATTR_STRINGT,tempGateway,1);

	
		cout << " modifyObjectAugmentCcb::before calling " << tempSwitchObj.c_str() <<endl;

		retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, tempSwitchObj.c_str(), &ipExtParam);
		if (retVal == ACS_CC_SUCCESS)
		{
			retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, tempSwitchObj.c_str(), &netmaskExtParam);
			if (retVal == ACS_CC_SUCCESS)
			{
				retVal = modifyObjectAugmentCcb(ccbHandleVal, adminOwnwrHandlerVal, tempSwitchObj.c_str(), &gatewayExtParam);
				if (retVal == ACS_CC_SUCCESS)
				{
					retVal = applyAugmentCcb (ccbHandleVal);

					if (retVal != ACS_CC_SUCCESS){
						cout << " applyAugmentCcb::ACS_CC_FAILURE " << endl;
						delete [] gatewayExtParam.attrValues;
						delete [] netmaskExtParam.attrValues;
						delete [] ipExtParam.attrValues;
						return ACS_CC_FAILURE;
					}else{
						cout << " applyAugmentCcb::ACS_CC_SUCCESS " << endl;
						cout << " modifyObjectAugmentCcb::SUCCESS value for object =  " << tempSwitchObj << endl;
					}
				}
			}
			else
			{
				cout << " modifyObjectAugmentCcb::ACS_CC_FAILURE " << endl;
				delete [] gatewayExtParam.attrValues;
				delete [] netmaskExtParam.attrValues;
				delete [] ipExtParam.attrValues;
				return ACS_CC_FAILURE;
			}
		}
		else
		{
			cout << " modifyObjectAugmentCcb::ACS_CC_FAILURE " << endl;
			delete [] gatewayExtParam.attrValues;
			delete [] netmaskExtParam.attrValues;
			delete [] ipExtParam.attrValues;
			return ACS_CC_FAILURE;			
		}

		delete [] gatewayExtParam.attrValues;
		delete [] netmaskExtParam.attrValues;
		delete [] ipExtParam.attrValues;
	}
		addToTransaction(ccbId, modExtIp, IMM_Util::MODIFY);
	}

	result = ACS_CC_SUCCESS;

	return result;
}

ACS_CC_ReturnType FIXS_TREFC_OI_NetworkConfiguration::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invoked           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " COMPLETECallback " << endl;
	cout << endl;

	ACS_CC_ReturnType result = ACS_CC_FAILURE;

	result = validateTransaction(ccbId);

	return result;
}

void FIXS_TREFC_OI_NetworkConfiguration::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbAbortCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " ABORTCallback " << endl;
	cout << endl;

	//Abort Transaction
	abortTransaction(ccbId);
}

void FIXS_TREFC_OI_NetworkConfiguration::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
{
	cout << "---------------------------------------------------" << endl;
	cout << "          CcbApplyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " APPLYCallback " << endl;
	cout << endl;


	//FIXS_TREFC_Manager* objTrefcManager = FIXS_TREFC_Manager::getInstance();


	commitTransaction(ccbId);

	//	TRACE(FIXS_TREFC_OI_NetworkConfigurationTrace, "%s", "Entering apply(...)");

}

ACS_CC_ReturnType FIXS_TREFC_OI_NetworkConfiguration::updateRuntime(const char* p_objName, const char* p_attrName)
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


void FIXS_TREFC_OI_NetworkConfiguration::adminOperationCallback(ACS_APGCC_OiHandle oiHandle,
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

	/*start*/
	cout << endl;

	int dim=0;
	int i=0;
	while(paramList[i]){
		i++;
		dim++;
	}

	i = 0;
	while( paramList[i] ){
		switch ( paramList[i]->attrType ) {
		case ATTR_INT32T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<int *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_UINT32T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<unsigned int *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_INT64T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<long long *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_UINT64T:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<unsigned long long *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_FLOATT:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<float *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_DOUBLET:
			cout << " paramListName: " << paramList[i]->attrName << "\t value: " <<*reinterpret_cast<double *>(paramList[i]->attrValues) << endl;
			break;
		case ATTR_NAMET:
		{
			cout << " attrName: " << paramList[i]->attrName << "\t value: " << reinterpret_cast<char *>(paramList[i]->attrValues) << endl;

		}
		break;
		case ATTR_STRINGT:
			cout << " attrName: " << paramList[i]->attrName << "\t value: " << reinterpret_cast<char *>(paramList[i]->attrValues) << endl;
			break;
		default:
			break;

		}
		i++;
	}

	cout << " Implementing Admin Operation with ID = " << operationId << endl;

	SaAisErrorT retVal = SA_AIS_OK;



	//############## ACTION 1 ----

	if (operationId == 1)
	{
		cout << " ----------- ACTION 1 ------------------\n  " << endl;
		retVal = SA_AIS_OK;
		this->adminOperationResult( oiHandle , invocation, retVal );
		return;

	}

}

void FIXS_TREFC_OI_NetworkConfiguration::addToTransaction (ACS_APGCC_CcbId ccbid, ExtIpSwitch extIp, IMM_Util::Action action) {

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	std::map <ACS_APGCC_CcbId, std::vector <ExtIpAction> >::iterator it;

	ExtIpAction temp;
	temp.element = extIp;
	temp.action = action;

	it=transaction.find(ccbid);
	if (it != transaction.end()) {
		it->second.push_back(temp);
	} else {
		std::vector <ExtIpAction> temp_vector;
		temp_vector.clear();
		temp_vector.push_back(temp);
		transaction.insert( pair <ACS_APGCC_CcbId, std::vector <ExtIpAction> >(ccbid,temp_vector));
	}

}


void FIXS_TREFC_OI_NetworkConfiguration::commitTransaction(ACS_APGCC_CcbId ccbId) {
	//Commit Transaction

	FIXS_TREFC_Manager* objTrefcManager = FIXS_TREFC_Manager::getInstance();

	std::map <ACS_APGCC_CcbId, std::vector <ExtIpAction> >::iterator it;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	it=transaction.find(ccbId);
	if (it != transaction.end()) {
		int size = it->second.size();
		for (int i=0; i< size; i++) {
			ExtIpAction temp = it->second[i];
			if (temp.action == IMM_Util::CREATE || temp.action == IMM_Util::MODIFY) {
				objTrefcManager->addScxExtIp(temp.element.extIp,temp.element.netmask,temp.element.defaultGateway,temp.element.slot);
			}
			else if (temp.action == IMM_Util::DELETE) {
				objTrefcManager->rmScxExtIp(temp.element.slot);
			}
		}
		transaction.erase(it);
	}
}

void FIXS_TREFC_OI_NetworkConfiguration::abortTransaction(ACS_APGCC_CcbId ccbId) {
	//Abort Transaction
	std::map <ACS_APGCC_CcbId, std::vector<ExtIpAction> >::iterator it;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	it=transaction.find(ccbId);
	if (it != transaction.end()) {
		transaction.erase(it);

	}
}

ACS_CC_ReturnType FIXS_TREFC_OI_NetworkConfiguration::validateTransaction(ACS_APGCC_CcbId ccbId) {

	FIXS_TREFC_Manager* objTrefcManager = FIXS_TREFC_Manager::getInstance();

	std::map <ACS_APGCC_CcbId, std::vector <ExtIpAction> >::iterator it;

	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_lock);

	it=transaction.find(ccbId);
	int res = 0;

	if (it != transaction.end()) {
		int size = it->second.size();
		for (int i=0; i< size; i++) {
			if (it->second[i].action == IMM_Util::DELETE) {
				res=objTrefcManager->validateScxExtIpAction(IMM_Util::DELETE);
				if (res != EXECUTED) {
					this->setExitCode(res,TREFC_Util::getErrrorText(res));
					return ACS_CC_FAILURE;
				}
			}

		}
	}
	return ACS_CC_SUCCESS;
}

