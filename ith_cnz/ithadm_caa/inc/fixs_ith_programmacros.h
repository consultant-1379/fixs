/*
 * fixs_ith_programmacros.h
 *
 *  Created on: Jan 25, 2016
 *      Author: xludesi
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_PROGRAMMACROS_H_
#define ITHADM_CAA_INC_FIXS_ITH_PROGRAMMACROS_H_

/** @file fixs_ith_programmacros.h
 *	@brief
 *	@author xnicmut (Nicola Muto)
 *	@date
 *
 *	COPYRIGHT Ericsson AB, 2010
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
 *	+=======+============+==============+=====================================+
 *	| REV   | DATE       | AUTHOR       | DESCRIPTION                         |
 *	+=======+============+==============+=====================================+
 *	| R-001 | 2011-07-19 | xnicmut      | Creation and first revision.        |
 *	+=======+============+==============+=====================================+
 */

/*
 * Macro utilities
 */
#define FIXS_ITH_ARRAY_SIZE(array) (sizeof(array)/sizeof(*(array)))
#define FIXS_ITH_CONDITIONAL_PROCEDURE_CALL(cond, proc, ...) if (cond) proc(__VA_ARGS__)

#define FIXS_ITH_STRINGIZER(s) #s
#define FIXS_ITH_STRINGIZE(s) FIXS_ITH_STRINGIZER(s)

/*
 * Logging macros
 */
#define FIXS_ITH_LOGGER_CLASS fixs_ith_logger

#define FIXS_ITH_LOGGER_METHOD logf
#define FIXS_ITH_LOGGER_ERRNO_METHOD logf_errno
#define FIXS_ITH_SYSLOGGER_METHOD syslogf
#define FIXS_ITH_SYSLOGGER_ERRNO_METHOD syslogf_errno

#define FIXS_ITH_LOGGER_METHOD_SOURCE logf_source
#define FIXS_ITH_LOGGER_ERRNO_METHOD_SOURCE logf_errno_source
#define FIXS_ITH_SYSLOGGER_METHOD_SOURCE syslogf_source
#define FIXS_ITH_SYSLOGGER_ERRNO_METHOD_SOURCE syslogf_errno_source


#define FIXS_ITH_LOG_IMPL(level, ...) FIXS_ITH_LOGGER_CLASS::FIXS_ITH_LOGGER_METHOD(level, __VA_ARGS__)
#define FIXS_ITH_LOG_ERRNO_IMPL(sys_errno, level, ...) FIXS_ITH_LOGGER_CLASS::FIXS_ITH_LOGGER_ERRNO_METHOD(sys_errno, level, __VA_ARGS__)
#define FIXS_ITH_SYSLOG_IMPL(syslog_priority, level, ...) FIXS_ITH_LOGGER_CLASS::FIXS_ITH_SYSLOGGER_METHOD(syslog_priority, level, __VA_ARGS__)
#define FIXS_ITH_SYSLOG_ERRNO_IMPL(sys_errno, syslog_priority, level, ...) FIXS_ITH_LOGGER_CLASS::FIXS_ITH_SYSLOGGER_ERRNO_METHOD(sys_errno, syslog_priority, level, __VA_ARGS__)

#define FIXS_ITH_LOG_SOURCE_IMPL(level, source, line, ...) FIXS_ITH_LOGGER_CLASS::FIXS_ITH_LOGGER_METHOD_SOURCE(level, source, line, __VA_ARGS__)
#define FIXS_ITH_LOG_SOURCE_ERRNO_IMPL(sys_errno, level, source, line, ...) FIXS_ITH_LOGGER_CLASS::FIXS_ITH_LOGGER_ERRNO_METHOD_SOURCE(sys_errno, level, source, line, __VA_ARGS__)
#define FIXS_ITH_SYSLOG_SOURCE_IMPL(syslog_priority, level, source, line, ...) FIXS_ITH_LOGGER_CLASS::FIXS_ITH_SYSLOGGER_METHOD_SOURCE(syslog_priority, level, source, line, __VA_ARGS__)
#define FIXS_ITH_SYSLOG_SOURCE_ERRNO_IMPL(sys_errno, syslog_priority, level, source, line, ...) FIXS_ITH_LOGGER_CLASS::FIXS_ITH_SYSLOGGER_ERRNO_METHOD_SOURCE(sys_errno, syslog_priority, level, source, line, __VA_ARGS__)

