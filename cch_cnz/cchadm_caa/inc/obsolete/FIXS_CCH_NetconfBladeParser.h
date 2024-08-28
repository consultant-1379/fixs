

#ifndef FIXS_CCH_NetconfBladeParser_H
#define	FIXS_CCH_NetconfBladeParser_H

#include <iostream>
#include <sstream>
#include <string>
#include "FIXS_CCH_NetconfParser.h"


#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>

using namespace xercesc;

using namespace std;

struct ipmifirmwaredata
{
	ipmifirmwaredata()
	{
		memset(type,0,10);
		memset(productNumber,0,20);
		memset(productRevisionState,0,20);
		memset(version,0,10);
		memset(runningVersion,0,10);
	};

    char type[10];
    char productNumber[20];
    char productRevisionState[20];
    char version[10];
    char runningVersion[10];
};



class FIXS_CCH_NetconfBladeParser  : public FIXS_CCH_NetconfParser{

public:
    
	FIXS_CCH_NetconfBladeParser();
    
    virtual ~FIXS_CCH_NetconfBladeParser();
    
    virtual void warning (const SAXParseException &exception);

    //	Metod to handle errors that occured during parsing
    virtual void error (const SAXParseException &exception);


    //	Metod to handle errors that occured during parsing
    virtual void fatalError (const SAXParseException &exception);

    //	Method to handle the start events that are generated for each element when
    //	an XML file is parsed
    
    virtual void characters (const XMLCh* const  chars, const unsigned int length);
    
    //	Method to handle the start events that are generated for each element when
    //	an XML file is parsed
    
    
    virtual void startElement(const XMLCh* const name,AttributeList& attributes);
    
    virtual void getResponse(nresponse &res);
    
    virtual void endElement(const XMLCh* const name);
    

private:
    
    void setOutputFirmwareData();

    void startIpmiFirmwareData(std::string);
    
    void resetFlags();
    
    
    
    bool fl_shelf;
    bool fl_shelfId;
    bool fl_Slot;
    bool fl_slotId;
    bool fl_Blade;
    bool fl_bladeId;

    bool fl_productName;
    bool fl_productNumber;
    bool fl_productRevisionState;
    bool fl_serialNumber;
    bool fl_manufacturingDate;
    bool fl_vendorName;

    bool fl_firstMacAddress;
    
    bool m_name ;
    bool m_IpmiFirmwareData ;
    bool m_rState ;
    
    ipmifirmwaredata ipm1;
    ipmifirmwaredata ipm2;
    ipmifirmwaredata ipm3;

    bool firstIpmifirmwaredata;
    bool secondIpmifirmwaredata;
    bool thirdIpmifirmwaredata;

    bool m_IpmiFirmwareDataProductNumber;
    bool m_IpmiFirmwareDataVersion;
    bool m_IpmiFirmwareDataProductRevisionState;
    bool m_IpmiFirmwareDataType;
    bool m_IpmiFirmwareDataRunningVersion;

    bool m_operationalLed;
    bool m_ledStatus;
    bool m_faultLed;
    bool m_statusLed;
    bool m_maintenanceLed;
    bool m_biosRunMode;
    bool m_powerState;
    bool m_fatalEventLog;
    bool m_getGprData;
    bool m_getGprRetVal;
    bool m_gprData;
    bool m_tenantId;
    bool m_ledType;
    bool m_boardConfiguration;
    unsigned int tenantCounter; 
};

#endif

