/*
 * FIXS_CCH_IMM_Util.h
 *
 *  Created on: Oct 5, 2011
 *      Author: eanform
 */

#ifndef FIXS_CCH_IMM_UTIL_H_
#define FIXS_CCH_IMM_UTIL_H_

#include "MyImplementer_2.h"
#include "FIXS_CCH_Event.h"
#include "Client_Server_Interface.h"

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "ace/OS_NS_poll.h"
#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include <ace/Task.h>

#include "ACS_APGCC_OiHandler.h"
#include "ACS_CC_Types.h"
#include "ACS_APGCC_Util.H"
#include "acs_apgcc_omhandler.h"

class MyThread_IMM2 : public ACE_Task_Base {
public :

	/**Constructor**/
	MyThread_IMM2(MyImplementer_2 *p_user1)
	:implementer(0){
		implementer = p_user1;
		isStop = false;
		isRunning = false;
		StopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);
	}

	/**Destructor**/
	virtual ~MyThread_IMM2(){
		if (implementer)
		{
			implementer = NULL;
		}

		FIXS_CCH_Event::CloseEvent(StopEvent);
	}

	/**This method is used to stop the thread**/
	int stop(){
		isStop=true;
		if (StopEvent)
		{
			int retries=0;
			//		cout << __FUNCTION__ <<" " << __LINE__ << endl;
			while (FIXS_CCH_Event::SetEvent(StopEvent) == 0)
			{
				if( ++retries > 2) break;
				sleep(1);
			}
		}
		return 1;
	}

	/**svc method **/
	int svc(void)
	{
		cout<<"Thread AVVIATO"<<endl;
		ACS_CC_ReturnType result;
		isRunning = true;

		int events[2];
		events[0]=StopEvent;
		events[1]=implementer->getSelObj();

		while(!isStop)
		{
			int eventIndex=FIXS_CCH_Event::WaitForEvents(2,events,10000);

			if (eventIndex == 0)
			{
				FIXS_CCH_Event::ResetEvent(StopEvent);
				isStop=true;
				continue;
			}
			else if (eventIndex == -1)
			{
				printf("MyThread_IMM2 - Invalid handle: \n");
				isStop=true;
				continue;
			}
			else if (eventIndex == 1)
			{
				result = implementer->dispatch(ACS_APGCC_DISPATCH_ALL);
				FIXS_CCH_Event::ResetEvent(events[1]);
				if (result == ACS_CC_FAILURE){
//					FIXS_CCH_Event::ResetEvent(events[1]);
				}
				continue;
			}
		}
		isRunning = false;
		cout<<"Thread TERMINATO"<<endl;
		return 0;
	}
public:
	inline bool isRunningMyThread_IMM2(){return isRunning;};
private:
	MyImplementer_2 *implementer;
	bool isStop;
	bool isRunning;
	FIXS_CCH_EventHandle StopEvent;

};

namespace IMM_Util
{
	//Path APG SW packages
	const std::string APG_SW_PATH = "/sw_package";
	const std::string NBI_root = "/data/opt/ap/nbi";

	static const string NAMESPACE_CCH = "BladeSwManagement";
	static const string NAMESPACE_CS= "AxeEquipment";

	//CLASS NAME
	static const char classApzFunction[] = "AxeFunctions";

	//NEW MODEL
	static const char classBladeSwManagement[] = "BladeSwManagementBladeSwM";
	static const char classBladeSwMAsyncActionResult[] = "BladeSwManagementBladeSwMAsyncActionResult";
	static const char classApManagedSwPackage[] = "BladeSwManagementApManagedSwPackage";
	static const char classCpManagedSwPackage[] = "BladeSwManagementCpManagedSwPackage";
	static const char classReportProgress[] = "BladeSwManagementReportProgress";
	static const char classBladeSwPersistance[] = "BladeSwManagementBladeSwPersistanceValue";
	static const char classManagedSwPackageActionResult[] = "BladeSwManagementManagedSwPackageActionResult";
	static const char classBrmPersistentDataOwner[] = "BrfPersistentDataOwner";

