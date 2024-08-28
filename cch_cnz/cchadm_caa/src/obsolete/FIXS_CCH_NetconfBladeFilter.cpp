/* 
 * File:   FIXS_CCH_NetconfBladeFilter.cpp
 * Author: renato
 * 
 * Created on 10 ottobre 2012, 17.08
 */

#include "FIXS_CCH_NetconfBladeFilter.h"
#include <xercesc/util/PlatformUtils.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMWriter.hpp>

#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <xercesc/util/OutOfMemoryException.hpp>
#include <stdio.h>
#include <algorithm>
#include <iostream>


FIXS_CCH_NetconfBladeFilter::~FIXS_CCH_NetconfBladeFilter() {
}

FIXS_CCH_NetconfBladeFilter::FIXS_CCH_NetconfBladeFilter():
		slot(0),
		mag("")
{

}
FIXS_CCH_NetconfBladeFilter::FIXS_CCH_NetconfBladeFilter(std::string mag,int slot)
{
	this->mag=mag;
	this->slot=slot;
}

int FIXS_CCH_NetconfBladeFilter::getXml(DOMDocument* Doc,std::string &xml) {

	XMLCh* gOutputEncoding = 0;

	gOutputEncoding = XMLString::transcode("UTF-8");

	std::string xmlStr;

	try {
		// get a serializer, an instance of DOMWriter
		XMLCh tempStr[10000];
		XMLString::transcode("LS", tempStr, 10000);
		DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
		DOMWriter *theSerializer = ((DOMImplementationLS*) impl)->createDOMWriter();

		// set user specified output encoding
		theSerializer->setEncoding(gOutputEncoding);

		XMLCh* out = theSerializer->writeToString(*Doc);

		char *msg = XERCES_CPP_NAMESPACE::XMLString::transcode(out);

		xmlStr=msg;

		int found = xmlStr.find_first_of("?");
		int last = xmlStr.find_first_of("?", found + 1);

		last += 1;
		std::string t = xmlStr.replace(found - 1, last + 1, "");

		free(out);
		free(msg);

		xmlStr=t;

		delete theSerializer;

	} catch (const OutOfMemoryException&) {
		XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
	} catch (XMLException& e) {
	}

	free(gOutputEncoding);
        
        std::cout << "DBG: " << " XML Blade Query " << xmlStr.c_str() <<std::endl;

	xml=xmlStr;

	return 0;
}



