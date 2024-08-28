

#include "FIXS_CCH_NetconfBladeParser.h"
#include <stdio.h>
#include <sstream>


FIXS_CCH_NetconfBladeParser::FIXS_CCH_NetconfBladeParser()
{
        resetFlags();
        m_name = false;
        firstIpmifirmwaredata=false;
        secondIpmifirmwaredata=false;
        thirdIpmifirmwaredata=false;

        m_IpmiFirmwareDataProductNumber=false;
        m_IpmiFirmwareDataType=false;

        m_IpmiFirmwareDataProductRevisionState=false;
        m_IpmiFirmwareDataVersion=false;
        m_IpmiFirmwareData=false;
        m_IpmiFirmwareDataRunningVersion=false;

        m_rState=false;
        m_faultLed=false;
        m_ledStatus=false;
        m_maintenanceLed=false;
        m_operationalLed=false;
        m_statusLed=false;
        m_tenantId=false;
        m_ledType=false;
        tenantCounter=0;
}
      
FIXS_CCH_NetconfBladeParser::~FIXS_CCH_NetconfBladeParser()
{
 
}
    

void FIXS_CCH_NetconfBladeParser::warning(const SAXParseException &/*exception*/) {

}

//	Metod to handle errors that occured during parsing

void FIXS_CCH_NetconfBladeParser::error(const SAXParseException &/*exception*/) {

}


//	Metod to handle errors that occured during parsing

void FIXS_CCH_NetconfBladeParser::fatalError(const SAXParseException &/*exception*/) {

}

//	Method to handle the start events that are generated for each element when
//	an XML file is parsed

