/*
 * fixs_ith_logger.h
 *
 *  Created on: Jan 25, 2016
 *      Author: xludesi
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_LOGGER_H_
#define ITHADM_CAA_INC_FIXS_ITH_LOGGER_H_

#include <stdarg.h>

#include <ACS_TRA_Logging.h>
#include <syslog.h>


/** @class fixs_ith_logger fixs_ith_logger.h
 *	@brief fixs_ith_logger class
 *	@author xnicmut (Nicola Muto)
 *	@date
 *
 *	fixs_ith_logger Class detailed description
 */
class fixs_ith_logger {
	//===========//
	// Constants //
	//===========//
private:
	enum {
		SYSLOG_DISABLED_MASK	=	99999,
		ERRNO_DISABLED_MASK		=	-99999
	};


	//==============//
	// Constructors //
	//==============//
private:
	/** @brief fixs_ith_logger Default constructor
	 *
	 *	fixs_ith_logger Constructor detailed description
	 *
	 *	@param[in] logging_level Description
	 *	@remarks Remarks
	 */
	fixs_ith_logger ();

	/** @brief fixs_ith_logger Copy constructor
	 *
	 *	fixs_ith_logger Copy Constructor detailed description
	 *
	 *	@param[in] rhs Description
	 *	@remarks Remarks
	 */
	fixs_ith_logger (const fixs_ith_logger & rhs);


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_logger Destructor
	 *
	 *	fixs_ith_logger Destructor detailed description
	 *
	 *	@remarks Remarks
	 */
	inline ~fixs_ith_logger () {}


	//===========//
	// Functions //
	//===========//
public:
	static ACS_TRA_LogResult open (const char * appender_name);
	static void close ();

	static ACS_TRA_LogResult syslogf_errno (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * format, ...);
	static ACS_TRA_LogResult syslogf_errno (int sys_errno, int syslog_priority, const char * format, ...);
	static ACS_TRA_LogResult syslogf_errno_source (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, ...);
	static ACS_TRA_LogResult syslogf_errno_source (int sys_errno, int syslog_priority, const char * source_func_name, int source_line, const char * format, ...);

	static ACS_TRA_LogResult syslogf (int syslog_priority, ACS_TRA_LogLevel level, const char * format, ...);
	static ACS_TRA_LogResult syslogf (int syslog_priority, const char * format, ...);
	static ACS_TRA_LogResult syslogf_source (int syslog_priority, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, ...);
	static ACS_TRA_LogResult syslogf_source (int syslog_priority, const char * source_func_name, int source_line, const char * format, ...);

	static ACS_TRA_LogResult logf (ACS_TRA_LogLevel level, const char * format, ...);
	static ACS_TRA_LogResult logf (const char * format, ...);
	static ACS_TRA_LogResult logf_source (ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, ...);
	static ACS_TRA_LogResult logf_source (const char * source_func_name, int source_line, const char * format, ...);

	static ACS_TRA_LogResult logf_errno_source (int sys_errno, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, ...);
	static ACS_TRA_LogResult logf_errno_source (int sys_errno, const char * source_func_name, int source_line, const char * format, ...);
	static ACS_TRA_LogResult logf_errno (int sys_errno, ACS_TRA_LogLevel level, const char * format, ...);
	static ACS_TRA_LogResult logf_errno (int sys_errno, const char * format, ...);

	static inline ACS_TRA_LogResult log (const char * message) { return logf(_logging_level, message); }
	static inline ACS_TRA_LogResult log (ACS_TRA_LogLevel level, const char * message) { return logf(level, message); }
	static inline ACS_TRA_LogResult log (const char * source_func_name, int source_line, const char * message) {
		return logf_source(_logging_level, source_func_name, source_line, message);
	}
	static inline ACS_TRA_LogResult log (ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * message) {
		return logf_source(level, source_func_name, source_line, message);
	}

	static inline ACS_TRA_LogResult log_errno (int sys_errno, const char * message) { return logf_errno(sys_errno, _logging_level, message); }
	static inline ACS_TRA_LogResult log_errno (int sys_errno, ACS_TRA_LogLevel level, const char * message) { return logf_errno(sys_errno, level, message); }
	static inline ACS_TRA_LogResult log_errno (int sys_errno, const char * source_func_name, int source_line, const char * message) {
		return logf_errno_source(sys_errno, _logging_level, source_func_name, source_line, message);
	}
	static inline ACS_TRA_LogResult log_errno (int sys_errno, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * message) {
		return logf_errno_source(sys_errno, level, source_func_name, source_line, message);
	}

	static inline ACS_TRA_LogResult syslog (int syslog_priority, const char * message) { return syslogf(syslog_priority, _logging_level, message); }
	static inline ACS_TRA_LogResult syslog (int syslog_priority, ACS_TRA_LogLevel level, const char * message) { return syslogf(syslog_priority, level, message); }
	static inline ACS_TRA_LogResult syslog (int syslog_priority, const char * source_func_name, int source_line, const char * message) {
		return syslogf_source(syslog_priority, _logging_level, source_func_name, source_line, message);
	}
	static inline ACS_TRA_LogResult syslog (int syslog_priority, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * message) {
		return syslogf_source(syslog_priority, level, source_func_name, source_line, message);
	}

	static inline ACS_TRA_LogResult syslog_errno (int sys_errno, int syslog_priority, const char * message) { return syslogf_errno(sys_errno, syslog_priority, _logging_level, message); }
	static inline ACS_TRA_LogResult syslog_errno (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * message) { return syslogf_errno(sys_errno, syslog_priority, level, message); }
	static inline ACS_TRA_LogResult syslog_errno (int sys_errno, int syslog_priority, const char * source_func_name, int source_line, const char * message) {
		return syslogf_errno_source(sys_errno, syslog_priority, _logging_level, source_func_name, source_line, message);
	}
	static inline ACS_TRA_LogResult syslog_errno (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * message) {
		return syslogf_errno_source(sys_errno, syslog_priority, level, source_func_name, source_line, message);
	}

	static inline ACS_TRA_LogLevel logging_level () { return _logging_level; }
	static inline void logging_level (ACS_TRA_LogLevel level) { _logging_level = level; }


private:
	static ACS_TRA_LogResult vlogf (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * format, va_list ap);
	static ACS_TRA_LogResult vlogf_source (int sys_errno, int syslog_priority, ACS_TRA_LogLevel level, const char * source_func_name, int source_line, const char * format, va_list ap);


	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_logger & operator= (const fixs_ith_logger & rhs);


	//========//
	// Fields //
	//========//
private:
	static ACS_TRA_LogLevel _logging_level;
};



#endif /* ITHADM_CAA_INC_FIXS_ITH_LOGGER_H_ */