int FIXS_CCH_NetconfBladeFilter::setXmlPowerStateOn(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml)
{

	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
		XMLString::release(&pMsg);
		return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;

	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

	try
	{
		DOMElement* powerState = ((DOMDocument*)doc)->createElement(Xi("pwr"));

		element->appendChild(powerState);

		DOMText* powerStateText = ((DOMDocument*)doc)->createTextNode(Xi("ON"));

		powerState->appendChild(powerStateText);

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();

	return errorCode;

}


int FIXS_CCH_NetconfBladeFilter::setXmlPowerStateOff(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml)
{

	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
				XMLString::release(&pMsg);
				return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;

	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

	try
	{
		DOMElement* powerState = ((DOMDocument*)doc)->createElement(Xi("pwr"));

		element->appendChild(powerState);


		DOMText* powerStateText = ((DOMDocument*)doc)->createTextNode(Xi("OFF"));

		powerState->appendChild(powerStateText);

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();

	return errorCode;

}

int FIXS_CCH_NetconfBladeFilter::boardblock(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string ledType,std::string &xml)
{
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
				XMLString::release(&pMsg);
				return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;
        std::string boardType = "NEW";

	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

	try
	{

		DOMElement* powerState = ((DOMDocument*)doc)->createElement(Xi("pwr"));

		element->appendChild(powerState);

		DOMText* powerStateText = ((DOMDocument*)doc)->createTextNode(Xi("OFF"));

		powerState->appendChild(powerStateText);

		DOMElement* maintenanceLed = doc->createElement(Xi("maintenanceLed"));
		element->appendChild(maintenanceLed);
                
		if (ledType.compare("NEW") == 0)
		{
			DOMElement*  ledcolor = ((DOMDocument*)doc)->createElement(Xi("color"));
                	maintenanceLed->appendChild(ledcolor);
                	DOMText* ledcolorText = ((DOMDocument*)doc)->createTextNode(Xi("BLUE"));
                	ledcolor->appendChild(ledcolorText);
                }
                else if(ledType.compare("OLD") == 0)
                {
			DOMElement*  ledcolor = ((DOMDocument*)doc)->createElement(Xi("color"));
                        maintenanceLed->appendChild(ledcolor);
                        DOMText* ledcolorText = ((DOMDocument*)doc)->createTextNode(Xi("YELLOW"));
                        ledcolor->appendChild(ledcolorText);
                }

		DOMElement* ledStatusMain = doc->createElement(Xi("status"));
		maintenanceLed->appendChild(ledStatusMain);
		DOMText* ledStatusText = ((DOMDocument*)doc)->createTextNode(Xi("ON"));
		ledStatusMain->appendChild(ledStatusText);
                 
                DOMElement* ledsupported = doc->createElement(Xi("supported"));
                maintenanceLed->appendChild(ledsupported);
                DOMText* ledsupportedText = ((DOMDocument*)doc)->createTextNode(Xi("true"));
                ledsupported->appendChild(ledsupportedText); 

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();

	return errorCode;

}


int FIXS_CCH_NetconfBladeFilter::getboardstatus(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml)
{
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
				XMLString::release(&pMsg);
				return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;

	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);


	try
	{
		DOMElement* powerState = ((DOMDocument*)doc)->createElement(Xi("pwr"));

		element->appendChild(powerState);

		DOMElement* maintenanceLed = doc->createElement(Xi("maintenanceLed"));
		element->appendChild(maintenanceLed);
		DOMElement* ledStatusMain = doc->createElement(Xi("status"));
		maintenanceLed->appendChild(ledStatusMain);

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();

	return errorCode;

}

int FIXS_CCH_NetconfBladeFilter::boarddblock(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string ledType,std::string &xml)
{
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
				XMLString::release(&pMsg);
				return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;

	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

	try
	{
		DOMElement* powerState = ((DOMDocument*)doc)->createElement(Xi("pwr"));

		element->appendChild(powerState);

		DOMText* powerStateText = ((DOMDocument*)doc)->createTextNode(Xi("ON"));

		powerState->appendChild(powerStateText);

		DOMElement* maintenanceLed = doc->createElement(Xi("maintenanceLed"));
		element->appendChild(maintenanceLed);
                  
		if (ledType.compare("NEW") == 0)
                {
                        DOMElement*  ledcolor = ((DOMDocument*)doc)->createElement(Xi("color"));
                        maintenanceLed->appendChild(ledcolor);
                        DOMText* ledcolorText = ((DOMDocument*)doc)->createTextNode(Xi("BLUE"));
                        ledcolor->appendChild(ledcolorText);
                }
                else if(ledType.compare("OLD") == 0)
                {
                        DOMElement*  ledcolor = ((DOMDocument*)doc)->createElement(Xi("color"));
                        maintenanceLed->appendChild(ledcolor);
                        DOMText* ledcolorText = ((DOMDocument*)doc)->createTextNode(Xi("YELLOW"));
                        ledcolor->appendChild(ledcolorText);
                }

                DOMElement* ledStatusMain = doc->createElement(Xi("status"));
                maintenanceLed->appendChild(ledStatusMain);
                DOMText* ledStatusText = ((DOMDocument*)doc)->createTextNode(Xi("OFF"));
                ledStatusMain->appendChild(ledStatusText);

                DOMElement* ledsupported = doc->createElement(Xi("supported"));
                maintenanceLed->appendChild(ledsupported);
                DOMText* ledsupportedText = ((DOMDocument*)doc)->createTextNode(Xi("true"));
                ledsupported->appendChild(ledsupportedText);

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();

	return errorCode;

}

int FIXS_CCH_NetconfBladeFilter::setXmlMiaLedOff(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml)
{
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
				XMLString::release(&pMsg);
				return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;

	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);
	try
	{

		DOMElement* powerState = ((DOMDocument*)doc)->createElement(Xi("pwr"));

		element->appendChild(powerState);


		DOMText* powerStateText = ((DOMDocument*)doc)->createTextNode(Xi("OFF"));

		powerState->appendChild(powerStateText);


		DOMElement* maintenanceLed = doc->createElement(Xi("maintenanceLed"));
		element->appendChild(maintenanceLed);

		DOMElement* ledStatusMain = doc->createElement(Xi("status"));
		maintenanceLed->appendChild(ledStatusMain);
		DOMText* ledStatusText = ((DOMDocument*)doc)->createTextNode(Xi("ON"));

		ledStatusMain->appendChild(ledStatusText);

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();

	return errorCode;

}


int FIXS_CCH_NetconfBladeFilter::setXmlMiaLedOn(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml)
{
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
				XMLString::release(&pMsg);
				return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;

	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);
	try
	{
		DOMElement* powerState = ((DOMDocument*)doc)->createElement(Xi("pwr"));

		element->appendChild(powerState);

		DOMText* powerStateText = ((DOMDocument*)doc)->createTextNode(Xi("ON"));

		powerState->appendChild(powerStateText);


		DOMElement* maintenanceLed = doc->createElement(Xi("maintenanceLed"));
		element->appendChild(maintenanceLed);

		DOMElement* ledStatusMain = doc->createElement(Xi("status"));
		maintenanceLed->appendChild(ledStatusMain);
		DOMText* ledStatusText = ((DOMDocument*)doc)->createTextNode(Xi("ON"));

		ledStatusMain->appendChild(ledStatusText);

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();

	return errorCode;


}


int FIXS_CCH_NetconfBladeFilter::getBladeStructureData(DOMDocument **docOut,DOMElement **element,std::string tenant,std::string /*physicAdd*/,std::string shelf_id,std::string slot_id)
{

	int errorCode = 0;

	DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(Xi("Core"));

	if (impl != NULL)
	{

		try
		{
                        //### ManagedElement #####

			(*docOut)=impl->createDocument(0,Xi("ManagedElement"),0);

			DOMElement* rootElem = (*docOut)->getDocumentElement();

			DOMElement* managedElementIdElem = (*docOut)->createElement(Xi("managedElementId"));

			DOMText* managedElementIdVal = (*docOut)->createTextNode(Xi(std::string("1").c_str()));

			managedElementIdElem->appendChild(managedElementIdVal);

			rootElem->appendChild(managedElementIdElem);
                          
                        //### Dmxc Function #####

                        DOMElement* DmxcFunctionElem = (*docOut)->createElement(Xi("DmxcFunction"));

                        rootElem->appendChild(DmxcFunctionElem);

                        DOMElement* dmxcFunctionIdElem = (*docOut)->createElement(Xi("dmxcFunctionId"));

                        DOMText* dmxcFunctionIdVal = (*docOut)->createTextNode(Xi(std::string("1").c_str()));

                        dmxcFunctionIdElem->appendChild(dmxcFunctionIdVal);

                        DmxcFunctionElem->appendChild(dmxcFunctionIdElem);   

                        
                        //#### Eqm ####

                        DOMElement* EqmElem = (*docOut)->createElement(Xi("Eqm"));

                        DmxcFunctionElem->appendChild(EqmElem);

                        DOMElement* eqmIdElem = (*docOut)->createElement(Xi("eqmId"));

                        DOMText* eqmIdVal = (*docOut)->createTextNode(Xi(std::string("1").c_str()));

                        eqmIdElem->appendChild(eqmIdVal);

                        EqmElem->appendChild(eqmIdElem);                     

                        //#### VirtualEquipment ####

                        DOMElement* VirtualEquipmentElem = (*docOut)->createElement(Xi("VirtualEquipment"));

                        EqmElem->appendChild(VirtualEquipmentElem);

                        DOMElement* virtualEquipmentIdElem = (*docOut)->createElement(Xi("virtualEquipmentId"));
                        
                        //###TODO ##Check value of VirtualEquipmentId with Actual BSP ##
                        
                        DOMText* virtualEquipmentIdVal = (*docOut)->createTextNode(Xi(tenant.c_str()));

                        virtualEquipmentIdElem->appendChild(virtualEquipmentIdVal);

                        VirtualEquipmentElem->appendChild(virtualEquipmentIdElem);
                        
                        // #### Blade ####

                        DOMElement* BladeElem =(*docOut)->createElement(Xi("Blade"));

                        VirtualEquipmentElem->appendChild(BladeElem);

                        DOMElement* BladeIdElem = (*docOut)->createElement(Xi("bladeId"));

                        if((!shelf_id.empty()) && (!slot_id.empty()))
                        {
                                std::string bladeId_str = shelf_id +std::string("-")+slot_id;
                                DOMText* BladeIdText = (*docOut)->createTextNode(Xi(bladeId_str.c_str()));
                                BladeIdElem->appendChild(BladeIdText);
                                BladeElem->appendChild(BladeIdElem);
                        }

                        *element=BladeElem; 

		} catch (const OutOfMemoryException&)
		{
			XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
			errorCode = 5;

			if(*docOut)
			{
				(*docOut)->release();
				*docOut=0;
			}


		} catch (const DOMException& e)
		{
			if(*docOut)
			{
				(*docOut)->release();
				*docOut=0;
			}


			XERCES_STD_QUALIFIER cerr << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;
			errorCode = 2;

		} catch (...)
		{

			if(*docOut)
			{
				(*docOut)->release();
				*docOut=0;
			}
			XERCES_STD_QUALIFIER cerr << "An error occurred creating the document" << XERCES_STD_QUALIFIER endl;
			errorCode = 3;
		}
	}// (inpl != NULL)
	else {
		XERCES_STD_QUALIFIER cerr << "Requested implementation is not supported" << XERCES_STD_QUALIFIER endl;
		errorCode = 4;
	}

	//XMLPlatformUtils::Terminate();
	return errorCode;

}


int FIXS_CCH_NetconfBladeFilter::setAdministrativeStateLocked(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml)
{

	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
		XMLString::release(&pMsg);
		return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;



	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

	try
	{
		DOMElement* administrativeState = ((DOMDocument*)doc)->createElement(Xi("administrativeState"));

		element->appendChild(administrativeState);

		DOMText* administrativeStateText = ((DOMDocument*)doc)->createTextNode(Xi("locked"));

		administrativeState->appendChild(administrativeStateText);

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();


	return errorCode;



}

int FIXS_CCH_NetconfBladeFilter::setAdministrativeStateUnlocked(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml)
{

	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
		XMLString::release(&pMsg);
		return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;

	getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

	try
	{
		DOMElement* administrativeState = ((DOMDocument*)doc)->createElement(Xi("administrativeState"));

		element->appendChild(administrativeState);

		DOMText* administrativeStateText =((DOMDocument*)doc)->createTextNode(Xi("unlocked"));

		administrativeState->appendChild(administrativeStateText);

		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());

		XMLString::release(&pMsg);

		return 1;
	}

	doc->release();

	XMLPlatformUtils::Terminate();

	return errorCode;
}

int FIXS_CCH_NetconfBladeFilter::resetboard(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml)
{

        try
                        {
                                XMLPlatformUtils::Initialize();
                        }
                        catch (const XMLException& toCatch)
                        {
                                char *pMsg = XMLString::transcode(toCatch.getMessage());
                                XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                                                << "  Exception message:"
                                                << pMsg;
                                XMLString::release(&pMsg);
                                return 1;
                        }

                int errorCode = 0;

                DOMDocument *doc=0;
                DOMElement *element=0;

                getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

                try
                {
                        DOMElement* resetElem = ((DOMDocument*)doc)->createElement(Xi("reset"));

                        element->appendChild(resetElem);

                        DOMElement* resetTypeElem = ((DOMDocument*)doc)->createElement(Xi("resetType"));

                        resetElem->appendChild(resetTypeElem);

                        DOMText* resetTypeText = ((DOMDocument*)doc)->createTextNode(Xi("HARD"));

                        resetTypeElem->appendChild(resetTypeText);

                        DOMElement* gracefulResetElem = ((DOMDocument*)doc)->createElement(Xi("gracefulReset"));

                        resetElem->appendChild(gracefulResetElem);

                        DOMText* gracefulResetText = ((DOMDocument*)doc)->createTextNode(Xi("FALSE"));

                        gracefulResetElem->appendChild(gracefulResetText);

                        getXml(doc,xml);


                }
                catch (const XMLException& toCatch)
                {
                        char *pMsg = XMLString::transcode(toCatch.getMessage());

                        XMLString::release(&pMsg);

                        return 1;
                }

                doc->release();

                XMLPlatformUtils::Terminate();

                return errorCode;
}
int FIXS_CCH_NetconfBladeFilter::resetboard(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string resettype,std::string gracefulreset,std::string &xml)
{

        try
                        {
                                XMLPlatformUtils::Initialize();
                        }
                        catch (const XMLException& toCatch)
                        {
                                char *pMsg = XMLString::transcode(toCatch.getMessage());
                                XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                                                << "  Exception message:"
                                                << pMsg;
                                XMLString::release(&pMsg);
                                return 1;
                        }

                int errorCode = 0;

                DOMDocument *doc=0;
                DOMElement *element=0;

                getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

                try
                {
			DOMElement* resetElem = ((DOMDocument*)doc)->createElement(Xi("reset"));

	                element->appendChild(resetElem);
                        
                        DOMElement* resetTypeElem = ((DOMDocument*)doc)->createElement(Xi("resetType"));

                        resetElem->appendChild(resetTypeElem);

                        DOMText* resetTypeText = ((DOMDocument*)doc)->createTextNode(Xi(resettype.c_str()));

                        resetTypeElem->appendChild(resetTypeText);

                        DOMElement* gracefulResetElem = ((DOMDocument*)doc)->createElement(Xi("gracefulReset"));

                        resetElem->appendChild(gracefulResetElem);

                        DOMText* gracefulResetText = ((DOMDocument*)doc)->createTextNode(Xi(gracefulreset.c_str()));

                        gracefulResetElem->appendChild(gracefulResetText);

        	        getXml(doc,xml);


                }
 		catch (const XMLException& toCatch)
                {
                        char *pMsg = XMLString::transcode(toCatch.getMessage());

                        XMLString::release(&pMsg);

                        return 1;
                }

                doc->release();

                XMLPlatformUtils::Terminate();

                return errorCode;
}

int FIXS_CCH_NetconfBladeFilter::setGprData(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string gprid,std::string gprdata,std::string &xml)
{

        try
                        {
                                XMLPlatformUtils::Initialize();
                        }
                        catch (const XMLException& toCatch)
                        {
                                char *pMsg = XMLString::transcode(toCatch.getMessage());
                                XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                                                << "  Exception message:"
                                                << pMsg;
                                XMLString::release(&pMsg);
                                return 1;
                        }

                int errorCode = 0;

                DOMDocument *doc=0;
                DOMElement *element=0;

                getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

                try
                {

			DOMElement* setGprData = ((DOMDocument*)doc)->createElement(Xi("setGprData"));

                        element->appendChild(setGprData);


			DOMElement* gprId = ((DOMDocument*)doc)->createElement(Xi("gprId"));

                        setGprData->appendChild(gprId);

                        DOMText* gprIdText =((DOMDocument*)doc)->createTextNode(Xi(gprid.c_str()));

                        gprId->appendChild(gprIdText);

		        DOMElement* gprData = ((DOMDocument*)doc)->createElement(Xi("gprData"));

                        setGprData->appendChild(gprData);
			DOMText* gprDataText = NULL;
                      	gprDataText =((DOMDocument*)doc)->createTextNode(Xi(gprdata.c_str()));

                        gprData->appendChild(gprDataText);

                        getXml(doc,xml);

                }
	        catch (const XMLException& toCatch)
                {
                        char *pMsg = XMLString::transcode(toCatch.getMessage());

                        XMLString::release(&pMsg);

                        return 1;
                }

                doc->release();

                XMLPlatformUtils::Terminate();

                return errorCode;
}


int FIXS_CCH_NetconfBladeFilter::getGprData(std::string tenant,std::string physicAdd,std::string shelf_id,std::string slot_id,std::string &xml,std::string gprId )
{

        try
                        {
                                XMLPlatformUtils::Initialize();
                        }
                        catch (const XMLException& toCatch)
                        {
                                char *pMsg = XMLString::transcode(toCatch.getMessage());
                                XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                                                << "  Exception message:"
                                                << pMsg;
                                XMLString::release(&pMsg);
                                return 1;
                        }

                int errorCode = 0;

                DOMDocument *doc=0;
                DOMElement *element=0;

                getBladeStructureData(&doc,&element,tenant,physicAdd,shelf_id,slot_id);

		 try
                {

                        DOMElement* getGprData = ((DOMDocument*)doc)->createElement(Xi("getGprData"));

                        element->appendChild(getGprData);


                        DOMElement* gprIde = ((DOMDocument*)doc)->createElement(Xi("gprId"));

                        getGprData->appendChild(gprIde);

                        DOMText* gprIdText =((DOMDocument*)doc)->createTextNode(Xi(gprId.c_str()));

                        gprIde->appendChild(gprIdText);

                        getXml(doc,xml);

                }
		 catch (const XMLException& toCatch)
                {
                        char *pMsg = XMLString::transcode(toCatch.getMessage());

                        XMLString::release(&pMsg);

                        return 1;
                }

                doc->release();

                XMLPlatformUtils::Terminate();

                return errorCode;
}

int FIXS_CCH_NetconfBladeFilter::getTenant(std::string &xml)
{
        try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
		XMLString::release(&pMsg);
		return 1;
	}

	int errorCode = 0;

	DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(Xi("Core"));

	DOMDocument* doc=0;


	if (impl != NULL)
	{

		try
		{
			//#### Managed Element ####

			doc=impl->createDocument(0,Xi("ManagedElement"),0);

			DOMElement* rootElem = doc->getDocumentElement();

			DOMElement* managedElementIdElem = doc->createElement(Xi("managedElementId"));

			DOMText* managedElementIdVal = doc->createTextNode(Xi(std::string("1").c_str()));

			managedElementIdElem->appendChild(managedElementIdVal);

			rootElem->appendChild(managedElementIdElem);

			//#### DmxcFunctions ####


			DOMElement* DmxcFunctionElem = doc->createElement(Xi("DmxcFunction"));

			rootElem->appendChild(DmxcFunctionElem);

			DOMElement* dmxcFunctionIdElem = doc->createElement(Xi("dmxcFunctionId"));

			DOMText* dmxcFunctionIdVal = doc->createTextNode(Xi(std::string("1").c_str()));

			dmxcFunctionIdElem->appendChild(dmxcFunctionIdVal);

			DmxcFunctionElem->appendChild(dmxcFunctionIdElem);

			//#### Tenants ####

			DOMElement* TenantsElem = doc->createElement(Xi("Tenant"));

			DmxcFunctionElem->appendChild(TenantsElem);

			DOMElement* tenantsIdElem = doc->createElement(Xi("tenantId"));

			TenantsElem->appendChild(tenantsIdElem);


			getXml(doc,xml);
			doc->release();


		} catch (const OutOfMemoryException&)
		{
			XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
			errorCode = 5;

			if(doc)
			{
				doc->release();
				doc=0;
			}


		} catch (const DOMException& e)
		{
			if(doc)
			{
				doc->release();
				doc=0;
			}


			XERCES_STD_QUALIFIER cerr << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;
			errorCode = 2;

		} catch (...)
		{

			if(doc)
			{
				doc->release();
				doc=0;
			}
			XERCES_STD_QUALIFIER cerr << "An error occurred creating the document" << XERCES_STD_QUALIFIER endl;
			errorCode = 3;
		}
	}// (inpl != NULL)
	else {
		XERCES_STD_QUALIFIER cerr << "Requested implementation is not supported" << XERCES_STD_QUALIFIER endl;
		errorCode = 4;
	}

	return errorCode;	
}

int FIXS_CCH_NetconfBladeFilter::getBladeUserLabel(std::string tenant, std::string shelf_id, std::string slot_id, std::string &xml)
{
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
				<< "  Exception message:"
				<< pMsg;
		XMLString::release(&pMsg);
		return 1;
	}

	int errorCode = 0;

	DOMDocument *doc=0;
	DOMElement *element=0;

	getBladeStructureData(&doc,&element,tenant,"",shelf_id,slot_id);

	try
	{
		DOMElement* userLabel = ((DOMDocument*)doc)->createElement(Xi("userLabel"));
		element->appendChild(userLabel);
		getXml(doc,xml);

	}
	catch (const XMLException& toCatch)
	{
		char *pMsg = XMLString::transcode(toCatch.getMessage());
		XMLString::release(&pMsg);
		return 1;
	}

	doc->release();
	XMLPlatformUtils::Terminate();

	return errorCode;
}

int FIXS_CCH_NetconfBladeFilter::getledType(std::string tenant, std::string shelf_id, std::string slot_id, std::string &xml)
{
        try
        {
                XMLPlatformUtils::Initialize();
        }
        catch (const XMLException& toCatch)
        {
                char *pMsg = XMLString::transcode(toCatch.getMessage());
                XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                                << "  Exception message:"
                                << pMsg;
                XMLString::release(&pMsg);
                return 1;
        }

        int errorCode = 0;

        DOMDocument *doc=0;
        DOMElement *element=0;

        getBladeStructureData(&doc,&element,tenant,"",shelf_id,slot_id);

        try
        {
                DOMElement* ledType = ((DOMDocument*)doc)->createElement(Xi("ledType"));
                element->appendChild(ledType);
                getXml(doc,xml);

        }
        catch (const XMLException& toCatch)
        {
                char *pMsg = XMLString::transcode(toCatch.getMessage());
                XMLString::release(&pMsg);
                return 1;
        }

        doc->release();
        XMLPlatformUtils::Terminate();

        return errorCode;
}

int FIXS_CCH_NetconfBladeFilter::getboardConfiguration(std::string tenant, std::string shelf_id, std::string slot_id, std::string &xml)
{
        try
        {
                XMLPlatformUtils::Initialize();
        }
        catch (const XMLException& toCatch)
        {
                char *pMsg = XMLString::transcode(toCatch.getMessage());
                XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                                << "  Exception message:"
                                << pMsg;
                XMLString::release(&pMsg);
                return 1;
        }

        int errorCode = 0;

        DOMDocument *doc=0;
        DOMElement *element=0;

        getBladeStructureData(&doc,&element,tenant,"",shelf_id,slot_id);

        try
        {
                DOMElement* boardConfigurationelem = ((DOMDocument*)doc)->createElement(Xi("boardConfiguration"));
                element->appendChild(boardConfigurationelem);
                getXml(doc,xml);

        }
        catch (const XMLException& toCatch)
        {
                char *pMsg = XMLString::transcode(toCatch.getMessage());
                XMLString::release(&pMsg);
                return 1;
        }
        printf("Msg Sent    ===  %s" ,  xml.c_str());
        doc->release();
        XMLPlatformUtils::Terminate();

        return errorCode;
}

int FIXS_CCH_NetconfBladeFilter::setboardConfiguration(std::string tenant, std::string shelf_id, std::string slot_id,std::string boardConfigStr, std::string &xml)
{
        try
        {
                XMLPlatformUtils::Initialize();
        }
        catch (const XMLException& toCatch)
        {
                char *pMsg = XMLString::transcode(toCatch.getMessage());
                XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                                << "  Exception message:"
                                << pMsg;
                XMLString::release(&pMsg);
                return 1;
        }

        int errorCode = 0;

        DOMDocument *doc=0;
        DOMElement *element=0;

        getBladeStructureData(&doc,&element,tenant,"",shelf_id,slot_id);

        try
        {
                DOMElement* boardConfigurationElem = ((DOMDocument*)doc)->createElement(Xi("boardConfiguration"));
                element->appendChild(boardConfigurationElem);
                DOMText* boardConfigurationText = ((DOMDocument*)doc)->createTextNode(Xi(boardConfigStr.c_str()));
                boardConfigurationElem->appendChild(boardConfigurationText);
                getXml(doc,xml);

        }
        catch (const XMLException& toCatch)
        {
                char *pMsg = XMLString::transcode(toCatch.getMessage());
                XMLString::release(&pMsg);
                return 1;
        }
        printf("Msg Sent    ===  %s" ,  xml.c_str());
        doc->release();
        XMLPlatformUtils::Terminate();

        return errorCode;
}

