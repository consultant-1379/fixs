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
	UNUSED(attr);
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
	UNUSED(attrMods);
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
