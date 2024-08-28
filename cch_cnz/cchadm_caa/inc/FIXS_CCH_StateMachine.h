
#ifndef FIXS_CCH_StateMachine_h_
#define FIXS_CCH_StateMachine_h_ 1

#include <iostream>
#include <sstream>
#include <string>
#include <ace/Task.h>
#include <ace/ACE.h>

// To avoid warning about unused parameter
#define UNUSED(expr) do { (void)(expr); } while (0)


class FIXS_CCH_StateMachine :  public ACE_Task_Base {

  public:
    //## Constructors (generated)
      FIXS_CCH_StateMachine();

    //## Destructor (generated)
      virtual ~FIXS_CCH_StateMachine(){};

      virtual void stop ();

      //## Operation: isRunning%4C0383F80327
      bool isRunning ();


      virtual int svc(void);

      	/**
      	   @brief  		This method initializes a task and prepare it for execution
      	*/
      virtual int open (void *args = 0);

      	/**
      	   @brief  		This method get the stop handle to terminate the svc thread
      	*/

  protected:

      bool m_exit;

      ACE_thread_mutex_t cs;

      bool m_running;

  private:
    //## Constructors (generated)
      FIXS_CCH_StateMachine(const FIXS_CCH_StateMachine &right);

    //## Assignment Operation (generated)
      FIXS_CCH_StateMachine & operator=(const FIXS_CCH_StateMachine &right);


  private: //## implementation


};


inline  void FIXS_CCH_StateMachine::stop ()
{
   	m_exit = true;
  	m_running = false;
}

inline  bool FIXS_CCH_StateMachine::isRunning ()
{
	return m_running;
}


#endif
