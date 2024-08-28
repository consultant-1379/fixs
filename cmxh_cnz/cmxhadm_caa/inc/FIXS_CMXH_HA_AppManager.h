#ifndef FIXS_CMXH_HA_APPMANAGER_H
#define FIXS_CMXH_HA_APPMANAGER_H

#include "ACS_APGCC_ApplicationManager.h"
#include "unistd.h"
#include "syslog.h"
#include "ace/Task.h"
#include "ace/OS_NS_poll.h"
#include "FIXS_CMXH_Server.h"
//#include "FIXS_CMXH_Util.h"
#include "ACS_TRA_Logging.h"

#define THR_SPAWNED 0x00000001
#define THR_RUNNING 0x00000002


class FIXS_CMXH_HA_AppManager: public ACS_APGCC_ApplicationManager {

   private:

        int readWritePipe[2];
        ACS_APGCC_BOOL Is_terminated;
        ACE_thread_t app_thread_id;

   public:
        FIXS_CMXH_HA_AppManager(const char* daemon_name);
        virtual ~FIXS_CMXH_HA_AppManager();

        ACS_APGCC_ReturnType performStateTransitionToActiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState);
        ACS_APGCC_ReturnType performStateTransitionToPassiveJobs(ACS_APGCC_AMF_HA_StateT previousHAState);
        ACS_APGCC_ReturnType performStateTransitionToQueisingJobs(ACS_APGCC_AMF_HA_StateT previousHAState);
        ACS_APGCC_ReturnType performStateTransitionToQuiescedJobs(ACS_APGCC_AMF_HA_StateT previousHAState);
        ACS_APGCC_ReturnType performComponentHealthCheck(void);
        ACS_APGCC_ReturnType performComponentTerminateJobs(void);
        ACS_APGCC_ReturnType performComponentRemoveJobs (void);
        ACS_APGCC_ReturnType performApplicationShutdownJobs(void);

        ACS_APGCC_ReturnType svc(void);

   private:

        FIXS_CMXH_Server *startSRV;
		ACS_TRA_Logging *FIXS_CMXH_logging;


};

#endif /* APG_APP1_CLASS_H */ 
