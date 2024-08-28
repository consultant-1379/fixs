/*
 * fixs_ith_snmpsessioncallback.h
 *
 *  Created on: Jan 28, 2016
 *      Author: xassore
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONCALLBACK_H_
#define ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONCALLBACK_H_

#include "fixs_ith_snmpconstants.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/transform_oids.h> // To get using SNMP Version 3


class fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_snmpsessioncallback() : error_code(fixs_ith_snmp::ERR_SNMP_NO_ERRORS), pdu_error_stat(fixs_ith_snmp::ERR_SNMP_NO_ERRORS),
	board_slot(-1), _board_magazine_str(0), _switch_board_ip(0){ board_fbn=-1;}

	inline fixs_ith_snmpsessioncallback (const char * board_magazine_str_, int32_t board_slot_, const char * switch_board_ip_, uint16_t board_fbn_=-1)
	: error_code(fixs_ith_snmp::ERR_SNMP_NO_ERRORS), pdu_error_stat(fixs_ith_snmp::ERR_SNMP_NO_ERRORS),
	  board_slot(board_slot_), board_fbn(board_fbn_), _board_magazine_str(board_magazine_str_), _switch_board_ip(switch_board_ip_) {}

private:
	fixs_ith_snmpsessioncallback (const fixs_ith_snmpsessioncallback & rhs);


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_snmpsessioncallback Destructor
	 */
	inline virtual ~fixs_ith_snmpsessioncallback () {}


	//===========//
	// Functions //
	//===========//
public:
	inline const char * board_magazine_str () const { return _board_magazine_str; }
	inline void board_magazine_str (const char * new_value) { _board_magazine_str = new_value; }

	inline const char * switch_board_ip () const { return _switch_board_ip; }
	inline void switch_board_ip (const char * new_value) { _switch_board_ip = new_value; }

protected:
	int check_pdu_operation (int operation) const;

	int check_pdu_error_stat (int error_stat, int error_stat_in_pdu) const;

	int chech_null_var_list (netsnmp_variable_list * var_list) const;

	int check_pdu_var_type (int var_type, int expected) const;

	int check_pdu_var_size (size_t var_size, size_t min_expected) const;

	int common_pdu_checks (int operation, int error_stat, int error_stat_in_pdu, netsnmp_variable_list * var_list) const;

	virtual int read_smx (netsnmp_variable_list * var_list) const;

	//===========//
	// Operators //
	//===========//
public:
  /*
   * int operation;
   * netsnmp_session *session;    The session authenticated under.
   * int reqid;                       The request id of this pdu (0 for TRAP)
   * netsnmp_pdu *pdu;        The pdu information.
   *
   * Returns 1 if request was successful, 0 if it should be kept pending.
   * Any data in the pdu must be copied because it will be freed elsewhere.
   */
//	virtual int operator() (int operation, netsnmp_session * netsnmp_session, int request_id, netsnmp_pdu * netsnmp_pdu) const = 0;
	virtual int operator() (int operation, netsnmp_session * netsnmp_session, int request_id, netsnmp_pdu * netsnmp_pdu) const;

private:
	fixs_ith_snmpsessioncallback & operator= (const fixs_ith_snmpsessioncallback & rhs);


	//========//
	// Fields //
	//========//
public:
	mutable int error_code;
	mutable int pdu_error_stat;

	int32_t board_slot;
	uint16_t board_fbn;

private:
	const char * _board_magazine_str;
	const char * _switch_board_ip;
};



#endif /* ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONCALLBACK_H_ */
