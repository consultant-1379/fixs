/*
 * MyImplementer.cpp
 *
 *  Created on: Sep 8, 2010
 *      Author: xfabron
 */

#include "MyImplementer.h"

#include <iostream>
using namespace std;

MyImplementer::MyImplementer(string p_impName ):ACS_APGCC_ObjectImplementer_EventHandler(p_impName){

	value = 0;
}

MyImplementer::MyImplementer(string p_objName, string p_impName, ACS_APGCC_ScopeT p_scope ): ACS_APGCC_ObjectImplementer_EventHandler(p_objName, p_impName, p_scope){

//	objName = p_objName;
//	impName = p_impName;
//	scope = p_scope;

	value = 0;
}


ACS_CC_ReturnType MyImplementer::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentName, ACS_APGCC_AttrValues **attr){

	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback INVOCATED           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " class Name: " << className << endl;
	cout << " parent Name: " << parentName << endl;

	cout << endl;

	int dim=0;
	int i=0;
	while(attr[i]){
		i++;
		dim++;
	}

	cout<<"DIM: "<<dim<<endl;

	i = 0;
	while( attr[i] ){
		switch ( attr[i]->attrType ) {
		case ATTR_INT32T:
			cout << " attrName: " << attr[i]->attrName << "\t value: " <<*reinterpret_cast<int *>(attr[i]->attrValues[0]) << endl;
			break;
		case ATTR_UINT32T:
			cout << " attrName: " << attr[i]->attrName << "\t value: " <<*reinterpret_cast<unsigned int *>(attr[i]->attrValues[0]) << endl;
			break;
		case ATTR_INT64T:
			cout << " attrName: " << attr[i]->attrName << "\t value: " <<*reinterpret_cast<long long *>(attr[i]->attrValues[0]) << endl;
			break;
		case ATTR_UINT64T:
			cout << " attrName: " << attr[i]->attrName << "\t value: " <<*reinterpret_cast<unsigned long long *>(attr[i]->attrValues[0]) << endl;
			break;
		case ATTR_FLOATT:
			cout << " attrName: " << attr[i]->attrName << "\t value: " <<*reinterpret_cast<float *>(attr[i]->attrValues[0]) << endl;
			break;
		case ATTR_DOUBLET:
			cout << " attrName: " << attr[i]->attrName << "\t value: " <<*reinterpret_cast<double *>(attr[i]->attrValues[0]) << endl;
			break;
		case ATTR_NAMET:
			cout << " attrName: " << attr[i]->attrName << "\t value: " << reinterpret_cast<char *>(attr[i]->attrValues[0]) << endl;
			break;
		case ATTR_STRINGT:
			cout << " attrName: " << attr[i]->attrName << "\t value: " << reinterpret_cast<char *>(attr[i]->attrValues[0]) << endl;
			break;
		default:
			break;

		}
		i++;
	}
	cout << endl;

	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType MyImplementer::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName){

	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectDeleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	cout << endl;

	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType MyImplementer::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods){

	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectModifyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;
	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;


	cout << endl;

	int i = 0;
	while( attrMods[i] ){
		switch ( attrMods[i]->modAttr.attrType ) {
		case ATTR_INT32T:
			value = *reinterpret_cast<int *>(attrMods[i]->modAttr.attrValues[0]) ;
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " <<*reinterpret_cast<int *>(attrMods[i]->modAttr.attrValues[0]) << endl;
			if( value == 100 ){
				cout << " Valore non valido: " << value << endl;
			//	value = 0;
			//	return ACS_CC_FAILURE;
			}
			break;
		case ATTR_UINT32T:
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " <<*reinterpret_cast<unsigned int *>(attrMods[i]->modAttr.attrValues[0]) << endl;
			break;
		case ATTR_INT64T:
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " <<*reinterpret_cast<long long *>(attrMods[i]->modAttr.attrValues[0]) << endl;
			break;
		case ATTR_UINT64T:
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " <<*reinterpret_cast<unsigned long long *>(attrMods[i]->modAttr.attrValues[0]) << endl;
			break;
		case ATTR_FLOATT:
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " <<*reinterpret_cast<float *>(attrMods[i]->modAttr.attrValues[0]) << endl;
			break;
		case ATTR_DOUBLET:
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " <<*reinterpret_cast<double *>(attrMods[i]->modAttr.attrValues[0]) << endl;
			break;
		case ATTR_NAMET:
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " << reinterpret_cast<char *>(attrMods[i]->modAttr.attrValues[0]) << endl;
			break;
		case ATTR_STRINGT:
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " << reinterpret_cast<char *>(attrMods[i]->modAttr.attrValues[0]) << endl;
			break;
		default:
			break;

		}
		i++;
	}
	cout << endl;


	/*

	cout<<"Modification Type: "<<attrMods[0]->modType<<endl;
	cout<<"attrName: "<<attrMods[0]->modAttr.attrName<<endl;
	//cout<<"attrValue: "<<*(reinterpret_cast<float *>(attrMods[0]->modAttr.attrValues[0]))<<endl;

	cout<<"attrValue: "<<reinterpret_cast<char *>((attrMods[0]->modAttr.attrValues[0]))<<endl;
*/

	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType MyImplementer::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId){

	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << endl;

	if(value == 100){
		value = 0;
		return ACS_CC_FAILURE;
	}else{
		return ACS_CC_SUCCESS;
	}

}

void MyImplementer::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId){

	cout << "---------------------------------------------------" << endl;
	cout << "          CcbAbortCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << endl;
}

void MyImplementer::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId){


	cout << "---------------------------------------------------" << endl;
	cout << "          CcbApplyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << endl;
}


ACS_CC_ReturnType MyImplementer::updateRuntime(const char* p_objName, const char* p_attrName){

	printf("------------------------------------------------------------\n");
	printf("                   updateRuntime called                     \n");
	printf("------------------------------------------------------------\n");

	cout<<"Object Name: "<<p_objName<<endl;
	cout<<"Attribute Name: "<<p_attrName<<endl;


	printf("------------------------------------------------------------\n");
	return ACS_CC_SUCCESS;

}
