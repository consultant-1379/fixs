// FIXS_CCH_StateMachine
#include "FIXS_CCH_StateMachine.h"

FIXS_CCH_StateMachine::FIXS_CCH_StateMachine()
      : m_exit(false),
        m_running(false)
{
}

int FIXS_CCH_StateMachine::open (void *args)
{
	UNUSED(args);
	return 0;
}

int  FIXS_CCH_StateMachine::svc(void)
{
//	FIXS_CCH_StateMachine * fsm = reinterpret_cast<FIXS_CCH_StateMachine *>(parameters);
//	FIXS_CCH_CriticalSectionGuard lock(fsm->m_criticalSection);

	/*if (fsm && !fsm->isRunning())
	{
		fsm->run();
	}
	else
	{
		//trace
	}
*/
	m_running = true;
	return 0;
}
