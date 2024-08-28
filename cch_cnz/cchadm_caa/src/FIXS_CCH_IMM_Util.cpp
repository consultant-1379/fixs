/*
 * FIXS_CCH_IMM_Util.cpp
 *
 *  Created on: Oct 5, 2011
 *      Author: eanform
 */
#include "FIXS_CCH_IMM_Util.h"
#include "FIXS_CCH_DiskHandler.h"
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
		else if(Param.attrValuesNum > 0)
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
		int id = -1;
                char* text;
		result = omHandler.deleteObject(dn.c_str(), p_scope);
                id = omHandler.getInternalLastError();
                text = omHandler.getInternalLastErrorText();

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
		MyImplementer_2 implementer_1(classImplementer);

		//add implementer to the class
		result = oi_Handler.addClassImpl(&implementer_1, p_className);
		if (result != ACS_CC_SUCCESS)	cout<<" ERROR set implementer............."<< endl;

		MyThread_IMM2 *thread_1 = new MyThread_IMM2(&implementer_1);

		thread_1->activate();
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


		thread_1->stop();

		//remove implementer
		result = oi_Handler.removeClassImpl(&implementer_1, p_className);
		if (result != ACS_CC_SUCCESS)	cout<<" ERROR remove implementer............."<< endl;


		int retry = 0;
		while (thread_1->isRunningMyThread_IMM2() && retry<3){
			sleep(1);
			retry++;
		}
		delete thread_1;


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

	MyImplementer_2 implementer_1(dn, implementerName, ACS_APGCC_ONE );

	result = oi_Handler.addObjectImpl(&implementer_1);
	if (result == ACS_CC_SUCCESS)
	{
		MyThread_IMM2 *thread_1 = new MyThread_IMM2(&implementer_1);

		thread_1->activate();
		sleep(1);

		ACS_CC_ImmParameter parToModify = defineParameterString(attr,ATTR_STRINGT,value,1);

		result = implementer_1.modifyRuntimeObj(dn,&parToModify);
		if (result != ACS_CC_SUCCESS) res = false;

		delete[] parToModify.attrValues;


		thread_1->stop();
		result = oi_Handler.removeObjectImpl(&implementer_1);

		int retry = 0;
		while (thread_1->isRunningMyThread_IMM2() && retry<3){
			sleep(1);
			retry++;
		}
		delete thread_1;


	}

	return res;
}

bool IMM_Util::modify_OI_ImmAttrINT(const char *implementerName, const char *dn, const char *attr, int value)
{
	bool res = true;
	ACS_CC_ReturnType result;
	ACS_APGCC_OiHandler oi_Handler;

	MyImplementer_2 implementer_1(dn, implementerName, ACS_APGCC_ONE );

	result = oi_Handler.addObjectImpl(&implementer_1);
	if (result == ACS_CC_SUCCESS)
	{

		MyThread_IMM2 *thread_1 = new MyThread_IMM2(&implementer_1);

		thread_1->activate();
		sleep(1);

		ACS_CC_ImmParameter parToModify = defineParameterInt(attr,ATTR_INT32T,&value,1);

		result = implementer_1.modifyRuntimeObj(dn,&parToModify);
		if (result != ACS_CC_SUCCESS) res = false;

		delete[] parToModify.attrValues;

		thread_1->stop();
		result = oi_Handler.removeObjectImpl(&implementer_1);

		int retry = 0;
		while (thread_1->isRunningMyThread_IMM2() && retry<3){
			sleep(1);
			retry++;
		}
		delete thread_1;



	}

	return res;
}

bool IMM_Util::modify_OI_ImmAttrSTRING(const char *implementerName, const char *className,const char *dn, const char *attr, const char *value)
{
	bool res = true;
	ACS_CC_ReturnType result;
	ACS_APGCC_OiHandler oi_Handler;

	MyImplementer_2 implementer_1(implementerName);

	result = oi_Handler.addClassImpl(&implementer_1,className);

	if (result == ACS_CC_SUCCESS)
	{
		MyThread_IMM2 *thread_1 = new MyThread_IMM2(&implementer_1);

		thread_1->activate();
		sleep(1);

		ACS_CC_ImmParameter parToModify = defineParameterString(attr,ATTR_STRINGT,value,1);

		result = implementer_1.modifyRuntimeObj(dn,&parToModify);
		if (result != ACS_CC_SUCCESS) res = false;

		delete[] parToModify.attrValues;

		thread_1->stop();
		result = oi_Handler.removeClassImpl(&implementer_1,className);

		int retry = 0;
		while (thread_1->isRunningMyThread_IMM2() && retry<3){
			sleep(1);
			retry++;
		}
		delete thread_1;



	}

	return res;
}

