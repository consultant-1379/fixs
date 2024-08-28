/*
 * FIXS_CMXH_ImmInterface.cpp
 *
 */

#include "FIXS_CMXH_ImmInterface.h"

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
		case ACS_CS_API_CommonBasedArchitecture::SMX:
			env = SMX;
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

std::string IMM_Interface::getDnSnmpPolling()
{
	std::string dn_out = "ComponentMainSwitchHandlerSnmpPollingId=1";

	return dn_out;
}


bool IMM_Interface::getSnmpPollingTime(int &polling_time)
{
	bool res = true;
	std::string dn_obj = getDnSnmpPolling();
	if (!IMM_Util::getImmAttributeInt(dn_obj,IMM_Util::ATT_SNMP_POLLING_TIME,polling_time))
	{
		cout << __FUNCTION__<< " ERROR: IMM attribute reading Error!!!\n";
		res =false;
	}
	else
		res = true;
	return res;
}

bool IMM_Interface::getVlanParameter(std::string dn_obj, vlanParameterStruct &data, int version)
{
	bool res = true;
	if(!IMM_Util::getImmAttributeString(dn_obj,IMM_Util::ATT_VLAN_NAME,data.name))
	{
		cout << __FUNCTION__<< " ERROR: IMM attribute Vlan Name reading Error!!!\n";
		res =false;
		return res;
	}

	if (!IMM_Util::getImmAttributeInt(dn_obj,IMM_Util::ATT_VLAN_ID,data.id))
	{
		cout << __FUNCTION__<< " ERROR: IMM attribute Vlan Id reading Error!!!\n";
		res =false;
		return res;
	}

	if (!IMM_Util::getImmAttributeInt(dn_obj,IMM_Util::ATT_VLAN_PRIORITY,data.priority))
	{
		cout << __FUNCTION__<< " ERROR: IMM attribute Vlan Priority reading Error!!!\n";
		res =false;
		return res;
	}

	if(version==4)
	{
		if(!IMM_Util::getImmAttributeString(dn_obj,IMM_Util::ATT_VLAN_PORTMASK_4,data.portMask))
		{
			cout << __FUNCTION__<< " ERROR: IMM attribute vlan Portmask for sw version 4 reading Error!!!\n";
			res =false;
			return res;

		}
		if(!IMM_Util::getImmAttributeString(dn_obj,IMM_Util::ATT_VLAN_UNTAGGED_PORTMASK_4,data.untaggedPortMask))
		{
			cout << __FUNCTION__<< " ERROR: IMM attribute vlan Untagged PortMask for sw version 4  reading Error!!!\n";
			res =false;
			return res;
		}
	}
	else if(version==2)
	{
		if(!IMM_Util::getImmAttributeString(dn_obj,IMM_Util::ATT_VLAN_PORTMASK,data.portMask))
		{
			cout << __FUNCTION__<< " ERROR: IMM attribute Vlan Portmask for sw version 2 reading Error!!!\n";
			res =false;
			return res;
		}
		if(!IMM_Util::getImmAttributeString(dn_obj,IMM_Util::ATT_VLAN_UNTAGGED_PORTMASK,data.untaggedPortMask))
		{
			cout << __FUNCTION__<< " ERROR: IMM attribute Vlan Untagged PortMask for sw version 2 reading Error!!!\n";
			res =false;
			return res;
		}
	}
	else
	{
		cout << __FUNCTION__<< "Invalid Software Revision" << endl;
		res=false;
	}
	return res;

}

bool IMM_Interface::modifyObject()
{	

	bool res = true;
	std::vector<std::string> dn_list;
	vlanParameterStruct tmpData;
	if (!IMM_Util::fetchDn(dn_list)) {
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error fetchDn()" << std::endl;
		res = false;
		return res;
	}

	ACE_INT32 id = CMXH_Util::getDRBDVlan();
	if (id == -1){
		// DRBD vlan does not exist
		res = false;
		return res;
	}

	for (std::vector<std::string>::iterator it = dn_list.begin(); it != dn_list.end(); it++ ) {

		int vlanId;

		// check if DRBD vlan is in IMM
		if(IMM_Util::getImmAttributeInt(it->c_str(),IMM_Util::ATT_VLAN_ID,vlanId))
		{

			if(vlanId == id) { break; }

			// check if the DRBD vlan fetched from the IMM is default one
			if ( vlanId == IMM_Util::DEFAULT_DRBD_ID) {

				if (!IMM_Util::modifyAttribute(it->c_str(), id)){
					cout << __FUNCTION__<< " ERROR: IMM modifyAttribute !\n";
					res = false;
				}
				break;
			}	
		}	
	}
	return res;
}	

