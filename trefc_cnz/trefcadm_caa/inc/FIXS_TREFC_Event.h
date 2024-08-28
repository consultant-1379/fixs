
#ifndef FIXS_TREFC_EVENT_H_
#define FIXS_TREFC_EVENT_H_


#include <map>
#include <vector>
#include <iostream>

#include <stdlib.h>
#include <sys/eventfd.h>
#include "ace/Mutex.h"
#include "ace/RW_Mutex.h"
#include "ace/Singleton.h"
#include "ace/Recursive_Thread_Mutex.h"

using std::string;
typedef int FIXS_TREFC_EventHandle;
const int FIXS_TREFC_Event_RC_ERROR   = -1;
const int FIXS_TREFC_Event_RC_TIMEOUT = -2;

class FIXS_TREFC_Event
{
    public:
        //
        // Create a new event. If the parameter name is set to NULL then a
        // unnamed event will be created
        //
        // Returns:
        //   Handle to the created event or FIXS_TREFC_Event_RC_ERROR if the
        //   event could not be created for some reason
        //
        static FIXS_TREFC_EventHandle CreateEvent(bool manualReset, bool initialState, const char* name);

        //
        // Opens a named event
        //
        // Returns:
        //   Handle to the named event or FIXS_TREFC_Event_RC_ERROR if the
        //   event could not be found
        //
        static FIXS_TREFC_EventHandle OpenNamedEvent(string name);

        //
        // Closes an event with handle as parameter, used for both named and unnamed events
        //
        static void CloseEvent(FIXS_TREFC_EventHandle eventHandle);

        //
        // Wait for multiple events
        //
        // Parameters:
        //   count         - Specifies how many events the eventHandles array contains
        //   *eventHandles - A array of handles to events (or file descriptors) that we're going to wait for.
        //                   So all file handles, socket descriptors can be sent in also.
        //   timeoutInMilliseconds - Timeout in ms (0 = INFINITE timeout)
        //
        // Returns:
        //   If successful then the return value will return the index to the handle that has changed state.
        //   FIXS_TREFC_Event_RC_ERROR   - Returned if some fault has occurred
        //   FIXS_TREFC_Event_RC_TIMEOUT - Returned if nothing has changed state in the given time slot
        //
        static int WaitForEvents(int count, FIXS_TREFC_EventHandle *eventHandles, int timeoutInMilliseconds);

        //
        // Flag event as set
        //
        static bool SetEvent(FIXS_TREFC_EventHandle eventHandle);

        //
        // Resets the event
        //
        static bool ResetEvent(FIXS_TREFC_EventHandle eventHandle);

        friend class ACE_Singleton<FIXS_TREFC_Event, ACE_Recursive_Thread_Mutex>;

    protected:
        FIXS_TREFC_Event();
        ~FIXS_TREFC_Event();
        static FIXS_TREFC_Event* instance();

    private:
        typedef struct {
            FIXS_TREFC_EventHandle handle;
            bool manualReset;
        } EventProperties_t;

        std::vector<EventProperties_t*> events_;
        std::map<string, EventProperties_t*> namedEvents_;

        typedef std::pair<string, FIXS_TREFC_Event::EventProperties_t*> NamedEventKeyTypePair;
        typedef std::vector<FIXS_TREFC_Event::EventProperties_t*>::iterator EventIteratorType;
        typedef std::map<string, FIXS_TREFC_Event::EventProperties_t*>::iterator NamedEventIteratorType;

        typedef ACE_Singleton<FIXS_TREFC_Event, ACE_Recursive_Thread_Mutex> instance_;


        // Private methods
        NamedEventIteratorType findNamedEventByHandle(FIXS_TREFC_EventHandle handle);
        EventIteratorType findEventByHandle(FIXS_TREFC_EventHandle handle);


        // Private variables
        ACE_RW_Mutex lock_;
};

#endif // FIXS_TREFC_Event_h


