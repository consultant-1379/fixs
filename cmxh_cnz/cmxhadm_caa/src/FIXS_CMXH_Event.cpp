/*
 * FIXS_CMXH_Event.cpp
 *
 */

#include "FIXS_CMXH_Event.h"

using std::map;

// macros
#define FD_MAX(max_now,x)     ((x) > (max_now) ? (x) : (max_now))


using namespace std;

//
// Protected constructor, we're using the singleton pattern
//
FIXS_CMXH_Event::FIXS_CMXH_Event()
{
}
FIXS_CMXH_Event::~FIXS_CMXH_Event()
{

}

FIXS_CMXH_Event* FIXS_CMXH_Event::instance()
{
    return instance_::instance();
}


FIXS_CMXH_EventHandle FIXS_CMXH_Event::CreateEvent(bool manualReset, bool initialState, const char* name)
{
    FIXS_CMXH_EventHandle eventHandle = -1;

    if (name != NULL) {
        // named event, check if it already exists
        if ((eventHandle = instance()->OpenNamedEvent(name)) != -1) {
            // do not create any new event as this already exists
            return eventHandle;
        }
    }

    // allocate new event
    if ((eventHandle = eventfd(0, 0)) == -1) {
        // failed to create eventlock_
        return FIXS_CMXH_Event_RC_ERROR;
    }

    // create event object
    EventProperties_t *properties = new EventProperties_t();

    properties->handle = eventHandle;
    properties->manualReset = manualReset;

    if (name != NULL) {
        // named event
        NamedEventKeyTypePair event(name, properties);

        instance()->lock_.acquire_write();
        instance()->namedEvents_.insert(event);
        instance()->lock_.release();


    } else {
        // unnamed event
        instance()->lock_.acquire_write();
        instance()->events_.push_back(properties);
        instance()->lock_.release();

    }

    if (initialState) {
        // event requested to be set from start
        instance()->SetEvent(eventHandle);
    }

    return eventHandle;
}


FIXS_CMXH_EventHandle FIXS_CMXH_Event::OpenNamedEvent(string name)
{
    instance()->lock_.acquire_read();

    NamedEventIteratorType it = instance()->namedEvents_.end();

    if ((it = instance()->namedEvents_.find(name)) == instance()->namedEvents_.end()) {
        // named event does not exist in our map
        instance()->lock_.release();
        return FIXS_CMXH_Event_RC_ERROR;
    }

    FIXS_CMXH_EventHandle handle = it->second->handle;
    instance()->lock_.release();

    return handle;
}


void FIXS_CMXH_Event::CloseEvent(FIXS_CMXH_EventHandle eventHandle)
{
    instance()->lock_.acquire_write();

    // check the list with named events
    NamedEventIteratorType neit = instance()->findNamedEventByHandle(eventHandle);

    if (neit != instance()->namedEvents_.end()) {


        close(eventHandle);     // close event handle

        delete neit->second;    // free EventProperties_t
        instance()->namedEvents_.erase(neit);
        instance()->lock_.release();
        return;
    }

    // check the list with unnamed events
    EventIteratorType ueit = instance()->findEventByHandle(eventHandle);

    if (ueit != instance()->events_.end()) {


        close(eventHandle);     // close event handle

        EventProperties_t *ptr = *ueit;
        instance()->events_.erase(ueit);
        delete ptr;

        instance()->lock_.release();
        return;
    }

    instance()->lock_.release();
}


