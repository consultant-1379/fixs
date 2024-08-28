#ifndef FIXS_AMIH_JTPClient_h
#define FIXS_AMIH_JTPClient_h 1

#include "ACS_JTP.h"
#include "ACS_TRA_trace.h"

class FIXS_AMIH_JTPClient
{
	public:

		FIXS_AMIH_JTPClient ();

		virtual ~FIXS_AMIH_JTPClient();

		int newJTPClient (unsigned short Len, ACS_JTP_Job &J1, unsigned short U1, unsigned short U2, char *Msg);

		bool waitForJTPResult (ACS_JTP_Job &J1, unsigned short Len, unsigned short U1, unsigned short U2, char *Msg);

		int waitForConfermation (ACS_JTP_Job &J1, unsigned short U1, unsigned short U2);

		void printTRA(std::string mesg);

	private:

		ACS_TRA_trace* _trace;

};

#endif
