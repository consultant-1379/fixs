/*
 * fixs_cch_netconfbuilder.cpp
 *
 *  Created on: Jan 25, 2013
 *      Author: estevol
 */
#include <xercesc/util/PlatformUtils.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMWriter.hpp>

#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <xercesc/util/OutOfMemoryException.hpp>
#include "fixs_cch_netconfbuilder.h"
#include <iostream>
#include <sstream>
using namespace std;
fixs_cch_netconfbuilder::fixs_cch_netconfbuilder() {
	// TODO Auto-generated constructor stub

}

fixs_cch_netconfbuilder::~fixs_cch_netconfbuilder() {
	// TODO Auto-generated destructor stub
}

int fixs_cch_netconfbuilder::createShelfQuery(std::string shelfId, std::string physicalAddress,std::string &xml)
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



	// Watch for special case help request
	int errorCode = 0;

	DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(X("Core"));

	if (impl != NULL) {

		try
		{
			DOMDocument* doc =impl->createDocument(0,X("ManagedElement"),0);

		        DOMElement* rootElem = doc->getDocumentElement();
            
            		DOMElement* managedElementIdElem = doc->createElement(X("managedElementId"));

            		DOMText* managedElementIdVal = doc->createTextNode(X(std::string("1").c_str()));

            		managedElementIdElem->appendChild(managedElementIdVal);

            		rootElem->appendChild(managedElementIdElem);

            		DOMElement* equipfElem = doc->createElement(X("Equipment"));

            		rootElem->appendChild(equipfElem);

            		DOMElement* equipmentIdElem = doc->createElement(X("equipmentId"));

            		DOMText* equipmentIdVal = doc->createTextNode(X(std::string("1").c_str()));

            		equipmentIdElem->appendChild(equipmentIdVal);

            		equipfElem->appendChild(equipmentIdElem);

            		DOMElement* shelf = doc->createElement(X("Shelf"));
            
            		equipfElem->appendChild(shelf);

            		DOMElement* shelfId_tag = doc->createElement(X("shelfId"));

            		if(!shelfId.empty())
            		{
                    		DOMText* shelfIdText = doc->createTextNode(X(shelfId.c_str()));
                    		shelfId_tag->appendChild(shelfIdText);
                    
            		}

            		shelf->appendChild(shelfId_tag);

            		DOMElement* physicalAddress_tag = doc->createElement(X("physicalAddress"));
            
            		if(!physicalAddress.empty())
			{

                    	DOMText* physicalAddressText = doc->createTextNode(X(physicalAddress.c_str()));
                    	physicalAddress_tag->appendChild(physicalAddressText);
                   
            		}
            		shelf->appendChild(physicalAddress_tag);
			getXml(doc,xml);

			doc->release();
		} catch (const OutOfMemoryException&) {
			XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
			errorCode = 5;
		} catch (const DOMException& e) {
			XERCES_STD_QUALIFIER cerr << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;
			errorCode = 2;
		} catch (...) {
			XERCES_STD_QUALIFIER cerr << "An error occurred creating the document" << XERCES_STD_QUALIFIER endl;
			errorCode = 3;
		}
	}// (inpl != NULL)
	else {
		XERCES_STD_QUALIFIER cerr << "Requested implementation is not supported" << XERCES_STD_QUALIFIER endl;
		errorCode = 4;
	}

	XMLPlatformUtils::Terminate();
	return errorCode;
}
int fixs_cch_netconfbuilder::createTenantQuery(std::string &xml)
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

        DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(X("Core"));

        DOMDocument* doc=0;


        if (impl != NULL)
        {

                try
                {
                        //#### Managed Element ####

                        doc=impl->createDocument(0,X("ManagedElement"),0);

                        DOMElement* rootElem = doc->getDocumentElement();

                        DOMElement* managedElementIdElem = doc->createElement(X("managedElementId"));

                        DOMText* managedElementIdVal = doc->createTextNode(X(std::string("1").c_str()));

                        managedElementIdElem->appendChild(managedElementIdVal);

                        rootElem->appendChild(managedElementIdElem);

                        //#### DmxcFunctions ####


                        DOMElement* DmxcFunctionElem = doc->createElement(X("DmxcFunction"));
			 rootElem->appendChild(DmxcFunctionElem);

                        DOMElement* dmxcFunctionIdElem = doc->createElement(X("dmxcFunctionId"));

                        DOMText* dmxcFunctionIdVal = doc->createTextNode(X(std::string("1").c_str()));

                        dmxcFunctionIdElem->appendChild(dmxcFunctionIdVal);

                        DmxcFunctionElem->appendChild(dmxcFunctionIdElem);

                        //#### Tenants ####

                        DOMElement* TenantsElem = doc->createElement(X("Tenant"));

                        DmxcFunctionElem->appendChild(TenantsElem);

                        DOMElement* tenantsIdElem = doc->createElement(X("tenantId"));

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


int fixs_cch_netconfbuilder::createBladeQuery(std::string tenant, std::string &xml)
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

        DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(X("Core"));

        DOMDocument* doc=0;


        if (impl != NULL)
        {

                try
                {
                        //#### Managed Element ####

                        doc=impl->createDocument(0,X("ManagedElement"),0);

                        DOMElement* rootElem = doc->getDocumentElement();

                        DOMElement* managedElementIdElem = doc->createElement(X("managedElementId"));

                        DOMText* managedElementIdVal = doc->createTextNode(X(std::string("1").c_str()));

                        managedElementIdElem->appendChild(managedElementIdVal);

                        rootElem->appendChild(managedElementIdElem);

                        //#### DmxcFunctions ####


                        DOMElement* DmxcFunctionElem = doc->createElement(X("DmxcFunction"));
			 rootElem->appendChild(DmxcFunctionElem);

                        DOMElement* dmxcFunctionIdElem = doc->createElement(X("dmxcFunctionId"));

                        DOMText* dmxcFunctionIdVal = doc->createTextNode(X(std::string("1").c_str()));

                        dmxcFunctionIdElem->appendChild(dmxcFunctionIdVal);

                        DmxcFunctionElem->appendChild(dmxcFunctionIdElem);
		
			//#######EQM######
			DOMElement* eqmElem = doc->createElement(X("Eqm"));
			
			DmxcFunctionElem->appendChild(eqmElem);


			DOMElement* eqmIdElem = doc->createElement(X("eqmId"));
                        DOMText* eqmIdVal = doc->createTextNode(X(std::string("1").c_str()));

                        eqmIdElem->appendChild(eqmIdVal);

                        eqmElem->appendChild(eqmIdElem);

			DmxcFunctionElem->appendChild(eqmElem);	
			  //#### VirtualEquipment ####

                        DOMElement* VirtualEquipElem = doc->createElement(X("VirtualEquipment"));

                        eqmElem->appendChild(VirtualEquipElem);

                        DOMElement* virtualEquipIdElem = doc->createElement(X("virtualEquipmentId"));


                        DOMText* virtualEquipIdVal = doc->createTextNode(X(tenant.c_str()));//Todo tenant

                        virtualEquipIdElem->appendChild(virtualEquipIdVal);

                        VirtualEquipElem->appendChild(virtualEquipIdElem);


                        DOMElement* bladeElem = doc->createElement(X("Blade"));
                        VirtualEquipElem->appendChild(bladeElem);

                        DOMElement* bladeId_tag = doc->createElement(X("bladeId"));

                        bladeElem->appendChild(bladeId_tag);

			
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
int fixs_cch_netconfbuilder::createBridgeQuery(std::string shelfId, std::string slotNumber, std::string tenant, std::string &xml)
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

	std::string bridgeId = shelfId + "-" + slotNumber;


	// Watch for special case help request
	int errorCode = 0;

	DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(X("Core"));

	if (impl != NULL) {

		try
		{
		
			DOMDocument* doc =impl->createDocument(0,X("ManagedElement"),0);

               		DOMElement* rootElem = doc->getDocumentElement();
            
            		DOMElement* managedElementIdElem = doc->createElement(X("managedElementId"));

            		DOMText* managedElementIdVal = doc->createTextNode(X(std::string("1").c_str()));

            		managedElementIdElem->appendChild(managedElementIdVal);

            		rootElem->appendChild(managedElementIdElem);
		
			DOMElement* dmxFuncElem = doc->createElement(X("DmxcFunction"));

            		rootElem->appendChild(dmxFuncElem);

            		DOMElement* dmxFuncIdElem = doc->createElement(X("dmxcFunctionId"));

            		DOMText* dmxFunctionIdVal = doc->createTextNode(X(std::string("1").c_str()));

            		dmxFuncIdElem->appendChild(dmxFunctionIdVal);

            		dmxFuncElem->appendChild(dmxFuncIdElem);

			DOMElement* trmElem = doc->createElement(X("Trm"));

                        dmxFuncElem->appendChild(trmElem);

                        DOMElement* trmIdElem = doc->createElement(X("trmId"));

                        DOMText* trmIdVal = doc->createTextNode(X(std::string("1").c_str()));

                        trmIdElem->appendChild(trmIdVal);

                        trmElem->appendChild(trmIdElem);
			
			 //#### VirtualBridge ####

                        DOMElement* VirtualBridgeElem = doc->createElement(X("VirtualBridge"));

                        trmElem->appendChild(VirtualBridgeElem);

                        DOMElement* virtualBridgeIdElem = doc->createElement(X("virtualBridgeId"));

                        //###TODO ##Check value of VirtualEquipmentId with Actual BSP ##

                        DOMText* virtualBridgeIdVal = doc->createTextNode(X(tenant.c_str()));//Todo tenant

                        virtualBridgeIdElem->appendChild(virtualBridgeIdVal);

                        VirtualBridgeElem->appendChild(virtualBridgeIdElem);


			DOMElement* bridgePortElem = doc->createElement(X("BridgePort"));
			VirtualBridgeElem->appendChild(bridgePortElem);

			DOMElement* bridgePortId_tag = doc->createElement(X("bridgePortId"));

			bridgePortElem->appendChild(bridgePortId_tag);
			

			getXml(doc,xml);


			doc->release();
		} catch (const OutOfMemoryException&) {
			XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
			errorCode = 5;
		} catch (const DOMException& e) {
			XERCES_STD_QUALIFIER cerr << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;
			errorCode = 2;
		} catch (...) {
			XERCES_STD_QUALIFIER cerr << "An error occurred creating the document" << XERCES_STD_QUALIFIER endl;
			errorCode = 3;
		}
	}// (inpl != NULL)
	else {
		XERCES_STD_QUALIFIER cerr << "Requested implementation is not supported" << XERCES_STD_QUALIFIER endl;
		errorCode = 4;
	}

	XMLPlatformUtils::Terminate();
	return errorCode;
}

int fixs_cch_netconfbuilder::createIfQuery(std::string shelfId, std::string slotNumber,std::string tenant, std::string interfaceId, std::string &xml)
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

	std::string bridgeId = shelfId + "-" + slotNumber;

	std::string PortId =  bridgeId + ":" +interfaceId;
	// Watch for special case help request
	int errorCode = 0;

	DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(X("Core"));

	if (impl != NULL) {

		try
		{
			
	            	DOMDocument* doc =impl->createDocument(0,X("ManagedElement"),0);

            		DOMElement* rootElem = doc->getDocumentElement();
            
            		DOMElement* managedElementIdElem = doc->createElement(X("managedElementId"));

            		DOMText* managedElementIdVal = doc->createTextNode(X(std::string("1").c_str()));

            		managedElementIdElem->appendChild(managedElementIdVal);

            		rootElem->appendChild(managedElementIdElem);

			 DOMElement* dmxFuncElem = doc->createElement(X("DmxcFunction"));

                        rootElem->appendChild(dmxFuncElem);

                        DOMElement* dmxFuncIdElem = doc->createElement(X("dmxcFunctionId"));

                        DOMText* dmxFunctionIdVal = doc->createTextNode(X(std::string("1").c_str()));

                        dmxFuncIdElem->appendChild(dmxFunctionIdVal);

                        dmxFuncElem->appendChild(dmxFuncIdElem);

                        DOMElement* trmElem = doc->createElement(X("Trm"));

                        dmxFuncElem->appendChild(trmElem);

                        DOMElement* trmIdElem = doc->createElement(X("trmId"));

                        DOMText* trmIdVal = doc->createTextNode(X(std::string("1").c_str()));

                        trmIdElem->appendChild(trmIdVal);

                        trmElem->appendChild(trmIdElem);

                         //#### VirtualBridge ####

                        DOMElement* VirtualBridgeElem = doc->createElement(X("VirtualBridge"));

                        trmElem->appendChild(VirtualBridgeElem);

                        DOMElement* virtualBridgeIdElem = doc->createElement(X("virtualBridgeId"));

                        //###TODO ##Check value of VirtualEquipmentId with Actual BSP ##

                        DOMText* virtualBridgeIdVal = doc->createTextNode(X(tenant.c_str()));//Todo tenant

                        virtualBridgeIdElem->appendChild(virtualBridgeIdVal);

                        VirtualBridgeElem->appendChild(virtualBridgeIdElem);


                        DOMElement* bridgePortElem = doc->createElement(X("BridgePort"));
                        VirtualBridgeElem->appendChild(bridgePortElem);

			DOMElement* bridgePortId = doc->createElement(X("bridgePortId"));

			DOMText* bridgePortIdVal = doc->createTextNode(X(PortId.c_str()));

                        bridgePortId->appendChild(bridgePortIdVal);

                        bridgePortElem->appendChild(bridgePortId);


			DOMElement* operState_tag = doc->createElement(X("operState"));
			DOMElement* inBroadcast_tag = doc->createElement(X("inBroadcast"));
			DOMElement* inDiscards_tag = doc->createElement(X("inDiscards"));
			DOMElement* inErrors_tag = doc->createElement(X("inErrors"));
			DOMElement* inMulticast_tag = doc->createElement(X("inMulticast"));
			DOMElement* inOctets_tag = doc->createElement(X("inOctets"));
			DOMElement* inUnicast_tag = doc->createElement(X("inUnicast"));
			DOMElement* outBroadcast_tag = doc->createElement(X("outBroadcast"));
			DOMElement* outDiscards_tag = doc->createElement(X("outDiscards"));
			DOMElement* outErrors_tag = doc->createElement(X("outErrors"));
			DOMElement* outMulticast_tag = doc->createElement(X("outMulticast"));
			DOMElement* outOctets_tag = doc->createElement(X("outOctets"));
			DOMElement* outUnicast_tag = doc->createElement(X("outUnicast"));

			bridgePortElem->appendChild(operState_tag);
			bridgePortElem->appendChild(inOctets_tag);
			bridgePortElem->appendChild(inUnicast_tag);
			bridgePortElem->appendChild(inDiscards_tag);
			bridgePortElem->appendChild(inErrors_tag);
			bridgePortElem->appendChild(inMulticast_tag);
			bridgePortElem->appendChild(inBroadcast_tag);
			bridgePortElem->appendChild(outOctets_tag);
			bridgePortElem->appendChild(outUnicast_tag);
			bridgePortElem->appendChild(outDiscards_tag);
			bridgePortElem->appendChild(outErrors_tag);
			bridgePortElem->appendChild(outMulticast_tag);
			bridgePortElem->appendChild(outBroadcast_tag);

			getXml(doc,xml);
			doc->release();
		} catch (const OutOfMemoryException&) {
			XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
			errorCode = 5;
		} catch (const DOMException& e) {
			XERCES_STD_QUALIFIER cerr << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;
			errorCode = 2;
		} catch (...) {
			XERCES_STD_QUALIFIER cerr << "An error occurred creating the document" << XERCES_STD_QUALIFIER endl;
			errorCode = 3;
		}
	}// (inpl != NULL)
	else {
		XERCES_STD_QUALIFIER cerr << "Requested implementation is not supported" << XERCES_STD_QUALIFIER endl;
		errorCode = 4;
	}

	XMLPlatformUtils::Terminate();
	return errorCode;
}

int fixs_cch_netconfbuilder::getXml(DOMDocument* Doc,std::string &xml) {

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
//        printf("%s\n %d %d\n", t.c_str(), found - 1, last + 1);

        free(out);
        free(msg);

        xmlStr=t;


        delete theSerializer;
    //    delete impl;

    } catch (const OutOfMemoryException&) {
        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
    } catch (XMLException& e) {
    }

    free(gOutputEncoding);

    xml=xmlStr;
    return 0;
}
