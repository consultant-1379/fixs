#ifndef FIXS_TREFC_SERVER_H_
#define FIXS_TREFC_SERVER_H_

#include <iostream>
#include <sys/eventfd.h>

#include "FIXS_TREFC_ImmHandler.h"
#include "FIXS_TREFC_ImmInterface.h"
#include "FIXS_TREFC_JTPServer.h"
#include "FIXS_TREFC_Util.h"
#include "ACS_TRA_trace.h"
#include "FIXS_TREFC_Manager.h"
//#include "FIXS_TREFC_DSDHandler.h"
#include "ACS_CS_API.h"
#include "ACS_CS_API_Set.h"


class FIXS_TREFC_ImmHandler;

/*=====================================================================
					CLASS DECLARATION SECTION
==================================================================== */

class FIXS_TREFC_Server {

 public:


	FIXS_TREFC_Server();

	virtual ~FIXS_TREFC_Server();

	bool run();

	bool startWorkerThreads();

	bool stopWorkerThreads();

	bool waitOnShotdown();

	//int& getStopHandle() { return m_StopEvent; };

	void printError(std::string mess,const char *func,int line);

 private:

	bool fxInitialize();

	TREFC_Util::EnvironmentType m_environment;
	/**
		@brief		Handler to IMM objects
	*/
	FIXS_TREFC_ImmHandler* IMM_Handler;

	/**
		@brief	fms_cpf_serverTrace
	*/
	ACS_TRA_trace* _trace;

	FIXS_TREFC_JTPServer *_jtpServer;

	/**
		@brief	stopImmHandler
		stop the IMM handler thread
	 */
	bool stopImmHandler();


};

#endif
