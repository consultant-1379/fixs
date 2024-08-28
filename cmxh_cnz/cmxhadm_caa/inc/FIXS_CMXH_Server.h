
#ifndef FIXS_CMXH_SERVER_H_
#define FIXS_CMXH_SERVER_H_

#include <iostream>
#include <sys/eventfd.h>

//#include "FIXS_CMXH_ImmHandler.h"
#include "FIXS_CMXH_TrapHandler.h"
#include "FIXS_CMXH_ImmInterface.h"
#include "FIXS_CMXH_Manager.h"

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"

class FIXS_CMXH_ImmInterface;
/*=====================================================================
					CLASS DECLARATION SECTION
==================================================================== */

class FIXS_CMXH_Server {

 public:

	FIXS_CMXH_Server();

	virtual ~FIXS_CMXH_Server();

	bool run();

	bool startWorkerThreads();

	bool stopWorkerThreads();

	void printError(std::string mess,const char *func,int line);

 private:

	bool fxInitialize();

	CMXH_Util::EnvironmentType m_environment;
	/**
		@brief		Handler to IMM objects
	 */
//	FIXS_CMXH_ImmHandler* IMM_Handler;

	/**
		@brief	fixs_cmxh_serverTrace
	 */
	ACS_TRA_trace* _trace;
	

	FIXS_CMXH_TrapHandler * m_trapHandler;
	 ACS_TRA_Logging *FIXS_CMXH_logging;


};

#endif
