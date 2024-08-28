/*
 * snmpsessioninfo.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: xassore
 */

#include <string.h>

#include "fixs_ith_snmpsessioninfo.h"

namespace {
	char gs_empty_string[] = "";
}

void fixs_ith_snmpsessioninfo::peername (char * new_value) { snmp_session::peername = new_value ?: gs_empty_string; }

void fixs_ith_snmpsessioninfo::community (char * new_value) {
	snmp_session::community = reinterpret_cast<u_char *>(new_value ?: gs_empty_string);
	snmp_session::community_len = ::strlen(reinterpret_cast<char *>(snmp_session::community));
}