	static const char classShelf[] = "AxeEquipmentShelf";
	static const char classOtherBlade[] = "AxeEquipmentOtherBlade";


	static const char classSwManagedBladeCategory[] = "BladeSwManagementLoadModulesHandler";
	static const char classRepository[] = "BladeSwManagementLoadModulesCategory";
	static const char classSWRepository[] = "BladeSwManagementLoadModuleInfo";
	static const char classOtherBladeFwStorage[] = "BladeSwManagementOtherBladeFwStorage";
	static const char classOtherBladeFwStorageInfoOld[] = "BladeSwManagementOtherBladeFwStorageInfoOld";
	static const char classOtherBladeFwStorageInfoNew[] = "BladeSwManagementOtherBladeFwStorageInfoNew";

	//CLASS IMPLEMENTERS
	static const char IMPLEMENTER_BLADESWM[] = "FIXS_CCH_BladeSwManagement";
	static const char IMPLEMENTER_BLADESWMSTRUCT[] = "FIXS_CCH_BladeSwStruct";
	static const char IMPLEMENTER_APSWPACKAGE[] = "FIXS_CCH_ApManagedSwPackage";
	static const char IMPLEMENTER_CPSWPACKAGE[] = "FIXS_CCH_CpManagedSwPackage";
	static const char IMPLEMENTER_REPORTPROGRESS[] = "FIXS_CCH_ReportProgress";
	static const char IMPLEMENTER_BLADESWPERCISTANCE[] = "FIXS_CCH_BladePercistanceValue";
	static const char IMPLEMENTER_BRMPERSISTENTDATAOWNER[] = "FIXS_CCH_BrmPersistentDataOwner";

	//AxeFunction CLASS
	static const char DN_APZ[] = "axeFunctionsId=1";
	static const char ATT_APZ_RDN[] = "axeFunctionsId";
	static const char ATT_APZ_SYSTEM[] = "systemType";
	static const char ATT_APZ_CBA[] = "apgShelfArchitecture";

	//FileM
	static const char DN_FILEM[] = "northBoundFoldersId=1";
	static const char ATT_FILEM_SWPACKAGE[] = "swPackage";
	static const char ATT_FILEM_SWPACKAGE_EPB1[] = "swPackageEPB1";
	static const char ATT_FILEM_SWPACKAGE_SCXB[] = "swPackageSCXB";
	static const char ATT_FILEM_SWPACKAGE_IPTB[] = "swPackageIPTB";
	static const char ATT_FILEM_SWPACKAGE_CMXB[] = "swPackageCMXB";
	static const char ATT_FILEM_SWPACKAGE_EVOET[] = "swPackageEvoET";
        static const char ATT_FILEM_SWPACKAGE_IPLB[]  = "swPackageIPLB";
	static const char ATT_FILEM_SWPACKAGE_FW[] = "swPackageFW";
	static const char ATT_FILEM_SWPACKAGE_CP[] = "swPackageCP";
	static const char ATT_FILEM_SWPACKAGE_SMXB[] = "swPackageSMXB";

	//Shelf CLASS
	static const char ATT_SHELF_RDN[] = "ShelfId";
	static const char ATT_SHELF_ADDRESS[] = "address";


	//BladeSwManagement CLASS
	static const char ATT_BLADESWM_RDN[] = "BladeSwManagementbladeSwMId";
	static const char RDN_BLADESWM[] = "BladeSwManagementbladeSwMId=1";
	static const char ATT_BLADESWM_ACTIONPROGRESS[] = "asyncActionProgress";

