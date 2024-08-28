#ifndef FIXS_CCH_NetconfShelfFilter_H
#define	FIXS_CCH_NetconfShelfFilter_H

#include <iostream>
#include <sstream>
#include "FIXS_CCH_NetconfFilter.h"


#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>

using namespace xercesc;

using namespace std;



class FIXS_CCH_NetconfShelfFilter : public FIXS_CCH_NetconfFilter {
    
    
public:
	FIXS_CCH_NetconfShelfFilter();
	FIXS_CCH_NetconfShelfFilter(std::string mag,int slot);
    virtual ~FIXS_CCH_NetconfShelfFilter();

    virtual int createShelfQuery(std::string shelfId,std::string userLabel,std::string rack,std::string shelfType,std::string position,std::string physicalAddress,std::string &xml);
    int getListOfShelfs(std::string &xml);
    int getShelfidFromPhysicalAddress(std::string physicalAddress,std::string &xml);
    
private:

    int getXml(DOMDocument*d,std::string&);

};

#endif

