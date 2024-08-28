/*
 * fixs_cch_netconfbuilder.h
 *
 *  Created on: Jan 25, 2013
 *      Author: estevol
 */


#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_USE

#ifndef FIXS_CCH_NETCONFBUILDER_H_
#define FIXS_CCH_NETCONFBUILDER_H_

class fixs_cch_netconfbuilder {
public:
	fixs_cch_netconfbuilder();
	virtual ~fixs_cch_netconfbuilder();

	int createShelfQuery(std::string shelfId, std::string physicalAddress, std::string &xml);
	int createIfQuery(std::string shelfId, std::string slotNumber,std::string tenant,  std::string interface, std::string &xml);
	int createBridgeQuery(std::string shelfId, std::string slotNumber,  std::string tenant,std::string &xml);
	
	int createBladeQuery(  std::string tenant,std::string &xml);
	int createTenantQuery(std::string &xml);
private:
	int getXml(DOMDocument* Doc, std::string &xml);

protected:

    class XStr {
    public:
        // -----------------------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------------------

        XStr(const char* const toTranscode) {
            // Call the private transcoding method
            fUnicodeForm = XMLString::transcode(toTranscode);
        }

        ~XStr() {
            XMLString::release(&fUnicodeForm);
        }

        const XMLCh* unicodeForm() const {
            return fUnicodeForm;
        }

    private:
        XMLCh* fUnicodeForm;
    };

    #define X(str) XStr(str).unicodeForm()
};

#endif /* FIXS_CCH_NETCONFBUILDER_H_ */
