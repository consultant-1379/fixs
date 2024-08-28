

#include "FIXS_CCH_NetconfShelfFilter.h"
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
#include <string.h>
#include <algorithm>



FIXS_CCH_NetconfShelfFilter::~FIXS_CCH_NetconfShelfFilter() {
}

FIXS_CCH_NetconfShelfFilter::FIXS_CCH_NetconfShelfFilter()
{
    
}



int FIXS_CCH_NetconfShelfFilter::createShelfQuery(std::string shelfId,std::string userLabel,std::string rack,std::string shelfType,std::string position,std::string physicalAddress,std::string &xml)
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

    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(Xi("Core"));

    if (impl != NULL) {
        
        try 
        {

            DOMDocument* doc =impl->createDocument(0,Xi("ManagedElement"),0);

            DOMElement* rootElem = doc->getDocumentElement();
            
            DOMElement* managedElementIdElem = doc->createElement(Xi("managedElementId"));

            DOMText* managedElementIdVal = doc->createTextNode(Xi(std::string("1").c_str()));

            managedElementIdElem->appendChild(managedElementIdVal);

            rootElem->appendChild(managedElementIdElem);

            DOMElement* equipfElem = doc->createElement(Xi("Equipment"));

            rootElem->appendChild(equipfElem);

            DOMElement* equipmentIdElem = doc->createElement(Xi("equipmentId"));

            DOMText* equipmentIdVal = doc->createTextNode(Xi(std::string("1").c_str()));

            equipmentIdElem->appendChild(equipmentIdVal);

            equipfElem->appendChild(equipmentIdElem);

            DOMElement* shelf = doc->createElement(Xi("Shelf"));
            
            equipfElem->appendChild(shelf);

            DOMElement* shelfId_tag = doc->createElement(Xi("shelfId"));

            if(!shelfId.empty())
            {
                    DOMText* shelfIdText = doc->createTextNode(Xi(shelfId.c_str()));
                    shelfId_tag->appendChild(shelfIdText);
                    
            }

            shelf->appendChild(shelfId_tag);

            DOMElement* physicalAddress_tag = doc->createElement(Xi("physicalAddress"));
            
            if(!physicalAddress.empty()){

                    DOMText* physicalAddressText = doc->createTextNode(Xi(physicalAddress.c_str()));
                    physicalAddress_tag->appendChild(physicalAddressText);
                   
            }
            shelf->appendChild(physicalAddress_tag);

            DOMElement* userLabel_tag = 0;
            
            if(!userLabel.empty())
            {
                    userLabel_tag=doc->createElement(Xi("userLabel"));
                    DOMText* userLabelText = doc->createTextNode(Xi(userLabel.c_str()));
                    userLabel_tag->appendChild(userLabelText);
                    shelf->appendChild(userLabel_tag);
            }


            DOMElement* rack_tag = 0;

            if(!rack.empty())
            {
                    rack_tag=doc->createElement(Xi("rack"));
                    DOMText* rackText = doc->createTextNode(Xi(rack.c_str()));
                    rack_tag->appendChild(rackText);
                    shelf->appendChild(rack_tag);
            }

            
            DOMElement* shelfType_tag = 0;

            if(!shelfType.empty())
            {
                    shelfType_tag=doc->createElement(Xi("shelfType"));
                    DOMText* shelfTypeText = doc->createTextNode(Xi(shelfType.c_str()));
                    shelfType_tag->appendChild(shelfTypeText);
                    shelf->appendChild(shelfType_tag);
            }
            

            DOMElement* position_tag = 0;

            if(!position.empty())
            {
                    position_tag=doc->createElement(Xi("position"));
                    DOMText* positionText = doc->createTextNode(Xi(position.c_str()));
                    position_tag->appendChild(positionText);
                    shelf->appendChild(position_tag);
            }            

            
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

int FIXS_CCH_NetconfShelfFilter::getListOfShelfs(std::string &xml)
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

    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(Xi("Core"));

    if (impl != NULL) {
        
        try 
        {
            DOMDocument* doc =impl->createDocument(0,Xi("ManagedElement"),0);

            DOMElement* rootElem = doc->getDocumentElement();

            rootElem->setAttribute(Xi("xmlns"), Xi("http://www.ericsson.com/dmx"));

            DOMElement* equipfElem = doc->createElement(Xi("Equipment"));

            rootElem->appendChild(equipfElem);

            DOMElement* shelf = doc->createElement(Xi("Shelf"));

            equipfElem->appendChild(shelf);

            DOMElement* shelfId_tag = doc->createElement(Xi("shelfId"));;

            shelf->appendChild(shelfId_tag);
            
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




int FIXS_CCH_NetconfShelfFilter::getXml(DOMDocument* Doc,std::string &xml) {

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
    
    
    std::cout << "DBG: " << " XML Query " << xmlStr.c_str() <<std::endl;
    
    xml=xmlStr;

    return 0;
}

