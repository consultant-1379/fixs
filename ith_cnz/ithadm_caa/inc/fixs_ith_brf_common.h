#ifndef __INCLUDE_GUARD_CLASS_FIXS_ITH_BRF_COMMON__
#define __INCLUDE_GUARD_CLASS_FIXS_ITH_BRF_COMMON__ fixs_ith_brf_common

/*
 *	@file fixs_ith_brf_common.h
 *	@brief
 *	@author xnicmut (Nicola Muto)
 *	@date 2016-09-07
 *
 *	COPYRIGHT Ericsson AB, 2016
 *	All rights reserved.
 *
 *	The information in this document is the property of Ericsson.
 *	Except as specifically authorized in writing by Ericsson, the receiver of
 *	this document shall keep the information contained herein confidential and
 *	shall protect the same in whole or in part from disclosure and dissemination
 *	to third parties. Disclosure and disseminations to the receivers employees
 *	shall only be made on a strict need to know basis.
 *
 *	REVISION INFO
 *	+==========+============+==============+=====================================+
 *	| REV      | DATE       | AUTHOR       | DESCRIPTION                         |
 *	+==========+============+==============+=====================================+
 *	| 2016-001 | 2016-09-07 | xnicmut      | Creation and first revision.        |
 *	+==========+============+==============+=====================================+
 */

#include <string>

class fixs_ith_brf_common {
	//=======//
	// CTORs //
	//=======//
private:
	fixs_ith_brf_common ();
	fixs_ith_brf_common (const fixs_ith_brf_common & rhs);

	//======//
	// DTOR //
	//======//
public:
	inline ~fixs_ith_brf_common () {}

	//===========//
	// Functions //
	//===========//
public:
	static const std::string & cba_pso_storage_path_clear ();

	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_brf_common & operator= (const fixs_ith_brf_common & rhs);

	//========//
	// Fields //
	//========//
private:
};

#endif /* __INCLUDE_GUARD_CLASS_FIXS_ITH_BRF_COMMON__ */
