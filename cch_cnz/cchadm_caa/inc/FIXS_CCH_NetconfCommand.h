/* 
 * File:   FIXS_CCH_IronsideCommand.h
 * Author: xgunsar
 *
 * Created on 30 Apr 2013
 */

#ifndef FIXS_CCH_IronsideCommand_H
#define	FIXS_CCH_IronsideCommand_H

#include <string>
#include "FIXS_CCH_IronsideSender.h"
 
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/XMLChar.hpp>

class FIXS_CCH_IronsideCommand {
public:
    
    FIXS_CCH_IronsideCommand()
    {
        
    }
    
    virtual ~FIXS_CCH_IronsideCommand()
    {

    }

    virtual void setSender(FIXS_CCH_IronsideSender*) = 0;    
    virtual const char *getErrorMessage() = 0;
    virtual int getErrorType() = 0;
    virtual int getErrorSeverity() = 0;
    virtual const char* getErrorMessageInfo() = 0;

};




#endif	/* FIXS_CCH_IronsideCommand_H */

