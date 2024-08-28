
#include "FIXS_CCH_IronsideUdpCommand.h"


#include <stdio.h>
#include <stdlib.h>
#include "FIXS_CCH_Util.h"
typedef std::map <std::string,std::string> nresponse; 


FIXS_CCH_IronsideUdpCommand::FIXS_CCH_IronsideUdpCommand(uint32_t dmx_addres1, uint32_t dmx_address2):
	message_error(0),
	message_error_type(0),
	message_error_severity(0),
	message_error_info(0),
	_sender(0),
	commandBuilder(dmx_addres1, dmx_address2)
{

}

FIXS_CCH_IronsideUdpCommand::~FIXS_CCH_IronsideUdpCommand() {
    

	if (_sender)
		delete(_sender);
        
        
}

void FIXS_CCH_IronsideUdpCommand::setSender(FIXS_CCH_IronsideSender* send)
{
	if (_sender)
	{
		delete _sender;
		_sender = 0;
	}
    _sender = send;
}

const char *FIXS_CCH_IronsideUdpCommand::getErrorMessage()
{
    return message_error;
}

int FIXS_CCH_IronsideUdpCommand::getErrorType()
{
    return message_error_type;
}


int FIXS_CCH_IronsideUdpCommand::getErrorSeverity()
{
    return message_error_severity;
}

const char* FIXS_CCH_IronsideUdpCommand::getErrorMessageInfo()
{
    return message_error_info;
}

int FIXS_CCH_IronsideUdpCommand::executeUdp(std::string xml_filter,std::string &udp_data)
{
	int result=0;
    	udp_data="";
    	int status=1;
    
    	if (!_sender)
	   return status;
   
	DEBUG("execute UDP  "<< xml_filter );
    	result=_sender->sendUdpMessage(xml_filter);
    	if(result==0)
    	{
		udp_data=_sender->getUdpData();
	}	
	return result;
}
