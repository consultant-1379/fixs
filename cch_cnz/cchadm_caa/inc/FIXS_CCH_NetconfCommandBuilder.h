
#ifndef FIXS_CCH_IRONSIDECOMMANDBUILDER_H
#define	FIXS_CCH_IRONSIDECOMMANDBUILDER_H

#include "FIXS_CCH_IronsideBuilder.h"
#include "FIXS_CCH_IronsideCommand.h"

class FIXS_CCH_IronsideCommandBuilder : public FIXS_CCH_IronsideBuilder{
    public:
    
	FIXS_CCH_IronsideCommandBuilder(uint32_t dmxc_address1, uint32_t dmxc_address2);

     ~FIXS_CCH_IronsideCommandBuilder();
    FIXS_CCH_IronsideCommand* make(ironsideBuilder::builds x, const unsigned* = 0);
 
private:
    unsigned _sender_timeout;
    uint32_t _dmxc_addresses[2];
};


#endif	/* FIXS_CCH_IronsideCommandBuilder_H */

