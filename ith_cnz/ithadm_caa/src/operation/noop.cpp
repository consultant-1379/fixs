//	********************************************************
//
//	 COPYRIGHT Ericsson 2015
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2015.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2015 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************
#include "operation/noop.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programconstants.h"

//FIXS_DDT_TRACE_DEFINE(FIXS_ITH_Operation_NoOp)

int cnt = 0;

namespace operation
{

  NoOp::NoOp()
  : OperationBase(NOOP)
  {
  //    FIXS_ITH_TRACE_FUNCTION;
  }

  int NoOp::call()
  {
	  ++cnt;

	  // FIXS_ITH_TRACE_FUNCTION;
      FIXS_ITH_LOG(LOG_LEVEL_WARN, "NoOp executed, cnt == %d", cnt);

      if(cnt == 5)
    	  m_operationResult.setErrorCode(fixs_ith::ERR_NO_ERRORS);
      else
    	  m_operationResult.setErrorCode(fixs_ith::ERR_GENERIC);

      setResultToCaller();
      return fixs_ith::ERR_NO_ERRORS;
  }

} /* namespace operation */
