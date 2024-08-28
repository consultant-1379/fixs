/*
 * MyImplementer_2.cpp
 *
 *  Created on: Sep 14, 2010
 *      Author: xfabron
 */


#include "MyImplementer_2.h"

#include <iostream>
using namespace std;

MyImplementer_2::MyImplementer_2(){

}

MyImplementer_2::MyImplementer_2(string p_impName ):ACS_APGCC_ObjectImplementerInterface(p_impName){

}

MyImplementer_2::MyImplementer_2(string p_objName, string p_impName, ACS_APGCC_ScopeT p_scope ): ACS_APGCC_ObjectImplementerInterface(p_objName, p_impName, p_scope){

}


ACS_CC_ReturnType MyImplementer_2::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentName, ACS_APGCC_AttrValues **attr){

	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectCreateCallback invocated           " << endl;
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
	cout << " HO TERMINATO LA CREATECallback " << endl;
	cout << endl;

	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType MyImplementer_2::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName){

	cout << "---------------------------------------------------" << endl;
	cout << "          ObjectDeleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;
	cout << " object Name: " << objName << endl;

	cout << " HO TERMINATO LA DeleteCallback " << endl;
	cout << endl;

	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType MyImplementer_2::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods){

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
			cout << " attrName: " << attrMods[i]->modAttr.attrName << "\t value: " <<*reinterpret_cast<int *>(attrMods[i]->modAttr.attrValues[0]) << endl;
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
	cout << " HO TERMINATO LA MODIFYCallback " << endl;
	cout << endl;

	return ACS_CC_SUCCESS;
}

ACS_CC_ReturnType MyImplementer_2::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId){

	cout << "---------------------------------------------------" << endl;
	cout << "          CcbCompleteCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " HO TERMINATO LA COMPLETECallback " << endl;
	cout << endl;


	return ACS_CC_SUCCESS;

}

void MyImplementer_2::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId){

	cout << "---------------------------------------------------" << endl;
	cout << "          CcbAbortCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " HO TERMINATO LA ABORTCallback " << endl;
	cout << endl;
}

void MyImplementer_2::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId){


	cout << "---------------------------------------------------" << endl;
	cout << "          CcbApplyCallback invocated           " << endl;
	cout << "---------------------------------------------------" << endl;
	cout << " ObjectImplementer: " << getImpName() << endl;

	cout << " Imm Handle: " << oiHandle << endl;
	cout << " ccbId: " << ccbId << endl;

	cout << " HO TERMINATO LA APPLYCallback " << endl;
	cout << endl;
}


ACS_CC_ReturnType MyImplementer_2::updateRuntime(const char* p_objName, const char* p_attrName){

	printf("------------------------------------------------------------\n");
	printf("                   updateRuntime called                     \n");
	printf("------------------------------------------------------------\n");

	cout<<"Object Name: "<<p_objName<<endl;
	cout<<"Attribute Name: "<<p_attrName<<endl;


	printf("------------------------------------------------------------\n");
	return ACS_CC_SUCCESS;

}