void FIXS_CCH_NetconfBladeParser::characters(const XMLCh * const chars, const unsigned int length)
{
    if(length < 1)
        return;
    
    char * str=(char*)malloc(sizeof(int)*(length+1));

    memset(str,0,length+1);

    for(unsigned int t=0;t<length;t++)
    {
        str[t]=chars[t];
    }

    cout<<"FIXS_CCH_NetconfBladeParser::characters==> STR = "<<str<<endl;
    nresponse::iterator it;


    if(fl_shelf)
    {
        //printf("%s = %s\n","Shelf",str);
        it = _response.find("Shelf");
        if (it != _response.end())
        	it->second=std::string("")+str;
    
    }

    if(fl_shelfId)
    {
        //printf("%s = %s\n","shelfId",str);
        it = _response.find("shelfId");

        if (it != _response.end())
        	it->second=std::string("")+str;
    }
    
    if(fl_Slot)
    {
        //printf("%s = %s\n","Slot",str);
        it = _response.find("Slot");
        if (it != _response.end())
        	it->second=std::string("")+str;
    }
            
    if(fl_slotId)
    {
        printf("aaaaaaaaaaaaaaaaaaaaaaaa %s = %s\n","slotId",str);
        it = _response.find("slotId");
        if (it != _response.end())
        	it->second=std::string("")+str;
    }

    if(fl_Blade)
    {
        printf("%s = %s\n","Blade",str);
        it = _response.find("Blade");
        if (it != _response.end())
        	it->second=std::string("")+str;
    }
    
    if(fl_bladeId)
    {
       //printf("valore %s = %s\n","bladeId",str);
        it = _response.find("bladeId");
        if (it != _response.end())
        	it->second=std::string("")+str;
    }
    
    if(fl_productName)
    {
       //printf("%s = %s\n","productName",str);
       it = _response.find("productName");
       if (it != _response.end())
    	   it->second=std::string("")+str;
    }
    
    if(fl_productNumber)
    {
        //printf("%s = %s\n","productNumber",str);

        it = _response.find("productNumber");
        if (it != _response.end())
        	it->second=std::string("")+str;
    }
    
    if(fl_productRevisionState)
    {
        //printf("%s = %s\n","productRevisionState",str);
        it = _response.find("productRevisionState");
        if (it != _response.end())
        	it->second=std::string("")+str;
    }

    if(fl_serialNumber)
    {
    	it = _response.find("serialNumber");
    	if (it != _response.end())
    		it->second=std::string("")+str;
    }

    if(fl_manufacturingDate)
    {
    	it = _response.find("manufacturingDate");
    	if (it != _response.end())
    		it->second=std::string("")+str;
    }

    if(fl_vendorName)
    {

    	it = _response.find("vendorName");
    	if (it != _response.end())
    		it->second = std::string("")+str;
    }
    
    if(fl_firstMacAddress)
    {
        //printf("%s = %s\n","firstMacAddress",str);
        it = _response.find("firstMacAddress");
        if (it != _response.end())
        	it->second=std::string("")+str;
    }
    


      if (m_IpmiFirmwareData)
      {
		std::string name = XMLString::transcode(chars);
//		std::cout <<"--------------------------- name = " << name.c_str() << std::endl;
      }
     
     if(m_IpmiFirmwareDataProductNumber)
     {
         if(firstIpmifirmwaredata)
         {
              std::string name = XMLString::transcode(chars);
              sprintf(ipm1.productNumber,"%s",name.c_str());
              // printf("ipm1.productNumber %s values inserted into ipmi1\n",ipm1.productNumber);
              
         }

         if(secondIpmifirmwaredata)
         {
             std::string name = XMLString::transcode(chars);
             sprintf(ipm2.productNumber,"%s",name.c_str());
             // printf("ipm2.productNumber %s values inserted into ipmi2\n",ipm2.productNumber);
         }
         
     }


     if(m_IpmiFirmwareDataProductRevisionState)
     {
         if(firstIpmifirmwaredata)
         {
              std::string name = XMLString::transcode(chars);
              sprintf(ipm1.productRevisionState,"%s",name.c_str());
              // printf("ipm1.productRevisionState %s values inserted into ipmi1\n",ipm1.productRevisionState);
              
         }

         if(secondIpmifirmwaredata)
         {
              std::string name = XMLString::transcode(chars);
              sprintf(ipm2.productRevisionState,"%s",name.c_str());
              // printf("ipm2.productRevisionState %s values inserted into ipmi2\n",ipm2.productRevisionState);
         }
         
     }


     if(m_IpmiFirmwareDataVersion)
     {
         if(firstIpmifirmwaredata)
         {
              std::string name = XMLString::transcode(chars);
              sprintf(ipm1.version,"%s",name.c_str());
              // printf("ipm1.version %s values inserted into ipmi1\n",ipm1.version);
              
         }

         if(secondIpmifirmwaredata)
         {
              std::string name = XMLString::transcode(chars);
              sprintf(ipm2.version,"%s",name.c_str());
              // printf("ipm2.version %s values inserted into ipmi2\n",ipm2.version);
         }
         
     }

     if(m_IpmiFirmwareDataType)
     {
         if(firstIpmifirmwaredata)
         {
              std::string name = XMLString::transcode(chars);
              sprintf(ipm1.type,"%s",name.c_str());
              // printf("ipm1.type %s values inserted into ipmi1\n",ipm1.type);
              
         }

         if(secondIpmifirmwaredata)
         {
              std::string name = XMLString::transcode(chars);
              sprintf(ipm2.type,"%s",name.c_str());
              // printf("ipm2.type %s values inserted into ipmi2\n",ipm2.type);
         }
         
     }
     
     if(m_IpmiFirmwareDataRunningVersion)
     {
         if(thirdIpmifirmwaredata)
         {
              std::string name = XMLString::transcode(chars);
              sprintf(ipm3.runningVersion,"%s",name.c_str());
             // printf("ipm3.runningVersion %s values inserted into ipmi3\n",ipm3.runningVersion);
         }
     }


     if(m_biosRunMode)
     {
    	 //std::string name = XMLString::transcode(chars);
    	 it = _response.find("biosRunMode");
    	 if (it != _response.end())
    		 it->second=std::string("")+str;
     }

     if (m_fatalEventLog)
     {
    	// std::string name = XMLString::transcode(chars);
    	 it = _response.find("biosRunMode");
    	 if (it != _response.end())
    		 it->second=std::string("")+str;
     }

     if(m_powerState)
     {
    	// std::string name = XMLString::transcode(chars);
	cout <<" inside m_powerState " << str << endl ;
    	 it = _response.find("pwr");
    	 if (it != _response.end())
    		 it->second=std::string("")+str;

     }


    if(m_ledStatus)
	{
    	m_ledStatus=false;

    	if(m_maintenanceLed)
    	{
    		it = _response.find("maintenanceLed");
    		if (it != _response.end())
    			it->second=std::string("")+str;
    		m_maintenanceLed=false;
    	}

    	if(m_operationalLed)
    	{
    		it = _response.find("operationalLed");
    		if (it != _response.end())
    			it->second=std::string("")+str;
    		m_operationalLed=false;
    	}

    	if(m_statusLed)
    	{
    		it = _response.find("statusLed");
    		if (it != _response.end())
    			it->second=std::string("")+str;
    		m_statusLed=false;
    	}

    	if(m_faultLed)
    	{
    		it = _response.find("faultLed");
    		if (it != _response.end())
    			it->second=std::string("")+str;
    		m_faultLed=false;
    	}

	}
	if (m_ledType)
        {
		it = _response.find("ledType");
                if (it != _response.end())
			it->second = std::string("")+str;
                m_ledType=false;
        } 
	if(m_getGprData)
     	{
        	// std::string name = XMLString::transcode(chars);
	         cout <<" inside getGprData " << str << endl ;
        	 it = _response.find("getGprData");
	         if (it != _response.end())
                 it->second=std::string("")+str;

        }
	
	if(m_getGprRetVal)
     	{
        	// std::string name = XMLString::transcode(chars);
	         cout <<" inside getGprRetVal " << str << endl ;
        	 it = _response.find("returnValue");
	         if (it != _response.end())
                 it->second=std::string("")+str;

        }
	if(m_gprData)
        {
                // std::string name = XMLString::transcode(chars);
                 cout <<" inside gprData " << str << endl ;
                 it = _response.find("gprData");
                 if (it != _response.end())
		{
			cout << "setting gprData" << endl;	
	                 it->second=std::string("")+str;
		}
		 m_gprData=false;

        }
        if(m_boardConfiguration)
        {
        	it = _response.find("boardConfiguration");
                if (it != _response.end())
        	{
			it->second=std::string("")+str;
		}         
	}
        if(m_tenantId)
        {
    	       std::stringstream ss;
    	       ss << tenantCounter;
    	       std::string mapKey = "tenantId_" + ss.str();
    	       _response.find(mapKey)->second=std::string("")+str;

    	       _response.find("tenantCounter")->second=std::string("")+ss.str();
       } 

    resetFlags();

   // XMLString::release(&str);

   memset(str,0,length+1);
   free(str);
   // delete(str);
    str=NULL;
}

