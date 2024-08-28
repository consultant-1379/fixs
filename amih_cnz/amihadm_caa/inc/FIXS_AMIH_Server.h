#ifndef FIXS_AMIH_Server_H_
#define FIXS_AMIH_Server_H_

#include <iostream>
#include <sys/eventfd.h>
#include <istream>
#include <ostream>
#include <sstream>
#include <list>
#include <string>

#include "ace/OS.h"
#include "ace/SOCK_Dgram.h"
#include "ace/INET_Addr.h"
#include "FIXS_AMIH_Util.h"
#include "ACS_TRA_trace.h"
#include "FIXS_AMIH_JTPHandler.h"
#include <iostream>
#include <list>
#include "FIXS_AMIH_HwcSubscriber.h"
#include "FIXS_AMIH_DSDHandler.h"



class FIXS_AMIH_Server : public ACE_Task_Base  {

 public:


	FIXS_AMIH_Server();

	virtual ~FIXS_AMIH_Server();

	virtual int svc(void);

	virtual int open();

	bool run();

	void printTRA(std::string mesg);

	bool waitOnShotdown();

	int& getStopHandle() { return m_StopEvent; };

	void printError(std::string mess,const char *func,int line);

	static const int AMIHSRV_NO_ERROR;
	static const int AMIHSRV_ERR_CREATE_STOP_EVENT;
	static const int AMIHSRV_ERR_NEW_SYS_CONFIGURATION;
	static const int AMIHSRV_ERR_SYS_CONFIGURATION;
	static const int AMIHSRV_ERR_CREATE_EVENT_HANDLER;
	static const int AMIHSRV_ERR_WAIT_FAILED;
	static const int AMIHSRV_ERR_NEW_MGR;
	static const int AMIHSRV_ERR_START_MGR;
	static const int AMIHSRV_ERR_NEW_STAT;
	static const int AMIHSRV_ERR_INIT_STAT;
	static const int AMIHSRV_ERR_LOAD_DEF_FILE;
	static const int AMIHSRV_ERR_CHECK_DATADISK;
	static const int AMIHSRV_START_PROTOCOL_TIMEOUT;
	static const int AMIHSRV_GET_CONFIGURATION_TIMEOUT;
	static const int AMIHSRV_WAIT_EVENT_TIMEOUT;
	static const int AMIHSRV_WAIT_FINALIZE_ALL_MGR;
	static const int AMIHSRV_MAX_RETRIES_ON_ERROR;

	bool stopWorkerThreads();
	bool startWorkerThreads();

	bool IsCba(bool &res);
	bool getImmAttributeInt(const std::string &strDn, const std::string &strAttr, int *iVal);
	const char *name();
	const char *domain ();

 private:

	 int m_exit;
     bool _alive;
     bool _stopRequested;
     bool isSubscribed;
     FIXS_AMIH_HwcSubscriber *observer;
     FIXS_AMIH_JTPHandler *jtpHandler;
     //FIXS_AMIH_DSDHandler *DSDHandle;

 	bool fxInitialize();

	ACS_TRA_trace* _trace;

	int m_StopEvent;
	int m_shutdownEvent;

};

#endif /* FIXS_AMIH_Server_H_ */
