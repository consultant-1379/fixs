#ifndef __INCLUDE_GUARD_CLASS_FIXS_ITH_BRF_POLICY_HANDLER__
#define __INCLUDE_GUARD_CLASS_FIXS_ITH_BRF_POLICY_HANDLER__ fixs_ith_brf_policy_handler

/*
 *	@file fixs_ith_brf_policy_handler.h
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

class fixs_ith_brf_policy_handler {
	//=======//
	// CTORs //
	//=======//
private:
	fixs_ith_brf_policy_handler ();
	fixs_ith_brf_policy_handler (const fixs_ith_brf_policy_handler & rhs);

	//======//
	// DTOR //
	//======//
public:
	inline ~fixs_ith_brf_policy_handler () {}

	//===========//
	// Functions //
	//===========//
public:
	static int initialize ();

	static int restart_due_to_a_restore ();

	static inline int restore_ongoing () {
		return (_restore_status ? _restore_status->restore_ongoing : -2);
	}

	static inline void restore_ongoing (int new_value) {
		_restore_status && (_restore_status->restore_ongoing = (new_value ? 1 : 0));
	}

	static inline int board_restore_ongoing (unsigned index) {
		return (
				_restore_status && (index < FIXS_ITH_ARRAY_SIZE(restore_status_t::boards_restore_status))
					? _restore_status->boards_restore_status[index].board_restore_ongoing
					: -2
		);
	}

	static inline void board_restore_ongoing (unsigned index, int new_value) {
		_restore_status
		&& (index < FIXS_ITH_ARRAY_SIZE(restore_status_t::boards_restore_status))
		&& (_restore_status->boards_restore_status[index].board_restore_ongoing = (new_value ? 1 : 0));
	}

private:
	static int file_exists (const char * pathname);
	static int create_restore_status_file (const char * pathname);
	static int init_restore_status_file (int fd);
	static int map_file_in_memory (const char * pathname);


//	static int application_restarted_due_restore ();
//	static int make_storage_clear_dir ();

	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_brf_policy_handler & operator= (const fixs_ith_brf_policy_handler & rhs);

	//================//
	// Sub structures //
	//================//
private:
	struct restore_status_t {
		signed char restart_due_to_a_restore;
		signed char restore_ongoing;
		struct boards_restore_status_t {
			signed char board_restore_ongoing;
		} boards_restore_status [128];
	};

	//========//
	// Fields //
	//========//
private:
	static int _initialized;
	static char _status_file_name [];
	static int _status_file_create_flags;
	static mode_t _status_file_create_modes;
	static restore_status_t * _restore_status;


//	static int _application_restarted_due_restore;
};

#endif /* __INCLUDE_GUARD_CLASS_FIXS_ITH_BRF_POLICY_HANDLER__ */