void FIXS_CCH_NetconfBladeParser::startElement(const XMLCh * const name, AttributeList& ) {

   // std::cout << "I saw element: " << XMLString::transcode(name) << std::endl;

    char* elem = XMLString::transcode(name);
    std::string element(elem);

    resetFlags();
    
    startIpmiFirmwareData(element);
    
    if(element.compare("bladeId")==0)
    {

    	printf("start element bladeId\n");

        
        std::string pN("bladeId");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
        
        fl_bladeId=true;
     
    }

    if(element.compare("productName")==0)
    {
        fl_productName=true;
        
        std::string pN("productName");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }


    if(element.compare("productNumber")==0)
    {
        
        fl_productNumber=true;
     
        std::string pN("productNumber");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }


    if(element.compare("productRevisionState")==0)
    {
        
        fl_productRevisionState=true;
     
        std::string pN("productRevisionState");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }

    if(element.compare("serialNumber")==0)
    {
    	fl_serialNumber=true;

    	std::string pN("serialNumber");
    	std::string emp("");

    	_response.insert(std::make_pair(pN,emp));
    }


    if(element.compare("manufacturingDate")==0)
    {

    	fl_manufacturingDate=true;

    	std::string pN("manufacturingDate");
    	std::string emp("");

    	_response.insert(std::make_pair(pN,emp));
    }


    if(element.compare("vendorName")==0)
    {

    	fl_vendorName=true;

    	std::string pN("vendorName");
    	std::string emp("");

    	_response.insert(std::make_pair(pN,emp));
    }

    
    if(element.compare("firstMacAddress")==0)
    {
        fl_firstMacAddress=true;
     
        std::string pN("firstMacAddress");
        std::string emp("");
       
        _response.insert(std::make_pair(pN,emp));
    }

    if((element.compare("pwr")==0))
    {
    	m_powerState=true;

    	printf("start element powerState\n");
        std::string pN("pwr");
        std::string emp("");
        _response.insert(std::make_pair(pN,emp));
    }

    if((element.compare("operationalLed")==0))
    {
        m_operationalLed=true;

        printf("start element operationalLed\n");
        std::string pN("operationalLed");
        std::string emp("");
        _response.insert(std::make_pair(pN,emp));
    }

    if((element.compare("maintenanceLed")==0))
    {
    	printf("start element maintenanceLed\n");
    	m_maintenanceLed=true;
        std::string pN("maintenanceLed");
        std::string emp("");
        _response.insert(std::make_pair(pN,emp));
    }


    if((element.compare("faultLed")==0))
    {
    	printf("start element faultLed\n");
    	m_faultLed=true;
        std::string pN("faultLed");
        std::string emp("");
        _response.insert(std::make_pair(pN,emp));
    }


    if((element.compare("statusLed")==0))
    {
    	m_statusLed=true;
        std::string pN("statusLed");
        std::string emp("");
        _response.insert(std::make_pair(pN,emp));
    }

    if((element.compare("status")==0))
    {
    	m_ledStatus=true;
    }
    if((element.compare("ledType")==0))
    {
	m_ledType=true;
        std::string pN("ledType");
        std::string emp("");
	_response.insert(std::make_pair(pN,emp));
    }

    if((element.compare("boardConfiguration")==0))
    {
        m_boardConfiguration=true;
        std::string pN("boardConfiguration");
        std::string emp("");
        _response.insert(std::make_pair(pN,emp));
    }
    if((element.compare("biosRunMode")==0))
    {
    	m_biosRunMode=true;

    	std::string pN("biosRunMode");
    	std::string emp("");

    	_response.insert(std::make_pair(pN,emp));
    }

    if((element.compare("fatalEventLog")==0))
    {
    	m_biosRunMode=true;

    	std::string pN("fatalEventLog");
    	std::string emp("");

    	_response.insert(std::make_pair(pN,emp));
    }


    if((element.compare("getGprData")==0))
    {
	m_getGprData= true;

        std::string pN("getGprData");
        std::string emp("");
        _response.insert(std::make_pair(pN,emp));
    }

    if((element.compare("returnValue")==0))
    {
	m_getGprRetVal= true;

        std::string pN("returnValue");
        std::string emp("");

        _response.insert(std::make_pair(pN,emp));
    }
    if(element.compare("gprData")==0)
    {

        printf("start element gprData\n");


        std::string pN("gprData");
        std::string emp("");

        _response.insert(std::make_pair(pN,emp));

        m_gprData=true;

    }

    if(element.compare("tenantId")==0)
    {

    	if (tenantCounter == 0)
    	{
    		std::string pN("tenantCounter");
    		std::string emp("");

    		_response.insert(std::make_pair(pN,emp));
    	}

    	std::stringstream ss;
    	ss << ++tenantCounter;

    	std::string pN = "tenantId_" + ss.str();
    	std::string emp("");

    	_response.insert(std::make_pair(pN,emp));


    	m_tenantId=true;

    }



     XMLString::release(&elem);
}

 void FIXS_CCH_NetconfBladeParser::getResponse(nresponse &res)
 {
        res = _response;
	nresponse::iterator it;
        it = _response.find("getGprData");
        if (it != _response.end())
        cout << "found the string in filter" << it->first << "value " << it->second << endl;

     
 }

