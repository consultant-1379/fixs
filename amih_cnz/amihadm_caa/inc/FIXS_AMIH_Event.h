
#ifndef FIXS_AMIH_EVENT_H_
#define FIXS_AMIH_EVENT_H_

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
typedef int FIXS_AMIH_EventHandle;
const int FIXS_AMIH_Event_RC_ERROR   = -1;
const int FIXS_AMIH_Event_RC_TIMEOUT = -2;

class FIXS_AMIH_Event
{
    public:
        //
        // Create a new event. If the parameter name is set to NULL then a
        // unnamed event will be created
        //
        // Returns:
        //   Handle to the created event or FIXS_AMIH_Event_RC_ERROR if the
        //   event could not be created for some reason
        //
        static FIXS_AMIH_EventHandle CreateEvent(bool manualReset, bool initialState, const char* name);

        //
        // Opens a named event
        //
        // Returns:
        //   Handle to the named event or FIXS_AMIH_Event_RC_ERROR if the
        //   event could not be found
        //
        static FIXS_AMIH_EventHandle OpenNamedEvent(string name);

        //
        // Closes an event with handle as parameter, used for both named and unnamed events
        //
        static void CloseEvent(FIXS_AMIH_EventHandle eventHandle);

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
        //   FIXS_AMIH_Event_RC_ERROR   - Returned if some fault has occurred
        //   FIXS_AMIH_Event_RC_TIMEOUT - Returned if nothing has changed state in the given time slot
        //
        static int WaitForEvents(int count, FIXS_AMIH_EventHandle *eventHandles, int timeoutInMilliseconds);

        //
        // Flag event as set
        //
        static bool SetEvent(FIXS_AMIH_EventHandle eventHandle);

        //
        // Resets the event
        //
        static bool ResetEvent(FIXS_AMIH_EventHandle eventHandle);

        friend class ACE_Singleton<FIXS_AMIH_Event, ACE_Recursive_Thread_Mutex>;

    protected:
        FIXS_AMIH_Event();
        ~FIXS_AMIH_Event();
        static FIXS_AMIH_Event* instance();

    private:
        typedef struct {
            FIXS_AMIH_EventHandle handle;
            bool manualReset;
        } EventProperties_t;

        std::vector<EventProperties_t*> events_;
        std::map<string, EventProperties_t*> namedEvents_;

        typedef std::pair<string, FIXS_AMIH_Event::EventProperties_t*> NamedEventKeyTypePair;
        typedef std::vector<FIXS_AMIH_Event::EventProperties_t*>::iterator EventIteratorType;
        typedef std::map<string, FIXS_AMIH_Event::EventProperties_t*>::iterator NamedEventIteratorType;

        typedef ACE_Singleton<FIXS_AMIH_Event, ACE_Recursive_Thread_Mutex> instance_;


        // Private methods
        NamedEventIteratorType findNamedEventByHandle(FIXS_AMIH_EventHandle handle);
        EventIteratorType findEventByHandle(FIXS_AMIH_EventHandle handle);


        // Private variables
        ACE_RW_Mutex lock_;
};


#endif /* FIXS_AMIH_Event_H_ */
