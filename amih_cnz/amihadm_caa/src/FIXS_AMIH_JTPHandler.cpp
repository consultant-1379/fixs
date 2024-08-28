//## begin module%1.10%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.10%.codegen_version

//## begin module%4C08D622000A.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%4C08D622000A.cm

//## begin module%4C08D622000A.cp preserve=no
//	*****************************************************************************
//
//	.NAME
//	    FIXS_AMIH_JTPHandler
//	.LIBRARY 3C++
//	.PAGENAME FIXS_CCH
//	.HEADER  FIXS_CCH
//
//	.COPYRIGHT
//	 COPYRIGHT Ericsson 2010.
//	 All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2009.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2009 or in
//	accordance with the terms and conditions stipulated in the
//	agreement/contract under which the program(s) have been
//	supplied.s

#include "FIXS_AMIH_JTPHandler.h"
#include "ACS_JTP.h"
#include "ACS_TRA_trace.h"
#include <iostream>


const int FIXS_AMIH_JTPHandler::MAX_RETRIES = 10;
const int FIXS_AMIH_JTPHandler::JOB_REJECTED = -1;
const int FIXS_AMIH_JTPHandler::JOB_FAILED = -2;
const int FIXS_AMIH_JTPHandler::JOB_MISMATCH = -3;



FIXS_AMIH_JTPHandler::FIXS_AMIH_JTPHandler()
{
	_trace = new ACS_TRA_trace("FIXS_AMIH_JTPHandler");

	m_waitHandles=0;
	m_noOfJTPHandles=1;
	m_stopRequested=false;
	m_notificationOn=false;

	m_shutdownEvent = FIXS_AMIH_Event::OpenNamedEvent(AMIH_Util::EVENT_NAME_SHUTDOWN);

	m_waitHandles = new int[m_noOfJTPHandles+1];
	
	for (int i=0; i < m_noOfJTPHandles; i++) {
		m_waitHandles[i] = 0;
	}
}


FIXS_AMIH_JTPHandler::~FIXS_AMIH_JTPHandler()
{
	 for (int i=0; i < m_noOfJTPHandles; i++) {
		 FIXS_AMIH_Event::CloseEvent(m_waitHandles[i]);
	}

	delete [] m_waitHandles;
}



