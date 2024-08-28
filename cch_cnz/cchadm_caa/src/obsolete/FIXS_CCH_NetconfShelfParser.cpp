
#include "FIXS_CCH_NetconfShelfParser.h"
#include <stdio.h>


FIXS_CCH_NetconfShelfParser::FIXS_CCH_NetconfShelfParser()
{
     resetFlags();   
    
}


FIXS_CCH_NetconfShelfParser::~FIXS_CCH_NetconfShelfParser()
{
 
}
    

void FIXS_CCH_NetconfShelfParser::warning(const SAXParseException &/*exception*/) {

}

//	Metod to handle errors that occured during parsing

void FIXS_CCH_NetconfShelfParser::error(const SAXParseException &/*exception*/) {

}


//	Metod to handle errors that occured during parsing

void FIXS_CCH_NetconfShelfParser::fatalError(const SAXParseException &/*exception*/) {

}

//	Method to handle the start events that are generated for each element when
//	an XML file is parsed

void FIXS_CCH_NetconfShelfParser::characters(const XMLCh * const chars, const unsigned int length)
{
    if(length<1)
        return;
    
    char * str=(char*)malloc(sizeof(int)*(length+1));
    
    memset(str,0,length+1);
    
    for(unsigned int t=0;t<length;t++)
    {
        str[t]=chars[t];
    }
    
    if(fl_shelfId)
    {
        //printf("%s = %s\n","shelfId",str);
        _response.find("shelfId")->second=std::string("")+str;
    }
    
    if(fl_userLabel)
    {
        //printf("%s = %s\n","Slot",str);
        _response.find("userLabel")->second=std::string("")+str;
    }
            
    if(fl_rack)
    {
        //printf("%s = %s\n","slotId",str);
        _response.find("rack")->second=std::string("")+str;
    }

    if(fl_shelfType)
    {
        //printf("%s = %s\n","Blade",str);
        _response.find("shelfType")->second=std::string("")+str;
    }
    
    if(fl_position)
    {
       //printf("valore %s = %s\n","bladeId",str);
        _response.find("position")->second=std::string("")+str;
    }
    
    if(fl_physicalAddress)
    {
        
       //printf("%s = %s\n","productName",str);
       _response.find("physicalAddress")->second=std::string("")+str;
    }
    
    
    resetFlags();

    memset(str,0,length+1);
    free(str);
    str=NULL;
    
    

}

void FIXS_CCH_NetconfShelfParser::startElement(const XMLCh * const name, AttributeList& ) {

   // std::cout << "I saw element: " << XMLString::transcode(name) << std::endl;

    char* elem = XMLString::transcode(name);
    std::string element(elem);

    resetFlags();
    
    if(element.compare("shelfId")==0)
    {
        
        std::string pN("shelfId");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
        
        fl_shelfId=true;
     
    }

    if(element.compare("userLabel")==0)
    {
        fl_userLabel=true;
        
        std::string pN("userLabel");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }


    if(element.compare("rack")==0)
    {
        
        fl_rack=true;
     
        std::string pN("rack");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }


    if(element.compare("shelfType")==0)
    {
        
        fl_shelfType=true;
     
        std::string pN("shelfType");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }

    
    if(element.compare("position")==0)
    {
     
        fl_position=true;
     
        std::string pN("position");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }
    

    if(element.compare("physicalAddress")==0)
    {
        fl_physicalAddress=true;
     
        std::string pN("physicalAddress");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }    
    

   /* 
    if (element == "WizardOpi") {
        std::cout << "DBG: " << __FUNCTION__ << " " << __LINE__ << std::endl;
        for (XMLSize_t i = 0; i < attributes.getLength(); i++) {
            char* nm = XMLString::transcode(attributes.getName(i));
            std::string name(nm);
            char* vl = XMLString::transcode(attributes.getValue(i));
            std::string value(vl);


            std::cout << "name: " << nm << std::endl;
            std::cout << "value: " << vl << std::endl;

            if (name == "name") {
                if (value == "scxb-kernel") {
                } else {
                    XMLString::release(&vl);
                    XMLString::release(&nm);
                    break;
                }
            } else if (name == "id") {
            } else if (name == "rstate") {
            } else {

            }

            XMLString::release(&vl);
            XMLString::release(&nm);
        }
    } else if (element == "file") {
        std::cout << "DBG: " << __FUNCTION__ << " " << __LINE__ << std::endl;
        for (XMLSize_t i = 0; i < attributes.getLength(); i++) {
            char* nm = XMLString::transcode(attributes.getName(i));
            std::string name(nm);
            char* vl = XMLString::transcode(attributes.getValue(i));
            std::string value(vl);

            if (name == "relpath") {
            } else if (name == "name") {
            } else if (name == "provider_prodnr") {
            } else if (name == "provider_rstate") {
            } else {

            }

            XMLString::release(&vl);
            XMLString::release(&nm);
        }
    }

    XMLString::release(&elem);
*/
     XMLString::release(&elem);
}

 void FIXS_CCH_NetconfShelfParser::getResponse(nresponse &res)
 {
     res = _response;
 }

// Additional Public Declarations


void FIXS_CCH_NetconfShelfParser::resetFlags()
{
     fl_shelfId=false;
     fl_userLabel=false;
     fl_rack=false;
     fl_shelfType=false;
     fl_position=false;
     fl_physicalAddress=false;
}


void FIXS_CCH_NetconfShelfParser::endElement(const XMLCh* const)
{
        
}