bool IMM_Util::modify_OI_ImmAttrINT(const char *implementerName, const char *className,const char *dn, const char *attr, int value)
{
	bool res = true;
	ACS_CC_ReturnType result;
	ACS_APGCC_OiHandler oi_Handler;

	MyImplementer_2 implementer_1(implementerName);

	result = oi_Handler.addClassImpl(&implementer_1,className);
	if (result == ACS_CC_SUCCESS)
	{

		MyThread_IMM2 *thread_1 = new MyThread_IMM2(&implementer_1);

		thread_1->activate();
		sleep(1);

		ACS_CC_ImmParameter parToModify = defineParameterInt(attr,ATTR_INT32T,&value,1);

		result = implementer_1.modifyRuntimeObj(dn,&parToModify);
		if (result != ACS_CC_SUCCESS) res = false;

		delete[] parToModify.attrValues;

		thread_1->stop();
		result = oi_Handler.removeClassImpl(&implementer_1,className);

		int retry = 0;
		while (thread_1->isRunningMyThread_IMM2() && retry<3){
			sleep(1);
			retry++;
		}
		delete thread_1;




	}

	return res;
}

bool IMM_Util::modify_OM_ImmAttr(const char *object, ACS_CC_ImmParameter parameter)
{
        ACS_TRA_Logging *FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
        ACS_TRA_trace *traceObj = new ACS_TRA_trace("FIXS_CCH_IMM_Util");
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
		if (result != ACS_CC_SUCCESS)
		{
			int id = 0;
			char * text;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error modifyAttribute()" << std::endl;
			id = omHandler.getInternalLastError();
			text = omHandler.getInternalLastErrorText();
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "ErrorCode: "<< id <<" TEXT: "<< text  << endl;
			res = false;
			{
			char tmpStr[512] = {0};
                        snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] ErrorCode: %d,text : %s", __FUNCTION__, __LINE__, id,text);
                        if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
			}
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
	if (getObject(dn,&paramList)) {

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
	} else {
		cout << "OBJECT NOT FOUND" << endl;
	}

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

bool IMM_Util::createClassFwStorage()
{
	bool res = true;

	ACS_CC_ReturnType result;
	OmHandler immHandler;

	/* Name of the class to be defined*/
	char *nameClassToBeDefined = ((char*)classOtherBladeFwStorage);//const_cast<char*>("Test_config");

	/*Category of class to be defined*/
	ACS_CC_ClassCategoryType  categoryClassToBeDefined = CONFIGURATION;

	/*Attributes list of the class to be defined */
	vector<ACS_CC_AttrDefinitionType> classAttributes;

	ACS_CC_AttrDefinitionType attributeRDN = {const_cast<char *>(RDN_OTHERBLADEFWSTORAGE),ATTR_STRINGT,ATTR_RDN|ATTR_CONFIG,0} ;
	ACS_CC_AttrDefinitionType attributeI32 = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGE_RFW_STATUS),ATTR_INT32T,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};
	ACS_CC_AttrDefinitionType attributeSTRING = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGE_RFW_RESULT),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};
	ACS_CC_AttrDefinitionType attributeSTRING1 = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGE_RFW_REASON),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};
	ACS_CC_AttrDefinitionType attributeSTRING2 = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGE_RFW_DATE),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};
	ACS_CC_AttrDefinitionType attributeSTRING3 = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGE_RFW_TIME),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};
	ACS_CC_AttrDefinitionType attributeSTRING4 = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGE_RFW_SH_FILE),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};
	ACS_CC_AttrDefinitionType attributeSTRING5 = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGE_RFW_XML_FILE),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};
	ACS_CC_AttrDefinitionType attributeSTRING6 = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGE_RFW_REVISION),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};

	classAttributes.push_back(attributeRDN);
	classAttributes.push_back(attributeI32);
	classAttributes.push_back(attributeSTRING);
	classAttributes.push_back(attributeSTRING1);
	classAttributes.push_back(attributeSTRING2);
	classAttributes.push_back(attributeSTRING3);
	classAttributes.push_back(attributeSTRING4);
	classAttributes.push_back(attributeSTRING5);
	classAttributes.push_back(attributeSTRING6);

	result=immHandler.Init();
	if(result == ACS_CC_SUCCESS)
	{
		int id = -1;
		char* text;

		result=immHandler.defineClass( nameClassToBeDefined,categoryClassToBeDefined,classAttributes );
		id = immHandler.getInternalLastError();
		text = immHandler.getInternalLastErrorText();

		if(result  == ACS_CC_SUCCESS)
		{
			cout<<"\n######################################################" << endl;
			cout<<"   	CLASS "<< classOtherBladeFwStorage <<" DEFINED !!! 	   " << endl;
			cout<<"######################################################\n" << endl;
		}
		else
		{
			if (id == -14) //SA_AIS_ERR_EXIST
			{
				cout<<"\n######################################################" << endl;
				cout<<"   	CLASS "<< classOtherBladeFwStorage <<" ...ALREADY.. DEFINED !!!" << endl;
				cout<<"######################################################\n" << endl;
			}
			else
			{
				cout<<"\n######################################################" << endl;
				cout<<"   	CLASS "<< classOtherBladeFwStorage <<" ...NOT... DEFINED !!!" << endl;
				cout<<"   	ErrorCode: "<< id <<" TEXT: "<< text  << endl;
				cout<<"######################################################\n" << endl;
				res = false;
			}
		}

		result = immHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "...Error Finalize()" << std::endl;
			res = false;
		}

	}
	else
	{
		cout << "...ERROR: init() FAILURE !!!\n "<< endl;
		res = false;
	}

	return res;
}

