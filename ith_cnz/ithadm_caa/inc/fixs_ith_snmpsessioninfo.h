/*
 * fixs_ith_snmpsessioninfo.h
 *
 *  Created on: Jan 27, 2016
 *      Author: xassore
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONINFO_H_
#define ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONINFO_H_

#include <stdlib.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/transform_oids.h> // To get using SNMP Version 3


/** @class fixs_ith_snmpsessioninfo fixs_ith_snmpsessioninfo.h
 *	@brief fixs_ith_snmpsessioninfo class
 *	@author
 *	@date 2016-01-27
 *
 *	fixs_ith_snmpsessioninfo <PUT DESCRIPTION>
 */

class fixs_ith_snmpsessioninfo : protected snmp_session {
	//=========//
	// Friends //
	//=========//
	friend class fixs_ith_snmpsessionhandler;


	//==============//
	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_snmpsessioninfo constructor
	 */
	inline fixs_ith_snmpsessioninfo () : snmp_session(), _last_snmp_error_text(0) { snmp_sess_init(this); }

	inline fixs_ith_snmpsessioninfo (const fixs_ith_snmpsessioninfo & rhs) : snmp_session(rhs), _last_snmp_error_text(0) {
		if (rhs._last_snmp_error_text) _last_snmp_error_text = ::strdup(rhs._last_snmp_error_text);
	}


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_snmpsessioninfo Destructor
	 */
	inline ~fixs_ith_snmpsessioninfo () { ::free(const_cast<char *>(_last_snmp_error_text)); }


	//================//
	// Field accessor //
	//================//
public:
	inline long version () const { return snmp_session::version; }
	inline void version (long new_value) { snmp_session::version = new_value; }

	inline const char * peername () const { return snmp_session::peername; }
	void peername (char * new_value);

	inline const char * community () const { return reinterpret_cast<char *>(snmp_session::community); }
	void community (char * new_value);

	inline u_short local_port () const { return snmp_session::local_port; }
	inline void local_port (u_short new_value) { snmp_session::local_port = new_value; }

	inline int last_sys_error () const { return snmp_session::s_errno; }
	inline int last_snmp_error () const { return snmp_session::s_snmp_errno; }
	inline const char * last_snmp_error_text () const { return _last_snmp_error_text ?: "OK"; }
	inline void reset_errors () { snmp_session::s_errno = 0; snmp_session::s_snmp_errno = 0; last_snmp_error_text(0); }

protected:
	inline void last_snmp_error_text (const char * new_ptr) { ::free(const_cast<char *>(_last_snmp_error_text)); _last_snmp_error_text = new_ptr; }
	inline void last_snmp_error_text_dup (const char * str_to_copy) {
		::free(const_cast<char *>(_last_snmp_error_text));
		_last_snmp_error_text = (str_to_copy ? ::strdup(str_to_copy) : 0);
	}


	//============//
	// Functions  //
	//============//
public:


	//===========//
	// Operators //
	//===========//
public:
	inline fixs_ith_snmpsessioninfo & operator= (const fixs_ith_snmpsessioninfo & rhs) {
		if (this != &rhs) {
			last_snmp_error_text_dup(rhs._last_snmp_error_text);
			snmp_session::operator=(rhs);
		}
		return *this;
	}


	//========//
	// Fields //
	//========//
private:
	const char * _last_snmp_error_text;
};

#endif /* ITHADM_CAA_INC_FIXS_ITH_SNMPSESSIONINFO_H_ */
