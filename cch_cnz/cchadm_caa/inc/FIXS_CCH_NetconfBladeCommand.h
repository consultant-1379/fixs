/* 
 * File:   FIXS_CCH_IronsideUdpCommand.h
 * Author: xgunsar
 *
 * Created on 30 April 2013
 */

#ifndef FIXS_CCH_IronsideUdpCommand_H
#define	FIXS_CCH_IronsideUdpCommand_H

#include "FIXS_CCH_IronsideCommand.h"
#include "FIXS_CCH_IronsideSender.h"
#include "FIXS_CCH_IronsideCommandBuilder.h"
#include "FIXS_CCH_IMM_Util.h"

#include <iostream>
#include <sstream>
#include <string>

class FIXS_CCH_IronsideUdpCommand :  public FIXS_CCH_IronsideCommand
{

public:
    
    FIXS_CCH_IronsideUdpCommand(uint32_t dmx_address1, uint32_t dmx_address2);
    virtual ~FIXS_CCH_IronsideUdpCommand();
    
    
    virtual void setSender(FIXS_CCH_IronsideSender*);    
	virtual int executeUdp(std::string xml_filter,std::string &udp_data);    
    virtual const char *getErrorMessage();
    virtual int getErrorType();
    virtual int getErrorSeverity();
    virtual const char* getErrorMessageInfo();   
    
    
private:

    const char *message_error;
    int  message_error_type;
    int  message_error_severity;
    const char *  message_error_info;

    
    FIXS_CCH_IronsideSender *_sender;    
    FIXS_CCH_IronsideCommandBuilder commandBuilder;
   
    
};


#endif	/* FIXS_CCH_IronsideUdpCommand_H */

