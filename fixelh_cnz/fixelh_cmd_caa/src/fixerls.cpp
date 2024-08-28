//******************************************************************************
//
//  NAME
//     fixserls.cpp
//
//  COPYRIGHT
//     Ericsson AB, Sweden 2013. All rights reserved.
//
//  The Copyright to the computer program(s) herein
//  is the property of Ericsson AB, Sweden.
//  The program(s) may be used and/or copied only with
//  the written permission from Ericsson AB or in
//  accordance with the terms and conditions stipulated in the
//  agreement/contract under which the program(s) have been
//  supplied.
//
//  DESCRIPTION
//     -
//
//  DOCUMENT NO
//	    190 89-CAA 109
//
//  AUTHOR
//     2007-04-26 by EAB/FTE/DDH UABCHSN (misclhls)
//     2011-01-28 by EAB/FLE/DD  UABMAGN "APG43 Linux (misclhls)
//     2013-06-04 by DEK  xdtthng APG43 Linux (based on misclhls)
//
//  SEE ALSO
//     -
//
//******************************************************************************


#include <cstdio>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <getopt.h>
#include <acs_prc_api.h>
//#include "ACS_PRC_Process.h"


#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>

#include "MAS_MISCLH_Fixs_Cmd.h"

using namespace std;

const char* GetDateString()
{
	static char szDate[16] = "";

	time_t now = time(0);      		// Current time
	tm result;                 		// Convert to calendar time or broken down time
	localtime_r(&now, &result);

	sprintf(szDate,"%04d%02d%02d",result.tm_year + 1900, result.tm_mon + 1, result.tm_mday);

	return szDate;
}

const char* GetTimeString()
{
	static char szTime[8] = "";
	time_t now = time(0);      		// Current time
	tm result;                 		// Convert to calendar time or broken down time
	localtime_r(&now, &result);

	sprintf(szTime,
			"%02d%02d",
			result.tm_hour,
			result.tm_min);

	return szTime;
}

void PrintUsage()
{
	cerr  << "Usage: fixerls  [-a start_time] [-b stop_time] [-c cp_id]\n"
			<< "                [-d start_date] [-e event_type]\n"
			<< "                [-g stop_date]" << endl;
} // End of PrintUsage


bool ValidateDate(const char* lpszDate)
{
	if (strlen(lpszDate) != 8)
		return false;

	char szDate[10];
	strcpy(szDate, lpszDate);
	int nDay = atol(szDate+6);
	if (nDay < 1 || nDay > 31)
		return false;

	szDate[6] = '\0';
	int nMon = atol(szDate+4);
	if (nMon < 1 || nMon > 12)
		return false;

	szDate[4] = '\0';
	int nYear = atol(szDate);
	if (nYear < 1900 || nYear > 2099)
		return false;

	if (nMon == 4 || nMon == 6 || nMon == 9 || nMon == 11)
	{
		if (nDay > 30)
			return false;
	}
	else if (nMon == 1 || nMon == 3 || nMon == 5 || nMon == 7 ||
			nMon == 8 || nMon == 10 || nMon == 12)
	{
		if (nDay > 31)
			return false;
	}
	else
	{
		if ( (nYear % 4) == 0)
		{
			if (nDay > 29)
				return false;
		}
		else
		{
			if (nDay > 28)
				return false;
		}
	}

	return true;
}


bool ValidateNumber(const char* lpszNumber)
{
	int nIndex = (*lpszNumber == '-' ? 1 : 0);

	if (lpszNumber[nIndex] == '\0')
		return false;

	while (lpszNumber[nIndex] != '\0')
	{
		if (lpszNumber[nIndex] < '0' || lpszNumber[nIndex] > '9')
			return false;
		else
			nIndex++;
	}

	return true;
}

bool ValidateTime(const char* lpszTime)
{
	if (strlen(lpszTime) != 4)
		return false;
	char szTime[8];
	int i=0;
	strcpy(szTime, lpszTime);
	for (i=0;i<4;i++)
	{
		if (isdigit(szTime[i]))
		{  }
		else{
			return false;
		}
	}
	int nMin = atol(szTime+2);
	if (nMin < 0 || nMin > 59)
		return false;

	szTime[2] = '\0';
	int nHour = atol(szTime);
	if (nHour < 0 || nHour > 23)
		return false;

	return true;
}


int checkIfActive()
{
	ACS_PRC_API prcApi;

	int nodeState = prcApi.askForNodeState();

	if (nodeState == 2) // Passive
	{
		return 2;
	}
	else if (nodeState == 1) //Active
	{
		return 1;
	}
	else
	{
		return -1; //Undefined
	}
}

