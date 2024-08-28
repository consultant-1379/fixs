/*
 * fixs_ith_snmpsessionhandler.h
 *
 *  Created on: Jan 28, 2016
 *      Author: xassore
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONHANDLER_H_
#define ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONHANDLER_H_


#include <ace/Recursive_Thread_Mutex.h>

#include "fixs_ith_programmacros.h"
#include "fixs_ith_snmpsessioninfo.h"
#include "fixs_ith_snmpsessioncallback.h"

#include "fixs_ith_snmpconstants.h"

class fixs_ith_snmpsessionhandler {

	//==============//
	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_snmpsessionhandler constructor
	 */
	inline fixs_ith_snmpsessionhandler () // TR HX65471
	: _session_handle(0){}

private:
	fixs_ith_snmpsessionhandler (const fixs_ith_snmpsessionhandler & rhs);


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_snmpsessionhandler Destructor
	 */
	inline ~fixs_ith_snmpsessionhandler () { close(); }


	//============//
	// Functions  //
	//============//
public:
	inline fixs_ith_snmp::SnmpSessionStateConstants state () const { return (_session_handle ? fixs_ith_snmp::SNMP_SESSION_STATE_OPEN : fixs_ith_snmp::SNMP_SESSION_STATE_CLOSED); }
	inline int open (fixs_ith_snmpsessioninfo & session_info) { return open_(session_info); }

	inline int open (fixs_ith_snmpsessioninfo & session_info, fixs_ith_snmpsessioncallback & callback) { return open_(session_info, &callback); }

	int close ();

	inline int get_fd_set (int & fd_count, fd_set & set) const { int dummy = 0; return get_fd_set(fd_count, dummy, set); }
	int get_fd_set (int & fd_count, int & fd_sup, fd_set & set) const;

	int read () const;
	inline int read (fd_set & set) const {
		return snmp_sess_read(_session_handle, &set) ? fixs_ith_snmp::ERR_SNMP_SESSION_READ : fixs_ith_snmp::ERR_SNMP_NO_ERRORS;
	}

	int send_synch (netsnmp_pdu * & request, const unsigned * timeout_ms) const;

private:
	int open_ (fixs_ith_snmpsessioninfo & session_info, fixs_ith_snmpsessioncallback * callback = 0);

	static int session_callback_dispatcher (int operation, netsnmp_session * netsnmp_session, int request_id, netsnmp_pdu * netsnmp_pdu, void * session_callback);


	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_snmpsessionhandler & operator= (const fixs_ith_snmpsessionhandler & rhs);


	//========//
	// Fields //
	//========//
private:
	void * _session_handle;

	// Access synchronization controllers
 	static ACE_Recursive_Thread_Mutex _sync_object; // TR HX65471 
};


#endif /* ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONHANDLER_H_ */
