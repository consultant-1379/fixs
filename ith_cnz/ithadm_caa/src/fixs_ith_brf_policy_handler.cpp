/*
 *	@file fixs_ith_brf_policy_handler.cpp
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <vector>

#include "acs_apgcc_omhandler.h"

#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_brf_common.h"
#include "imm/imm.h"
#include "fixs_ith_brf_policy_handler.h"

namespace {
	const int RETRY_MAX = 5;

	int imm_configuration_tree_size () {
		OmHandler om_handler;

		ACS_CC_ReturnType imm_call_result = om_handler.Init();
		if (imm_call_result != ACS_CC_SUCCESS) {
			// ERROR: Initializing OM handler to access IMM configuration
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call to 'om_handler.Init()' failed: Cannot initialize OM IMM handler: imm_call_result == <%d>", imm_call_result);
			return -1;
		}

		int size = -1; // Suppose IMM error

		// Try to fetch the configuration tree
		std::vector<std::string> ith_imm_config_tree;

		if ((imm_call_result = om_handler.getChildren(imm::mom_dn::ROOT_MOC_DN, ACS_APGCC_SUBTREE, &ith_imm_config_tree)) != ACS_CC_SUCCESS) {
			int imm_internal_last_error = om_handler.getInternalLastError();

			if (imm_internal_last_error == -12) size = -2; // Root object NOT FOUND

			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to 'om_handler.getChildren()' failed: Cannot get the ITH IMM "
					"config tree to evaluate its size: imm_call_result == <%d>, imm_internal_last_error == <%d>",
					imm_call_result, imm_internal_last_error);
		} else {
			size = static_cast<int>(ith_imm_config_tree.size());
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Now ITH server has a IMM configuration tree with a size == <%d>", size);
		}

		om_handler.Finalize();

		return size;
	}
}

/*
 * Meaning of values
 * 	-2: Uninitialized (First time call) or Persistent error
 * 	-1: ERROR: Cannot check/create the application directory
 * 	 0: OK: Normal restart
 * 	 1: OK: Restart due to a restore procedure
 *
 * In case of -1, 0 and 1 values non other calls will be performed.
 */
//int fixs_ith_brf_policy_handler::_application_restarted_due_restore = -2;

int fixs_ith_brf_policy_handler::_initialized = 0;
char fixs_ith_brf_policy_handler::_status_file_name [] = "restore_status.bin";
int fixs_ith_brf_policy_handler::_status_file_create_flags = O_RDWR | O_CREAT | O_EXCL;
mode_t fixs_ith_brf_policy_handler::_status_file_create_modes = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; // rw-rw-r--
fixs_ith_brf_policy_handler::restore_status_t * fixs_ith_brf_policy_handler::_restore_status = 0;

int fixs_ith_brf_policy_handler::file_exists (const char * pathname) {
	struct stat sbuf;

	int return_code = 1; // Suppose file exists

	int call_result = stat(pathname, &sbuf);
	if (call_result < 0) { // ERROR: calling stat sys-call
		int errno_save = errno;

		return_code = ((errno_save == ENOENT) ? 0 : -1);

		if (return_code < 0) FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'stat()' failed: errno == %d", errno_save);
	}

	return return_code;
}

int fixs_ith_brf_policy_handler::init_restore_status_file (int fd) {
	// Seek file pointer to the start
	off_t seek_result = lseek(fd, 0, SEEK_SET);
	if (seek_result == static_cast<off_t>(-1)) { // ERROR: on seeking to the begin of the restore file status
		int errno_save = errno;
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to lseek() failed: cannot seek to the begin of the restore file status: "
				"fd == <%d>: errno_save == <%d>", fd, errno_save);
		return -1;
	}

	restore_status_t initial_restore_status;

	initial_restore_status.restart_due_to_a_restore = -1;
	initial_restore_status.restore_ongoing = -1;
	for (size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(restore_status_t::boards_restore_status); ++i) {
		initial_restore_status.boards_restore_status[i].board_restore_ongoing = -1;
	}

	// Write initial values into the restore file status
	int call_result = 0;
	unsigned char * buf_to_write = reinterpret_cast<unsigned char *>(&initial_restore_status);
	size_t bytes_to_write = sizeof(initial_restore_status);
	int retry = RETRY_MAX;

	while (retry > 0) {
		call_result = write(fd, buf_to_write, bytes_to_write);
		if (call_result < 0) { // ERROR: Writing into the file
			--retry;
			int errno_save = errno;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to write() failed: cannot initialize the restore status file: fd == <%d>: "
					"errno_save == <%d>: retry == <%d>", fd, errno_save, retry);
		} else { // OK: Wrote call_result bytes
			retry = RETRY_MAX;
			if ((bytes_to_write -= static_cast<size_t>(call_result)) == 0) break;
			buf_to_write += static_cast<size_t>(call_result);
		}
	}

	if (retry <= 0) { // ERROR: File initialization failed
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Restore status file failed: fd == <%d>", fd);
	}

	return ((retry > 0) ? 0 : -1);
}

