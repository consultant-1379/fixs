/* 
 * File:   FIXS_CCH_IronsideCommandBuilder.cpp
 * Author: xgunsar
 * 
 * Created on 30 April 2013
 */

#include "FIXS_CCH_IronsideCommandBuilder.h"
#include "FIXS_CCH_IronsideUdpCommand.h"
#include "FIXS_CCH_IronsideRemoteSender.h"
#include<stdarg.h>

FIXS_CCH_IronsideCommandBuilder::FIXS_CCH_IronsideCommandBuilder(uint32_t address1, uint32_t address2):
_sender_timeout(0)
{
	 _dmxc_addresses[0]= address1;
	 _dmxc_addresses[1]= address2;
}

FIXS_CCH_IronsideCommandBuilder::~FIXS_CCH_IronsideCommandBuilder()
{
    
}

FIXS_CCH_IronsideCommand * FIXS_CCH_IronsideCommandBuilder::make(ironsideBuilder::builds choice, const unsigned *sender_timeout)
{
    
	_sender_timeout = (sender_timeout)? *sender_timeout: 0;

	FIXS_CCH_IronsideCommand *coMac = 0;
	FIXS_CCH_IronsideSender  *sender = 0;
   
 
    switch (choice)
    {
    case ironsideBuilder::UDP:
		
	 coMac = new FIXS_CCH_IronsideUdpCommand(_dmxc_addresses[0], _dmxc_addresses[1]);
	 sender = new FIXS_CCH_IronsideRemoteSender(_dmxc_addresses[0], _dmxc_addresses[1], _sender_timeout);
		 
	 coMac->setSender(sender);
		 
	 break;  
    default:
    	break;

    }           
    return coMac;


}

