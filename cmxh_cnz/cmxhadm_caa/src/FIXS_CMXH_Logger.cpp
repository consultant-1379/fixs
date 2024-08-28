/*
 * FIXS_CMXH_Logger.cpp
 *
 */
#include "FIXS_CMXH_Logger.h"

ACS_TRA_Logging* FIXS_CMXH_Logger::FIXS_CMXH_logging = NULL;

ACS_TRA_Logging* FIXS_CMXH_Logger::getLogInstance ()
{
	if (FIXS_CMXH_logging == NULL)
	{
		FIXS_CMXH_logging = new ACS_TRA_Logging();
		FIXS_CMXH_logging->Open("CMXH");
		FIXS_CMXH_logging->Write(" FIXS_CMXH - Create Instance for logging !!! ",LOG_LEVEL_DEBUG);
	}

	return FIXS_CMXH_logging;
}

void FIXS_CMXH_Logger::closeLogInstance ()
{
	if (FIXS_CMXH_logging)
	{
		FIXS_CMXH_logging->Write(" FIXS_CMXH - Destroy Instance for logging !!! ",LOG_LEVEL_DEBUG);
		FIXS_CMXH_logging->Close();
		delete(FIXS_CMXH_logging);
		FIXS_CMXH_logging=NULL;
	}
}