	//BladeSwPersistancValue CLASS
	static const char ATT_BLADESWPERSISTANCE_RDN[] = "bladeSwPersistanceValueId";
	static const char ATT_BLADESWPERSISTANCE_MAG[] = "magazine";
	static const char ATT_BLADESWPERSISTANCE_SLOT[] = "slot";
	static const char ATT_BLADESWPERSISTANCE_IPMI_STATE[] = "ipmiState";
	static const char ATT_BLADESWPERSISTANCE_IPMI_RESULT[] = "ipmiUpgradeResult";
	static const char ATT_BLADESWPERSISTANCE_IPMI_TIME[] = "ipmiUpgradeTime";
	//static const char ATT_BLADESWPERSISTANCE_PFM_UPPER_STATE[] = "pfmUpperState";
	//static const char ATT_BLADESWPERSISTANCE_PFM_LOWER_STATE[] = "pfmLowerState";
	static const char ATT_BLADESWPERSISTANCE_FAN_UPPER_STATE[] = "fanUpperState";
	static const char ATT_BLADESWPERSISTANCE_FAN_UPPER_RESULT[] = "fanUpperUpgradeResult";
	static const char ATT_BLADESWPERSISTANCE_POWER_UPPER_STATE[] = "powerUpperState";
	static const char ATT_BLADESWPERSISTANCE_POWER_UPPER_RESULT[] = "powerUpperUpgradeResult";
	static const char ATT_BLADESWPERSISTANCE_FAN_LOWER_STATE[] = "fanLowerState";
	static const char ATT_BLADESWPERSISTANCE_FAN_LOWER_RESULT[] = "fanLowerUpgradeResult";
	static const char ATT_BLADESWPERSISTANCE_POWER_LOWER_STATE[] = "powerLowerState";
	static const char ATT_BLADESWPERSISTANCE_POWER_LOWER_RESULT[] = "powerLowerUpgradeResult";
	static const char ATT_BLADESWPERSISTANCE_CURRENTCXP[] = "currentCxp";
	static const char ATT_BLADESWPERSISTANCE_EXPECTCXP[] = "expectCxp";
	static const char PARAM_BLADESWPERSISTANCE_RESET_TYPE[] = "type";
	static const char PARAM_BLADESWPERSISTANCE_FORCE[] = "force";


	//BladeSwManagement CLASS STRUCT
	static const char DN_BLADESWMACTION[] = "id=bladeSwmResultInstance,BladeSwManagementbladeSwMId=1";
	static const char RDN_BLADESWMACTION[] = "id=bladeSwmResultInstance";
	static const char ATT_BLADESWMACTION_RDN[] = "id";
	static const char ATT_BLADESWMACTION_RESULT[] = "result";
	static const char ATT_BLADESWMACTION_RESULTINFO[] = "resultInfo";
	static const char ATT_BLADESWMACTION_STATE[] = "state";
	static const char ATT_BLADESWMACTION_TIME[] = "timeActionCompleted";

	//ApManagedSwPackage and CpManagedSwPackage CLASS STRUCT
	static const char ATT_APSWPACK_RDN[] = "apManagedSwPackageId";
	static const char ATT_CPSWPACK_RDN[] = "cpManagedSwPackageId";
	static const char ATT_SWPACK_DEFAULT[] = "isDefaultPackage";
	static const char ATT_SWPACK_FAULTY[] = "packageStatus";
	static const char ATT_SWPACK_FBN[] = "functionalBoardName";
	static const char ATT_SWPACK_CXC[] = "productIdentity";
	static const char ATT_SWPACK_PATH[] = "loadModuleFileName";
	static const char ATT_SWPACK_USED_BY[] = "usedBy";
	static const char ATT_SWPACK_PROGRESS[] = "asyncActionProgress";
	static const char ATT_SWPACK_UPGRADETYPE[] = "upgradeType";
	static const char ATT_SW_VERSION_TYPE[] = "swVerType";

	//ManagedSwPackageActionResult CLASS STRUCT
	static const char ATT_SWMACTION_RDN[] = "id";
	static const char ATT_SWMACTION_RESULT[] = "result";
	static const char ATT_SWMACTION_RESULTINFO[] = "resultInfo";
	static const char ATT_SWMACTION_STATE[] = "state";
	static const char ATT_SWMACTION_TIME[] = "timeActionCompleted";


