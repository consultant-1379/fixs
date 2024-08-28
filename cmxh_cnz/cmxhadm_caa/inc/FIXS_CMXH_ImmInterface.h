/*
 * FIXS_CMXH_ImmInterface.h
 *
 
 */

#ifndef FIXS_CMXH_IMMINTERFACE_H_
#define FIXS_CMXH_IMMINTERFACE_H_

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <bits/stl_map.h>
#include <list>


#include "FIXS_CMXH_Util.h"
#include "FIXS_CMXH_IMM_Util.h"


#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"

using namespace CMXH_Util;

namespace IMM_Interface
{
	//General
	void getEnvironmentType(EnvironmentType &env);

	std::string getDnSnmpPolling();
	bool getSnmpPollingTime(int &polling_time);
	bool getVlanParameter(std::string dn, vlanParameterStruct &data,int version);
	bool modifyObject();

};//namespace

#endif /* FIXS_CMXH_IMMINTERFACE_H_ */
