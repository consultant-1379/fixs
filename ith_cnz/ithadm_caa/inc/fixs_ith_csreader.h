/*
 * fixs_ith_csreader.h
 *
 *  Created on: Jan 27, 2016
 *      Author: xludesi
 */


#ifndef ITHADM_CAA_INC_FIXS_ITH_CSREADER_H_
#define ITHADM_CAA_INC_FIXS_ITH_CSREADER_H_

#include <vector>

#include <ace/Recursive_Thread_Mutex.h>

#include <ACS_CS_API.h>

#include <ACS_DSD_Macros.h>

#include "fixs_ith_switchboardinfo.h"

class fixs_ith_server;

/** @class fixs_ith_csreader fixs_ith_csreader.h
 *	@brief fixs_ith_csreader class
 *	@author
 *	@date 2016-01-10
 *
 *	fixs_ith_csreader <PUT DESCRIPTION>
 */
class fixs_ith_csreader {
	//==============//
	// Enumerations //
	//==============//
public:
	enum FIXS_ITH_SwitchBoardFbnIdentifiers {
		SWITCH_BOARD_FBN_SMXB	= 400, /*ACS_CS_API_HWC_NS::FBN_SMXB*/
                SWITCH_BOARD_FBN_CMXB   = 380 /*ACS_CS_API_HWC_NS::FBN_CMXB*/
	};


	//==============//
	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_csreader constructor
	 */
	inline explicit fixs_ith_csreader()
	: _my_ap_node(), _my_ipna_str(), _my_ipnb_str(), _my_magazine(0), _my_magazine_str(), _my_slot_position(-1),
	  _my_partner_slot_position(-1),_my_side(-1), _my_system_number(0), _hwc(0), _my_ap_info_loaded(0), _sync_object()
	{
		*_my_ipna_str = 0;
		*_my_ipnb_str = 0;
		*_my_magazine_str = 0;
	}


private:
	fixs_ith_csreader (const fixs_ith_csreader & rhs);


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_csreader Destructor
	 */
	inline ~fixs_ith_csreader () { unbind(); }


	//===========//
	// Functions //
	//===========//
public:
	int bind_to_cs ();

	int unbind ();

	int get_ap_sys_no(int ap_board_name, std::vector<uint16_t> & ap_sys_no_list);

	int get_my_ap_node (ACS_DSD_Node & node);

	int get_my_ipn_addresses (char (& ipna_str) [16], char (& ipnb_str) [16]);
	int get_ip_eth_addresses (char (& ipna_address) [16], char (& ipnb_address) [16], BoardID board_id);
	int get_alias_ip_eth_addresses (char (& alias_ipna_address) [16], char (& alias_ipnb_address) [16], BoardID board_id);
	int get_my_side (uint16_t & side);
	int get_my_magazine (uint32_t & magazine);
	int get_my_magazine (char (& magazine) [16]);
	int get_other_magazines (std::vector <uint32_t> & magazines);
	int get_all_magazines (std::vector <uint32_t> & magazines);

	int map_magazine_plug_number (uint32_t & magazine, int mag_plug_number);

	int get_my_slot (int32_t & slot);
	int get_my_partner_slot (int32_t & slot);
	int get_my_system_number (uint16_t &sys_no);

	int get_environment(int & env);

	int get_my_switch_boards_info (std::vector <fixs_ith_switchboardinfo> & boards);
	int get_other_switch_boards_info (std::vector <fixs_ith_switchboardinfo> & boards);
	int get_switch_boards_info (std::vector <fixs_ith_switchboardinfo> & boards);

	int reset_info ();

	static int uint32_to_ip_format (char (& value_ip) [16], uint32_t value);
	static int uint32_to_ip_format (char (& value_ip) [16], uint32_t value, char separator);
	static int ip_format_to_uint32 (uint32_t & value, const char * value_ip);
	static int uint32_to_ip_fields (uint8_t & ip1, uint8_t & ip2, uint8_t & ip3, uint8_t & ip4, uint32_t value);
	static int uint32_to_ip_fields (uint8_t (& ip_fields) [4], uint32_t value);
	static int ip_format_to_ip_fields (uint8_t & ip1, uint8_t & ip2, uint8_t & ip3, uint8_t & ip4, const char * value_ip);
	static int ip_format_to_ip_fields (uint8_t (& ip_fields) [4], const char * value_ip);



private:
	int get_my_ap_all_info ();
	int get_switch_boards_info (std::vector <fixs_ith_switchboardinfo> & boards, uint32_t magazine, bool match);

	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_csreader & operator= (const fixs_ith_csreader & rhs);


	//========//
	// Fields //
	//========//
private:
	ACS_DSD_Node _my_ap_node;
	char _my_ipna_str [16];
	char _my_ipnb_str [16];
	uint32_t _my_magazine;
	char _my_magazine_str[16];
	int32_t _my_slot_position;
	int32_t _my_partner_slot_position;
	uint16_t _my_side;
	uint16_t _my_system_number;

	static int _environment;

	ACS_CS_API_HWC * _hwc;
	int _my_ap_info_loaded;

	// Access synchronization controllers
	ACE_Recursive_Thread_Mutex _sync_object;
};




#endif /* ITHADM_CAA_INC_FIXS_ITH_CSREADER_H_ */