// Additional Public Declarations


void FIXS_CCH_NetconfBladeParser::resetFlags()
{
     fl_shelf=false;
     fl_shelfId=false;
     fl_Slot=false;
     fl_slotId=false;
     fl_Blade=false;
     fl_bladeId=false;
     fl_productName=false;
     fl_productNumber=false;
     fl_productRevisionState=false;
     fl_serialNumber = false;
     fl_manufacturingDate = false;
     fl_vendorName = false;
     fl_firstMacAddress=false;
     m_biosRunMode=false;
     m_powerState=false;
     m_fatalEventLog = false;
     m_getGprData = false;
     m_getGprRetVal = false;
     m_gprData = false;
     m_tenantId=false;
     m_ledType=false;
     m_boardConfiguration=false;

}


void FIXS_CCH_NetconfBladeParser::endElement(const XMLCh* const name)
{
     char* elem = XMLString::transcode(name);

    std::string element(elem);
    
    if (element == "getFirmwareData")
    {
    	//	std::cout <<"END element = " << element.c_str() << std::endl;
    	m_name = false;
    	firstIpmifirmwaredata=false;
    	secondIpmifirmwaredata=false;
    	thirdIpmifirmwaredata=false;

    	setOutputFirmwareData();
    }

    if (element == "productNumber")
    {
    	//std::cout <<"END element = " << element.c_str() << std::endl;
    	m_IpmiFirmwareDataProductNumber=false;
    }

    if (element == "type")
    {
    	//	std::cout <<"END element = " << element.c_str() << std::endl;
    	m_IpmiFirmwareDataType=false;

    }

    if (element == "productRevisionState")
    {
    	//std::cout <<"END element = " << element.c_str() << std::endl;
    	m_IpmiFirmwareDataProductRevisionState=false;
    }

    if (element == "version")
    {
    	m_IpmiFirmwareDataVersion=false;
    }

    if (element == "IpmiFirmwareData")
    {
    	//std::cout <<"END element = " << element.c_str() << std::endl;
    	m_IpmiFirmwareData=false;
    }


    if (element == "runningVersion")
    {
    	//std::cout <<"END element = " << element.c_str() << std::endl;
        m_IpmiFirmwareDataRunningVersion=false;
    }

    if(element == "gprData")
    {	
	cout << "END ELEMENT " << endl;
	m_gprData = false;

    }	

    XMLString::release(&elem);
        
}

