#ifndef FIXS_AMIH_HWCSUBSCRIBER_H_
#define FIXS_AMIH_HWCSUBSCRIBER_H_ 23
#include <stdint.h>
#include "ACS_CS_API.h"
#include "ACS_TRA_trace.h"
#include "ACS_CS_API_ClientObserver.h"

#include "FIXS_AMIH_JTPHandler.h"


class FIXS_AMIH_HwcSubscriber : public ACS_CS_API_HWCTableObserver
{

  public:

      FIXS_AMIH_HwcSubscriber(FIXS_AMIH_JTPHandler  *aJtpHandler);

      virtual ~FIXS_AMIH_HwcSubscriber ();

      virtual void update(const ACS_CS_API_HWCTableChange& observer);

  private:

      void printTRA(std::string mesg);

      ACS_TRA_trace *_trace;

      FIXS_AMIH_JTPHandler *jtpHandler;

};

#endif /* FIXS_AMIH_HWCSUBSCRIBER_H_ */
