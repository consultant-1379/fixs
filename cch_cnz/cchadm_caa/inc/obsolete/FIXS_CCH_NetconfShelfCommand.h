#ifndef FIXS_CCH_NetconfShelfCommand_H
#define	FIXS_CCH_NetconfShelfCommand_H

#include "FIXS_CCH_NetconfCommand.h"
#include "FIXS_CCH_NetconfParser.h"
#include "FIXS_CCH_NetconfSender.h"
#include "FIXS_CCH_NetconfShelfFilter.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class FIXS_CCH_NetconfShelfCommand :public FIXS_CCH_NetconfCommand {

public:
    
    FIXS_CCH_NetconfShelfCommand();
    virtual ~FIXS_CCH_NetconfShelfCommand();
    
    virtual void setParser(FIXS_CCH_NetconfParser*);
    virtual void setSender(FIXS_CCH_NetconfSender*);
    virtual int execute();
    virtual void getResult(nresponse &);
    virtual void setXmlFilter(string filter);
    virtual const char *getErrorMessage();
    virtual int getErrorType();
    virtual int getErrorSeverity();
    virtual const char* getErrorMessageInfo();
    
    int createShelfQuery(std::string shelfId,std::string userLabel,std::string rack,std::string shelfType,std::string position,std::string physicalAddress);
    int getListOfShelfs();
    
    virtual void setFilter(FIXS_CCH_NetconfFilter* filter);
    
private:
     
    std::string xml_filter;
    
    const char *message_error;
    int  message_error_type;
    int  message_error_severity;
    const char *  message_error_info;

    FIXS_CCH_NetconfParser *_parser;
    FIXS_CCH_NetconfSender *_sender;
    FIXS_CCH_NetconfShelfFilter *_shelf_filter;
    
    nresponse _response;
    
};


#endif	/* FIXS_CCH_NetconfShelfCommand_H */

