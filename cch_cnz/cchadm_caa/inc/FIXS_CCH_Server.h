
#ifndef FIXS_CCH_SERVER_H_
#define FIXS_CCH_SERVER_H_

#include <iostream>
#include <sys/eventfd.h>
#include <sys/capability.h>
#include <string.h>
#include <errno.h>

#include "FIXS_CCH_CmdHandler.h"
#include "FIXS_CCH_ImmHandler.h"
#include "FIXS_CCH_JTPServer.h"
#include "FIXS_CCH_DiskHandler.h"
#include "FIXS_CCH_SoftwareMap.h"
#include "FIXS_CCH_TrapHandler.h"
#include "FIXS_CCH_ImmInterface.h"
#include "FIXS_CCH_UpgradeManager.h"

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"

class FIXS_CCH_ImmInterface;
/*=====================================================================
					CLASS DECLARATION SECTION
==================================================================== */

class FIXS_CCH_Server {

 public:

	FIXS_CCH_Server();

	virtual ~FIXS_CCH_Server();

	bool run();

	bool startWorkerThreads();

	bool stopWorkerThreads();

	void printError(std::string mess,const char *func,int line);

 private:

	bool fxInitialize();

	CCH_Util::EnvironmentType m_environment;
	/**
		@brief		Handler to IMM objects
	 */
	FIXS_CCH_ImmHandler* IMM_Handler;

	/**
					@brief	Handler to JTP
	 */
	FIXS_CCH_JTPServer *_jtpServer;


	/**
		@brief	fms_cpf_serverTrace
	 */
	ACS_TRA_trace* _trace;

	/**
		@brief	Handler of cpf command
	 */
	FIXS_CCH_CmdHandler* m_CmdHandle;


	ACS_TRA_Logging *FIXS_CCH_logging;

	FIXS_CCH_TrapHandler * m_trapHandler;

	CCH_Util::APGtype m_apgNumber;


};

#endif