	//PowerFan CLASS
	static const char RDN_UPPER_POWERFANMODULE[] = "powerFanModuleId=UPPER";
	static const char RDN_LOWER_POWERFANMODULE[] = "powerFanModuleId=LOWER";
	static const char ATT_POWERFANMODULE_RDN[] = "powerFanModuleId";
	static const char ATT_POWERFANMODULE_MODULE[] = "moduleType";
	static const char ATT_POWERMODULE_DEVICE[] = "powerDeviceType";
	static const char ATT_FANMODULE_DEVICE[] = "fanDeviceType";
	static const char ATT_POWERMODULE_STATUS[] = "powerModuleUpgradeStatus";
	static const char ATT_FANMODULE_STATUS[] = "fanModuleUpgradeStatus";
	static const char ATT_POWERFANMODULE_POWER_REVISION[] = "runningPowerFirmwareRevision";
	static const char ATT_POWERFANMODULE_FAN_REVISION[] = "runningFanFirmwareRevision";

	//OtherBladeFwStorage
	static const char RDN_OTHERBLADEFWSTORAGE[] = "otherBladeFwStorageId";
	static const char ATT_OTHERBLADEFWSTORAGE_RDN[] = "otherBladeFwStorageId=1";
	static const char ATT_OTHERBLADEFWSTORAGE_RFW_STATUS[] = "remoteFwStatus";
	static const char ATT_OTHERBLADEFWSTORAGE_RFW_TIME[] = "lastTime";
	static const char ATT_OTHERBLADEFWSTORAGE_RFW_DATE[] = "lastDate";
	static const char ATT_OTHERBLADEFWSTORAGE_RFW_RESULT[] = "result";
	static const char ATT_OTHERBLADEFWSTORAGE_RFW_REASON[] = "reason";
	static const char ATT_OTHERBLADEFWSTORAGE_RFW_SH_FILE[] = "shPathFile";
	static const char ATT_OTHERBLADEFWSTORAGE_RFW_XML_FILE[] = "xmlPathFile";
	static const char ATT_OTHERBLADEFWSTORAGE_RFW_REVISION[] = "revState";

	//OtherBladeFwStorageInfoOld 	//OtherBladeFwStorageInfoNew
	static const char RDN_OTHERBLADEFWSTORAGEINFO_OLD[] = "otherBladeFwStorageInfoOldId";
	static const char ATT_OTHERBLADEFWSTORAGEINFO_OLD_RDN[] = "otherBladeFwStorageInfoOldId=1";
	static const char RDN_OTHERBLADEFWSTORAGEINFO_NEW[] = "otherBladeFwStorageInfoNewId";
	static const char ATT_OTHERBLADEFWSTORAGEINFO_NEW_RDN[] = "otherBladeFwStorageInfoNewId=old";
	static const char ATT_OTHERBLADEFWSTORAGEINFO_FW_TYPE[] = "fwType";
	static const char ATT_OTHERBLADEFWSTORAGEINFO_FW_VERSION[] = "fwVersion";

	//BrmParticipantContainer CLASS STRUCT
	static const char ATT_BRMPARTICIPANTCONTAINER_RDN[] = "brfParticipantContainerId";
	static const char RDN_BRMPARTICIPANTCONTAINER[] = "brfParticipantContainerId=1";

	//BrmPersistentDataOwner CLASS STRUCT
	static const char ATT_BRMPERSISTENTDATAOWNER_RDN[] = "brfPersistentDataOwnerId";
	static const char RDN_BRMPERSISTENTDATAOWNER[] = "brfPersistentDataOwnerId=ERIC-APG-FIXS-CCH";
	static const char DN_BRMPERSISTENTDATAOWNER[] = "brfPersistentDataOwnerId=ERIC-APG-FIXS-CCH,brfParticipantContainerId=1";
	static const char ATT_BRMPERSISTENTDATAOWNER_VERSION[] = "version";
	static const char ATT_BRMPERSISTENTDATAOWNER_BACKUPTYPE[] = "backupType";
	static const char ATT_BRMPERSISTENTDATAOWNER_REBOOTAFTERRESTORE[] = "rebootAfterRestore";


