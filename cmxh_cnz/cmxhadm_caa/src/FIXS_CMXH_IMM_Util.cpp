/*
 * FIXS_CMXH_IMM_Util.cpp
 *
 */
#include "FIXS_CMXH_IMM_Util.h"

bool IMM_Util::getImmAttributeString (std::string object, std::string attribute, std::string &value)
{
	bool res = true;
	ACS_CC_ReturnType result;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{
		ACS_CC_ImmParameter Param;
		//Param.ACS_APGCC_IMMFreeMemory(1);
		char *name_attrPath = const_cast<char*>(attribute.c_str());
		Param.attrName = name_attrPath;
		result = omHandler.getAttribute(object.c_str(), &Param );
		if ( result != ACS_CC_SUCCESS ){	cout << "ERROR: Param " << attribute.c_str()<<" FAILURE!!!\n"; res = false; }
		else value = (char*)Param.attrValues[0];

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
			res = false;
		}
	}

	return res;
}


bool IMM_Util::getImmAttributeInt(std::string object, std::string attribute, int &value)
{
 	ACS_CC_ReturnType result;
 	OmHandler omHandler;
 	bool res = true;
 	result = omHandler.Init();
 	if (result != ACS_CC_SUCCESS)
 	{
 		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
 		res = false;
 	}
 	else
 	{
 		ACS_CC_ImmParameter Param;
 		//Param.ACS_APGCC_IMMFreeMemory(1);
 		char *name_attrPath = const_cast<char*>(attribute.c_str());
 		Param.attrName = name_attrPath;
 		result = omHandler.getAttribute(object.c_str(), &Param );
 		if ( result != ACS_CC_SUCCESS ){	cout << "ERROR: Param " << attribute.c_str()<<" FAILURE!!!\n"; res = false; }
 		else value = (*(int*)Param.attrValues[0]);

 		result = omHandler.Finalize();
 		if (result != ACS_CC_SUCCESS)
 		{
 			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
 			res = false;
 		}
 	}
 	return res;
}

bool IMM_Util::fetchDn(std::vector<std::string> &dn_list)
{
	OmHandler omHandler;
	ACS_CC_ReturnType rCode;
	bool res = true;
	rCode = omHandler.Init();
	if (rCode != ACS_CC_SUCCESS){
   		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}else{
		rCode = omHandler.getClassInstances(classVlanParameter, dn_list);
		if (rCode != ACS_CC_SUCCESS){
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error getClassInstances()" << std::endl;
			res = false;
		}
		rCode = omHandler.Finalize();
		if (rCode != ACS_CC_SUCCESS){
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}	
	}
	return res;
}

bool IMM_Util::modifyAttribute(std::string dn, ACE_UINT32 id)
{
	OmHandler omHandler;
	ACS_CC_ReturnType rCode;
	bool res = true;
	rCode = omHandler.Init();
	if (rCode != ACS_CC_SUCCESS){
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}else{
		char attrVlanId[] = "VlanId";
		ACS_CC_ImmParameter attribute_id;
		void *attr_value[1] = {const_cast<void *>(reinterpret_cast<void *>(&id))};
		attribute_id.attrName = attrVlanId;
		attribute_id.attrType = ATTR_INT32T;
		attribute_id.attrValuesNum = 1;
		attribute_id.attrValues = attr_value;

		rCode = omHandler.modifyAttribute(dn.c_str(), &attribute_id);
		if (rCode != ACS_CC_SUCCESS){
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error modifyAttribute()" << std::endl;
			res=false;
		}	
		rCode = omHandler.Finalize();
		if (rCode != ACS_CC_SUCCESS){
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}
	return res;
}

