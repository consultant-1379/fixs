/*
 * fixs_cch_netconfparser.h
 *
 *  Created on: Jan 28, 2013
 *      Author: estevol
 */

#ifndef FIXS_CCH_NETCONFPARSER_H_
#define FIXS_CCH_NETCONFPARSER_H_

#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <map>
#include <string>
#include <vector>

namespace dmx
{

	struct shelf
	{
		shelf(std::string id, std::string addr):
			shelfId(id),physicalAddress(addr) {}

		std::string shelfId;
		std::string physicalAddress;
	};
	struct bridgeport
	{
		bridgeport(std::string bridgeId, std::string portId ,int interfaceNo):
                        bridgeId(bridgeId),portId(portId),interfaceNo(interfaceNo) {}

                std::string bridgeId;
                std::string portId;
		int interfaceNo;
	};
}

using namespace xercesc;
class fixs_cch_cmdshelfmanager;

class fixs_cch_netconfparser : virtual  HandlerBase {
public:
	fixs_cch_netconfparser();
	fixs_cch_netconfparser(fixs_cch_cmdshelfmanager* man,int no=0);
	virtual ~fixs_cch_netconfparser();


	//	Metod to handle errors that occured during parsing
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

	virtual void getShelfList(std::vector<dmx::shelf> &);

	virtual int getIfNumber();

	virtual void getBridge(std::string &ret);
	virtual void getBridgePortList(std::map<std::string, std::vector<std::string> > &strlist);
	virtual void getTenantList(std::vector<std::string> &ret);

	virtual void getBladeList(std::vector<std::string> &ret);

	virtual void endElement(const XMLCh* const name);

protected:

	void resetFlags();

	bool fl_shelfId;
	bool fl_physicalAddress;
	std::vector<dmx::shelf> shelfList;
	std::map<std::string, std::vector<std::string> > bridgePortList;
	bool fl_bridgeId;
	std::vector<std::string>tenantList;
	std::vector<std::string>bladeList;
	bool fl_portId;
    bool fl_portAddress;
    bool fl_operState;
    bool fl_inOctets;
    bool fl_inUnicast;
    bool fl_inDiscards;
    bool fl_inErrors;
    bool fl_inMulticast;
    bool fl_inBroadcast;
    bool fl_outOctets;
    bool fl_outUnicast;
    bool fl_outDiscards;
    bool fl_outErrors;
    bool fl_outMulticast;
    bool fl_outBroadcast;
	bool f1_tenantId;
	int tenantCounter;
	fixs_cch_cmdshelfmanager* shelfManager;
	unsigned int interfaceNumber;
	bool f1_bladeId;
	std::string bridgeId;



};

#endif /* FIXS_CCH_NETCONFPARSER_H_ */
