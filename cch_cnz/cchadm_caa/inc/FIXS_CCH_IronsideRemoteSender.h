/* 
 * File:   FIXS_CCH_IronsideRemoteSender.h
 * Author: xgunsar
 *
 * Created on 30 April 2013 */

#ifndef FIXS_CCH_IronsideRemoteSender_H
#define	FIXS_CCH_IronsideRemoteSender_H

#include "FIXS_CCH_IronsideSender.h"
#include "FIXS_CCH_Util.h"
class FIXS_CCH_IronsideRemoteSender : virtual public FIXS_CCH_IronsideSender{
public:
    FIXS_CCH_IronsideRemoteSender(uint32_t dmxc_address1, uint32_t dmxc_address2, unsigned = 5000);
    virtual ~FIXS_CCH_IronsideRemoteSender();

    
    const char* getErrorMessage();
    int         getErrorType();
    int         getErrorSeverity();
    const char* getErrorMessageInfo();


	virtual int sendUdpMessage(std::string query);
    std::string getUdpData();
    
private:
  
	std::string udp_data;
    const char *message_error;
    int  message_error_type;
    int  message_error_severity;
    const char *  message_error_info;
    char _dmxc_ipa[16];
    char _dmxc_ipb[16];
    uint32_t _dmxc_addresses[2];    
    unsigned _timeout_ms;
};

#endif	

