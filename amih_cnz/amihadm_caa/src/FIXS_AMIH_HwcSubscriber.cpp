#include "FIXS_AMIH_HwcSubscriber.h"

#include <stdio.h>
#include <iostream>
#include <string>


using namespace std;

FIXS_AMIH_HwcSubscriber::FIXS_AMIH_HwcSubscriber ( FIXS_AMIH_JTPHandler  *aJtpHandler)
{
	printf("FIXS_AMIH_HwcSubscriber::FIXS_AMIH_HwcSubscriber creation\n");
	jtpHandler = aJtpHandler;
	_trace= new ACS_TRA_trace("FIXS_AMIH_HwcSubscriber");
}


FIXS_AMIH_HwcSubscriber::~FIXS_AMIH_HwcSubscriber ()
{

}


void FIXS_AMIH_HwcSubscriber::update(const ACS_CS_API_HWCTableChange& /*observer*/)
{
	printf("FIXS_AMIH_HwcSubscriber::update\n");

	jtpHandler->setNotificationNeeded();

	printTRA("FIXS_AMIH_HwcSubscriber notification!");

}

void FIXS_AMIH_HwcSubscriber::printTRA(std::string mesg)
{
	if (_trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, mesg.c_str());
		_trace->ACS_TRA_event(1, tmpStr);

	}
}