void FIXS_CCH_NetconfBladeParser::startIpmiFirmwareData(std::string element)
{
    if (m_name || (element == "getFirmwareData"  ))
    {
        if(element == "getFirmwareData"  )
        {
            m_name=true;
        }
        else
        {
            if (m_IpmiFirmwareData || (element == "IpmiFirmwareData"))
            {
                if(element == "IpmiFirmwareData")
                {
                    m_IpmiFirmwareData = true;

                    if(!firstIpmifirmwaredata && !secondIpmifirmwaredata && !thirdIpmifirmwaredata)
                    {
                        firstIpmifirmwaredata=true;
                    }
                    else
                    {
                        if(firstIpmifirmwaredata  && !secondIpmifirmwaredata && !thirdIpmifirmwaredata)
                        {
                           firstIpmifirmwaredata=false;
                           secondIpmifirmwaredata=true;
                        }
                        else
                        {
                           if(!firstIpmifirmwaredata  && secondIpmifirmwaredata && !thirdIpmifirmwaredata)
                           {
                              firstIpmifirmwaredata=false;
                              secondIpmifirmwaredata=false;
                              thirdIpmifirmwaredata=true;
                           }
                        }
                    }
                }
                else
                {
                    if (element == "productNumber")
                    {
                            m_IpmiFirmwareDataProductNumber = true;
                    }

                    if (element == "version")
                    {
                            m_IpmiFirmwareDataVersion = true;
                    }

                    if (element == "productRevisionState")
                    {
                            m_IpmiFirmwareDataProductRevisionState = true;
                    }

                    if (element == "type")
                    {
                            m_IpmiFirmwareDataType = true;
                    }


                    if (element == "runningVersion")
                    {
                            m_IpmiFirmwareDataRunningVersion = true;
                    }

                }
            }
        }
    }
}


void FIXS_CCH_NetconfBladeParser::setOutputFirmwareData()
{
    
    ipmifirmwaredata ipm;
    
    sprintf(ipm.runningVersion,"%s",ipm3.runningVersion);
    
    if(strcmp(ipm1.type,ipm3.runningVersion)==0)
    {
        sprintf(ipm.type,"%s",ipm1.type);
        sprintf(ipm.productNumber,"%s",ipm1.productNumber);
        sprintf(ipm.productRevisionState,"%s",ipm1.productRevisionState);
        sprintf(ipm.version,"%s",ipm1.version);
        
//        printf("IMP1:\n");
//        printf("IMP1:%s\n",ipm1.productNumber);
    }
    
    
    if(strcmp(ipm2.type,ipm3.runningVersion)==0)
    {
        sprintf(ipm.type,"%s",ipm2.type);
        sprintf(ipm.productNumber,"%s",ipm2.productNumber);
        sprintf(ipm.productRevisionState,"%s",ipm2.productRevisionState);
        sprintf(ipm.version,"%s",ipm2.version);
//        printf("IMP2");
//        printf("IMP2:%s\n",ipm2.productNumber);
    }

    _response.clear();
    
    std::string pN("productNumber");
    std::string emp(ipm.productNumber);

     _response.insert(std::make_pair(pN,emp));

     pN="productRevisionState";
     emp=ipm.productRevisionState;

     _response.insert(std::make_pair(pN,emp));

     pN="version";
     emp=ipm.version;

     _response.insert(std::make_pair(pN,emp));

     pN="runningVersion";
     emp=ipm.runningVersion;

     _response.insert(std::make_pair(pN,emp));

     memset(ipm1.type,0,sizeof(ipm1.type));
     memset(ipm1.productRevisionState,0,sizeof(ipm1.productRevisionState));
     memset(ipm1.version,0,sizeof(ipm1.version));
     memset(ipm1.runningVersion,0,sizeof(ipm1.runningVersion));
     memset(ipm1.productNumber,0,sizeof(ipm1.productNumber));
     
     memset(ipm2.type,0,sizeof(ipm2.type));
     memset(ipm2.productRevisionState,0,sizeof(ipm2.productRevisionState));
     memset(ipm2.version,0,sizeof(ipm2.version));
     memset(ipm2.runningVersion,0,sizeof(ipm2.runningVersion));
     memset(ipm2.productNumber,0,sizeof(ipm2.productNumber));
     
}