int main(int argc, char* argv[])
{

	int ch,cAct;
	int a_flag = 0;
	int b_flag = 0;
	int c_flag = 0;
	int d_flag = 0;
	int e_flag = 0;
	int g_flag = 0;



	char* pStartTime = NULL;
	char* pStopTime = NULL;
	char* pCPNumber = NULL;
	char* pStartDate = NULL;
	char* pFaultCode = NULL;
	char* pStopDate = NULL;

	// AP_SetCleanupAndCrashRoutine(g_lpszName, NULL);
	//AP_InitProcess(g_lpszName, AP_COMMAND);

	cAct = checkIfActive();

	if (cAct == -1)
	{
		cerr << "AP node is in undefined state" <<endl;
		cerr << "cannot connect to server" <<endl;
		return 7;
	}
	else if (cAct == 2)//Passive node
	{
		cerr << "Unable to connect to server" << endl;
		return 117;
	}

	opterr = 0;
	bool bError = false;
	while ((ch = getopt(argc, argv, "a:b:c:d:e:g:")) != EOF)
	{
		switch (ch)
		{
		case 'a':
			a_flag++;
			pStartTime = optarg;
			break;

		case 'b':
			b_flag++;
			pStopTime = optarg;
			break;

		case 'c':
			c_flag++;
			pCPNumber = optarg;
			break;

		case 'd':
			d_flag++;
			pStartDate = optarg;
			break;

		case 'e':
			e_flag++;
			pFaultCode = optarg;
			break;

		case 'g':
			g_flag++;
			pStopDate = optarg;
			break;

		case '?':
			PrintUsage();
			return 1;

		default:
			break;
		} // End of case
	} // End of while
	// Option parsing
	if (a_flag > 1 || b_flag > 1 || c_flag > 1 || d_flag > 1 || e_flag > 1 || g_flag > 1)
	{
		bError = true;

	}

	// Operands or arguments not handled
	if (argv[optind] != NULL)
	{
		bError = true;

	}

	// Usage error
	if (bError)
	{
		cerr << "Incorrect usage" << endl;
		PrintUsage();
		return 2;
	}

	// Validate start time argument
	if (a_flag)
	{
		if (!ValidateTime(pStartTime))
		{
			cerr << "Unreasonable value" << endl;
			return 24;
		}
	}

	// Validate stop time argument
	if (b_flag)
	{
		if (!ValidateTime(pStopTime))
		{
			cerr << "Unreasonable value" << endl;
			return 24;
		}
	}

	// Validate CP number argument
	if (c_flag)
	{
		if (!ValidateNumber(pCPNumber))
		{
			cerr << "Incorrect usage" << endl;
			PrintUsage();
			return 2;
		}

		int nTemp = atol(pCPNumber);
		if (nTemp < 0 || nTemp > 63)
		{
			cerr << "Unreasonable value" << endl;
			return 24;
		}
	}
	// Validate start date argument
	if (d_flag)
	{
		if (!ValidateDate(pStartDate))
		{
			cerr << "Unreasonable value" << endl;
			return 24;
		}
	}

	// Validate fault code argument
	if (e_flag)
	{
		if (!ValidateNumber(pFaultCode))
		{
			cerr << "Incorrect usage" << endl;
			PrintUsage();
			return 2;
		}

		int nTemp = atol(pFaultCode);
		if (nTemp < 0 || nTemp > 9999)
		{
			cerr << "Unreasonable value" << endl;
			return 24;
		}
	}
	// Validate stop date argument
	if (g_flag)
	{
		if (!ValidateDate(pStopDate))
		{
			cerr << "Unreasonable value" << endl;
			return 24;
		}
	}

	char szStartTime[8] = "";
	if (pStartTime)
		strcpy(szStartTime, pStartTime);

	char szStopTime[8] = "";
	if (pStopTime)
		strcpy(szStopTime, pStopTime);

	char szStartDate[10] = "";
	if (pStartDate)
		strcpy(szStartDate, pStartDate);

	char szStopDate[10] = "";
	if (pStopDate)
		strcpy(szStopDate, pStopDate);

	// Start date was given but start time was not
	if (!pStartTime && pStartDate)
	{
		strcpy(szStartTime, "0000");
#ifdef DEBUG
		cout << "Start time set to 00:00" << endl;
#endif
	}

	// Stop date was given but stop time was not
	if (!pStopTime && pStopDate)
	{
		strcpy(szStopTime, "2359");
#ifdef DEBUG
		cout << "Stop time set to 23:59" << endl;
#endif
	}

	// Start time and/or stop time was given but start date and/or stop date was not
	if ( (pStartTime && !pStartDate) || (pStopTime && !pStopDate) )
	{
		boost::gregorian::date d = boost::gregorian::day_clock::universal_day();

		// Assign the start date to the current date
		if (pStartTime && !pStartDate)
		{
			int year = d.year();
			int month =d.month();
			int day =d.day();
			sprintf(szStartDate,"%04d%02d%02d",year,month,day);

#ifdef DEBUG
			cout << "Start date set to " << szStartDate << endl;
#endif
		}

		// Assign the stop date to the current date
		if (pStopTime && !pStopDate)
		{
			int year = d.year();
			int month =d.month();
			int day =d.day();
			sprintf(szStopDate,"%04d%02d%02d",year,month,day);

#ifdef DEBUG
			cout << "Stop date set to " << szStopDate << endl;
#endif
		}
	}

	// Check that stop date+time is larger than start date+time
	if (*szStartTime && *szStopTime)
	{
		if (strcmp(szStartDate, szStopDate) == 0)
		{
			if (strcmp(szStartTime, szStopTime) > 0)
			{
				// Start date+time is larger then stop date+time
				cerr <<"Unreasonable value\nStart time is later than the stop time" << endl;
				return 28;
			}
		}
		else
		{
			if (strcmp(szStartDate, szStopDate) > 0)
			{
				// Start date is larger then stop date
				cerr << "Unreasonable value\nStart time is later than the stop time" << endl;
				return 28;
			}
		}
	}

	char pid[8]={0};
	sprintf(pid,"%d",getpid());


	// Format string is for the following header
	// "CP-R", "DATE", "TIME", "EVENTNO", "EVENTTYPE", "INF1", "INF2", "INF3", "INF4"
	string format = "%4s  %-8s  %-6s  %7s  %9s  %-6s  %-6s  %-6s  %-6s";

	MAS_MISCLH_Fixs_Cmd cmd(pid);
	cmd.setCmdOptions("-a", (*szStartTime != '\0' ? szStartTime : ""));
	cmd.setCmdOptions("-b", (*szStopTime != '\0' ? szStopTime : ""));
	cmd.setCmdOptions("-d", (*szStartDate != '\0' ? szStartDate : ""));
	cmd.setCmdOptions("-g", (*szStopDate != '\0' ? szStopDate : ""));
	cmd.setCmdOptions("-c", (c_flag > 0 ? pCPNumber : ""));
	cmd.setCmdOptions("-e", (e_flag > 0 ? pFaultCode : ""));
	cmd.setCmdOptions("-p", pid);
	cmd.setCmdOptions("-output_format", format.c_str());
	format += "\n";

	int result = cmd.execute();
	int exitCode = 0;

	int recCount = 0;

	if (result == MISCLH_RC_OK) {
		result = cmd.readCommandData();
		if (result == MISCLH_RC_NOMATCH) {
			printf("FIXS INFRASTRUCTURE EVENT RECORD\n\n");
			printf("%-12s  %-12s\n", "CURRENT DATE", "CURRENT TIME");
			printf("%-12s  %-12s\n\n", GetDateString(), GetTimeString());

			printf(format.c_str(),
					"CP-R", "DATE", "TIME", "EVENTNO", "EVENTTYPE", "INF1", "INF2", "INF3", "INF4");
		}
		else if (result == MISCLH_RC_OK || result == MISCLH_RC_DATA) {
			bool eventRecordHeaderPrinted = false;
			long printoutType = 0;
			bool done = false;
			while (!done) {
				int numOfData = cmd.getNumberOfData();
				//cout << "***\n\n numOfData = " << numOfData << "\n***\n" << endl;
				for (int idx = 0; idx < numOfData ; ++idx) {
					if (idx % 2 == 0) {
						printoutType = cmd.getResultType(idx);
					}
					else {
						if (printoutType == MISCLH_PRINT_TYPE_EVENT_RECORD && !eventRecordHeaderPrinted) {

							printf("FIXS INFRASTRUCTURE EVENT RECORD\n\n");
							printf("%-12s  %-12s\n", "CURRENT DATE", "CURRENT TIME");
							printf("%-12s  %-12s\n\n", GetDateString(), GetTimeString());

							// ---  "%4s  %-8s  %-6s  %7s  %9s  %-6s  %-6s  %-6s  %-6s\n";
							printf(format.c_str(),
									"CP-R", "DATE", "TIME", "EVENTNO", "EVENTTYPE", "INF1", "INF2", "INF3", "INF4");
							eventRecordHeaderPrinted = true;
						}
						// Print the event record data
						cout << cmd.getResult(idx) << endl;
						++recCount;
					}

				}

				// There is likely more data to fetch
				//cout << "*** result is " << result << " *** " << endl;
				if (result == MISCLH_RC_DATA) {
					//cout << "**** read more data **** " << endl;
					result = cmd.readCommandData();
					done = !(result == MISCLH_RC_OK || result == MISCLH_RC_DATA);
				}
				else {
					//cout << " *** no more data reading **" << endl;
					done = true;
				}
			}


			if (result != MISCLH_RC_OK && result != MISCLH_RC_NO_MORE_DATA)
				cerr << "\n" << cmd.getResultCodeText(result, exitCode) << endl;
		}
		else {
			cerr << cmd.getResultCodeText(result, exitCode) << endl;
		}
	}
	else {
		cerr << cmd.getResultCodeText(result, exitCode) << endl;
	}

	//cout << "\nTotal record printed = " << recCount << "\n" << endl;

	return exitCode;
}
