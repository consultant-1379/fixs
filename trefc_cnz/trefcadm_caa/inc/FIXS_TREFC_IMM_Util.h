/*
 * FIXS_TREFC_IMM_Util.h
 *
 *  Created on: Oct 5, 2011
 *      Author: eanform
 */

#ifndef FIXS_TREFC_IMM_UTIL_H_
#define FIXS_TREFC_IMM_UTIL_H_

#include "MyImplementer.h"

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include <ace/Task.h>

#include "ACS_APGCC_OiHandler.h"
#include "ACS_CC_Types.h"
#include "ACS_APGCC_Util.H"
#include "acs_apgcc_omhandler.h"


//class MyImplementer;


class MyThread_IMM : public ACE_Task_Base {

	public:

		MyThread_IMM(MyImplementer *p_user1, ACE_Reactor *p_reactor ){

			implementer = p_user1;
			reactor = p_reactor;

		}

		virtual ~MyThread_IMM(){}

		int svc(void){

			implementer->dispatch(reactor, ACS_APGCC_DISPATCH_ALL);

			reactor->run_reactor_event_loop();

			return 0;
		}
	private:

		MyImplementer *implementer;
		ACE_Reactor *reactor;
};


namespace IMM_Util
{
	//Path APG SW packages
	const std::string APG_SW_PATH = "/sw_package";
	const std::string NBI_root = "/data/opt/ap/nbi";

	//CLASS NAME
	//static const char classApBlade[] = "AxeEquipmentApBlade";
	static const char classTimeReferenceConfiguration[] = "AxeTimeReferenceTimeReferenceM";
	static const char classTimeServer[] = "AxeTimeReferenceNetworkTimeServer";
	static const char classScxTimeService[] = "AxeTimeReferenceScxTimeService";
	static const char classScxTimeReference[] = "AxeTimeReferenceScxTimeReference";
//	static const char classGpsDevice[] = "GpsDevice";
	static const char classNetworkConfiguration[] = "AxeTimeReferenceSCXNetworkConfigurationStruct";
	static const char classNetworkConfigurationSwitch[] = "AxeTimeReferenceSwitchNetworkConfigStruct";

	static const char classSwitchTimeService[] = "AxeTimeReferenceSwitchTimeService";
	static const char classSwitchTimeReference[] = "AxeTimeReferenceSwitchTimeReference";
//	static const char classGpsDevice[] = "GpsDevice";
//	static const char classNetworkConfiguration[] = "AxeTimeReferenceSwitchNetworkConfigurationStruct";

	//static const char classShelf[] = "AxeEquipmentShelf";
	static const char classOtherBlade[] = "AxeEquipmentOtherBlade";


	//CLASS IMPLEMENTERS
	static const char IMPLEMENTER_TIMESERVICE[] = "FIXS_TREFC_TimeService";
	static const char IMPLEMENTER_TIMESERVER[] = "FIXS_TREFC_TimeServer";
	static const char IMPLEMENTER_SCXTIMESERVICE[] = "FIXS_TREFC_ScxTimeService";
	static const char IMPLEMENTER_SCXTIMEREFERENCE[] = "FIXS_TREFC_ScxTimeReference";
	static const char IMPLEMENTER_NETWORKCONFIGURATION[] = "FIXS_TREFC_NetworkConfiguration";
	static const char IMPLEMENTER_NETWORKCONFIGURATION_SWITCH[] = "FIXS_TREFC_NetworkConfigurationSwitch";
	static const char IMPLEMENTER_SWITCHTIMESERVICE[] = "FIXS_TREFC_SwitchTimeService";
	static const char IMPLEMENTER_SWITCHTIMEREFERENCE[] = "FIXS_TREFC_SwitchTimeReference";

	//AP Blade class
	static const char ATT_APBLADE_SYSNUM[] = "systemNumber";


	//AxeFunction CLASS
	static const char DN_APZ[] = "axeFunctionsId=1";
	static const char ATT_APZ_RDN[] = "axeFunctionsId";
	static const char ATT_APZ_SYSTEM[] = "systemType";
	static const char ATT_APZ_CBA[] = "apgShelfArchitecture";

	//FileM
	static const char DN_FILEM[] = "northBoundFoldersId=1";
	static const char ATT_FILEM_SWPACKAGE[] = "swPackage";

	//Shelf CLASS
	static const char ATT_SHELF_RDN[] = "shelfId";
	//static const char ATT_SHELF_ADDRESS[] = "address";

	//AxetimeReference Class
	static const char ATT_TIMEREFERENCE_OBJ[] = "AxeTimeReferencetimeReferenceMId=1";
	static const char ATT_TIMEREFERENCE_RDN[] = "AxeTimeReferencetimeReferenceMId";
	static const char ATT_TIMEREFERENCE_STATUS[] = "operationalState";
	static const char ATT_TIMEREFERENCE_ADMIN_STATE[] = "administrativeState";
	static const char ATT_TIMEREFERENCE_RESULT_STATE[] = "changeAdmStateResult";
	static const char ATT_TIMEREFERENCE_NWTIMESRV_NAME[] = "NetworkTimeServer";
	static const char ATT_TIMEREFERENCE_NWTIMESRV_IP[] = "ipAddress";

