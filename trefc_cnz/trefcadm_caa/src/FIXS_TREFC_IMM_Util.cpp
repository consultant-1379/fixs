/*
 * FIXS_TREFC_IMM_Util.cpp
 *
 *  Created on: Oct 5, 2011
 *      Author: eanform
 */
#include "FIXS_TREFC_IMM_Util.h"

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
		else if (Param.attrValuesNum > 0)
			value = (char*)Param.attrValues[0];
		else
			res = false;

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
 		else if (Param.attrValuesNum > 0)
 			value = (*(int*)Param.attrValues[0]);
 		else
 			res = false;

 		result = omHandler.Finalize();
 		if (result != ACS_CC_SUCCESS)
 		{
 			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
 			res = false;
 		}
 	}

 	return res;
}

bool IMM_Util::getChildrenObject(std::string dn, std::vector<std::string> & list )
{
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{

		result = omHandler.getChildren(dn.c_str(),ACS_APGCC_SUBLEVEL, &list);
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error getChildren" << std::endl;
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}

bool IMM_Util::getClassObjectsList(std::string className, std::vector<std::string> & list )
{
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{

		result = omHandler.getClassInstances(className.c_str(),list);
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error getClassIstance" << std::endl;
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}

bool IMM_Util::deleteImmObject(std::string dn)
{
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{

		result = omHandler.deleteObject(dn.c_str());
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error deleteObject" << std::endl;
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}

bool IMM_Util::deleteImmObject(std::string dn, ACS_APGCC_ScopeT p_scope)
{
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{

		result = omHandler.deleteObject(dn.c_str(),p_scope);
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error deleteObject" << std::endl;
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}

bool IMM_Util::getObject(std::string dn, ACS_APGCC_ImmObject *object)
{
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;
	char *nameObject =  const_cast<char*>(dn.c_str());
	object->objName = nameObject;


	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{
		result = omHandler.getObject(object);
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error getObject()" << std::endl;
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}

bool IMM_Util::getDnParent (std::string object_dn, std::string &parent)
{
	bool result = false;
	if (!object_dn.empty())
	{
		parent = ACS_APGCC::after(object_dn,",");
		result = true;
	}
	return result;
}

bool IMM_Util::getRdnAttribute (std::string object_dn, std::string &rdn)
{
	bool result = false;
	if (!object_dn.empty())
	{
		rdn = ACS_APGCC::before(object_dn,"=");
		result = true;
	}
	return result;
}

bool IMM_Util::getRdnObject (std::string object_dn, std::string &rdn)
{
	bool result = false;
	if (!object_dn.empty())
	{
		rdn = ACS_APGCC::before(object_dn,",");
		result = true;
	}
	return result;
}

ACS_CC_ValuesDefinitionType IMM_Util::defineAttributeString(const char* attributeName, ACS_CC_AttrValueType type, const char *value, unsigned int numValue)
{
	ACS_CC_ValuesDefinitionType attribute;
	char* stringValue = const_cast<char *>(value);
	attribute.attrName = const_cast<char*>(attributeName);
	attribute.attrType = type;
	attribute.attrValuesNum = numValue;

	if (attribute.attrValuesNum == 0)	attribute.attrValues = 0;
	else
	{
		attribute.attrValues=new void*[attribute.attrValuesNum];
		attribute.attrValues[0] =reinterpret_cast<void*>(stringValue);
	}

	return attribute;
}

ACS_CC_ValuesDefinitionType IMM_Util::defineAttributeInt(const char* attributeName, ACS_CC_AttrValueType type, int *value, unsigned int numValue)
{
	ACS_CC_ValuesDefinitionType attribute;

	attribute.attrName = const_cast<char*>(attributeName);
	attribute.attrType = type;
	attribute.attrValuesNum = numValue;

	if (attribute.attrValuesNum == 0)	attribute.attrValues = 0;
	else
	{
		attribute.attrValues=new void*[attribute.attrValuesNum];
		attribute.attrValues[0] =reinterpret_cast<void*>(value);
	}


	return attribute;
}

bool IMM_Util::createImmObject(const char *classImplementer, const char *p_className, const char *parent, vector<ACS_CC_ValuesDefinitionType> AttrList)
{
	/*************************************************************************************************
	* This method has to be used within a thread and an implementer previously activated on the class *
	*************************************************************************************************/

	bool res = false;
	ACS_CC_ReturnType result;
	OmHandler omHandler;
	ACS_APGCC_OiHandler oi_Handler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{
		MyImplementer implementer_1(classImplementer);

		//add implementer to the class
		result = oi_Handler.addClassImpl(&implementer_1, p_className);
		if (result != ACS_CC_SUCCESS)	cout<<" ERROR set implementer............."<< endl;

		ACE_TP_Reactor *tp_reactor_impl = new ACE_TP_Reactor;
		ACE_Reactor *reactor = new ACE_Reactor(tp_reactor_impl) ;

		reactor->open(1);

		MyThread_IMM thread_1(&implementer_1, reactor);

		thread_1.activate();
		sleep(1);


		//char* n_Parent = const_cast<char*>(parent);

		result = omHandler.createObject(p_className, parent, AttrList);
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error createObject()" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Into Class : "<<  p_className << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "From Parent Object: "<<  parent << std::endl;
			res = false;
		}
		else res = true;


		//remove implementer
		result = oi_Handler.removeClassImpl(&implementer_1, p_className);
		if (result != ACS_CC_SUCCESS)	cout<<" ERROR remove implementer............."<< endl;

		implementer_1.reactor()->end_reactor_event_loop();
		delete reactor;
		delete tp_reactor_impl;

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}

bool IMM_Util::modify_OI_ImmAttrSTRING(const char *implementerName, const char *dn, const char *attr, const char *value)
{
	bool res = true;
	ACS_CC_ReturnType result;
	ACS_APGCC_OiHandler oi_Handler;

	MyImplementer implementer_1(dn, implementerName, ACS_APGCC_ONE );

	result = oi_Handler.addObjectImpl(&implementer_1);
	if (result == ACS_CC_SUCCESS)
	{
		ACE_TP_Reactor *tp_reactor_impl = new ACE_TP_Reactor;
		ACE_Reactor *reactor = new ACE_Reactor(tp_reactor_impl) ;

		reactor->open(1);

		MyThread_IMM thread_1(&implementer_1, reactor);

		thread_1.activate();
		sleep(1);

		ACS_CC_ImmParameter parToModify = defineParameterString(attr,ATTR_STRINGT,value,1);

		result = implementer_1.modifyRuntimeObj(dn,&parToModify);
		if (result != ACS_CC_SUCCESS) res = false;

		delete[] parToModify.attrValues;

		result = oi_Handler.removeObjectImpl(&implementer_1);

		implementer_1.reactor()->end_reactor_event_loop();
		delete reactor;
		delete tp_reactor_impl;
	}

	return res;
}

bool IMM_Util::modify_OI_ImmAttrINT(const char *implementerName, const char *dn, const char *attr, int value)
{
	bool res = true;
	ACS_CC_ReturnType result;
	ACS_APGCC_OiHandler oi_Handler;

	MyImplementer implementer_1(dn, implementerName, ACS_APGCC_ONE );

	result = oi_Handler.addObjectImpl(&implementer_1);
	if (result == ACS_CC_SUCCESS)
	{

		ACE_TP_Reactor *tp_reactor_impl = new ACE_TP_Reactor;
		ACE_Reactor *reactor = new ACE_Reactor(tp_reactor_impl) ;

		reactor->open(1);

		MyThread_IMM thread_1(&implementer_1, reactor);

		thread_1.activate();
		sleep(1);

		ACS_CC_ImmParameter parToModify = defineParameterInt(attr,ATTR_INT32T,&value,1);
		cout << "value = " << value << endl;
		result = implementer_1.modifyRuntimeObj(dn,&parToModify);
		cout << "result of modify runtime obj :" << result << endl;
		if (result != ACS_CC_SUCCESS) res = false;

		delete[] parToModify.attrValues;

		result = oi_Handler.removeObjectImpl(&implementer_1);

		implementer_1.reactor()->end_reactor_event_loop();
		delete reactor;
		delete tp_reactor_impl;
	}
	else
	{
		cout << "could not add obj implementer" << endl;
		res = false;
	}

	return res;
}
bool IMM_Util::modify_OM_ImmAttr(const char *object, ACS_CC_ImmParameter parameter)
{
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{
		result = omHandler.modifyAttribute(object,&parameter);

		int id = -1;
		char* text;

		id = omHandler.getInternalLastError();
		text = omHandler.getInternalLastErrorText();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error modifyAttribute()" << std::endl;
			cout<<"   	ErrorCode: "<< id <<" TEXT: "<< text  << endl;
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}

ACS_CC_ImmParameter IMM_Util::defineParameterInt(const char* attributeName, ACS_CC_AttrValueType type, int *value, unsigned int numValue)
{
	ACS_CC_ImmParameter parToModify;

	char *name_attrUnsInt32 = const_cast<char*>(attributeName);
	parToModify.attrName = name_attrUnsInt32;
	parToModify.attrType = type;
	parToModify.attrValuesNum = numValue;

	if (parToModify.attrValuesNum == 0)	parToModify.attrValues = 0;
	else
	{
		parToModify.attrValues=new void*[parToModify.attrValuesNum];
		parToModify.attrValues[0] =reinterpret_cast<void*>(value);
	}

return parToModify;

}

ACS_CC_ImmParameter IMM_Util::defineParameterString(const char* attributeName, ACS_CC_AttrValueType type, const char* value, unsigned int numValue)
{
	ACS_CC_ImmParameter parToModify;

	char* stringValue = const_cast<char *>(value);
	char *name_attrString = const_cast<char*>(attributeName);
	parToModify.attrName = name_attrString;
	parToModify.attrType = type;
	parToModify.attrValuesNum = numValue;

	if (parToModify.attrValuesNum == 0)	parToModify.attrValues = 0;
	else
	{
		parToModify.attrValues=new void*[parToModify.attrValuesNum];
		parToModify.attrValues[0] =reinterpret_cast<void*>(stringValue);
	}


return parToModify;

}

void IMM_Util::printDebugObject (std::string dn)
{
	ACS_APGCC_ImmObject paramList;

	std::cout<<"\n--------------------------------------------------------------------------------"<< std::endl;
	std::cout<<"    OBJECT : " << dn.c_str() << std::endl;
	std::cout<<"--------------------------------------------------------------------------------"<< std::endl;

	//get Obj
	getObject(dn,&paramList);


	for (unsigned i=0; i<paramList.pointers.size(); i++)
	{
		ACS_APGCC_ImmAttribute att;
		att = paramList.attributes[i];


		switch ( att.attrType )
		{
			case ATTR_INT32T:
				cout << att.attrName.c_str() << "  =  " <<*reinterpret_cast<int *>(att.attrValues[0]) << endl;
				break;
			case ATTR_UINT32T:
				cout << att.attrName.c_str() << "  =  " <<*reinterpret_cast<unsigned int *>(att.attrValues[0]) << endl;
				break;
			case ATTR_INT64T:
				cout << att.attrName.c_str() << "  =  " <<*reinterpret_cast<long long *>(att.attrValues[0]) << endl;
				break;
			case ATTR_UINT64T:
				cout << att.attrName.c_str() << "  =  " <<*reinterpret_cast<unsigned long long *>(att.attrValues[0]) << endl;
				break;
			case ATTR_FLOATT:
				cout << att.attrName.c_str() << "  =  " <<*reinterpret_cast<float *>(att.attrValues[0]) << endl;
				break;
			case ATTR_DOUBLET:
				cout << att.attrName.c_str() << "  =  " <<*reinterpret_cast<double *>(att.attrValues[0]) << endl;
				break;
			case ATTR_NAMET:
				cout << att.attrName.c_str() << "  =  " << reinterpret_cast<char *>(att.attrValues[0]) << endl;
				break;
			case ATTR_STRINGT:
				cout << att.attrName.c_str() << "  =  " << reinterpret_cast<char *>(att.attrValues[0]) << endl;
				break;
			default:
				break;

		}//switch
	}//for

	std::cout << std::endl;
}

void IMM_Util::printDebugAllObjectClass (std::string className)
{
	std::vector<std::string> list;

	std::cout<<"\n-----------------------------------------------------------------------------------"<< std::endl;
	std::cout<<"    CLASS NAME : " << className.c_str() << std::endl;

	//get objects
	getClassObjectsList(className,list);

	std::cout<<"    # of instances : " << list.size() << std::endl;
	std::cout<<"----------------------------------------------------------------------------------"<< std::endl;

	//loop of istance
	for (unsigned i=0; i<list.size(); i++)
	{
		printDebugObject(list[i]);
	}


}

string IMM_Util::getIdValueFromRdn(const string & rdn)
{
	string rdnId = rdn;

	size_t index = rdnId.find_first_of(",");
	if (index != string::npos)
		rdnId = rdnId.substr(0, index);

	index = rdnId.find_first_of("=");
	if (index != string::npos)
		rdnId = rdnId.substr(index+1);

	return rdnId;
}

int IMM_Util::getError()
{

return IMM_Util::errorCode;

}

bool IMM_Util::createImmObject_NO_OI(const char *p_className, const char *parent, vector<ACS_CC_ValuesDefinitionType> AttrList)
{
	bool res = false;
	ACS_CC_ReturnType result;
	OmHandler omHandler;

	char* nameClasse = const_cast<char*>(p_className);
	char* parentName = const_cast<char*>(parent);

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{
		//int id = -1;
		char* text;
		result = omHandler.createObject(nameClasse, parentName, AttrList);
		 IMM_Util::errorCode = omHandler.getInternalLastError();
		text = omHandler.getInternalLastErrorText();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error createObject()" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Into Class : "<<  p_className << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "From Parent Object: "<<  parent << std::endl;
			cout<<"   	ErrorCode: "<< IMM_Util::errorCode <<" TEXT: "<< text  << endl;
			res = false;
		}
		else res = true;

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}
