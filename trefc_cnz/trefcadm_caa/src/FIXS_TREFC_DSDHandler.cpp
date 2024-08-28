/*
 * FIXS_TREFC_DSDHandler.cpp
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_DSDHandler.h"
#include "FIXS_TREFC_Event.h"
#include "FIXS_TREFC_Util.h"

namespace dsdHandler{

	const int threadTermination = -1;
};
/*============================================================================
	ROUTINE: CONSTRUCTOR
 ============================================================================ */
FIXS_TREFC_DSDHandler::FIXS_TREFC_DSDHandler()
{
	_trace = new ACS_TRA_trace("FIXS_TREFC_DSDHandler");}

/*============================================================================
	ROUTINE: DESTRUCTOR
 ============================================================================ */
FIXS_TREFC_DSDHandler::~FIXS_TREFC_DSDHandler()
{
	int result = 0;
	tExit = true;

	/************ DSD DISCONNECT *************/
	//cout << __FUNCTION__ << "@" << __LINE__ << "Try to close DSD session..." << endl;
	result = session.close();

	//cout << __FUNCTION__ << "@" << __LINE__ << "Try to unregister from DSD..." << endl;
	result = server.unregister();

	//cout << __FUNCTION__ << "@" << __LINE__ << "Try to close DSD server..." << endl;
	result = server.close();

	/************ DSD DISCONNECT *************/
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_TREFC_DSDHandler::open(void *args)
{
	int result = 0;

	UNUSED(args);

	//Thread will stay up!
	tExit = false;

	cout << __FUNCTION__ << "@" << __LINE__ << " Try to activate..." << endl;
	result = activate();
	cout << __FUNCTION__ << "@" << __LINE__ << " activate DSD thread: " << result << endl;
    if(result != 0)
    {
    	cout << __FUNCTION__ << "@" << __LINE__ << " Activation failed" << endl;
    	tExit = true;
    }

    return result;
}

/*============================================================================
	ROUTINE: close
 ============================================================================ */
int FIXS_TREFC_DSDHandler::close (u_long flags)
{
	//Thread will stop!
	UNUSED(flags);

	tExit = true;

	//todo delete all "new"

    return 0;
}

/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_TREFC_DSDHandler::svc()
{
	int result = 0;

	int dsdSteps = 1;
	FIXS_TREFC_EventHandle shutdownEvent;

	//Configure DSD Handles
	int handle_count = 0;
	acs_dsd::HANDLE* handles = NULL;

	bool openSuccess = false;

	result = -1;

	while(!tExit)
	{
		switch (dsdSteps)
		{
		case 1: //init DSD

			if (!openSuccess)
				result = server.open(acs_dsd::SERVICE_MODE_INET_SOCKET);

			if (result < 0) { //Server open failed. Try again after 10 seconds
				cout << __FUNCTION__ << "@" << __LINE__ << "server.open failed !" << endl;

				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  server.open failed ! ");
					_trace->ACS_TRA_event(1,tmpStr);
				}

				dsdSteps = 1;
				sleep(10);
				break;
			}
			else
			{
				//Just for tracing ;)
				if (result > 0) { //Server open successful but a warning was raised!
					cout << __FUNCTION__ << "@" << __LINE__ << "Open with a warning !!" << endl;

					if (_trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  Open with a warning !! ");
						_trace->ACS_TRA_event(1,tmpStr);
					}
				}
				else
				{ //Server open sucessful!
					cout << __FUNCTION__ << "@" << __LINE__ << "Open successfully !!" << endl;

					if (_trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  Open successfully !! ");
						_trace->ACS_TRA_event(1,tmpStr);
					}
					openSuccess = true;
				}

				//Try to publish
				result = server.publish("AMITRCAP", "JTP"); //Name + App Domain
				if (result < 0) { //Publish failed! Try again in 10 seconds
					cout << __FUNCTION__ << "@" << __LINE__ << "server.publish failed !" << endl;
					if (_trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  Server.publish failed !! ");
						_trace->ACS_TRA_event(1,tmpStr);
					}
					//					result = server.close();
					dsdSteps = 1;
					sleep(10);
					break;
				}
				else
				{
					//Connection done !
					dsdSteps = 2;

					//Just for tracing ;)
					if (result > 0) { //Publish successful but a warning was returned
						cout << __FUNCTION__ << "@" << __LINE__ << "Publish return a warning !!" << endl;
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  Publish return a warning !! ");
							_trace->ACS_TRA_event(1,tmpStr);
						}
					}
					else { //Publish successful
						cout << __FUNCTION__ << "@" << __LINE__ << "Published successfully !!" << endl;
						if (_trace->ACS_TRA_ON())
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  Published successfully !! ");
							_trace->ACS_TRA_event(1,tmpStr);
						}
					}
				}
			}//else open result
			break;

		case 2: //Retrieve DSD Handles

			cout << __FUNCTION__ << "@" << __LINE__ << "Start step 2 !" << endl;
			//retrieve handles number first

			if (handle_count == 0) {
				result = server.get_handles(0, handle_count);
				if (result <= 0) {
					cout << __FUNCTION__ << "@" << __LINE__ << "Handles number failed !" << endl;
					if (_trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  Handles number failed !! ");
						_trace->ACS_TRA_event(1,tmpStr);
					}
				}
			}

			cout << __FUNCTION__ << "@" << __LINE__ << "handle_count: " << handle_count << endl;


			handles = new acs_dsd::HANDLE[handle_count];

			result = server.get_handles(handles, handle_count);
			if (result < 0) {
				delete[] handles;
				cout << __FUNCTION__ << "@" << __LINE__ << "Handles failed !" << endl;
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  Handles failed !! ");
					_trace->ACS_TRA_event(1,tmpStr);
				}
			}
			else {
				dsdSteps = 3;
				handle_count++; //increase array len for the shutdown.
				//add shutdown event !
//				handles[handle_count] = FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);
				shutdownEvent = FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);

				//Put shutdown in the last position
				//handles[handle_count] = dynamic_cast<acs_dsd::HANDLE*> (shutdownEvent);
				//dynamic_cast<FIXS_CCH_EventHandle>
				//handles[handle_count] = shutdownEvent;
				//cout << __FUNCTION__ << "@" << __LINE__ << " handles[handle_count]: " << handles[handle_count] << endl;
				if (_trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, " FIXS_TREFC_DSDHandler -  Get Handles successfully !! ");
					_trace->ACS_TRA_event(1,tmpStr);
				}

			}

			break;

		case 3: //Wait for handles (actually wait only for shutdown event, since the other events are not handled!)

			// handle events
			int waitRes = -1; //

			if (! tExit)
			{
				waitRes = FIXS_TREFC_Event::WaitForEvents(1, &shutdownEvent, 100000);
			}
			else
			{
				cout << __FUNCTION__ << "@" << __LINE__ << " EXIT Signaled "<< endl;
				break;
			}
			switch (waitRes)
			{
				case FIXS_TREFC_Event_RC_TIMEOUT:  // Time out, means startup supervision completed as expected
					//Do nothing
					sleep(1);
					break;

				default:


					if (waitRes == 0){ // Shutdown event received
						tExit = true;
						sleep(1);
					}
					break;
			} //switch (waitRes)
			break;
		} //switch (dsdSteps)
	} //while(!tExit)

	if(handles)
		delete[] handles;

	cout << __FUNCTION__ << "@" << __LINE__ << "Disconnect from DSD " << endl;
	session.close();
	server.unregister();
	server.close();

	return result;
}

std::string FIXS_TREFC_DSDHandler::getOpt(char *buf, int bufSize)
{
	std::string tmp;
	tmp.assign(buf,0, bufSize);

	std::cout << tmp.c_str() << std::endl;

	if (tmp.find(":") != std::string::npos)
		tmp = tmp.substr(0, tmp.find(":"));
	else
		tmp = "";

	return tmp;
}

int FIXS_TREFC_DSDHandler::splitParams(char *buf, std::list<std::string> &lstParams)
{
	char * par;
	par = strtok(buf, ":");

	lstParams.clear();

	while (par != NULL) {
		lstParams.push_back(par);
		std::cout << par << std::endl;
		par = strtok(NULL, ":");
	}

	std::list<std::string>::iterator it;
	for (it = lstParams.begin() ; it != lstParams.end() ; it++)
	{
		std::cout << it->c_str() << std::endl;
	}

	return lstParams.size();
}
