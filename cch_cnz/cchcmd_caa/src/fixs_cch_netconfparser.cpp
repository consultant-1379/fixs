/*
 * fixs_cch_netconfparser.cpp
 *
 *  Created on: Jan 28, 2013
 *      Author: estevol
 */

#include "fixs_cch_netconfparser.h"
#include "fixs_cch_cmdshelfmanager.h"

fixs_cch_netconfparser::fixs_cch_netconfparser():
shelfManager(0),
interfaceNumber(0)
{
	resetFlags();
}

fixs_cch_netconfparser::fixs_cch_netconfparser(fixs_cch_cmdshelfmanager* man, int inf_no):
shelfManager(man),
interfaceNumber(inf_no)
{
	resetFlags();
}

fixs_cch_netconfparser::~fixs_cch_netconfparser()
{

}


void fixs_cch_netconfparser::warning(const SAXParseException &/*exception*/) {

}

//	Metod to handle errors that occured during parsing

void fixs_cch_netconfparser::error(const SAXParseException &/*exception*/) {

}


//	Metod to handle errors that occured during parsing

void fixs_cch_netconfparser::fatalError(const SAXParseException &/*exception*/) {

}

//	Method to handle the start events that are generated for each element when
//	an XML file is parsed

void fixs_cch_netconfparser::characters(const XMLCh * const chars, const unsigned int length)
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
        int pos = shelfList.size() - 1;
        shelfList.at(pos).shelfId = str;
    }

    if(fl_physicalAddress)
    {
    	int pos = shelfList.size() - 1;
    	shelfList.at(pos).physicalAddress = str;
    }

    if (fl_portId)
    {

   	std::string bridgeid,portid; 
	string::size_type pos_start = 0, pos_end;
        std::string bridge_port = str;
	pos_end = bridge_port.find(":",pos_start);
    	if (pos_end != string::npos)
    	{
	        bridgeid = bridge_port.substr(0, pos_end);
		portid = bridge_port.substr(pos_end+1);      
    	}
	bridgePortList[bridgeid].push_back(portid);
    	if (shelfManager)
    	{
    		shelfManager->m_ifAlias[interfaceNumber] = portid;
    	}
    }

    if (fl_portAddress)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifphysaddress[interfaceNumber] = str;
    	}
    }

    if (fl_operState)
    {
    	if (shelfManager)
    	{
    		if (strcmp(str,"ENABLED") == 0)
    		{
    			shelfManager->m_ifOperStatus[interfaceNumber] = "UP";
    		}
    		else
    		{
    			shelfManager->m_ifOperStatus[interfaceNumber] = "DOWN";
    		}
    	}
    }

    if (fl_inOctets)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifHCInOctets[interfaceNumber ] = strtoull(str, NULL, 0);
    	}
    }


    if (fl_inUnicast)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifHCInUcastPkts[interfaceNumber] = strtoull(str, NULL, 0);
    	}
    }

    if (fl_inDiscards)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifInDiscards[interfaceNumber ] = strtoul(str, NULL, 0);
    	}
    }

    if (fl_inErrors)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifInErrors[interfaceNumber] = strtoul(str, NULL, 0);
    	}
    }

    if (fl_inMulticast)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifHCInMulticastPkts[interfaceNumber] = strtoull(str, NULL, 0);
    	}
    }

    if (fl_inBroadcast)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifHCInBroadcastPkts[interfaceNumber] = strtoull(str, NULL, 0);
    	}
    }

    if (fl_outOctets)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifHCOutOctets[interfaceNumber] = strtoull(str, NULL, 0);
    	}
    }

    if (fl_outUnicast)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifHCOutUcastPkts[interfaceNumber] = strtoull(str, NULL, 0);
    	}
    }

    if (fl_outDiscards)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifOutDiscards[interfaceNumber] = strtoul(str, NULL, 0);
    	}
    }

    if (fl_outErrors)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifOutErrors[interfaceNumber] = strtoul(str, NULL, 0);
    	}
    }

    if (fl_outMulticast)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifHCOutMulticastPkts[interfaceNumber] = strtoull(str, NULL, 0);
    	}
    }

    if (fl_outBroadcast)
    {
    	if (shelfManager)
    	{
    		shelfManager->m_ifHCOutBroadcastPkts[interfaceNumber] = strtoull(str, NULL, 0);
    	}
    }

    if (fl_bridgeId)
    {
    	bridgeId = str;
    }
    if(f1_tenantId)
    {
	tenantList.push_back(str);
    }
    if(f1_bladeId)
    {
	bladeList.push_back(str);
    }
    resetFlags();

    memset(str,0,length+1);
    free(str);
    str=NULL;


}

