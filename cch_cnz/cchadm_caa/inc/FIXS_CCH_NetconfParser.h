/* 
 * File:   FIXS_CCH_NetconfParser.h
 * Author: xgunsar
 *
 * Created on 30 Apr 2013
 */

#ifndef FIXS_CCH_NetconfParser_H
#define	FIXS_CCH_NetconfParser_H

#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <map>
#include <string>

typedef std::map <std::string,std::string> nresponse;

using namespace xercesc;



class FIXS_CCH_NetconfParser : virtual HandlerBase {
public:

    
	FIXS_CCH_NetconfParser()
    {
        
    }

    virtual ~FIXS_CCH_NetconfParser()
    {
        
    }

    //	Metod to handle errors that occured during parsing
    virtual void warning (const SAXParseException &exception)=0;

    //	Metod to handle errors that occured during parsing
    virtual void error (const SAXParseException &exception)=0;


    //	Metod to handle errors that occured during parsing
    virtual void fatalError (const SAXParseException &exception)=0;

    //	Method to handle the start events that are generated for each element when
    //	an XML file is parsed
    
    virtual void characters (const XMLCh* const  chars, const unsigned int length)=0;
    
    //	Method to handle the start events that are generated for each element when
    //	an XML file is parsed
    
    inline void clearResponse() {_response.clear();};
    

    virtual void startElement(const XMLCh* const name,AttributeList& attributes)=0;
          
    virtual void getResponse(nresponse &res)=0;
    
    virtual void endElement(const XMLCh* const name)=0;
    
protected:
    nresponse _response;

};

#endif	/* FIXS_CCH_NetconfParser_H */