//## Other Operations (implementation)
void FIXS_AMIH_JTPHandler::isNotificationNeeded ()
{
}

 void FIXS_AMIH_JTPHandler::setNotificationNeeded ()
{
	if (!m_notificationOn)
	{
		if (m_noOfJTPHandles > 0 && m_waitHandles[0] != 0)
		{
			printTRA("Notification event is signalled...");

			FIXS_AMIH_Event::SetEvent(m_waitHandles[0]);

		} 
		else 
		{
			printTRA("Notification event is not created again.");
		}
	}
	else
	{
		printTRA("A notification is already ongoing.");
	}

}

 int FIXS_AMIH_JTPHandler::handleNotification (int version)
{
	char *ServiceName = (char *) "AMIHWCP";
	unsigned short Len = 0;
	char *Msg = 0;
	unsigned short U1, U2;

	//TODO ----test----
	Msg = (char*) new (std::nothrow) char[strlen("")+1];
	Len = 0;
	memset(Msg,0,Len);
	strcpy(Msg,"");
	//TODO ----test----

	U1 = 256 * version;
	U2 = 0;

	int exitCode = 0;

	// Create a JTP_Conversation object with name and maxbuffer
	ACS_JTP_Job J1(ServiceName, Len);
	
	// Override the default timeout which is 10 seconds
	J1.setTimeOut(10);
	
	FIXS_AMIH_JTPClient *jTPClient = new FIXS_AMIH_JTPClient();

	int result = jTPClient->newJTPClient(Len, J1, U1, U2, Msg);
	
	if (result == 0)
	{
		m_notificationOn = false;

		int confermation = jTPClient->waitForConfermation(J1, U1, U2);
		
		if (confermation == 0)
		{
			bool waitRes = jTPClient->waitForJTPResult(J1, Len, U1, U2, Msg);
			
			if (!waitRes)
			{
				exitCode = JOB_FAILED;

				printTRA("JTP waitForJTPResult - Unexpected error");
			}
			else
			{
				printTRA("JTP waitForJTPResult - SUCCESSFULL");
			}
		}
		else
		{
			exitCode = confermation;

			printTRA("JTP waitForJTPResult - SUCCESSFULL");

		}
	}
	else
	{
		m_notificationOn = false;
		exitCode = result;
		printTRA("JTP request NOT OK");

	}

	delete jTPClient;

	//TODO ----test----
	delete[] Msg;
	Msg = 0;
	//TODO ----test----
	return exitCode;

}

 void FIXS_AMIH_JTPHandler::waitForNotifications (int numOfWaitHandles)
{
	unsigned retryCounter = 0;

	for (; !m_stopRequested && (retryCounter < (unsigned)MAX_RETRIES) ;)
	{
		const int WAIT_SHUTDOWN	=   numOfWaitHandles-1;
		const int WAIT_FAILED   	= -1;
		const int WAIT_TIMEOUT		= -2;

		int result = FIXS_AMIH_Event::WaitForEvents(numOfWaitHandles,m_waitHandles,0);

		if ( result == WAIT_SHUTDOWN)
		{
			printTRA("Shutdown event is signalled.");
			m_stopRequested = true;

			break;
		}
		else if (result == WAIT_FAILED)
		{
			printTRA("Waiting for events failed..");
			retryCounter++;
		}
		else if (result == WAIT_TIMEOUT)
		{
			printTRA("time out event ..");
			retryCounter++;
		}
		else if (result <numOfWaitHandles - 1)
		{
			int version = 1;

			m_notificationOn = true;

			FIXS_AMIH_Event::ResetEvent(m_waitHandles[result]);

			int exitCode = this->handleNotification(version);


			if (exitCode == JOB_REJECTED) //Not Confermation or jinitreq failed
			{
				printTRA("retry to send CP request after job rejected or failed jinitreq..");

				int retryForConf = 0;

				bool sendNotification = true;

				while (sendNotification && retryForConf < 3 && !m_stopRequested)
				{
					int res=FIXS_AMIH_Event::WaitForEvents(1,&m_shutdownEvent,60000);

					if (res == 0 || res == WAIT_FAILED) {
						printTRA("Shutdown event is signalled.");
						m_stopRequested = true;
						break;

					}

					std::stringstream ss;
					ss << "Send CP. retry number: " << retryForConf;
					printTRA(ss.str());

					FIXS_AMIH_Event::ResetEvent(m_waitHandles[result]);

					int exitConf = this->handleNotification(version);

					if (exitConf == 0) 
					{
						sendNotification = false;
					}
					else
					{
						retryForConf++;
					}
				}
			}
			else if (exitCode == JOB_FAILED) //Not Result Ind
			{
				printTRA("retry to send CP request after job failed..");

				FIXS_AMIH_Event::ResetEvent(m_waitHandles[result]);

				this->handleNotification(version);
			}
			else if (exitCode == JOB_MISMATCH) // Protocol version mismatch
			{
				int downgradeVersion = version - 1;

				printTRA("retry to send CP request after protocol mismatch..");

				FIXS_AMIH_Event::ResetEvent(m_waitHandles[result]);

				this->handleNotification(downgradeVersion);
			}
		}
		else 
		{
			printTRA("nothing from the socket.....");
		}
	}

}


int FIXS_AMIH_JTPHandler::svc()
{
	for (int i=0; i < m_noOfJTPHandles; i++) {
		m_waitHandles[i] =FIXS_AMIH_Event::CreateEvent(true, false, NULL);
	}

	m_waitHandles[m_noOfJTPHandles] = m_shutdownEvent;

	this->waitForNotifications(m_noOfJTPHandles + 1);

	return 0;

}

 void FIXS_AMIH_JTPHandler::printTRA(std::string mesg)
 {
 	if (_trace->ACS_TRA_ON())
 	{
 		char tmpStr[512] = {0};
 		snprintf(tmpStr, sizeof(tmpStr) - 1, mesg.c_str());
 		_trace->ACS_TRA_event(1, tmpStr);

 	}
 }


  int FIXS_AMIH_JTPHandler::open (void *)
  {

	  return 0;
  }