	//ScxTimeService Class
	static const char ATT_SCXTIMESERVICE_RDN[] = "scxTimeServiceId";
	static const char ATT_SCXTIMESERVICE_OBJ[] = "scxTimeServiceId=1";
	static const char ATT_SCXTIMESERVICE_DN[] = "scxTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	static const char ATT_SCXTIMESERVICE_TYPE[] = "serviceType";
	static const char ATT_SCXTIMESERVICE_REF_TYPE[] = "referenceType";

	//ScxTimeReference Class
	static const char ATT_SCXTIMEREFERENCE_RDN[] = "scxTimeReferenceId";
	static const char ATT_SCXTIMEREFERENCE_SCXA_RDN[] = "scxTimeReferenceId=SCXA";
	static const char ATT_SCXTIMEREFERENCE_SCXA_OBJ[] = "scxTimeReferenceId=SCXA,scxTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	static const char ATT_SCXTIMEREFERENCE_SCXB_RDN[] = "scxTimeReferenceId=SCXB";
	static const char ATT_SCXTIMEREFERENCE_SCXB_OBJ[] = "scxTimeReferenceId=SCXB,scxTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	static const char ATT_SCXTIMEREFERENCE_SCXA_NW_OBJ[] = "networkConfigurationId_0,scxTimeReferenceId=SCXA,scxTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	static const char ATT_SCXTIMEREFERENCE_STATUS[] = "timeServiceStatus";
	static const char ATT_SCXTIMEREFERENCE_REF_ID[] = "timeReferenceId";
	static const char ATT_SCXTIMEREFERENCE_REF_STRUCT[] = "networkConfiguration";
	static const char SCXTIMEREFERENCE_SCXA[] = "SCXA";
	static const char SCXTIMEREFERENCE_SCXB[] = "SCXB";
	//@S
	static const char ATT_SCXTIMEREFERENCE_NW_CONF_ID[] = "id";
	static const char ATT_SCXTIMEREFERENCE_MASK[]="netmask";
	static const char ATT_SCXTIMEREFERENCE_GATEWAY[]="gateway";
	static const char ATT_SCXTIMEREFERENCE_IP[] = "ipAddress";
	static const char ATT_SCXTIMEREFERENCE_SCX_NW_OBJA[] = "id=networkConfiguration_0,scxTimeReferenceId=SCXA,scxTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	static const char ATT_SCXTIMEREFERENCE_SCX_NW_OBJB[] = "id=networkConfiguration_0,scxTimeReferenceId=SCXB,scxTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	//@E

	static const char ATT_SWITCHTIMESERVICE_RDN[] = "switchTimeServiceId";
    static const char ATT_SWITCHTIMESERVICE_OBJ[] = "switchTimeServiceId=1";
    static const char ATT_SWITCHTIMESERVICE_DN[] = "switchTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	static const char ATT_SWITCHTIMESERVICE_TYPE[] = "serviceType";
    static const char ATT_SWITCHTIMESERVICE_REF_TYPE[] = "referenceType";
	
	static const char ATT_SWITCHTIMEREFERENCE_RDN[] = "switchTimeReferenceId";
    static const char ATT_SWITCHTIMEREFERENCE_SWITCHA_RDN[] = "switchTimeReferenceId=SWITCHA";
    static const char ATT_SWITCHTIMEREFERENCE_SWITCHA_OBJ[] = "switchTimeReferenceId=SWITCHA,switchTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
    static const char ATT_SWITCHTIMEREFERENCE_SWITCHB_RDN[] = "switchTimeReferenceId=SWITCHB";
    static const char ATT_SWITCHTIMEREFERENCE_SWITCHB_OBJ[] = "switchTimeReferenceId=SWITCHB,switchTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	static const char SWITCHTIMEREFERENCE_SWITCHA[] = "SWITCHA";
    static const char SWITCHTIMEREFERENCE_SWITCHB[] = "SWITCHB";
	static const char ATT_SWITCHTIMEREFERENCE_STATUS[] = "timeServiceStatus";
	static const char ATT_SWITCHTIMEREFERENCE_REF_ID[] = "timeReferenceId";
	//@S
	static const char ATT_SWITCHTIMEREFERENCE_MASK[]="netmask";
	static const char ATT_SWITCHTIMEREFERENCE_GATEWAY[]="gateway";
	static const char ATT_SWITCHTIMEREFERENCE_IP[] = "ipAddress";
	static const char ATT_SWITCHTIMEREFERENCE_SWITCHA_NW_OBJ[] ="id=networkConfiguration_0,switchTimeReferenceId=SWITCHA,switchTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	static const char ATT_SWITCHTIMEREFERENCE_SWITCHB_NW_OBJ[] ="id=networkConfiguration_0,switchTimeReferenceId=SWITCHB,switchTimeServiceId=1,AxeTimeReferencetimeReferenceMId=1";
	//@E