int fixs_ith_brf_policy_handler::create_restore_status_file (const char * pathname) {
	// Create the directory if doesn't exist
	const char * const path = fixs_ith_brf_common::cba_pso_storage_path_clear().c_str();

	int call_result = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	int errno_save = errno;

	if ((call_result < 0) && (errno_save != EEXIST)) {
		// ERROR: Creating the directory. Check errno to understand how ith server was restarted
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to 'mkdir()' failed: creating the application clear "
			"storage directory <%s>: errno == <%d>", path, errno_save);

		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Storage directory <%s> created successfully or already there: errno == <%d>", path, errno_save);

	int fd = open(pathname, _status_file_create_flags, _status_file_create_modes);
	if (fd < 0) { // ERROR: Creating the restore status memory file
		int errno_save = errno;
		if (errno_save == EEXIST) // The file already exists. Leave it as is
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "The restore status file <%s> already exists", pathname);
		else // ERROR: Opening the restore status file
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to open() failed: pathname == <%s>: errno_save == <%d>", pathname, errno_save);

		return ((errno_save == EEXIST) ? 0 : -1);
	}

	//Initialize the file
	call_result = init_restore_status_file(fd);

	close(fd);

	if (call_result < 0) { // ERROR: Initializing the restore status file
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to init_restore_status_file() failed: pathname == <%s>: fd == <%d>", pathname, fd);

		// Delete the file
		if (unlink(pathname) < 0) {
			int errno_save = errno;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to unlink() failed: pathname == <%s>: errno_save == <%d>", pathname, errno_save);
		}
	}

	return ((call_result < 0) ? -1 : 0);
}

int fixs_ith_brf_policy_handler::map_file_in_memory (const char * pathname) {
	int fd = open(pathname, O_RDWR);
	if (fd < 0) { // ERROR: opening the restore status file
		int errno_save = errno;
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call open() failed: pathname == <%s>: errno_save == %d", pathname, errno_save);
		return -1;
	}

	int return_code = 0;
	void * p = mmap(0, sizeof(restore_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) { // ERROR: Mapping the restore status file into the process memory
		int errno_save = errno;
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call mmap() failed: pathname == <%s>: errno_save == %d", pathname, errno_save);
		return_code = -1;
	} else {
		_restore_status = reinterpret_cast<restore_status_t *>(p);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Mapping OK: pathname == <%s>", pathname);
	}

	close(fd);

	return return_code;
}

int fixs_ith_brf_policy_handler::initialize () {
	if (_initialized) return 0;
	_initialized = 1;

	char status_file_path [4096] = {0};
	snprintf(status_file_path, sizeof(status_file_path), "%s/%s",
			fixs_ith_brf_common::cba_pso_storage_path_clear().c_str(), _status_file_name);

	int call_result = 0;

	// Create and initialize the restore status file. If the file already exists do nothing
	call_result = create_restore_status_file(status_file_path);
	if (call_result < 0) { // ERROR: Creating the restore status file
		_initialized = 0;
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'create_restore_status_file()' failed: status_file_path == <%s>: call_result == %d",
				status_file_path, call_result);
		return call_result;
	}

	call_result = map_file_in_memory(status_file_path);
	if (call_result < 0) { // ERROR: Creating the restore status file
		_initialized = 0;
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'map_file_in_memory()' failed: call_result == %d", call_result);
		return call_result;
	}

	return 0;
}

int fixs_ith_brf_policy_handler::restart_due_to_a_restore () {
	if (!_restore_status) return -2;

	if (_restore_status->restart_due_to_a_restore < 0) {
		int size = imm_configuration_tree_size();
		if (size == -1) {
			// ERROR: Cannot get the ITH IMM configuration tree
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to 'imm_configuration_tree_size()' failed");
		} else {
			_restore_status->restart_due_to_a_restore = 0;
			signed char restore_ongoing = ((size >= 0) ? 1 : 0);
			_restore_status->restore_ongoing = restore_ongoing;

			for (size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(restore_status_t::boards_restore_status); ++i) {
				_restore_status->boards_restore_status[i].board_restore_ongoing = restore_ongoing;
			}

			return restore_ongoing;
		}
	}

	return _restore_status->restart_due_to_a_restore;
}

#if 0 // TO BE DELETE
int fixs_ith_brf_policy_handler::application_restarted_due_restore () {
	if (_application_restarted_due_restore >= -1) return _application_restarted_due_restore;

	const char * const path = fixs_ith_brf_common::cba_pso_storage_path_clear().c_str();

	struct stat sbuf;

	if ((_application_restarted_due_restore = (stat(path, &sbuf) ? 1 : 0))) {
		// It seems ITH Server was restarted due to a restore procedure.
		// Now IMM configuration should be checked to be sure that ITH server was restarted due to a restore procedure
		int size = imm_configuration_tree_size();

		if (size == -1) {
			// ERROR: Cannot get the ITH IMM configuration tree
			_application_restarted_due_restore = -2;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to 'imm_configuration_tree_size()' failed");
		} else _application_restarted_due_restore = ((size >= 0) ? 1 : 0);
	}

	if (_application_restarted_due_restore >= 0) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ITH server was restarted %s",
			(_application_restarted_due_restore ? "due a restore procedure" : "normally"));
	}

	return _application_restarted_due_restore;
}

int fixs_ith_brf_policy_handler::make_storage_clear_dir () {
	_application_restarted_due_restore = 0;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Restore operation success: resetting internal state (_application_restarted_due_restore) to 0 (zero)");

	const char * const path = fixs_ith_brf_common::cba_pso_storage_path_clear().c_str();

	if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) && (errno != EEXIST)) {
		// ERROR: Creating the directory. Check errno to understand how ith server was restarted
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call to 'mkdir()' failed: creating the application clear "
				"storage directory <%s>: errno == <%d>", path, errno);

		return fixs_ith::ERR_MKDIR;
	}

	return fixs_ith::ERR_NO_ERRORS;
}
#endif
