/*
 * ACS_AMIH_SRV.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: qrenbor
 */

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>

#include "FIXS_AMIH_HA_AppManager.h"
#include "FIXS_AMIH_Server.h"
#include "FIXS_AMIH_Event.h"
#include "FIXS_AMIH_Util.h"

#include "acs_aeh_signalhandler.h"

void sighandler(int signum);
int setupSignalHandler(const struct sigaction* sigAction);
void printusage();
void CreateEvents();

FIXS_AMIH_Server *g_ptr_AMIHServer = 0;
FIXS_AMIH_HA_AppManager *haObj =0;

bool g_InteractiveMode = false;

FIXS_AMIH_EventHandle shutdownEventHandle = 0;

static const char FIXS_AMIH_Daemon[] = "fixs_amihd";
using namespace std;

ACE_INT32 ACE_TMAIN( ACE_INT32 argc , ACE_TCHAR* argv[])
{
	ACE_INT32 result = 0;

	struct sigaction sa;
	sa.sa_handler = sighandler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask );

	syslog(LOG_INFO, "Starting acs_amihd main started.. ");


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

				g_ptr_AMIHServer = new (std::nothrow) FIXS_AMIH_Server() ;

				if( NULL == g_ptr_AMIHServer )
				{
					cout << "Memory allocated failed for FIXS_AMIH_Server" << endl;
					result = -1;
				}
				else
				{
					cout << "\n acs_amihd started in debug mode...\n\n" << endl;

					//g_ptr_AMIHServer->startWorkerThreads();
					result = g_ptr_AMIHServer->run();

					// wait signal SIGTERM for termination
					//ACE_OS::sleep(80);
					sigpause(SIGTERM);

					shutdownEventHandle = FIXS_AMIH_Event::OpenNamedEvent(AMIH_Util::EVENT_NAME_SHUTDOWN);
					int retries = 0;

					if (shutdownEventHandle >= 0) {
						printf("shutdown event sending\n");

						while (FIXS_AMIH_Event::SetEvent(shutdownEventHandle) == 0)
						{
							cout << "Set shutdown.... retries: " << retries << endl;
							if( ++retries > 10) break;
							sleep(1);
						}

						sleep(1);

						FIXS_AMIH_Event::CloseEvent(shutdownEventHandle);
					}

					//g_ptr_AMIHServer->waitOnShotdown();
					cout << "\n acs_amihd stopped in debug mode..." << endl;
					delete g_ptr_AMIHServer;
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
			syslog(LOG_INFO, "acs_amihd started with CoreMw.. ");

			acs_aeh_setSignalExceptionHandler(FIXS_AMIH_Daemon);

			g_InteractiveMode = false;

			ACS_APGCC_HA_ReturnType errorCode = ACS_APGCC_HA_SUCCESS;

			haObj = new FIXS_AMIH_HA_AppManager(FIXS_AMIH_Daemon);

			//Check for Memory allocation
			if( haObj == 0 )
			{
				printf("Memory allocation failed for FIXS_AMIH_HA_AppManager Class");
			}
			else
			{
				syslog(LOG_INFO, "Starting acs_amihd service.. ");

				errorCode = haObj->activate();

				if (errorCode == ACS_APGCC_HA_FAILURE){
						syslog(LOG_ERR, "acs_amihd, HA Activation Failed!!");
						result = ACS_APGCC_FAILURE;
				}

				if (errorCode == ACS_APGCC_HA_FAILURE_CLOSE){
						syslog(LOG_ERR, "acs_amihd, HA Application Failed to Gracefully closed!!");
						result = ACS_APGCC_FAILURE;
				}

				if (errorCode == ACS_APGCC_HA_SUCCESS){
						syslog(LOG_ERR, "acs_amihd, HA Application Gracefully closed!!");
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
	syslog(LOG_INFO, "acs_amihd was terminated.. ");
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
			if(g_ptr_AMIHServer != 0)
			{
				g_ptr_AMIHServer->stopWorkerThreads();
			}
		}
	}
}

int setupSignalHandler(const struct sigaction* sigAction)
{
	if( sigaction(SIGINT, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGINT in acs_amihd ");
		return -1;
	}

	if( sigaction(SIGTERM, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGTERM in acs_amihd ");
		return -1;
	}

	if( sigaction(SIGTSTP, sigAction, NULL ) == -1)
	{
		syslog(LOG_ERR, "Error occurred while handling SIGTSTP in acs_amihd ");
		return -1;
	}

    return 0;
}

void printusage()
{
	cout<<"Usage: acs_amihd -d for debug mode.\n"<<endl;
}

void CreateEvents()
{

    	(void) FIXS_AMIH_Event::CreateEvent(true, false, AMIH_Util::EVENT_NAME_SHUTDOWN);

}

