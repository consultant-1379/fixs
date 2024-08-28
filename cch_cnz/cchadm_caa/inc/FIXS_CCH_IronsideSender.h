/* 
 * File:   FIXS_CCH_IronsideSender.h
 * Author: xgunsar
 *
 * Created on 30 April 2013
 */

#ifndef FIXS_CCH_IronsideSender_H
#define	FIXS_CCH_IronsideSender_H

#include <string>

class FIXS_CCH_IronsideSender{
public:
    
	FIXS_CCH_IronsideSender()
    {
        
    }
    
    virtual ~FIXS_CCH_IronsideSender()
    {
        
    }
    
   
	virtual int sendUdpMessage(std::string query)=0;
    virtual const char* getErrorMessage()=0;
    virtual int         getErrorType()=0;
    virtual int         getErrorSeverity()=0;
    virtual const char* getErrorMessageInfo()=0;
   
	virtual std::string getUdpData()=0;
   
   
    

protected:
 
  
 
};

#endif	/* FIXS_CCH_IronsideSender_H */