	//CpRelatedSwManager
	static const char DN_CPRELATEDSWMANAGER[] = "cpRelatedSwManagerId=1,BladeSwManagementbladeSwMId=1";


	enum ImportResultState {
		SUCCESS = 0,
		FAILURE = 1,
		NOT_AVAILABLE = 2
	};

	enum ImportResultInfo {
		EXECUTED 						= 0,
		INTERNAL_ERROR 					= 1,
		INVALID_PACKAGE 				= 2,
		PACKAGE_ALREADY_INSTALLED 		= 3,
		MAXIMUM_LOAD_MODULES_INSTALLED	= 4,
		DISK_QUOTA_EXCEEDED				= 5,
		BACKUP_ONGOING					= 6,
		INVALID_CONFIGURATION           = 7,
		FBN_NOT_ALLOWED                 = 8
	};

	enum ActionStateType {
		RUNNING 	= 0,
		FINISHED	= 1,
		NOT_ACTION_AVAILABLE = 2
	};

	enum NodeArchitecture {

		EGEM_SCB_RP	 = 0,
		EGEM2_SCX_SA = 1,
		DMX			 = 2
	};

	enum SystemType {

		SINGLE_CP_SYSTEM = 0,
		MULTI_CP_SYSTEM  = 1
	};

	enum PfmDevice {

		LOD		= 0,
		HOD		= 1,
		SNMP_FAILURE = 2

	};

	enum PfmModule {

		UPPER	= 1,
		LOWER	= 2
	};

	//enumeration for PFM and IPMI
	enum FwUpgradeState {

		FW_UNKNOWN	= 0,
		FW_IDLE		= 1,
		FW_ONGOING	= 2
	};

	enum FwUpgradeResult {

		FW_OK = 0,
		FW_NOT_OK = 1
	};

	enum SwUpgradeResult {

		SW_OK		= 0,
		SW_NOT_OK	= 1
	};

	enum SwUpgradeResultInfo {

		SW_EXECUTED 			= 0,
		SW_PREPARE_FAILURE		= 1,
		SW_ACTIVATE_FAILURE		= 2,
		SW_ROLLBACK_FAILURE		= 3,
		SW_CANCEL_FAILURE		= 4,
		SW_MANUALLY_CANCELLED	= 5
	};

	enum ActionSwResultInfo {

		ASW_EXECUTED 						= 0,
		ASW_HARDWARE_NOT_PRESENT			= 1,
		ASW_SNMP_FAILURE					= 2,
		ASW_LM_ALREADY_PRESENT				= 3,
		ASW_UPGRADE_ALREADY_ONGOING			= 4,
		ASW_WRONG_ACTION_IN_THIS_STATE		= 5,
		ASW_ONE_INSTALLED_PACKAGE_IS_FAULTY = 6,
		ASW_BACKUP_ONGOING					= 7,
		ASW_NO_REPLY_FROM_CS				= 8,
		ASW_NOT_ALLOWED_BY_CP				= 9,
                ASW_PACKAGE_NOT_APPLICABLE                      =10,
		ASW_IRONSIDE_FAILURE				=11,
		ASW_OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE            =12,
		ASW_BOARD_ALREADY_BLOCKED                       =13,
		ASW_INVALID_PARAM				=14
	};

	enum BoardResetType
	{
		RESET_NORMAL			= 0,
		RESET_FACTORY_DEFAULT	= 1,
		RESET_NETWORK_BOOT	= 2,
		RESET_FORCE_NORMAL	= 3,
		RESET_FORCE_NETWORK_BOOT = 4
	};

	enum BootType
	{
		FLASH_BOOT = 0,
		NETWORK_BOOT = 1
	};	

	enum BoardUpgradeType
	{
		NORMAL_UPGRADE = 0,
        INITIAL_INSTALLATION = 1
	};

