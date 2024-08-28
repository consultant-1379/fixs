/*
 * FIXS_CMXH_Logger.h
 *
 */

#ifndef FIXS_CMXH_LOGGER_H_
#define FIXS_CMXH_LOGGER_H_

#include "ACS_TRA_Logging.h"

class FIXS_CMXH_Logger {

public:
	static ACS_TRA_Logging * getLogInstance ();
	static void closeLogInstance ();

private:
	FIXS_CMXH_Logger();
	virtual ~FIXS_CMXH_Logger();
	static ACS_TRA_Logging* FIXS_CMXH_logging;
};

#endif /* FIXS_CMXH_LOGGER_H_ */
