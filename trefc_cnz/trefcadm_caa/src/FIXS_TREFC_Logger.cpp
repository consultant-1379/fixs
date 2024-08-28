/*
 * FIXS_TREFC_Logger.cpp
 *
 *  Created on: Mar 12, 2012
 *      Author: estevol
 */
#include "FIXS_TREFC_Logger.h"

ACS_TRA_Logging* FIXS_TREFC_Logger::FIXS_TREFC_logging = NULL;

ACS_TRA_Logging* FIXS_TREFC_Logger::getLogInstance ()
{
	if (FIXS_TREFC_logging == NULL)
	{
		FIXS_TREFC_logging = new ACS_TRA_Logging();
		FIXS_TREFC_logging->Open("TREFC");
		FIXS_TREFC_logging->Write(" FIXS_TREFC - Create Instance for logging !!! ",LOG_LEVEL_DEBUG);
	}

	return FIXS_TREFC_logging;
}

void FIXS_TREFC_Logger::closeLogInstance ()
{
	if (FIXS_TREFC_logging)
	{
		FIXS_TREFC_logging->Write(" FIXS_TREFC - Destroy Instance for logging !!! ",LOG_LEVEL_DEBUG);
		FIXS_TREFC_logging->Close();
		delete(FIXS_TREFC_logging);
		FIXS_TREFC_logging=NULL;
	}
}
