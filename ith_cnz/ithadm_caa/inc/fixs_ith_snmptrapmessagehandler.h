//	********************************************************
//
//	 COPYRIGHT Ericsson 2016
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2016.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2016 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************

#ifndef ITHADM_CAA_INC_FIXS_ITH_SNMPTRAPMESSAGEHANDLER_H_
#define ITHADM_CAA_INC_FIXS_ITH_SNMPTRAPMESSAGEHANDLER_H_

#include <ACS_TRAPDS_StructVariable.h>


enum typTrapEnm{
                enmTrapNull = 0,
                enmTrapColdStart,
                enmWarmRestart,
                enmColdStartWarmStart,
};


class fixs_ith_snmptrapmessagehandler {
	//==========//
	// Typedefs //
	//==========//
	typedef std::list<ACS_TRAPDS_varlist> trapds_var_list_t;


	//==============//
	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_snmptrapmessagehandler constructor
	 */
	inline explicit fixs_ith_snmptrapmessagehandler () {}

private:
	fixs_ith_snmptrapmessagehandler (const fixs_ith_snmptrapmessagehandler & rhs);


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_snmptrapmessagehandler Destructor
	 */
	inline virtual ~fixs_ith_snmptrapmessagehandler () {}


	//===========//
	// Functions //
	//===========//
public:

	int handle_trap (ACS_TRAPDS_StructVariable & var);

	int handle_ColdStartTrap(const std::string& board_key,const char * source_trap_ip) const;

	inline int handle_WarmStartTrap(const std::string& board_key,const char * source_trap_ip) const
	{
		return  handle_ColdStartTrap(board_key,source_trap_ip);
	}

	int handle_VrrpMasterChange(trapds_var_list_t::iterator & var_it, trapds_var_list_t::iterator & var_list_end, const std::string& board_key) const;

	int handle_PimTrap (
			trapds_var_list_t::iterator & var_it,
			trapds_var_list_t::iterator & var_list_end,
			const std::string& board_key) const;

	int handle_InterfaceLinkDown (
			trapds_var_list_t::iterator & var_it,
			trapds_var_list_t::iterator & var_list_end,
			const std::string& board_key) const;

	int handle_InterfaceLinkUp (
			trapds_var_list_t::iterator & var_it,
			trapds_var_list_t::iterator & var_list_end,
			const std::string& board_key) const;

	int handle_BfdSessionUp(trapds_var_list_t::iterator& var_it, trapds_var_list_t::iterator& var_list_end, const std::string& board_key) const;

	int handle_BfdSessionDown(trapds_var_list_t::iterator& var_it, trapds_var_list_t::iterator& var_list_end, const std::string& board_key) const;
        void OidToString(oid *iodIn, unsigned int len, std::string *strOut);
protected:

	void oid_to_string (char * buf, size_t buf_size, const oid * name, size_t name_size) const;

private:

	bool getSmxIdFromIp(const char* boardIp, std::string& smxKey) const;

	bool getBFDSessionInfoFromBfdMib(trapds_var_list_t::iterator& var_it, trapds_var_list_t::iterator& var_list_end, std::string& routerName, std::string& ipAddress, int& sessionState) const;

	const char * utl_link_down_admin_status_to_string (int admin_status) const;
	const char * utl_link_down_oper_status_to_string (int oper_status) const;

	const char * utl_link_up_admin_status_to_string (int admin_status) const;
	const char * utl_link_up_oper_status_to_string (int oper_status) const;

	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_snmptrapmessagehandler & operator= (const fixs_ith_snmptrapmessagehandler & rhs);


	//========//
	// Fields //
	//========//
private:
};


#endif /* ITHADM_CAA_INC_FIXS_ITH_SNMPTRAPMESSAGEHANDLER_H_ */
