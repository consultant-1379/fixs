/*
 * FIXS_CCH_ThreadActions.h
 *
 *  Created on: Apr 4, 2012
 *      Author: eanform
 */

#ifndef FIXS_CCH_ThreadActions_H_
#define FIXS_CCH_ThreadActions_H_

#include <ace/Task.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <map>

#include "acs_apgcc_paramhandling.h"
#include "acs_apgcc_adminoperation.h"

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include "Client_Server_Interface.h"
#include "FIXS_CCH_Event.h"
//#include "FIXS_CCH_Util.h"

#include "FIXS_CCH_UpgradeManager.h"
//#include "FIXS_CCH_SoftwareMap.h"
#include "FIXS_CCH_Installer.h"

class FIXS_CCH_Installer;

class FIXS_CCH_ThreadActions: public ACE_Task_Base {

	typedef enum
	{
		CMD_IMPORT_PACKAGE,	    //0
		CMD_SET_DEFAULT,		//1
		CMD_PREPARE,			//2
		CMD_ACTIVATE,			//3
		CMD_CANCEL,				//4
		CMD_COMMIT,				//5
		NO_CMD					//6

	} CommandId;

	class ActionMapData
	{
	public:
		// Data Members for Class Attributes

		int m_id;

		CCH_Util::upgradeActionType m_actionT;

		std::string m_magazine;

		std::string m_slot;

		std::string m_container;

		std::string m_path;

		int m_fbn;

		int m_errorCode;

		int m_upgradeType;
	
		bool m_force;

		// Additional Public Declarations

	protected:
		// Additional Protected Declarations

	private:
		// Additional Private Declarations

	};

 public:
	/**
		@brief	Constructor of FIXS_CCH_ThreadActions class
	*/
	FIXS_CCH_ThreadActions();

	/**
		@brief	Destructor of FIXS_CCH_ThreadActions class
	*/
	virtual ~FIXS_CCH_ThreadActions();

	/**
	   @brief  		Run by a daemon thread
	*/
	virtual int svc(void);

	/**
	   @brief  		This method initializes a task and prepare it for execution
	*/
	virtual int open (void *args = 0);

	int stop();

//	inline void setNewDefaultPackage(std::string package){ defaultPackage = package;};

//	inline void setPathPackage(std::string software){ pathPackage = software;};

//	inline void setFbnPackage(int typeBoard){ fbn = typeBoard;};

	inline bool isRunningThread(){ return running;};

	inline void setContainerToUpgrade(std::string package){container = package;};

	int setEvent (CommandId event);

	int prepare(std::string sMag, std::string sSlot, std::string package, int upgradeType, bool force);

	int activateSw(std::string sMag, std::string sSlot);

	int cancel(std::string sMag, std::string sSlot);

	int commit(std::string sMag, std::string sSlot);

	int insert_software(std::string pathSw, int fbn, int errorCode = -1);

	int change_default_package();

	int prepareEpb1(std::string sMag, std::string sSlot, std::string package);

	int prepareEvoEt(std::string sMag, std::string sSlot, std::string package);

	int prepare_Shelf(std::string sMag, std::string package, int fbn);

	int prepare_all(std::string package, int fbn);

	bool isInstallationOngoing();



 private:

	bool tExit;

	bool running;

	bool installing;

	ACS_TRA_trace *_trace;

	ACS_TRA_Logging *FIXS_CCH_logging;

	FIXS_CCH_EventHandle m_shutdownEvent;

//	FIXS_CCH_EventHandle m_stopEvent;

	FIXS_CCH_EventHandle m_CmdEvent;

//	std::string pathPackage;
//
//	int fbn;
//
//	std::string defaultPackage;
//
//	std::string magazine;
//
//	std::string slot;

	std::string container;

	int countAction;

//	CCH_Util::upgradeActionType actionType;

	void handleCommandEvent ();

	int insert_action(std::string pathSw, int fbn, int errorCode);

	int default_action();

	int prepare_action(std::string mag, std::string slot, std::string cxp, int upgradeType , bool force);

	int activate_action(std::string sMag, std::string sSlot);

	int cancel_action(std::string sMag, std::string sSlot);

	int commit_action(std::string sMag, std::string sSlot);

	bool setManagedSwPackageStrunct(int state, int result, int resultInfo);

	typedef std::map<int, FIXS_CCH_ThreadActions::ActionMapData> mapObjectAction;

	mapObjectAction objectAction;

	void addObjectAction(CCH_Util::upgradeActionType actType, std::string sMag = "", std::string sSlot = "", std::string package = "", std::string path = "", int fbn = 0, int errorCode = -1, int upgradeType = 0, bool force = false);

	void removeObjectAction(int id);

	void cleanActionMap();

	void prepareEpb1_action(std::string sMag, std::string sSlot, std::string package);

	void prepareEvoEt_action(std::string sMag, std::string sSlot, std::string package);

	void prepare_Shelf_action(std::string sMag, std::string package, int fbn);

	void prepare_all_action(std::string package, int fbn);


};


#endif /* FIXS_CCH_ThreadActions_H_ */
