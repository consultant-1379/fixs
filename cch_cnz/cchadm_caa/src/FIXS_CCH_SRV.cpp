#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>

#include "FIXS_CCH_HA_AppManager.h"
#include "FIXS_CCH_Server.h"
#include "FIXS_CCH_Event.h"
#include "FIXS_CCH_Util.h"

#include "acs_aeh_signalhandler.h"


void sighandler(int signum);
int setupSignalHandler(const struct sigaction* sigAction);
void printusage();
void CreateEvents();

FIXS_CCH_Server *g_ptr_CCHServer = 0;
FIXS_CCH_HA_AppManager *haObj =0;

ACS_TRA_Logging* FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

bool g_InteractiveMode = false;

FIXS_CCH_EventHandle shutdownEventHandle = 0;

static const char FIXS_CCH_Daemon[] = "fixs_cchd";
using namespace std;

ACE_INT32 ACE_TMAIN( ACE_INT32 argc , ACE_TCHAR* argv[])
{
	ACE_INT32 result = 0;

	struct sigaction sa;
	sa.sa_handler = sighandler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask );

	syslog(LOG_INFO, "Starting fixs_cchd main started.. ");
	if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - Starting fixs_cchd main started !!! ",LOG_LEVEL_DEBUG);

	// set the signal handler for the main
	result = setupSignalHandler(&sa);

	CreateEvents();

	if(0 == result )
	{
		if( argc > 1)
		{
			// fixs_cchd started by command line
			if( argc == 2 && ( 0 == strcmp(argv[1],"-d") ) )
			{
				g_InteractiveMode = true;

				CCH_Util::setDebug(true);

				g_ptr_CCHServer = new (std::nothrow) FIXS_CCH_Server() ;

				if( NULL == g_ptr_CCHServer )
				{
					DEBUG("Memory allocated failed for FIXS_CCH_Server");
					if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - Memory allocated failed for FIXS_CCH_Server !!! ",LOG_LEVEL_ERROR);
					result = -1;
				}
				else
				{
					DEBUG("fixs_cchd started in debug mode...");
					if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - fixs_cchd started in debug mode !!! ",LOG_LEVEL_DEBUG);

					//g_ptr_CCHServer->startWorkerThreads();
					result = g_ptr_CCHServer->run();

					// wait signal SIGTERM for termination
					//ACE_OS::sleep(80);
					sigpause(SIGTERM);

					shutdownEventHandle = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
					int retries = 0;

				    while (FIXS_CCH_Event::SetEvent(shutdownEventHandle) == 0)
					{
				    	DEBUG("Set shutdown.... retries: " << retries);
						if( ++retries > 10) break;
						sleep(1);
					}

					sleep(1);
					//g_ptr_CCHServer->waitOnShotdown();
					DEBUG("fixs_cchd stopped in debug mode...");
					if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - fixs_cchd stopped in debug mode !!! ",LOG_LEVEL_DEBUG);

					//close logging
//					FIXS_CCH_DiskHandler::closeLogInstance();

					FIXS_CCH_Event::CloseEvent(shutdownEventHandle);
					delete g_ptr_CCHServer;
				}
			}
			else
			{
				printusage();
				result = -1;
			}
		}
		else
		{
			syslog(LOG_INFO, "fixs_cchd started with CoreMw.. ");

			acs_aeh_setSignalExceptionHandler(FIXS_CCH_Daemon);

			if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - fixs_cchd started with CoreMw !!! ",LOG_LEVEL_DEBUG);
			g_InteractiveMode = false;

			ACS_APGCC_HA_ReturnType errorCode = ACS_APGCC_HA_SUCCESS;

			haObj = new FIXS_CCH_HA_AppManager(FIXS_CCH_Daemon);

			//Check for Memory allocation
			if( haObj == 0 )
			{
				DEBUG("Memory allocation failed for FIXS_CCH_HA_AppManager Class");
				if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - Memory allocation failed for FIXS_CCH_HA_AppManager Class !!! ",LOG_LEVEL_ERROR);
			}
			else
			{
				syslog(LOG_INFO, "Starting fixs_cchd service.. ");

				errorCode = haObj->activate();

				if (errorCode == ACS_APGCC_HA_FAILURE){
						syslog(LOG_ERR, "fixs_cchd, HA Activation Failed!!");
						if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - fixs_cchd, HA Activation Failed !!! ",LOG_LEVEL_ERROR);

						result = ACS_APGCC_FAILURE;
				}

				if (errorCode == ACS_APGCC_HA_FAILURE_CLOSE){
						syslog(LOG_ERR, "fixs_cchd, HA Application Failed to Gracefullly closed!!");
						if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - fixs_cchd, HA Application Failed to Gracefullly closed !!! ",LOG_LEVEL_ERROR);

						result = ACS_APGCC_FAILURE;
				}

				if (errorCode == ACS_APGCC_HA_SUCCESS){
						syslog(LOG_ERR, "fixs_cchd, HA Application Gracefully closed!!");
						if(FIXS_CCH_logging) FIXS_CCH_logging->Write(" FIXS_CCH_SRV - fixs_cchd, HA Application Gracefully closed !!! ",LOG_LEVEL_DEBUG);

						result = ACS_APGCC_FAILURE;
				}
			}
		}
	}

	if (haObj != NULL)
	{
		delete haObj;
		haObj = NULL;
	}
	syslog(LOG_INFO, "fixs_cchd was terminated.. ");
	return result;
} //End of MAIN


void sighandler(int signum)
{
	if( signum == SIGTERM || signum == SIGINT || signum == SIGTSTP )
	{
		if(!g_InteractiveMode)
		{
			if(haObj != 0)
			{
				haObj->performComponentRemoveJobs();
				haObj->performComponentTerminateJobs();
			}
		}
		else
		{
			if(g_ptr_CCHServer != 0)
			{
				g_ptr_CCHServer->stopWorkerThreads();
			}
		}
	}
}

int setupSignalHandler(const struct sigaction* sigAction)
{
	if( sigaction(SIGINT, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGINT in fixs_cchd ");
		return -1;
	}

	if( sigaction(SIGTERM, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGTERM in fixs_cchd ");
		return -1;
	}

	if( sigaction(SIGTSTP, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGTSTP in fixs_cchd ");
		return -1;
	}

    return 0;
}

void printusage()
{
	cout << "Usage: fixs_cchd -d for debug mode." << endl;
}

void CreateEvents()
{
	(void) FIXS_CCH_Event::CreateEvent(true, false, CCH_Util::EVENT_NAME_SHUTDOWN);
}