bool IMM_Util::createClassFwStorageInfo()
{
	bool res = true;

	ACS_CC_ReturnType result;
	OmHandler immHandler;

	/* Name of the class to be defined*/
	char *nameClassToBeDefined = ((char*)classOtherBladeFwStorageInfoOld);
	char *nameClassToBeDefined1 = ((char*)classOtherBladeFwStorageInfoNew);

	/*Category of class to be defined OLD*/
	ACS_CC_ClassCategoryType  categoryClassToBeDefined = CONFIGURATION;

	/*Attributes list of the class to be defined */
	vector<ACS_CC_AttrDefinitionType> classAttributes;

	ACS_CC_AttrDefinitionType attributeRDN = {const_cast<char *>(RDN_OTHERBLADEFWSTORAGEINFO_OLD),ATTR_STRINGT,ATTR_RDN|ATTR_CONFIG,0} ;
	ACS_CC_AttrDefinitionType attributeSTRING = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGEINFO_FW_TYPE),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};
	ACS_CC_AttrDefinitionType attributeSTRING2 = {const_cast<char *>(ATT_OTHERBLADEFWSTORAGEINFO_FW_VERSION),ATTR_STRINGT,SA_IMM_ATTR_CONFIG|ATTR_WRITABLE,0};

	classAttributes.push_back(attributeRDN);
	classAttributes.push_back(attributeSTRING);
	classAttributes.push_back(attributeSTRING2);


	/*Attributes list of the class to be defined NEW*/
	vector<ACS_CC_AttrDefinitionType> classAttributes1;

	ACS_CC_AttrDefinitionType attributeRDN1 = {const_cast<char *>(RDN_OTHERBLADEFWSTORAGEINFO_NEW),ATTR_STRINGT,ATTR_RDN|ATTR_CONFIG,0} ;

	classAttributes1.push_back(attributeRDN1);
	classAttributes1.push_back(attributeSTRING);
	classAttributes1.push_back(attributeSTRING2);

	result=immHandler.Init();
	if(result == ACS_CC_SUCCESS)
	{
		int id = -1;
		char* text;

		result=immHandler.defineClass( nameClassToBeDefined,categoryClassToBeDefined,classAttributes );
		id = immHandler.getInternalLastError();
		text = immHandler.getInternalLastErrorText();
		if(result == ACS_CC_FAILURE)
		{
			cout<<"\n######################################################" << endl;
			cout<<"   	CLASS "<< nameClassToBeDefined <<" ...NOT... DEFINED !!!" << endl;
			cout<<"   	ErrorCode: "<< id <<" TEXT: "<< text  << endl;
			cout<<"######################################################\n" << endl;
		}

		result=immHandler.defineClass( nameClassToBeDefined1,categoryClassToBeDefined,classAttributes1 );
		id = immHandler.getInternalLastError();
		text = immHandler.getInternalLastErrorText();

		if(result == ACS_CC_FAILURE)
		{
			cout<<"\n######################################################" << endl;
			cout<<"   	CLASS "<< nameClassToBeDefined1 <<" ...NOT... DEFINED !!!" << endl;
			cout<<"   	ErrorCode: "<< id <<" TEXT: "<< text  << endl;
			cout<<"######################################################\n" << endl;
		}
		result = immHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "...Error Finalize()" << std::endl;
			res = false;
		}
	}
	else
	{
		cout << "...ERROR: init() FAILURE !!!\n "<< endl;
		res = false;
	}

	return res;
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
		int id = -1;
		char* text;
		result = omHandler.createObject(nameClasse, parentName, AttrList);
		id = omHandler.getInternalLastError();
		text = omHandler.getInternalLastErrorText();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error createObject()" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Into Class : "<<  p_className << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "From Parent Object: "<<  parent << std::endl;
			cout<<"   	ErrorCode: "<< id <<" TEXT: "<< text  << endl;
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


