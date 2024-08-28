#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>

#include "FIXS_TREFC_HA_AppManager.h"
#include "FIXS_TREFC_Server.h"
#include "FIXS_TREFC_Event.h"
#include "FIXS_TREFC_Logger.h"


#include "acs_aeh_signalhandler.h"

void sighandler(int signum);
int setupSignalHandler(const struct sigaction* sigAction);
void printusage();
void CreateEvents();

FIXS_TREFC_Server *g_ptr_TREFCServer = 0;
FIXS_TREFC_HA_AppManager *haObj =0;

bool g_InteractiveMode = false;

FIXS_TREFC_EventHandle shutdownEventHandle = 0;

static const char FIXS_TREFC_Daemon[] = "fixs_trefcd";
using namespace std;

ACE_INT32 ACE_TMAIN( ACE_INT32 argc , ACE_TCHAR* argv[])
{
	ACE_INT32 result = 0;

	struct sigaction sa;
	sa.sa_handler = sighandler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask );

	syslog(LOG_INFO, "Starting fixs_trefcd main started.. ");


	// set the signal handler for the main
	result = setupSignalHandler(&sa);

	CreateEvents();

	if(0 == result )
	{
		if( argc > 1)
		{
			// fixs_trefcd started by command line
			if( argc == 2 && ( 0 == strcmp(argv[1],"-d") ) )
			{
				g_InteractiveMode = true;

				g_ptr_TREFCServer = new (std::nothrow) FIXS_TREFC_Server() ;

				if( NULL == g_ptr_TREFCServer )
				{
					cout << "Memory allocated failed for FIXS_TREFC_Server" << endl;
					result = -1;
				}
				else
				{
					cout << "\nfixs_trefcd started in debug mode...\n\n" << endl;

					//g_ptr_TREFCServer->startWorkerThreads();
					result = g_ptr_TREFCServer->run();

					// wait signal SIGTERM for termination
					//ACE_OS::sleep(80);
					sigpause(SIGTERM);

					shutdownEventHandle = FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);
					int retries = 0;

					if (shutdownEventHandle >= 0) {
						while (FIXS_TREFC_Event::SetEvent(shutdownEventHandle) == 0)
						{
							cout << "Set shutdown.... retries: " << retries << endl;
							if( ++retries > 10) break;
							sleep(1);
						}

						sleep(1);

						cout << "\nfixs_trefcd stopped in debug mode..." << endl;

						FIXS_TREFC_Event::CloseEvent(shutdownEventHandle);
					} else {
						cout << "\nfixs_trefcd failed to set shutdown event: forcing stop now..." << endl;
					}

					delete g_ptr_TREFCServer;

					//close logging
					FIXS_TREFC_Logger::closeLogInstance();
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
			syslog(LOG_INFO, "fixs_trefcd started with CoreMw.. ");

			acs_aeh_setSignalExceptionHandler(FIXS_TREFC_Daemon);

			g_InteractiveMode = false;

			ACS_APGCC_HA_ReturnType errorCode = ACS_APGCC_HA_SUCCESS;

			haObj = new FIXS_TREFC_HA_AppManager(FIXS_TREFC_Daemon);

			//Check for Memory allocation
			if( haObj == 0 )
			{
				printf("Memory allocation failed for FIXS_TREFC_HA_AppManager Class");
			}
			else
			{
				syslog(LOG_INFO, "Starting fixs_trefcd service.. ");

				errorCode = haObj->activate();

				if (errorCode == ACS_APGCC_HA_FAILURE){
						syslog(LOG_ERR, "fixs_trefcd, HA Activation Failed!!");

						//close logging
						FIXS_TREFC_Logger::closeLogInstance();

						result = ACS_APGCC_FAILURE;
				}

				if (errorCode == ACS_APGCC_HA_FAILURE_CLOSE){
						syslog(LOG_ERR, "fixs_trefcd, HA Application Failed to Gracefullly closed!!");

						//close logging
						FIXS_TREFC_Logger::closeLogInstance();

						result = ACS_APGCC_FAILURE;
				}

				if (errorCode == ACS_APGCC_HA_SUCCESS){
						syslog(LOG_ERR, "fixs_trefcd, HA Application Gracefully closed!!");

						//close logging
						FIXS_TREFC_Logger::closeLogInstance();

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
	syslog(LOG_INFO, "fixs_trefcd was terminated.. ");
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
			if(g_ptr_TREFCServer != 0)
			{
				g_ptr_TREFCServer->stopWorkerThreads();
			}
		}
	}
}

int setupSignalHandler(const struct sigaction* sigAction)
{
	if( sigaction(SIGINT, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGINT in fixs_trefcd ");
		return -1;
	}

	if( sigaction(SIGTERM, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGTERM in fixs_trefcd ");
		return -1;
	}

	if( sigaction(SIGTSTP, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGTSTP in fixs_trefcd ");
		return -1;
	}

    return 0;
}

void printusage()
{
	cout<<"Usage: fixs_trefcd -d for debug mode.\n"<<endl;
}

void CreateEvents()
{
    	(void) FIXS_TREFC_Event::CreateEvent(true, false, TREFC_Util::EVENT_NAME_SHUTDOWN);
}

