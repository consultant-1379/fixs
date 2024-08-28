#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>

#include "FIXS_CMXH_HA_AppManager.h"
#include "FIXS_CMXH_Server.h"
#include "FIXS_CMXH_Event.h"
#include "acs_aeh_signalhandler.h"
#include "FIXS_CMXH_Logger.h"

void sighandler(int signum);
int setupSignalHandler(const struct sigaction* sigAction);
void printusage();
void CreateEvents();

FIXS_CMXH_Server *g_ptr_CMXHServer = 0;
FIXS_CMXH_HA_AppManager *haObj =0;
ACS_TRA_Logging* FIXS_CMXH_logging = FIXS_CMXH_Logger::getLogInstance();

bool g_InteractiveMode = false;

FIXS_CMXH_EventHandle shutdownEventHandle = 0;

static const char FIXS_CMXH_Daemon[] = "fixs_cmxhd";
using namespace std;

ACE_INT32 ACE_TMAIN( ACE_INT32 argc , ACE_TCHAR* argv[])
{
	ACE_INT32 result = 0;

	struct sigaction sa;
	sa.sa_handler = sighandler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask );

	syslog(LOG_INFO, "Starting fixs_cmxhd main started.. ");
	if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - Starting fixs_cmxhd main started !!! ",LOG_LEVEL_DEBUG);
	// set the signal handler for the main
	result = setupSignalHandler(&sa);

	CreateEvents();

	if(0 == result )
	{
		if( argc > 1)
		{
			// fixs_cmxhd started by command line
			if( argc == 2 && ( 0 == strcmp(argv[1],"-d") ) )
			{
				g_InteractiveMode = true;

				g_ptr_CMXHServer = new (std::nothrow) FIXS_CMXH_Server() ;

				if( NULL == g_ptr_CMXHServer )
				{
					cout << "Memory allocated failed for FIXS_CMXH_Server" << endl;
					if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - Memory allocated failed for FIXS_CMXH_Server !!! ",LOG_LEVEL_DEBUG);
					result = -1;
				}
				else
				{
					cout << "\nfixs_cmxhd started in debug mode...\n\n" << endl;
					if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - fixs_cmhd started in debug mode !!! ",LOG_LEVEL_DEBUG);
					//g_ptr_CMXHServer->startWorkerThreads();
					result = g_ptr_CMXHServer->run();

					// wait signal SIGTERM for termination
					//ACE_OS::sleep(80);
					sigpause(SIGTERM);

					shutdownEventHandle = FIXS_CMXH_Event::OpenNamedEvent(CMXH_Util::EVENT_NAME_SHUTDOWN);
					int retries = 0;

				    while (FIXS_CMXH_Event::SetEvent(shutdownEventHandle) == 0)
					{
				    	cout << "Set shutdown.... retries: " << retries << endl;
						if( ++retries > 10) break;
						sleep(1);
					}

					sleep(1);
					//g_ptr_CMXHServer->waitOnShotdown();
					cout << "\nfixs_cmxhd stopped in debug mode..." << endl;
					if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - fixs_cmxhd stopped in debug mode !!! ",LOG_LEVEL_DEBUG);

					FIXS_CMXH_Event::CloseEvent(shutdownEventHandle);
					delete g_ptr_CMXHServer;
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
			syslog(LOG_INFO, "fixs_cmxhd started with CoreMw.. ");
			if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - fixs_cmxhd started with CoreMw !!! ",LOG_LEVEL_DEBUG);

			acs_aeh_setSignalExceptionHandler(FIXS_CMXH_Daemon);
			g_InteractiveMode = false;

			ACS_APGCC_HA_ReturnType errorCode = ACS_APGCC_HA_SUCCESS;

			haObj = new FIXS_CMXH_HA_AppManager(FIXS_CMXH_Daemon);

			//Check for Memory allocation
			if( haObj == 0 )
			{
				printf("Memory allocation failed for FIXS_CMXH_HA_AppManager Class");
				if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - Memory allocation failed for FIXS_CMXH_HA_AppManager Class !!! ",LOG_LEVEL_DEBUG);
			}
			else
			{
				syslog(LOG_INFO, "Starting fixs_cmxhd service.. ");

				errorCode = haObj->activate();

				if (errorCode == ACS_APGCC_HA_FAILURE){
						syslog(LOG_ERR, "fixs_cmxhd, HA Activation Failed!!");
						if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - fixs_cmxhd, HA Activation Failed !!! ",LOG_LEVEL_DEBUG);
						result = ACS_APGCC_FAILURE;
				}

				if (errorCode == ACS_APGCC_HA_FAILURE_CLOSE){
						syslog(LOG_ERR, "fixs_cmxhd, HA Application Failed to Gracefullly closed!!");
						if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - fixs_cmxhd, HA Application Failed to Gracefullly closed !!! ",LOG_LEVEL_DEBUG);
						result = ACS_APGCC_FAILURE;
				}

				if (errorCode == ACS_APGCC_HA_SUCCESS){
						syslog(LOG_ERR, "fixs_cmxhd, HA Application Gracefully closed!!");
						if(FIXS_CMXH_logging) FIXS_CMXH_logging->Write(" FIXS_CMXH_SRV - fixs_cmxhd, HA Application Gracefully closed !!! ",LOG_LEVEL_DEBUG);
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
	syslog(LOG_INFO, "fixs_cmxhd was terminated.. ");
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
			if(g_ptr_CMXHServer != 0)
			{
				g_ptr_CMXHServer->stopWorkerThreads();
			}
		}
	}
}

int setupSignalHandler(const struct sigaction* sigAction)
{
	if( sigaction(SIGINT, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGINT in fixs_cmxhd ");
		return -1;
	}

	if( sigaction(SIGTERM, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGTERM in fixs_cmxhd ");
		return -1;
	}

	if( sigaction(SIGTSTP, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGTSTP in fixs_cmxhd ");
		return -1;
	}

    return 0;
}

void printusage()
{
	cout<<"Usage: fixs_cmxhd -d for debug mode.\n"<<endl;
}

void CreateEvents()
{
    	(void) FIXS_CMXH_Event::CreateEvent(true, false, CMXH_Util::EVENT_NAME_SHUTDOWN);
}

