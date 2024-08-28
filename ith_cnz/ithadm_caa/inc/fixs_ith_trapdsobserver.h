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

#ifndef ITHADM_CAA_INC_FIXS_ITH_TRAPDSOBSERVER_H_
#define ITHADM_CAA_INC_FIXS_ITH_TRAPDSOBSERVER_H_


#include <ace/Recursive_Thread_Mutex.h>

#include <ACS_TRAPDS_API.h>

#include <map>
#include <string>

/*
 * Forward declarations
 */
class fixs_ith_server;

namespace trap_mask
{
	const unsigned short COLD_RESTART = 0x01;
    const unsigned short LINK_UP = 0x02;
    const unsigned short LINK_DOWN = 0x04;
    const unsigned short BFD_UP = 0x08;
    const unsigned short BFD_DOWN = 0x10;
    const unsigned short VRRP_CHANGE = 0x20;
    const unsigned short SMX_PIM = 0x40;
    const unsigned short ALL = (COLD_RESTART | LINK_UP | LINK_DOWN | BFD_UP | BFD_DOWN | VRRP_CHANGE |SMX_PIM);
}

class fixs_ith_trapdsobserver : public ACS_TRAPDS_API {
	//==============//
	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_snmptrapdsobserver constructor
	 */
	inline explicit fixs_ith_trapdsobserver ()
	: _subscribed(0), _sync_object(), m_trap_status() {}

private:
	fixs_ith_trapdsobserver (const fixs_ith_trapdsobserver & rhs);


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_snmptrapdsobserver Destructor
	 */
	inline virtual ~fixs_ith_trapdsobserver () {}


	//===========//
	// Functions //
	//===========//
public:
	// BEGIN: ACS_TRAPDS_API Interface
	virtual void handleTrap (ACS_TRAPDS_StructVariable var);
	// END: ACS_TRAPDS_API Interface

	int subscribe(std::string serviceName);
	int unsubscribe();

	void enable_traps(const std::string& board_key, int traps_mask);

	void disable_traps(const std::string& board_key, int traps_mask);

	bool is_trap_enabled(const std::string& board_key, int trap_mask);

  //===========//
	// Operators //
	//===========//
private:
	fixs_ith_trapdsobserver & operator= (const fixs_ith_trapdsobserver & rhs);


	//========//
	// Fields //
	//========//
private:
	uint8_t _subscribed;

	// Access synchronization controllers
	ACE_Recursive_Thread_Mutex _sync_object;

	typedef std::map<std::string, int> traps_list_t;
	traps_list_t m_trap_status;
};


#endif /* ITHADM_CAA_INC_FIXS_ITH_TRAPDSOBSERVER_H_ */
