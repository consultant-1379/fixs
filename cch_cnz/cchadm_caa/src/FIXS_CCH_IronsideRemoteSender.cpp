/* 
 * File:  FIXS_CCH_IronsideRemoteSender.cpp
 * Author: sarita 
 * 
 * Created on April 26 2013
 */

//#include "FIXS_CCH_IronsideCommandBuilder.h"

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "FIXS_CCH_IronsideRemoteSender.h"
#include "acs_nclib_session.h"
#include "acs_nclib_message.h"
#include "acs_nclib_hello.h"
#include "acs_nclib_rpc.h"
#include "acs_nclib_factory.h"
#include "acs_nclib_library.h"
#include "acs_nclib_udp_reply.h"
#include "acs_nclib_udp.h"
#include "stdio.h"
#include "stdlib.h"

//#define DMX_ADDRESS_A 	"192.254.15.1"
//#define DMX_ADDRESS_B 	"192.254.16.1"
#define DMX_PORT 		831
//#define DMX_USER 		"root"
//#define DMX_PWD 		"tre,14"
//#define DMX_ACTION_NS	"http://tail-f.com/ns/netconf/actions/1.0"
#define DMX_ACTION_NS "urn:com:ericsson:ecim:1.0"

#define DMX_DEFAULT_TIMEOUT_MS	5000


FIXS_CCH_IronsideRemoteSender::FIXS_CCH_IronsideRemoteSender(uint32_t dmxc_address1, uint32_t dmxc_address2, unsigned q_timeout):
	udp_data(""),
    	message_error(0),
	message_error_type(0),
	message_error_severity(0),
	message_error_info(0)
{
	_dmxc_addresses[0] = dmxc_address1;
	_dmxc_addresses[1] = dmxc_address2;
	_timeout_ms = (q_timeout > 0)? q_timeout: DMX_DEFAULT_TIMEOUT_MS;
}

FIXS_CCH_IronsideRemoteSender::~FIXS_CCH_IronsideRemoteSender()
{
    
}

const char* FIXS_CCH_IronsideRemoteSender::getErrorMessage()
{
        return NULL;
}

int FIXS_CCH_IronsideRemoteSender::getErrorType()
{
    return message_error_type;
}

std::string FIXS_CCH_IronsideRemoteSender::getUdpData()
{
	return udp_data;
}
int FIXS_CCH_IronsideRemoteSender::getErrorSeverity()
{
    return message_error_severity;
}

const char* FIXS_CCH_IronsideRemoteSender::getErrorMessageInfo()
{
    return message_error_info;
}

///////////////

int FIXS_CCH_IronsideRemoteSender::sendUdpMessage(std::string query)
{
	udp_data=std::string("");
	acs_nclib_session session(acs_nclib::UDP);
	std::string xml_filter = query;
        bool query_sent = false;
        udp_data = std::string("");
	int errfl=0;

	acs_nclib_udp* udp = acs_nclib_factory::create_udp(acs_nclib::OP_UDP_GET);
        for(int i=0; i<2 && !query_sent;i++)
	{
		if (session.open( _dmxc_addresses[i],DMX_PORT,acs_nclib::USER_AUTH_NONE,"") != acs_nclib::ERR_NO_ERRORS)
	      	{
			errfl=-1;
        	       	DEBUG("FIXS_CCH_NetConfRemoteSender::sendUdpMessage : session.open 1");               
			continue;
		}
		udp->set_cmd(query);
		acs_nclib_message* answer = 0;
		acs_nclib_udp_reply * reply=0;
		int result = 0;
		if ((result = session.send(udp)) == 0)
		{
		DEBUG("UDP Get Message sent:" << udp );
		if (session.receive(answer, 3000) == acs_nclib::ERR_NO_ERRORS)
		{
			reply=dynamic_cast<acs_nclib_udp_reply*>(answer);
			if((reply!=0) &&  (reply->get_message_id() == udp->get_message_id()))
			{
				reply->get_data(udp_data);
				errfl=0;
				DEBUG("Answer Received:" << udp_data);
			}
			else
			{	
				DEBUG("DBG: Receive message have different id or reply error" );
				errfl=-1;
			}			
			query_sent =true;
		}
		else
		{
			DEBUG("DBG: Receive Failed, error");
			errfl=-1;
		}
			if (answer)
				acs_nclib_factory::dereference(answer);
	    	}	
		else
		{
			DEBUG("Send Failed. RC = " << session.last_error_code() << " " << session.last_error_text());
			errfl=-1;
		}
		
		if (udp)
                	acs_nclib_factory::dereference(udp);
		if (session.close() == 0)
                	DEBUG("Session Closed");
	}

	return errfl;
}

