
#include "FIXS_TREFC_CmdHandler.h"

//#include "ACS_TRA_trace.h"
//#include "ACS_TRA_Logging.h"
using namespace std;
namespace cmdHandler{
  const int threadTermination = -1;
};
/*============================================================================
	ROUTINE: FMS_CPF_CmdHandler
 ============================================================================ */
FIXS_TREFC_CmdHandler::FIXS_TREFC_CmdHandler()
{
	// Initialize the svc state flag
	svc_run = false;
	shutdownEvent = FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);

}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
FIXS_TREFC_CmdHandler::~FIXS_TREFC_CmdHandler()
{
	//FIXS_TREFC_Event::CloseEvent(shutdownEvent);

	// Delete all queued request

/*	while(!((bool)m_Activation_Queue.is_empty()) )
	{
		// Dequeue the next method object
		std::cout << "FIXS_TREFC_CmdHandler dequeue"<< std::endl;
		auto_ptr<ACE_Method_Request> cmdRequest(m_Activation_Queue.dequeue());
	}
*/
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_TREFC_CmdHandler::open(void *args)
{
	int result;

	UNUSED(args);

	svc_run = true;
	result = activate();

    if(0 != result)
    {
    	svc_run = false;
    }
  	return result;
}

/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_TREFC_CmdHandler::svc()
{
	int result = 0;

	int events[1];
	events[0]=shutdownEvent;

	while(svc_run)
	{
		int eventIndex = FIXS_TREFC_Event::WaitForEvents(1,events,0);

		if (eventIndex == 0)   // Shutdown event
		{
				svc_run=false;
				break;
		}

/*		auto_ptr<ACE_Method_Request> cmdRequest(m_Activation_Queue.dequeue());
		if(cmdRequest->call() == cmdHandler::threadTermination )
		{
			cout << "FIXS_TREFC_CmdHandler threadTermination ! \n"<< endl;
			svc_run = false;
			//sleep(1);
		}
*/
	}
	return result;
}

/*============================================================================
	ROUTINE: enqueue
 ============================================================================ */
/*int FIXS_TREFC_CmdHandler::enqueue(ACE_Method_Request* cmdRequest)
{
	return m_Activation_Queue.enqueue(cmdRequest);
}
*/
