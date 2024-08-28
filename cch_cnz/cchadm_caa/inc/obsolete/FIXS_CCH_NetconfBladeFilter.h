/* 
 * File:   FIXS_CCH_NetconfBladeFilter.h
 * Author: xgunsar
 *
 * Created on 30 Apr 2013
 */

#ifndef FIXS_CCH_NetconfBladeFilter_H
#define	FIXS_CCH_NetconfBladeFilter_H

#include "FIXS_CCH_NetconfFilter.h"

#include <string>
    
class FIXS_CCH_NetconfBladeFilter : public FIXS_CCH_NetconfFilter {
    
    
public:
	FIXS_CCH_NetconfBladeFilter();
	FIXS_CCH_NetconfBladeFilter(std::string mag,int slot);
    virtual ~FIXS_CCH_NetconfBladeFilter();

    int makeXmlgetFirmwareDataAct(std::string tenant, std::string physicalAddress,std::string slot,std::string &xml);
    int setBladeQuery(std::string tenant, std::string physicalAddress, std::string slot_id, std::string &xml);
    int setXmlGetLedInfo(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setXmlBiosRunMode(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    //int setXmlPowerState(std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setXmlFatalEventLog(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setAdministrativeStateLocked(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setAdministrativeStateUnlocked(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setReset(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setXmlPowerStateOff(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setXmlPowerStateOn(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setXmlMiaLedOn(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int setXmlMiaLedOff(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int boardblock(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string ledType,std::string &xml);	
    int boarddblock(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string ledType,std::string &xml);
    int getboardstatus(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int resetboard(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml);
    int resetboard(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string resettype,std::string gracefulreset,std::string &xml);

  
    int setGprData(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string gprid,std::string gprdata,std::string &xml);
    int getGprData(std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml,std::string gprId );

    int getTenant(std::string &xml);
    int getBladeUserLabel(std::string tenant, std::string shelf_id, std::string slot_id, std::string &xml);
    int getledType(std::string tenant, std::string shelf_id, std::string slot_id, std::string &xml);
    int getboardConfiguration(std::string tenant, std::string shelf_id, std::string slot_id, std::string &xml);
    int setboardConfiguration(std::string tenant, std::string shelf_id, std::string slot_id,std::string boardConfigStr,std::string &xml);

private:

    int getBladeStructureData(DOMDocument **docOut,DOMElement **element,std::string tenant, std::string physicAdd,std::string shelf_id,std::string slot_id);
//    int getBladeStructureData(DOMDocument **docOut,std::string physicAdd,std::string shelf_id,std::string slot_id);
//    int getBladeStructureData(std::string physicAdd,std::string shelf_id,std::string slot_id);

    int slot;
    std::string mag;
    
    int getXml(DOMDocument*d,std::string&);

};

#endif	/* FIXS_CCH_NetconfBladeFilter_H */