#define UNUSED(expr) do { (void)(expr); } while (0)

#ifdef FIXS_ITH_LOGGING_ENABLED
#	ifdef FIXS_ITH_LOGGING_HAS_SOURCE_INFO
#		ifdef FIXS_ITH_LOGGING_USE_PRETTY_FUNCTION
#			define FIXS_ITH_LOG(level, ...) FIXS_ITH_LOG_SOURCE_IMPL(level, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)
#			define FIXS_ITH_LOG_ERRNO(sys_errno, level, ...) FIXS_ITH_LOG_SOURCE_ERRNO_IMPL(sys_errno, level, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)
#			define FIXS_ITH_SYSLOG(syslog_priority, level, ...) FIXS_ITH_SYSLOG_SOURCE_IMPL(syslog_priority, level, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)
#			define FIXS_ITH_SYSLOG_ERRNO(sys_errno, syslog_priority, level, ...) FIXS_ITH_SYSLOG_SOURCE_ERRNO_IMPL(sys_errno, syslog_priority, level, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)
#		else
#			define FIXS_ITH_LOG(level, ...) FIXS_ITH_LOG_SOURCE_IMPL(level, __func__, __LINE__, __VA_ARGS__)
#			define FIXS_ITH_LOG_ERRNO(sys_errno, level, ...) FIXS_ITH_LOG_SOURCE_ERRNO_IMPL(sys_errno, level, __func__, __LINE__, __VA_ARGS__)
#			define FIXS_ITH_SYSLOG(syslog_priority, level, ...) FIXS_ITH_SYSLOG_SOURCE_IMPL(syslog_priority, level, __func__, __LINE__, __VA_ARGS__)
#			define FIXS_ITH_SYSLOG_ERRNO(sys_errno, syslog_priority, level, ...) FIXS_ITH_SYSLOG_SOURCE_ERRNO_IMPL(sys_errno, syslog_priority, level, __func__, __LINE__, __VA_ARGS__)
#		endif // FIXS_ITH_LOGGING_USE_PRETTY_FUNCTION
#	else
#		define FIXS_ITH_LOG(level, ...) FIXS_ITH_LOG_IMPL(level, __VA_ARGS__)
#		define FIXS_ITH_LOG_ERRNO(sys_errno, level, ...) FIXS_ITH_LOG_ERRNO_IMPL(sys_errno, level, __VA_ARGS__)
#		define FIXS_ITH_SYSLOG(syslog_priority, level, ...) FIXS_ITH_SYSLOG_IMPL(syslog_priority, level, __VA_ARGS__)
#		define FIXS_ITH_SYSLOG_ERRNO(sys_errno, syslog_priority, level, ...) FIXS_ITH_SYSLOG_ERRNO_IMPL(sys_errno, syslog_priority, level, __VA_ARGS__)
#	endif //FIXS_ITH_LOGGING_HAS_SOURCE_INFO
#else
#	define FIXS_ITH_LOG(level, ...) FIXS_ITH_LOG_NO_LOGGING()
#	define FIXS_ITH_LOG_ERRNO(sys_errno, level, ...) FIXS_ITH_LOG_NO_LOGGING()
#	define FIXS_ITH_SYSLOG(syslog_priority, level, ...) FIXS_ITH_LOG_NO_LOGGING()
#	define FIXS_ITH_SYSLOG_ERRNO(sys_errno, syslog_priority, level, ...) FIXS_ITH_LOG_NO_LOGGING()
#endif //FIXS_ITH_LOGGING_ENABLED


inline int FIXS_ITH_LOG_NO_LOGGING () { return 0; }


#ifndef FIXS_ITH_CRITICAL_SECTION_GUARD_BEGIN
#	define	FIXS_ITH_CRITICAL_SECTION_GUARD_BEGIN(sync) do { ACE_Guard<ACE_Recursive_Thread_Mutex> guard(sync, true)
#	define	FIXS_ITH_CRITICAL_SECTION_GUARD_END()	} while (0)
#endif



#endif /* ITHADM_CAA_INC_FIXS_ITH_PROGRAMMACROS_H_ */