int FIXS_CMXH_Event::WaitForEvents(int count, int *eventHandles, int timeoutInMilliseconds)
{
    fd_set fds;
    int nfds = 0;
    struct timeval *tv = NULL;

    // set timeout value (if any)
    if (timeoutInMilliseconds > 0) {
        tv = new struct timeval;

        unsigned long seconds = timeoutInMilliseconds / 1000;
        unsigned long ms = timeoutInMilliseconds % 1000;

        tv->tv_sec = seconds;
        tv->tv_usec = ms * 1000;
    }

    FD_ZERO(&fds);

    // go thru all fd's
    for (int i = 0;i < count; i++)
    {
    	if (eventHandles[i] < 0) {//Invalid handle
    		if (tv != NULL) {
    			// free allocated memory
    			delete tv;
    			tv = NULL;
    		}
    		return FIXS_CMXH_Event_RC_ERROR;
    	}

        FD_SET(eventHandles[i], &fds);

        // we want the highest fds
        nfds = FD_MAX(nfds, eventHandles[i]);
    }

    // wait for something to happen
    int ret = select(nfds + 1, &fds, NULL, NULL, tv);

    if (tv != NULL) {
        // free allocated memory
        delete tv;
        tv = NULL;
    }

    if (ret == 0) {
        // timeout has occurred
        return FIXS_CMXH_Event_RC_TIMEOUT;

    } else if (ret == -1) {
        // a fault has occurred
        return FIXS_CMXH_Event_RC_ERROR;
    }    // check the list of unnamed events

    int n = -1;

    // find out which one of the fds that has been signaled
    for (int i = 0;i < count; i++) {
        if (FD_ISSET(eventHandles[i], &fds)) {
            n = i;
            break;
        }
    }

    if (n != -1)
    {
        int handleActivated = eventHandles[n];

        FIXS_CMXH_Event::EventProperties_t *ep = NULL;

        // locate what type of event this is, start in the named events list
        instance()->lock_.acquire_read();
        FIXS_CMXH_Event::NamedEventIteratorType neit = instance()->findNamedEventByHandle(handleActivated);
        instance()->lock_.release();

        if (neit != instance()->namedEvents_.end()) {
            ep = neit->second;
        }

        if (ep == NULL) {
            // no property found in the named events list, search unnamed events
            instance()->lock_.acquire_read();
            FIXS_CMXH_Event::EventIteratorType ueit = instance()->findEventByHandle(handleActivated);
            instance()->lock_.release();

            if (ueit != instance()->events_.end()) {
                ep = (*ueit);
            }
        }

        if (ep != NULL && !ep->manualReset) {
            // we should reset the event
            ResetEvent(handleActivated);
        }

        return n;
    }

    // should never come here
    return FIXS_CMXH_Event_RC_ERROR;
}


bool FIXS_CMXH_Event::SetEvent(FIXS_CMXH_EventHandle eventHandle)
{
    ssize_t sz;
    uint64_t n = 1;

    sz = write(eventHandle, &n, sizeof(uint64_t));
    if (sz != sizeof(uint64_t)) {
        // write fault
        //ACS_CS_TRACE((FIXS_CMXH_Event_TRACE, "(%t) Failed to set event for handle: %d", eventHandle));
        return false;
    }

    return true;
}


bool FIXS_CMXH_Event::ResetEvent(FIXS_CMXH_EventHandle eventHandle)
{
    fd_set fds;
    struct timeval tv;

    // poll and make sure that it is possible to reset the event
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    FD_ZERO(&fds);

    if (eventHandle < 0) {//Invalid handle

    	return FIXS_CMXH_Event_RC_ERROR;
    }

    FD_SET(eventHandle, &fds);

    int ret = select(eventHandle + 1, &fds, NULL, NULL, &tv);

    if (ret == -1) {
        // something bad has happened


        return false;

    } else if (ret == 0) {
        // no data available (or rather, event not set)
        return false;
    }

    // reset event
    ssize_t s;
    uint64_t u = 0;

    s = read(eventHandle, &u, sizeof(uint64_t));

    if (s != sizeof(uint64_t)) {
        return false;
    }

    return true;
}


// NOTE! Must be wrapped inside acquire/release statement
FIXS_CMXH_Event::NamedEventIteratorType FIXS_CMXH_Event::findNamedEventByHandle(FIXS_CMXH_EventHandle handle)
{
    // check the list with named events
    NamedEventIteratorType neit = instance()->namedEvents_.begin();

    for (;neit != instance()->namedEvents_.end(); neit++) {
        // check if the handle is in the named events list
        if (neit->second->handle == handle) {
            break;
        }
    }

    return neit;
}


// NOTE! Must be wrapped inside acquire/release statement
FIXS_CMXH_Event::EventIteratorType FIXS_CMXH_Event::findEventByHandle(FIXS_CMXH_EventHandle handle)
{
    // check the list of unnamed events
    EventIteratorType ueit = instance()->events_.begin();

    for (;ueit != instance()->events_.end(); ueit++) {
        if ((*ueit)->handle == handle) {
            break;
        }
    }

    return ueit;
}
