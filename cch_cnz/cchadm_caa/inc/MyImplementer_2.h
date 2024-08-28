/*
 * MyImplementer_2.h
 *
 *  Created on: Sep 14, 2010
 *      Author: xfabron
 */

#ifndef MYIMPLEMENTER_2_H_
#define MYIMPLEMENTER_2_H_

#include "ACS_APGCC_ObjectImplementerInterface.h"

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)

class MyImplementer_2 : public ACS_APGCC_ObjectImplementerInterface{
public :

	MyImplementer_2();

	MyImplementer_2(string p_impName );

	MyImplementer_2(string p_objName, string p_impName, ACS_APGCC_ScopeT p_scope );

	virtual ~MyImplementer_2(){};

	ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

	ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

	ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

	ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

	void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

	void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

	ACS_CC_ReturnType updateRuntime(const char* p_nameObj, const char* p_nameAttr);

};


#endif /* MYIMPLEMENTER_2_H_ */
