#ifndef FIXS_CCH_IronsideBuilder_H
#define	FIXS_CCH_IronsideBuilder_H

#include "FIXS_CCH_IronsideCommand.h"


namespace ironsideBuilder
{
	enum builds {
		UDP             = 1
};

namespace PHYSICAL_POSITION
{
	struct query{
		char magazine[10];
		int slot;

	};
}


}

class FIXS_CCH_IronsideBuilder {
public:
    
	FIXS_CCH_IronsideBuilder()
    {
        
    }
    
    virtual ~FIXS_CCH_IronsideBuilder()
    {
        
    }
    virtual FIXS_CCH_IronsideCommand* make(ironsideBuilder::builds g, const unsigned* = 0) = 0;
    
private:

};


#endif	/* FIXS_CCH_IronsideBuilder_H */

