
#include "FIXS_AMIH_DSDHandler.h"

#include "FIXS_AMIH_Event.h"

#include "Client_Server_Interface.h"

using namespace std;

namespace dsdHandler{

const int threadTermination = -1;
};


static const char EVENT_NAME_SHUTDOWN[]    = "FIXS_AMIH_SHUTDOWN";

/*============================================================================
	ROUTINE: CONSTRUCTOR
 ============================================================================ */
FIXS_AMIH_DSDHandler::FIXS_AMIH_DSDHandler()
{
	_trace = new ACS_TRA_trace("FIXS_AMIH_DSDHandler");
}

/*============================================================================
	ROUTINE: DESTRUCTOR
 ============================================================================ */
FIXS_AMIH_DSDHandler::~FIXS_AMIH_DSDHandler()
{
	int result = 0;
	tExit = true;

	/************ DSD DISCONNECT *************/
	//	cout << __FUNCTION__ << "@" << __LINE__ << "Try to close DSD session..." << endl;
	result = session.close();

	//	cout << __FUNCTION__ << "@" << __LINE__ << "Try to unregister from DSD..." << endl;
	result = server.unregister();

	//	cout << __FUNCTION__ << "@" << __LINE__ << "Try to close DSD server..." << endl;
	result = server.close();

	delete (_trace);

	/************ DSD DISCONNECT *************/
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_AMIH_DSDHandler::open(void */*args*/)
{

	int result = 0;

	//Thread will stay up!
	tExit = false;

	result = activate();
	sleep(1);

	if(result != 0)
	{
		cout << "\n ####################################################### " << endl;
		cout << " 	DSD Activation failed 			" << endl;
		cout << " ####################################################### \n" << endl;
		tExit = true;
	}

	return result;
}

/*============================================================================
	ROUTINE: close
 ============================================================================ */
int FIXS_AMIH_DSDHandler::close (u_long /*flags*/)
{
	//Thread will stop!
	tExit = true;

	//todo delete all "new"

	return 0;
}

/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_AMIH_DSDHandler::svc()
{
	int result = 0;

	int dsdSteps = 1;

	FIXS_AMIH_EventHandle shutdownEvent;

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
				{
					result = server.open(acs_dsd::SERVICE_MODE_INET_SOCKET);
				}

				if (result < 0)
				{
					cout << __FUNCTION__ << "@" << __LINE__ << "server.open failed !" << endl;

					printTRA("FIXS_AMIH_DSDHandler -  server.open failed !");

					dsdSteps = 1;

					sleep(10);

					break;
				}
				else
				{
					//Just for tracing ;)
					if (result > 0)
					{
						cout << __FUNCTION__ << "@" << __LINE__ << "Open with a warning !!" << endl;

						printTRA("FIXS_AMIH_DSDHandler -  Open with a warning!");

					}
					else
					{
						cout << __FUNCTION__ << "@" << __LINE__ << "Open successfully !!" << endl;

						printTRA("FIXS_AMIH_DSDHandler -  Open successfully!");

						openSuccess = true;
					}

					//Try to publish
					result = server.publish("AMIHWAP", "JTP"); //Name + App Domain //TODO

					if (result < 0)
					{
						printTRA("FIXS_AMIH_DSDHandler -  Server.publish failed !! ");

						cout << __FUNCTION__ << "@" << __LINE__ << "server.publish failed !" << endl;

						dsdSteps = 1;

						sleep(10);

						break;
					}
					else
					{
						//Connection done !
						dsdSteps = 2;

						//Just for tracing ;)

						if (result > 0)
						{
							cout << __FUNCTION__ << "@" << __LINE__ << "Publish return a warning !!" << endl;

							printTRA("FIXS_AMIH_DSDHandler -  Publish return a warning !!");

						}
						else
						{
							cout << __FUNCTION__ << "@" << __LINE__ << "Published successfully !!" << endl;

							printTRA("FIXS_AMIH_DSDHandler -  Published successfully !!");
						}
					}
				}//else open result
				break;

			case 2: //Retrieve DSD Handles

				cout << __FUNCTION__ << "@" << __LINE__ << "Start step 2 !" << endl;
				//retrieve handles number

				if (handle_count == 0)
				{
					result = server.get_handles(0, handle_count);

					if (result <= 0)
					{
						printTRA("FIXS_AMIH_DSDHandler -  Handles number failed !!");
						cout << __FUNCTION__ << "@" << __LINE__ << "Handles number failed !" << endl;
					}
				}

				cout << __FUNCTION__ << "@" << __LINE__ << "handle_count: " << handle_count << endl;

				handle_count++; //increase array len for the shutdown.
				handles = new acs_dsd::HANDLE[handle_count];

				result = server.get_handles(handles, handle_count);

				if (result < 0)
				{
					delete handles;

					cout << __FUNCTION__ << "@" << __LINE__ << "Handles failed !" << endl;

					printTRA("FIXS_AMIH_DSDHandler -  Handles failed !!");

				}
				else
				{
					dsdSteps = 3;

					shutdownEvent = FIXS_AMIH_Event::OpenNamedEvent(EVENT_NAME_SHUTDOWN);

					cout << __FUNCTION__ << "@" << __LINE__ << " handles[handle_count]: " << handles[handle_count] << endl;

					printTRA("FIXS_AMIH_DSDHandler -  Get Handles successfully !!");

				}

				break;

			case 3: //Wait for handles

				// handle events
				int waitRes = -1; //

				if (! tExit)
				{
					waitRes = FIXS_AMIH_Event::WaitForEvents(1, &shutdownEvent, 100000);
				}
				else
				{
					cout << __FUNCTION__ << "@" << __LINE__ << " EXIT Signaled "<< endl;
					break;
				}
				switch (waitRes)
				{
					case FIXS_AMIH_Event_RC_TIMEOUT:  // Time out, means startup supervision completed as expected

						sleep(1);
						break;

					default:

						if (waitRes == handle_count){ // Shutdown event received
							//						cout << __FUNCTION__ << "@" << __LINE__ << " Shutdown ;) !!" << endl;
							//						cout << __FUNCTION__ << "@" << __LINE__ << " waitRes: " << waitRes << endl;
							tExit = true;
							FIXS_AMIH_Event::ResetEvent(handles[handle_count]);
							sleep(1);
						}
						break;
				} //switch (waitRes)
				break;
		} //switch (dsdSteps)
	} //while(!tExit)


	for(int i = 0 ; i < handle_count - 1 ; i++)
	{
		FIXS_AMIH_Event::CloseEvent(handles[i]);
	}

	delete handles;

	//	cout << __FUNCTION__ << "@" << __LINE__ << "Disconnect from DSD " << endl;
	session.close();
	server.unregister();
	server.close();

	return result;
}

std::string FIXS_AMIH_DSDHandler::getOpt(char *buf, int bufSize)
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

int FIXS_AMIH_DSDHandler::splitParams(char *buf, std::list<std::string> &lstParams)
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


void FIXS_AMIH_DSDHandler::printTRA(std::string mesg)
{
	if (_trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, mesg.c_str());
		_trace->ACS_TRA_event(1, tmpStr);

	}
}