	enum BladeSwManagement
	{
		IMPORT_PACKAGE = 1
	};

	//OtherBlade CLASS
	static const char ATT_OTHERBLADE_RDN[] = "otherBladeId";
	static const char ATT_OTHERBLADE_CURRSW[] = "currentLoadModuleVersion";
	static const char ATT_OTHERBLADE_ROJ[] = "bladeProductNumber";
	static const char ATT_OTHERBLADE_SLOT[] = "slotNumber";
	static const char ATT_OTHERBLADE_FBN[] = "functionalBoardName";
	static const char ATT_OTHERBLADE_IPA[] = "ipAddressEthA";
	static const char ATT_OTHERBLADE_IPB[] = "ipAddressEthB";
        static const char ATT_OTHERBLADE_SYSNUM[] ="systemNumber"; 

	//OtherBladeHardwareInfo CLASS
	static const char RDN_OTHERBLADEHARDWAREINFO[] = "otherBladeHardwareInfoId=1";
	static const char ATT_OTHERBLADEHARDWAREINFO_RDN[] = "otherBladeHardwareInfoId";
	static const char ATT_OTHERBLADEHARDWAREINFO_IPMI_STATUS[] = "ipmiUpgradeStatus";
	static const char ATT_OTHERBLADEHARDWAREINFO_IPMI_NUMBER[] = "ipmiProductNumber";
	static const char ATT_OTHERBLADEHARDWAREINFO_IPMI_REVISION[] = "ipmiProductRevision";
	static const char ATT_OTTHERBLADEHARDWAREINFO_FALLBACK_STATUS[] = "fallBackStatus";

	//LoadModulesCategory CLASS
	static const char ATT_REPOSITORY_RDN[] = "loadModulesCategoryId";
	static const char ATT_REPOSITORY_SCXB[] = "loadModulesCategoryId=SCXB";
	static const char ATT_REPOSITORY_IPTB[] = "loadModulesCategoryId=IPTB";
	static const char ATT_REPOSITORY_EPB1[] = "loadModulesCategoryId=EPB1";


	//UpgradeLoadModule CLASS
	static const char RDN_SWMANAGEDBLADE[] = "upgradeLoadModuleId=1";
	static const char ATT_SWMANAGEDBLADE_RDN[] = "upgradeLoadModuleId";

	//ReportProgress CLASS old
	static const char RDN_SWINVENTORY[] = "reportProgressId=1";
	static const char ATT_SWINVENTORY_LASTDATE[] = "lastUpgradeDate";
	static const char ATT_SWINVENTORY_LASTTIME[] = "lastUpgradeTime";
	static const char ATT_SWINVENTORY_SWCURRENT[] = "currentLoadModule";
	static const char ATT_SWINVENTORY_SWEXPECT[] = "expectLoadModule";

	//ReportProgress CLASS
	static const char ATT_SWINVENTORY_RDN[] = "reportProgressId";
	static const char ATT_SWINVENTORY_RESULT[] = "result";
	static const char ATT_SWINVENTORY_REASON[] = "resultInfo";
	static const char ATT_SWINVENTORY_STATE[] = "state";
	static const char ATT_SWINVENTORY_TIME[] = "timeUpgradeCompleted";
	static const char ATT_SWINVENTORY_ERRORCODE[] = "lastErrorCode";
	static const char ATT_SWINVENTORY_FORCEDUPGRADE[] = "forcedUpgrade";
	static const char ATT_SWINVENTORY_UPGRADETYPE[] = "upgradeType";

	enum ProgressState {

		IDLE						= 0,
		PREPARING					= 1,
		PREPARED					= 2,
		ACTIVATING					= 3,
		ACTIVATED					= 4,
		CANCELLING					= 5,
		CANCELLED					= 6,
		ROLLBACK_IN_PROGRESS	 	= 7,
		ROLLBACK_COMPLETED		 	= 8,
		UNKNOWN						= 9,
		FAILED						= 10

	};