	//NetworkTimeServer Class
	static const char ATT_TIMESERVER_RDN[] = "networkTimeServerId";
//	static const char ATT_TIMESERVER_RDN[] = "timeServerId";
	static const char ATT_TIMESERVER_IP[] = "ipAddress";
	//	static const char ATT_TIMESERVER_NAME[] = "name";

	//SCXNetworkConfigurationStruct Class
	static const char ATT_NETWORKCONFIGURATION_RDN[] = "id";
	static const char ATT_NETWORKCONFIGURATION_OBJ[] = "id=networkConfiguration_0";
	
	static const char ATT_NETWORKCONFIGURATION_NETMASK[] = "netmask";
	static const char ATT_NETWORKCONFIGURATION_IP[] = "ipAddress";
	static const char ATT_NETWORKCONFIGURATION_GATEWAY[] = "gateway";

//	//GpsDevice Class
//	static const char ATT_GPSDEVICE_RDN[] = "gpsDeviceId";
//	static const char ATT_GPSDEVICE_SCXA_STATUS[] = "scxAstatus";
//	static const char ATT_GPSDEVICE_SCXB_STATUS[] = "scxBstatus";


//	static const char ATT_NETWORKCONFIGURATION_NETMASK_B[] = "netmaskScxB";
//	static const char ATT_NETWORKCONFIGURATION_IP_B[] = "ipAddressScxB";
//	static const char ATT_NETWORKCONFIGURATION_GATEWAY_B[] = "defaultGatewayScxB";


	enum ExternalReferenceType{

		NOT_CONFIGURED		 = 0,
		NETWORK_REFERENCE 	 = 1,
		GPS_REFERENCE		 = 2
	};

	enum NodeArchitecture {

		EGEM_SCB_RP	 = 0,
		EGEM2_SCX_SA = 1,
		EGEM2_SMX_SA = 4
	};

	enum ApName {

		AP1	 = 1,
		AP2  = 2
	};

	enum SystemType {

		SINGLE_CP_SYSTEM = 0,
		MULTI_CP_SYSTEM  = 1
	};

	enum TimeReferenceStatus {

		NOT_CONNECTED	= 0,
		CONNECTED		= 1,
		RESYNCH			= 2
	};

	enum TimeServiceStatus {

		DISABLED	= 0,
		ENABLED		= 1,
		BUSY		= 2
	};

	enum TimeServiceAdm {

		LOCKED		= 0,
		UNLOCKED	= 1
	};

	enum TimeServiceType {

		NONE	= 0,
		NTP	= 1,
		NTPHP	= 2
	};


	enum Action {
		UNDEFINED = 0,
		CREATE	= 1,
		DELETE	= 2,
		MODIFY	= 3
	};

	enum ChangeAdmStateResultType {

		EXECUTED 		  			= 0,
		ADM_NOT_ALLOWED_BY_CP 		= 1,
		ADM_REF_NOT_CONFIGURED 		= 2,
		ADM_SERV_NOT_CONFIGURED 	= 3,
		ADM_NO_REF_EXIST			= 4,
		ADM_INVALID_REF				= 5,
		ADM_SNMP_FAILURE			= 6,
		ADM_INTERNAL_ERROR			= 7
	};

	//OtherBlade CLASS
	static const char ATT_OTHERBLADE_RDN[] = "otherBladeId";
	static const char ATT_OTHERBLADE_SLOT[] = "slotNumber";
	static const char ATT_OTHERBLADE_FBN[] = "functionalBoardName";
	static const char ATT_OTHERBLADE_IPA[] = "ipAddressEthA";
	static const char ATT_OTHERBLADE_IPB[] = "ipAddressEthB";
	static const char ATT_OTHERBLADE_APTETHPORT[] = "aptEthInPort";


	//board type
	static const int SCXB = 340;
	static const int IPTB = 350;
	static const int EPB1 = 360;
	static const int CETB1 = 361;

	//default attribute value
	static const char ATT_DEFAULT_STRING[] = "-";
	static const char ATT_DEFAULT_INT[] = "0";
        static int errorCode =0;

	//METHODS
	bool getImmAttributeString (std::string object, std::string attribute, std::string &value);
	bool getImmAttributeInt(std::string object, std::string attribute, int &value);
	bool getChildrenObject(std::string dn, std::vector<std::string> & list );
	bool getClassObjectsList(std::string className, std::vector<std::string> & list );
	bool deleteImmObject(std::string dn);
	bool deleteImmObject(std::string dn, ACS_APGCC_ScopeT p_scope);
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
	bool modify_OI_ImmAttrINT(const char *implementerName, const char *dn, const char *attr, int value);
	bool modify_OM_ImmAttr(const char *object, ACS_CC_ImmParameter parameter);
	string getIdValueFromRdn(const string & rdn);
	bool createImmObject_NO_OI(const char *p_className, const char *parent, vector<ACS_CC_ValuesDefinitionType> AttrList);
        int getError();
}; // End of namespace


#endif /* FIXS_TREFC_IMM_UTIL_H_ */
