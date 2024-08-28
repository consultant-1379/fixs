/*
 * FIXS_TREFC_HA_AppManager.cpp
 *
 *  Created on: 27 Jun 2011
 *      Author: eanform
 */
#include "FIXS_TREFC_HA_AppManager.h"

ACE_THR_FUNC_RETURN svc_run(void *);

FIXS_TREFC_HA_AppManager::FIXS_TREFC_HA_AppManager(const char* daemon_name):ACS_APGCC_ApplicationManager(daemon_name){
        /* create the pipe for shutdown handler */
        Is_terminated = FALSE;
        startSRV = NULL;
        //startSRV = new (std::nothrow) FIXS_TREFC_Server();

        if ( (pipe(readWritePipe)) < 0) {
                syslog(LOG_ERR, "pipe creation FAILED");
        }

        if ( (fcntl(readWritePipe[0], F_SETFL, O_NONBLOCK)) < 0) {
                syslog(LOG_ERR, "pipe fcntl on readn");
        }

        if ( (fcntl(readWritePipe[1], F_SETFL, O_NONBLOCK)) < 0) {
                syslog(LOG_ERR, "pipe fcntl on writen");
        }

    	app_thread_id=0;

        FIXS_TREFC_logging = FIXS_TREFC_Logger::getLogInstance();


}