bool IMM_Util::getRdnValue (std::string object_dn, std::string &value)
{
	bool result = false;
	if (!object_dn.empty())
	{
		std::string rdn("");
		getRdnObject(object_dn, rdn);
		value = ACS_APGCC::after(rdn,"=");
		result = true;
	}
	return result;
}

bool IMM_Util::getImmMultiAttributeString (std::string object, std::string attribute, std::vector<std::string> &value)
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
		char *name_attrPath = const_cast<char*>(attribute.c_str());
		Param.attrName = name_attrPath;
		result = omHandler.getAttribute(object.c_str(), &Param );
		if ( result != ACS_CC_SUCCESS ){	cout << "ERROR: Param " << attribute.c_str()<<" FAILURE!!!\n"<< endl; res = false; }
		else
		{
			if (Param.attrValuesNum > 0)
			{
				for (unsigned int i=0; i<Param.attrValuesNum; i++ )
				{
					value.push_back((char*)Param.attrValues[i]);
				}
			}
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
			res = false;
		}
	}

	return res;
}

ACS_CC_ImmParameter IMM_Util::defineMultiParameterString(const char* attributeName, ACS_CC_AttrValueType type, std::vector<std::string> value, unsigned int numValue)
{
	ACS_CC_ImmParameter parToModify;


	char *name_attrString = const_cast<char*>(attributeName);
	parToModify.attrName = name_attrString;
	parToModify.attrType = type;
	parToModify.attrValuesNum = numValue;

	if (parToModify.attrValuesNum == 0)	parToModify.attrValues = 0;
	else
	{
		parToModify.attrValues=new void*[parToModify.attrValuesNum];

		for (unsigned int i=0; i<parToModify.attrValuesNum; i++)
		{
			char* stringValue = const_cast<char *>(value[i].c_str());
			parToModify.attrValues[i] =reinterpret_cast<void*>(stringValue);
		}
	}


return parToModify;

}

std::string IMM_Util::getErrorText(int errNo)
{
	string text = "";

	switch(errNo){
	case DELETE_FAULTY_PACKAGE:        			 text = "Cannot delete faulty SW package";                         				break;
	case DELETE_DEFAULT_PACKAGE:       			 text = "Cannot delete default SW package";          		           			break;
	case DELETE_USED_PACKAGE:         			 text = "Cannot delete SW package in use";          						break;
	case DELETE_UPGRADE_ONGOING:      			 text = "Cannot delete SW package when an upgrade is ongoing";  				break;

	case DELETE_INVALID_FBN:         			 text = "Error when executing: invalid FBN";     			                	break;
	case CREATE_INVALID_FBN:         			 text = "Error when executing: invalid FBN";     						break;
	case FAILED_REMOVE_FOLDER:         			 text = "Error when executing: failed to remove SW package from disk";     			break;
	case FAULTY_PACKAGE:         				 text = "Operation not allowed: one installed package is marked as faulty";     		break;
	case FAILED_HARDWARE_NOT_PRESENT:			 text = "Blade not present";							     		break;
	case FAILED_INVALID_PARAMETERS:	   			 text = "Invalid parameters";							     		break;
	case FAILED_FBN_NOT_ALLOWED:      			 text = "Action not allowed for this Functional Board Name (FBN)";                   		break;
	case FAILED_IMPORT_ACTION_ALREADY_RUNNING:   text = "Import package action already running";		                   		break;
	case FAILED_INVALID_CONFIGURATION:      	 text = "Configuration not supported";				                   		break;
	case FAILED_PATH_NOT_EXIST:      	      	 text = "Invalid file path or file name";						        break;
	case FAILED_WRONG_EXTENSION:      	      	 text = "Invalid file format";						                      	break;
	case FAILED_INVALID_FILENAME:      	      	 text = "Invalid filename";						                      	break;
	case FAILED_INVALID_FBN:      	      		 text = "Invalid Functional Board Name";						       	break;
    case FAILED_SNMP_FAILURE:                    text = "SNMP Failure"; break;
    case BOARD_BLOCKED_NO_DEFAULT_RESET:         text = "Operation not allowed: blade blocked"; break;
    
    case IRONSIDE_FAILURE:        text = "Communication Failure"; break;

    case FAILURE_OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE : text = "Operation not allowed: Other IPLB in cluster is not active"; break; 
	case MODIFY_UPGRADE_ONGOING:                 text = "Cannot modify upgrade type when an upgrade is ongoing"; break;

	case BOARD_BLOCKED:                   text = "Blade already blocked";break;
	
	case BOARD_ALREADY_BLOCKED:                   text = "Blade already blocked";break;
	case BOARD_DEBLOCKED:                 text = "Blade already deblocked";break;
	case NOTALLOWED_AP2:                  text = "Not allowed on AP2";	
	break;
	default:                      				 text = "Error when executing (general fault)";

	}

	return text;
}
