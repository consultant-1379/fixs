/*
 * FIXS_TREFC_Logger.h
 *
 *  Created on: Mar 12, 2012
 *      Author: estevol
 */

#ifndef FIXS_TREFC_LOGGER_H_
#define FIXS_TREFC_LOGGER_H_

#include "ACS_TRA_Logging.h"

class FIXS_TREFC_Logger {

public:
	static ACS_TRA_Logging * getLogInstance ();
	static void closeLogInstance ();

private:
	FIXS_TREFC_Logger();
	virtual ~FIXS_TREFC_Logger();
	static ACS_TRA_Logging* FIXS_TREFC_logging;
};

#endif /* FIXS_TREFC_LOGGER_H_ */