FIXS_TREFC_HA_AppManager::~FIXS_TREFC_HA_AppManager(){
	//destroy dynamic allocations

}


ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState){

        /* Check if we have received the ACTIVE State Again.
         * This means that, our application is already Active and
         * again we have got a callback from AMF to go active.
         * Ignore this case anyway. This case should rarely happens
         */
		Is_terminated = FALSE;
        ACE_TCHAR state[1] = {'A'};
        if(ACS_APGCC_AMF_HA_ACTIVE == previousHAState)
                return ACS_APGCC_SUCCESS;

        /* Our application has received state ACTIVE from AMF.
         * Start off with the activities needs to be performed
         * on ACTIVE
         */
         /* Check if it is due to State Transition ( Passive --> Active)*/
        if ( ACS_APGCC_AMF_HA_UNDEFINED != previousHAState ){
                syslog(LOG_INFO, "State Transision happend. Becomming Active now");
                /* Inform the thread to go "active" state */
                write(readWritePipe[1], &state, sizeof(state));
                return ACS_APGCC_SUCCESS;
        }

        /* Handle here what needs to be done when you are given ACTIVE State */
        syslog(LOG_INFO, "My Application Component received ACTIVE state assignment!!!");

        /* Create a thread with the state machine (active, passive, stop states)
         * and start off with "active" state activities.
         */

        /* spawn thread */
        const ACE_TCHAR* thread_name = "ApplicationThread";
        ACE_HANDLE threadHandle = ACE_Thread_Manager::instance()->spawn(&svc_run,
                                                                        (void *)this ,
                                                                        THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
                                                                        &app_thread_id,
                                                                        0,
                                                                        ACE_DEFAULT_THREAD_PRIORITY,                                                                        -1,
                                                                        0,
                                                                        ACE_DEFAULT_THREAD_STACKSIZE,
                                                                        &thread_name);
        if (threadHandle == -1){
                syslog(LOG_ERR, "Error creating the application thread");
                return ACS_APGCC_FAILURE;
        }

        /* check if the thread is spawned */
        ACE_UINT32 tState;
        ACE_Thread_Manager::instance()->thr_state(app_thread_id, tState);

        syslog(LOG_INFO, "Thread state [%u]", tState);

        if (tState == THR_RUNNING || tState == THR_SPAWNED) {
        	syslog(LOG_INFO, "Posting START to the application thread");
        	write(readWritePipe[1], &state, sizeof(state));
        	return ACS_APGCC_SUCCESS;
        }
        else
        	syslog(LOG_INFO, "Thread state [%u]", tState);


        return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::performStateTransitionToPassiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState) {

	/* Check if we have received the PASSIVE State Again.
	 * This means that, our application was already Passive and
	 * again we have got a callback from AMF to go passive.
	 * Ignore this case anyway. This case should rarely happens.
	 */

	Is_terminated = FALSE;
	ACE_TCHAR state[1] = {'P'};
	if(ACS_APGCC_AMF_HA_STANDBY == previousHAState)
		return ACS_APGCC_SUCCESS;

	/* Our application has received state PASSIVE from AMF.
	 * Check if the state received is due to State Transistion.
	 * (Active->Passive).
	 */
	if ( ACS_APGCC_AMF_HA_UNDEFINED != previousHAState ){
		syslog(LOG_INFO, "State Transision happend. Becomming Passive now");
		/* Inform the thread to go "passive" state*/
		write(readWritePipe[1], &state, sizeof(state));
		return ACS_APGCC_SUCCESS;
	}

	/* Create a thread with the state machine (active, passive, stop states)
	 * and start off the thread with "passive" state.
	 */

	syslog(LOG_INFO, "My Application Component received STANDBY state assignment!!!");

	const ACE_TCHAR* thread_name = "ApplicationThread";
	ACE_HANDLE threadHandle = ACE_Thread_Manager::instance()->spawn(&svc_run,
			(void *)this ,
			THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
			&app_thread_id,
			0,
			ACE_DEFAULT_THREAD_PRIORITY,
			-1,
			0,
			ACE_DEFAULT_THREAD_STACKSIZE,
			&thread_name);
	if (threadHandle == -1){
		syslog(LOG_ERR, "Error creating the application thread");
		return ACS_APGCC_FAILURE;
	}

	/* check if the thread is spawned */
	ACE_UINT32 tState;
	ACE_Thread_Manager::instance()->thr_state(app_thread_id, tState);

	syslog(LOG_INFO, "Thread state [%u]", tState);

	if ( tState == THR_RUNNING || tState == THR_SPAWNED) {
		syslog(LOG_INFO, "Posting START to the application thread");
		write(readWritePipe[1], &state, sizeof(state));
		return ACS_APGCC_SUCCESS;
	}
	else
		syslog(LOG_INFO, "Thread state [%u]", tState);

	return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
        (void)previousHAState;
        ACE_TCHAR state[1] = {'S'};

        /* We were active and now losing active state due to some shutdown admin
         * operation performed on our SU.
         * Inform the thread to go to "stop" state
         */

        syslog(LOG_INFO, "My Application Component received QUIESING state assignment!!!");

        /* Inform the thread to go "stop" state */
        if ( !Is_terminated ) {
        	write(readWritePipe[1], &state, sizeof(state));
        	syslog(LOG_INFO, "Waiting for the application thread to terminate.");
        	ACE_Thread_Manager::instance()->join(app_thread_id);
        	Is_terminated = TRUE;
        }

        return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT previousHAState)
{
        (void)previousHAState;
        ACE_TCHAR state[1] = {'S'}; /* We were Active and now losting Active state due to Lock admin
         * operation performed on our SU.
         * Inform the thread to go to "stop" state
         */

        syslog(LOG_INFO, "My Application Component received QUIESCED state assignment!");

        /* Inform the thread to go "stop" state */
        if ( !Is_terminated ) {
        	write(readWritePipe[1], &state, sizeof(state));
        	syslog(LOG_INFO, "Waiting for the application thread to terminate.");
        	ACE_Thread_Manager::instance()->join(app_thread_id);
        	Is_terminated = TRUE;
        }


        return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::performComponentHealthCheck(void)
{

        /* Application has received health check callback from AMF. Check the
         * sanity of the application and reply to AMF that you are ok.
         */
       // syslog(LOG_INFO, "My Application Component received healthcheck query!!!");

        return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::performComponentTerminateJobs(void)
{
        /* Application has received terminate component callback due to
         * LOCK-INST admin opreration perform on SU. Terminate the thread if
         * we have not terminated in performComponentRemoveJobs case or double
         * check if we are done so.
         */
        syslog(LOG_INFO, "My Application Component received terminate callback!!!");
	
	ACE_TCHAR state[1] = {'S'}; //HY72256

        /* Inform the thread to go "stop" state */
        if ( !Is_terminated ) {
                write(readWritePipe[1], &state, sizeof(state));
                syslog(LOG_INFO, "Waiting for the application thread to terminate.");
                ACE_Thread_Manager::instance()->join(app_thread_id);
                Is_terminated = TRUE;
        }

        return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::performComponentRemoveJobs(void)
{

        /* Application has received Removal callback. State of the application
         * is neither Active nor Standby. This is with the result of LOCK admin operation
         * performed on our SU. Terminate the thread by informing the thread to go "stop" state.
         */

        ACE_TCHAR state[1] = {'S'}; 

        syslog(LOG_INFO, "Application Assignment is removed now");

        /* Inform the thread to go "stop" state */
        if ( !Is_terminated ) {
        	write(readWritePipe[1], &state, sizeof(state));
        	syslog(LOG_INFO, "Waiting for the application thread to terminate.");
        	ACE_Thread_Manager::instance()->join(app_thread_id);
        	Is_terminated = TRUE;
        }

        return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::performApplicationShutdownJobs() {

        syslog(LOG_ERR, "Shutting down the application");
        ACE_TCHAR state[1] = {'S'};

        /* Inform the thread to go "stop" state */
        if ( !Is_terminated ) {
        	write(readWritePipe[1], &state, sizeof(state));
        	syslog(LOG_INFO, "Waiting for the application thread to terminate.");
        	ACE_Thread_Manager::instance()->join(app_thread_id);
        	Is_terminated = TRUE;
        }

        return ACS_APGCC_SUCCESS;
}

ACE_THR_FUNC_RETURN svc_run(void *ptr){
	FIXS_TREFC_HA_AppManager *haObj = (FIXS_TREFC_HA_AppManager*) ptr;
        haObj->svc();
        return 0;
}
ACS_APGCC_ReturnType FIXS_TREFC_HA_AppManager::svc(){

        struct pollfd fds[1];
        nfds_t nfds = 1;
        ACE_INT32 ret;
        ACE_Time_Value timeout;

        ACE_INT32 retCode;
        //FIXS_TREFC_Server *startSRV = new (std::nothrow) FIXS_TREFC_Server();

        syslog(LOG_INFO, "Starting Application Thread");

        __time_t secs = 5;
        __suseconds_t usecs = 0;
        timeout.set(secs, usecs);


        fds[0].fd = readWritePipe[0];
        fds[0].events = POLLIN;

        while(true)
        {
                ret = ACE_OS::poll(fds, nfds, 0); // poll can also be a blocking call, such case timeout = 0

                if (ret == -1) {
                        if (errno == EINTR)
                                continue;
                        syslog(LOG_ERR,"poll Failed - %s, Exiting...",strerror(errno));
                        kill(getpid(), SIGTERM);
                        return ACS_APGCC_FAILURE;
                }

                if (ret == 0){
                        //syslog(LOG_INFO, "timeout on ACE_OS::poll");
                        continue;
                }

                if (fds[0].revents & POLLIN){
                        ACE_TCHAR ha_state[1] = {'\0'};
                        ACE_TCHAR* ptr = (ACE_TCHAR*) &ha_state;
                        ACE_INT32 len = sizeof(ha_state);

                        while (len > 0){
                                retCode=read(readWritePipe[0], ptr, len);
                                if ( retCode < 0 && errno != EINTR){
                                        syslog(LOG_ERR, "Read interrupted by error: [%s]",strerror(errno));
                                        kill(getpid(), SIGTERM);
                                        return ACS_APGCC_FAILURE;
                                }
                                else {
                                        ptr += retCode;
                                        len -= retCode;
                                }
                                if (retCode == 0)
                                   break;
                        }

                        if ( len != 0) {
                                syslog(LOG_ERR, "Improper Msg Len Read [%d]", len);
                                kill(getpid(), SIGTERM);
                                return ACS_APGCC_FAILURE;
                        }
                        len = sizeof(ha_state);

                        if (ha_state[0] == 'A'){
                                syslog(LOG_ERR, "######################################## Thread:: Application is Active");

                                /* start application work */
                                //FIXS_TREFC_Server startSRV; // NO
                                if (startSRV == NULL){
                                    startSRV = new (std::nothrow) FIXS_TREFC_Server();
                                	startSRV->run();
                                }
                        }

                        if (ha_state[0] == 'P'){
                                syslog(LOG_ERR, "Thread:: Application is Passive");
                                /* do nothing */
                        }

                        if (ha_state[0] == 'S'){
                        	syslog(LOG_ERR, "Thread:: Request to stop application");

                        	/* Request to stop the thread, perform the gracefull activities here */

                        	FIXS_TREFC_EventHandle handle = FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);
                        	if (handle >= 0)
                        		(void) FIXS_TREFC_Event::SetEvent(handle);
				FIXS_TREFC_Manager::getInstance()->setStopOperation();

                        	if(startSRV){
                        		sleep(TREFC_Util::Daemon_Timeout);
                        		delete startSRV;
                        		startSRV = NULL;
                        	}

                        	if (handle >= 0)
                        		FIXS_TREFC_Event::ResetEvent(handle);

                        	break;
                        }
                }
        }

        // TODO: DELETE startSRV;
        syslog(LOG_INFO, "Application Thread Terminated successfully");
        return ACS_APGCC_SUCCESS;
}