void fixs_cch_netconfparser::startElement(const XMLCh * const name, AttributeList& ) {
    char* elem = XMLString::transcode(name);
    std::string element(elem);
    resetFlags();

    if(element.compare("Shelf") == 0)
    {
    	dmx::shelf new_shelf("","");
    	shelfList.push_back(new_shelf);
    }

    if(element.compare("shelfId")==0)
    {
        fl_shelfId = true;
    }

    if(element.compare("physicalAddress")==0)
    {
        fl_physicalAddress = true;
    }

    if(element.compare("bridgeId") == 0)
    {

    	fl_bridgeId = true;
    }

    if (element.compare("BridgePort") == 0)
    {
    }


    if(element.compare("bridgePortId")==0)
    {
    	fl_portId = true;
    }

    if(element.compare("portAddress")==0)
    {
    	fl_portAddress = true;

    }

    if(element.compare("operState")==0)
    {
    	fl_operState = true;

    }

    if(element.compare("inOctets")==0)
    {
    	fl_inOctets = true;

    }

    if(element.compare("inUnicast")==0)
    {
    	fl_inUnicast = true;

    }

    if(element.compare("inDiscards")==0)
    {
    	fl_inDiscards = true;

    }

    if(element.compare("inErrors")==0)
    {
    	fl_inErrors = true;

    }

    if(element.compare("inMulticast")==0)
    {
    	fl_inMulticast = true;

    }

    if(element.compare("inBroadcast")==0)
    {
    	fl_inBroadcast = true;

    }

    if(element.compare("outOctets")==0)
    {
    	fl_outOctets = true;

    }

    if(element.compare("outUnicast")==0)
    {
    	fl_outUnicast = true;

    }

    if(element.compare("outDiscards")==0)
    {
    	fl_outDiscards = true;

    }

    if(element.compare("outErrors")==0)
    {
    	fl_outErrors = true;

    }

    if(element.compare("outMulticast")==0)
    {
    	fl_outMulticast = true;

    }

    if(element.compare("outBroadcast")==0)
    {
    	fl_outBroadcast = true;

    }

    if(element.compare("tenantId")==0)
    {
        f1_tenantId = true;

    }
    if(element.compare("bladeId")==0)
    {
        f1_bladeId = true;

    }

     XMLString::release(&elem);
}


// Additional Public Declarations


void fixs_cch_netconfparser::resetFlags()
{
     fl_shelfId=false;
     fl_physicalAddress=false;

     fl_portId = false;
     fl_portAddress = false;
     fl_operState = false;
     fl_inOctets = false;
     fl_inUnicast = false;
     fl_inDiscards = false;
     fl_inErrors = false;
     fl_inMulticast = false;
     fl_inBroadcast = false;
     fl_outOctets = false;
     fl_outUnicast = false;
     fl_outDiscards = false;
     fl_outErrors = false;
     fl_outMulticast = false;
     fl_outBroadcast = false;
     f1_tenantId=false;
     fl_bridgeId = false;
     f1_bladeId = false;
}

void fixs_cch_netconfparser::getShelfList(std::vector<dmx::shelf> &ret)
{
	ret = shelfList;
}
void fixs_cch_netconfparser::getBridgePortList(std::map<string,vector<std::string> > &ret)
{
        ret = bridgePortList ;
}
int fixs_cch_netconfparser::getIfNumber()
{
	return interfaceNumber;
}

void fixs_cch_netconfparser::getBridge(std::string &ret)
{
	ret = bridgeId;
}
void fixs_cch_netconfparser::getTenantList(std::vector<std::string> &ret)
{
        ret = tenantList;
}

void fixs_cch_netconfparser::getBladeList(std::vector<std::string> &ret)
{
        ret = bladeList;
}
void fixs_cch_netconfparser::endElement(const XMLCh* const)
{

}