	//LoadModuleInfo CLASS
	static const char ATT_SWREPOSITORY_RDN[] = "loadModuleInfoId";
	static const char ATT_SWREPOSITORY_DEFAULTPACKAGE[] = "isDefaultLoadModule";
	static const char ATT_SWREPOSITORY_SWUSED[] = "isUsed";
	static const char ATT_SWREPOSITORY_FORFBN[] = "functionalBoardName";
	static const char ATT_SWREPOSITORY_SWPATH[] = "loadModuleFileName";
	static const char ATT_SWREPOSITORY_CXCNUM[] = "productIdentity";
	static const char ATT_SWREPOSITORY_CXPNUM[] = "packageIdentity";

	//board type
	static const int SCXB = 340;
	static const int IPTB = 350;
	static const int EPB1 = 360;
	static const int CETB1 = 361;
	static const int IPNA = 150;
	static const int CPUB = 200;
	static const int EVOET = 370;
	static const int CMXB = 380;
	static const int IPLB = 390;
	static const int SCB_RP = 100;
	static const int SMXB = 400;

	//default attribute value
	static const char ATT_DEFAULT_STRING[] = "-";
	static const char ATT_DEFAULT_INT[] = "0";

	//METHODS
	bool getImmAttributeString (std::string object, std::string attribute, std::string &value);
	bool getImmAttributeInt(std::string object, std::string attribute, int &value);
	bool getChildrenObject(std::string dn, std::vector<std::string> & list );
	bool getClassObjectsList(std::string className, std::vector<std::string> & list );
	bool deleteImmObject(std::string dn);
	bool getObject(std::string dn, ACS_APGCC_ImmObject *object);
	void printDebugAllObjectClass (std::string className);
	void printDebugObject (std::string dn);
	bool getDnParent (std::string object_dn, std::string &parent);
	bool getRdnObject (std::string object_dn, std::string &rdn);
	bool getRdnAttribute (std::string object_dn, std::string &rdn);
	ACS_CC_ValuesDefinitionType defineAttributeString(const char* attributeName, ACS_CC_AttrValueType type, const char *value, unsigned int numValue);
	ACS_CC_ValuesDefinitionType defineAttributeInt(const char* attributeName, ACS_CC_AttrValueType type, int *value, unsigned int numValue);
	bool createImmObject(const char *classImplementer, const char *p_className, const char *parent, vector<ACS_CC_ValuesDefinitionType> AttrList);
	ACS_CC_ImmParameter defineParameterInt(const char* attributeName, ACS_CC_AttrValueType type, int *value, unsigned int numValue);
	ACS_CC_ImmParameter defineParameterString(const char* attributeName, ACS_CC_AttrValueType type, const char* value, unsigned int numValue);
	bool modify_OI_ImmAttrSTRING(const char *implementerName, const char *dn, const char *attr, const char *value);
	bool modify_OI_ImmAttrSTRING(const char *implementerName, const char *className,const char *dn, const char *attr, const char *value);
	bool modify_OI_ImmAttrINT(const char *implementerName, const char *dn, const char *attr, int value);
	bool modify_OI_ImmAttrINT(const char *implementerName, const char *className,const char *dn, const char *attr, int value);
	bool modify_OM_ImmAttr(const char *object, ACS_CC_ImmParameter parameter);
	bool createClassFwStorage();
	bool createClassFwStorageInfo();
	bool createImmObject_NO_OI(const char *p_className, const char *parent, vector<ACS_CC_ValuesDefinitionType> AttrList);
	bool getRdnValue (std::string object_dn, std::string &value);
	bool getImmMultiAttributeString (std::string object, std::string attribute, std::vector<std::string> &value);
	ACS_CC_ImmParameter defineMultiParameterString(const char* attributeName, ACS_CC_AttrValueType type, std::vector<std::string> value, unsigned int numValue);
	bool deleteImmObject(std::string dn, ACS_APGCC_ScopeT p_scope);
	std::string getErrorText(int errNo);

}; // End of namespace


#endif /* FIXS_CCH_IMM_UTIL_H_ */
