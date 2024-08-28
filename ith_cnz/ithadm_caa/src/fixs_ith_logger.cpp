/*
 * fixs_ith_logger.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: xludesi
 */

#include <string.h>
#include <syslog.h>

#include <new>

#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"


namespace {
	char g_tra_logging_object_buffer [sizeof(ACS_TRA_Logging)];
	ACS_TRA_Logging * g_tra_logger_ptr = 0;
}


ACS_TRA_LogLevel fixs_ith_logger::_logging_level = LOG_LEVEL_INFO;


ACS_TRA_LogResult fixs_ith_logger::open (const char * appender_name) {
	if (!g_tra_logger_ptr) g_tra_logger_ptr = new (g_tra_logging_object_buffer) ACS_TRA_Logging;

	return g_tra_logger_ptr->Open(appender_name);
}

void fixs_ith_logger::close () {
	if (g_tra_logger_ptr) g_tra_logger_ptr->~ACS_TRA_Logging();
	g_tra_logger_ptr = 0;
}

ACS_TRA_LogResult fixs_ith_logger::syslogf_errno (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf(sys_errno, syslog_priority, level, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::syslogf_errno (int sys_errno, int syslog_priority, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf(sys_errno, syslog_priority, _logging_level, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::syslogf_errno_source (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf_source(sys_errno, syslog_priority, level, source_func_name, source_line, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::syslogf_errno_source (int sys_errno, int syslog_priority, const char * source_func_name, int source_line, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf_source(sys_errno, syslog_priority, _logging_level, source_func_name, source_line, format, argp);
	va_end(argp);
	return call_result;
}

ACS_TRA_LogResult fixs_ith_logger::syslogf (int syslog_priority, ACS_TRA_LogLevel level, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf(ERRNO_DISABLED_MASK, syslog_priority, level, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::syslogf (int syslog_priority, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf(ERRNO_DISABLED_MASK, syslog_priority, _logging_level, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::syslogf_source (int syslog_priority, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf_source(ERRNO_DISABLED_MASK, syslog_priority, level, source_func_name, source_line, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::syslogf_source (int syslog_priority, const char * source_func_name, int source_line, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf_source(ERRNO_DISABLED_MASK, syslog_priority, _logging_level, source_func_name, source_line, format, argp);
	va_end(argp);
	return call_result;
}

ACS_TRA_LogResult fixs_ith_logger::logf (ACS_TRA_LogLevel level, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf(ERRNO_DISABLED_MASK, SYSLOG_DISABLED_MASK, level, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::logf (const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf(ERRNO_DISABLED_MASK, SYSLOG_DISABLED_MASK, _logging_level, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::logf_source (ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf_source(ERRNO_DISABLED_MASK, SYSLOG_DISABLED_MASK, level, source_func_name, source_line, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::logf_source (const char * source_func_name, int source_line, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf_source(ERRNO_DISABLED_MASK, SYSLOG_DISABLED_MASK, _logging_level, source_func_name, source_line, format, argp);
	va_end(argp);
	return call_result;
}

ACS_TRA_LogResult fixs_ith_logger::logf_errno (int sys_errno, ACS_TRA_LogLevel level, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf(sys_errno, SYSLOG_DISABLED_MASK, level, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::logf_errno (int sys_errno, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf(sys_errno, SYSLOG_DISABLED_MASK, _logging_level, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::logf_errno_source (int sys_errno, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf_source(sys_errno, SYSLOG_DISABLED_MASK, level, source_func_name, source_line, format, argp);
	va_end(argp);
	return call_result;
}
ACS_TRA_LogResult fixs_ith_logger::logf_errno_source (int sys_errno, const char * source_func_name, int source_line, const char * format, ...) {
	va_list argp;
	va_start(argp, format);
	const ACS_TRA_LogResult call_result = vlogf_source(sys_errno, SYSLOG_DISABLED_MASK, _logging_level, source_func_name, source_line, format, argp);
	va_end(argp);
	return call_result;
}

ACS_TRA_LogResult fixs_ith_logger::vlogf (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * format, va_list ap) {
	if (!g_tra_logger_ptr) return TRA_LOG_OK;

	char buffer[8 * 1024];
	int char_count = 0;

	char_count = ::vsnprintf(buffer, FIXS_ITH_ARRAY_SIZE(buffer), format, ap);

	if (sys_errno ^ ERRNO_DISABLED_MASK) { // Add system errno information to the log message
		char errno_buf[1024];
		::snprintf(buffer + char_count, FIXS_ITH_ARRAY_SIZE(buffer) - char_count, " [errno == %d, errno_text == '%s']",
				sys_errno, ::strerror_r(sys_errno, errno_buf, FIXS_ITH_ARRAY_SIZE(errno_buf)));
	}

	if (syslog_priority ^ SYSLOG_DISABLED_MASK) ::syslog(syslog_priority, buffer);

	return g_tra_logger_ptr->Write(buffer, level);
}

ACS_TRA_LogResult fixs_ith_logger::vlogf_source (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, va_list ap) {
	if (!g_tra_logger_ptr) return TRA_LOG_OK;

	char buffer[10 * 1024];
	int char_count = 0;

	char_count = ::snprintf(buffer, FIXS_ITH_ARRAY_SIZE(buffer), "{%s@@%d} ", source_func_name, source_line);

	char_count += ::vsnprintf(buffer + char_count, FIXS_ITH_ARRAY_SIZE(buffer) - char_count, format, ap);

	if (sys_errno ^ ERRNO_DISABLED_MASK) { // Add system errno information to the log message
		char errno_buf[1024];
		::snprintf(buffer + char_count, FIXS_ITH_ARRAY_SIZE(buffer) - char_count, " [errno == %d, errno_text == '%s']",
				sys_errno, ::strerror_r(sys_errno, errno_buf, FIXS_ITH_ARRAY_SIZE(errno_buf)));
	}

	if (syslog_priority ^ SYSLOG_DISABLED_MASK) ::syslog(syslog_priority, buffer);

	return g_tra_logger_ptr->Write(buffer, level);
}



